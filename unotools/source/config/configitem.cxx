/*************************************************************************
 *
 *  $RCSfile: configitem.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: os $ $Date: 2000-11-24 12:05:38 $
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
#ifndef _UTL_CONFIGITEM_HXX_
#include "unotools/configitem.hxx"
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include "unotools/configmgr.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESLISTENER_HPP_
#include <com/sun/star/util/XChangesListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

namespace utl{
    class ConfigChangeListener_Impl : public cppu::WeakImplHelper1
    <
        com::sun::star::util::XChangesListener
    >
    {
            ConfigItem*                 pParent;
            const Sequence< OUString >  aPropertyNames;
        public:
            ConfigChangeListener_Impl(ConfigItem& rItem, Sequence< OUString >& rNames);
            ~ConfigChangeListener_Impl();

        //XChangesListener
        virtual void SAL_CALL changesOccurred( const ChangesEvent& Event ) throw(RuntimeException);

        //XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) throw(RuntimeException);

    };
}
/* -----------------------------29.08.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
ConfigChangeListener_Impl::ConfigChangeListener_Impl(
             ConfigItem& rItem, Sequence< OUString >& rNames) :
    pParent(&rItem),
    aPropertyNames(rNames)
{
}
/* -----------------------------29.08.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
ConfigChangeListener_Impl::~ConfigChangeListener_Impl()
{
}
/* -----------------------------29.08.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool lcl_Find(
        const rtl::OUString& rTemp,
        const OUString* pCheckPropertyNames,
        sal_Int32 nLength)
{
    //return true if the path is completely correct or if it is longer
    //i.e ...Print/Content/Graphic and .../Print
    for(sal_Int32 nIndex = 0; nIndex < nLength; nIndex++)
        if(pCheckPropertyNames[nIndex] == rTemp ||
            pCheckPropertyNames[nIndex].getLength() < rTemp.getLength() &&
                rTemp[pCheckPropertyNames[nIndex].getLength()] == '/' &&
                    0 == rTemp.compareTo(pCheckPropertyNames[nIndex], pCheckPropertyNames[nIndex].getLength())
            )
            return sal_True;
    return sal_False;
}
//-----------------------------------------------------------------------------
void ConfigChangeListener_Impl::changesOccurred( const ChangesEvent& rEvent ) throw(RuntimeException)
{
    const ElementChange* pElementChanges = rEvent.Changes.getConstArray();

    Sequence<OUString>  aChangedNames(rEvent.Changes.getLength());
    OUString* pNames = aChangedNames.getArray();

    const OUString* pCheckPropertyNames = aPropertyNames.getConstArray();

    const sal_Int32 nBaseTreeLen = ConfigManager::GetConfigBaseURL().getLength() +
        pParent->GetSubTreeName().getLength() + 1;
    sal_Int32 nNotify = 0;
    for(int i = 0; i < aChangedNames.getLength(); i++)
    {
        OUString sTemp;
        pElementChanges[i].Accessor >>= sTemp;
//#ifdef DEBUG // workaround to #79499#
//      if(0 == sTemp.compareToAscii("/org.openoffice.Office.Writer/Print/", 36))
//      {
//          OUString sValue = sTemp.copy(36);
//          sTemp = OUString::createFromAscii("/org.openoffice.Office.Writer/Print/");
//          sTemp += OUString::createFromAscii("Content/");
//          sTemp += sValue;
//      }
//#endif
        OSL_ENSHURE(nBaseTreeLen < sTemp.getLength(),"property name incorrect");
        sTemp = sTemp.copy(nBaseTreeLen);
        if(lcl_Find(sTemp, pCheckPropertyNames, aPropertyNames.getLength()))
            pNames[nNotify++] = sTemp;
    }
    if(nNotify)
    {
        aChangedNames.realloc(nNotify);
        pParent->CallNotify(aChangedNames);
    }
}
/* -----------------------------29.08.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigChangeListener_Impl::disposing( const EventObject& Source ) throw(RuntimeException)
{
    OSL_DEBUG_ONLY("ConfigChangeListener_Impl::disposing");
}
/* -----------------------------29.08.00 12:50--------------------------------

 ---------------------------------------------------------------------------*/
ConfigItem::ConfigItem(const OUString rSubTree ) :
    pManager(ConfigManager::GetConfigManager()),
    sSubTree(rSubTree),
    bIsModified(sal_False),
    bInPutValues(sal_False),
    bHasChangedProperties(sal_False)
{
    xHierarchyAccess = pManager->AddConfigItem(*this);
}
/* -----------------------------17.11.00 13:53--------------------------------

 ---------------------------------------------------------------------------*/
ConfigItem::ConfigItem(utl::ConfigManager&  rManager, const rtl::OUString rSubTree) :
    pManager(&rManager),
    sSubTree(rSubTree),
    bIsModified(sal_False),
    bInPutValues(sal_False),
    bHasChangedProperties(sal_False)
{
    xHierarchyAccess = pManager->AddConfigItem(*this);
}

/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
ConfigItem::~ConfigItem()
{
    if(pManager)
    {
        if(bHasChangedProperties && xHierarchyAccess.is())
        {
            try
            {
                Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
                xBatch->commitChanges();
            }
            catch(Exception& rEx)
            {
    #ifdef DBG_UTIL
                OString sMsg("Exception from commitChanges(): ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                OSL_DEBUG_ONLY(sMsg.getStr());
    #endif
            }
        }
        RemoveListener();
        pManager->RemoveConfigItem(*this);
    }
}
/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigItem::Commit()
{
    OSL_DEBUG_ONLY("Base class called");
}
/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigItem::ReleaseConfigMgr()
{
    if(bHasChangedProperties && xHierarchyAccess.is())
    {
        try
        {
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            xBatch->commitChanges();
        }
        catch(Exception& rEx)
        {
#ifdef DBG_UTIL
            OString sMsg("Exception from commitChanges(): ");
            sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
            OSL_DEBUG_ONLY(sMsg.getStr());
#endif
        }
        bHasChangedProperties = sal_False;
    }
    RemoveListener();
    OSL_ENSHURE(pManager, "ConfigManager already released");
    pManager = 0;
}
/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigItem::CallNotify( const com::sun::star::uno::Sequence<OUString>& rPropertyNames )
{
    if(!bInPutValues)
        Notify(rPropertyNames);
    else
        bHasChangedProperties = sal_True;
}
/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigItem::Notify( const com::sun::star::uno::Sequence<OUString>& rPropertyNames)
{
    OSL_DEBUG_ONLY("Base class called");
}
/* -----------------------------29.08.00 15:10--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Any > ConfigItem::GetProperties(const Sequence< OUString >& rNames)
{
    Sequence< Any > aRet(rNames.getLength());
    const OUString* pNames = rNames.getConstArray();
    Any* pRet = aRet.getArray();
    if(xHierarchyAccess.is())
    {
        for(int i = 0; i < rNames.getLength(); i++)
        {
            try
            {
                    pRet[i] = xHierarchyAccess->getByHierarchicalName(pNames[i]);
            }
    #ifdef DBG_UTIL
            catch(Exception& rEx)
            {
                OString sMsg("XHierarchicalNameAccess: ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                sMsg += OString("\n");
                sMsg += OString(ConfigManager::GetConfigBaseURL().getStr(),
                    ConfigManager::GetConfigBaseURL().getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                sMsg += OString(sSubTree.getStr(),
                    sSubTree.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                sMsg += OString("/");
                sMsg += OString(pNames[i].getStr(),
                    pNames[i].getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                OSL_DEBUG_ONLY(sMsg.getStr());
            }
#else
            catch(Exception&){}
#endif
        }
    }
    return aRet;
}
/* -----------------------------29.08.00 17:28--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::PutProperties( const Sequence< OUString >& rNames,
                                                const Sequence< Any>& rValues)
{
    bInPutValues = sal_True;
    Reference<XPropertySet> xPropSet(xHierarchyAccess, UNO_QUERY);
    sal_Bool bRet = xHierarchyAccess.is() && xPropSet.is();
    if(bRet)
    {
        const OUString* pNames = rNames.getConstArray();
        const Any* pValues = rValues.getConstArray();
        for(int i = 0; i < rNames.getLength(); i++)
        {
            try
            {
                sal_Int32 nLastIndex = pNames[i].lastIndexOf( '/',  pNames[i].getLength());

                if(nLastIndex > 0)
                {
                    OUString sNode =    pNames[i].copy( 0, nLastIndex );
                    OUString sProperty =    pNames[i].copy( nLastIndex + 1, pNames[i].getLength() - nLastIndex - 1 );
                    Any aNode = xHierarchyAccess->getByHierarchicalName(sNode);
                    Reference<XNameAccess> xNodeAcc;
                    aNode >>= xNodeAcc;
                    Reference<XNameReplace> xNodeReplace(xNodeAcc, UNO_QUERY);
                    if(xNodeReplace.is())
                    {
                        xNodeReplace->replaceByName(sProperty, pValues[i]);
                    }
                    else
                        bRet = sal_False;
                }
                else //direct value
                {
                    xPropSet->setPropertyValue(pNames[i], pValues[i]);
                }
            }
#ifdef DBG_UTIL
            catch(Exception& rEx)
            {
                OString sMsg("Exception from PutProperties: ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                OSL_DEBUG_ONLY(sMsg.getStr());
            }
#else
            catch(Exception&){}
#endif
        }
        try
        {
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            xBatch->commitChanges();
        }
        catch(Exception& rEx)
        {
#ifdef DBG_UTIL
            OString sMsg("Exception from commitChanges(): ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
            OSL_DEBUG_ONLY(sMsg.getStr());
#endif
        }
    }

    bInPutValues = sal_False;
    return bRet;
}
/* -----------------------------29.08.00 16:19--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool    ConfigItem::EnableNotification(Sequence< OUString >& rNames)
{
    Reference<XChangesNotifier> xChgNot(xHierarchyAccess, UNO_QUERY);
    if(!xChgNot.is())
        return sal_False;
    sal_Bool bRet = sal_True;

    try
    {
        xChangeLstnr = new ConfigChangeListener_Impl(*this, rNames);
        xChgNot->addChangesListener( xChangeLstnr );
    }
    catch(RuntimeException& )
    {
        bRet = sal_False;
    }
    return bRet;
}
/* -----------------------------29.08.00 16:47--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigItem::RemoveListener()
{
    Reference<XChangesNotifier> xChgNot(xHierarchyAccess, UNO_QUERY);
    if(xChgNot.is() && xChangeLstnr.is())
    {
        try
        {
            xChgNot->removeChangesListener( xChangeLstnr );
            xHierarchyAccess = 0;
        }
        catch(Exception & )
        {
        }
    }
}
/* -----------------------------15.09.00 12:06--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > ConfigItem::GetNodeNames(const OUString& rNode)
{
    Sequence< OUString > aRet;
    if(xHierarchyAccess.is())
    {
        try
        {
            Reference<XNameAccess> xCont;
            if(rNode.getLength())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameAccess> (xHierarchyAccess, UNO_QUERY);
            if(xCont.is())
            {
                aRet = xCont->getElementNames();
            }
        }
#ifdef DBG_UTIL
        catch(Exception& rEx)
        {
            OString sMsg("Exception from GetNodeNames: ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
            OSL_DEBUG_ONLY(sMsg.getStr());
        }
#else
        catch(Exception&){}
#endif
    }
    return aRet;
}
/* -----------------------------15.09.00 15:52--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::ClearNodeSet(const OUString& rNode)
{
    sal_Bool bRet;
    if(xHierarchyAccess.is())
    {
        try
        {
            Reference<XNameContainer> xCont;
            if(rNode.getLength())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return sal_False;
            Sequence< OUString > aNames = xCont->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            for(sal_Int32 i = 0; i < aNames.getLength(); i++)
            {
                try
                {
                    xCont->removeByName(pNames[i]);
                    xBatch->commitChanges();
                }
                catch(Exception& rEx)
                {
        #ifdef DBG_UTIL
                    OString sMsg("Exception from commitChanges(): ");
                    sMsg += OString(rEx.Message.getStr(),
                        rEx.Message.getLength(),
                         RTL_TEXTENCODING_ASCII_US);
                    OSL_DEBUG_ONLY(sMsg.getStr());
        #endif
                }
            }
        }
        catch(Exception& rEx)
        {
#ifdef DBG_UTIL
            OString sMsg("Exception from GetNodeNames: ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
            OSL_DEBUG_ONLY(sMsg.getStr());
#endif
            bRet = sal_False;
        }
    }
    return bRet;
}
/* -----------------------------24.11.00 10:58--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::ClearNodeElements(const OUString& rNode, Sequence< OUString >& rElements)
{
    sal_Bool bRet;
    if(xHierarchyAccess.is())
    {
        const OUString* pElements = rElements.getConstArray();
        try
        {
            Reference<XNameContainer> xCont;
            if(rNode.getLength())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return sal_False;
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            try
            {
                for(sal_Int32 nElement = 0; nElement < rElements.getLength(); nElement++)
                {
                    xCont->removeByName(pElements[nElement]);
                    xBatch->commitChanges();
                }
            }
            catch(Exception& rEx)
            {
    #ifdef DBG_UTIL
                OString sMsg("Exception from commitChanges(): ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                OSL_DEBUG_ONLY(sMsg.getStr());
    #endif
            }
        }
        catch(Exception& rEx)
        {
#ifdef DBG_UTIL
            OString sMsg("Exception from GetNodeNames: ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
            OSL_DEBUG_ONLY(sMsg.getStr());
#endif
            bRet = sal_False;
        }
    }
    return bRet;
}
/* -----------------------------15.09.00 15:52--------------------------------
    add or change properties
 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::SetSetProperties(
    const OUString& rNode, Sequence< PropertyValue > rValues)
{
    sal_Bool bRet;
    if(xHierarchyAccess.is())
    {
        Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
        try
        {
            Reference<XNameContainer> xCont;
            if(rNode.getLength())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return sal_False;
            const PropertyValue* pProperties = rValues.getConstArray();
            Sequence< OUString > aSubNodeNames(rValues.getLength());
            OUString* pSubNodeNames = aSubNodeNames.getArray();
            sal_Int32 nNodeLength = rNode.getLength() + 1;
            OUString sLastSubNode;
            sal_Int32 nSubIndex = 0;

            for(sal_Int32 i = 0; i < rValues.getLength(); i++)
            {
                OUString sSubNode = pProperties[i].Name.copy(nNodeLength);
                sSubNode = sSubNode.copy(0, sSubNode.indexOf('/'));
                if(sLastSubNode != sSubNode)
                {
                    pSubNodeNames[nSubIndex++] = sSubNode;
                }
                sLastSubNode = sSubNode;
            }
            aSubNodeNames.realloc(nSubIndex);
            pSubNodeNames = aSubNodeNames.getArray();

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);
            for(sal_Int32 j = 0; j < nSubIndex; j++)
            {
                if(!xCont->hasByName(pSubNodeNames[j]))
                {
                    //create if not available
                    if(!xFac.is())
                        return sal_False;
                    Reference<XInterface> xInst = xFac->createInstance();
                    Any aVal; aVal <<= xInst;
                    xCont->insertByName(pSubNodeNames[j], aVal);
                    //node changes must be commited before values can be changed
                    try
                    {
                        xBatch->commitChanges();
                    }
                    catch(Exception& rEx)
                    {
            #ifdef DBG_UTIL
                        OString sMsg("Exception from commitChanges(): ");
                        sMsg += OString(rEx.Message.getStr(),
                            rEx.Message.getLength(),
                             RTL_TEXTENCODING_ASCII_US);
                        OSL_DEBUG_ONLY(sMsg.getStr());
            #endif
                    }
                }
                //set values
            }
            Sequence< OUString > aSetNames(rValues.getLength());
            OUString* pSetNames = aSetNames.getArray();
            Sequence< Any> aSetValues(rValues.getLength());
            Any* pSetValues = aSetValues.getArray();
            for(sal_Int32 k = 0; k < rValues.getLength(); k++)
            {
                pSetNames[k] =  pProperties[k].Name;
                pSetValues[k] = pProperties[k].Value;
            }
            try
            {
                xBatch->commitChanges();
            }
            catch(Exception& rEx)
            {
    #ifdef DBG_UTIL
                OString sMsg("Exception from commitChanges(): ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                OSL_DEBUG_ONLY(sMsg.getStr());
    #endif
            }
            bRet = PutProperties(aSetNames, aSetValues);

        }
        catch(Exception& rEx)
        {
#ifdef DBG_UTIL
            OString sMsg("Exception from GetNodeNames: ");
            sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
            OSL_DEBUG_ONLY(sMsg.getStr());
#endif
            bRet = sal_False;
        }
    }
    return bRet;
}
/* -----------------------------15.09.00 15:52--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::ReplaceSetProperties(
    const OUString& rNode, Sequence< PropertyValue > rValues)
{
    sal_Bool bRet;
    if(xHierarchyAccess.is())
    {
        Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
        try
        {
            Reference<XNameContainer> xCont;
            if(rNode.getLength())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(rNode);
                aNode >>= xCont;
            }
            else
                xCont = Reference<XNameContainer> (xHierarchyAccess, UNO_QUERY);
            if(!xCont.is())
                return sal_False;
            const PropertyValue* pProperties = rValues.getConstArray();
            //remove unknown members first

            Sequence< OUString > aSubNodeNames(rValues.getLength());
            OUString* pSubNodeNames = aSubNodeNames.getArray();
            sal_Int32 nNodeLength = rNode.getLength() + 1;
            OUString sLastSubNode;
            sal_Int32 nSubIndex = 0;

            for(sal_Int32 i = 0; i < rValues.getLength(); i++)
            {
                OUString sSubNode = pProperties[i].Name.copy(nNodeLength);
                sSubNode = sSubNode.copy(0, sSubNode.indexOf('/'));
                if(sLastSubNode != sSubNode)
                {
                    pSubNodeNames[nSubIndex++] = sSubNode;
                }
                sLastSubNode = sSubNode;
            }
            aSubNodeNames.realloc(nSubIndex);
            pSubNodeNames = aSubNodeNames.getArray();

            Sequence<OUString> aContainerSubNodes = xCont->getElementNames();
            const OUString* pContainerSubNodes = aContainerSubNodes.getConstArray();
            for(sal_Int32 nContSub = 0; nContSub < aContainerSubNodes.getLength(); nContSub++)
            {
                sal_Bool bFound = sal_False;
                for(sal_Int32 j = 0; j < nSubIndex; j++)
                    if(pSubNodeNames[j] == pContainerSubNodes[nContSub])
                    {
                        bFound = sal_True;
                        break;
                    }
                if(!bFound)
                {
                    xCont->removeByName(pContainerSubNodes[nContSub]);
                    //node changes must be commited before values can be changed
                    try
                    {
                        xBatch->commitChanges();
                    }
                    catch(Exception& rEx)
                    {
            #ifdef DBG_UTIL
                        OString sMsg("Exception from commitChanges(): ");
                        sMsg += OString(rEx.Message.getStr(),
                            rEx.Message.getLength(),
                             RTL_TEXTENCODING_ASCII_US);
                        OSL_DEBUG_ONLY(sMsg.getStr());
            #endif
                    }
                }
            }

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);
            for(sal_Int32 j = 0; j < nSubIndex; j++)
            {
                if(!xCont->hasByName(pSubNodeNames[j]))
                {
                    //create if not available
                    if(!xFac.is())
                        return sal_False;
                    Reference<XInterface> xInst = xFac->createInstance();
                    Any aVal; aVal <<= xInst;
                    xCont->insertByName(pSubNodeNames[j], aVal);
                    //node changes must be commited before values can be changed
                    try
                    {
                        xBatch->commitChanges();
                    }
                    catch(Exception& rEx)
                    {
            #ifdef DBG_UTIL
                        OString sMsg("Exception from commitChanges(): ");
                        sMsg += OString(rEx.Message.getStr(),
                            rEx.Message.getLength(),
                             RTL_TEXTENCODING_ASCII_US);
                        OSL_DEBUG_ONLY(sMsg.getStr());
            #endif
                    }
                }
                //set values
            }
            Sequence< OUString > aSetNames(rValues.getLength());
            OUString* pSetNames = aSetNames.getArray();
            Sequence< Any> aSetValues(rValues.getLength());
            Any* pSetValues = aSetValues.getArray();
            for(sal_Int32 k = 0; k < rValues.getLength(); k++)
            {
                pSetNames[k] =  pProperties[k].Name;
                pSetValues[k] = pProperties[k].Value;
            }

            //node changes must be commited before values can be changed
            try
            {
                xBatch->commitChanges();
            }
            catch(Exception& rEx)
            {
    #ifdef DBG_UTIL
                OString sMsg("Exception from commitChanges(): ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
                OSL_DEBUG_ONLY(sMsg.getStr());
    #endif
            }
            bRet = PutProperties(aSetNames, aSetValues);

        }
        catch(Exception& rEx)
        {
#ifdef DBG_UTIL
            OString sMsg("Exception from GetNodeNames: ");
                sMsg += OString(rEx.Message.getStr(),
                    rEx.Message.getLength(),
                     RTL_TEXTENCODING_ASCII_US);
            OSL_DEBUG_ONLY(sMsg.getStr());
#endif
            bRet = sal_False;
        }
    }
    return bRet;
}



