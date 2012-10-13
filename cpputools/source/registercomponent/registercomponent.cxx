/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include "sal/main.h"
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/shlib.hxx>

#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/registry/XImplementationRegistration2.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <algorithm>
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
using namespace ::cppu;
using namespace ::std;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using com::sun::star::container::XSet;
using com::sun::star::container::XContentEnumerationAccess;
using com::sun::star::container::XEnumeration;

namespace {

OUString replacePrefix(OUString const & url, OUString const & prefix) {
    sal_Int32 i = url.lastIndexOf('/');
    // Backward compatibility with stoc/source/implementationregistration/
    // implreg.cxx:1.27 l. 1892:
    if (i == -1) {
        i = url.lastIndexOf('\\');
    }
    return prefix + url.copy(i + 1);
}

}

sal_Bool isFileUrl(const OUString& fileName)
{
    if (fileName.indexOf("file://") == 0 )
        return sal_True;
    return sal_False;
}

OUString convertToFileUrl(const OUString& fileName)
{
    if ( isFileUrl(fileName) )
    {
        return fileName;
    }

    OUString uUrlFileName;
    if ( fileName.indexOf('.') == 0 || fileName.indexOf(SEPARATOR) < 0 )
    {
        OUString uWorkingDir;
        if (osl_getProcessWorkingDir(&uWorkingDir.pData) != osl_Process_E_None) {
            OSL_ASSERT(false);
        }
        if (FileBase::getAbsoluteFileURL(uWorkingDir, fileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    } else
    {
        if (FileBase::getFileURLFromSystemPath(fileName, uUrlFileName)
            != FileBase::E_None)
        {
            OSL_ASSERT(false);
        }
    }

    return uUrlFileName;
}
static void usingRegisterImpl()
{
    fprintf(stderr, "usage: regcomp -register|revoke -r registryfile -c locationUrl [-br registryfile] [-l componentLoaderUrl] [-s] [-classpath path]\n");
    fprintf(stderr, " Parameters:\n");
    fprintf(stderr, "  -register\n"
                    "        register a new component.\n");
    fprintf(stderr, "  -revoke\n"
                    "        revoke a component.\n");
    fprintf(stderr, "  -br registryfile\n"
                    "        the name of the registry used for bootstrapping\n"
                    "        regcomp. The option can be given twice, each\n"
                    "        one followed by exactly one registry file.\n"
                    "        The registries are used to access both types and\n"
                    "        registered components.\n");
    fprintf(stderr, "  -r registryfile\n"
                    "        the name of the target registry (will be created\n"
                    "        if it does not exists). The file name may match\n"
                    "        with one of the filenames given with the -br option.\n");
    fprintf(stderr, "  -c locationUrls\n"
                    "        the location of a component (a url to a shared\n"
                    "        library or a absolute url to a .jar file) or a\n"
                    "        list of urls seperated by ';' or ' '. Note if a\n"
                    "        list of urls is specified, the components must\n"
                    "        all need the same loader (quoting is possible with\n"
                    "        \\ or \"\").\n");
    fprintf(stderr, "  -l componentLoaderUrl\n"
                    "        the name of the needed loader. If no loader is\n"
                    "        specified and the components have a .jar suffix,\n"
                    "        the default is com.sun.star.loader.Java2.\n"
                    "        Otherwise, the default is\n"
                    "        com.sun.star.loader.SharedLibrary\n"
                    "  -s\n"
                    "        silent, regcomp prints messages only on error.\n"
                    "  -wop\n"
                    "        register the component name only without path\n"
                    "  -wop=prefix\n"
                    "        register the component name with path replaced\n"
                    "        by given prefix\n"
                    "  -classpath path\n"
                    "        sets the java classpath to path (overwriting the\n"
                    "        current classpath environment variable). Note that\n"
                    "        in case you start regcomp e.g. within an office\n"
                    "        environment, the classpath entries in the\n"
                    "        configuration still have precedence over this\n"
                    "        option.\n");
}

class IllegalArgument
{
public:
    IllegalArgument(const OString& rMessage)
        : m_message(rMessage)
        {}

    OString m_message;
};

struct Options
{
    Options()
        : bRegister(sal_False)
        , bRevoke(sal_False)
        , bSilent( sal_False )
        , bPrefix( sal_False )
        {}

    sal_Bool bRegister;
    sal_Bool bRevoke;
    sal_Bool bSilent;
    sal_Bool bPrefix;
    OUString sPrefix;
    OUString sProgramName;
    OUString sBootRegName;
    OUString sBootRegName2;
    OUString sRegName;
    OUString sComponentUrls;
    OUString sLoaderName;
};

sal_Bool parseOptions(int ac, char* av[], Options& rOptions, sal_Bool bCmdFile)
    throw( IllegalArgument )
{
    sal_Bool    ret = sal_True;
    sal_uInt16  i=0;
    sal_Bool bLoaderExplicitlyGiven = sal_False;

    rOptions.sProgramName = OUString::createFromAscii(av[i++]);

    if (!bCmdFile)
    {
        bCmdFile = sal_True;

        if (ac < 2)
        {
            usingRegisterImpl();
            ret = sal_False;
        }
    }

    for (; i < ac; i++)
    {
        if (av[i][0] == '-')
        {
            switch (av[i][1])
            {
                case 'r':
                    if (strcmp(av[i], "-register") == 0)
                    {
                        rOptions.bRegister = sal_True;
                    } else
                    if (strcmp(av[i], "-revoke") == 0)
                    {
                        rOptions.bRevoke = sal_True;
                    } else
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            rOptions.sRegName = OStringToOUString(av[i], osl_getThreadTextEncoding());
                        } else
                        {
                            OString tmp("'-r', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i+1]) + "'";
                            }
                            throw IllegalArgument(tmp);
                        }
                    } else
                    {
                        rOptions.sRegName = OStringToOUString(av[i]+2, osl_getThreadTextEncoding());
                    }
                    break;
                case 'b':
                    if (av[i][2] != 'r')
                    {
                        OString tmp("'-b', invalid option!");
                        throw IllegalArgument(tmp);
                    }
                    if (av[i][3] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            OUString regName = OStringToOUString(av[i], osl_getThreadTextEncoding());
                            if( rOptions.sBootRegName.isEmpty() )
                            {
                                rOptions.sBootRegName = regName;
                            }
                            else
                            {
                                rOptions.sBootRegName2 = regName;
                            }
                        } else
                        {
                            OString tmp("'-br', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i+1]) + "'";
                            }
                            throw IllegalArgument(tmp);
                        }
                    } else
                    {
                        rOptions.sBootRegName = OStringToOUString(av[i]+3, osl_getThreadTextEncoding());
                    }
                    break;
                case 'c':
                {
                    OUString sUrls;
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            sUrls = OStringToOUString(av[i], osl_getThreadTextEncoding());
                        } else
                        {
                            OString tmp("'-c', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i+1]) + "'";
                            }
                            throw IllegalArgument(tmp);
                        }
                    }
                    else if( 0 == strncmp( av[i] , "-classpath" ,10 ) )
                    {
                        i++;
                        if( i < ac )
                        {
                            rtl::OUString envVar(RTL_CONSTASCII_USTRINGPARAM("CLASSPATH"));
                            rtl::OUString envValue(av[i], strlen(av[i]), osl_getThreadTextEncoding());
                            osl_setEnvironment(envVar.pData, envValue.pData);
                        }
                        break;
                    }
                    else
                    {
                        sUrls = OStringToOUString(av[i]+2, osl_getThreadTextEncoding());
                    }

                    if (!rOptions.sComponentUrls.isEmpty())
                    {
                        OUString tmp(rOptions.sComponentUrls + OUString(";", 1, osl_getThreadTextEncoding()) + sUrls);
                        rOptions.sComponentUrls = tmp;
                    } else
                    {
                        rOptions.sComponentUrls = sUrls;
                    }
                    break;
                }
                case 'l':
                {
                    if (av[i][2] == '\0')
                    {
                        if (i < ac - 1 && av[i+1][0] != '-')
                        {
                            i++;
                            rOptions.sLoaderName = OUString::createFromAscii(av[i]);
                            bLoaderExplicitlyGiven = sal_True;
                        } else
                        {
                            OString tmp("'-l', please check");
                            if (i <= ac - 1)
                            {
                                tmp += " your input '" + OString(av[i+1]) + "'";
                            }
                            throw IllegalArgument(tmp);
                        }
                    } else
                    {
                        bLoaderExplicitlyGiven = sal_True;
                        rOptions.sLoaderName = OUString::createFromAscii(av[i]+2);
                    }
                    break;
                }
                case 's':
                {
                    if( av[i][2] == 0 )
                    {
                        rOptions.bSilent = sal_True;
                    }
                    else
                    {
                        rtl::OStringBuffer buf;
                        buf.append( "Unknown error " );
                        buf.append( av[i] );
                        throw IllegalArgument( av[i] );
                    }
                    break;
                }
                case 'e':
                {
                    if( av[i][2] == 'n' && av[i][3] == 'v' && av[i][4] == ':' )
                    {
                        // bootstrap variable, ignore it
                        break;
                    }
                }
                case 'w':
                {
                    if (strcmp(av[i], "-wop") == 0)
                    {
                        rOptions.bPrefix = sal_True;
                        rOptions.sPrefix = OUString();
                            // in case there are multiple -wops
                        break;
                    }
                    else if (
                        strncmp(av[i], "-wop=", RTL_CONSTASCII_LENGTH("-wop="))
                        == 0)
                    {
                        rOptions.bPrefix = sal_True;
                        rOptions.sPrefix = OStringToOUString(
                            av[i] + RTL_CONSTASCII_LENGTH("-wop="),
                            osl_getThreadTextEncoding());
                        break;
                    }
                }
                default:
                {
                    OString tmp( "unknown option " );
                    tmp += av[i];
                    throw IllegalArgument( tmp );
                }
            }
        } else
        {
            if (av[i][0] == '@')
            {
                FILE* cmdFile = fopen(av[i]+1, "r");
                  if( cmdFile == NULL )
                  {
                    usingRegisterImpl();
                    ret = sal_False;
                } else
                {
                    fseek( cmdFile , 0 , SEEK_END );
                    sal_Int32 nLen = ftell( cmdFile);
                    fseek( cmdFile, 0, SEEK_SET );

                    // 2 chars per string is a upper limit for the number of
                    // substrings ( at least one separator char needed for fscanf).
                    char ** rargv = (char **)rtl_allocateMemory( nLen * sizeof( char* ) /2);
                    if( ! rargv )
                    {
                        OStringBuffer buf;
                        buf.append( "Not enough memory for reading command file " );
                        buf.append( av[i] +1 );
                        buf.append( " with length " );
                        buf.append( nLen );
                        buf.append( "." );
                        throw IllegalArgument( buf.makeStringAndClear() );
                    }
                    char *buffer = ( char * )rtl_allocateMemory( nLen +1 );
                    if( ! buffer )
                    {
                        OStringBuffer buf;
                        buf.append( "Not enough memory for reading command file " );
                        buf.append( av[i] +1 );
                        buf.append( " with length " );
                        buf.append( nLen );
                        buf.append( "." );
                        throw IllegalArgument( buf.makeStringAndClear() );
                    }

                    // we start at one to omit argv[0]
                    sal_Int32 rargc = 1;
                    rargv[0] = av[0];
                    while ( fscanf(cmdFile, "%s", buffer) != EOF )
                    {
                        rargv[rargc]= (char * )rtl_allocateMemory( strlen( buffer ) +1 );
                        if( ! rargv[rargc] )
                        {
                            OStringBuffer buf;
                            buf.append( "Not enough memory for reading command file " );
                            buf.append( av[i] +1 );
                            buf.append( " with length " );
                            buf.append( nLen );
                            buf.append( "." );
                            throw IllegalArgument( buf.makeStringAndClear() );
                        }
                        strcpy( rargv[rargc] , buffer ); // #100211# - checked
                        rargc++;
                    }
                    fclose(cmdFile);

                    parseOptions(rargc, rargv, rOptions, bCmdFile);

                    for (long j=1; j < rargc; j++)
                    {
                        rtl_freeMemory(rargv[j]);
                    }
                    rtl_freeMemory( buffer );
                    rtl_freeMemory( rargv );
                }
            } else
            {
                usingRegisterImpl();
                ret = sal_False;
            }
        }
    }

    if( ! bLoaderExplicitlyGiven )
    {
        if ( rOptions.sComponentUrls.getLength() > 4 &&
             rOptions.sComponentUrls.matchAsciiL(
                 ".jar" , 4 , rOptions.sComponentUrls.getLength() - 4 ) )
        {
            if( ! rOptions.bSilent )
            {
                printf( "using loader com.sun.star.loader.Java2\n" );
            }
            rOptions.sLoaderName = OUString(
                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.Java2"));
        }
        else
        {
            rOptions.sLoaderName = OUString(
                RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary") );
        }
    }

    return ret;
}


struct DoIt
{
    sal_Bool                                _bRegister;
    sal_Bool                                _bRevoke;
    sal_Bool                                _bSilent;
    sal_Bool                                _bPrefix;
    OUString                                _sPrefix;
    OString                                 _sRegName;
    OUString                                _sLoaderName;
    Reference<XImplementationRegistration2> _xImplRegistration;
    Reference<XSimpleRegistry>              _xReg;
    sal_uInt32                            * _exitCode;

    DoIt(sal_Bool bRegister,
         sal_Bool bRevoke,
         sal_Bool bSilent,
         sal_Bool bPrefix,
         const OUString & sPrefix,
         const Reference<XSimpleRegistry> & xReg,
         const OString & sRegName,
         const Reference<XImplementationRegistration2> & xImplRegistration,
         const OUString & sLoaderName,
         sal_uInt32 * exitCode)
        throw();

    void operator()(const OUString & url) throw();
};

DoIt::DoIt(sal_Bool bRegister,
           sal_Bool bRevoke,
           sal_Bool bSilent,
           sal_Bool bPrefix,
           const OUString & sPrefix,
           const Reference<XSimpleRegistry> & xReg,
           const OString & sRegName,
           const Reference<XImplementationRegistration2> & xImplRegistration,
           const OUString & sLoaderName,
           sal_uInt32 * exitCode) throw()
    : _bRegister(bRegister),
      _bRevoke(bRevoke),
      _bSilent( bSilent ),
      _bPrefix( bPrefix ),
      _sPrefix( sPrefix ),
      _sRegName(sRegName),
      _sLoaderName(sLoaderName),
      _xImplRegistration(xImplRegistration),
      _xReg(xReg),
      _exitCode(exitCode)
{}

void DoIt::operator() (const OUString & url) throw()
{
    OString sUrl = OUStringToOString(url, osl_getThreadTextEncoding());

    if (_bRegister)
    {
        try
        {
            Reference<XImplementationRegistration2> _xImplRegistration2(_xImplRegistration, UNO_QUERY);
            if ( _bPrefix ) {
                _xImplRegistration->registerImplementationWithLocation(
                    _sLoaderName, url, replacePrefix(url, _sPrefix), _xReg);
            } else {
                _xImplRegistration->registerImplementation(_sLoaderName, url, _xReg);
            }

            if ( ! _bSilent )
            {
                fprintf(stderr, "register component '%s' in registry '%s' successful!\n", sUrl.getStr(), _sRegName.getStr());
            }

        }
        catch(CannotRegisterImplementationException & cannotRegisterImplementationException) {
            OString aMessage(OUStringToOString(cannotRegisterImplementationException.Message, RTL_TEXTENCODING_ASCII_US));
            fprintf(stderr, "register component '%s' in registry '%s' failed!\n", sUrl.getStr(), _sRegName.getStr());
            fprintf(stderr, "error (CannotRegisterImplementationException): %s\n", aMessage.getStr());

            ++ (*_exitCode);
        }
        catch( RuntimeException & e )
        {
            OString aMessage(OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US));
            fprintf(stderr, "register component '%s' in registry '%s' failed!\n", sUrl.getStr(), _sRegName.getStr());
            fprintf(stderr, "error (RuntimeException): %s\n", aMessage.getStr());

            ++ (*_exitCode);
        }
    }
    else if(_bRevoke)
    {
        try
        {
            sal_Bool bRet = _xImplRegistration->revokeImplementation(url, _xReg);

            if (bRet)
            {
                if ( ! _bSilent )
                    fprintf(stderr, "revoke component '%s' from registry '%s' successful!\n", sUrl.getStr(), _sRegName.getStr());
            }
            else
            {
                fprintf(stderr, "revoke component '%s' from registry '%s' failed!\n", sUrl.getStr(), _sRegName.getStr());
                ++ (*_exitCode);
            }
        }
        catch( RuntimeException & e )
        {
            OString aMessage(OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US));
            fprintf( stderr,
                     "revoke component '%s' from registry '%s' failed!\n",
                     sUrl.getStr(),
                     _sRegName.getStr() );
            fprintf( stderr, "RuntimeException: %s\n" , aMessage.getStr());
            ++ (*_exitCode);
        }
    }
}

static bool hasService(
    const Reference< XMultiServiceFactory > &xSMgr,
    const sal_Char * service )
{
    bool ret = false;

    Reference< XContentEnumerationAccess > access( xSMgr, UNO_QUERY );
    if( access.is( ))
    {
        Reference< XEnumeration > enumeration = access->createContentEnumeration(
            OUString::createFromAscii( service ) );

        if( enumeration.is() && enumeration->hasMoreElements() )
        {
            ret = true;
        }
    }
    return ret;
}

static void bootstrap(
    Options & opt ,
    Reference< XMultiServiceFactory > &xSMgr,
    Reference< XSimpleRegistry > & reg ) throw ( Exception )
{
    if( opt.sRegName.equals( opt.sBootRegName2 ) )
    {
        OUString tmp2 = opt.sBootRegName;
        opt.sBootRegName = opt.sBootRegName2;
        opt.sBootRegName2 = tmp2;
    }

    if ( opt.sRegName.equals(opt.sBootRegName) )
    {
        if( !opt.sBootRegName2.isEmpty() )
        {
            xSMgr = createRegistryServiceFactory(
                convertToFileUrl(opt.sRegName),
                convertToFileUrl(opt.sBootRegName2),
                sal_False );
        }
        else
        {
            xSMgr = createRegistryServiceFactory(
                convertToFileUrl(opt.sRegName) , sal_False );
        }
    }
    else
    {
        if( !opt.sBootRegName2.isEmpty() )
        {
            xSMgr = createRegistryServiceFactory(
                convertToFileUrl( opt.sBootRegName2 ),
                convertToFileUrl( opt.sBootRegName ),
                sal_True );
        }
        else if ( !opt.sBootRegName.isEmpty() )
        {
            xSMgr = createRegistryServiceFactory(
                convertToFileUrl( opt.sBootRegName ),
                sal_True );
        }
        else
        {
            xSMgr = createServiceFactory();
        }
        reg = Reference< XSimpleRegistry >(
            xSMgr->createInstance(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.SimpleRegistry"))), UNO_QUERY);

        if (reg.is())
        {
            try
            {
                reg->open( convertToFileUrl(opt.sRegName), sal_False, sal_True);
                if (!reg->isValid())
                {
                    fprintf(stderr, "ERROR: open|create registry '%s' failed!\n",
                            OUStringToOString(opt.sRegName, osl_getThreadTextEncoding() ).getStr());
                    exit(1);
                }
            }
            catch( InvalidRegistryException & e)
            {
                OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
                fprintf(stderr,
                        "ERROR: create registry '%s' failed!\n"
                        "InvalidRegistryException: %s\n",
                         OUStringToOString( opt.sRegName, osl_getThreadTextEncoding()).getStr(),
                        o.getStr() );
                exit(1);
            }
        }
    }

    if( ! opt.sLoaderName.compareToAscii( "com.sun.star.loader.Java2" ) &&
        ! hasService( xSMgr, "com.sun.star.loader.Java2" ) )
    {
        // we know our java loader, so we check, whether a java-loader is
        // registered
        Reference< XInterface > r = loadSharedLibComponentFactory(
            OUString(RTL_CONSTASCII_USTRINGPARAM("javavm.uno" SAL_DLLEXTENSION)),
            OUString(),
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.JavaVirtualMachine")),
            xSMgr,
            Reference< XRegistryKey > (),
            "javavm" );
        Reference< XInterface > r2 = loadSharedLibComponentFactory(
            OUString(RTL_CONSTASCII_USTRINGPARAM("javaloader.uno" SAL_DLLEXTENSION)),
            OUString(),
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.JavaComponentLoader")),
            xSMgr,
            Reference< XRegistryKey > (),
            "javaloader" );
        Reference <XSet> xSet( xSMgr, UNO_QUERY );
        if( r.is() && r2.is() && xSet.is() )
        {
            xSet->insert( makeAny( r ) );
            xSet->insert( makeAny( r2 ) );
        }
    }
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    sal_Bool    bRet = sal_False;
    sal_uInt32  exitCode = 0;
    Options     aOptions;

    try
    {
        if ( !parseOptions(argc, argv, aOptions, sal_False) )
        {
            exit(1);
        }
    }
    catch ( IllegalArgument& e)
    {
        fprintf(stderr, "ERROR: %s\n", e.m_message.getStr());
        exit(1);
    }

    if( aOptions.sRegName.isEmpty() )
    {
        fprintf( stderr, "ERROR: target registry missing (-r option)\n" );
        exit( 1 );
    }
    if ( aOptions.sComponentUrls.isEmpty() )
    {
        fprintf(stderr, "ERROR: no component url is specified!\n");
        exit(1);
    }

    Reference< XMultiServiceFactory >   xSMgr;
    Reference< XSimpleRegistry >        xReg;
    try
    {
        bootstrap( aOptions, xSMgr ,xReg );
    }
    catch( Exception& e )
    {
        fprintf(stderr, "ERROR: create ServiceManager failed!\n");
        if ( !e.Message.isEmpty() )
        {
            fprintf(stderr, "ERROR description: %s\n",
                    OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
        }
        exit(1);
    }

    Reference<XImplementationRegistration2> xImplRegistration(
        xSMgr->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                         "com.sun.star.registry.ImplementationRegistration"))),
        UNO_QUERY);

    if (xImplRegistration.is())
    {
        sal_Int32 index = 0;
        vector<OUString> urls;

        OUString urlListWithSemikolon = aOptions.sComponentUrls;
        do {
            OUString aToken = urlListWithSemikolon.getToken( 0, ';', index);
            fprintf(stderr, "%s\n", OUStringToOString(aToken, osl_getThreadTextEncoding()).getStr());
            urls.push_back(aToken);
        } while ( index >= 0 );


        OString sRegName = OUStringToOString( aOptions.sRegName, osl_getThreadTextEncoding() );
        if(aOptions.bRegister || aOptions.bRevoke)
        {
            for_each(urls.begin(), urls.end(),
                     DoIt(aOptions.bRegister, aOptions.bRevoke, aOptions.bSilent,
                          aOptions.bPrefix, aOptions.sPrefix,
                          xReg, sRegName, xImplRegistration,
                          aOptions.sLoaderName, &exitCode));
        }
        else
        {
            ++ exitCode;
             usingRegisterImpl();
        }
    }
    else
    {
        fprintf(stderr, "Component registration service could not be loaded!\n");
        exitCode++;
    }

    if (!bRet && xReg.is() && xReg->isValid())
        xReg->close();

    Reference< XComponent > xComponent( xSMgr, UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    return exitCode;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
