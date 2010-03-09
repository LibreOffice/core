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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#define _PRIVATE_TEST_

#include <iostream>
using namespace std;

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>


#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <osl/time.h>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/uno/Any.h>

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
    OString aTxt = input(_aDefault, _bIsAPassword ? '*' : 0);

    OUString sValue = OUString::createFromAscii(aTxt);
    return sValue;
}

// -----------------------------------------------------------------------------
Reference< XNameAccess > beginChanges(Reference< XMultiServiceFactory > xFactory, OUString sPath)
{
    Sequence< Any > aArgs(1);
    aArgs[0] <<= configmgr::createPropertyValue(ASCII("nodepath"),sPath);

    cout << "starting update for node:" << sPath << endl;

    Reference< XNameAccess > xTree(xFactory->createInstanceWithArguments(OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess"),
            aArgs), UNO_QUERY);

    return xTree;
}

template <class Type>
// -----------------------------------------------------------------------------
void update(Reference< XInterface > xIFace, OUString sRelPath, Type sValue)
{
    Reference< XHierarchicalPropertySet > xTree(xIFace, UNO_QUERY);
    Any aValue;
    aValue <<= sValue;

    cout << "updating node:" << sRelPath << endl;
    xTree->setHierarchicalPropertyValue(sRelPath, aValue);
}

// -----------------------------------------------------------------------------
Reference< XHierarchicalPropertySet > insertTree(Reference< XInterface > xIFace, OUString aName)
{
    if (!aName.getLength())
        aName =   enterValue("/nEnter a Tree to insert: ", "", false);

    Reference< XSingleServiceFactory > xFactory(xIFace, UNO_QUERY);
    Reference< XHierarchicalPropertySet > xNewElement(xFactory->createInstance(), UNO_QUERY);

    cout << "inserting new tree element:" << aName << endl;

    Any aTree;
    aTree <<= xNewElement;
    Reference< XNameContainer >(xFactory, UNO_QUERY)->insertByName(aName, aTree);

    return xNewElement;
}

// -----------------------------------------------------------------------------
void removeTree(Reference< XInterface > xIFace, OUString aName)
{
    if (!aName.getLength())
        aName =   enterValue("/nEnter a Tree to remove: ", "", false);

    cout << "removing new tree element:" << aName << endl;

    Reference< XNameContainer >(xIFace, UNO_QUERY)->removeByName(aName);
}

// -----------------------------------------------------------------------------
void commitChanges(Reference< XInterface > xIFace)
{
    cout << "committing changes:" << endl;

    Reference< XChangesBatch > xChangesBatch(xIFace, UNO_QUERY);
    xChangesBatch->commitChanges();
}

// -----------------------------------------------------------------------------
void displayTree(Reference< XNameAccess > xIFace, sal_Int32 nLevel)
{
    const char* pTab = "  ";
    Sequence<OUString> aNames( xIFace->getElementNames() );
    for (int i = 0; i < aNames.getLength(); ++i)
    {
        Any aElement = xIFace->getByName(aNames[i]);
        Reference< XNameAccess > xAccess;
        Reference< XSingleServiceFactory > xFactory;
        aElement >>= xFactory;
        aElement >>= xAccess;

        cout << endl;
        for (int j = 0; j < nLevel; j++)
            cout << "  ";

        if (xAccess.is())
        {
            OUString sType;
            if (xFactory.is())
                sType = OUString::createFromAscii(" type = 'set' " );

            cout << "<" << aNames[i] << sType << ">";
            displayTree(xAccess, nLevel + 1);
            cout << endl << "</" << aNames[i] << ">";
        }
        else
            cout << "<" << aNames[i] << "/>";

    }
    ::flush(cout);
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

        /*OUString sServerType = enterValue("Enter Servertype: ", "remote", false);
        cout << endl;*/


        rtl::OUString sFilePath;
        rtl::OUString sPort;
        rtl::OUString sUser;

        sUser =   enterValue("Enter User: ", "user1", false);
        cout << endl;

        OUString sPasswd;// = enterValue("Enter Password: ", "", true);
        cout << endl;

        aCPArgs = createSequence(sUser, sPasswd);

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

        Reference< XNameAccess > xUpdateAccess;

// now do updates for the user
        xUpdateAccess = beginChanges(xCfgProvider, OUString::createFromAscii("org.openoffice.Security"));
        displayTree(xUpdateAccess, 0);

        update(xUpdateAccess, OUString::createFromAscii("_3D_Engine/Dithering"), sal_Bool(sal_False));

/*      xUpdateAccess = beginChanges(xCfgProvider, OUString::createFromAscii("org.openoffice.Inet"));

        update(xUpdateAccess, OUString::createFromAscii("Proxy/FTP/Port"), sal_Int32(11));
        update(xUpdateAccess, OUString::createFromAscii("Proxy/FTP/Name"), OUString::createFromAscii("Test3"));
        update(xUpdateAccess, OUString::createFromAscii("DNS/IP_Address"), OUString::createFromAscii("Test4"));
*/
        commitChanges(xUpdateAccess);

// now do updates with inserting and removing of nodes

        xUpdateAccess = beginChanges(xCfgProvider, OUString::createFromAscii("org.openoffice.Security/MountPoints"));
        displayTree(xUpdateAccess, 0);

        Reference< XHierarchicalPropertySet > xTree = insertTree(xUpdateAccess, OUString());
        update(xUpdateAccess, OUString::createFromAscii("InstallationDirectory/Directory"), OUString::createFromAscii("Test1"));
        removeTree(xUpdateAccess, OUString());
        commitChanges(xUpdateAccess);
    }
    catch (Exception& e)
    {
        ::flush(cout);
        cerr << "Caught exception: " << e.Message << endl;
    }
/*
    catch (...)
    {
        ::flush(cout);
        cerr << "BUG: Caught UNKNOWN exception (?) " << endl;
    }
*/
    return 0;
}

