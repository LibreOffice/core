/*************************************************************************
 *
 *  $RCSfile: cfgadmin.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-17 08:30:26 $
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

#ifndef _COM_SUN_STAR_BEANS_XHIERARCHICALPROPERTYSET_HPP_
#include <com/sun/star/beans/XHierarchicalPropertySet.hpp>
#endif

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
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
// -----------------------------------------------------------------------------
void write(Reference< XChild >& xChild)
{
        if (xChild.is())
            cout << "\n[ P ] -\tParent";
        else
            cout << "BUG: Parent not available (no XChild)";
        cout << endl;
}

// -----------------------------------------------------------------------------
void displayTree(Reference< XNameAccess > xIFace){

    write(Reference< XNameAccess >(xIFace));
}


// -----------------------------------------------------------------------------
void displayGroups(Reference< XNameAccess > xGroupAccess)
{

    cout << "Currently available groups    !\n---------------------------------------------------------------" << endl;
    write(xGroupAccess);
}

// -----------------------------------------------------------------------------
OUString insertGroup(Reference< XNameAccess > xGroupAccess)
{
    OUString sGroup =   enterValue("    Enter a new group to create: ", "", true);

    Reference< XSingleServiceFactory > xFactory(xGroupAccess, UNO_QUERY);
    Reference< XNameAccess > xNewGroup(xFactory->createInstance(), UNO_QUERY);

    cout << "Group data:                !\n---------------------------------------------------------------" << endl;
    write(xNewGroup);
    Any aGroup;
    aGroup <<= xNewGroup;
    Reference< XNameContainer >(xGroupAccess, UNO_QUERY)->insertByName(sGroup, aGroup);
    return sGroup;
}

// -----------------------------------------------------------------------------
void deleteGroup(Reference< XNameAccess > xGroupAccess, OUString sGroup)
{
    if (!sGroup.getLength())
        sGroup =   enterValue("    Enter a group to delete: ", "", true);
    cout << "deleting group          !\n---------------------------------------------------------------" << endl;
    Reference< XNameContainer >(xGroupAccess, UNO_QUERY)->removeByName(sGroup);
}

// -----------------------------------------------------------------------------
void displayUsers(Reference< XNameAccess > xUserAccess)
{
    cout << "Currently available users    !\n---------------------------------------------------------------" << endl;
    write(xUserAccess);
}

// -----------------------------------------------------------------------------
OUString insertUser(Reference< XNameAccess > xUserAccess, OUString aGroup)
{
    OUString sUser =   enterValue("    Enter a new User to create: ", "", true);
    Reference< XSingleServiceFactory > xFactory(xUserAccess, UNO_QUERY);

    Sequence< Any > aArgs(1);
    aArgs[0] <<= configmgr::createPropertyValue(ASCII("group"), aGroup);
    Reference< XNameAccess > xNewUser(xFactory->createInstanceWithArguments(aArgs), UNO_QUERY);

    cout << "User data:             !\n---------------------------------------------------------------" << endl;
    write(xNewUser);

    Any aValue;
    aValue <<= ASCII("MyCompany");

    // now do some updates for the user
    Reference< XHierarchicalPropertySet > xUpdate(xNewUser, UNO_QUERY);
    xUpdate->setHierarchicalPropertyValue(ASCII("Data/Company"),aValue);

    Any aUser;
    aUser <<= xNewUser;
    Reference< XNameContainer >(xUserAccess, UNO_QUERY)->insertByName(sUser, aUser);

    return sUser;
}

// -----------------------------------------------------------------------------
void deleteUser(Reference< XNameAccess > xUserAccess, OUString sUser)
{
    if (!sUser.getLength())
        sUser =   enterValue("    Enter a User to delete: ", "", true);
    cout << "deleting User          !\n---------------------------------------------------------------" << endl;
    Reference< XNameContainer >(xUserAccess, UNO_QUERY)->removeByName(sUser);
}

// -----------------------------------------------------------------------------
Reference< XNameAccess > beginChanges(Reference< XMultiServiceFactory > xFactory, OUString sPath, OUString& sUser)
{
    if (!sUser.getLength())
        sUser =   enterValue("    Enter a User: ", "", true);

    Sequence< Any > aArgs(2);
    aArgs[0] <<= configmgr::createPropertyValue(ASCII("user"), sUser);
    aArgs[1] <<= configmgr::createPropertyValue(ASCII("nodepath"),sPath);

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
            flush(cout);
            cerr << "Could not create the service factory !\n\n";
            return 1;
        }
        cout << "Service factory created !\n---------------------------------------------------------------" << endl;

        Sequence< Any > aCPArgs = createSequence(OUString::createFromAscii("Administrator"), OUString());

        Reference< XMultiServiceFactory > xCfgProvider(
            xORB->createInstanceWithArguments(
                ::rtl::OUString::createFromAscii("com.sun.star.configuration.AdministrationProvider"),
                aCPArgs),
            UNO_QUERY);

        if (!xCfgProvider.is())
        {
            flush(cout);
            cerr << "Could not create the configuration provider !\n\n";
            return 3;
        }

        cout << "Configuration Provider created !\n---------------------------------------------------------------" << endl;

        Reference< XNameAccess > xUpdateAccess;
        Reference< XNameAccess > xGroupAccess(xCfgProvider->createInstance(OUString::createFromAscii("com.sun.star.configuration.GroupAccess")),UNO_QUERY);
        Reference< XNameAccess > xUserAccess(xCfgProvider->createInstance(OUString::createFromAscii("com.sun.star.configuration.UserAccess")),UNO_QUERY);

        displayGroups(xGroupAccess);
        displayUsers(xUserAccess);

// create a group
        OUString sGroupName = insertGroup(xGroupAccess);
// create a user
        OUString sUserName;
        sUserName = insertUser(xUserAccess, sGroupName);

// now do updates for the user
        xUpdateAccess = beginChanges(xCfgProvider, OUString::createFromAscii("org.openoffice.Inet"), sUserName);

        update(xUpdateAccess, OUString::createFromAscii("Proxy/FTP/Port"), sal_Int32(12));
        update(xUpdateAccess, OUString::createFromAscii("Proxy/FTP/Name"), OUString::createFromAscii("demo"));
        update(xUpdateAccess, OUString::createFromAscii("DNS/IP_Address"), OUString::createFromAscii("demo1"));

        xUpdateAccess = beginChanges(xCfgProvider, OUString::createFromAscii("org.openoffice.Office.Common"), sUserName);
        update(xUpdateAccess, OUString::createFromAscii("_3D_Engine/Dithering"), sal_Bool(sal_False));
        commitChanges(xUpdateAccess);

// now do updates with inserting and removing of nodes

        xUpdateAccess = beginChanges(xCfgProvider, OUString::createFromAscii("org.openoffice.Security/MountPoints"), sUserName);
        displayTree(xUpdateAccess);

        Reference< XHierarchicalPropertySet > xTree = insertTree(xUpdateAccess, OUString());
        update(xUpdateAccess, OUString::createFromAscii("InstallationDirectory/Directory"), OUString::createFromAscii("Test1"));
        removeTree(xUpdateAccess, OUString());
        commitChanges(xUpdateAccess);


/*      deleteUser(xUserAccess, sUserName);
        deleteGroup(xGroupAccess, sGroupName);      */

        displayGroups(xGroupAccess);
        displayUsers(xUserAccess);
    }
    catch (Exception& e)
    {
        flush(cout);
        cerr << "Caught exception: " << e.Message << endl;
        return 1;
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




