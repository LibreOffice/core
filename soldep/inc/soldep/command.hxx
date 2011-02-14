/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
    sal_Bool            bTmpWrite;

public:
                    CommandLine(sal_Bool bTmpWrite = sal_False);
                    CommandLine(const char *, sal_Bool bTmpWrite = sal_False);
                    CommandLine(const CommandLine&, sal_Bool bTmpWrite = sal_False);
    virtual         ~CommandLine();

    int             nArgc;

    CommandLine&    operator=(const CommandLine&);
    CommandLine&    operator=(const char *);
    void            BuildCommand(const char *);
    char**          GetCommand(void) { return ppArgv; }
    void            Strtokens(const char *);
    void            Print();
};

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
    sal_uIntPtr             nArgc;
    int                 nError;

protected:
    void            ImplInit();
    void            Initpp( sal_uIntPtr nCount, ByteString &rStr );

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
    static ByteString   Search( ByteString sEnv,
                                    ByteString sItem = COMMAND_SHELL );

                    /** Spawns the Process
                        @return 0 when spawned without errors, otherwise a error code
                    */
    operator int();

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

typedef sal_uIntPtr CommandBits;

/** Allowes to spawn programms hidden, waiting etc.
    @see CCommand
*/
class CCommandd : public CCommand
{
    CommandBits     nFlag;
public:
                    CCommandd( ByteString &rString, CommandBits nBits );
                    CCommandd( const char *pChar, CommandBits nBits );
    operator int();
};

#endif
