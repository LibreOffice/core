/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: command.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-04 13:54:25 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef COMMAND_HXX
#define COMMAND_HXX

#include <iostream>

#include <tools/stream.hxx>
#define STRLEN 100
#ifndef UNX
#define TMPNAME "\\command.tmp"
#else
#define TMPNAME "/tmp/command.tmp"
#endif

/** Different types of spawnable programs
*/
enum ExeType
{
    EXE,    /// programm is a native executable
    BAT,    /// programm is a DOS-Batch
    BTM     /// programm is a 4DOS-Batch
};

#define COMMAND_NOTFOUND    0x0001
#define COMMAND_TOOBIG      0x0002
#define COMMAND_INVALID     0x0004
#define COMMAND_NOEXEC      0x0008
#define COMMAND_NOMEM       0x0010
#define COMMAND_UNKNOWN     0x0020

#ifdef WNT
#define COMMAND_SHELL   "4nt.exe"
#endif
#ifdef OS2
#define COMMAND_SHELL  "4os2.exe"
#endif
#ifdef UNX
#define COMMAND_SHELL   "csh"
#endif

class CommandLine;
class LogWindow;

class CommandLine
{
friend class ChildProcess;
private:
    char            *CommandBuffer;
    char            *ComShell;
    char            **ppArgv;
    BOOL            bTmpWrite;

public:
                    CommandLine(BOOL bTmpWrite = FALSE);
                    CommandLine(const char *, BOOL bTmpWrite = FALSE);
                    CommandLine(const CommandLine&, BOOL bTmpWrite = FALSE);
    virtual         ~CommandLine();

    int             nArgc;

    CommandLine&    operator=(const CommandLine&);
    CommandLine&    operator=(const char *);
    void            BuildCommand(const char *);
    char**          GetCommand(void) { return ppArgv; }
    void            Strtokens(const char *);
    void            Print();
};

static ByteString thePath( "PATH" );

/** Declares and spawns a child process.
    The spawned programm could be a native executable or a schell script.
*/
class CCommand
{
private:
    ByteString          aCommandLine;
    ByteString          aCommand;
    char                *pArgv;
    char                **ppArgv;
    ULONG               nArgc;
    int                 nError;

protected:
    void            ImplInit();
    void            Initpp( ULONG nCount, ByteString &rStr );

public:
                    /** Creates the process specified without spawning it
                        @param rString specifies the programm or shell scrip
                    */
                    CCommand( ByteString &rString );

                    /** Creates the process specified without spawning it
                        @param pChar specifies the programm or shell scrip
                    */
                    CCommand( const char *pChar );

                    /** Try to find the given programm in specified path
                        @param sEnv specifies the current search path, defaulted by environment
                        @param sItem specifies the system shell
                        @return the Location (when programm was found)
                    */
    static ByteString   Search( ByteString sEnv = thePath,
                                    ByteString sItem = COMMAND_SHELL );

                    /** Spawns the Process
                        @return 0 when spawned without errors, otherwise a error code
                    */
    operator const  int();

    ByteString          GetCommandLine_() { return aCommandLine; }
    ByteString          GetCommand() { return aCommand; }

    char**  GetCommandStr() { return ppArgv; }
};

#define COMMAND_EXECUTE_WINDOW  0x0000001
#define COMMAND_EXECUTE_CONSOLE 0x0000002
#define COMMAND_EXECUTE_HIDDEN  0x0000004
#define COMMAND_EXECUTE_START   0x0000008
#define COMMAND_EXECUTE_WAIT    0x0000010
#define COMMAND_EXECUTE_REMOTE  0x1000000

typedef ULONG CommandBits;

/** Allowes to spawn programms hidden, waiting etc.
    @see CCommand
*/
class CCommandd : public CCommand
{
    CommandBits     nFlag;
public:
                    CCommandd( ByteString &rString, CommandBits nBits );
                    CCommandd( const char *pChar, CommandBits nBits );
    operator const  int();
};

#endif
