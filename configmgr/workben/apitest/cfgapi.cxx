/*************************************************************************
 *
 *  $RCSfile: cfgapi.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:20:17 $
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
#define _PRIVATE_TEST_

#include <iostream>
using namespace std;

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif


#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _OSL_PROFILE_HXX_
#include <osl/profile.hxx>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _OSL_FILE_H_
#include <osl/file.h>
#endif

#include "createpropertyvalue.hxx"

#include "typeconverter.hxx"

// #include <com/sun/star/configuration/XConfigurationSync.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
//using namespace ::com::sun::star::util;
using namespace ::com::sun::star::util;

using ::rtl::OUString;
using ::rtl::OString;
//using namespace ::configmgr;

using namespace ::cppu;

#define ASCII(x) ::rtl::OUString::createFromAscii(x)

ostream& operator << (ostream& out, rtl::OUString const& aStr)
{
    sal_Unicode const* const pStr = aStr.getStr();
    sal_Unicode const* const pEnd = pStr + aStr.getLength();
    for (sal_Unicode const* p = pStr; p < pEnd; ++p)
        if (0 < *p && *p < 127) // ASCII
            out << char(*p);
        else
            out << "[\\u" << hex << *p << "]";
    return out;
}

void showSequence(const Sequence<OUString> &aSeq)
{
    OUString aArray;
    const OUString *pStr = aSeq.getConstArray();
    for (int i=0;i<aSeq.getLength();i++)
    {
        OUString aStr = pStr[i];
        // aArray += aStr + ASCII(", ");
        cout << aStr << endl;
    }
    volatile int dummy = 0;
}

//=============================================================================

inline void operator <<= (::rtl::OUString& _rUnicodeString, const sal_Char* _pAsciiString)
{
    _rUnicodeString = ::rtl::OUString::createFromAscii(_pAsciiString);
}

inline void operator <<= (::rtl::OUString& _rUnicodeString, const ::rtl::OString& _rAsciiString)
{
    _rUnicodeString <<= _rAsciiString.getStr();
}

inline void operator <<= (Any& _rUnoValue, const sal_Char* _pAsciiString)
{
    _rUnoValue <<= ::rtl::OUString::createFromAscii(_pAsciiString);
}

inline void operator <<= (Any& _rUnoValue, const ::rtl::OString& _rAsciiString)
{
    _rUnoValue <<= _rAsciiString.getStr();
}

//=============================================================================
void test_read_access(Reference< XInterface >& xIface, Reference< XMultiServiceFactory > &xMSF);
//=============================================================================
struct prompt_and_wait
{
    char const* myText;
    prompt_and_wait(char const* text = "") : myText(text) {}
    ~prompt_and_wait()
    {
        cout << myText << ">" << endl;
        int const mx = int( (+0u - +1u) >> 1);

        char c=0;
        if (cin.get(c) && c != '\n')
            cin.ignore(mx,'\n');
    }
};
static prompt_and_wait exit_prompt("Quitting\nQ");


Reference< XChangesBatch > xChangesBatch = NULL;
void commit()
{
    if (xChangesBatch.is())
    {
        xChangesBatch->commitChanges();
    }
}

// -----------------------------------------------------------------------------
static sal_Bool             s_bInitialized  =   sal_False;
#ifdef LLA_PRIVAT_DEBUG
static const sal_Char*      s_pSourcePath   =   "l:/src625/configmgr/workben/local_io/share";
static const sal_Char*      s_pUpdatePath   =   "l:/src625/configmgr/workben/local_io/user";
static const sal_Char*      s_pRootNode     =   "org.openoffice.ucb.Hierarchy"; // "org.openoffice.test";
static const sal_Char*      s_pServerType   =   "local";
static const sal_Char*      s_pLocale       =   "de-DE";
static const sal_Char*      s_pServer       =   "";
static const sal_Char*      s_pUser         =   "";
static const sal_Char*      s_pPassword     =   "";
#else
static const sal_Char*      s_pSourcePath   =   "g:/src/configmgr/workben/local_io/share";
static const sal_Char*      s_pUpdatePath   =   "g:/src/configmgr/workben/local_io/user";
static const sal_Char*      s_pRootNode     =   "org.openoffice.Office.TypeDetection";
static const sal_Char*      s_pServerType   =   "setup";
static const sal_Char*      s_pLocale       =   "de-DE";
static const sal_Char*      s_pServer       =   "lautrec-3108:19205";
static const sal_Char*      s_pUser         =   "lars";
static const sal_Char*      s_pPassword     =   "";
#endif


// -----------------------------------------------------------------------------
static void loadDefaults()
{
    if (s_bInitialized)
        return;

    s_bInitialized = sal_True;

    try
    {
        // the executable file name
        ::rtl::OUString sExecutable;
        osl_getExecutableFile(&sExecutable.pData);
        // cut the name, add a cfgapi.ini to the path
        sal_Int32 nLastSep = sExecutable.lastIndexOf('/');
        if (-1 != nLastSep)
            sExecutable = sExecutable.copy(0, nLastSep + 1);
#ifdef UNX
        sExecutable += ::rtl::OUString::createFromAscii("cfgapirc");
#else
        sExecutable += ::rtl::OUString::createFromAscii("cfgapi.ini");
#endif
        ::rtl::OUString sNormalized;
        sNormalized = sExecutable;
        if (1)
        {
            ::osl::Profile aProfile(sNormalized);

            static ::rtl::OString   sSection("defaults");
            static ::rtl::OString   sSourcePath("sourcepath");
            static ::rtl::OString   sUpdatePath("updatepath");
            static ::rtl::OString   sRootNode("rootnode");
            static ::rtl::OString   sServerType("servertype");
            static ::rtl::OString   sLocale("Locale");
            static ::rtl::OString   sServer("Server");
            static ::rtl::OString   sUser("User");
            static ::rtl::OString   sPassword("Password");

            // read some strings.
            // Do this static because we want to redirect the global static character pointers to the buffers.
            static ::rtl::OString s_sSourcePath = aProfile.readString(sSection, sSourcePath, s_pSourcePath);
            static ::rtl::OString s_sUpdatePath = aProfile.readString(sSection, sUpdatePath, s_pUpdatePath);
            static ::rtl::OString s_sRootNode   = aProfile.readString(sSection, sRootNode, s_pRootNode);
            static ::rtl::OString s_sServerType = aProfile.readString(sSection, sServerType, s_pServerType);
            static ::rtl::OString s_sLocale     = aProfile.readString(sSection, sLocale, s_pLocale);
            static ::rtl::OString s_sServer     = aProfile.readString(sSection, sServer, s_pServer);
            static ::rtl::OString s_sUser       = aProfile.readString(sSection, sUser, s_pUser);
            static ::rtl::OString s_sPassword   = aProfile.readString(sSection, sPassword, s_pPassword);

            // do this redirection
            s_pSourcePath   =   s_sSourcePath.getStr();
            s_pUpdatePath   =   s_sUpdatePath.getStr();
            s_pRootNode     =   s_sRootNode.getStr();
            s_pServerType   =   s_sServerType.getStr();
            s_pLocale       =   s_sLocale.getStr();
            s_pServer       =   s_sServer.getStr();
            s_pUser         =   s_sUser.getStr();
            s_pPassword     =   s_sPassword.getStr();
        }
    }
    catch(std::exception& e)
    {
        e.what();   // silence warnings
    }
}

// -----------------------------------------------------------------------------
Sequence<Any> createSequence(const OUString &sUser, const OUString &sPasswd)
{
    Sequence< Any > aCPArgs;

    if (sUser.getLength() > 0)
    {
        aCPArgs.realloc(1);
        aCPArgs[0] <<= configmgr::createPropertyValue(ASCII("user"), sUser);
    }
    if (sPasswd.getLength() > 0)
    {
        aCPArgs.realloc(2);
        aCPArgs[1] <<= configmgr::createPropertyValue(ASCII("password"), sPasswd);
    }
    return aCPArgs;
}

//=============================================================================
#include <string.h>
#if (defined UNX) || (defined OS2)
#else
#include <conio.h>
#endif

OString input(const char* pDefaultText, char cEcho)
{
    // PRE: a Default Text would be shown, cEcho is a Value which will show if a key is pressed.
    const int MAX_INPUT_LEN = 500;
    char aBuffer[MAX_INPUT_LEN];

    strcpy(aBuffer, pDefaultText);
    int nLen = strlen(aBuffer);

#ifdef WNT
    char ch = '\0';

    cout << aBuffer;
    cout.flush();

    while(ch != 13)
    {
        ch = getch();
        if (ch == 8)
        {
            if (nLen > 0)
            {
                cout << "\b \b";
                cout.flush();
                --nLen;
                aBuffer[nLen] = '\0';
            }
            else
            {
                cout << "\a";
                cout.flush();
            }
        }
        else if (ch != 13)
        {
            if (nLen < MAX_INPUT_LEN)
            {
                if (cEcho == 0)
                {
                    cout << ch;
                }
                else
                {
                    cout << cEcho;
                }
                cout.flush();
                aBuffer[nLen++] = ch;
                aBuffer[nLen] = '\0';
            }
            else
            {
                cout << "\a";
                cout.flush();
            }
        }
    }
#else
    if (!cin.getline(aBuffer,sizeof aBuffer))
        return OString();
#endif
    return OString(aBuffer);
}

// -----------------------------------------------------------------------------
rtl::OUString enterValue(const char* _aStr, const char* _aDefault, bool _bIsAPassword)
{
    cout << _aStr;
    cout.flush();

    OUString sValue;
    sValue <<= input(_aDefault, _bIsAPassword ? '*' : 0);
    return sValue;
}



// -----------------------------------------------------------------------------
// ---------------------------------- M A I N ----------------------------------
// -----------------------------------------------------------------------------

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    TimeValue aTimeout;
    aTimeout.Seconds = 5;
    aTimeout.Nanosec = 0;

    // cout << "    Please insert Text: ";
    // cout.flush();
    // OString aTxt = input("Der Text", 0);
    // cout << endl << "You inserted: " << aTxt.getStr() << endl;
    //
    // cout << "Please insert Password: ";
    // cout.flush();
    // OString aPasswd = input("", '*');
    // cout << endl << "You inserted: " << aPasswd.getStr() << endl;

    loadDefaults();

    try
    {
        OUString const sServiceRegistry = OUString::createFromAscii( argc > 1 ? argv[1] : "applicat.rdb" );
        Reference< XMultiServiceFactory > xORB = createRegistryServiceFactory(
            sServiceRegistry,
            ::rtl::OUString()
            );
        if (!xORB.is())
        {
            ::flush(cout);
            cerr << "Could not create the service factory !\n\n";
            return 1;
        }
        cout << "Service factory created !\n---------------------------------------------------------------" << endl;

        Sequence< Any > aCPArgs;

        OUString sServerType = enterValue("servertype: ", s_pServerType, false);
        cout << endl;


        rtl::OUString sUser;

        bool bLocal = sServerType.equalsIgnoreAsciiCase(ASCII("local")) || sServerType.equalsIgnoreAsciiCase(ASCII("setup"));
        if (!bLocal)
        {
            rtl::OUString sServer;
            sServer =           enterValue("server  : ", s_pServer,false);
            cout << endl;

            sUser =             enterValue("user    : ", s_pUser, false);
            cout << endl;

            OUString sPasswd =  enterValue("password: ", s_pPassword, true);
            cout << endl;

            aCPArgs = createSequence(sUser, sPasswd);

            aCPArgs.realloc(aCPArgs.getLength() + 1);
            aCPArgs[aCPArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("server"), sServer);

            OUString sTimeout = ASCII("10000");
            aCPArgs.realloc(aCPArgs.getLength() + 1);
            aCPArgs[aCPArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("timeout"), sTimeout);

        }
        else
        {
            rtl::OUString sSharePath, sUserPath;
            sSharePath =        enterValue("share path: ", s_pSourcePath, false);
            cout << endl;
            sUserPath =         enterValue("user path : ", s_pUpdatePath, false);
            cout << endl;

            aCPArgs.realloc(aCPArgs.getLength() + 1);
            sal_Int32 nCount = aCPArgs.getLength() - 1;
            Any *pAny = &aCPArgs[nCount];
            *pAny <<= configmgr::createPropertyValue(ASCII("sourcepath"), sSharePath);
            aCPArgs.realloc(aCPArgs.getLength() + 1);
            aCPArgs[aCPArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("updatepath"), sUserPath);
        }

        aCPArgs.realloc(aCPArgs.getLength() + 1);
        aCPArgs[aCPArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("servertype"), sServerType);

        Reference< XMultiServiceFactory > xCfgProvider(
            xORB->createInstanceWithArguments(
                ::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider"),
                aCPArgs),
            UNO_QUERY);
        if (!xCfgProvider.is())
        {
            ::flush(cout);
            cerr << "Could not create the configuration provider !\n\n";
            return 3;
        }




        char aPath[300] =           "/";
        int nStart = sizeof(    "/" ) - 1;

        cout << "---------------------------------------------------------------\n Configuration Provider created !\n---------------------------------------------------------------" << endl;

        Sequence< Any > aArgs;
        aArgs = createSequence(sUser, ASCII(""));

        OUString sPath =    enterValue("nodepath: ", s_pRootNode, false);
        cout << endl;

        aArgs.realloc(aArgs.getLength() + 1);
        aArgs[aArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("nodepath"), sPath);

        if (!bLocal)
        {
            OUString sLocale =  enterValue("locale  : ", s_pLocale, false);
            cout << endl;
            aArgs.realloc(aArgs.getLength() + 1);
            aArgs[aArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("locale"), sLocale);
        }
/*
#else
        OUString aStr = ASCII("String");
        sal_Int32 nDepth = 10;
        Sequence< Any > aArgs(2);

        aArgs[0] <<= aStr;
        aArgs[1] <<= nDepth;
#endif
*/
        Reference< XInterface > xIFace = xCfgProvider->createInstanceWithArguments(
            OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess"),
            aArgs);
        cout << "---------------------------------------------------------------\n Configuration Read/Write Access created !\n---------------------------------------------------------------" << endl;

        xChangesBatch = Reference< XChangesBatch >(xIFace, UNO_QUERY);

        Sequence<OUString> aSeq = xCfgProvider->getAvailableServiceNames();
        showSequence(aSeq);

        test_read_access(xIFace, xCfgProvider);
    }
    catch (Exception& e)
    {
        ::flush(cout);
        cerr << "Caught exception: " << e.Message << endl;
    }
/*
    catch (...)
    {
        flush(cout);
        cerr << "BUG: Caught UNKNOWN exception (?) " << endl;
    }
*/
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
void test(Reference< XHierarchicalName >& xAccessName)
{
        if (xAccessName.is())
            cout << "Accessing Node: " << xAccessName->getHierarchicalName();
        else
            cout << "BUG: XHierarchicalName not available";
        cout << endl;
}
void test(Reference< XNamed >& xAccess)
{
        if (xAccess.is())
            cout << "Node is named: " << xAccess->getName();
        else
            cout << "BUG: XNamed not available";
        cout << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////
void write(Reference<XNameAccess  >& xAccess)
{
        if (xAccess.is())
        {
            Sequence<OUString> aNames( xAccess->getElementNames() );

            cout << "Element Names: (" << aNames.getLength() << ")";
            for (int i = 0; i < aNames.getLength(); ++i)
                cout << "\n[" << i << "] -\t" << aNames[i];
            cout << endl;
        }
        else
            cout << "BUG: XNameAccess not available";
        cout << endl;
}
void write(Reference< XChild >& xChild)
{
        if (xChild.is())
            cout << "\n[ P ] -\tParent";
        else
            cout << "BUG: Parent not available (no XChild)";
        cout << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////

bool ask(Reference< XInterface >& xIface, Reference<XMultiServiceFactory> &);

void test_read_access(Reference< XInterface >& xIface, Reference< XMultiServiceFactory > &xMSF)
{
    using com::sun::star::uno::UNO_QUERY;
    do
    {
        cout << "\n\n---------------------------------------------------------------" << endl;
        Reference< XNameAccess > xAccess(xIface, UNO_QUERY);
        Reference< XChild > xChild(xIface, UNO_QUERY);
        Reference< XHierarchicalName > xAccessPath(xIface,UNO_QUERY);
        Reference< XNamed > xAccessName(xIface,UNO_QUERY);
//      Reference< XHierarchicalNameAccess >& xAccess(xIface, UNO_QUERY);

        test(xAccessPath);
        test(xAccessName);
        write(xAccess);
        write(xChild);
    }
    while (ask(xIface, xMSF));
}

bool ask(Reference< XInterface >& xIface, Reference< XMultiServiceFactory > &xMSF)
{
    cout << "\n[ Q ] -> <Quit>";
    cout << "\n[ S ] -> <SetValue> ";
    cout << endl;

    cout << "\n:> " << flush;
    char buf[200] = {0};
    try
    {
        bool bHandled = false;
        bool bInserted = false;

        if (cin.getline(buf,sizeof buf))
        {
            Reference< XInterface > xNext;
            if ((buf[0] == 'q' || buf[0] == 'Q') && (0 == buf[1]))
            {
                return false;
            }
            else if (buf[0] == 0)
            {
                return true;
            }
            else if((buf[0] == 0 || buf[0] == 'o' || buf[0] == 'O') && (0 == buf[1]))
            {
/*
                cout << "work Offline" << endl;
                Reference<com::sun::star::configuration::XConfigurationSync> xSync(xMSF, UNO_QUERY);

                Sequence< Any > aArgs2(5);
                sal_Int32 n=0;
                aArgs2[n++] <<= configmgr::createPropertyValue(ASCII("path"), ASCII("org.openoffice.Setup"));
                // aArgs2[n++] <<= configmgr::createPropertyValue(ASCII("path"), ASCII("org.openoffice.Office.Common"));
                // aArgs2[n++] <<= configmgr::createPropertyValue(ASCII("path"), ASCII("org.openoffice.Office.Java"));
                // aArgs2[n++] <<= configmgr::createPropertyValue(ASCII("path"), ASCII("org.openoffice.Office.Writer"));
                // aArgs2[n++] <<= configmgr::createPropertyValue(ASCII("path"), ASCII("org.openoffice.Office.ucb.Hierarchy"));
                xSync->offline(aArgs2);
                bHandled = true;
*/
            }
            else if((buf[0] == 0 || buf[0] == 's' || buf[0] == 'S') && (0 == buf[1]))
            {
                // Replace a Value
                Reference< XNameAccess > xAccess(xIface, UNO_QUERY);

                cout << "SetMode, insert a Number" << endl;
                cin.getline(buf,sizeof buf);
                bInserted = true;
            }

            else if ((buf[0] == 'p' || buf[0] == 'P') && (0 == buf[1]))
            {
                Reference< XChild > xChild(xIface, UNO_QUERY);
                if (xChild.is())
                    xNext = xChild->getParent();
                bHandled = true;
            }

            if (bHandled == false)
            {
                Reference< XNameAccess > xAccess(xIface, UNO_QUERY);
                Reference< XHierarchicalNameAccess > xDeepAccess(xIface, UNO_QUERY);
                Reference< XExactName > xExactName(xIface, UNO_QUERY);

                if (xAccess.is() || xDeepAccess.is())
                {
                    OUString aName;
                    OUString aInput = OUString::createFromAscii(buf);

                    if (xExactName.is())
                    {
                        ::rtl::OUString sTemp = xExactName->getExactName(aInput);
                        if (sTemp.getLength())
                            aInput = sTemp;
                    }

                    if (xAccess.is() && xAccess->hasByName(aInput))
                    {
                        aName = aInput;
                    }
                    else if (xDeepAccess.is() && xDeepAccess->hasByHierarchicalName(aInput))
                    {
                        aName = aInput;
                    }
                    else if ('0' <= buf[0] && buf[0] <= '9' && xAccess.is())
                    {
                        unsigned int n = unsigned(atoi(buf));
                        Sequence<OUString> aNames = xAccess->getElementNames();
                        if (n < aNames.getLength())
                            aName = aNames[n];
                    }

                    if (aName.getLength())
                    {
                        bool bNest = aInput.indexOf(sal_Unicode('/')) >= 0;

                        Any aElement = bNest    ? ( xDeepAccess.is() ? xDeepAccess->getByHierarchicalName(aName) : Any())
                                                : ( xAccess.    is() ? xAccess->    getByName(aName)             : Any() );

                        while (aElement.getValueTypeClass() == TypeClass_ANY)
                        {
                            Any aWrap(aElement);
                            aWrap >>= aElement;
                        }
                        sal_Bool bValue = true;
                        sal_Bool bValueOk = false;

                        switch (aElement.getValueTypeClass() )
                        {
                        case TypeClass_INTERFACE: bValue = false; break;
                        case TypeClass_BOOLEAN:
                            {
                                sal_Bool* pVal = (sal_Bool*)aElement.getValue();
                                bValueOk = (pVal != 0);

                                cout << "VALUE '" << aName << "' is a BOOLEAN = ";
                                if (!bValueOk)
                                    cout << "NULL (error!!)";
                                else if (*pVal)
                                    cout << "'TRUE'";
                                else
                                    cout << "'FALSE'";

                                cout << endl;
                            }
                            break;
                        case TypeClass_SHORT:
                            {
                                sal_Int16 aValue;
                                cout << "VALUE '" << aName << "' is a SHORT (16 bit) = ";
                                if (bValueOk = (aElement >>= aValue))
                                    cout << aValue;
                                else
                                    cout << "ERROR RETRIEVING VALUE";
                                cout << endl;
                            }
                            break;
                        case TypeClass_LONG:
                            {

                                sal_Int32 aValue;
                                cout << "VALUE '" << aName << "' is a INT (32 bit) = ";
                                if (bValueOk = (aElement >>= aValue))
                                    cout << aValue;
                                else
                                    cout << "ERROR RETRIEVING VALUE";
                                cout << endl;
                            }
                            break;
                        case TypeClass_HYPER:
                            {
                                sal_Int64 aValue;
                                cout << "VALUE '" << aName << "' is a LONG (64 bit) = ";
                                if (bValueOk = (aElement >>= aValue))
                                    cout << double(aValue);
                                else
                                    cout << "ERROR RETRIEVING VALUE";
                                cout << endl;
                            }
                            break;
                        case TypeClass_DOUBLE:
                            {
                                double aValue;
                                cout << "VALUE '" << aName << "' is a DOUBLE = ";
                                if (bValueOk = (aElement >>= aValue))
                                    cout << aValue;
                                else
                                    cout << "ERROR RETRIEVING VALUE";
                                cout << endl;
                            }
                            break;
                        case TypeClass_STRING:
                            {
                                OUString aValue;
                                cout << "VALUE '" << aName << "' is a STRING = ";
                                if (bValueOk = (aElement >>= aValue))
                                    cout << "\"" << aValue << "\"";
                                else
                                    cout << "ERROR RETRIEVING VALUE";
                                cout << endl;
                            }
                            break;
                        case TypeClass_SEQUENCE:
                            {
                                cout << "VALUE '" << aName << "' is a SEQUENCE or BINARY" << endl;

                                Type aTypeS = configmgr::getSequenceElementType(aElement.getValueType());
                                OUString sType = configmgr::toTypeName(aTypeS.getTypeClass());
                                cout << "Real type is Sequence<" << sType << ">" << endl;
                                bValueOk = true;
                            }
                            break;
                        case TypeClass_VOID:
                            cout << "ELEMENT '" << aName << "' is NULL and VOID " << endl;
                            bValueOk = true;
                            break;
                        default:
                                cout << "Error: ELEMENT '" << aName << "' is of unknown or unrecognized type" << endl;
                            break;
                        }
                        if (bValue)
                        {
                            if (bInserted)
                            {
                                if (aElement.getValueTypeClass() == TypeClass_BOOLEAN ||
                                    aElement.getValueTypeClass() == TypeClass_VOID)
                                {
                                    cout << "Set Value (Type=BOOL) to :";
                                    cout.flush();
                                    cin.getline(buf,sizeof buf);
                                    OUString aInput = OUString::createFromAscii(buf);
                                    sal_Bool bValue = false;
                                    if (aInput.equalsIgnoreAsciiCase(ASCII("true")))
                                        bValue = true;

                                    OUString aStr = ASCII("false");
                                    Any aValueAny;
                                    aValueAny <<= bValue;

                                    Reference< XNameReplace > xNameReplace(xAccess, UNO_QUERY);
                                    if (xNameReplace.is())
                                    {
                                        xNameReplace->replaceByName(aName, aValueAny);
                                        commit();
                                    }
                                    bInserted = false;
                                }
                                else if (aElement.getValueTypeClass() == TypeClass_STRING)
                                {
                                    cout << "set value (type = string) to : ";
                                    cout.flush();
                                    cin.getline(buf,sizeof buf);
                                    Any aValue;
                                    aValue <<= buf;

                                    Reference< XNameReplace > xNameReplace(xAccess, UNO_QUERY);
                                    if (xNameReplace.is())
                                    {
                                        xNameReplace->replaceByName(aName, aValue);
                                        commit();
                                    }
                                    bInserted = false;
                                }
                                else
                                {
                                    cout << "Sorry, only BOOLEAN Values can changed today." << endl;
                                }
                            }
                            prompt_and_wait();
                            return bValueOk ? true : false;
                        }

                        if (aElement >>= xNext)
                            cout << "Got an Interface for '" << aName << "'" << endl;
                        else
                            cout << "Error: Cannot get an Interface for '" << aName << "'" << endl;
                    }
                    else
                    {
                        cout << "Error: No element \"" << aInput << "\" found." <<endl;
                    }
                }

            }
            if (xNext.is())
            {
                xIface = xNext;
                return true;
            }
            cout << "Error: could not obtain the requested Object " << endl;
        }
        else
        {
            cout << "Input Error " << endl;
            return true;
        }
    }
    catch (Exception& e)
    {
        cout << "An Exception occurred: " << e.Message << endl;

    }
    catch (...)
    {
        cout << "An UNKNOWN Exception occurred !" << endl;
    }

    prompt_and_wait();
    return true;
}
