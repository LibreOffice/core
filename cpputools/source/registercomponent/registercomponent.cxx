/*************************************************************************
 *
 *  $RCSfile: registercomponent.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-23 16:18:34 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <vector>
#
#include <rtl/strbuf.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/shlib.hxx>

#include <com/sun/star/container/XSet.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <algorithm>

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

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

#ifdef SAL_W32
#define putenv _putenv
#endif

sal_Bool isFileUrl(const OUString& fileName)
{
    if (fileName.indexOf(OUString::createFromAscii("file://")) == 0 )
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
        OSL_VERIFY( osl_getProcessWorkingDir(&uWorkingDir.pData) == osl_Process_E_None );
        OSL_VERIFY( FileBase::getAbsoluteFileURL(uWorkingDir, fileName, uUrlFileName) == FileBase::E_None );
    } else
    {
        OSL_VERIFY( FileBase::getFileURLFromSystemPath(fileName, uUrlFileName) == FileBase::E_None );
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
        {}

    sal_Bool bRegister;
    sal_Bool bRevoke;
    sal_Bool bSilent;
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

    char    *s=NULL;
    for (i; i < ac; i++)
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
                            if( ! rOptions.sBootRegName.getLength() )
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
                            // leak this string as some platforms assume to own
                            // the pointer
                            sal_Char * p = (sal_Char *) rtl_allocateMemory( 13+ strlen( av[i] ) );
                            p[0] = 0;
                            strcat( p, "CLASSPATH=" ); // #100211# - checked
                            strcat( p, av[i] );        // #100211# - checked

                            putenv( p );
                        }
                        break;
                    }
                    else
                    {
                        sUrls = OStringToOUString(av[i]+2, osl_getThreadTextEncoding());
                    }

                    if (rOptions.sComponentUrls.getLength())
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

                    for (long i=1; i < rargc; i++)
                    {
                        rtl_freeMemory(rargv[i]);
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
    sal_Bool                               _bRegister;
    sal_Bool                               _bRevoke;
    sal_Bool                               _bSilent;
    OString                                _sRegName;
    OUString                               _sLoaderName;
    Reference<XImplementationRegistration> _xImplRegistration;
    Reference<XSimpleRegistry>             _xReg;
    sal_uInt32                           * _exitCode;

    DoIt(sal_Bool bRegister,
         sal_Bool bRevoke,
         sal_Bool bSilent,
         const Reference<XSimpleRegistry> & xReg,
         const OString & sRegName,
         const Reference<XImplementationRegistration> & xImplRegistration,
         const OUString & sLoaderName,
         sal_uInt32 * exitCode)
        throw();

    void operator()(const OUString & url) throw();
};

DoIt::DoIt(sal_Bool bRegister,
           sal_Bool bRevoke,
           sal_Bool bSilent,
           const Reference<XSimpleRegistry> & xReg,
           const OString & sRegName,
           const Reference<XImplementationRegistration> & xImplRegistration,
           const OUString & sLoaderName,
           sal_uInt32 * exitCode) throw()
    : _bRegister(bRegister),
      _bRevoke(bRevoke),
      _bSilent( bSilent ),
      _xReg(xReg),
      _sRegName(sRegName),
      _xImplRegistration(xImplRegistration),
      _sLoaderName(sLoaderName),
      _exitCode(exitCode)
{}

void DoIt::operator() (const OUString & url) throw()
{
    OString sUrl = OUStringToOString(url, osl_getThreadTextEncoding());

    if (_bRegister)
    {
        try
        {
            _xImplRegistration->registerImplementation(_sLoaderName, url, _xReg);

            if ( ! _bSilent )
            {
                fprintf(stderr, "register component '%s' in registry '%s' succesful!\n", sUrl.getStr(), _sRegName.getStr());
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
                    fprintf(stderr, "revoke component '%s' from registry '%s' succesful!\n", sUrl.getStr(), _sRegName.getStr());
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
        if( opt.sBootRegName2.getLength() )
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
        if( opt.sBootRegName2.getLength() )
        {
            xSMgr = createRegistryServiceFactory(
                convertToFileUrl( opt.sBootRegName2 ),
                convertToFileUrl( opt.sBootRegName ),
                sal_True );
        }
        else if ( opt.sBootRegName.getLength() )
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
                rtl::OUString::createFromAscii("com.sun.star.registry.SimpleRegistry")), UNO_QUERY);

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
            OUString::createFromAscii( "javavm.uno" SAL_DLLEXTENSION ),
            OUString(),
            OUString::createFromAscii( "com.sun.star.comp.stoc.JavaVirtualMachine" ),
            xSMgr,
            Reference< XRegistryKey > () );
        Reference< XInterface > r2 = loadSharedLibComponentFactory(
            OUString::createFromAscii( "javaloader.uno" SAL_DLLEXTENSION ),
            OUString(),
            OUString::createFromAscii(( "com.sun.star.comp.stoc.JavaComponentLoader" ) ),
            xSMgr,
            Reference< XRegistryKey > () );
        Reference <XSet> xSet( xSMgr, UNO_QUERY );
        if( r.is() && r2.is() && xSet.is() )
        {
            xSet->insert( makeAny( r ) );
            xSet->insert( makeAny( r2 ) );
        }
    }
}


#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
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

    if( ! aOptions.sRegName.getLength() )
    {
        fprintf( stderr, "ERROR: target registry missing (-r option)\n" );
        exit( 1 );
    }
    if ( aOptions.sComponentUrls.getLength() == 0 )
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
        if ( e.Message.getLength() )
        {
            fprintf(stderr, "ERROR description: %s\n",
                    OUStringToOString(e.Message, osl_getThreadTextEncoding()).getStr());
        }
        exit(1);
    }

    Reference<XImplementationRegistration> xImplRegistration(
        xSMgr->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                         "com.sun.star.registry.ImplementationRegistration"))),
        UNO_QUERY);

    if (xImplRegistration.is())
    {
        const OUString bSlash(OUString(RTL_CONSTASCII_USTRINGPARAM("\\")));
        const OUString tuedle(OUString(RTL_CONSTASCII_USTRINGPARAM("\"")));
        const OUString semikolon(OUString(RTL_CONSTASCII_USTRINGPARAM(";")));
        const OUString emptyString(OUString(RTL_CONSTASCII_USTRINGPARAM("")));
        const OUString space(OUString(RTL_CONSTASCII_USTRINGPARAM(" ")));

        sal_Int32 index = 0;
        sal_Bool  quote = sal_False;
        sal_Bool  inString = sal_False;

        const sal_Unicode * raw_urls = aOptions.sComponentUrls.getStr();

        OUString tmp_url;

        vector<OUString> urls;

        // go over the string and parse it, chars can be quoted in strings or with back slash
        while(index < aOptions.sComponentUrls.getLength())
        {
            if((raw_urls[index] == semikolon.getStr()[0] ||
                raw_urls[index] == space.getStr()[0]) && !quote && !inString) // a semikolon or space?
            {
                tmp_url = tmp_url.trim();
                if(tmp_url.getLength())
                    urls.push_back(tmp_url);

                tmp_url = emptyString;
            }
            else if(raw_urls[index] == bSlash.getStr()[0] && !quote) // a back slash?
            {
                quote = sal_True;
            }
            else if(raw_urls[index] == tuedle.getStr()[0] && !quote) // begin or end of string?
                inString = !inString;

            else // no special handling
            {
                tmp_url += OUString(raw_urls + index, 1);
                quote = sal_False;
            }

            ++ index;
        }

        tmp_url = tmp_url.trim();
        if(tmp_url.getLength())
            urls.push_back(tmp_url);

        OString sRegName = OUStringToOString( aOptions.sRegName, osl_getThreadTextEncoding() );
        if(aOptions.bRegister || aOptions.bRevoke)
        {
            for_each(urls.begin(), urls.end(),
                     DoIt(aOptions.bRegister, aOptions.bRevoke, aOptions.bSilent,
                          xReg, sRegName, xImplRegistration, aOptions.sLoaderName, &exitCode));
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


