#include "i2plauncher.h"
#include "repugnoapplication.h"
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QDebug>


#ifdef WIN32

#include <windows.h>
#endif

#include <stdio.h>

#ifdef WIN32
#include <string.h>
LPWSTR ConvertToLPWSTR( const std::string& s )
{
  LPWSTR ws = new wchar_t[s.size()+1]; // +1 for zero at the end
  copy( s.begin(), s.end(), ws );
  ws[s.size()] = 0; // zero at the end
  return ws;
}
#endif

I2PLauncher::I2PLauncher(QString i2pPath) : m_i2pPath(i2pPath)
{
}

void I2PLauncher::Run()
{
    QProcess p;
    QString cmd = m_i2pPath + "/i2p";
#ifdef WIN32
    cmd = cmd + ".exe";
    // Quick fix for Windows, turning slashes.
    m_i2pPath = QString(m_i2pPath.replace("/","\\\\"));
#endif

    QDir logDir(RepugnoApplication::applicationDirPath() +QDir::separator()+"Logs");
    if (!logDir.exists()) logDir.mkdir(logDir.absolutePath());

    // Logging
    p.setStandardErrorFile(logDir.absolutePath()+QDir::separator()+"i2p.stderr.log",QIODevice::Append);
    p.setStandardOutputFile(logDir.absolutePath()+QDir::separator()+"i2p.stdout.log",QIODevice::Append);

#ifdef WIN32
#if WINVER == 0x0602
// Why?
// http://msdn.microsoft.com/en-us/library/windows/desktop/ms686331(v=vs.85).aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa383745(v=vs.85).aspx#macros_for_conditional_declarations
#include "Processthreadsapi.h"
#endif

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process.
    if( !CreateProcess( NULL,   // No module name (use command line)
        ConvertToLPWSTR(cmd.toStdString()),        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        CREATE_NO_WINDOW, // Don't show console to the end user.
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure

    )
    {
        //qDebug() <<  sprintf( "CreateProcess failed (%d).\n", GetLastError()) 

        // Adding fallback for i2pd start
        qDebug() << "Could not start I2P, trying failover solution";
        QProcess::execute(cmd);

        return;
    }
    // I2P Process started
    WaitForSingleObject( pi.hProcess, INFINITE );

    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
#else

    p.start(cmd);
    p.waitForFinished(-1);
#if QT_VERSION < 0x050300
    qint64 i2pPid = p.pid();
    //ProcessId first added in Qt 5.3
#else
    qint64 i2pPid = p.processId();
#endif
    qDebug() << "I2P should have started now. With process id: " << QString::number(i2pPid);

#endif

}
