/*************************************************************************
 *
 *  $RCSfile: implreg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: jsc $ $Date: 2001-03-16 16:30:34 $
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

#include <list>

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX
#include <cppuhelper/implbase3.hxx>
#endif

#include <uno/mapping.hxx>
#include <osl/thread.h>


#include <rtl/ustring.hxx>

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#if defined(SAL_W32) || defined(SAL_OS2)
#include <io.h>
#endif


using namespace com::sun::star::uno;
using namespace com::sun::star::loader;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace cppu;
using namespace rtl;

namespace stoc_impreg
{

#define IMPLEMENTATION_NAME "com.sun.star.comp.stoc.ImplementationRegistration"
#define SERVICE_NAME        "com.sun.star.registry.ImplementationRegistration"

//*************************************************************************
//  static getTempName()
//
// Shouldn't this whole routine be rewritten ???? (jbu)

static OUString getTempName()
{
    static OUString TMP(RTL_CONSTASCII_USTRINGPARAM("TMP"));
    static OUString TEMP(RTL_CONSTASCII_USTRINGPARAM("TEMP"));

    OUString    uTmpPath;
    OString     tmpPath;
    sal_Char    tmpPattern[512];
    sal_Char    *pTmpName = NULL;

    if ( osl_getEnvironment(TMP.pData, &uTmpPath.pData) != osl_Process_E_None )
    {
        if ( osl_getEnvironment(TEMP.pData, &uTmpPath.pData) != osl_Process_E_None )
        {
#if defined(SAL_W32) || defined(SAL_OS2)
            tmpPath = OString("c:\\temp");
#else
            tmpPath = OString("/tmp");
#endif
        }
    }

    if (uTmpPath.getLength())
    {
        tmpPath = OUStringToOString(uTmpPath, osl_getThreadTextEncoding());
    }

#if defined(WIN32) || defined(WNT)
    strcpy(tmpPattern, tmpPath.getStr());
    strcat(tmpPattern, "\\reg_XXXXXX");
    pTmpName = mktemp(tmpPattern);
#endif

#ifdef __OS2__
    strcpy(tmpPattern, tempnam(NULL, "reg_"));
    pTmpName = tmpPattern;
#endif

#ifdef UNX
    strcpy(tmpPattern, tmpPath.getStr());
    strcat(tmpPattern, "/reg_XXXXXX");
    pTmpName = mktemp(tmpPattern);
#endif

    return OStringToOUString(pTmpName, osl_getThreadTextEncoding());
}

//*************************************************************************
//  static deleteAllLinkReferences()
//
static void deleteAllLinkReferences(const Reference < XSimpleRegistry >& xReg,
                                    const Reference < XRegistryKey >& xSource)
{
    try
    {
        Reference < XRegistryKey > xKey = xSource->openKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/REGISTRY_LINKS") ) );

        if (xKey.is() && (xKey->getValueType() == RegistryValueType_ASCIILIST))
        {
            Sequence<OUString> linkNames = xKey->getAsciiListValue();

            if (linkNames.getLength())
            {
                const OUString* pLinkNames = linkNames.getConstArray();

                OUString            aLinkName;
                OUString            aLinkParent;
                Reference < XRegistryKey >  xLinkParent;
                const sal_Unicode*  pTmpName = NULL;
                const sal_Unicode*  pShortName = NULL;
                sal_Int32           sEnd = 0;

                for (sal_Int32 i = 0; i < linkNames.getLength(); i++)
                {
                    aLinkName = pLinkNames[i];

                    pTmpName = aLinkName.getStr();

                    if (pTmpName[0] != L'/')
                        continue;

                    sal_Int32 nIndex = rtl_ustr_indexOfChar( pTmpName, '%' );
                    if ( nIndex == -1 )
                        pShortName = 0;
                    else
                        pShortName = pTmpName+nIndex;

                    while (pShortName && pShortName[1] == L'%')
                    {
                        nIndex = rtl_ustr_indexOfChar( pShortName+2, '%' );
                        if ( nIndex == -1 )
                            pShortName = 0;
                        else
                            pShortName += nIndex+2;
                    }

                    if (pShortName)
                    {
                        aLinkName = aLinkName.copy(0, pShortName - pTmpName);
                    }

                    xReg->getRootKey()->deleteLink(aLinkName);

                    sEnd = rtl_ustr_lastIndexOfChar( aLinkName.getStr(), '/' );

                    aLinkParent = aLinkName.copy(0, sEnd);

                    while(aLinkParent.len())
                    {
                        xLinkParent = xReg->getRootKey()->openKey(aLinkParent);

                        if (xLinkParent.is() && (xLinkParent->getKeyNames().getLength() == 0))
                        {
                            aLinkName = aLinkParent;

                            xReg->getRootKey()->deleteKey(aLinkParent);

                            sEnd = rtl_ustr_lastIndexOfChar( aLinkName.getStr(), '/' );

                            aLinkParent = aLinkName.copy(0, sEnd);
                        } else
                        {
                            break;
                        }
                    }
                }
            }
        }
    }
    catch(InvalidRegistryException&)
    {
    }
}

//*************************************************************************
//  static prepareLink
//
static void prepareLink( const Reference < XSimpleRegistry > & xDest,
                         const Reference < XRegistryKey > & xSource,
                         const OUString& link)
{
    try
    {
        OUString linkRefName = xSource->getKeyName();
        OUString linkName(link);
        sal_Bool    isRelativ = sal_False;

        const sal_Unicode*  pTmpName = link.getStr();
        const sal_Unicode*  pShortName;
        sal_Int32           nIndex = rtl_ustr_indexOfChar( pTmpName, '%' );
        if ( nIndex == -1 )
            pShortName = 0;
        else
            pShortName = pTmpName+nIndex;

        if (pTmpName[0] != L'/')
            isRelativ = sal_True;

        while (pShortName && pShortName[1] == L'%')
        {
            nIndex = rtl_ustr_indexOfChar( pShortName+2, '%' );
            if ( nIndex == -1 )
                pShortName = 0;
            else
                pShortName += nIndex+2;
        }

        if (pShortName)
        {
            linkRefName = linkRefName + link.copy(pShortName - pTmpName + 1);
            linkName = link.copy(0, pShortName - pTmpName);
        }

        if (isRelativ)
            xSource->createLink(linkName, linkRefName);
        else
            xDest->getRootKey()->createLink(linkName, linkRefName);
    }
    catch(InvalidRegistryException&)
    {
    }
}

//*************************************************************************
//  static searchImplForLink
//
static OUString searchImplForLink(const Reference < XRegistryKey > & xRootKey,
                                 const OUString& linkName,
                                 const OUString& implName)
{
    OUString ret;

    try
    {
        Reference < XRegistryKey > xKey = xRootKey->openKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/IMPLEMENTATIONS") ) );

        if (xKey.is())
        {
            Sequence< Reference < XRegistryKey > > subKeys = xKey->openKeys();

            const Reference < XRegistryKey > * pSubKeys = subKeys.getConstArray();
            Reference < XRegistryKey > xImplKey;

            for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
            {
                xImplKey = pSubKeys[i];

                try
                {
                    if (xImplKey->getKeyType( OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO") ) + linkName) == RegistryKeyType_LINK)
                    {
                        OUString oldImplName = xImplKey->getKeyName().copy(strlen("/IMPLEMENTATIONS/"));
                        if (implName != oldImplName)
                        {
                            ret = oldImplName;
                            break;
                        }
                    }
                }
                catch(InvalidRegistryException&)
                {
                }
            }
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    return ret;
}

//*************************************************************************
//  static searchLinkTargetForImpl
//
static OUString searchLinkTargetForImpl(const Reference < XRegistryKey >& xRootKey,
                                        const OUString& linkName,
                                        const OUString& implName)
{
    OUString ret;

    try
    {
        Reference < XRegistryKey > xKey = xRootKey->openKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/IMPLEMENTATIONS") ) );

        if (xKey.is())
        {
            Sequence< Reference < XRegistryKey > > subKeys = xKey->openKeys();

            const Reference < XRegistryKey >* pSubKeys = subKeys.getConstArray();
            Reference < XRegistryKey > xImplKey;

            for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
            {
                xImplKey = pSubKeys[i];

                OUString tmpImplName = xImplKey->getKeyName().copy(strlen("/IMPLEMENTATIONS/"));
                if (tmpImplName == implName &&
                    xImplKey->getKeyType(OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO") ) + linkName) == RegistryKeyType_LINK)
                {
                    return xImplKey->getLinkTarget(OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO") ) + linkName);
                }
            }
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    return ret;
}

//*************************************************************************
//  static createUniqueSubEntry
//
static void createUniqueSubEntry(const Reference < XRegistryKey > & xSuperKey,
                                 const OUString& value)
{
    if (xSuperKey.is())
    {
        try
        {
            if (xSuperKey->getValueType() == RegistryValueType_ASCIILIST)
            {
                sal_Int32 entryNum = 1;
                sal_Int32 length = 0;
                sal_Bool bReady = sal_False;

                Sequence<OUString> implEntries = xSuperKey->getAsciiListValue();
                length = implEntries.getLength();

                for (sal_Int32 i = 0; !bReady && (i < length); i++)
                {
                    bReady = (implEntries.getConstArray()[i] == value);
                }

                if (bReady)
                {
                    Sequence<OUString> implEntriesNew(length);
                    implEntriesNew.getArray()[0] = value;

                    for (sal_Int32 i=0, j=1; i < length; i++)
                    {
                        if (implEntries.getConstArray()[i] != value)
                            implEntriesNew.getArray()[j++] = implEntries.getConstArray()[i];
                    }
                    xSuperKey->setAsciiListValue(implEntriesNew);
                } else
                {
                    Sequence<OUString> implEntriesNew(length+1);
                    implEntriesNew.getArray()[0] = value;

                    for (sal_Int32 i = 0; i < length; i++)
                    {
                        implEntriesNew.getArray()[i+1] = implEntries.getConstArray()[i];
                    }
                    xSuperKey->setAsciiListValue(implEntriesNew);
                }
            } else
            {
                Sequence<OUString> implEntriesNew(1);

                implEntriesNew.getArray()[0] = value;

                xSuperKey->setAsciiListValue(implEntriesNew);
            }
        }
        catch(InvalidRegistryException&)
        {
        }
    }
}

//*************************************************************************
//  static deleteSubEntry
//
static sal_Bool deleteSubEntry(const Reference < XRegistryKey >& xSuperKey, const OUString& value)
{
    if (xSuperKey.is())
    {
        try
        {
            if (xSuperKey->getValueType() == RegistryValueType_ASCIILIST)
            {
                Sequence<OUString> implEntries = xSuperKey->getAsciiListValue();
                sal_Int32 length = implEntries.getLength();
                sal_Int32 equals = 0;
                sal_Bool hasNoImplementations = sal_False;

                for (sal_Int32 i = 0; i < length; i++)
                {
                    if (implEntries.getConstArray()[i] == value)
                        equals++;
                }

                if (equals == length)
                {
                    hasNoImplementations = sal_True;
                } else
                {
                    Sequence<OUString> implEntriesNew(length - equals);

                    sal_Int32 j = 0;
                    for (sal_Int32 i = 0; i < length; i++)
                    {
                        if (implEntries.getConstArray()[i] != value)
                        {
                            implEntriesNew.getArray()[j++] = implEntries.getConstArray()[i];
                        }
                    }
                    xSuperKey->setAsciiListValue(implEntriesNew);
                }

                if (hasNoImplementations)
                {
                    return sal_True;
                }
            }
        }
        catch(InvalidRegistryException&)
        {
        }
    }

    return sal_False;
}

//*************************************************************************
//  static prepareUserLink
//
static sal_Bool prepareUserLink(const Reference < XSimpleRegistry >& xDest,
                            const OUString& linkName,
                            const OUString& linkTarget,
                            const OUString& implName)
{
    sal_Bool ret = sal_False;

    Reference < XRegistryKey > xRootKey;

    try
    {
        xRootKey = xDest->getRootKey();

        if (xRootKey->getKeyType(linkName) == RegistryKeyType_LINK)
        {
            OUString oldImplName(searchImplForLink(xRootKey, linkName, implName));

            if (oldImplName.len())
            {
                createUniqueSubEntry(xDest->getRootKey()->createKey(
                    linkName + OUString( RTL_CONSTASCII_USTRINGPARAM(":old") ) ), oldImplName);
            }
        }
    }
    catch (InvalidRegistryException&)
    {
    }

    try
    {
        if (xRootKey->isValid())
        {
            ret = xRootKey->createLink(linkName, linkTarget);
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    return ret;
}

//*************************************************************************
//  static deleteUserLink
//
static void deletePathIfPossible(const Reference < XRegistryKey >& xRootKey,
                                 const OUString& path)
{
    try
    {
        Sequence<OUString> keyNames(xRootKey->openKey(path)->getKeyNames());

        if (keyNames.getLength() == 0 &&
            xRootKey->openKey(path)->getValueType() == RegistryValueType_NOT_DEFINED)
        {
            xRootKey->deleteKey(path);

            OUString tmpPath(path);
            OUString newPath = tmpPath.copy(0, tmpPath.lastIndexOf('/')).getStr();

            if (newPath.len() > 1)
                deletePathIfPossible(xRootKey, newPath);
        }
    }
    catch(InvalidRegistryException&)
    {
    }
}


//*************************************************************************
//  static deleteUserLink
//
static sal_Bool deleteUserLink(const Reference < XRegistryKey >& xRootKey,
                           const OUString& linkName,
                           const OUString& linkTarget,
                           const OUString& implName)
{
    sal_Bool ret = sal_False;

    try
    {
        sal_Bool bClean = sal_False;

        if (xRootKey->getKeyType(linkName) == RegistryKeyType_LINK)
        {
            OUString tmpTarget = xRootKey->getLinkTarget(linkName);

            if (tmpTarget == linkTarget)
            {
                xRootKey->deleteLink(linkName);
            }
        }

        Reference < XRegistryKey > xOldKey = xRootKey->openKey(
            linkName + OUString( RTL_CONSTASCII_USTRINGPARAM(":old") ));
        if (xOldKey.is())
        {
            sal_Bool hasNoImplementations = sal_False;

            if (xOldKey->getValueType() == RegistryValueType_ASCIILIST)
            {
                Sequence<OUString> implEntries = xOldKey->getAsciiListValue();
                sal_Int32 length = implEntries.getLength();
                sal_Int32 equals = 0;

                for (sal_Int32 i = 0; i < length; i++)
                {
                    if (implEntries.getConstArray()[i] == implName)
                        equals++;
                }

                if (equals == length)
                {
                    hasNoImplementations = sal_True;
                } else
                {
                    OUString oldImpl;

                    if (length > equals + 1)
                    {
                        Sequence<OUString> implEntriesNew(length - equals - 1);

                        sal_Int32 j = 0;
                        sal_Bool first = sal_True;
                        for (sal_Int32 i = 0; i < length; i++)
                        {
                            if (implEntries.getConstArray()[i] != implName)
                            {
                                if (first)
                                {
                                    oldImpl = implEntries.getConstArray()[i];
                                    first = sal_False;
                                } else
                                {
                                    implEntriesNew.getArray()[j++] = implEntries.getConstArray()[i];
                                }
                            }
                        }

                        xOldKey->setAsciiListValue(implEntriesNew);
                    } else
                    {
                        oldImpl = implEntries.getConstArray()[0];

                        xOldKey->closeKey();
                        xRootKey->deleteKey(xOldKey->getKeyName());
                    }

                    OUString oldTarget = searchLinkTargetForImpl(xRootKey, linkName, oldImpl);
                    if (oldTarget.len())
                    {
                        xRootKey->createLink(linkName, oldTarget);
                    }
                }

                if (hasNoImplementations)
                {
                    bClean = sal_True;
                    hasNoImplementations = sal_False;
                    xOldKey->closeKey();
                    xRootKey->deleteKey(xOldKey->getKeyName());
                }
            }
        } else
        {
            bClean = sal_True;
        }

        if (bClean)
        {
            OUString tmpName(linkName);
            OUString path = tmpName.copy(0, tmpName.lastIndexOf('/')).getStr();
            deletePathIfPossible(xRootKey, path);
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    return ret;
}

//*************************************************************************
//  static prepareUserKeys
//
static sal_Bool prepareUserKeys(const Reference < XSimpleRegistry >& xDest,
                            const Reference < XRegistryKey >& xUnoKey,
                            const Reference < XRegistryKey >& xKey,
                            const OUString& implName,
                            sal_Bool bRegister)
{
    sal_Bool ret = sal_False;
    sal_Bool hasSubKeys = sal_False;

    try
    {
        Sequence<OUString> keyNames = xKey->getKeyNames();

        OUString relativKey;
        if (keyNames.getLength())
            relativKey = keyNames.getConstArray()[0].copy(xKey->getKeyName().len()+1);

        if (keyNames.getLength() == 1 &&
            xKey->getKeyType(relativKey) == RegistryKeyType_LINK)
        {
            hasSubKeys = sal_True;

            OUString linkTarget = xKey->getLinkTarget(relativKey);
            OUString linkName(xKey->getKeyName().copy(xUnoKey->getKeyName().len()));

            linkName = linkName + OUString( RTL_CONSTASCII_USTRINGPARAM("/") ) + relativKey;

            if (bRegister)
            {
                prepareUserLink(xDest, linkName, linkTarget, implName);
            } else
            {
                deleteUserLink(xDest->getRootKey(), linkName, linkTarget, implName);
            }
        } else
        {
            Sequence< Reference < XRegistryKey> > subKeys = xKey->openKeys();

            if (subKeys.getLength())
            {
                hasSubKeys = sal_True;
                const Reference < XRegistryKey > * pSubKeys = subKeys.getConstArray();

                for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
                {
                    ret = prepareUserKeys(xDest, xUnoKey, pSubKeys[i], implName, bRegister);
                }
            }
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    try
    {
        if (hasSubKeys)
        {
            return ret;
        }

        OUString keyName(xKey->getKeyName().copy(xUnoKey->getKeyName().len()));

        Reference < XRegistryKey > xRootKey = xDest->getRootKey();
        if (bRegister)
        {
            createUniqueSubEntry(xRootKey->createKey(keyName), implName);
        } else
        {
            if (deleteSubEntry(xRootKey->openKey(keyName), implName))
            {
                xRootKey->deleteKey(keyName);

                OUString tmpName(keyName);
                OUString path = tmpName.copy(0, tmpName.lastIndexOf('/')).getStr();
                deletePathIfPossible(xRootKey, path);
            }
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    return ret;
}

//*************************************************************************
//  static deleteAllImplementations
//
static void deleteAllImplementations(   const Reference < XSimpleRegistry >& xReg,
                                        const Reference < XRegistryKey >& xSource,
                                        const OUString& locationUrl,
                                        std::list<OUString> & implNames)
{
    try
    {
        Sequence < Reference < XRegistryKey > > subKeys = xSource->openKeys();

        if (subKeys.getLength() > 0)
        {
            const Reference < XRegistryKey> * pSubKeys = subKeys.getConstArray();
            Reference < XRegistryKey > xImplKey;
            sal_Bool hasLocationUrl = sal_False;

            for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
            {
                xImplKey = pSubKeys[i];
                Reference < XRegistryKey > xKey = xImplKey->openKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/LOCATION") ) );;

                if (xKey.is() && (xKey->getValueType() == RegistryValueType_ASCII))
                {
                    if (xKey->getAsciiValue() == locationUrl)
                    {
                        hasLocationUrl = sal_True;

                        OUString implName(xImplKey->getKeyName().getStr() + 1);
                        sal_Int32 firstDot = implName.search(L'/');

                        if (firstDot >= 0)
                            implName = implName.copy(firstDot + 1);

                        implNames.push_back(implName);

                        deleteAllLinkReferences(xReg, xImplKey);

                        xKey = xImplKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO")));
                        if (xKey.is())
                        {
                            Sequence< Reference < XRegistryKey > > subKeys = xKey->openKeys();

                            if (subKeys.getLength())
                            {
                                const Reference < XRegistryKey > * pSubKeys = subKeys.getConstArray();

                                for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
                                {
                                    if (pSubKeys[i]->getKeyName() != (xImplKey->getKeyName() + OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVCIES") )) &&
                                        pSubKeys[i]->getKeyName() != (xImplKey->getKeyName() + OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/REGISTRY_LINKS") )) &&
                                        pSubKeys[i]->getKeyName() != (xImplKey->getKeyName() + OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/ACTIVATOR") )) &&
                                        pSubKeys[i]->getKeyName() != (xImplKey->getKeyName() + OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/LOCATION") )))
                                    {
                                        prepareUserKeys(xReg, xKey, pSubKeys[i], implName, sal_False);
                                    }
                                }
                            }
                        }
                    }
                }

                if (hasLocationUrl)
                {
                    hasLocationUrl = sal_False;
                    xImplKey->closeKey();
                    xReg->getRootKey()->deleteKey(xImplKey->getKeyName());
                }
            }

            subKeys = xSource->openKeys();
            if (subKeys.getLength() == 0)
            {
                xSource->closeKey();
                xReg->getRootKey()->deleteKey(xSource->getKeyName());
            }
        } else
        {
            xSource->closeKey();
            xReg->getRootKey()->deleteKey(xSource->getKeyName());
        }
    }
    catch(InvalidRegistryException&)
    {
    }
}

//*************************************************************************
//  static deleteAllServiceEntries
//
static void deleteAllServiceEntries(    const Reference < XSimpleRegistry >& xReg,
                                        const Reference < XRegistryKey >& xSource,
                                        const OUString& implName)
{
    try
    {
        Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

        if (subKeys.getLength() > 0)
        {
            const Reference < XRegistryKey > * pSubKeys = subKeys.getConstArray();
            Reference < XRegistryKey > xServiceKey;
            sal_Bool hasNoImplementations = sal_False;

            for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
            {
                xServiceKey = pSubKeys[i];

                if (xServiceKey->getValueType() == RegistryValueType_ASCIILIST)
                {
                    Sequence<OUString> implEntries = xServiceKey->getAsciiListValue();
                    sal_Int32 length = implEntries.getLength();
                    sal_Int32 equals = 0;

                    for (sal_Int32 i = 0; i < length; i++)
                    {
                        if (implEntries.getConstArray()[i] == implName)
                            equals++;
                    }

                    if (equals == length)
                    {
                        hasNoImplementations = sal_True;
                    } else
                    {
                        if (equals > 0)
                        {
                            Sequence<OUString> implEntriesNew(length-equals);

                            sal_Int32 j = 0;
                            for (sal_Int32 i = 0; i < length; i++)
                            {
                                if (implEntries.getConstArray()[i] != implName)
                                {
                                    implEntriesNew.getArray()[j++] = implEntries.getConstArray()[i];
                                }
                            }

                            xServiceKey->setAsciiListValue(implEntriesNew);
                        }
                    }
                }

                if (hasNoImplementations)
                {
                    hasNoImplementations = sal_False;
                    xServiceKey->closeKey();
                    xReg->getRootKey()->deleteKey(xServiceKey->getKeyName());
                }
            }

            subKeys = xSource->openKeys();
            if (subKeys.getLength() == 0)
            {
                xSource->closeKey();
                xReg->getRootKey()->deleteKey(xSource->getKeyName());
            }
        } else
        {
            xSource->closeKey();
            xReg->getRootKey()->deleteKey(xSource->getKeyName());
        }
    }
    catch(InvalidRegistryException&)
    {
    }
}


//*************************************************************************
//  static prepareRegistry
//
static sal_Bool prepareRegistry(const Reference < XSimpleRegistry >& xDest,
                            const Reference < XRegistryKey >& xSource,
                            const OUString& implementationLoaderUrl,
                            const OUString& locationUrl)
{
    sal_Bool ret = sal_False;

    try
    {
        Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

        if (!subKeys.getLength())
        {
            return ret;
        }


        const Reference < XRegistryKey >* pSubKeys = subKeys.getConstArray();
        Reference < XRegistryKey > xImplKey;

        for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
        {
            xImplKey = pSubKeys[i];

            Reference < XRegistryKey >  xKey = xImplKey->openKey(
                OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") ) );

            if (xKey.is())
            {
                // update entries in SERVICES section
                Sequence< Reference < XRegistryKey > > serviceKeys = xKey->openKeys();
                OUString implName;

                if (serviceKeys.getLength())
                {
                    const Reference < XRegistryKey > * pServiceKeys = serviceKeys.getConstArray();

                    implName = OUString(xImplKey->getKeyName().getStr() + 1);
                    sal_Int32 firstDot = implName.search(L'/');

                    if (firstDot >= 0)
                        implName = implName.copy(firstDot + 1);

                    sal_Int32 offset = xKey->getKeyName().len() + 1;

                    for (sal_Int32 i = 0; i < serviceKeys.getLength(); i++)
                    {
                        OUString serviceName = pServiceKeys[i]->getKeyName().copy(offset);

                        createUniqueSubEntry(
                                xDest->getRootKey()->createKey(
                                    OUString( RTL_CONSTASCII_USTRINGPARAM("/SERVICES/") ) + serviceName ),
                                implName);
                    }

                    ret = sal_True;
                }

                xKey = xImplKey->openKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO") ));
                if (xKey.is())
                {
                    Sequence< Reference < XRegistryKey > > subKeys = xKey->openKeys();

                    if (subKeys.getLength())
                    {
                        const Reference < XRegistryKey > * pSubKeys = subKeys.getConstArray();

                        for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
                        {
                            if (pSubKeys[i]->getKeyName() != (xImplKey->getKeyName() + OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVCIES") )) &&
                                pSubKeys[i]->getKeyName() != (xImplKey->getKeyName() + OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/REGISTRY_LINKS") )))
                            {
                                prepareUserKeys(xDest, xKey, pSubKeys[i], implName, sal_True);
                            }
                        }
                    }
                }

                // update LOCATION entry
                xKey = xImplKey->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/LOCATION") ) );

                if (xKey.is())
                {
                    xKey->setAsciiValue(locationUrl);
                }

                // update ACTIVATOR entry
                xKey = xImplKey->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/ACTIVATOR") ) );

                if (xKey.is())
                {
                    xKey->setAsciiValue(implementationLoaderUrl);
                }

                // update DATA entry
                //xKey = xImplKey->createKey("/DATA");

                xKey = xImplKey->openKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/REGISTRY_LINKS") ) );

                if (xKey.is() && (xKey->getValueType() == RegistryValueType_ASCIILIST))
                {
                    // update link entries in REGISTRY_LINKS section
                    Sequence<OUString> linkNames = xKey->getAsciiListValue();

                    if (linkNames.getLength())
                    {
                        const OUString* pLinkNames = linkNames.getConstArray();

                        for (sal_Int32 i = 0; i < linkNames.getLength(); i++)
                        {
                            prepareLink(xDest, xImplKey, pLinkNames[i]);
                        }
                    }
                }
            }
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    return ret;
}


static void findImplementations(    const Reference < XRegistryKey > & xSource,
                                    std::list <OUString>& implNames)
{
    sal_Bool isImplKey = sal_False;

    try
    {
        Reference < XRegistryKey > xKey = xSource->openKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES") ) );

        if (xKey.is() && (xKey->getKeyNames().getLength() > 0))
        {
            isImplKey = sal_True;

            OUString implName = OUString(xSource->getKeyName().getStr() + 1).replace(L'/', L'.').getStr();
            sal_Int32 firstDot = implName.search(L'.');

            if (firstDot >= 0)
                implName = implName.copy(firstDot + 1);

            implNames.push_back(implName);
        }
    }
    catch(InvalidRegistryException&)
    {
    }

    if (isImplKey) return;

    try
    {
        Sequence< Reference < XRegistryKey > > subKeys = xSource->openKeys();

        if (subKeys.getLength() > 0)
        {
            const Reference < XRegistryKey >* pSubKeys = subKeys.getConstArray();

            for (sal_Int32 i = 0; i < subKeys.getLength(); i++)
            {
                findImplementations(pSubKeys[i], implNames);
            }

        }
    }
    catch(InvalidRegistryException&)
    {
    }
}








//*************************************************************************
//  class ImplementationRegistration the implenetation of interface
//  XImplementationRegistration
//
class ImplementationRegistration
    : public WeakImplHelper3< XImplementationRegistration, XServiceInfo, XInitialization >
{
public:
                                ImplementationRegistration( const Reference < XMultiServiceFactory > & rSMgr );
                                 ~ImplementationRegistration();

    // XServiceInfo
    OUString                        SAL_CALL getImplementationName() throw();
    sal_Bool                        SAL_CALL supportsService(const OUString& ServiceName) throw();
    Sequence< OUString >            SAL_CALL getSupportedServiceNames(void) throw();
    static Sequence< OUString >     SAL_CALL getSupportedServiceNames_Static(void) throw();

    // Methoden von XImplementationRegistration
    virtual void SAL_CALL registerImplementation(
        const OUString& implementationLoader,
        const OUString& location,
        const Reference < XSimpleRegistry > & xReg)
        throw(  CannotRegisterImplementationException, RuntimeException );

    virtual sal_Bool SAL_CALL revokeImplementation(
        const OUString& location,
        const Reference < XSimpleRegistry >& xReg)
        throw( RuntimeException );

    virtual Sequence< OUString > SAL_CALL getImplementations(
        const OUString& implementationLoader,
        const OUString& location)
        throw( RuntimeException );
    virtual Sequence< OUString > SAL_CALL checkInstantiation(
        const OUString& implementationName)
        throw( RuntimeException );

public: // XInitialization
    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments )
        throw(  ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

private: // helper methods

    static sal_Bool doRegistration( const Reference < XMultiServiceFactory >& xSMgr,
                                    const Reference < XImplementationLoader >& xAct,
                                    const Reference < XSimpleRegistry >& xDest,
                                    const OUString& implementationLoaderUrl,
                                    const OUString& locationUrl,
                                    sal_Bool bRegister);

    Reference< XSimpleRegistry > getRegistryFromServiceManager();

    static Reference< XSimpleRegistry > createTemporarySimpleRegistry( const Reference < XMultiServiceFactory > & r );

private: // members
    Reference < XMultiServiceFactory >  m_xSMgr;
};

//*************************************************************************
// ImplementationRegistration()
//
ImplementationRegistration::ImplementationRegistration( const Reference < XMultiServiceFactory > & rSMgr )
    : m_xSMgr( rSMgr )
{
}

//*************************************************************************
// ~ImplementationRegistration()
//
ImplementationRegistration::~ImplementationRegistration()
{
}


// XServiceInfo
OUString ImplementationRegistration::getImplementationName() throw()
{
    return OUString::createFromAscii( IMPLEMENTATION_NAME );
}

// XServiceInfo
sal_Bool ImplementationRegistration::supportsService(const OUString& ServiceName) throw()
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if( pArray[i] == ServiceName )
            return sal_True;
    return sal_False;
}

// XServiceInfo
Sequence< OUString > ImplementationRegistration::getSupportedServiceNames(void) throw()
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
Sequence< OUString > ImplementationRegistration::getSupportedServiceNames_Static(void) throw ()
{
    OUString aServiceName( OUString::createFromAscii( SERVICE_NAME ) );
    Sequence< OUString > aSNS( &aServiceName, 1 );
    return aSNS;
}


Reference< XSimpleRegistry > ImplementationRegistration::getRegistryFromServiceManager()
{
    Reference < XPropertySet > xPropSet =
                            Reference< XPropertySet >::query (m_xSMgr );
    Reference < XSimpleRegistry > xRegistry;

    if( xPropSet.is() ) {

        try {  // the implementation does not support XIntrospectionAccess !

            Any aAny = xPropSet->getPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("Registry") ) );

            if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE ) {
                aAny >>= xRegistry;
            }
         }
         catch( UnknownPropertyException e ) {
             // empty reference is error signal !
        }
    }

    return xRegistry;
}


//************************************************************************
// XInitialization
//
void ImplementationRegistration::initialize(
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArgs )
    throw(  ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{

    if( aArgs.getLength() != 4 ) {
        throw IllegalArgumentException();
    }

    Reference< XImplementationLoader > rLoader;
    OUString loaderServiceName;
    OUString locationUrl;
    Reference< XSimpleRegistry > rReg;

    // 1st argument : An instance of an implementation loader
    if( aArgs.getConstArray()[0].getValueType().getTypeClass() == TypeClass_INTERFACE ) {
        aArgs.getConstArray()[0] >>= rLoader;
    }
    if( !rLoader.is()) {
        throw IllegalArgumentException();
    }

    // 2nd argument : The service name of the loader. This name is written into the registry
    if( aArgs.getConstArray()[1].getValueType().getTypeClass() == TypeClass_STRING ) {
        aArgs.getConstArray()[1] >>= loaderServiceName;
    }
    if( ! loaderServiceName.getLength() ) {
        throw IllegalArgumentException();
    }

    // 3rd argument : The file name of the dll, that contains the loader
    if( aArgs.getConstArray()[2].getValueType().getTypeClass() == TypeClass_STRING ) {
        aArgs.getConstArray()[2] >>= locationUrl;
    }
    if( ! locationUrl.getLength() ) {
        throw IllegalArgumentException();
    }

    // 4th argument : The registry, the service should be written to
    if( aArgs.getConstArray()[3].getValueType().getTypeClass() == TypeClass_INTERFACE ) {
        aArgs.getConstArray()[3] >>= rReg;
    }

    if( !rReg.is() ) {
        rReg = getRegistryFromServiceManager();
        if( !rReg.is() ) {
            throw IllegalArgumentException();
        }
    }


    // TODO : SimpleRegistry in doRegistration von hand anziehen !
    if (!doRegistration(m_xSMgr, rLoader , rReg, loaderServiceName , locationUrl, sal_True)) {
        throw Exception();
    }

    // ------------------------------------------------------------
}



//*************************************************************************
// virtual function registerImplementation of XImplementationRegistration
//
void ImplementationRegistration::registerImplementation(
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    const Reference < XSimpleRegistry > & xReg)
    throw( CannotRegisterImplementationException, RuntimeException )
{
    OUString implLoaderUrl(implementationLoaderUrl);
    OUString activatorName;

    if (implementationLoaderUrl.len() > 0)
    {
        OUString tmpActivator(implementationLoaderUrl.getStr());
        activatorName = tmpActivator.getToken(0, L':').getStr();
    } else
    {
        // check locationUrl to find out what kind of loader is needed
        // set iimplLoaderUrl
    }

    if( m_xSMgr.is() ) {
        Reference < XImplementationLoader > xAct(  m_xSMgr->createInstance(activatorName) , UNO_QUERY );
        if (xAct.is())
        {
            Reference < XSimpleRegistry > xRegistry;

            if (xReg.is())
            {
                // registry supplied by user
                xRegistry = xReg;
            }
            else
            {
                xRegistry = getRegistryFromServiceManager();
            }

            if ( xRegistry.is())
            {
                if (!doRegistration(m_xSMgr, xAct, xRegistry, implLoaderUrl, locationUrl, sal_True))
                    throw CannotRegisterImplementationException();

                return;
            }
        }
    }

    throw CannotRegisterImplementationException();
}

//*************************************************************************
// virtual function revokeImplementation of XImplementationRegistration
//
sal_Bool ImplementationRegistration::revokeImplementation(const OUString& location,
                                                      const Reference < XSimpleRegistry >& xReg)
    throw ( RuntimeException )
{
    sal_Bool ret = sal_False;

    Reference < XSimpleRegistry > xRegistry;

    if (xReg.is()) {
        xRegistry = xReg;
    }
    else {
        Reference < XPropertySet > xPropSet = Reference< XPropertySet >::query( m_xSMgr );
        if( xPropSet.is() ) {
            try {
                Any aAny = xPropSet->getPropertyValue(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("Registry") ) );

                if( aAny.getValueType().getTypeClass() == TypeClass_INTERFACE )
                {
                    aAny >>= xRegistry;
                }
            }
            catch ( UnknownPropertyException e )  {
            }
        }
    }

    if (xRegistry.is())
    {
        ret = doRegistration(m_xSMgr, Reference< XImplementationLoader > (), xRegistry, OUString(), location, sal_False);
    }

    return ret;
}

//*************************************************************************
// virtual function getImplementations of XImplementationRegistration
//
Sequence< OUString > ImplementationRegistration::getImplementations(
    const OUString & implementationLoaderUrl,
    const OUString & locationUrl)
    throw ( RuntimeException )
{
    OUString implLoaderUrl(implementationLoaderUrl);
    OUString activatorName;

    if (implementationLoaderUrl.len() > 0)
    {
        OUString tmpActivator(implementationLoaderUrl.getStr());
        activatorName = tmpActivator.getToken(0, L':').getStr();
    } else
    {
        // check locationUrl to find out what kind of loader is needed
        // set implLoaderUrl
    }

    if( m_xSMgr.is() ) {

        Reference < XImplementationLoader > xAct( m_xSMgr->createInstance( activatorName ), UNO_QUERY );

        if (xAct.is())
        {

            Reference < XSimpleRegistry > xReg =    createTemporarySimpleRegistry( m_xSMgr);

            if (xReg.is())
            {
                try
                {
                    OUString aTempName = getTempName();

                    xReg->open(aTempName, sal_False, sal_True);
                    Reference < XRegistryKey > xImpl;

                    { // only necessary for deleting the temporary variable of rootkey
                        xImpl = xReg->getRootKey()->createKey(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("/IMPLEMENTATIONS") ) );
                    }
                    if (xAct->writeRegistryInfo(xImpl, implementationLoaderUrl, locationUrl))
                    {
                        std::list <OUString> implNames;

                        findImplementations(xImpl, implNames);

                        if (!implNames.empty())
                        {
                            std::list<OUString>::const_iterator iter = implNames.begin();

                            Sequence<OUString> seqImpl(implNames.size());
                            OUString *pImplNames = seqImpl.getArray();

                            sal_Int32 index = 0;
                            while (iter != implNames.end())
                            {
                                pImplNames[index] = *iter;
                                index++;
                                ++iter;
                            }

                            xImpl->closeKey();
                            xReg->destroy();
                            return seqImpl;
                        }
                    }

                    xImpl->closeKey();
                    xReg->destroy();
                }
                catch(MergeConflictException&)
                {
                }
                catch(InvalidRegistryException&)
                {
                }
            }
        }
    }

    return Sequence<OUString>();
}

//*************************************************************************
// virtual function checkInstantiation of XImplementationRegistration
//
Sequence< OUString > ImplementationRegistration::checkInstantiation(const OUString& implementationName)
    throw ( RuntimeException )
{
    OSL_ENSURE( sal_False, "ImplementationRegistration::checkInstantiation not implemented" );
    return Sequence<OUString>();
}

//*************************************************************************
// helper function doRegistration
//
sal_Bool ImplementationRegistration::doRegistration(
    const Reference< XMultiServiceFactory > & xSMgr,
    const Reference < XImplementationLoader > & xAct,
    const Reference < XSimpleRegistry >& xDest,
    const OUString& implementationLoaderUrl,
    const OUString& locationUrl,
    sal_Bool bRegister)
{
    sal_Bool ret = sal_False;

    if (!bRegister)
    {
        // deregister
        try
        {
            if( xDest.is() )
            {
                std::list<OUString> aNames;

                Reference < XRegistryKey > xRootKey( xDest->getRootKey() );

                Reference < XRegistryKey > xKey = xRootKey->openKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/IMPLEMENTATIONS") ) );
                if (xKey.is())
                {
                    deleteAllImplementations(xDest, xKey, locationUrl, aNames);
                    ret = sal_True;
                }


                xKey = xRootKey->openKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/SERVICES") ) );
                if (xKey.is())
                {
                    std::list<OUString>::const_iterator iter = aNames.begin();

                    while (iter != aNames.end())
                    {
                        deleteAllServiceEntries(xDest, xKey, *iter);
                        ++iter;
                    }
                }

                if (xRootKey.is())
                      xRootKey->closeKey();
                if (xKey.is())
                    xKey->closeKey();
            }
        }
        catch(InvalidRegistryException&)
        {
        }
    } else
    {
        Reference < XSimpleRegistry >   xReg = createTemporarySimpleRegistry( xSMgr );
        Reference < XRegistryKey >      xSourceKey;

        if (xAct.is() && xReg.is() && xDest.is())
        {
            OUString aTempName = getTempName();
            try
            {
                xReg->open(aTempName, sal_False, sal_True);

                { // only necessary for deleting the temporary variable of rootkey
                xSourceKey = xReg->getRootKey()->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/IMPLEMENTATIONS") ) );
                }

                if (xAct->writeRegistryInfo(xSourceKey, implementationLoaderUrl, locationUrl))
                {
                    if (prepareRegistry(xDest, xSourceKey, implementationLoaderUrl, locationUrl))
                    {
                        // Release Source key and registry.
                        xSourceKey->closeKey();
                        xReg->close();

                        xDest->mergeKey(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("/") ), aTempName );

                        ret = sal_True;
                    }
                }

                // Cleanup Source registry.
                if ( xSourceKey->isValid() )
                    xSourceKey->closeKey();
                if ( xReg->isValid() )
                    xReg->destroy();
                else
                {
                    xReg->open( aTempName, sal_False, sal_True );
                    xReg->destroy();
                }
            }
            catch(MergeConflictException&)
            {
            }
            catch(InvalidRegistryException&)
            {
            }
            catch(CannotRegisterImplementationException&)
            {
                // destroy temp registry
                if ( xSourceKey->isValid() )
                    xSourceKey->closeKey();
                if ( xReg->isValid() )
                    xReg->destroy();
                else
                {
                    xReg->open( aTempName, sal_False, sal_True );
                    xReg->destroy();
                }
                // and throw again
                throw;
            }
        }
    }

    return ret;
}



Reference< XSimpleRegistry > ImplementationRegistration::createTemporarySimpleRegistry(
    const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > & rSMgr)
{

    Reference < XSimpleRegistry > xReg =    Reference< XSimpleRegistry >::query(
        rSMgr->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.SimpleRegistry") ) ) );
    OSL_ASSERT( xReg.is() );
//      if( ! xReg.is() ) {
//          // use as fallback ( bootstrap )

//          Reference< XInterface > r = ::cppu::__loadLibComponentFactory(
//               "simreg",
//               "com.sun.star.comp.stoc.SimpleRegistry",
//               rSMgr ,
//               Reference < XRegistryKey >() )->createInstance();

//           xReg = Reference< XSimpleRegistry > ( r , UNO_QUERY );
//      }

    return xReg;
}



//*************************************************************************
static Reference<XInterface> SAL_CALL ImplementationRegistration_CreateInstance( const Reference<XMultiServiceFactory> & rSMgr ) throw(Exception)
{
    return (XImplementationRegistration *)new ImplementationRegistration(rSMgr);
}

}

extern "C"
{
//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii( "/" IMPLEMENTATION_NAME "/UNO/SERVICES" ) ) );

            const Sequence< OUString > & rSNL =
                ::stoc_impreg::ImplementationRegistration::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLEMENTATION_NAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString::createFromAscii( pImplName ),
            ::stoc_impreg::ImplementationRegistration_CreateInstance,
            ::stoc_impreg::ImplementationRegistration::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}


