/*************************************************************************
 *
 *  $RCSfile: registercomponent.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jbu $ $Date: 2002-05-22 12:54:28 $
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
    fprintf(stderr, "usage: regcomp -register|revoke -r registryfile -c locationUrl [-br registryfile] [-l componentLoaderUrl] [-s]\n");
    fprintf(stderr, " Parameters:\n");
    fprintf(stderr, "      -register              = register a new extern component.\n");
    fprintf(stderr, "      -revoke                = revoke an extern component.\n\n");
    fprintf(stderr, "      -br registryfile       = the name of the registry used for bootstrapping the program.\n"
                    "                               If the bootstrap registry have the same name as the registration registry\n"
                    "                               the -r option is optional.\n");
    fprintf(stderr, "      -r registryfile        = the name of the registry (will be created if not exists).\n");
    fprintf(stderr, "      -c locationUrls        = the location of a component (DLL, Class name, url of a jar file, ...)\n"
                    "                               or a list of urls seperated by ';' or ' '. Note if a list of urls is specified, the\n"
                    "                               components must all need the same loader (quoting is possible with \\ or \"\").\n");
    fprintf(stderr, "      -l componentLoaderUrl  = the name of the needed loader, if no loader is specified\n"
                    "                               the 'com.sun.star.loader.SharedLibrary' is used.\n"
                    "                               loaders: com.sun.star.loader.SharedLibrary | com.sun.star.loader.Java2\n"
                    "      -s                     = silent, no output on success\n" );
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
                            rOptions.sBootRegName = OStringToOUString(av[i], osl_getThreadTextEncoding());
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
                    } else
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
                        strcpy( rargv[rargc] , buffer );
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

    if( ! bLoaderExplicitlyGiven && rOptions.sComponentUrls.getLength() > 4 )
    {
        if ( rOptions.sComponentUrls.matchAsciiL(
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
            fprintf(stderr, "CannotRegisterImplementationException: %s\n", aMessage.getStr());

            ++ (*_exitCode);
        }
        catch( RuntimeException & e )
        {
            OString aMessage(OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US));
            fprintf(stderr, "register component '%s' in registry '%s' failed!\n", sUrl.getStr(), _sRegName.getStr());
            fprintf(stderr, "RuntimeException: %s\n", aMessage.getStr());

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

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
void _cdecl main( int argc, char * argv[] )
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

    Reference< XMultiServiceFactory >   xSMgr;
    Reference< XSimpleRegistry >        xReg;

    try
    {
        if ( aOptions.sBootRegName.getLength() )
        {
            xSMgr = createRegistryServiceFactory( convertToFileUrl(aOptions.sBootRegName) );
        } else
          {
            xSMgr = createServiceFactory();

            // this may be added in future, when the javavm can get its initial settings
            // from the uno context, now it is quite useless
//              if( ! aOptions.sLoaderName.compareToAscii( "com.sun.star.loader.Java2" ) )
//              {
//                  // we know our java loader, so in order to make it a little easier ...
//                  Reference< XInterface > r = loadSharedLibComponentFactory(
//                      OUString::createFromAscii( "jen" ), OUString(),
//                      OUString::createFromAscii( "com.sun.star.comp.stoc.JavaVirtualMachine" ),
//                      xSMgr,
//                      Reference< XRegistryKey > () );
//                  Reference< XInterface > r2 = loadSharedLibComponentFactory(
//                      OUString::createFromAscii( "javaloader" ), OUString(),
//                      OUString::createFromAscii(( "com.sun.star.comp.stoc.JavaComponentLoader" ) ),
//                      xSMgr,
//                      Reference< XRegistryKey > () );
//                  Reference <XSet> xSet( xSMgr, UNO_QUERY );
//                  if( r.is() && r2.is() && xSet.is() )
//                  {
//                      xSet->insert( makeAny( r ) );
//                      xSet->insert( makeAny( r2 ) );
//                  }
//              }
        }
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

    OString     sRegName;
    if ( aOptions.sRegName.getLength() )
    {
        sRegName = OUStringToOString(aOptions.sRegName, osl_getThreadTextEncoding());
    } else
    {
        sRegName = OUStringToOString(aOptions.sBootRegName, osl_getThreadTextEncoding());
    }


    OString tmp = OUStringToOString(aOptions.sComponentUrls, osl_getThreadTextEncoding());

    if ( aOptions.sComponentUrls.getLength() == 0 )
    {
        fprintf(stderr, "ERROR: no component url is specified!\n");
        exit(1);
    }

    if ( !sRegName.equals(OUStringToOString(aOptions.sBootRegName, osl_getThreadTextEncoding())) )
    {
        xReg = Reference< XSimpleRegistry >( xSMgr->createInstance(rtl::OUString::createFromAscii("com.sun.star.registry.SimpleRegistry")), UNO_QUERY);

        if (xReg.is())
        {
            try
            {
                xReg->open( convertToFileUrl(aOptions.sRegName), sal_False, sal_True);
                if (!xReg->isValid())
                {
                    fprintf(stderr, "ERROR: open|create registry '%s' failed!\n", sRegName.getStr());
                    exit(1);
                }
            }
            catch( InvalidRegistryException & e)
            {
                OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
                fprintf(stderr,
                        "ERROR: create registry '%s' failed!\n"
                        "InvalidRegistryException: %s\n",
                         sRegName.getStr(), o.getStr() );
                exit(1);
            }
        }
    }

    Reference<XImplementationRegistration> xImplRegistration(xSMgr->createInstance(OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration"))),
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
            if((raw_urls[index] == semikolon.getStr()[0] || raw_urls[index] == space.getStr()[0]) && !quote && !inString) // a semikolon or space?
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

        if(aOptions.bRegister || aOptions.bRevoke)
            for_each(urls.begin(), urls.end(),
                     DoIt(aOptions.bRegister, aOptions.bRevoke, aOptions.bSilent,
                          xReg, sRegName, xImplRegistration, aOptions.sLoaderName, &exitCode));

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

    exit(exitCode);
}


