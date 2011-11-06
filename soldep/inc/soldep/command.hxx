/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
