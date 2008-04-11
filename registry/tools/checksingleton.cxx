/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: checksingleton.cxx,v $
 * $Revision: 1.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_registry.hxx"
#include <stdio.h>
#include <string.h>

#include <set>
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

typedef ::std::set< OUString, LessString > StringSet;

class Options
{
public:
    Options()
        : m_bForceOutput(sal_False)
        {}
    ~Options()
        {}

    sal_Bool initOptions(int ac, char* av[], sal_Bool bCmdFile=sal_False);

    OString prepareHelp();
    OString prepareVersion();

    const OString& getProgramName()
        { return m_program; }
    const OString& getIndexReg()
        { return m_indexRegName; }
    const OString& getTypeReg()
        { return m_typeRegName; }
    sal_Bool hasBase()
        { return m_base.getLength() > 0; }
    const OString& getBase()
        { return m_base; }
    sal_Bool forceOutput()
        { return m_bForceOutput; }
protected:
    OString     m_program;
    OString     m_indexRegName;
    OString     m_typeRegName;
    OString     m_base;
    sal_Bool    m_bForceOutput;
};

sal_Bool Options::initOptions(int ac, char* av[], sal_Bool bCmdFile)
{
    sal_Bool bRet = sal_True;
    sal_uInt16  i=0;

    if (!bCmdFile)
    {
        bCmdFile = sal_True;

        m_program = av[0];

        if (ac < 2)
        {
            fprintf(stderr, "%s", prepareHelp().getStr());
            bRet = sal_False;
        }

        i = 1;
    } else
    {
        i = 0;
    }

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
                case 'o':
                case 'O':
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
                    m_indexRegName = OString(s);
                    break;
                case 'b':
                case 'B':
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
                    m_base = OString(s);
                    break;
                case 'f':
                case 'F':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                        bRet = sal_False;
                    }
                    m_bForceOutput = sal_True;
                    break;
                    case 'h':
                case '?':
                    if (av[i][2] != '\0')
                    {
                        fprintf(stderr, "%s: invalid option '%s'\n", m_program.getStr(), av[i]);
                        bRet = sal_False;
                    } else
                    {
                        fprintf(stdout, "%s", prepareHelp().getStr());
                        exit(0);
                    }
                    break;
                default:
                    fprintf(stderr, "%s: unknown option '%s'\n", m_program.getStr(), av[i]);
                    bRet = sal_False;
                    break;
            }
        } else
        {
            if (av[i][0] == '@')
            {
                FILE* cmdFile = fopen(av[i]+1, "r");
                  if( cmdFile == NULL )
                  {
                    fprintf(stderr, "%s", prepareHelp().getStr());
                    bRet = sal_False;
                } else
                {
                    int rargc=0;
                    char* rargv[512];
                    char  buffer[512];

                    while ( fscanf(cmdFile, "%s", buffer) != EOF )
                    {
                        rargv[rargc]= strdup(buffer);
                        rargc++;
                    }
                    fclose(cmdFile);

                    bRet = initOptions(rargc, rargv, bCmdFile);

                    for (long j=0; j < rargc; j++)
                    {
                        free(rargv[j]);
                    }
                }
            } else
            {
                fprintf(stderr, "%s: unknown option '%s'\n", m_program.getStr(), av[i]);
                bRet = sal_False;
            }
        }
    }

    return bRet;
}

OString Options::prepareHelp()
{
    OString help("\nusing: ");
    help += m_program + " -r<filename> -o<filename> [-options] | @<filename>\n";
    help += "    -o<filename>  = filename specifies the name of the new singleton index registry.\n";
    help += "    -r<filename>  = filename specifies the name of the type registry.\n";
    help += "    @<filename>    = filename specifies a command file.\n";
    help += "Options:\n";
    help += "    -b<name>  = name specifies the name of a start key. The types will be searched\n";
    help += "                under this key in the type registry.\n";
    help += "    -f        = force the output of all found singletons.\n";
    help += "    -h|-?     = print this help message and exit.\n";
    help += prepareVersion();

    return help;
}

OString Options::prepareVersion()
{
    OString version("\nSun Microsystems (R) ");
    version += m_program + " Version 1.0\n\n";
    return version;
}

static Options options;

static sal_Bool checkSingletons(RegistryKey& singletonKey, RegistryKey& typeKey)
{
    RegValueType valueType = RG_VALUETYPE_NOT_DEFINED;
    sal_uInt32 size = 0;
    OUString tmpName;
    sal_Bool bRet = sal_False;

    RegError e = typeKey.getValueInfo(tmpName, &valueType, &size);

    if ( e != REG_VALUE_NOT_EXISTS && e != REG_INVALID_VALUE && valueType == RG_VALUETYPE_BINARY)
    {
        RegistryKey entryKey;
        RegValue value = rtl_allocateMemory(size);

        typeKey.getValue(tmpName, value);

        RegistryTypeReader reader((sal_uInt8*)value, size, sal_False);

        if ( reader.isValid() && reader.getTypeClass() == RT_TYPE_SINGLETON )
        {
            OUString singletonName = reader.getTypeName().replace('/', '.');
            if ( singletonKey.createKey(singletonName, entryKey) )
            {
                fprintf(stderr, "%s: could not create SINGLETONS entry for \"%s\"\n",
                    options.getProgramName().getStr(), U2S( singletonName ));
            } else
            {
                bRet = sal_True;
                OUString value2 = reader.getSuperTypeName();

                if ( entryKey.setValue(tmpName, RG_VALUETYPE_UNICODE,
                                       (RegValue)value2.getStr(), sizeof(sal_Unicode)* (value2.getLength()+1)) )
                {
                    fprintf(stderr, "%s: could not create data entry for singleton \"%s\"\n",
                            options.getProgramName().getStr(), U2S( singletonName ));
                }

                if ( options.forceOutput() )
                {
                    fprintf(stderr, "%s: create SINGLETON entry for \"%s\" -> \"%s\"\n",
                            options.getProgramName().getStr(), U2S( singletonName ), U2S(value2));
                }
            }
        }

        rtl_freeMemory(value);
    }

       RegistryKeyArray subKeys;

    typeKey.openSubKeys(tmpName, subKeys);

    sal_uInt32 length = subKeys.getLength();
    RegistryKey elementKey;
    for (sal_uInt32 i = 0; i < length; i++)
    {
        elementKey = subKeys.getElement(i);
        if ( checkSingletons(singletonKey, elementKey) )
        {
            bRet = sal_True;
        }
    }
    return bRet;
}

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

    OUString indexRegName( convertToFileUrl(options.getIndexReg()) );
    OUString typeRegName( convertToFileUrl(options.getTypeReg()) );

    Registry indexReg;
    Registry typeReg;

    if ( indexReg.open(indexRegName, REG_READWRITE) )
    {
        if ( indexReg.create(indexRegName) )
        {
            fprintf(stderr, "%s: open registry \"%s\" failed\n",
                    options.getProgramName().getStr(), options.getIndexReg().getStr());
            exit(2);
        }
    }
    if ( typeReg.open(typeRegName, REG_READONLY) )
    {
        fprintf(stderr, "%s: open registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getTypeReg().getStr());
        exit(3);
    }

    RegistryKey indexRoot, typeRoot;
    if ( indexReg.openRootKey(indexRoot) )
    {
        fprintf(stderr, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getIndexReg().getStr());
        exit(4);
    }
    if ( typeReg.openRootKey(typeRoot) )
    {
        fprintf(stderr, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getTypeReg().getStr());
        exit(5);
    }

    RegistryKey singletonKey, typeKey;
    if ( options.hasBase() )
    {
        if ( typeRoot.openKey(S2U(options.getBase()), typeKey) )
        {
            fprintf(stderr, "%s: open base key of registry \"%s\" failed\n",
                    options.getProgramName().getStr(), options.getTypeReg().getStr());
            exit(6);
        }
    } else
    {
        typeKey = typeRoot;
    }

    if ( indexRoot.createKey(OUString::createFromAscii("SINGLETONS"), singletonKey) )
    {
        fprintf(stderr, "%s: open/create SINGLETONS key of registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getIndexReg().getStr());
        exit(7);
    }

    sal_Bool bSingletonsExist = checkSingletons(singletonKey, typeKey);

    indexRoot.closeKey();
    typeRoot.closeKey();
    typeKey.closeKey();
    singletonKey.closeKey();
    if ( indexReg.close() )
    {
        fprintf(stderr, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getIndexReg().getStr());
        exit(9);
    }
    if ( !bSingletonsExist )
    {
        if ( indexReg.destroy(OUString()) )
        {
            fprintf(stderr, "%s: destroy registry \"%s\" failed\n",
                    options.getProgramName().getStr(), options.getIndexReg().getStr());
            exit(10);
        }
    }
    if ( typeReg.close() )
    {
        fprintf(stderr, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().getStr(), options.getTypeReg().getStr());
        exit(11);
    }
}


