/*************************************************************************
 *
 *  $RCSfile: configitem.cxx,v $
 *
 *  $Revision: 1.42 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 13:37:27 $
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
#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include "unotools/configpathes.hxx"
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
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAME_HPP_
#include <com/sun/star/container/XHierarchicalName.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_XTEMPLATECONTAINER_HPP_
#include <com/sun/star/configuration/XTemplateContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XSTRINGESCAPE_HPP_
#include <com/sun/star/util/XStringEscape.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <rtl/ustrbuf.hxx>

using namespace utl;
using rtl::OUString;
using rtl::OString;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace com::sun::star::configuration;

#define C2U(cChar) OUString::createFromAscii(cChar)

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

#ifdef DBG_UTIL
inline void lcl_CFG_DBG_EXCEPTION(const sal_Char* cText, const Exception& rEx)
{
    OString sMsg(cText);
    sMsg += OString(rEx.Message.getStr(), rEx.Message.getLength(), RTL_TEXTENCODING_ASCII_US);
    OSL_ENSURE(sal_False, sMsg.getStr());
}
#define CATCH_INFO(a) \
catch(Exception& rEx)   \
{                       \
    lcl_CFG_DBG_EXCEPTION(a, rEx);\
}
#else
    #define lcl_CFG_DBG_EXCEPTION( a, b)
    #define CATCH_INFO(a) catch(Exception& ){}
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
            ConfigChangeListener_Impl(ConfigItem& rItem, const Sequence< OUString >& rNames);
            ~ConfigChangeListener_Impl();

        //XChangesListener
        virtual void SAL_CALL changesOccurred( const ChangesEvent& Event ) throw(RuntimeException);

        //XEventListener
        virtual void SAL_CALL disposing( const EventObject& Source ) throw(RuntimeException);

    };
/* -----------------------------12.02.01 11:38--------------------------------

 ---------------------------------------------------------------------------*/
struct ConfigItem_Impl
{
    utl::ConfigManager*         pManager;
       sal_Int16                    nMode;
    sal_Bool                    bIsModified;
    sal_Bool                    bEnableInternalNotification;

    sal_Int16                   nInValueChange;
    ConfigItem_Impl() :
        pManager(0),
        nMode(0),
        nInValueChange(0),
        bIsModified(sal_False),
        bEnableInternalNotification(sal_False){}
};
}
/* -----------------------------04.12.00 10:25--------------------------------

 ---------------------------------------------------------------------------*/
class ValueCounter_Impl
{
    sal_Int16& rCnt;
public:
    ValueCounter_Impl(sal_Int16& rCounter):
        rCnt(rCounter)
            {rCnt++;}
    ~ValueCounter_Impl()
            {
                OSL_ENSURE(rCnt>0, "RefCount < 0 ??");
                rCnt--;
            }
};
/* -----------------------------03.12.02 -------------------------------------

 ---------------------------------------------------------------------------*/
namespace
{
    // helper to achieve exception - safe handling of an Item under construction
    template <class TYP>
    class AutoDeleter // : Noncopyable
    {
        TYP* pItem;
    public:
        AutoDeleter(TYP * pItem)
        : pItem(pItem)
        {
        }

        ~AutoDeleter()
        {
            delete pItem;
        }

        void keep() { pItem = 0; }
    };
}
/* -----------------------------29.08.00 16:34--------------------------------

 ---------------------------------------------------------------------------*/
ConfigChangeListener_Impl::ConfigChangeListener_Impl(
             ConfigItem& rItem, const Sequence< OUString >& rNames) :
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
        if( isPrefixOfConfigurationPath(rTemp, pCheckPropertyNames[nIndex]) )
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
}
/* -----------------------------29.08.00 12:50--------------------------------

 ---------------------------------------------------------------------------*/
ConfigItem::ConfigItem(const OUString rSubTree, sal_Int16 nSetMode ) :
    pImpl(new ConfigItem_Impl),
    sSubTree(rSubTree)
{
    AutoDeleter<ConfigItem_Impl> aNewImpl(pImpl);

    pImpl->pManager = ConfigManager::GetConfigManager();
    pImpl->nMode = nSetMode;
    if(0 != (nSetMode&CONFIG_MODE_RELEASE_TREE))
        pImpl->pManager->AddConfigItem(*this);
    else
        m_xHierarchyAccess = pImpl->pManager->AddConfigItem(*this);

    // no more exceptions after c'tor has finished
    aNewImpl.keep();
    pImpl->nMode &= ~CONFIG_MODE_PROPAGATE_ERRORS;
}
/* -----------------------------17.11.00 13:53--------------------------------

 ---------------------------------------------------------------------------*/
ConfigItem::ConfigItem(utl::ConfigManager&  rManager, const rtl::OUString rSubTree) :
    pImpl(new ConfigItem_Impl),
    sSubTree(rSubTree)
{
    pImpl->pManager = &rManager;
    pImpl->nMode = CONFIG_MODE_IMMEDIATE_UPDATE; // does not allow exceptions
    m_xHierarchyAccess = pImpl->pManager->AddConfigItem(*this);
}
//---------------------------------------------------------------------
//--- 02.08.2002 16:33:23 -----------------------------------------------
sal_Bool ConfigItem::IsValidConfigMgr() const
{
    return ( pImpl->pManager && pImpl->pManager->GetConfigurationProvider().is() );
}

/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
ConfigItem::~ConfigItem()
{
    if(pImpl->pManager)
    {
        RemoveListener();
        pImpl->pManager->RemoveConfigItem(*this);
    }
    delete pImpl;
}
/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigItem::Commit()
{
    OSL_ENSURE(sal_False, "Base class called");
}
/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigItem::ReleaseConfigMgr()
{
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        try
        {
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            xBatch->commitChanges();
        }
        CATCH_INFO("Exception from commitChanges(): ")
    }
    RemoveListener();
    OSL_ENSURE(pImpl->pManager, "ConfigManager already released");
    pImpl->pManager = 0;
}
/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigItem::CallNotify( const com::sun::star::uno::Sequence<OUString>& rPropertyNames )
{
    if(!IsInValueChange() || pImpl->bEnableInternalNotification)
        Notify(rPropertyNames);
}
/* -----------------------------29.08.00 12:52--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigItem::Notify( const com::sun::star::uno::Sequence<OUString>& rPropertyNames)
{
    OSL_ENSURE(sal_False, "Base class called");
}
/* -----------------------------12.12.00 17:09--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool lcl_IsLocalProperty(const OUString& rSubTree, const OUString& rProperty)
{
    static const sal_Char* aLocalProperties[] =
    {
        "Office.Common/Path/Current/Storage",
        "Office.Common/Path/Current/Temp",
        "Security/MountPoints"
    };
    static const int aLocalPropLen[] =
    {
        34,
        31,
        20
    };
    OUString sProperty(rSubTree);
    sProperty += C2U("/");
    sProperty += rProperty;

    if(sProperty.equalsAsciiL( aLocalProperties[0], aLocalPropLen[0]) ||
        sProperty.equalsAsciiL( aLocalProperties[1], aLocalPropLen[1]) ||
        sProperty.
            equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                             "Office.Common/Path/Current/OfficeInstall")) ||
        sProperty.
            equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(
                             "UserProfile/Office/InstallPath")))
        return sal_True;

    if(!sProperty.compareToAscii( aLocalProperties[2],  aLocalPropLen[2]))
        return sal_True;
    return sal_False;
}
/* -----------------------------10.04.01 15:00--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigItem::impl_packLocalizedProperties(  const   Sequence< OUString >&   lInNames    ,
                                                const   Sequence< Any >&        lInValues   ,
                                                        Sequence< Any >&        lOutValues  )
{
    // Safe impossible cases.
    // This method should be called for special ConfigItem-mode only!
    OSL_ENSURE( ((pImpl->nMode & CONFIG_MODE_ALL_LOCALES ) == CONFIG_MODE_ALL_LOCALES), "ConfigItem::impl_packLocalizedProperties()\nWrong call of this method detected!\n" );

    sal_Int32                   nSourceCounter      ;   // used to step during input lists
    sal_Int32                   nSourceSize         ;   // marks end of loop over input lists
    sal_Int32                   nDestinationCounter ;   // actual position in output lists
    sal_Int32                   nPropertyCounter    ;   // counter of inner loop for Sequence< PropertyValue >
    sal_Int32                   nPropertiesSize     ;   // marks end of inner loop
    Sequence< OUString >        lPropertyNames      ;   // list of all locales for localized entry
    Sequence< PropertyValue >   lProperties         ;   // localized values of an configuration entry packed for return
    Reference< XInterface >     xLocalizedNode      ;   // if cfg entry is localized ... lInValues contains an XInterface!

    // Optimise follow algorithm ... A LITTLE BIT :-)
    // There exist two different possibilities:
    //  i ) There exist no localized entries ...                        =>  size of lOutValues will be the same like lInNames/lInValues!
    //  ii) There exist some (mostly one or two) localized entries ...  =>  size of lOutValues will be the same like lInNames/lInValues!
    //  ... Why? If a localized value exist - the any is filled with an XInterface object (is a SetNode-service).
    //      We read all his child nodes and pack it into Sequence< PropertyValue >.
    //      The result list we pack into the return any. We never change size of lists!
    nSourceSize = lInNames.getLength();
    lOutValues.realloc( nSourceSize );

    // Algorithm:
    // Copy all names and values from in to out lists.
    // Look for special localized entries ... You can detect it as "XInterface" packed into an Any.
    // Use this XInterface-object to read all localized values and pack it into Sequence< PropertValue >.
    // Add this list to out lists then.

    nDestinationCounter = 0;
    for( nSourceCounter=0; nSourceCounter<nSourceSize; ++nSourceCounter )
    {
        // If item a special localized one ... convert and pack it ...
        if( lInValues[nSourceCounter].getValueTypeName() == C2U("com.sun.star.uno.XInterface") )
        {
            lInValues[nSourceCounter] >>= xLocalizedNode;
            Reference< XNameContainer > xSetAccess( xLocalizedNode, UNO_QUERY );
            if( xSetAccess.is() == sal_True )
            {
                lPropertyNames  =   xSetAccess->getElementNames()   ;
                nPropertiesSize =   lPropertyNames.getLength()      ;
                lProperties.realloc( nPropertiesSize )              ;

                for( nPropertyCounter=0; nPropertyCounter<nPropertiesSize; ++nPropertyCounter )
                {
                    #if OSL_DEBUG_LEVEL > 1
                    // Sometimes it's better to see what's going on :-)
                    OUString sPropName   = lInNames[nSourceCounter];
                    OUString sLocaleName = lPropertyNames[nPropertyCounter];
                    #endif
                    lProperties[nPropertyCounter].Name  =   lPropertyNames[nPropertyCounter]                            ;
                    OUString sLocaleValue;
                    xSetAccess->getByName( lPropertyNames[nPropertyCounter] ) >>= sLocaleValue  ;
                    lProperties[nPropertyCounter].Value <<= sLocaleValue;
                }

                lOutValues[nDestinationCounter] <<= lProperties;
            }
        }
        // ... or copy normal items to return lists directly.
        else
        {
            lOutValues[nDestinationCounter] = lInValues[nSourceCounter];
        }
        ++nDestinationCounter;
    }
}
/* -----------------------------10.04.01 15:00--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigItem::impl_unpackLocalizedProperties(    const   Sequence< OUString >&   lInNames    ,
                                                    const   Sequence< Any >&        lInValues   ,
                                                            Sequence< OUString >&   lOutNames   ,
                                                            Sequence< Any >&        lOutValues  )
{
    // Safe impossible cases.
    // This method should be called for special ConfigItem-mode only!
    OSL_ENSURE( ((pImpl->nMode & CONFIG_MODE_ALL_LOCALES ) == CONFIG_MODE_ALL_LOCALES), "ConfigItem::impl_unpackLocalizedProperties()\nWrong call of this method detected!\n" );

    sal_Int32                   nSourceCounter      ;   // used to step during input lists
    sal_Int32                   nSourceSize         ;   // marks end of loop over input lists
    sal_Int32                   nDestinationCounter ;   // actual position in output lists
    sal_Int32                   nPropertyCounter    ;   // counter of inner loop for Sequence< PropertyValue >
    sal_Int32                   nPropertiesSize     ;   // marks end of inner loop
    OUString                    sNodeName           ;   // base name of node ( e.g. "UIName/" ) ... expand to locale ( e.g. "UIName/de" )
    Sequence< PropertyValue >   lProperties         ;   // localized values of an configuration entry getted from lInValues-Any

    // Optimise follow algorithm ... A LITTLE BIT :-)
    // There exist two different possibilities:
    //  i ) There exist no localized entries ...                        =>  size of lOutNames/lOutValues will be the same like lInNames/lInValues!
    //  ii) There exist some (mostly one or two) localized entries ...  =>  size of lOutNames/lOutValues will be some bytes greater then lInNames/lInValues.
    //  =>  I think we should make it fast for i). ii) is a special case and mustn't be SOOOO... fast.
    //      We should reserve same space for output list like input ones first.
    //      Follow algorithm looks for these borders and change it for ii) only!
    //      It will be faster then a "realloc()" call in every loop ...
    nSourceSize = lInNames.getLength();

    lOutNames.realloc   ( nSourceSize );
    lOutValues.realloc  ( nSourceSize );

    // Algorithm:
    // Copy all names and values from const to return lists.
    // Look for special localized entries ... You can detect it as Sequence< PropertyValue > packed into an Any.
    // Split it ... insert PropertyValue.Name to lOutNames and PropertyValue.Value to lOutValues.

    nDestinationCounter = 0;
    for( nSourceCounter=0; nSourceCounter<nSourceSize; ++nSourceCounter )
    {
        // If item a special localized one ... split it and insert his parts to output lists ...
        if( lInValues[nSourceCounter].getValueType() == ::getCppuType( (const Sequence< PropertyValue >*)NULL ) )
        {
            lInValues[nSourceCounter]   >>= lProperties             ;
            sNodeName               =   lInNames[nSourceCounter]    ;
            sNodeName               +=  C2U("/")                    ;
            nPropertiesSize         =   lProperties.getLength()     ;

            if( (nDestinationCounter+nPropertiesSize) > lOutNames.getLength() )
            {
                lOutNames.realloc   ( nDestinationCounter+nPropertiesSize );
                lOutValues.realloc  ( nDestinationCounter+nPropertiesSize );
            }

            for( nPropertyCounter=0; nPropertyCounter<nPropertiesSize; ++nPropertyCounter )
            {
                 lOutNames [nDestinationCounter] = sNodeName + lProperties[nPropertyCounter].Name   ;
                lOutValues[nDestinationCounter] = lProperties[nPropertyCounter].Value               ;
                ++nDestinationCounter;
            }
        }
        // ... or copy normal items to return lists directly.
        else
        {
            if( (nDestinationCounter+1) > lOutNames.getLength() )
            {
                lOutNames.realloc   ( nDestinationCounter+1 );
                lOutValues.realloc  ( nDestinationCounter+1 );
            }

            lOutNames [nDestinationCounter] = lInNames [nSourceCounter];
            lOutValues[nDestinationCounter] = lInValues[nSourceCounter];
            ++nDestinationCounter;
        }
    }
}
/* -----------------------------03.02.2003 14:44------------------------------

 ---------------------------------------------------------------------------*/
Sequence< sal_Bool > ConfigItem::GetReadOnlyStates(const com::sun::star::uno::Sequence< rtl::OUString >& rNames)
{
    // size of return list is fix!
    // Every item must match to length of incoming name list.
    sal_Int32 nCount = rNames.getLength();
    Sequence< sal_Bool > lStates(nCount);

    // We must be shure to return a valid information everytime!
    // Set default to non readonly ... similar to the configuration handling of this property.
    for (sal_Int32 i=0; i<nCount; ++i)
        lStates[i] = sal_False;

    // no access - no informations ...
    Reference< XHierarchicalNameAccess > xHierarchyAccess = GetTree();
    if (!xHierarchyAccess.is())
        return lStates;

    for (i=0; i<nCount; ++i)
    {
        try
        {
            if(pImpl->pManager->IsLocalConfigProvider() && lcl_IsLocalProperty(sSubTree, rNames[i]))
            {
                OSL_ENSURE(sal_False, "ConfigItem::IsReadonly()\nlocal mode seams to be used!?\n");
                continue;
            }

            OUString sName = rNames[i];
            OUString sPath;
            OUString sProperty;

            ::utl::splitLastFromConfigurationPath(sName,sPath,sProperty);
            if (!sPath.getLength() && !sProperty.getLength())
            {
                OSL_ENSURE(sal_False, "ConfigItem::IsReadonly()\nsplitt failed\n");
                continue;
            }

            Reference< XInterface >       xNode;
            Reference< XPropertySet >     xSet ;
            Reference< XPropertySetInfo > xInfo;
            if (sPath.getLength())
            {
                Any aNode = xHierarchyAccess->getByHierarchicalName(sPath);
                if (!(aNode >>= xNode) || !xNode.is())
                {
                    OSL_ENSURE(sal_False, "ConfigItem::IsReadonly()\nno set available\n");
                    continue;
                }
            }
            else
            {
                xNode = Reference< XInterface >(xHierarchyAccess, UNO_QUERY);
            }

        xSet = Reference< XPropertySet >(xNode, UNO_QUERY);
            if (xSet.is())
        {
            xInfo = xSet->getPropertySetInfo();
                OSL_ENSURE(xInfo.is(), "ConfigItem::IsReadonly()\ngetPropertySetInfo failed ...\n");
        }
            else
        {
               xInfo = Reference< XPropertySetInfo >(xNode, UNO_QUERY);
                OSL_ENSURE(xInfo.is(), "ConfigItem::IsReadonly()\nUNO_QUERY failed ...\n");
        }

            if (!xInfo.is())
            {
                OSL_ENSURE(sal_False, "ConfigItem::IsReadonly()\nno prop info available\n");
                continue;
            }

            Property aProp = xInfo->getPropertyByName(sProperty);
            lStates[i] = ((aProp.Attributes & PropertyAttribute::READONLY) == PropertyAttribute::READONLY);
        }
        catch(Exception&){}
    }

    return lStates;
}

/* -----------------------------29.08.00 15:10--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< Any > ConfigItem::GetProperties(const Sequence< OUString >& rNames)
{
    Sequence< Any > aRet(rNames.getLength());
    const OUString* pNames = rNames.getConstArray();
    Any* pRet = aRet.getArray();
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    if(xHierarchyAccess.is())
    {
        for(int i = 0; i < rNames.getLength(); i++)
        {
            try
            {
                if(pImpl->pManager->IsLocalConfigProvider() && lcl_IsLocalProperty(sSubTree, pNames[i]))
                {
                    OUString sProperty(sSubTree);
                    sProperty += C2U("/");
                    sProperty += pNames[i];
                    pRet[i] = pImpl->pManager->GetLocalProperty(sProperty);
                }
                else
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
                OSL_ENSURE(sal_False, sMsg.getStr());
            }
#else
            catch(Exception&){}
#endif
        }

        // In special mode "ALL_LOCALES" we must convert localized values to Sequence< PropertyValue >.
        if((pImpl->nMode & CONFIG_MODE_ALL_LOCALES ) == CONFIG_MODE_ALL_LOCALES)
        {
            Sequence< Any > lValues;
            impl_packLocalizedProperties( rNames, aRet, lValues );
            aRet = lValues;
        }
    }
    return aRet;
}
/* -----------------------------29.08.00 17:28--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::PutProperties( const Sequence< OUString >& rNames,
                                                const Sequence< Any>& rValues)
{
    ValueCounter_Impl aCounter(pImpl->nInValueChange);
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    Reference<XNameReplace> xTopNodeReplace(xHierarchyAccess, UNO_QUERY);
    sal_Bool bRet = xHierarchyAccess.is() && xTopNodeReplace.is();
    if(bRet)
    {
        Sequence< OUString >    lNames          ;
        Sequence< Any >         lValues         ;
        const OUString*         pNames  = NULL  ;
        const Any*              pValues = NULL  ;
        sal_Int32               nNameCount      ;
        if(( pImpl->nMode & CONFIG_MODE_ALL_LOCALES ) == CONFIG_MODE_ALL_LOCALES )
        {
            // If ConfigItem works in "ALL_LOCALES"-mode ... we must support a Sequence< PropertyValue >
            // as value of an localized configuration entry!
            // How we can do that?
            // We must split all PropertyValues to "Sequence< OUString >" AND "Sequence< Any >"!
            impl_unpackLocalizedProperties( rNames, rValues, lNames, lValues );
            pNames      = lNames.getConstArray  ();
            pValues     = lValues.getConstArray ();
            nNameCount  = lNames.getLength      ();
        }
        else
        {
            // This is the normal mode ...
            // Use given input lists directly.
            pNames      = rNames.getConstArray  ();
            pValues     = rValues.getConstArray ();
            nNameCount  = rNames.getLength      ();
        }
        for(int i = 0; i < nNameCount; i++)
        {
            if(pImpl->pManager->IsLocalConfigProvider() && lcl_IsLocalProperty(sSubTree, pNames[i]))
            {
                OUString sProperty(sSubTree);
                sProperty += C2U("/");
                sProperty += pNames[i];
                pImpl->pManager->PutLocalProperty(sProperty, pValues[i]);
            }
            else
            {
                try
                {
                    OUString sNode, sProperty;
                    if (splitLastFromConfigurationPath(pNames[i],sNode, sProperty))
                    {
                        Any aNode = xHierarchyAccess->getByHierarchicalName(sNode);

                        Reference<XNameAccess> xNodeAcc;
                        aNode >>= xNodeAcc;
                        Reference<XNameReplace>   xNodeReplace(xNodeAcc, UNO_QUERY);
                        Reference<XNameContainer> xNodeCont   (xNodeAcc, UNO_QUERY);

                        sal_Bool bExist = (xNodeAcc.is() && xNodeAcc->hasByName(sProperty));
                        if (bExist && xNodeReplace.is())
                            xNodeReplace->replaceByName(sProperty, pValues[i]);
                        else
                        if (!bExist && xNodeCont.is())
                            xNodeCont->insertByName(sProperty, pValues[i]);
                        else
                            bRet = sal_False;
                    }
                    else //direct value
                    {
                        xTopNodeReplace->replaceByName(sProperty, pValues[i]);
                    }
                }
#ifdef DBG_UTIL
                catch(Exception& rEx)
                {
                    lcl_CFG_DBG_EXCEPTION("Exception from PutProperties: ", rEx);
                }
#else
                catch(Exception&){}
#endif
            }
        }
        try
        {
            Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
            xBatch->commitChanges();
        }
        CATCH_INFO("Exception from commitChanges(): ")
    }

    return bRet;
}
/* -----------------------------29.08.00 16:19--------------------------------

 ---------------------------------------------------------------------------*/
#if SUPD<637
sal_Bool    ConfigItem::EnableNotification(const Sequence< OUString >& rNames)
{
    return EnableNotification(rNames, sal_False);
}
#endif
sal_Bool    ConfigItem::EnableNotification(const Sequence< OUString >& rNames,
                sal_Bool bEnableInternalNotification )

{
    OSL_ENSURE(0 == (pImpl->nMode&CONFIG_MODE_RELEASE_TREE), "notification in CONFIG_MODE_RELEASE_TREE mode not possible");
    pImpl->bEnableInternalNotification = bEnableInternalNotification;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    Reference<XChangesNotifier> xChgNot(xHierarchyAccess, UNO_QUERY);
    if(!xChgNot.is())
        return sal_False;

    OSL_ENSURE(!xChangeLstnr.is(), "EnableNotification already called");
    if(xChangeLstnr.is())
        xChgNot->removeChangesListener( xChangeLstnr );
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
    Reference<XChangesNotifier> xChgNot(m_xHierarchyAccess, UNO_QUERY);
    if(xChgNot.is() && xChangeLstnr.is())
    {
        try
        {
            xChgNot->removeChangesListener( xChangeLstnr );
        }
        catch(Exception & )
        {
        }
    }
}
/* -----------------------------10.07.00      --------------------------------

 ---------------------------------------------------------------------------*/
void lcl_normalizeLocalNames(Sequence< OUString >& _rNames, ConfigNameFormat _eFormat, Reference<XInterface> const& _xParentNode)
{
    switch (_eFormat)
    {
    case CONFIG_NAME_LOCAL_NAME:
        // unaltered - this is our input format
        break;

    case CONFIG_NAME_FULL_PATH:
        {
            Reference<XHierarchicalName> xFormatter(_xParentNode, UNO_QUERY);
            if (xFormatter.is())
            {
                OUString * pNames = _rNames.getArray();
                for(int i = 0; i<_rNames.getLength(); ++i)
                try
                {
                    pNames[i] = xFormatter->composeHierarchicalName(pNames[i]);
                }
                CATCH_INFO("Exception from composeHierarchicalName(): ")
                break;
            }
        }
        OSL_ENSURE(false, "Cannot create absolute pathes: missing interface");
        // make local pathes instaed

    case CONFIG_NAME_LOCAL_PATH:
        {
            Reference<XTemplateContainer> xTypeContainer(_xParentNode, UNO_QUERY);
            if (xTypeContainer.is())
            {
                OUString sTypeName = xTypeContainer->getElementTemplateName();
                sTypeName = sTypeName.copy(sTypeName.lastIndexOf('/')+1);

                OUString * pNames = _rNames.getArray();
                for(int i = 0; i<_rNames.getLength(); ++i)
                {
                    pNames[i] = wrapConfigurationElementName(pNames[i],sTypeName);
                }
            }
            else
            {
                static const OUString sSetService(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.SetAccess"));
                Reference<XServiceInfo> xSVI(_xParentNode, UNO_QUERY);
                if (xSVI.is() && xSVI->supportsService(sSetService))
                {
                    OUString * pNames = _rNames.getArray();
                    for(int i = 0; i<_rNames.getLength(); ++i)
                    {
                        pNames[i] = wrapConfigurationElementName(pNames[i]);
                    }
                }
            }
        }
        break;

    case CONFIG_NAME_PLAINTEXT_NAME:
        {
            Reference<XStringEscape> xEscaper(_xParentNode, UNO_QUERY);
            if (xEscaper.is())
            {
                OUString * pNames = _rNames.getArray();
                for(int i = 0; i<_rNames.getLength(); ++i)
                try
                {
                    pNames[i] = xEscaper->unescapeString(pNames[i]);
                }
                CATCH_INFO("Exception from unescapeString(): ")
            }
        }
        break;

    }
}
/* -----------------------------10.07.00      --------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > ConfigItem::GetNodeNames(const OUString& rNode)
{
    ConfigNameFormat const eDefaultFormat = CONFIG_NAME_LOCAL_NAME; // CONFIG_NAME_DEFAULT;

    return GetNodeNames(rNode, eDefaultFormat);
}
/* -----------------------------15.09.00 12:06--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > ConfigItem::GetNodeNames(const OUString& rNode, ConfigNameFormat eFormat)
{
    Sequence< OUString > aRet;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
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
                lcl_normalizeLocalNames(aRet,eFormat,xCont);
            }

        }
#ifdef DBG_UTIL
        catch(Exception& rEx)
        {
            lcl_CFG_DBG_EXCEPTION("Exception from GetNodeNames: ", rEx);
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
    ValueCounter_Impl aCounter(pImpl->nInValueChange);
    sal_Bool bRet = sal_False;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
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
                }
                CATCH_INFO("Exception from removeByName(): ")
            }
            xBatch->commitChanges();
            bRet = sal_True;
        }
        CATCH_INFO("Exception from ClearNodeSet")
    }
    return bRet;
}
/* -----------------------------24.11.00 10:58--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::ClearNodeElements(const OUString& rNode, Sequence< OUString >& rElements)
{
    ValueCounter_Impl aCounter(pImpl->nInValueChange);
    sal_Bool bRet = sal_False;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
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
            try
            {
                for(sal_Int32 nElement = 0; nElement < rElements.getLength(); nElement++)
                {
                    xCont->removeByName(pElements[nElement]);
                }
                Reference<XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
                xBatch->commitChanges();
            }
            CATCH_INFO("Exception from commitChanges(): ")
            bRet = sal_True;
        }
        CATCH_INFO("Exception from GetNodeNames: ")
    }
    return bRet;
}
//----------------------------------------------------------------------------
static inline
OUString lcl_extractSetPropertyName( const OUString& rInPath, const OUString& rPrefix )
{
    OUString const sSubPath = dropPrefixFromConfigurationPath( rInPath, rPrefix);
    return extractFirstFromConfigurationPath( sSubPath );
}
//----------------------------------------------------------------------------
static
Sequence< OUString > lcl_extractSetPropertyNames( const Sequence< PropertyValue >& rValues, const OUString& rPrefix )
{
    const PropertyValue* pProperties = rValues.getConstArray();

    Sequence< OUString > aSubNodeNames(rValues.getLength());
    OUString* pSubNodeNames = aSubNodeNames.getArray();

    OUString sLastSubNode;
    sal_Int32 nSubIndex = 0;

    for(sal_Int32 i = 0; i < rValues.getLength(); i++)
    {
        OUString const sSubPath = dropPrefixFromConfigurationPath( pProperties[i].Name, rPrefix);
        OUString const sSubNode = extractFirstFromConfigurationPath( sSubPath );

        if(sLastSubNode != sSubNode)
        {
            pSubNodeNames[nSubIndex++] = sSubNode;
        }

        sLastSubNode = sSubNode;
    }
    aSubNodeNames.realloc(nSubIndex);

    return aSubNodeNames;
}
/* -----------------------------15.09.00 15:52--------------------------------
    add or change properties
 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::SetSetProperties(
    const OUString& rNode, Sequence< PropertyValue > rValues)
{
    ValueCounter_Impl aCounter(pImpl->nInValueChange);
    sal_Bool bRet;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
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

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);

            if(xFac.is())
            {
                const Sequence< OUString > aSubNodeNames = lcl_extractSetPropertyNames(rValues, rNode);

                const sal_Int32 nSubNodeCount = aSubNodeNames.getLength();

                for(sal_Int32 j = 0; j <nSubNodeCount ; j++)
                {
                    if(!xCont->hasByName(aSubNodeNames[j]))
                    {
                        Reference<XInterface> xInst = xFac->createInstance();
                        Any aVal; aVal <<= xInst;
                        xCont->insertByName(aSubNodeNames[j], aVal);
                    }
                    //set values
                }
                try
                {
                    xBatch->commitChanges();
                }
                CATCH_INFO("Exception from commitChanges(): ")

                const PropertyValue* pProperties = rValues.getConstArray();

                Sequence< OUString > aSetNames(rValues.getLength());
                OUString* pSetNames = aSetNames.getArray();

                Sequence< Any> aSetValues(rValues.getLength());
                Any* pSetValues = aSetValues.getArray();

                sal_Bool bEmptyNode = rNode.getLength() == 0;
                for(sal_Int32 k = 0; k < rValues.getLength(); k++)
                {
                    pSetNames[k] =  pProperties[k].Name.copy( bEmptyNode ? 1 : 0);
                    pSetValues[k] = pProperties[k].Value;
                }
                bRet = PutProperties(aSetNames, aSetValues);
            }
            else
            {
                //if no factory is available then the node contains basic data elements
                const PropertyValue* pValues = rValues.getConstArray();
                for(int nValue = 0; nValue < rValues.getLength();nValue++)
                {
                    try
                    {
                        OUString sSubNode = lcl_extractSetPropertyName( pValues[nValue].Name, rNode );

                        if(xCont->hasByName(sSubNode))
                            xCont->replaceByName(sSubNode, pValues[nValue].Value);
                        else
                            xCont->insertByName(sSubNode, pValues[nValue].Value);

                        OSL_ENSURE( xHierarchyAccess->hasByHierarchicalName(pValues[nValue].Name),
                            "Invalid config path" );
                    }
                    CATCH_INFO("Exception form insert/replaceByName(): ")
                }
                xBatch->commitChanges();
            }
        }
#ifdef DBG_UTIL
        catch(Exception& rEx)
        {
            lcl_CFG_DBG_EXCEPTION("Exception from SetSetProperties: ", rEx);
#else
        catch(Exception&)
        {
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
    ValueCounter_Impl aCounter(pImpl->nInValueChange);
    sal_Bool bRet = sal_True;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
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

            // JB: Change: now the same name handling for sets of simple values
            const Sequence< OUString > aSubNodeNames = lcl_extractSetPropertyNames(rValues, rNode);
            const OUString* pSubNodeNames = aSubNodeNames.getConstArray();
            const sal_Int32 nSubNodeCount = aSubNodeNames.getLength();

            //remove unknown members first
            {
                const Sequence<OUString> aContainerSubNodes = xCont->getElementNames();
                const OUString* pContainerSubNodes = aContainerSubNodes.getConstArray();

                for(sal_Int32 nContSub = 0; nContSub < aContainerSubNodes.getLength(); nContSub++)
                {
                    sal_Bool bFound = sal_False;
                    for(sal_Int32 j = 0; j < nSubNodeCount; j++)
                    {
                        if(pSubNodeNames[j] == pContainerSubNodes[nContSub])
                        {
                            bFound = sal_True;
                            break;
                        }
                    }
                    if(!bFound)
                    {
                        xCont->removeByName(pContainerSubNodes[nContSub]);
                    }
                }
                try { xBatch->commitChanges(); }
                CATCH_INFO("Exception from commitChanges(): ")
            }

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);
            if(xFac.is())
            {
                for(sal_Int32 j = 0; j < nSubNodeCount; j++)
                {
                    if(!xCont->hasByName(pSubNodeNames[j]))
                    {
                        //create if not available
                        Reference<XInterface> xInst = xFac->createInstance();
                        Any aVal; aVal <<= xInst;
                        xCont->insertByName(pSubNodeNames[j], aVal);
                    }
                }
                try { xBatch->commitChanges(); }
                CATCH_INFO("Exception from commitChanges(): ")

                const PropertyValue* pProperties = rValues.getConstArray();

                Sequence< OUString > aSetNames(rValues.getLength());
                OUString* pSetNames = aSetNames.getArray();

                Sequence< Any> aSetValues(rValues.getLength());
                Any* pSetValues = aSetValues.getArray();

                sal_Bool bEmptyNode = rNode.getLength() == 0;
                for(sal_Int32 k = 0; k < rValues.getLength(); k++)
                {
                    pSetNames[k] =  pProperties[k].Name.copy( bEmptyNode ? 1 : 0);
                    pSetValues[k] = pProperties[k].Value;
                }
                bRet = PutProperties(aSetNames, aSetValues);
            }
            else
            {
                const PropertyValue* pValues = rValues.getConstArray();

                //if no factory is available then the node contains basic data elements
                for(int nValue = 0; nValue < rValues.getLength();nValue++)
                {
                    try
                    {
                        OUString sSubNode = lcl_extractSetPropertyName( pValues[nValue].Name, rNode );

                        if(xCont->hasByName(sSubNode))
                            xCont->replaceByName(sSubNode, pValues[nValue].Value);
                        else
                            xCont->insertByName(sSubNode, pValues[nValue].Value);
                    }
                    CATCH_INFO("Exception from insert/replaceByName(): ");
                }
                xBatch->commitChanges();
            }
        }
#ifdef DBG_UTIL
        catch(Exception& rEx)
        {
            lcl_CFG_DBG_EXCEPTION("Exception from ReplaceSetProperties: ", rEx);
#else
        catch(Exception&)
        {
#endif
            bRet = sal_False;
        }
    }
    return bRet;
}
/* -----------------------------07.05.01 12:15--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::getUniqueSetElementName( const ::rtl::OUString& _rSetNode, ::rtl::OUString& _rName)
{
    ::rtl::OUString sNewElementName;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
    sal_Bool bRet = sal_False;
    if(xHierarchyAccess.is())
    {
        try
        {
            Reference< XNameAccess > xSetNode;
            xHierarchyAccess->getByHierarchicalName(_rSetNode) >>= xSetNode;
            if (xSetNode.is())
            {
                const sal_uInt32 nPrime = 65521;                            // a prime number
                const sal_uInt32 nPrimeLess2 = nPrime - 2;
                sal_uInt32 nEngendering     = (rand() % nPrimeLess2) + 2;   // the engendering of the field

                // the element which will loop through the field
                sal_uInt32 nFieldElement = nEngendering;

                for (; 1 != nFieldElement; nFieldElement = (nFieldElement * nEngendering) % nPrime)
                {
                    ::rtl::OUString sThisRoundTrial = _rName;
                    sThisRoundTrial += ::rtl::OUString::valueOf((sal_Int32)nFieldElement);

                    if (!xSetNode->hasByName(sThisRoundTrial))
                    {
                        _rName = sThisRoundTrial;
                        bRet =  sal_True;
                        break;
                    }
                }
            }
        }
        CATCH_INFO("Exception from getUniqueSetElementName(): ")
    }
    return bRet;
}
/* -----------------------------23.01.01 12:49--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::AddNode(const rtl::OUString& rNode, const rtl::OUString& rNewNode)
{
    ValueCounter_Impl aCounter(pImpl->nInValueChange);
    sal_Bool bRet;
    Reference<XHierarchicalNameAccess> xHierarchyAccess = GetTree();
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

            Reference<XSingleServiceFactory> xFac(xCont, UNO_QUERY);

            if(xFac.is())
            {
                if(!xCont->hasByName(rNewNode))
                {
                    Reference<XInterface> xInst = xFac->createInstance();
                    Any aVal; aVal <<= xInst;
                    xCont->insertByName(rNewNode, aVal);
                }
                try
                {
                    xBatch->commitChanges();
                }
                CATCH_INFO("Exception from commitChanges(): ")
            }
            else
            {
                //if no factory is available then the node contains basic data elements
                try
                {
                    if(!xCont->hasByName(rNewNode))
                        xCont->insertByName(rNewNode, Any());
                }
                CATCH_INFO("Exception from AddNode(): ")
            }
            xBatch->commitChanges();
        }
#ifdef DBG_UTIL
        catch(Exception& rEx)
        {
            lcl_CFG_DBG_EXCEPTION("Exception from AddNode(): ", rEx);
#else
        catch(Exception&)
        {
#endif
            bRet = sal_False;
        }
    }
    return bRet;
}
/* -----------------------------12.02.01 11:38--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16   ConfigItem::GetMode() const
{
    return pImpl->nMode;
}
/* -----------------------------12.02.01 13:31--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigItem::SetModified()
{
    pImpl->bIsModified = sal_True;
}
/* -----------------------------05.05.01 14:07--------------------------------

 ---------------------------------------------------------------------------*/
void    ConfigItem::ClearModified()
{
    pImpl->bIsModified = sal_False;
}
/* -----------------------------12.02.01 13:31--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::IsModified() const
{
    return pImpl->bIsModified;
}
/* -----------------------------12.02.01 13:33--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool ConfigItem::IsInValueChange() const
{
    return pImpl->nInValueChange > 0;
}
/* -----------------------------21.06.01 12:26--------------------------------

 ---------------------------------------------------------------------------*/
Reference< XHierarchicalNameAccess> ConfigItem::GetTree()
{
    Reference< XHierarchicalNameAccess> xRet;
    if(!m_xHierarchyAccess.is())
        xRet = pImpl->pManager->AcquireTree(*this);
    else
        xRet = m_xHierarchyAccess;
    OSL_ENSURE(xRet.is(), "AcquireTree failed");
    return xRet;
}
/* -----------------------------22.06.01 08:42--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigItem::LockTree()
{
    OSL_ENSURE(0 != (pImpl->nMode&CONFIG_MODE_RELEASE_TREE), "call LockTree in CONFIG_MODE_RELEASE_TREE mode, only");
    m_xHierarchyAccess = GetTree();
}
/* -----------------------------22.06.01 08:42--------------------------------

 ---------------------------------------------------------------------------*/
void ConfigItem::UnlockTree()
{
    OSL_ENSURE(0 != (pImpl->nMode&CONFIG_MODE_RELEASE_TREE), "call UnlockTree in CONFIG_MODE_RELEASE_TREE mode, only");
    if(0 != (pImpl->nMode&CONFIG_MODE_RELEASE_TREE))
        m_xHierarchyAccess = 0;
}



