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
#include <linguistic/misc.hxx>
#include <o3tl/safeint.hxx>
#include <tools/debug.hxx>
#include <unotools/lingucfg.hxx>

#include <comphelper/sequence.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/lang/Locale.hpp>

using namespace osl;
using namespace com::sun::star;
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

namespace {

struct WID_Name
{
    sal_Int32        nWID;
    OUString         aPropertyName;
};

}

//! order of entries is import (see LinguOptions::GetName)
//! since the WID is used as index in this table!
WID_Name const aWID_Name[] =
{
    { 0,                                  u""_ustr },
    { UPH_IS_USE_DICTIONARY_LIST,         UPN_IS_USE_DICTIONARY_LIST },
    { UPH_IS_IGNORE_CONTROL_CHARACTERS,   UPN_IS_IGNORE_CONTROL_CHARACTERS },
    { UPH_IS_SPELL_UPPER_CASE,            UPN_IS_SPELL_UPPER_CASE },
    { UPH_IS_SPELL_WITH_DIGITS,           UPN_IS_SPELL_WITH_DIGITS },
    { UPH_HYPH_MIN_LEADING,               UPN_HYPH_MIN_LEADING },
    { UPH_HYPH_MIN_TRAILING,              UPN_HYPH_MIN_TRAILING },
    { UPH_HYPH_MIN_WORD_LENGTH,           UPN_HYPH_MIN_WORD_LENGTH },
    { UPH_DEFAULT_LOCALE,                 UPN_DEFAULT_LOCALE },
    { UPH_IS_SPELL_AUTO,                  UPN_IS_SPELL_AUTO },
    { 0,                                  u""_ustr },
    { 0,                                  u""_ustr },
    { UPH_IS_SPELL_SPECIAL,               UPN_IS_SPELL_SPECIAL },
    { UPH_IS_HYPH_AUTO,                   UPN_IS_HYPH_AUTO },
    { UPH_IS_HYPH_SPECIAL,                UPN_IS_HYPH_SPECIAL },
    { UPH_IS_WRAP_REVERSE,                UPN_IS_WRAP_REVERSE },
    { 0,                                  u""_ustr },
    { 0,                                  u""_ustr },
    { 0,                                  u""_ustr },
    { 0,                                  u""_ustr },
    { UPH_DEFAULT_LANGUAGE,               UPN_DEFAULT_LANGUAGE },
    { UPH_DEFAULT_LOCALE_CJK,             UPN_DEFAULT_LOCALE_CJK },
    { UPH_DEFAULT_LOCALE_CTL,             UPN_DEFAULT_LOCALE_CTL },
    { UPH_IS_SPELL_CLOSED_COMPOUND,       UPN_IS_SPELL_CLOSED_COMPOUND },
    { UPH_IS_SPELL_HYPHENATED_COMPOUND,   UPN_IS_SPELL_HYPHENATED_COMPOUND }
};


OUString LinguOptions::GetName( sal_Int32 nWID )
{
    MutexGuard  aGuard( GetLinguMutex() );

    OUString aRes;

    if (0 <= nWID && o3tl::make_unsigned(nWID) < SAL_N_ELEMENTS(aWID_Name)
        && aWID_Name[ nWID ].nWID == nWID)
        aRes = aWID_Name[nWID].aPropertyName;
    else
        OSL_FAIL("lng : unknown WID");

    return aRes;
}


//! map must be sorted by first entry in alphabetical increasing order.
static std::span<const SfxItemPropertyMapEntry> lcl_GetLinguProps()
{
    static const SfxItemPropertyMapEntry aLinguProps[] =
    {
        { UPN_DEFAULT_LANGUAGE,           UPH_DEFAULT_LANGUAGE,
                ::cppu::UnoType<sal_Int16>::get(),    0, 0 },
        { UPN_DEFAULT_LOCALE,             UPH_DEFAULT_LOCALE,
                ::cppu::UnoType<Locale>::get(),       0, 0 },
        { UPN_DEFAULT_LOCALE_CJK,         UPH_DEFAULT_LOCALE_CJK,
                ::cppu::UnoType<Locale>::get(),       0, 0 },
        { UPN_DEFAULT_LOCALE_CTL,         UPH_DEFAULT_LOCALE_CTL,
                ::cppu::UnoType<Locale>::get(),       0, 0 },
        { UPN_HYPH_MIN_LEADING,           UPH_HYPH_MIN_LEADING,
                ::cppu::UnoType<sal_Int16>::get(),    0, 0 },
        { UPN_HYPH_MIN_TRAILING,          UPH_HYPH_MIN_TRAILING,
                ::cppu::UnoType<sal_Int16>::get(),    0, 0 },
        { UPN_HYPH_MIN_WORD_LENGTH,       UPH_HYPH_MIN_WORD_LENGTH,
                ::cppu::UnoType<sal_Int16>::get(),    0, 0 },
        { UPN_IS_HYPH_AUTO,               UPH_IS_HYPH_AUTO,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_HYPH_SPECIAL,            UPH_IS_HYPH_SPECIAL,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_IGNORE_CONTROL_CHARACTERS,   UPH_IS_IGNORE_CONTROL_CHARACTERS,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_SPELL_AUTO,              UPH_IS_SPELL_AUTO,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_SPELL_CLOSED_COMPOUND,   UPH_IS_SPELL_CLOSED_COMPOUND,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_SPELL_HYPHENATED_COMPOUND,   UPH_IS_SPELL_HYPHENATED_COMPOUND,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_SPELL_SPECIAL,           UPH_IS_SPELL_SPECIAL,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_SPELL_UPPER_CASE,        UPH_IS_SPELL_UPPER_CASE,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_SPELL_WITH_DIGITS,       UPH_IS_SPELL_WITH_DIGITS,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_USE_DICTIONARY_LIST,     UPH_IS_USE_DICTIONARY_LIST,
                cppu::UnoType<bool>::get(),            0, 0 },
        { UPN_IS_WRAP_REVERSE,            UPH_IS_WRAP_REVERSE,
                cppu::UnoType<bool>::get(),            0, 0 },
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
    comphelper::OInterfaceContainerHelper3<XPropertyChangeListener> *pContainer =
            aPropListeners.getContainer( rEvt.PropertyHandle );
    if (pContainer)
        pContainer->notifyEach( &XPropertyChangeListener::propertyChange, rEvt );
}

Reference< XPropertySetInfo > SAL_CALL LinguProps::getPropertySetInfo()
{
    MutexGuard  aGuard( GetLinguMutex() );

    static Reference< XPropertySetInfo > aRef =
            new SfxItemPropertySetInfo( aPropertyMap );
    return aRef;
}

void SAL_CALL LinguProps::setPropertyValue(
            const OUString& rPropertyName, const Any& rValue )
{
    MutexGuard  aGuard( GetLinguMutex() );

    const SfxItemPropertyMapEntry* pCur = aPropertyMap.getByName( rPropertyName );
    if (pCur)
    {
        Any aOld( aConfig.GetProperty( pCur->nWID ) );
        if (aOld != rValue && aConfig.SetProperty( pCur->nWID, rValue ))
        {
            PropertyChangeEvent aChgEvt( static_cast<XPropertySet *>(this), rPropertyName,
                    false, pCur->nWID, aOld, rValue );
            launchEvent( aChgEvt );
        }
    }
}

Any SAL_CALL LinguProps::getPropertyValue( const OUString& rPropertyName )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aRet;

    const SfxItemPropertyMapEntry* pCur = aPropertyMap.getByName( rPropertyName );
    if(pCur)
    {
        aRet = aConfig.GetProperty( pCur->nWID );
    }

    return aRet;
}

void SAL_CALL LinguProps::addPropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
    {
        const SfxItemPropertyMapEntry* pCur = aPropertyMap.getByName( rPropertyName );
        if(pCur)
            aPropListeners.addInterface( pCur->nWID, rxListener );
    }
}

void SAL_CALL LinguProps::removePropertyChangeListener(
            const OUString& rPropertyName,
            const Reference< XPropertyChangeListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
    {
        const SfxItemPropertyMapEntry* pCur = aPropertyMap.getByName( rPropertyName );
        if(pCur)
            aPropListeners.removeInterface( pCur->nWID, rxListener );
    }
}

void SAL_CALL LinguProps::addVetoableChangeListener(
            const OUString& /*rPropertyName*/,
            const Reference< XVetoableChangeListener >& /*xListener*/ )
{
}

void SAL_CALL LinguProps::removeVetoableChangeListener(
            const OUString& /*rPropertyName*/,
            const Reference< XVetoableChangeListener >& /*xListener*/ )
{
}


void SAL_CALL LinguProps::setFastPropertyValue( sal_Int32 nHandle, const Any& rValue )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aOld( aConfig.GetProperty( nHandle ) );
    if (aOld != rValue && aConfig.SetProperty( nHandle, rValue ))
    {
        PropertyChangeEvent aChgEvt( static_cast<XPropertySet *>(this),
                LinguOptions::GetName( nHandle ), false, nHandle, aOld, rValue );
        launchEvent( aChgEvt );
    }
}


Any SAL_CALL LinguProps::getFastPropertyValue( sal_Int32 nHandle )
{
    MutexGuard  aGuard( GetLinguMutex() );

    Any aRes( aConfig.GetProperty( nHandle ) );
    return aRes;
}


Sequence< PropertyValue > SAL_CALL
    LinguProps::getPropertyValues()
{
    MutexGuard  aGuard( GetLinguMutex() );

    std::vector<PropertyValue> aProps;
    aProps.reserve(aPropertyMap.getPropertyEntries().size());
    for(auto pEntry : aPropertyMap.getPropertyEntries())
        aProps.push_back(PropertyValue(pEntry->aName, pEntry->nWID,
                                 aConfig.GetProperty(pEntry->nWID),
                                 css::beans::PropertyState_DIRECT_VALUE));
    return comphelper::containerToSequence(aProps);
}

void SAL_CALL
    LinguProps::setPropertyValues( const Sequence< PropertyValue >& rProps )
{
    MutexGuard  aGuard( GetLinguMutex() );

    for (const PropertyValue &rVal : rProps)
    {
        setPropertyValue( rVal.Name, rVal.Value );
    }
}

void SAL_CALL
    LinguProps::dispose()
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing)
    {
        bDisposing = true;

        //! it's too late to save the options here!
        // (see AppExitListener for saving)
        //aOpt.Save();  // save (possible) changes before exiting

        EventObject aEvtObj( static_cast<XPropertySet *>(this) );
        aEvtListeners.disposeAndClear( aEvtObj );
        aPropListeners.disposeAndClear( aEvtObj );
    }
}

void SAL_CALL
    LinguProps::addEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.addInterface( rxListener );
}

void SAL_CALL
    LinguProps::removeEventListener( const Reference< XEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (!bDisposing && rxListener.is())
        aEvtListeners.removeInterface( rxListener );
}


// Service specific part

// XServiceInfo
OUString SAL_CALL LinguProps::getImplementationName()
{
    return u"com.sun.star.lingu2.LinguProps"_ustr;
}

// XServiceInfo
sal_Bool SAL_CALL LinguProps::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
uno::Sequence< OUString > SAL_CALL LinguProps::getSupportedServiceNames()
{
    return { u"com.sun.star.linguistic2.LinguProperties"_ustr };
}

bool LinguProps::getPropertyBool(const OUString& aPropertyName)
{
   uno::Any any = getPropertyValue(aPropertyName);
   bool b = false;
   any >>= b;
   return b;
}

sal_Int16 LinguProps::getPropertyInt16(const OUString& aPropertyName)
{
   uno::Any any = getPropertyValue(aPropertyName);
   sal_Int16 b = 0;
   any >>= b;
   return b;
}

Locale LinguProps::getPropertyLocale(const OUString& aPropertyName)
{
   uno::Any any = getPropertyValue(aPropertyName);
   css::lang::Locale b;
   any >>= b;
   return b;
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
linguistic_LinguProps_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new LinguProps());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
