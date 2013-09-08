/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <services/pathsettings.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <services.h>

#include "helper/mischelper.hxx"

#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XProperty.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/PathSubstitution.hpp>

#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/configpaths.hxx>

#include <fwkdllapi.h>


#define CFGPROP_USERPATHS OUString("UserPaths")
#define CFGPROP_WRITEPATH OUString("WritePath")

/*
    0 : old style              "Template"              string using ";" as separator
    1 : internal paths         "Template_internal"     string list
    2 : user paths             "Template_user"         string list
    3 : write path             "Template_write"        string
 */

#define POSTFIX_INTERNAL_PATHS OUString("_internal")
#define POSTFIX_USER_PATHS OUString("_user")
#define POSTFIX_WRITE_PATH OUString("_writable")

const sal_Int32 IDGROUP_OLDSTYLE        = 0;
const sal_Int32 IDGROUP_INTERNAL_PATHS = 1;
const sal_Int32 IDGROUP_USER_PATHS     = 2;
const sal_Int32 IDGROUP_WRITE_PATH      = 3;

const sal_Int32 IDGROUP_COUNT           = 4;

sal_Int32 impl_getPropGroup(sal_Int32 nID)
{
    return (nID % IDGROUP_COUNT);
}


#define SERVICENAME_PATHSETTINGS                                DECLARE_ASCII("com.sun.star.util.PathSettings"                      )
#define IMPLEMENTATIONNAME_PATHSETTINGS                         DECLARE_ASCII("com.sun.star.comp.framework.PathSettings"            )

namespace framework
{

//-----------------------------------------------------------------------------
// XInterface, XTypeProvider, XServiceInfo

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2(   PathSettings                                            ,
                                            ::cppu::OWeakObject                                     ,
                                            SERVICENAME_PATHSETTINGS                                ,
                                            IMPLEMENTATIONNAME_PATHSETTINGS
                                        )

DEFINE_INIT_SERVICE                     (   PathSettings,
                                            {
                                                /*Attention
                                                    I think we don't need any mutex or lock here ... because we are called by our own static method impl_createInstance()
                                                    to create a new instance of this class by our own supported service factory.
                                                    see macro DEFINE_XSERVICEINFO_MULTISERVICE and "impl_initService()" for further information!
                                                */

                                                // fill cache
                                                impl_readAll();
                                            }
                                        )

//-----------------------------------------------------------------------------
PathSettings::PathSettings( const css::uno::Reference< css::uno::XComponentContext >& xContext )
    //  Init baseclasses first
    //  Attention: Don't change order of initialization!
    //      ThreadHelpBase is a struct with a lock as member. We can't use a lock as direct member!
    //      We must garant right initialization and a valid value of this to initialize other baseclasses!
    :   ThreadHelpBase()
    ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >(m_aLock.getShareableOslMutex())
    ,   ::cppu::OPropertySetHelper(*(static_cast< ::cppu::OBroadcastHelper* >(this)))
    // Init member
    ,   m_xContext (xContext)
    ,   m_pPropHelp(0    )
    ,  m_bIgnoreEvents(sal_False)
{
}

//-----------------------------------------------------------------------------
PathSettings::~PathSettings()
{
    css::uno::Reference< css::util::XChangesNotifier > xBroadcaster(m_xCfgNew, css::uno::UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->removeChangesListener(m_xCfgNewListener);
    if (m_pPropHelp)
       delete m_pPropHelp;
}

//------------------------------------------------------------------
css::uno::Any SAL_CALL PathSettings::queryInterface( const css::uno::Type& _rType )
    throw(css::uno::RuntimeException)
{
    css::uno::Any aRet = PathSettings_BASE::queryInterface( _rType );
    if ( !aRet.hasValue() )
        aRet = OPropertySetHelper::queryInterface( _rType );
    return aRet;
}

//------------------------------------------------------------------------------
css::uno::Sequence< css::uno::Type > SAL_CALL PathSettings::getTypes(  )
    throw(css::uno::RuntimeException)
{
    return comphelper::concatSequences(
        PathSettings_BASE::getTypes(),
        ::cppu::OPropertySetHelper::getTypes()
    );
}

//-----------------------------------------------------------------------------
void SAL_CALL PathSettings::changesOccurred(const css::util::ChangesEvent& aEvent)
    throw (css::uno::RuntimeException)
{
    sal_Int32 c                 = aEvent.Changes.getLength();
    sal_Int32 i                 = 0;
    sal_Bool  bUpdateDescriptor = sal_False;

    for (i=0; i<c; ++i)
    {
        const css::util::ElementChange& aChange = aEvent.Changes[i];

        OUString sChanged;
        aChange.Accessor >>= sChanged;

        OUString sPath = ::utl::extractFirstFromConfigurationPath(sChanged);
        if (!sPath.isEmpty())
        {
            PathSettings::EChangeOp eOp = impl_updatePath(sPath, sal_True);
            if (
                (eOp == PathSettings::E_ADDED  ) ||
                (eOp == PathSettings::E_REMOVED)
               )
                bUpdateDescriptor = sal_True;
        }
    }

    if (bUpdateDescriptor)
        impl_rebuildPropertyDescriptor();
}

//-----------------------------------------------------------------------------
void SAL_CALL PathSettings::disposing(const css::lang::EventObject& aSource)
    throw(css::uno::RuntimeException)
{
    WriteGuard aWriteLock(m_aLock);

    if (aSource.Source == m_xCfgNew)
        m_xCfgNew.clear();

    aWriteLock.unlock();
}

//-----------------------------------------------------------------------------
OUString PathSettings::getStringProperty(const OUString& p1)
    throw(css::uno::RuntimeException)
{
    css::uno::Any a = ::cppu::OPropertySetHelper::getPropertyValue(p1);
    OUString s;
    a >>= s;
    return s;
}

//-----------------------------------------------------------------------------
void PathSettings::setStringProperty(const OUString& p1, const OUString& p2)
    throw(css::uno::RuntimeException)
{
    ::cppu::OPropertySetHelper::setPropertyValue(p1, css::uno::Any(p2));
}

//-----------------------------------------------------------------------------
void PathSettings::impl_readAll()
{
    try
    {
        // TODO think about me
        css::uno::Reference< css::container::XNameAccess > xCfg    = fa_getCfgNew();
        css::uno::Sequence< OUString >              lPaths = xCfg->getElementNames();

        sal_Int32 c = lPaths.getLength();
        for (sal_Int32 i = 0; i < c; ++i)
        {
            const OUString& sPath = lPaths[i];
            impl_updatePath(sPath, sal_False);
        }
    }
    catch(const css::uno::RuntimeException& )
    {
    }

    impl_rebuildPropertyDescriptor();
}

//-----------------------------------------------------------------------------
// NO substitution here ! It's done outside ...
OUStringList PathSettings::impl_readOldFormat(const OUString& sPath)
{
    css::uno::Reference< css::container::XNameAccess > xCfg( fa_getCfgOld() );
    OUStringList aPathVal;

    if( xCfg->hasByName(sPath) )
    {
        css::uno::Any aVal( xCfg->getByName(sPath) );

        OUString                       sStringVal;
        css::uno::Sequence< OUString > lStringListVal;

        if (aVal >>= sStringVal)
        {
            aPathVal.push_back(sStringVal);
        }
        else if (aVal >>= lStringListVal)
        {
            aPathVal << lStringListVal;
        }
    }

    return aPathVal;
}

//-----------------------------------------------------------------------------
// NO substitution here ! It's done outside ...
PathSettings::PathInfo PathSettings::impl_readNewFormat(const OUString& sPath)
{
    const OUString CFGPROP_INTERNALPATHS("InternalPaths");
    const OUString CFGPROP_ISSINGLEPATH("IsSinglePath");

    css::uno::Reference< css::container::XNameAccess > xCfg = fa_getCfgNew();

    // get access to the "queried" path
    css::uno::Reference< css::container::XNameAccess > xPath;
    xCfg->getByName(sPath) >>= xPath;

    PathSettings::PathInfo aPathVal;

    // read internal path list
    css::uno::Reference< css::container::XNameAccess > xIPath;
    xPath->getByName(CFGPROP_INTERNALPATHS) >>= xIPath;
    aPathVal.lInternalPaths << xIPath->getElementNames();

    // read user defined path list
    aPathVal.lUserPaths << xPath->getByName(CFGPROP_USERPATHS);

    // read the writeable path
    xPath->getByName(CFGPROP_WRITEPATH) >>= aPathVal.sWritePath;

    // read state props
    xPath->getByName(CFGPROP_ISSINGLEPATH) >>= aPathVal.bIsSinglePath;

    // analyze finalized/mandatory states
    aPathVal.bIsReadonly = sal_False;
    css::uno::Reference< css::beans::XProperty > xInfo(xPath, css::uno::UNO_QUERY);
    if (xInfo.is())
    {
        css::beans::Property aInfo = xInfo->getAsProperty();
        sal_Bool bFinalized = ((aInfo.Attributes & css::beans::PropertyAttribute::READONLY  ) == css::beans::PropertyAttribute::READONLY  );

        // Note: Till we support finalized / mandatory on our API more in detail we handle
        // all states simple as READONLY ! But because all realy needed paths are "mandatory" by default
        // we have to handle "finalized" as the real "readonly" indicator .
        aPathVal.bIsReadonly = bFinalized;
    }

    return aPathVal;
}

//-----------------------------------------------------------------------------
void PathSettings::impl_storePath(const PathSettings::PathInfo& aPath)
{
    m_bIgnoreEvents = sal_True;

    css::uno::Reference< css::container::XNameAccess > xCfgNew = fa_getCfgNew();
    css::uno::Reference< css::container::XNameAccess > xCfgOld = fa_getCfgOld();

    // try to replace path-parts with well known and uspported variables.
    // So an office can be moved easialy to another location without loosing
    // it's related paths.
    PathInfo aResubstPath(aPath);
    impl_subst(aResubstPath, sal_True);

    // update new configuration
    if (! aResubstPath.bIsSinglePath)
    {
        ::comphelper::ConfigurationHelper::writeRelativeKey(xCfgNew,
                                                            aResubstPath.sPathName,
                                                            CFGPROP_USERPATHS,
                                                            css::uno::makeAny(aResubstPath.lUserPaths.getAsConstList()));
    }

    ::comphelper::ConfigurationHelper::writeRelativeKey(xCfgNew,
                                                        aResubstPath.sPathName,
                                                        CFGPROP_WRITEPATH,
                                                        css::uno::makeAny(aResubstPath.sWritePath));

    ::comphelper::ConfigurationHelper::flush(xCfgNew);

    // remove the whole path from the old configuration !
    // Otherwise we cant make sure that the diff between new and old configuration
    // on loading time realy represent an user setting !!!

    // Check if the given path exists inside the old configuration.
    // Because our new configuration knows more then the list of old paths ... !
    if (xCfgOld->hasByName(aResubstPath.sPathName))
    {
        css::uno::Reference< css::beans::XPropertySet > xProps(xCfgOld, css::uno::UNO_QUERY_THROW);
        xProps->setPropertyValue(aResubstPath.sPathName, css::uno::Any());
        ::comphelper::ConfigurationHelper::flush(xCfgOld);
    }

    m_bIgnoreEvents = sal_False;
}

//-----------------------------------------------------------------------------
void PathSettings::impl_mergeOldUserPaths(      PathSettings::PathInfo& rPath,
                                          const OUStringList&           lOld )
{
    OUStringList::const_iterator pIt;
    for (  pIt  = lOld.begin();
           pIt != lOld.end()  ;
         ++pIt                )
    {
        const OUString& sOld = *pIt;

        if (rPath.bIsSinglePath)
        {
            LOG_ASSERT2(lOld.size()>1, "PathSettings::impl_mergeOldUserPaths()", "Single path has more then one path value inside old configuration (Common.xcu)!")
            if (! rPath.sWritePath.equals(sOld))
               rPath.sWritePath = sOld;
        }
        else
        {
            if (
                (  rPath.lInternalPaths.findConst(sOld) == rPath.lInternalPaths.end()) &&
                (  rPath.lUserPaths.findConst(sOld)     == rPath.lUserPaths.end()    ) &&
                (! rPath.sWritePath.equals(sOld)                                     )
               )
               rPath.lUserPaths.push_back(sOld);
        }
    }
}

//-----------------------------------------------------------------------------
PathSettings::EChangeOp PathSettings::impl_updatePath(const OUString& sPath          ,
                                                            sal_Bool         bNotifyListener)
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    PathSettings::PathInfo* pPathOld = 0;
    PathSettings::PathInfo* pPathNew = 0;
    PathSettings::EChangeOp eOp      = PathSettings::E_UNDEFINED;
    PathSettings::PathInfo  aPath;

    try
    {
        aPath = impl_readNewFormat(sPath);
        aPath.sPathName = sPath;
        // replace all might existing variables with real values
        // Do it before these old paths will be compared against the
        // new path configuration. Otherwise some striungs uses different variables ... but substitution
        // will produce strings with same content (because some variables are redundant!)
        impl_subst(aPath, sal_False);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::container::NoSuchElementException&)
        { eOp = PathSettings::E_REMOVED; }
    catch(const css::uno::Exception&)
        { throw; }

    try
    {
        // migration of old user defined values on demand
        // can be disabled for a new major
        OUStringList lOldVals = impl_readOldFormat(sPath);
        // replace all might existing variables with real values
        // Do it before these old paths will be compared against the
        // new path configuration. Otherwise some striungs uses different variables ... but substitution
        // will produce strings with same content (because some variables are redundant!)
        impl_subst(lOldVals, fa_getSubstitution(), sal_False);
        impl_mergeOldUserPaths(aPath, lOldVals);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    // Normal(!) exceptions can be ignored!
    // E.g. in case an addon installs a new path, which was not well known for an OOo 1.x installation
    // we cant find a value for it inside the "old" configuration. So a NoSuchElementException
    // will be normal .-)
    catch(const css::uno::Exception&)
        {}

    PathSettings::PathHash::iterator pPath = m_lPaths.find(sPath);
    if (eOp == PathSettings::E_UNDEFINED)
    {
        if (pPath != m_lPaths.end())
            eOp = PathSettings::E_CHANGED;
        else
            eOp = PathSettings::E_ADDED;
    }

    switch(eOp)
    {
        case PathSettings::E_ADDED :
             {
                if (bNotifyListener)
                {
                    pPathOld = 0;
                    pPathNew = &aPath;
                    impl_notifyPropListener(eOp, sPath, pPathOld, pPathNew);
                }
                m_lPaths[sPath] = aPath;
             }
             break;

        case PathSettings::E_CHANGED :
             {
                if (bNotifyListener)
                {
                    pPathOld = &(pPath->second);
                    pPathNew = &aPath;
                    impl_notifyPropListener(eOp, sPath, pPathOld, pPathNew);
                }
                m_lPaths[sPath] = aPath;
             }
             break;

        case PathSettings::E_REMOVED :
             {
                if (pPath != m_lPaths.end())
                {
                    if (bNotifyListener)
                    {
                        pPathOld = &(pPath->second);
                        pPathNew = 0;
                        impl_notifyPropListener(eOp, sPath, pPathOld, pPathNew);
                    }
                    m_lPaths.erase(pPath);
                }
             }
             break;

        default: // to let compiler be happy
             break;
    }

    return eOp;
}

//-----------------------------------------------------------------------------
css::uno::Sequence< sal_Int32 > PathSettings::impl_mapPathName2IDList(const OUString& sPath)
{
    OUString sOldStyleProp = sPath;
    OUString sInternalProp = sPath+POSTFIX_INTERNAL_PATHS;
    OUString sUserProp     = sPath+POSTFIX_USER_PATHS;
    OUString sWriteProp    = sPath+POSTFIX_WRITE_PATH;

    // Attention: The default set of IDs is fix and must follow these schema.
    // Otherwhise the outside code ant work for new added properties.
    // Why ?
    // The outside code must fire N events for every changed property.
    // And the knowing about packaging of variables of the structure PathInfo
    // follow these group IDs ! But if such ID isnt in the range of [0..IDGROUP_COUNT]
    // the outside cant determine the right group ... and cant fire the right events .-)

    css::uno::Sequence< sal_Int32 > lIDs(IDGROUP_COUNT);
    lIDs[0] = IDGROUP_OLDSTYLE       ;
    lIDs[1] = IDGROUP_INTERNAL_PATHS;
    lIDs[2] = IDGROUP_USER_PATHS    ;
    lIDs[3] = IDGROUP_WRITE_PATH     ;

    sal_Int32 c = m_lPropDesc.getLength();
    sal_Int32 i = 0;
    for (i=0; i<c; ++i)
    {
        const css::beans::Property& rProp = m_lPropDesc[i];

        if (rProp.Name.equals(sOldStyleProp))
            lIDs[IDGROUP_OLDSTYLE] = rProp.Handle;
        else
        if (rProp.Name.equals(sInternalProp))
            lIDs[IDGROUP_INTERNAL_PATHS] = rProp.Handle;
        else
        if (rProp.Name.equals(sUserProp))
            lIDs[IDGROUP_USER_PATHS] = rProp.Handle;
        else
        if (rProp.Name.equals(sWriteProp))
            lIDs[IDGROUP_WRITE_PATH] = rProp.Handle;
    }

    return lIDs;
}

//-----------------------------------------------------------------------------
void PathSettings::impl_notifyPropListener(      PathSettings::EChangeOp /*eOp*/     ,
                                           const OUString&        sPath   ,
                                           const PathSettings::PathInfo* pPathOld,
                                           const PathSettings::PathInfo* pPathNew)
{
    css::uno::Sequence< sal_Int32 >     lHandles(1);
    css::uno::Sequence< css::uno::Any > lOldVals(1);
    css::uno::Sequence< css::uno::Any > lNewVals(1);

    css::uno::Sequence< sal_Int32 > lIDs   = impl_mapPathName2IDList(sPath);
    sal_Int32                       c      = lIDs.getLength();
    sal_Int32                       i      = 0;
    sal_Int32                       nMaxID = m_lPropDesc.getLength()-1;
    for (i=0; i<c; ++i)
    {
        sal_Int32 nID = lIDs[i];

        if (
            (nID < 0     ) ||
            (nID > nMaxID)
           )
           continue;

        lHandles[0] = nID;
        switch(impl_getPropGroup(nID))
        {
            case IDGROUP_OLDSTYLE :
                 {
                    if (pPathOld)
                    {
                        OUString sVal = impl_convertPath2OldStyle(*pPathOld);
                        lOldVals[0] <<= sVal;
                    }
                    if (pPathNew)
                    {
                        OUString sVal = impl_convertPath2OldStyle(*pPathNew);
                        lNewVals[0] <<= sVal;
                    }
                 }
                 break;

            case IDGROUP_INTERNAL_PATHS :
                 {
                    if (pPathOld)
                        lOldVals[0] <<= pPathOld->lInternalPaths.getAsConstList();
                    if (pPathNew)
                        lNewVals[0] <<= pPathNew->lInternalPaths.getAsConstList();
                 }
                 break;

            case IDGROUP_USER_PATHS :
                 {
                    if (pPathOld)
                        lOldVals[0] <<= pPathOld->lUserPaths.getAsConstList();
                    if (pPathNew)
                        lNewVals[0] <<= pPathNew->lUserPaths.getAsConstList();
                 }
                 break;

            case IDGROUP_WRITE_PATH :
                 {
                    if (pPathOld)
                        lOldVals[0] <<= pPathOld->sWritePath;
                    if (pPathNew)
                        lNewVals[0] <<= pPathNew->sWritePath;
                 }
                 break;
        }

        fire(lHandles.getArray(),
             lNewVals.getArray(),
             lOldVals.getArray(),
             1,
             sal_False);
    }
}

//-----------------------------------------------------------------------------
void PathSettings::impl_subst(      OUStringList&                                          lVals   ,
                              const css::uno::Reference< css::util::XStringSubstitution >& xSubst  ,
                                    sal_Bool                                               bReSubst)
{
    OUStringList::iterator pIt;

    for (  pIt  = lVals.begin();
           pIt != lVals.end()  ;
         ++pIt                 )
    {
        const OUString& sOld = *pIt;
              OUString  sNew ;
        if (bReSubst)
            sNew = xSubst->reSubstituteVariables(sOld);
        else
            sNew = xSubst->substituteVariables(sOld, sal_False);

        *pIt = sNew;
    }
}

//-----------------------------------------------------------------------------
void PathSettings::impl_subst(PathSettings::PathInfo& aPath   ,
                              sal_Bool                bReSubst)
{
    css::uno::Reference< css::util::XStringSubstitution > xSubst = fa_getSubstitution();

    impl_subst(aPath.lInternalPaths, xSubst, bReSubst);
    impl_subst(aPath.lUserPaths    , xSubst, bReSubst);
    if (bReSubst)
        aPath.sWritePath = xSubst->reSubstituteVariables(aPath.sWritePath);
    else
        aPath.sWritePath = xSubst->substituteVariables(aPath.sWritePath, sal_False);
}

//-----------------------------------------------------------------------------
OUString PathSettings::impl_convertPath2OldStyle(const PathSettings::PathInfo& rPath) const
{
    OUStringList::const_iterator pIt;
    OUStringList                 lTemp;
    lTemp.reserve(rPath.lInternalPaths.size() + rPath.lUserPaths.size() + 1);

    for (  pIt  = rPath.lInternalPaths.begin();
           pIt != rPath.lInternalPaths.end()  ;
         ++pIt                                 )
    {
        lTemp.push_back(*pIt);
    }
    for (  pIt  = rPath.lUserPaths.begin();
           pIt != rPath.lUserPaths.end()  ;
         ++pIt                             )
    {
        lTemp.push_back(*pIt);
    }

    if (!rPath.sWritePath.isEmpty())
        lTemp.push_back(rPath.sWritePath);

    OUStringBuffer sPathVal(256);
    for (  pIt  = lTemp.begin();
           pIt != lTemp.end()  ;
                               )
    {
        sPathVal.append(*pIt);
        ++pIt;
        if (pIt != lTemp.end())
            sPathVal.appendAscii(";");
    }

    return sPathVal.makeStringAndClear();
}

//-----------------------------------------------------------------------------
OUStringList PathSettings::impl_convertOldStyle2Path(const OUString& sOldStylePath) const
{
    OUStringList lList;
    sal_Int32    nToken = 0;
    do
    {
        OUString sToken = sOldStylePath.getToken(0, ';', nToken);
        if (!sToken.isEmpty())
            lList.push_back(sToken);
    }
    while(nToken >= 0);

    return lList;
}

//-----------------------------------------------------------------------------
void PathSettings::impl_purgeKnownPaths(const PathSettings::PathInfo& rPath,
                                               OUStringList&           lList)
{
    OUStringList::const_iterator pIt;
    for (  pIt  = rPath.lInternalPaths.begin();
           pIt != rPath.lInternalPaths.end()  ;
         ++pIt                                 )
    {
        const OUString& rItem = *pIt;
        OUStringList::iterator pItem = lList.find(rItem);
        if (pItem != lList.end())
            lList.erase(pItem);
    }
    for (  pIt  = rPath.lUserPaths.begin();
           pIt != rPath.lUserPaths.end()  ;
         ++pIt                             )
    {
        const OUString& rItem = *pIt;
        OUStringList::iterator pItem = lList.find(rItem);
        if (pItem != lList.end())
            lList.erase(pItem);
    }

    OUStringList::iterator pItem = lList.find(rPath.sWritePath);
    if (pItem != lList.end())
        lList.erase(pItem);
}

//-----------------------------------------------------------------------------
void PathSettings::impl_rebuildPropertyDescriptor()
{
    // SAFE ->
    WriteGuard aWriteLock(m_aLock);

    sal_Int32 c = (sal_Int32)m_lPaths.size();
    sal_Int32 i = 0;
    m_lPropDesc.realloc(c*IDGROUP_COUNT);

    PathHash::const_iterator pIt;
    for (  pIt  = m_lPaths.begin();
           pIt != m_lPaths.end()  ;
         ++pIt                     )
    {
        const PathSettings::PathInfo& rPath = pIt->second;
              css::beans::Property*   pProp = 0;

        pProp             = &(m_lPropDesc[i]);
        pProp->Name       = rPath.sPathName;
        pProp->Handle     = i;
        pProp->Type       = ::getCppuType((OUString*)0);
        pProp->Attributes = css::beans::PropertyAttribute::BOUND;
        if (rPath.bIsReadonly)
            pProp->Attributes |= css::beans::PropertyAttribute::READONLY;
        ++i;

        pProp             = &(m_lPropDesc[i]);
        pProp->Name       = rPath.sPathName+POSTFIX_INTERNAL_PATHS;
        pProp->Handle     = i;
        pProp->Type       = ::getCppuType((css::uno::Sequence< OUString >*)0);
        pProp->Attributes = css::beans::PropertyAttribute::BOUND   |
                            css::beans::PropertyAttribute::READONLY;
        ++i;

        pProp             = &(m_lPropDesc[i]);
        pProp->Name       = rPath.sPathName+POSTFIX_USER_PATHS;
        pProp->Handle     = i;
        pProp->Type       = ::getCppuType((css::uno::Sequence< OUString >*)0);
        pProp->Attributes = css::beans::PropertyAttribute::BOUND;
        if (rPath.bIsReadonly)
            pProp->Attributes |= css::beans::PropertyAttribute::READONLY;
        ++i;

        pProp             = &(m_lPropDesc[i]);
        pProp->Name       = rPath.sPathName+POSTFIX_WRITE_PATH;
        pProp->Handle     = i;
        pProp->Type       = ::getCppuType((OUString*)0);
        pProp->Attributes = css::beans::PropertyAttribute::BOUND;
        if (rPath.bIsReadonly)
            pProp->Attributes |= css::beans::PropertyAttribute::READONLY;
        ++i;
    }

    if (m_pPropHelp)
       delete m_pPropHelp;
    m_pPropHelp = new ::cppu::OPropertyArrayHelper(m_lPropDesc, sal_False); // false => not sorted ... must be done inside helper

    aWriteLock.unlock();
    // <- SAFE
}

//-----------------------------------------------------------------------------
css::uno::Any PathSettings::impl_getPathValue(sal_Int32 nID) const
{
    const PathSettings::PathInfo* pPath = impl_getPathAccessConst(nID);
    if (! pPath)
        throw css::container::NoSuchElementException();

    css::uno::Any aVal;
    switch(impl_getPropGroup(nID))
    {
        case IDGROUP_OLDSTYLE :
             {
                OUString sVal = impl_convertPath2OldStyle(*pPath);
                aVal <<= sVal;
             }
             break;

        case IDGROUP_INTERNAL_PATHS :
             {
                aVal <<= pPath->lInternalPaths.getAsConstList();
             }
             break;

        case IDGROUP_USER_PATHS :
             {
                aVal <<= pPath->lUserPaths.getAsConstList();
             }
             break;

        case IDGROUP_WRITE_PATH :
             {
                aVal <<= pPath->sWritePath;
             }
             break;
    }

    return aVal;
}

//-----------------------------------------------------------------------------
void PathSettings::impl_setPathValue(      sal_Int32      nID ,
                                     const css::uno::Any& aVal)
{
    PathSettings::PathInfo* pOrgPath = impl_getPathAccess(nID);
    if (! pOrgPath)
        throw css::container::NoSuchElementException();

    // We work on a copied path ... so we can be sure that errors during this operation
    // does not make our internal cache invalid  .-)
    PathSettings::PathInfo aChangePath(*pOrgPath);

    switch(impl_getPropGroup(nID))
    {
        case IDGROUP_OLDSTYLE :
             {
                OUString sVal;
                aVal >>= sVal;
                OUStringList lList = impl_convertOldStyle2Path(sVal);
                impl_subst(lList, fa_getSubstitution(), sal_False);
                impl_purgeKnownPaths(aChangePath, lList);
                if (! impl_isValidPath(lList))
                    throw css::lang::IllegalArgumentException();

                if (aChangePath.bIsSinglePath)
                {
                    LOG_ASSERT2(lList.size()>1, "PathSettings::impl_setPathValue()", "You try to set more then path value for a defined SINGLE_PATH!")
                    if ( !lList.empty() )
                        aChangePath.sWritePath = *(lList.begin());
                    else
                        aChangePath.sWritePath = OUString();
                }
                else
                {
                    OUStringList::const_iterator pIt;
                    for (  pIt  = lList.begin();
                           pIt != lList.end()  ;
                         ++pIt                 )
                    {
                        aChangePath.lUserPaths.push_back(*pIt);
                    }
                }
             }
             break;

        case IDGROUP_INTERNAL_PATHS :
             {
                if (aChangePath.bIsSinglePath)
                {
                    OUStringBuffer sMsg(256);
                    sMsg.appendAscii("The path '"    );
                    sMsg.append     (aChangePath.sPathName);
                    sMsg.appendAscii("' is defined as SINGLE_PATH. It's sub set of internal paths cant be set.");
                    throw css::uno::Exception(sMsg.makeStringAndClear(),
                                              static_cast< ::cppu::OWeakObject* >(this));
                }

                OUStringList lList;
                lList << aVal;
                if (! impl_isValidPath(lList))
                    throw css::lang::IllegalArgumentException();
                aChangePath.lInternalPaths = lList;
             }
             break;

        case IDGROUP_USER_PATHS :
             {
                if (aChangePath.bIsSinglePath)
                {
                    OUStringBuffer sMsg(256);
                    sMsg.appendAscii("The path '"    );
                    sMsg.append     (aChangePath.sPathName);
                    sMsg.appendAscii("' is defined as SINGLE_PATH. It's sub set of internal paths cant be set.");
                    throw css::uno::Exception(sMsg.makeStringAndClear(),
                                              static_cast< ::cppu::OWeakObject* >(this));
                }

                OUStringList lList;
                lList << aVal;
                if (! impl_isValidPath(lList))
                    throw css::lang::IllegalArgumentException();
                aChangePath.lUserPaths = lList;
             }
             break;

        case IDGROUP_WRITE_PATH :
             {
                OUString sVal;
                aVal >>= sVal;
                if (! impl_isValidPath(sVal))
                    throw css::lang::IllegalArgumentException();
                aChangePath.sWritePath = sVal;
             }
             break;
    }

    // TODO check if path has at least one path value set
    // At least it depends from the feature using this path, if an empty path list is allowed.

    // first we should try to store the changed (copied!) path ...
    // In case an error occure on saving time an exception is thrown ...
    // If no exception occures we can update our internal cache (means
    // we can overwrite pOrgPath !
    impl_storePath(aChangePath);
    pOrgPath->takeOver(aChangePath);
}

//-----------------------------------------------------------------------------
sal_Bool PathSettings::impl_isValidPath(const OUStringList& lPath) const
{
    OUStringList::const_iterator pIt;
    for (  pIt  = lPath.begin();
           pIt != lPath.end()  ;
         ++pIt                 )
    {
        const OUString& rVal = *pIt;
        if (! impl_isValidPath(rVal))
            return sal_False;
    }

    return sal_True;
}

//-----------------------------------------------------------------------------
sal_Bool PathSettings::impl_isValidPath(const OUString& sPath) const
{
    // allow empty path to reset a path.
// idea by LLA to support empty paths
//    if (sPath.getLength() == 0)
//    {
//        return sal_True;
//    }

    return (! INetURLObject(sPath).HasError());
}

//-----------------------------------------------------------------------------
OUString impl_extractBaseFromPropName(const OUString& sPropName)
{
    sal_Int32 i = sPropName.indexOf(POSTFIX_INTERNAL_PATHS);
    if (i > -1)
        return sPropName.copy(0, i);
    i = sPropName.indexOf(POSTFIX_USER_PATHS);
    if (i > -1)
        return sPropName.copy(0, i);
    i = sPropName.indexOf(POSTFIX_WRITE_PATH);
    if (i > -1)
        return sPropName.copy(0, i);

    return sPropName;
}

//-----------------------------------------------------------------------------
PathSettings::PathInfo* PathSettings::impl_getPathAccess(sal_Int32 nHandle)
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    if (nHandle > (m_lPropDesc.getLength()-1))
        return 0;

    const css::beans::Property&            rProp = m_lPropDesc[nHandle];
          OUString                  sProp = impl_extractBaseFromPropName(rProp.Name);
          PathSettings::PathHash::iterator rPath = m_lPaths.find(sProp);

    if (rPath != m_lPaths.end())
       return &(rPath->second);

    return 0;
    // <- SAFE
}

//-----------------------------------------------------------------------------
const PathSettings::PathInfo* PathSettings::impl_getPathAccessConst(sal_Int32 nHandle) const
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);

    if (nHandle > (m_lPropDesc.getLength()-1))
        return 0;

    const css::beans::Property&                  rProp = m_lPropDesc[nHandle];
          OUString                        sProp = impl_extractBaseFromPropName(rProp.Name);
          PathSettings::PathHash::const_iterator rPath = m_lPaths.find(sProp);

    if (rPath != m_lPaths.end())
       return &(rPath->second);

    return 0;
    // <- SAFE
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL PathSettings::convertFastPropertyValue(      css::uno::Any& aConvertedValue,
                                                               css::uno::Any& aOldValue      ,
                                                               sal_Int32      nHandle        ,
                                                         const css::uno::Any& aValue         )
    throw(css::lang::IllegalArgumentException)
{
    // throws NoSuchElementException !
    css::uno::Any aCurrentVal = impl_getPathValue(nHandle);

    return PropHelper::willPropertyBeChanged(
                aCurrentVal,
                aValue,
                aOldValue,
                aConvertedValue);
}

//-----------------------------------------------------------------------------
void SAL_CALL PathSettings::setFastPropertyValue_NoBroadcast(      sal_Int32      nHandle,
                                                             const css::uno::Any& aValue )
    throw(css::uno::Exception)
{
    // throws NoSuchElement- and IllegalArgumentException !
    impl_setPathValue(nHandle, aValue);
}

//-----------------------------------------------------------------------------
void SAL_CALL PathSettings::getFastPropertyValue(css::uno::Any& aValue ,
                                                 sal_Int32      nHandle) const
{
    aValue = impl_getPathValue(nHandle);
}

//-----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SAL_CALL PathSettings::getInfoHelper()
{
    return *m_pPropHelp;
}

//-----------------------------------------------------------------------------
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL PathSettings::getPropertySetInfo()
    throw(css::uno::RuntimeException)
{
    return css::uno::Reference< css::beans::XPropertySetInfo >(createPropertySetInfo(getInfoHelper()));
}

//-----------------------------------------------------------------------------
css::uno::Reference< css::util::XStringSubstitution > PathSettings::fa_getSubstitution()
{
    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext >  xContext  = m_xContext;
    css::uno::Reference< css::util::XStringSubstitution >  xSubst = m_xSubstitution;
    aReadLock.unlock();
    // <- SAFE

    if (! xSubst.is())
    {
        // create the needed substitution service.
        // We must replace all used variables inside readed path values.
        // In case we can't do so ... the whole office can't work really.
        // That's why it seams to be OK to throw a RuntimeException then.
        xSubst = css::util::PathSubstitution::create(xContext);

        // SAFE ->
        WriteGuard aWriteLock(m_aLock);
        m_xSubstitution = xSubst;
        aWriteLock.unlock();
    }

    return xSubst;
}

//-----------------------------------------------------------------------------
css::uno::Reference< css::container::XNameAccess > PathSettings::fa_getCfgOld()
{
    const OUString CFG_NODE_OLD("org.openoffice.Office.Common/Path/Current");

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    css::uno::Reference< css::container::XNameAccess >     xCfg  = m_xCfgOld;
    aReadLock.unlock();
    // <- SAFE

    if (! xCfg.is())
    {
        xCfg = css::uno::Reference< css::container::XNameAccess >(
                   ::comphelper::ConfigurationHelper::openConfig(
                        xContext,
                        CFG_NODE_OLD,
                        ::comphelper::ConfigurationHelper::E_STANDARD), // not readonly! Sometimes we need write access there !!!
                   css::uno::UNO_QUERY_THROW);

        // SAFE ->
        WriteGuard aWriteLock(m_aLock);
        m_xCfgOld = xCfg;
        aWriteLock.unlock();
    }

    return xCfg;
}

//-----------------------------------------------------------------------------
css::uno::Reference< css::container::XNameAccess > PathSettings::fa_getCfgNew()
{
    const OUString CFG_NODE_NEW("org.openoffice.Office.Paths/Paths");

    // SAFE ->
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::uno::XComponentContext > xContext = m_xContext;
    css::uno::Reference< css::container::XNameAccess >     xCfg  = m_xCfgNew;
    if (xCfg.is())
       return xCfg;
    aReadLock.unlock();
    // <- SAFE

    if (! xCfg.is())
    {
        xCfg = css::uno::Reference< css::container::XNameAccess >(
                   ::comphelper::ConfigurationHelper::openConfig(
                        xContext,
                        CFG_NODE_NEW,
                        ::comphelper::ConfigurationHelper::E_STANDARD),
                   css::uno::UNO_QUERY_THROW);

        // SAFE ->
        WriteGuard aWriteLock(m_aLock);
        m_xCfgNew = xCfg;
        m_xCfgNewListener = new WeakChangesListener(this);
        aWriteLock.unlock();

        css::uno::Reference< css::util::XChangesNotifier > xBroadcaster(xCfg, css::uno::UNO_QUERY_THROW);
        xBroadcaster->addChangesListener(m_xCfgNewListener);
    }

    return xCfg;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
