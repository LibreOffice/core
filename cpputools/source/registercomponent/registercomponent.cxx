/*************************************************************************
 *
 *  $RCSfile: registercomponent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2000-11-13 15:39:15 $
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

#include <stdio.h>
#include <string.h>

#include <vector>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <algorithm>
static void usingRegisterImpl()
{
    fprintf(stderr, "\nusing: regcomp -register|revoke -r registryfile -c locationUrl [-br registryfile] [-l componentLoaderUrl]\n");
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
                    "                               loaders: com.sun.star.loader.SharedLibrary | com.sun.star.loader.Java2\n\n");
}


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;

using namespace cppu;
using namespace rtl;
using namespace std;

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
        , sLoaderName( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary") )
        {}

    sal_Bool bRegister;
    sal_Bool bRevoke;
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
                            rOptions.sRegName = OUString::createFromAscii(av[i]);
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
                        rOptions.sRegName = OUString::createFromAscii(av[i]+2);
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
                            rOptions.sBootRegName = OUString::createFromAscii(av[i]);
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
                        rOptions.sBootRegName = OUString::createFromAscii(av[i]+3);
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
                            sUrls = OUString::createFromAscii(av[i]);
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
                        sUrls = OUString::createFromAscii(av[i]+2);
                    }

                    if (rOptions.sComponentUrls.getLength())
                    {
                        OUString tmp(rOptions.sComponentUrls + OUString(RTL_CONSTASCII_USTRINGPARAM(";")) + sUrls);
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
                        rOptions.sLoaderName = OUString::createFromAscii(av[i]+2);
                    }
                    break;
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
                    int rargc=0;
                    char* rargv[512];
                    char  buffer[512];

                    while ( fscanf(cmdFile, "%s", buffer) != EOF )
                    {
                        rargv[rargc]= strdup(buffer);
                        rargc++;
                    }
                    fclose(cmdFile);

                    parseOptions(rargc, rargv, rOptions, bCmdFile);

                    for (long i=0; i < rargc; i++)
                    {
                        free(rargv[i]);
                    }
                }
            } else
            {
                usingRegisterImpl();
                ret = sal_False;
            }
        }
    }

    return ret;
}


struct DoIt
{
    sal_Bool                               _bRegister;
    sal_Bool                               _bRevoke;
    OString                                _sRegName;
    OUString                               _sLoaderName;
    Reference<XImplementationRegistration> _xImplRegistration;
    Reference<XSimpleRegistry>             _xReg;
    sal_uInt32                           * _exitCode;

    DoIt(sal_Bool bRegister,
         sal_Bool bRevoke,
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
           const Reference<XSimpleRegistry> & xReg,
           const OString & sRegName,
           const Reference<XImplementationRegistration> & xImplRegistration,
           const OUString & sLoaderName,
           sal_uInt32 * exitCode) throw()
    : _bRegister(bRegister),
      _bRevoke(bRevoke),
      _xReg(xReg),
      _sRegName(sRegName),
      _xImplRegistration(xImplRegistration),
      _sLoaderName(sLoaderName),
      _exitCode(exitCode)
{}

void DoIt::operator() (const OUString & url) throw()
{
    OString sUrl = OUStringToOString(url, RTL_TEXTENCODING_ASCII_US);

    if (_bRegister)
    {
        OSL_TRACE("regcomp - registering: %s in %s", sUrl.getStr(), _sRegName.getStr());
        try
        {
            _xImplRegistration->registerImplementation(_sLoaderName, url, _xReg);

            fprintf(stderr, "\nregister component \"%s\" in registry \"%s\" succesful!\n", sUrl.getStr(), _sRegName.getStr());
        }
        catch(CannotRegisterImplementationException & cannotRegisterImplementationException) {
            OString aMessage(OUStringToOString(cannotRegisterImplementationException.Message, RTL_TEXTENCODING_ASCII_US));
            fprintf(stderr, "\nregister component \"%s\" in registry \"%s\" failed!\n", sUrl.getStr(), _sRegName.getStr());
            fprintf(stderr, "\nERROR: %s\n", aMessage.getStr());

            ++ (*_exitCode);
        }
    }
    else if(_bRevoke)
    {
        OSL_TRACE("regcomp - revoking: %s from %s", sUrl.getStr(), _sRegName.getStr());

        try
        {
            sal_Bool bRet = _xImplRegistration->revokeImplementation(url, _xReg);

            if (bRet)
                fprintf(stderr, "\nrevoke component \"%s\" from registry \"%s\" succesful!\n", sUrl.getStr(), _sRegName.getStr());
            else
            {
                fprintf(stderr, "\nrevoke component \"%s\" from registry \"%s\" failed!\n", sUrl.getStr(), _sRegName.getStr());

                  ++ (*_exitCode);
            }
        }
        catch( CannotRegisterImplementationException& e )
        {
            OString aMessage( OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US) );
            fprintf(stderr, "\nrevoke component \"%s\" from registry \"%s\" failed!\n", sUrl.getStr(), _sRegName.getStr());
            fprintf(stderr, "\nERROR: %s\n", aMessage.getStr() );

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
            xSMgr = createRegistryServiceFactory( aOptions.sBootRegName );
        } else
          {
            xSMgr = createServiceFactory();
        }
    }
    catch( Exception& e )
    {
        fprintf(stderr, "ERROR: create ServiceManager failed!\n");
        if ( e.Message.getLength() )
        {
            fprintf(stderr, "ERROR description: %s\n", OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US).getStr());
        }
        exit(1);
    }

    OString     sRegName;
    if ( aOptions.sRegName.getLength() )
    {
        sRegName = OUStringToOString(aOptions.sRegName, RTL_TEXTENCODING_ASCII_US);
    } else
    {
        sRegName = OUStringToOString(aOptions.sBootRegName, RTL_TEXTENCODING_ASCII_US);
    }


    OString tmp = OUStringToOString(aOptions.sComponentUrls, RTL_TEXTENCODING_ASCII_US);
    OSL_TRACE("regcomp - aOptions.sComponentUrls: %s", tmp.getStr());


    if ( aOptions.sComponentUrls.getLength() == 0 )
    {
        fprintf(stderr, "ERROR: no component url is specified!\n");
        exit(1);
    }

    if ( !sRegName.equals(OUStringToOString(aOptions.sBootRegName, RTL_TEXTENCODING_ASCII_US)) )
    {
        xReg = Reference< XSimpleRegistry >( xSMgr->createInstance(rtl::OUString::createFromAscii("com.sun.star.registry.SimpleRegistry")), UNO_QUERY);

        if (xReg.is())
        {
            try
            {
                xReg->open( aOptions.sRegName, sal_False, sal_True);
                if (!xReg->isValid())
                {
                    fprintf(stderr, "ERROR: open|create registry \"%s\" failed!\n", sRegName.getStr());
                    exit(1);
                }
            }
            catch( InvalidRegistryException&)
            {
                fprintf(stderr, "ERROR: create registry \"%s\" failed!\n", sRegName.getStr());
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
            for_each(urls.begin(), urls.end(), DoIt(aOptions.bRegister, aOptions.bRevoke, xReg, sRegName, xImplRegistration, aOptions.sLoaderName, &exitCode));

        else
        {
            ++ exitCode;
             usingRegisterImpl();
        }
    }
    else
    {
        fprintf(stderr, "\nComponent registration service could not be loaded!\n");
        exitCode++;
    }

    if (!bRet && xReg.is() && xReg->isValid())
        xReg->close();

    Reference< XComponent > xComponent( xSMgr, UNO_QUERY );
    if ( xComponent.is() )
        xComponent->dispose();

    exit(exitCode);
}


