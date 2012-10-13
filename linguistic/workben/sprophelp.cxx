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
#include "linguistic/misc.hxx"

#include "sprophelp.hxx"
#include "linguistic/lngprops.hxx"

#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/mutex.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;

#define A2OU(x) ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( x ))



PropertyChgHelper::PropertyChgHelper(
        const Reference< XInterface > & rxSource,
        Reference< XPropertySet > &rxPropSet,
        const char *pPropNames[], sal_uInt16 nPropCount )
    : aPropNames(nPropCount)
    , xMyEvtObj(rxSource)
    , aLngSvcEvtListeners(GetLinguMutex())
    , xPropSet(rxPropSet)
{
    OUString *pName = aPropNames.getArray();
    for (sal_Int32 i = 0;  i < nPropCount;  ++i)
    {
        pName[i] = ::rtl::OUString::createFromAscii( pPropNames[i] );
    }
}


PropertyChgHelper::PropertyChgHelper( const PropertyChgHelper &rHelper ) :
    aLngSvcEvtListeners (GetLinguMutex())
{
    xPropSet    = rHelper.xPropSet;
    aPropNames  = rHelper.aPropNames;
    AddAsPropListener();

    xMyEvtObj   = rHelper.xMyEvtObj;
}


PropertyChgHelper::~PropertyChgHelper()
{
}


void PropertyChgHelper::AddAsPropListener()
{
    if (xPropSet.is())
    {
        sal_Int32 nLen = aPropNames.getLength();
        const OUString *pPropName = aPropNames.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            if (pPropName[i].getLength())
                xPropSet->addPropertyChangeListener( pPropName[i], this );
        }
    }
}

void PropertyChgHelper::RemoveAsPropListener()
{
    if (xPropSet.is())
    {
        sal_Int32 nLen = aPropNames.getLength();
        const OUString *pPropName = aPropNames.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            if (pPropName[i].getLength())
                xPropSet->removePropertyChangeListener( pPropName[i], this );
        }
    }
}


void PropertyChgHelper::LaunchEvent( const LinguServiceEvent &rEvt )
{
    cppu::OInterfaceIteratorHelper aIt( aLngSvcEvtListeners );
    while (aIt.hasMoreElements())
    {
        Reference< XLinguServiceEventListener > xRef( aIt.next(), UNO_QUERY );
        if (xRef.is())
            xRef->processLinguServiceEvent( rEvt );
    }
}


void SAL_CALL PropertyChgHelper::disposing( const EventObject& rSource )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (rSource.Source == xPropSet)
    {
        RemoveAsPropListener();
        xPropSet = NULL;
        aPropNames.realloc( 0 );
    }
}


sal_Bool SAL_CALL
    PropertyChgHelper::addLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (rxListener.is())
    {
        sal_Int32   nCount = aLngSvcEvtListeners.getLength();
        bRes = aLngSvcEvtListeners.addInterface( rxListener ) != nCount;
    }
    return bRes;
}


sal_Bool SAL_CALL
    PropertyChgHelper::removeLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxListener )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    sal_Bool bRes = sal_False;
    if (rxListener.is())
    {
        sal_Int32   nCount = aLngSvcEvtListeners.getLength();
        bRes = aLngSvcEvtListeners.removeInterface( rxListener ) != nCount;
    }
    return bRes;
}


static const char *aSP[] =
{
    UPN_IS_GERMAN_PRE_REFORM,
    UPN_IS_IGNORE_CONTROL_CHARACTERS,
    UPN_IS_USE_DICTIONARY_LIST,
    UPN_IS_SPELL_UPPER_CASE,
    UPN_IS_SPELL_WITH_DIGITS,
    UPN_IS_SPELL_CAPITALIZATION
};


PropertyHelper_Spell::PropertyHelper_Spell(
        const Reference< XInterface > & rxSource,
        Reference< XPropertySet > &rxPropSet ) :
    PropertyChgHelper   ( rxSource, rxPropSet, aSP, sizeof(aSP) / sizeof(aSP[0]) )
{
    SetDefault();
    sal_Int32 nLen = GetPropNames().getLength();
    if (rxPropSet.is() && nLen)
    {
        const OUString *pPropName = GetPropNames().getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            sal_Bool *pbVal     = NULL,
                 *pbResVal  = NULL;

            if (A2OU( UPN_IS_GERMAN_PRE_REFORM ) == pPropName[i])
            {
                pbVal    = &bIsGermanPreReform;
                pbResVal = &bResIsGermanPreReform;
            }
            else if (A2OU( UPN_IS_IGNORE_CONTROL_CHARACTERS ) == pPropName[i])
            {
                pbVal    = &bIsIgnoreControlCharacters;
                pbResVal = &bResIsIgnoreControlCharacters;
            }
            else if (A2OU( UPN_IS_USE_DICTIONARY_LIST ) == pPropName[i])
            {
                pbVal    = &bIsUseDictionaryList;
                pbResVal = &bResIsUseDictionaryList;
            }
            else if (A2OU( UPN_IS_SPELL_UPPER_CASE ) == pPropName[i])
            {
                pbVal    = &bIsSpellUpperCase;
                pbResVal = &bResIsSpellUpperCase;
            }
            else if (A2OU( UPN_IS_SPELL_WITH_DIGITS ) == pPropName[i])
            {
                pbVal    = &bIsSpellWithDigits;
                pbResVal = &bResIsSpellWithDigits;
            }
            else if (A2OU( UPN_IS_SPELL_CAPITALIZATION ) == pPropName[i])
            {
                pbVal    = &bIsSpellCapitalization;
                pbResVal = &bResIsSpellCapitalization;
            }

            if (pbVal && pbResVal)
            {
                rxPropSet->getPropertyValue( pPropName[i] ) >>= *pbVal;
                *pbResVal = *pbVal;
            }
        }
    }
}


PropertyHelper_Spell::~PropertyHelper_Spell()
{
}


void PropertyHelper_Spell::SetDefault()
{
    bResIsGermanPreReform           = bIsGermanPreReform            = sal_False;
    bResIsIgnoreControlCharacters   = bIsIgnoreControlCharacters    = sal_True;
    bResIsUseDictionaryList         = bIsUseDictionaryList          = sal_True;
    bResIsSpellUpperCase            = bIsSpellUpperCase             = sal_False;
    bResIsSpellWithDigits           = bIsSpellWithDigits            = sal_False;
    bResIsSpellCapitalization       = bIsSpellCapitalization        = sal_True;
}


void SAL_CALL
    PropertyHelper_Spell::propertyChange( const PropertyChangeEvent& rEvt )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        sal_Int16 nLngSvcFlags = 0;
        sal_Bool bSCWA = sal_False, // SPELL_CORRECT_WORDS_AGAIN ?
             bSWWA = sal_False; // SPELL_WRONG_WORDS_AGAIN ?

        sal_Bool *pbVal = NULL;
        switch (rEvt.PropertyHandle)
        {
            case UPH_IS_IGNORE_CONTROL_CHARACTERS :
            {
                pbVal = &bIsIgnoreControlCharacters;
                break;
            }
            case UPH_IS_GERMAN_PRE_REFORM         :
            {
                pbVal = &bIsGermanPreReform;
                bSCWA = bSWWA = sal_True;
                break;
            }
            case UPH_IS_USE_DICTIONARY_LIST       :
            {
                pbVal = &bIsUseDictionaryList;
                bSCWA = bSWWA = sal_True;
                break;
            }
            case UPH_IS_SPELL_UPPER_CASE          :
            {
                pbVal = &bIsSpellUpperCase;
                bSCWA = sal_False == *pbVal;    // sal_False->sal_True change?
                bSWWA = !bSCWA;             // sal_True->sal_False change?
                break;
            }
            case UPH_IS_SPELL_WITH_DIGITS         :
            {
                pbVal = &bIsSpellWithDigits;
                bSCWA = sal_False == *pbVal;    // sal_False->sal_True change?
                bSWWA = !bSCWA;             // sal_True->sal_False change?
                break;
            }
            case UPH_IS_SPELL_CAPITALIZATION      :
            {
                pbVal = &bIsSpellCapitalization;
                bSCWA = sal_False == *pbVal;    // sal_False->sal_True change?
                bSWWA = !bSCWA;             // sal_True->sal_False change?
                break;
            }
            default:
                OSL_FAIL( "unknown property" );
        }
        if (pbVal)
            rEvt.NewValue >>= *pbVal;

        if (bSCWA)
            nLngSvcFlags |= LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN;
        if (bSWWA)
            nLngSvcFlags |= LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN;
        if (nLngSvcFlags)
        {
            LinguServiceEvent aEvt( GetEvtObj(), nLngSvcFlags );
            LaunchEvent( aEvt );
        }
    }
}


void PropertyHelper_Spell::SetTmpPropVals( const PropertyValues &rPropVals )
{
    // set return value to default value unless there is an
    // explicitly supplied temporary value
    bResIsGermanPreReform           = bIsGermanPreReform;
    bResIsIgnoreControlCharacters   = bIsIgnoreControlCharacters;
    bResIsUseDictionaryList         = bIsUseDictionaryList;
    bResIsSpellUpperCase            = bIsSpellUpperCase;
    bResIsSpellWithDigits           = bIsSpellWithDigits;
    bResIsSpellCapitalization       = bIsSpellCapitalization;

    sal_Int32 nLen = rPropVals.getLength();
    if (nLen)
    {
        const PropertyValue *pVal = rPropVals.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            sal_Bool *pbResVal = NULL;
            switch (pVal[i].Handle)
            {
                case UPH_IS_GERMAN_PRE_REFORM         : pbResVal = &bResIsGermanPreReform; break;
                case UPH_IS_IGNORE_CONTROL_CHARACTERS : pbResVal = &bResIsIgnoreControlCharacters; break;
                case UPH_IS_USE_DICTIONARY_LIST       : pbResVal = &bResIsUseDictionaryList; break;
                case UPH_IS_SPELL_UPPER_CASE          : pbResVal = &bResIsSpellUpperCase; break;
                case UPH_IS_SPELL_WITH_DIGITS         : pbResVal = &bResIsSpellWithDigits; break;
                case UPH_IS_SPELL_CAPITALIZATION      : pbResVal = &bResIsSpellCapitalization; break;
                default:
                    OSL_FAIL( "unknown property" );
            }
            if (pbResVal)
                pVal[i].Value >>= *pbResVal;
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
