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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_registry.hxx"

#include <stdio.h>
#include <string.h>

#include "registry/registry.hxx"
#include "registry/reflread.hxx"
#include <rtl/ustring.hxx>
#include <rtl/alloc.h>
#include <osl/process.h>
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/file.hxx>

#ifdef SAL_UNX
#define SEPARATOR '/'
#else
#define SEPARATOR '\\'
#endif

using namespace ::rtl;
using namespace ::osl;

sal_Bool isFileUrl(const OString& fileName)
{
    if (fileName.indexOf("file://") == 0 )
        return sal_True;
    return sal_False;
}

OUString convertToFileUrl(const OString& fileName)
{
    if ( isFileUrl(fileName) )
    {
        return OStringToOUString(fileName, osl_getThreadTextEncoding());
    }

    OUString uUrlFileName;
    OUString uFileName(fileName.getStr(), fileName.getLength(), osl_getThreadTextEncoding());
    if ( fileName.indexOf('.') == 0 || fileName.indexOf(SEPARATOR) < 0 )
    {
        OUString uWorkingDir;
        if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None)
        {
            OSL_ASSERT(false);
        }
        if (FileBase::getAbsoluteFileURL(uWorkingDir, uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    } else
    {
        if (FileBase::getFileURLFromSystemPath(uFileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    }

    return uUrlFileName;
}

#define U2S( s ) \
    OUStringToOString(s, RTL_TEXTENCODING_UTF8).getStr()
#define S2U( s ) \
    OStringToOUString(s, RTL_TEXTENCODING_UTF8)

struct LessString
{
    sal_Bool operator()(const OUString& str1, const OUString& str2) const
    {
        return (str1 < str2);
    }
};

enum Command {
       DELETEKEY
};

class Options
{
public:
    Options()
        : m_bVerbose(false)
        {}
    ~Options()
        {}

    bool initOptions(int ac, char* av[]);

    OString prepareHelp();
    OString prepareVersion();

    const OString& getProgramName()
        { return m_program; }
    const OString& getTypeReg()
        { return m_typeRegName; }
    const OString& getKeyName()
        { return m_keyName; }
    Command getCommand()
        { return m_command; }
    bool verbose()
        { return m_bVerbose; }
protected:
    OString     m_program;
    OString     m_typeRegName;
    OString     m_keyName;
    Command     m_command;
    bool        m_bVerbose;
};

bool Options::initOptions(int ac, char* av[])
{
    bool bRet = true;
    sal_uInt16  i=1;

    if (ac < 2)
    {
        fprintf(stderr, "%s", prepareHelp().getStr());
        bRet = sal_False;
    }

    m_program = av[0];
    sal_Int32 index = -1;
    if ((index=m_program.lastIndexOf(SEPARATOR)) > 0)
        m_program = av[0]+index+1;

    char    *s=NULL;
    for (; i < ac; i++)
    {
        if (av[i][0] == '-')
        {
            switch (av[i][1])
            {
                case 'r':
                case 'R':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                            bRet = sal_False;
                            break;
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }
                    m_typeRegName = OString(s);
                    break;
                case 'd':
                case 'D':
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            s = av[i];
                        } else
                        {
                            fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                            bRet = sal_False;
                            break;
                        }
                    } else
                    {
                        s = av[i] + 2;
                    }
                    m_keyName = OString(s);
                    break;
                case 'v':
                case 'V':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                        bRet = sal_False;
                    }
                    m_bVerbose = true;
                    break;
                case 'h':
                case '?':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                        bRet = false;
                    } else
                    {
                        fprintf(stdout, "%s", prepareHelp().getStr());
                        exit(0);
                    }
                    break;
                default:
                    fprintf(stderr, "%s: unknown option '%s'\n", m_program.getStr(), av[i]);
                    bRet = false;
                    break;
            }
        } else
        {
            fprintf(stderr, "%s: unknown option '%s'\n", m_program.getStr(), av[i]);
            bRet = false;
        }
    }

    return bRet;
}

OString Options::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " -r<filename> <command>\n";
    help += "    -r<filename>  = filename specifies the name of the type registry.\n";
    help += "Commands:\n";
    help += "    -d <keyname>  = delete the specified key from the registry. Keyname\n";
    help += "                    specifies the name of the key that get deleted.\n";
    help += "    -v            = verbose output.\n";
    help += "    -h|-?         = print this help message and exit.\n";
    help += prepareVersion();

    return help;
}

OString Options::prepareVersion()
{
    OString version(m_program);
    version += " Version 1.0\n\n";
    return version;
}

static Options options;


#if (defined UNX) || (defined OS2) || (defined __MINGW32__)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    if ( !options.initOptions(argc, argv) )
    {
        exit(1);
    }

    OUString typeRegName( convertToFileUrl(options.getTypeReg()) );

    Registry typeReg;

    if ( typeReg.open(typeRegName, REG_READWRITE) )
    {
        fprintf(stderr, "%s: open registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getTypeReg().getStr());
        exit(2);
    }

    RegistryKey typeRoot;
    if ( typeReg.openRootKey(typeRoot) )
    {
        fprintf(stderr, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getTypeReg().getStr());
        exit(3);
    }

    if ( options.getCommand() == DELETEKEY )
    {
        if ( typeRoot.deleteKey(S2U(options.getKeyName())) )
          {
            fprintf(stderr, "%s: delete key \"%s\" of registry \"%s\" failed\n",
                    options.getProgramName().getStr(), options.getKeyName().getStr(), options.getTypeReg().getStr());
            exit(4);
        } else {
          if (options.verbose())
            fprintf(stderr, "%s: delete key \"%s\" of registry \"%s\"\n",
                    options.getProgramName().getStr(), options.getKeyName().getStr(), options.getTypeReg().getStr());
        }
    }

    typeRoot.releaseKey();
    if ( typeReg.close() )
    {
        fprintf(stderr, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getTypeReg().getStr());
        exit(5);
    }
}


