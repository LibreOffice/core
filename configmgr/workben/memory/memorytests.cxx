/*************************************************************************
 *
 *  $RCSfile: memorytests.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2001-06-15 08:29:44 $
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
#include <vector>
#include "treeload.hxx"

#include <fstream>

#define ENABLE_MEMORYMEASURE
#define ENABLE_LOGMECHANISM

// If you wish to enable this memory measure macros ... you need "windows.h"
// But it's not agood idea to include it in your header!!! Because it's not compatible to VCL header .-(
// So you must include it here ... in cxx, where you whish to use it.
#ifdef ENABLE_MEMORYMEASURE
    #define VCL_NEED_BASETSD
    #include <tools/presys.h>
    #include <windows.h>
    #include <tools/postsys.h>
    #undef  VCL_NEED_BASETSD
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_MEMORYMEASURE_HXX_
#include "memorymeasure.hxx"
#endif

#include "logmechanism.hxx"

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

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
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

#include <conio.h>

#include "createpropertyvalue.hxx"

#include "typeconverter.hxx"

#include "memory.hxx"

#include "valuenode.hxx"


// #include <com/sun/star/configuration/XConfigurationSync.hpp>

namespace configmgr
{

using namespace std;

namespace css = com::sun::star;
namespace uno = css::uno;
namespace lang = css::lang;

using namespace uno;
using namespace lang;

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;

using ::rtl::OUString;
using ::rtl::OString;

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

inline void operator <<= (::rtl::OString& _rAsciiString, ::rtl::OUString const& _rUnicodeString )
{
    _rAsciiString = rtl::OUStringToOString(_rUnicodeString, RTL_TEXTENCODING_ASCII_US).getStr();
}

// -----------------------------------------------------------------------------

rtl::OString input(const char* pDefaultText, char cEcho)
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
    return rtl::OString(aBuffer);
}

// -----------------------------------------------------------------------------
rtl::OUString enterValue(const char* _aStr, const char* _aDefault, bool _bIsAPassword)
{
    cout << _aStr;
    cout.flush();

    rtl::OUString sValue;
    sValue <<= input(_aDefault, _bIsAPassword ? '*' : 0);
    return sValue;
}
//=============================================================================

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
// static const sal_Char*       s_pSourcePath   =   "//./l|/src632/configmgr/workben/local_io/share";
// static const sal_Char*       s_pUpdatePath   =   "//./l|/src632/configmgr/workben/local_io/user";
static const sal_Char*      s_pSourcePath   =   "file:///f:/office60_633/share/config/registry";
static const sal_Char*      s_pUpdatePath   =   "file:///f:/office60_633/user/config/registry";
static const sal_Char*      s_pRootNode     =   "org.openoffice.test";
static const sal_Char*      s_pServerType   =   "local";
static const sal_Char*      s_pLocale       =   "de-DE";
static const sal_Char*      s_pServer       =   "";
static const sal_Char*      s_pUser         =   "";
static const sal_Char*      s_pPassword     =   "";
#else
static const sal_Char*      s_pSourcePath   =   "g:/src/configmgr/workben/local_io/share";
static const sal_Char*      s_pUpdatePath   =   "g:/src/configmgr/workben/local_io/user";
static const sal_Char*      s_pRootNode     =   "org.openoffice.test";
static const sal_Char*      s_pServerType   =   "local";
static const sal_Char*      s_pLocale       =   "de-DE";
static const sal_Char*      s_pServer       =   "lautrec-3108:19205";
static const sal_Char*      s_pUser         =   "lars";
static const sal_Char*      s_pPassword     =   "";
#endif

static bool m_bChange = false;

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
        if (1) // osl_File_E_None == osl_normalizePath(sExecutable.pData, &sNormalized.pData))
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

// -----------------------------------------------------------------------------

void test_configuration_provider(uno::Reference<lang::XMultiServiceFactory> _xCfgProvider,
                                 rtl::OUString const& _sPath,
                                 rtl::OUString const& _sUser, bool _bLocal, sal_Int32 _nCount);


// -----------------------------------------------------------------------------

uno::Reference<lang::XMultiServiceFactory>
getProvider(
    uno::Reference< lang::XMultiServiceFactory > _xServiceRegistry,
    rtl::OUString const& _sServerType,
    rtl::OUString const& _sSharePath, rtl::OUString const& _sUserPath,
    bool &_bLocal)
{
    try
    {
        Sequence< Any > aCPArgs;

        OUString sServerType = _sServerType; // enterValue("servertype: ", s_pServerType, false);

        rtl::OUString sUser;

        _bLocal = sServerType.equalsIgnoreAsciiCase(ASCII("local")) || sServerType.equalsIgnoreAsciiCase(ASCII("setup"));
        if (!_bLocal)
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
            sSharePath = _sSharePath;//     enterValue("share path: ", s_pSourcePath, false);
            // cout << endl;
            sUserPath = _sUserPath; //      enterValue("user path : ", s_pUpdatePath, false);
            // cout << endl;

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
            _xServiceRegistry->createInstanceWithArguments(
                ::rtl::OUString::createFromAscii("com.sun.star.configuration.ConfigurationProvider"),
                aCPArgs),
            UNO_QUERY);
        if (!xCfgProvider.is())
        {
            ::flush(cout);
            cerr << "Could not create the configuration provider !\n\n";
            return 0;
        }
// -----------------------------------------------------------------------------
        return xCfgProvider;
    }
    catch (Exception& e)
    {
        ::flush(cout);
        cerr << "Caught exception: " << e.Message << endl;
    }
    return 0;
}

// -----------------------------------------------------------------------------
sal_Int32 m_nCount = 1;

void test(uno::Reference<lang::XMultiServiceFactory> _xORB, rtl::OUString const& _sSharePath,
          rtl::OUString const& _sUserPath, rtl::OUString const& _sPath)
{

    rtl::OUString sUser;
    bool bLocal;
    cout << m_nCount << ". start test with: " << _sPath << endl;
 {
     uno::Reference<lang::XMultiServiceFactory>xCfgProvider =
         getProvider(_xORB, ASCII("local"), _sSharePath, _sUserPath,
                     bLocal);

     test_configuration_provider(xCfgProvider, _sPath, sUser, bLocal, m_nCount); // xml

     uno::Reference<lang::XComponent>xComponent(xCfgProvider,UNO_QUERY);
     xComponent->dispose();
 }

/*
 // If m_nCount == 1:
 // sal_Int16 will increase by 1
 ++ m_nCount;
    cout << m_nCount << ". start test with: " << _sPath << endl;
 {
     uno::Reference<lang::XMultiServiceFactory>xCfgProvider =
         getProvider(_xORB, ASCII("local"), _sSharePath, _sUserPath,
                     bLocal);

     test_configuration_provider(xCfgProvider, _sPath, sUser, bLocal, m_nCount); // binary

     uno::Reference<lang::XComponent>xComponent(xCfgProvider,UNO_QUERY);
     xComponent->dispose();
 }
 // if m_nCount == 2:
 // sal_Int16 will decrease by 1
 ++m_nCount;
 cout << m_nCount << ". start test with: " << _sPath << endl;
 {
     uno::Reference<lang::XMultiServiceFactory>xCfgProvider =
         getProvider(_xORB, ASCII("local"), _sSharePath, _sUserPath,
                     bLocal);

     test_configuration_provider(xCfgProvider, _sPath, sUser, bLocal, m_nCount); // binary

     uno::Reference<lang::XComponent>xComponent(xCfgProvider,UNO_QUERY);
     xComponent->dispose();
 }

 ++m_nCount;
 cout << m_nCount << ". start test with: " << _sPath << endl;
 {
     uno::Reference<lang::XMultiServiceFactory>xCfgProvider =
         getProvider(_xORB, ASCII("local"), _sSharePath, _sUserPath,
                     bLocal);

     test_configuration_provider(xCfgProvider, _sPath, sUser, bLocal, m_nCount); // binary

     uno::Reference<lang::XComponent>xComponent(xCfgProvider,UNO_QUERY);
     xComponent->dispose();
 }

 ++m_nCount;
 cout << m_nCount << ". start test with: " << _sPath << endl;
 {
     uno::Reference<lang::XMultiServiceFactory>xCfgProvider =
         getProvider(_xORB, ASCII("local"), _sSharePath, _sUserPath,
                     bLocal);

     test_configuration_provider(xCfgProvider, _sPath, sUser, bLocal, m_nCount); // binary

     uno::Reference<lang::XComponent>xComponent(xCfgProvider,UNO_QUERY);
     xComponent->dispose();
 }

 ++m_nCount;
 cout << m_nCount << ". start test with: " << _sPath << endl;
 {
     uno::Reference<lang::XMultiServiceFactory>xCfgProvider =
         getProvider(_xORB, ASCII("local"), _sSharePath, _sUserPath,
                     bLocal);

     test_configuration_provider(xCfgProvider, _sPath, sUser, bLocal, m_nCount); // binary

     uno::Reference<lang::XComponent>xComponent(xCfgProvider,UNO_QUERY);
     xComponent->dispose();
 }
*/
 cout << "finish" << endl;

 // Test Version 1 and 3, it MUST be equal
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// ---------------------------------- M A I N ----------------------------------
// -----------------------------------------------------------------------------

int main( int argc, char * argv[] )
{
    TimeValue aTimeout;
    aTimeout.Seconds = 5;
    aTimeout.Nanosec = 0;

    sal_Int32 nSizeISubtree = sizeof(ISubtree);
    sal_Int32 nSizeINode = sizeof(INode);
    sal_Int32 nSizeIValueNode = sizeof(ValueNode);
    {
        loadDefaults();

        OUString const sServiceRegistry = OUString::createFromAscii( argc > 1 ? argv[1] : "applicat.rdb" );
        Reference< XMultiServiceFactory > xORB;
        try
        {
            xORB = createRegistryServiceFactory( sServiceRegistry, ::rtl::OUString() );
            if (!xORB.is())
            {
                ::flush(cout);
                cerr << "Could not create the service factory !\n\n";
                return 0;
            }
        }
        catch (uno::Exception &e)
        {
            cout << "Error: can't get ServiceFactory" << sServiceRegistry << endl;
        }

        rtl::OUString sSharePath = enterValue("share path: ", s_pSourcePath, false);
        cout << endl;
        rtl::OUString sUserPath =   enterValue("user path : ", s_pUpdatePath, false);
        cout << endl;

        // OUString sPath = enterValue("nodepath: ", s_pRootNode, false);
        // cout << endl;

        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.test"));

        // char* pMem = new char[1000 * 1000 * 10];
        // showMemoryStatistic();

//      test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Views"));

        START_MEMORYMEASURE( aMemoryInfo );

        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "initialisierung" );

        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "initialisierung weil beim ersten mal falsch!" );

        // delete [] pMem;
        // showMemoryStatistic();

        // standard office start.
//      test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Common/Start"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Common"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Setup/CJK/Enable"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Setup/Office/Modules"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Inet"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Views"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Setup/Product"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.ucb.Configuration"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.ucb.Store/ContentProperties"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.TypeDetection"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Writer"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.WriterWeb"));
        test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Linguistic"));

        goto halt;
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Common"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Linguistic"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.TypeDetection"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Setup"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.UserProfile"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Inet"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Calc"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Chart"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.DataAccess"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Draw"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Impress"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Java"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Labels"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Math"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Views"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.Writer"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.Office.WriterWeb"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.ucb.Configuration"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.ucb.Hierarchy"));
        // test(xORB, sSharePath, sUserPath, ASCII("org.openoffice.ucb.Store"));

      halt:

        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "ich habe fertig!" );

        // stop the programm clear.
        Reference< XComponent > xComponent(xORB, UNO_QUERY);
        xComponent->dispose();
        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "after dispose component." );

        LOG_MEMORYMEASURE( "FirstTest_of_memusage", "Values of memory access for standard filters.", aMemoryInfo );
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
void test(Reference< XHierarchicalName >& xAccessName)
{
        if (xAccessName.is())
        {
            // cout << "Accessing Node: " << xAccessName->getHierarchicalName();
        }
        else
        {
            // cout << "BUG: XHierarchicalName not available";
        }
        // cout << endl;
}
void test(Reference< XNamed >& xAccess)
{
        if (xAccess.is())
        {
            // cout << "Node is named: " << xAccess->getName();
        }
        else
        {
            // cout << "BUG: XNamed not available";
        }
        // cout << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////
void fillAllNames(Reference<XNameAccess  >& xAccess, Sequence<OUString>& _aSeq)
{
        if (xAccess.is())
        {
            _aSeq = Sequence <OUString>(xAccess->getElementNames());

            // cout << "Element Names: (" << _aSeq.getLength() << ")";
            // for (int i = 0; i < _aSeq.getLength(); ++i)
            //  cout << "\n[" << i << "] -\t" << _aSeq[i];
            //cout << endl;
        }
        else
        {
            // cout << "BUG: XNameAccess not available";
        }
        // cout << endl;
}
void write(Reference< XChild >& xChild)
{
        if (xChild.is())
        {
            // cout << "\n[ P ] -\tParent";
        }
        else
        {
            // cout << "BUG: Parent not available (no XChild)";
        }
        // cout << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
Reference< XInterface > readaccess(uno::Reference< lang::XMultiServiceFactory > &xMSF,
                                   uno::Reference< uno::XInterface > const& xIface,
                                   rtl::OString const& buf,
                                   ostream& outStream)
{
    Reference< XInterface > xNext;
    try
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

                Any aElement = bNest ?
                    ( xDeepAccess.is() ?
                      xDeepAccess->getByHierarchicalName(aName) : Any()
                      ) :
                    ( xAccess.is() ?
                      xAccess->getByName(aName) : Any()
                      );

                while (aElement.getValueTypeClass() == TypeClass_ANY)
                {
                    Any aWrap(aElement);
                    aWrap >>= aElement;
                }
                sal_Bool bValue = true;
                sal_Bool bValueOk = false;

                switch (aElement.getValueTypeClass() )
                {
                case TypeClass_INTERFACE:
                    bValue = false;
                    if (aElement >>= xNext)
                    {
                        outStream << "Group: " << aName << endl;
                    }
                    else
                    {
                        outStream << "ERROR: can't get the interface" << endl;
                    }
                    break;
                case TypeClass_BOOLEAN:
                {
                    sal_Bool* pVal = (sal_Bool*)aElement.getValue();
                    bValueOk = (pVal != 0);

                    outStream << "VALUE '" << aName << "' is a BOOLEAN = ";
                    if (!bValueOk)
                        outStream << "NULL (error!!)";
                    else if (*pVal)
                        outStream << "'TRUE'";
                    else
                        outStream << "'FALSE'";

                    outStream << endl;

                    // tryToChange(xAccess, aName, pVal);
                    break;
                }
                case TypeClass_SHORT:
                {
                    sal_Int16 aValue;
                    outStream << "VALUE '" << aName << "' is a SHORT (16 bit) = ";
                    if (bValueOk = (aElement >>= aValue))
                    {
                        outStream << aValue;
                        // tryToChange(xAccess, aName, aValue);
                    }
                    else
                    {
                        outStream << "ERROR RETRIEVING VALUE";
                    }
                    outStream << endl;

                    break;
                }
                case TypeClass_LONG:
                {

                    sal_Int32 aValue;
                    outStream << "VALUE '" << aName << "' is a INT (32 bit) = ";
                    if (bValueOk = (aElement >>= aValue))
                    {
                        outStream << aValue;
                        // tryToChange(xAccess, aName, aValue);
                    }
                    else
                    {
                        outStream << "ERROR RETRIEVING VALUE";
                    }
                    outStream << endl;
                    break;
                }
                case TypeClass_HYPER:
                {
                    sal_Int64 aValue;
                    outStream << "VALUE '" << aName << "' is a LONG (64 bit) = ";
                    if (bValueOk = (aElement >>= aValue))
                    {
                        outStream << double(aValue);
                        // tryToChange(xAccess, aName, aValue);
                    }
                    else
                    {
                        outStream << "ERROR RETRIEVING VALUE";
                    }
                    outStream << endl;
                    break;
                }
                case TypeClass_DOUBLE:
                {
                    double aValue;
                    outStream << "VALUE '" << aName << "' is a DOUBLE = ";
                    if (bValueOk = (aElement >>= aValue))
                    {
                        outStream << aValue;
                    }
                    else
                    {
                        outStream << "ERROR RETRIEVING VALUE";
                    }
                    outStream << endl;
                    break;
                }
                case TypeClass_STRING:
                {
                    OUString aValue;
                    outStream << "VALUE '" << aName << "' is a STRING = ";
                    if (bValueOk = (aElement >>= aValue))
                    {
                        outStream << "\"" << aValue << "\"";
                        // tryToChange(xAccess, aName, aValue);
                    }
                    else
                    {
                        outStream << "ERROR RETRIEVING VALUE";
                    }
                    outStream << endl;
                    break;
                }
                case TypeClass_SEQUENCE:
                {
                    outStream << "VALUE '" << aName << "' is a SEQUENCE or BINARY" << endl;

                    Type aTypeS = configmgr::getSequenceElementType(aElement.getValueType());
                    OUString sType = configmgr::toTypeName(aTypeS.getTypeClass());
                    outStream << "UNO type is " << aElement.getValueType().getTypeName()  << endl;
                    outStream << "Real type is Sequence<" << sType << ">" << endl;
                    // outSequence(aElement, aTypeS, outStream);
                    bValueOk = true;
                    break;
                }
                case TypeClass_VOID:
                    outStream << "ELEMENT '" << aName << "' is NULL and VOID " << endl;
                    bValueOk = true;
                    break;
                default:
                    outStream << "Error: ELEMENT '" << aName << "' is of unknown or unrecognized type" << endl;
                    break;
                }

            }
            else
            {
                outStream << "Error: No element \"" << aInput << "\" found." <<endl;
            }
        }
    }
    catch (Exception& e)
    {
        outStream << "An Exception occurred: " << e.Message << endl;
    }
    catch (...)
    {
        outStream << "An UNKNOWN Exception occurred !" << endl;
    }

    return xNext;
}




// -----------------------------------------------------------------------------
void test_read_access( uno::Reference< lang::XMultiServiceFactory > &xMSF,
                       uno::Reference< uno::XInterface >& xIface,
                       ofstream & out)
{
    Sequence<OUString> aAllNames;

    using com::sun::star::uno::UNO_QUERY;

    // cout << "\n\n---------------------------------------------------------------" << endl;
    Reference< XNameAccess > xAccess(xIface, UNO_QUERY);
    Reference< XChild > xChild(xIface, UNO_QUERY);
    Reference< XHierarchicalName > xAccessPath(xIface,UNO_QUERY);
    Reference< XNamed > xAccessName(xIface,UNO_QUERY);
//      Reference< XHierarchicalNameAccess >& xAccess(xIface, UNO_QUERY);

    test(xAccessPath);
    test(xAccessName);
    fillAllNames(xAccess, aAllNames);
    write(xChild);

    for (sal_Int32 i=0;i<aAllNames.getLength();i++)
    {
        OString aValue;
        aValue <<= aAllNames[i];
        uno::Reference<uno::XInterface> xFace = readaccess(xMSF, xIface, aValue, out);
        if (xFace.is())
        {
            test_read_access(xMSF, xFace, out);
        }
    }
}

// -----------------------------------------------------------------------------
void test_read_access( uno::Reference< lang::XMultiServiceFactory > &xMSF,
                       uno::Reference< uno::XInterface >& xIface,
                       rtl::OString aFilename)
{
    ofstream out(aFilename.getStr());
    test_read_access(xMSF, xIface, out);
}

// -----------------------------------------------------------------------------
void test_configuration_provider(uno::Reference<lang::XMultiServiceFactory> _xCfgProvider,
                                 rtl::OUString const& _sPath,
                                 rtl::OUString const& _sUser, bool _bLocal,
                                 sal_Int32 _nCount)
{
    Sequence< Any > aArgs;
    aArgs = createSequence(_sUser, ASCII(""));

    aArgs.realloc(aArgs.getLength() + 1);
    aArgs[aArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("nodepath"), _sPath);

    if (!_bLocal)
    {
        OUString sLocale =  enterValue("locale  : ", s_pLocale, false);
        cout << endl;
        aArgs.realloc(aArgs.getLength() + 1);
        aArgs[aArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("locale"), sLocale);
    }

    sal_Bool bLazyWrite = true;
    aArgs.realloc(aArgs.getLength() + 1);
    aArgs[aArgs.getLength() - 1] <<= configmgr::createPropertyValue(ASCII("lazywrite"), bLazyWrite);

    Reference< XInterface > xIFace = _xCfgProvider->createInstanceWithArguments(
        /* OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess"), */
        OUString::createFromAscii("com.sun.star.configuration.ConfigurationAccess"),
        aArgs);
    // cout << "---------------------------------------------------------------\n Configuration Read/Write Access created !\n---------------------------------------------------------------" << endl;

    xChangesBatch = Reference< XChangesBatch >(xIFace, UNO_QUERY);

    Sequence<OUString> aSeq = _xCfgProvider->getAvailableServiceNames();

    OString sPath;
    sPath <<= _sPath;

    OString aFilename = "c:\\temp\\fileout_";
    aFilename += sPath;
    aFilename += OString::valueOf(_nCount);
    aFilename += ".txt";
    test_read_access(_xCfgProvider, xIFace, aFilename);
}


// -----------------------------------------------------------------------------
// ----------------------------------- Main 2 -----------------------------------
// -----------------------------------------------------------------------------

int requestTest( int argc, char * argv[] )
{
    TimeValue aTimeout;
    aTimeout.Seconds = 5;
    aTimeout.Nanosec = 0;

    sal_Int32 nSizeISubtree = sizeof(ISubtree);
    sal_Int32 nSizeINode = sizeof(INode);
    sal_Int32 nSizeIValueNode = sizeof(ValueNode);
    {
        loadDefaults();

        OUString const sServiceRegistry = OUString::createFromAscii( argc > 1 ? argv[1] : "applicat.rdb" );
        Reference< XMultiServiceFactory > xORB;
        try
        {
            xORB = createRegistryServiceFactory( sServiceRegistry, ::rtl::OUString() );
            if (!xORB.is())
            {
                ::flush(cout);
                cerr << "Could not create the service factory !\n\n";
                return 0;
            }
        }
        catch (uno::Exception &e)
        {
            cout << "Error: can't get ServiceFactory" << sServiceRegistry << endl;
        }

        rtl::OUString sSharePath = enterValue("share path: ", s_pSourcePath, false);
        cout << endl;
        rtl::OUString sUserPath =   enterValue("user path : ", s_pUpdatePath, false);
        cout << endl;

        START_MEMORYMEASURE( aMemoryInfo );

        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "initialisierung" );

        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "initialisierung weil beim ersten mal falsch!" );


        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "request subtrees" );

        OTreeLoad aTreeLoad(xORB, sSharePath, sUserPath);
        sal_Int32 nIdx = 0;
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Office.Common"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Setup/CJK/Enable"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Setup/Office/Modules"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Inet"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Office.Views"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Setup/Product"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.ucb.Configuration"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.ucb.Store/ContentProperties"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Office.TypeDetection"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Office.Writer"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Office.WriterWeb"));
        aTreeLoad.requestSubtree(ASCII("org.openoffice.Office.Linguistic"));

        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "release subtrees" );

        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Office.Common"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Setup/CJK/Enable"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Setup/Office/Modules"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Inet"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Office.Views"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Setup/Product"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.ucb.Configuration"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.ucb.Store/ContentProperties"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Office.TypeDetection"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Office.Writer"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Office.WriterWeb"));
        aTreeLoad.releaseSubtree(ASCII("org.openoffice.Office.Linguistic"));

        MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "ich habe fertig." );
        LOG_MEMORYMEASURE( "FirstTest_of_memusage", "Values of memory access for standard filters.", aMemoryInfo );

        volatile int dummy = 0;
    }
    return 0;
}

// -----------------------------------------------------------------------------
// --------------------------------- Trust Test ---------------------------------
// -----------------------------------------------------------------------------

int trust( int argc, char * argv[] )
{
    rtl::OUString const sServiceRegistry = OUString::createFromAscii( argc > 1 ? argv[1] : "applicat.rdb" );
    uno::Reference< lang::XMultiServiceFactory > xORB;
    try
    {
        xORB = createRegistryServiceFactory( sServiceRegistry, ::rtl::OUString() );
        if (!xORB.is())
        {
            ::flush(cout);
            cerr << "Could not create the service factory !\n\n";
            return 0;
        }
    }
    catch (uno::Exception &e)
    {
        cout << "Error: can't get ServiceFactory" << sServiceRegistry << endl;
    }
    START_MEMORYMEASURE( aMemoryInfo );

    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "initialisierung" );
    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "initialisierung weil beim ersten mal falsch!" );

    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "get 8 MB" );

    char* pChar = new char[8 * 1024 * 1024];

    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "free 8 MB" );

    delete [] pChar;

    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "get 8 MB as 1024 pieces" );

    std::vector<char*> aMemHolder;

    for (sal_Int32 i=0;i<1024;i++)
    {
        pChar = new char[8192];
        aMemHolder.push_back(pChar);
    }

    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "free pieces" );

    pChar = NULL;
    for (std::vector<char*>::iterator it = aMemHolder.begin();
         it != aMemHolder.end();
         it++)
    {
        pChar = *it;
        delete []pChar;
    }

    MAKE_MEMORY_SNAPSHOT( aMemoryInfo, "ich habe fertig." );
    LOG_MEMORYMEASURE( "FirstTest_of_memusage", "Values of memory access for standard filters.", aMemoryInfo );

    volatile int dummy = 0;
}

} // namespace configmgr


// -----------------------------------------------------------------------------
// ------------------------------------ Main ------------------------------------
// -----------------------------------------------------------------------------

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    return configmgr::main(argc, argv); // API
//  return configmgr::requestTest(argc, argv);
//  return configmgr::trust(argc, argv);
}

