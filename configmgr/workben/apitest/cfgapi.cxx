/*************************************************************************
 *
 *  $RCSfile: cfgapi.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: jb $ $Date: 2002-10-17 11:51:53 $
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
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>

#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/beans/XExactName.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif


#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

#include <cppuhelper/bootstrap.hxx>

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
bool test_cfg_access(Reference< XInterface >& xIface, Reference< XMultiServiceFactory > &xMSF);
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
static const sal_Char*  const   s_pProviderService  =   "com.sun.star.configuration.ConfigurationProvider";
static const sal_Char*  const   s_pRootNode         =   "org.openoffice.Office.Common";
static const sal_Char*  const   s_pLocale           =   "en-US";

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

    try
    {
        Reference< XComponentContext > xUnoContext = defaultBootstrap_InitialComponentContext();
        if (!xUnoContext.is())
        {
            cout.flush();
            cerr << "Could not create the UNO context !\n\n";
            return 1;
        }

        Reference< XMultiServiceFactory > xORB( xUnoContext->getServiceManager(), UNO_QUERY );
        if (!xORB.is())
        {
            cout.flush();
            cerr << "Could not create the service factory !\n\n";
            return 2;
        }
        cout << "Service factory created !\n---------------------------------------------------------------" << endl;


        OUString sProviderService = enterValue("Use provider service: ", s_pProviderService, false);
        cout << endl;

        Reference< XMultiServiceFactory > xCfgProvider( xORB->createInstance(sProviderService), UNO_QUERY);
        if (!xCfgProvider.is())
        {
            cout.flush();
            cerr << "Could not create the configuration provider !\n\n";
            return 3;
        }




        char aPath[300] =           "/";
        int nStart = sizeof(    "/" ) - 1;

        cout << "---------------------------------------------------------------\n Configuration Provider created !";

        bool bQuit = true;
        do
        {
            cout << "\n---------------------------------------------------------------" << endl;

            Sequence< Any > aArgs(2);

            OUString sPath =    enterValue("nodepath: ", s_pRootNode, false);
            cout << endl;

            aArgs[0] <<= configmgr::createPropertyValue(ASCII("nodepath"), sPath);

            OUString sLocale =  enterValue("locale  : ", s_pLocale, false);
            cout << endl;

            aArgs[1] <<= configmgr::createPropertyValue(ASCII("locale"), sLocale);

            Reference< XInterface > xIFace = xCfgProvider->createInstanceWithArguments(
                OUString::createFromAscii("com.sun.star.configuration.ConfigurationUpdateAccess"),
                aArgs);
            cout << "---------------------------------------------------------------\n Configuration Read/Write Access created !\n---------------------------------------------------------------" << endl;

            xChangesBatch = Reference< XChangesBatch >(xIFace, UNO_QUERY);

            Sequence<OUString> aSeq = xCfgProvider->getAvailableServiceNames();
            showSequence(aSeq);

            bQuit = test_cfg_access(xIFace, xCfgProvider);
        }
        while (!bQuit);
    }
    catch (Exception& e)
    {
        cout.flush();
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
            cout << "\n ROOT -\tParent not available ";
        cout << endl;
}
///////////////////////////////////////////////////////////////////////////////////////////

bool ask(Reference< XInterface >& xIface, Reference<XMultiServiceFactory> &, bool&);

bool test_cfg_access(Reference< XInterface >& xIface, Reference< XMultiServiceFactory > &xMSF)
{
    bool bQuit = true;

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
    while (ask(xIface, xMSF, bQuit));

    return bQuit;
}

bool ask(Reference< XInterface >& xIface, Reference< XMultiServiceFactory > &xMSF, bool& rbQuit)
{
    cout << "\n[ S ] -> <SetValue> ";
    cout << "\n[ Z ] -> <SetToNull> ";
    cout << "\n[ D ] -> <SetToDefault> ";
    cout << "\n[ I ] -> <InsertElement> ";
    cout << "\n[ R ] -> <RemoveElement> ";
    cout << "\n[ N ] -> <New Access>";
    cout << "\n[ Q ] -> <Quit>";
    cout << endl;

    cout << "\n:> " << flush;
    char buf[200] = "";
    try
    {

        enum { nop, show, insert, replace, remove, reset, nullify };

        int eToDo = nop;
        bool bNeedValue = false;

        if (cin.getline(buf,sizeof buf))
        {
            Reference< XInterface > xNext = xIface;
            if ((buf[0] == 'q' || buf[0] == 'Q') && (0 == buf[1]))
            {
                rbQuit = true;
                return false;
            }
            else if ((buf[0] == 'n' || buf[0] == 'N') && (0 == buf[1]))
            {
                rbQuit = false;
                return false;
            }
            else if (buf[0] == 0)
            {
                return true;
            }
            else if( (buf[0] == 's' || buf[0] == 'S') && (0 == buf[1]))
            {
                // Replace a Value
                Reference< XNameReplace > xAccess(xIface, UNO_QUERY);

                if (xAccess.is())
                {
                    cout << "Select a Value" << endl;
                    if (cin.getline(buf,sizeof buf))
                        eToDo = replace;

                    bNeedValue = true;
                }
            }
            else if( (buf[0] == 'z' || buf[0] == 'Z') && (0 == buf[1]))
            {
                // Replace a Value
                Reference< XNameReplace > xAccess(xIface, UNO_QUERY);

                if (xAccess.is())
                {
                    cout << "Select a Value" << endl;
                    if (cin.getline(buf,sizeof buf))
                        eToDo = nullify;
                }
            }
            else if( (buf[0] == 'd' || buf[0] == 'D') && (0 == buf[1]))
            {
                // Replace a Value
                Reference< XPropertyState > xAccess(xIface, UNO_QUERY);

                if (xAccess.is())
                {
                    cout << "Select a Value" << endl;
                    if (cin.getline(buf,sizeof buf))
                        eToDo = reset;
                }
            }
            else if( (buf[0] == 'r' || buf[0] == 'R') && (0 == buf[1]))
            {
                // Insert an Element
                Reference< XNameContainer > xAccess(xIface, UNO_QUERY);

                if (xAccess.is())
                {
                    cout << "Select an Element" << endl;
                    if (cin.getline(buf,sizeof buf))
                        eToDo = remove;
                }
            }
            else if( (buf[0] == 'i' || buf[0] == 'I') && (0 == buf[1]))
            {
                // Insert an Element
                Reference< XNameContainer > xAccess(xIface, UNO_QUERY);

                if (xAccess.is())
                {
                    cout << "Enter a New Element-Name" << endl;
                    if (cin.getline(buf,sizeof buf))
                        eToDo = insert;

                    bNeedValue = ! Reference< XSingleServiceFactory >::query(xAccess).is();
                }
            }
            else if ((buf[0] == 'p' || buf[0] == 'P') && (0 == buf[1]))
            {
                Reference< XChild > xChild(xIface, UNO_QUERY);
                if (xChild.is())
                    xNext = xChild->getParent();
                eToDo = nop;
            }
            else
                eToDo = show;

            if (nop != eToDo)
            {
                Reference< XNameAccess > xAccess(xIface, UNO_QUERY);
                Reference< XHierarchicalNameAccess > xDeepAccess(xIface, UNO_QUERY);

                OUString aName;
                OUString aInput = OUString::createFromAscii(buf);
                bool bNested = false;

                if (insert == eToDo)
                {
                    aName = aInput;
                }
                else if (xAccess.is() || xDeepAccess.is())
                {
                    Reference< XExactName > xExactName(xIface, UNO_QUERY);
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
                        bNested = true;
                    }
                    else if ('0' <= buf[0] && buf[0] <= '9' && xAccess.is())
                    {
                        int n = (atoi(buf));
                        Sequence<OUString> aNames = xAccess->getElementNames();
                        if (0 <= n && n < aNames.getLength())
                            aName = aNames[n];
                    }
                }
                if (aName.getLength())
                {
                    bool bValueOk = true;

                    Any aElement;
                    if (insert != eToDo)
                    {
                        aElement =  bNested      ? xDeepAccess->getByHierarchicalName(aName) :
                                    xAccess.is() ? xAccess->    getByName(aName)             : Any();

                        switch (aElement.getValueTypeClass() )
                        {
                        case TypeClass_INTERFACE:
                            bNeedValue = false;
                            cout << "ELEMENT '" << aName << "' is an INNER NODE " << endl;
                            break;

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
                                if (bValueOk = !!(aElement >>= aValue))
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
                                if (bValueOk = !!(aElement >>= aValue))
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
                                if (bValueOk = !!(aElement >>= aValue))
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
                                if (bValueOk = !!(aElement >>= aValue))
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
                                if (bValueOk = !!(aElement >>= aValue))
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
                            bValueOk = false;
                            cout << "Error: ELEMENT '" << aName << "' is of unknown or unrecognized type" << endl;
                            break;
                        }
                    }
                    else
                        bValueOk = true;

                    Any aValue;
                    if (bNeedValue)
                    {
                        if (aElement.getValueTypeClass() == TypeClass_BOOLEAN )
                        {
                            cout << "Set Value (boolean) to :";
                            cout.flush();
                            cin.getline(buf,sizeof buf);
                            OUString aInput = OUString::createFromAscii(buf);

                            sal_Bool bBoolValue = false;
                            if (aInput.equalsIgnoreAsciiCase(ASCII("true")))
                                bBoolValue = true;

                            else if (!aInput.equalsIgnoreAsciiCase(ASCII("false")))
                                cout << "Warning: Not a valid bool value - setting to false" << endl;

                            aValue <<= bBoolValue;
                        }
                        else
                        {
                            if ( aElement.getValueTypeClass() == TypeClass_VOID )
                                cout << "Warning: Cannot determine value type (value is NULL)" << endl;

                            else if ( aElement.getValueTypeClass() != TypeClass_STRING)
                                cout << "Warning: No explict support for value type found" << endl;

                            cout << "Set value to : ";
                            cout.flush();
                            cin.getline(buf,sizeof buf);

                            aValue <<= buf;
                        }
                    }
                    else if (insert == eToDo || replace == eToDo)
                    {
                        Reference<XSingleServiceFactory> xFactory(xIface, UNO_QUERY);

                        OSL_ASSERT(xFactory.is());
                        if (xFactory.is())
                        {
                            xNext = xFactory->createInstance();
                            aValue <<= xNext;
                        }
                    }

                    switch (eToDo)
                    {
                    case nullify:
                        OSL_ASSERT(!aValue.hasValue()); // nullify is replace with NULL value
                        // fall thru

                    case replace:
                        {
                            Reference< XNameReplace > xNameReplace(xIface, UNO_QUERY);
                            if (xNameReplace.is())
                                xNameReplace->replaceByName(aName, aValue);
                        } break;

                    case reset:
                        {
                            Reference< XPropertyState > xReset(xIface, UNO_QUERY);
                            OSL_ASSERT(xReset.is());
                            if (xReset.is())
                            {
                                xReset->setPropertyToDefault(aName);
                            }
                        } break;

                    case insert:
                        {
                            Reference< XNameContainer> xNameContainer(xIface, UNO_QUERY);
                            OSL_ASSERT(xNameContainer.is());
                            if (xNameContainer.is())
                            {
                                xNameContainer->insertByName(aName, aValue);
                                aElement = aValue;
                            }
                        } break;

                    case remove:
                        {
                            Reference< XNameContainer> xNameContainer(xIface, UNO_QUERY);
                            OSL_ASSERT(xNameContainer.is());
                            if (xNameContainer.is())
                            {
                                xNameContainer->removeByName(aName);
                                aElement = aValue;
                            }
                        } break;

                    case show:
                        {
                            if (aElement >>= xNext)
                                cout << "Got an Interface for '" << aName << "'" << endl;

                            else if (!aElement.hasValue())
                                cout << "Error: Cannot get a Value or Interface for '" << aName << "'" << endl;
                        } break;
                    }
                    if (show != eToDo)
                        commit();

                    if (bNeedValue)
                    {
                        prompt_and_wait();
                        return bValueOk;
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
            return false;
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
