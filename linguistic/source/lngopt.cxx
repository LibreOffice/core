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


#include <sal/macros.h>
#include "lngopt.hxx"
#include "lngreg.hxx"
#include "linguistic/lngprops.hxx"
#include "linguistic/misc.hxx"
#include <tools/debug.hxx>
#include <unotools/lingucfg.hxx>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <i18nlangtag/mslangid.hxx>

using namespace utl;
using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using namespace com::sun::star::registry;




// static member initialization
SvtLinguOptions *   LinguOptions::pData = nullptr;
oslInterlockedCount LinguOptions::nRefCount;


LinguOptions::LinguOptions()
{
    if (!pData)
    {
        pData = new SvtLinguOptions;
        SvtLinguConfig aLinguCfg;
        aLinguCfg.GetOptions( *pData );
    }

    osl_atomic_increment( &nRefCount );
}


LinguOptions::LinguOptions(const LinguOptions & /*rOpt*/)
{
    DBG_ASSERT( pData, "lng : data missing" );
    osl_atomic_increment( &nRefCount );
}


LinguOptions::~LinguOptions()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if ( osl_atomic_decrement( &nRefCount ) == 0 )
    {
        delete pData;   pData  = nullptr;
    }
}

struct WID_Name
{
    sal_Int32        nWID;
    const char  *pPropertyName;
};

//! order of entries is import (see LinguOptions::GetName)
//! since the WID is used as index in this table!
WID_Name aWID_Name[] =
{
    { 0,                                  nullptr },
    { UPH_IS_USE_DICTIONARY_LIST,         UPN_IS_USE_DICTIONARY_LIST },
    { UPH_IS_IGNORE_CONTROL_CHARACTERS,   UPN_IS_IGNORE_CONTROL_CHARACTERS },
    { UPH_IS_SPELL_UPPER_CASE,            UPN_IS_SPELL_UPPER_CASE },
    { UPH_IS_SPELL_WITH_DIGITS,           UPN_IS_SPELL_WITH_DIGITS },
    { UPH_IS_SPELL_CAPITALIZATION,        UPN_IS_SPELL_CAPITALIZATION },
    { UPH_HYPH_MIN_LEADING,               UPN_HYPH_MIN_LEADING },
    { UPH_HYPH_MIN_TRAILING,              UPN_HYPH_MIN_TRAILING },
    { UPH_HYPH_MIN_WORD_LENGTH,           UPN_HYPH_MIN_WORD_LENGTH },
    { UPH_DEFAULT_LOCALE,                 UPN_DEFAULT_LOCALE },
    { UPH_IS_SPELL_AUTO,                  UPN_IS_SPELL_AUTO },
    { 0,                                  nullptr },
    { 0,                                  nullptr },
    { UPH_IS_SPELL_SPECIAL,               UPN_IS_SPELL_SPECIAL },
    { UPH_IS_HYPH_AUTO,                   UPN_IS_HYPH_AUTO },
    { UPH_IS_HYPH_SPECIAL,                UPN_IS_HYPH_SPECIAL },
    { UPH_IS_WRAP_REVERSE,                UPN_IS_WRAP_REVERSE },
    { 0,                                  nullptr },
    { 0,                                  nullptr },
    { 0,                                  nullptr },
    { 0,                                  nullptr },
    { UPH_DEFAULT_LANGUAGE,               UPN_DEFAULT_LANGUAGE },
    { UPH_DEFAULT_LOCALE_CJK,             UPN_DEFAULT_LOCALE_CJK },
    { UPH_DEFAULT_LOCALE_CTL,             UPN_DEFAULT_LOCALE_CTL }
};


OUString LinguOptions::GetName( sal_Int32 nWID )
{
    MutexGuard  aGuard( GetLinguMutex() );

    OUString aRes;

    sal_Int32 nLen = sizeof (aWID_Name) / sizeof (aWID_Name[0]);
    if (0 <= nWID && nWID < nLen && aWID_Name[ nWID ].nWID == nWID)
        aRes = OUString::createFromAscii(aWID_Name[nWID].pPropertyName);
    else
        OSL_FAIL("lng : unknown WID");

    return aRes;
}


//! map must be sorted by first entry in alphabetical increasing order.
static const SfxItemPropertyMapEntry* lcl_GetLinguProps()
{
    static const SfxItemPropertyMapEntry aLinguProps[] =
    {
        { OUString(UPN_DEFAULT_LANGUAGE),           UPH_DEFAULT_LANGUAGE,
                ::cppu::UnoType<sal_Int16>::get(),    0, 0 },
        { OUString(UPN_DEFAULT_LOCALE),             UPH_DEFAULT_LOCALE,
                ::cppu::UnoType<Locale>::get(),       0, 0 },
        { OUString(UPN_DEFAULT_LOCALE_CJK),         UPH_DEFAULT_LOCALE_CJK,
                ::cppu::UnoType<Locale>::get(),       0, 0 },
        { OUString(UPN_DEFAULT_LOCALE_CTL),         UPH_DEFAULT_LOCALE_CTL,
                ::cppu::UnoType<Locale>::get(),       0, 0 },
        { OUString(UPN_HYPH_MIN_LEADING),           UPH_HYPH_MIN_LEADING,
                ::cppu::UnoType<sal_Int16>::get(),    0, 0 },
        { OUString(UPN_HYPH_MIN_TRAILING),          UPH_HYPH_MIN_TRAILING,
                ::cppu::UnoType<sal_Int16>::get(),    0, 0 },
        { OUString(UPN_HYPH_MIN_WORD_LENGTH),       UPH_HYPH_MIN_WORD_LENGTH,
                ::cppu::UnoType<sal_Int16>::get(),    0, 0 },
        { OUString(UPN_IS_GERMAN_PRE_REFORM),       UPH_IS_GERMAN_PRE_REFORM,       /*! deprecated !*/
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_HYPH_AUTO),               UPH_IS_HYPH_AUTO,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_HYPH_SPECIAL),            UPH_IS_HYPH_SPECIAL,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_IGNORE_CONTROL_CHARACTERS),   UPH_IS_IGNORE_CONTROL_CHARACTERS,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_SPELL_AUTO),              UPH_IS_SPELL_AUTO,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_SPELL_CAPITALIZATION),    UPH_IS_SPELL_CAPITALIZATION,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_SPELL_HIDE),              UPH_IS_SPELL_HIDE,              /*! deprecated !*/
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_SPELL_IN_ALL_LANGUAGES),  UPH_IS_SPELL_IN_ALL_LANGUAGES,  /*! deprecated !*/
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_SPELL_SPECIAL),           UPH_IS_SPELL_SPECIAL,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_SPELL_UPPER_CASE),        UPH_IS_SPELL_UPPER_CASE,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_SPELL_WITH_DIGITS),       UPH_IS_SPELL_WITH_DIGITS,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_USE_DICTIONARY_LIST),     UPH_IS_USE_DICTIONARY_LIST,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(UPN_IS_WRAP_REVERSE),            UPH_IS_WRAP_REVERSE,
                cppu::UnoType<bool>::get(),            0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aLinguProps;
}
LinguProps::LinguProps() :
    aEvtListeners   (GetLinguMutex()),
    aPropListeners  (GetLinguMutex()),
    aPropertyMap(lcl_GetLinguProps())
{
    bDisposing = false;
}

void LinguProps::launchEvent( const PropertyChangeEvent &rEvt ) const
{
    cppu::OInterfaceContainerHelper *pContainer =
            aPropListeners.getContainer( rEvt.PropertyHandle );
    if (pContainer)
    {
        cppu::OInterfaceIteratorHelper aIt( *pContainer );
        while (aIt.hasMoreElements())
        {
            Reference< XPropertyChangeListener > xRef( aIt.next(), UNO_QUERY );
            if (xRef.is())
                xRef->propertyChange( rEvt );
        }
    }
}

Reference< XInterface > SAL_CALL LinguProps_CreateInstance(
            const Reference< XMultiServiceFactory > & /*rSMgr*/ )
        throw(Exception)
{
    Reference< XInterface > xService = static_cast<cppu::OWeakObject*>(new LinguProps);
    return xService;
}

Reference< XPropertySetInfo > SAL_CALL LinguProps::getPropertySetInfo()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    static Reference< XPropertySetInfo > aRef =
            new SfxItemPropertySetInfo( aPropertyMap );
    return aRef;
}

void SAL_CALL LinguProps::setPropertyValue(
            const OUString& rPropertyName, const Any& rValue )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    const SfxItemPropertySimpleEntry* pCur = aPropertyMap.getByName( rPropertyName );
    if (pCur)
    {
        Any aOld( aConfig.GetProperty( pCur->nWID ) );
        if (aOld != rValue && aConfig.SetProperty( pCur->nWID, rValue ))
        {
            PropertyChangeEvent aChgEvt( static_cast<XPropertySet *>(this), rPropertyName,
                    sal_False, pCur->nWID, aOld, rValue );
            launchEvent( aChgEvt );
        }
    }
}

Any SAL_CALL LinguProps::getPropertyValue( const OUString& rPropertyName )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aRet;

    const SfxItemPropertySimpleEntry* pCur = aPropertyMap.getByName( rPropertyName );
    if(pCur)
    {
        aRet = aConfig.GetProperty( pCur->nWID );
    }

    return aRet;
}

void SAL_CALL LinguProps::addPropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& rxListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
    {
        const SfxItemPropertySimpleEntry* pCur = aPropertyMap.getByName( rPropertyName );
        if(pCur)
            aPropListeners.addInterface( pCur->nWID, rxListener );
    }
}

void SAL_CALL LinguProps::removePropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& rxListener )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
    {
        const SfxItemPropertySimpleEntry* pCur = aPropertyMap.getByName( rPropertyName );
        if(pCur)
            aPropListeners.removeInterface( pCur->nWID, rxListener );
    }
}

void SAL_CALL LinguProps::addVetoableChangeListener(
            const OUString& /*rPropertyName*/,
            const Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}

void SAL_CALL LinguProps::removeVetoableChangeListener(
            const OUString& /*rPropertyName*/,
            const Reference< XVetoableChangeListener >& /*xListener*/ )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
}


void SAL_CALL LinguProps::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aOld( aConfig.GetProperty( nHandle ) );
    if (aOld != rValue && aConfig.SetProperty( nHandle, rValue ))
    {
        PropertyChangeEvent aChgEvt( static_cast<XPropertySet *>(this),
                LinguOptions::GetName( nHandle ), sal_False, nHandle, aOld, rValue );
        launchEvent( aChgEvt );
    }
}


Any SAL_CALL LinguProps::getFastPropertyValue( sal_Int32 nHandle )
        throw(UnknownPropertyException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aRes( aConfig.GetProperty( nHandle ) );
    return aRes;
}


Sequence< PropertyValue > SAL_CALL
    LinguProps::getPropertyValues()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nLen = aPropertyMap.getSize();
    Sequence< PropertyValue > aProps( nLen );
    PropertyValue *pProp = aProps.getArray();
    PropertyEntryVector_t aPropEntries = aPropertyMap.getPropertyEntries();
    PropertyEntryVector_t::const_iterator aIt = aPropEntries.begin();
    for (sal_Int32 i = 0;  i < nLen;  ++i, ++aIt)
    {
        PropertyValue &rVal = pProp[i];
        Any aAny( aConfig.GetProperty( aIt->nWID ) );

        rVal.Name   = aIt->sName;
        rVal.Handle = aIt->nWID;
        rVal.Value  = aAny;
        rVal.State  = PropertyState_DIRECT_VALUE ;
    }
    return aProps;
}

void SAL_CALL
    LinguProps::setPropertyValues( const Sequence< PropertyValue >& rProps )
        throw(UnknownPropertyException, PropertyVetoException,
              IllegalArgumentException, WrappedTargetException, RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Int32 nLen = rProps.getLength();
    const PropertyValue *pVal = rProps.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        const PropertyValue &rVal = pVal[i];
        setPropertyValue( rVal.Name, rVal.Value );
    }
}

void SAL_CALL
    LinguProps::dispose()
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;

        //! its too late to save the options here!
        // (see AppExitListener for saving)
        //aOpt.Save();  // save (possible) changes before exiting

        EventObject aEvtObj( static_cast<XPropertySet *>(this) );
        aEvtListeners.disposeAndClear( aEvtObj );
        aPropListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL
    LinguProps::addEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL
    LinguProps::removeEventListener( const Reference< XEventListener >& rxListener )
        throw(RuntimeException, std::exception)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


// Service specific part

// XServiceInfo
OUString SAL_CALL LinguProps::getImplementationName()
        throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

// XServiceInfo
sal_Bool SAL_CALL LinguProps::supportsService( const OUString& ServiceName )
        throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
uno::Sequence< OUString > SAL_CALL LinguProps::getSupportedServiceNames()
        throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

// ORegistryServiceManager_Static
uno::Sequence< OUString > LinguProps::getSupportedServiceNames_Static()
        throw()
{
    uno::Sequence< OUString > aSNS { "com.sun.star.linguistic2.LinguProperties" };
    return aSNS;
}

bool LinguProps::getPropertyBool(const OUString& aPropertyName) throw (css::uno::RuntimeException)
{
   uno::Any any = getPropertyValue(aPropertyName);
   bool b = false;
   any >>= b;
   return b;
}

sal_Int16 LinguProps::getPropertyInt16(const OUString& aPropertyName) throw (css::uno::RuntimeException)
{
   uno::Any any = getPropertyValue(aPropertyName);
   sal_Int16 b = 0;
   any >>= b;
   return b;
}

Locale LinguProps::getPropertyLocale(const OUString& aPropertyName) throw (css::uno::RuntimeException)
{
   uno::Any any = getPropertyValue(aPropertyName);
   css::lang::Locale b;
   any >>= b;
   return b;
}

void * SAL_CALL LinguProps_getFactory( const sal_Char * pImplName,
            XMultiServiceFactory *pServiceManager, void * )
{
    void * pRet = nullptr;
    if ( LinguProps::getImplementationName_Static().equalsAscii( pImplName ) )
    {
        Reference< XSingleServiceFactory > xFactory =
            cppu::createOneInstanceFactory(
                pServiceManager,
                LinguProps::getImplementationName_Static(),
                LinguProps_CreateInstance,
                LinguProps::getSupportedServiceNames_Static());
        // acquire, because we return an interface pointer instead of a reference
        xFactory->acquire();
        pRet = xFactory.get();
    }
    return pRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
