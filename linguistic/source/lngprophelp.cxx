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


#include <tools/debug.hxx>
#include <sal/macros.h>

#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/mutex.hxx>

#include <linguistic/misc.hxx>
#include <linguistic/lngprops.hxx>

#include <linguistic/lngprophelp.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;
using namespace linguistic;

using ::rtl::OUString;

namespace linguistic
{


static const char *aCH[] =
{
    UPN_IS_IGNORE_CONTROL_CHARACTERS,
    UPN_IS_USE_DICTIONARY_LIST,
};

static int nCHCount = sizeof(aCH) / sizeof(aCH[0]);


PropertyChgHelper::PropertyChgHelper(
        const Reference< XInterface > &rxSource,
        Reference< XPropertySet > &rxPropSet,
        int nAllowedEvents ) :
    PropertyChgHelperBase(),
    aPropNames          (nCHCount),
    xMyEvtObj           (rxSource),
    aLngSvcEvtListeners (GetLinguMutex()),
    xPropSet            (rxPropSet),
    nEvtFlags           (nAllowedEvents)
{
    OUString *pName = aPropNames.getArray();
    for (sal_Int32 i = 0;  i < nCHCount;  ++i)
    {
        pName[i] = ::rtl::OUString::createFromAscii( aCH[i] );
    }

    SetDefaultValues();
}


PropertyChgHelper::PropertyChgHelper( const PropertyChgHelper &rHelper ) :
    PropertyChgHelperBase(),
    aLngSvcEvtListeners (GetLinguMutex())
{
    RemoveAsPropListener();
    aPropNames  = rHelper.aPropNames;
    xMyEvtObj   = rHelper.xMyEvtObj;
    xPropSet    = rHelper.xPropSet;
    nEvtFlags   = rHelper.nEvtFlags;
    AddAsPropListener();

    SetDefaultValues();
    GetCurrentValues();
}


PropertyChgHelper::~PropertyChgHelper()
{
}


void PropertyChgHelper::AddPropNames( const char *pNewNames[], sal_Int32 nCount )
{
    if (pNewNames && nCount)
    {
        sal_Int32 nLen = GetPropNames().getLength();
        GetPropNames().realloc( nLen + nCount );
        OUString *pName = GetPropNames().getArray();
        for (sal_Int32 i = 0;  i < nCount;  ++i)
        {
            pName[ nLen + i ] = ::rtl::OUString::createFromAscii( pNewNames[ i ] );

        }
    }
}


void PropertyChgHelper::SetDefaultValues()
{
    bResIsIgnoreControlCharacters   = bIsIgnoreControlCharacters    = sal_True;
    bResIsUseDictionaryList         = bIsUseDictionaryList          = sal_True;
}


void PropertyChgHelper::GetCurrentValues()
{
    sal_Int32 nLen = GetPropNames().getLength();
    if (GetPropSet().is() && nLen)
    {
        const OUString *pPropName = GetPropNames().getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            sal_Bool *pbVal     = NULL,
                 *pbResVal  = NULL;

            if ( pPropName[i] == UPN_IS_IGNORE_CONTROL_CHARACTERS )
            {
                pbVal    = &bIsIgnoreControlCharacters;
                pbResVal = &bResIsIgnoreControlCharacters;
            }
            else if ( pPropName[i] == UPN_IS_USE_DICTIONARY_LIST )
            {
                pbVal    = &bIsUseDictionaryList;
                pbResVal = &bResIsUseDictionaryList;
            }

            if (pbVal && pbResVal)
            {
                GetPropSet()->getPropertyValue( pPropName[i] ) >>= *pbVal;
                *pbResVal = *pbVal;
            }
        }
    }
}


void PropertyChgHelper::SetTmpPropVals( const PropertyValues &rPropVals )
{
    // return value is default value unless there is an explicitly supplied
    // temporary value
    bResIsIgnoreControlCharacters   = bIsIgnoreControlCharacters;
    bResIsUseDictionaryList         = bIsUseDictionaryList;

    sal_Int32 nLen = rPropVals.getLength();
    if (nLen)
    {
        const PropertyValue *pVal = rPropVals.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            sal_Bool  *pbResVal = NULL;
            switch (pVal[i].Handle)
            {
                case UPH_IS_IGNORE_CONTROL_CHARACTERS :
                        pbResVal = &bResIsIgnoreControlCharacters; break;
                case UPH_IS_USE_DICTIONARY_LIST     :
                        pbResVal = &bResIsUseDictionaryList; break;
                default:
                        ;
            }
            if (pbResVal)
                pVal[i].Value >>= *pbResVal;
        }
    }
}


sal_Bool PropertyChgHelper::propertyChange_Impl( const PropertyChangeEvent& rEvt )
{
    sal_Bool bRes = sal_False;

    if (GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        sal_Int16 nLngSvcFlags = (nEvtFlags & AE_HYPHENATOR) ?
                    LinguServiceEventFlags::HYPHENATE_AGAIN : 0;
        sal_Bool bSCWA = sal_False, // SPELL_CORRECT_WORDS_AGAIN ?
             bSWWA = sal_False; // SPELL_WRONG_WORDS_AGAIN ?

        sal_Bool  *pbVal = NULL;
        switch (rEvt.PropertyHandle)
        {
            case UPH_IS_IGNORE_CONTROL_CHARACTERS :
            {
                pbVal = &bIsIgnoreControlCharacters;
                nLngSvcFlags = 0;
                break;
            }
            case UPH_IS_USE_DICTIONARY_LIST       :
            {
                pbVal = &bIsUseDictionaryList;
                bSCWA = bSWWA = sal_True;
                break;
            }
            default:
            {
                bRes = sal_False;
            }
        }
        if (pbVal)
            rEvt.NewValue >>= *pbVal;

        bRes = 0 != pbVal;  // sth changed?
        if (bRes)
        {
            sal_Bool bSpellEvts = (nEvtFlags & AE_SPELLCHECKER) ? sal_True : sal_False;
            if (bSCWA && bSpellEvts)
                nLngSvcFlags |= LinguServiceEventFlags::SPELL_CORRECT_WORDS_AGAIN;
            if (bSWWA && bSpellEvts)
                nLngSvcFlags |= LinguServiceEventFlags::SPELL_WRONG_WORDS_AGAIN;
            if (nLngSvcFlags)
            {
                LinguServiceEvent aEvt( GetEvtObj(), nLngSvcFlags );
                LaunchEvent( aEvt );
            }
        }
    }

    return bRes;
}


void SAL_CALL
    PropertyChgHelper::propertyChange( const PropertyChangeEvent& rEvt )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    propertyChange_Impl( rEvt );
}


void PropertyChgHelper::AddAsPropListener()
{
    if (xPropSet.is())
    {
        sal_Int32 nLen = aPropNames.getLength();
        const OUString *pPropName = aPropNames.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            if (!pPropName[i].isEmpty())
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
            if (!pPropName[i].isEmpty())
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



PropertyHelper_Thes::PropertyHelper_Thes(
        const Reference< XInterface > &rxSource,
        Reference< XPropertySet > &rxPropSet ) :
    PropertyChgHelper   ( rxSource, rxPropSet, 0 )
{
    SetDefaultValues();
    GetCurrentValues();
}


PropertyHelper_Thes::~PropertyHelper_Thes()
{
}


void SAL_CALL
    PropertyHelper_Thes::propertyChange( const PropertyChangeEvent& rEvt )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    PropertyChgHelper::propertyChange_Impl( rEvt );
}



// list of properties from the property set to be used
// and listened to
static const char *aSP[] =
{
    UPN_IS_SPELL_UPPER_CASE,
    UPN_IS_SPELL_WITH_DIGITS,
    UPN_IS_SPELL_CAPITALIZATION
};


PropertyHelper_Spell::PropertyHelper_Spell(
        const Reference< XInterface > & rxSource,
        Reference< XPropertySet > &rxPropSet ) :
    PropertyChgHelper   ( rxSource, rxPropSet, AE_SPELLCHECKER )
{
    AddPropNames( aSP, sizeof(aSP) / sizeof(aSP[0]) );
    SetDefaultValues();
    GetCurrentValues();

    nResMaxNumberOfSuggestions = GetDefaultNumberOfSuggestions();
}


PropertyHelper_Spell::~PropertyHelper_Spell()
{
}


void PropertyHelper_Spell::SetDefaultValues()
{
    PropertyChgHelper::SetDefaultValues();

    bResIsSpellUpperCase        = bIsSpellUpperCase         = sal_False;
    bResIsSpellWithDigits       = bIsSpellWithDigits        = sal_False;
    bResIsSpellCapitalization   = bIsSpellCapitalization    = sal_True;
}


void PropertyHelper_Spell::GetCurrentValues()
{
    PropertyChgHelper::GetCurrentValues();

    sal_Int32 nLen = GetPropNames().getLength();
    if (GetPropSet().is() && nLen)
    {
        const OUString *pPropName = GetPropNames().getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            sal_Bool *pbVal     = NULL,
                 *pbResVal  = NULL;

            if ( pPropName[i] == UPN_IS_SPELL_UPPER_CASE )
            {
                pbVal    = &bIsSpellUpperCase;
                pbResVal = &bResIsSpellUpperCase;
            }
            else if ( pPropName[i] == UPN_IS_SPELL_WITH_DIGITS )
            {
                pbVal    = &bIsSpellWithDigits;
                pbResVal = &bResIsSpellWithDigits;
            }
            else if ( pPropName[i] == UPN_IS_SPELL_CAPITALIZATION )
            {
                pbVal    = &bIsSpellCapitalization;
                pbResVal = &bResIsSpellCapitalization;
            }

            if (pbVal && pbResVal)
            {
                GetPropSet()->getPropertyValue( pPropName[i] ) >>= *pbVal;
                *pbResVal = *pbVal;
            }
        }
    }
}


sal_Bool PropertyHelper_Spell::propertyChange_Impl( const PropertyChangeEvent& rEvt )
{
    sal_Bool bRes = PropertyChgHelper::propertyChange_Impl( rEvt );

    if (!bRes  &&  GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        sal_Bool bSCWA = sal_False, // SPELL_CORRECT_WORDS_AGAIN ?
             bSWWA = sal_False; // SPELL_WRONG_WORDS_AGAIN ?

        sal_Bool *pbVal = NULL;
        switch (rEvt.PropertyHandle)
        {
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
                DBG_ASSERT( 0, "unknown property" );
        }
        if (pbVal)
            rEvt.NewValue >>= *pbVal;

        bRes = (pbVal != 0);
        if (bRes)
        {
            sal_Int16 nLngSvcFlags = 0;
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

    return bRes;
}


void SAL_CALL
    PropertyHelper_Spell::propertyChange( const PropertyChangeEvent& rEvt )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    propertyChange_Impl( rEvt );
}


void PropertyHelper_Spell::SetTmpPropVals( const PropertyValues &rPropVals )
{
    PropertyChgHelper::SetTmpPropVals( rPropVals );

    // return value is default value unless there is an explicitly supplied
    // temporary value
    nResMaxNumberOfSuggestions  = GetDefaultNumberOfSuggestions();
    bResIsSpellWithDigits       = bIsSpellWithDigits;
    bResIsSpellCapitalization   = bIsSpellCapitalization;
    bResIsSpellUpperCase        = bIsSpellUpperCase;

    sal_Int32 nLen = rPropVals.getLength();
    if (nLen)
    {
        const PropertyValue *pVal = rPropVals.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            if ( pVal[i].Name == UPN_MAX_NUMBER_OF_SUGGESTIONS )
            {
                pVal[i].Value >>= nResMaxNumberOfSuggestions;
            }
            else
            {
                sal_Bool *pbResVal = NULL;
                switch (pVal[i].Handle)
                {
                    case UPH_IS_SPELL_UPPER_CASE     : pbResVal = &bResIsSpellUpperCase; break;
                    case UPH_IS_SPELL_WITH_DIGITS    : pbResVal = &bResIsSpellWithDigits; break;
                    case UPH_IS_SPELL_CAPITALIZATION : pbResVal = &bResIsSpellCapitalization; break;
                    default:
                        DBG_ASSERT( 0, "unknown property" );
                }
                if (pbResVal)
                    pVal[i].Value >>= *pbResVal;
            }
        }
    }
}

sal_Int16 PropertyHelper_Spell::GetDefaultNumberOfSuggestions() const
{
    return 16;
}


static const char *aHP[] =
{
    UPN_HYPH_MIN_LEADING,
    UPN_HYPH_MIN_TRAILING,
    UPN_HYPH_MIN_WORD_LENGTH
};


PropertyHelper_Hyphen::PropertyHelper_Hyphen(
        const Reference< XInterface > & rxSource,
        Reference< XPropertySet > &rxPropSet ) :
    PropertyChgHelper   ( rxSource, rxPropSet, AE_HYPHENATOR )
{
    AddPropNames( aHP, sizeof(aHP) / sizeof(aHP[0]) );
    SetDefaultValues();
    GetCurrentValues();
}


PropertyHelper_Hyphen::~PropertyHelper_Hyphen()
{
}


void PropertyHelper_Hyphen::SetDefaultValues()
{
    PropertyChgHelper::SetDefaultValues();

    nResHyphMinLeading      = nHyphMinLeading       = 2;
    nResHyphMinTrailing     = nHyphMinTrailing      = 2;
    nResHyphMinWordLength   = nHyphMinWordLength    = 0;
}


void PropertyHelper_Hyphen::GetCurrentValues()
{
    PropertyChgHelper::GetCurrentValues();

    sal_Int32 nLen = GetPropNames().getLength();
    if (GetPropSet().is() && nLen)
    {
        const OUString *pPropName = GetPropNames().getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            sal_Int16  *pnVal    = NULL,
                   *pnResVal = NULL;

            if ( pPropName[i] == UPN_HYPH_MIN_LEADING )
            {
                pnVal    = &nHyphMinLeading;
                pnResVal = &nResHyphMinLeading;
            }
            else if ( pPropName[i] == UPN_HYPH_MIN_TRAILING )
            {
                pnVal    = &nHyphMinTrailing;
                pnResVal = &nResHyphMinTrailing;
            }
            else if ( pPropName[i] == UPN_HYPH_MIN_WORD_LENGTH )
            {
                pnVal    = &nHyphMinWordLength;
                pnResVal = &nResHyphMinWordLength;
            }

            if (pnVal && pnResVal)
            {
                GetPropSet()->getPropertyValue( pPropName[i] ) >>= *pnVal;
                *pnResVal = *pnVal;
            }
        }
    }
}


sal_Bool PropertyHelper_Hyphen::propertyChange_Impl( const PropertyChangeEvent& rEvt )
{
    sal_Bool bRes = PropertyChgHelper::propertyChange_Impl( rEvt );

    if (!bRes  &&  GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        sal_Int16 nLngSvcFlags = LinguServiceEventFlags::HYPHENATE_AGAIN;

        sal_Int16   *pnVal = NULL;
        switch (rEvt.PropertyHandle)
        {
            case UPH_HYPH_MIN_LEADING     : pnVal = &nHyphMinLeading; break;
            case UPH_HYPH_MIN_TRAILING    : pnVal = &nHyphMinTrailing; break;
            case UPH_HYPH_MIN_WORD_LENGTH : pnVal = &nHyphMinWordLength; break;
            default:
                DBG_ASSERT( 0, "unknown property" );
        }
        if (pnVal)
            rEvt.NewValue >>= *pnVal;

        bRes = (pnVal != 0);
        if (bRes)
        {
            if (nLngSvcFlags)
            {
                LinguServiceEvent aEvt( GetEvtObj(), nLngSvcFlags );
                LaunchEvent( aEvt );
            }
        }
    }

    return bRes;
}


void SAL_CALL
    PropertyHelper_Hyphen::propertyChange( const PropertyChangeEvent& rEvt )
        throw(RuntimeException)
{
    MutexGuard  aGuard( GetLinguMutex() );
    propertyChange_Impl( rEvt );
}


void PropertyHelper_Hyphen::SetTmpPropVals( const PropertyValues &rPropVals )
{
    PropertyChgHelper::SetTmpPropVals( rPropVals );

    // return value is default value unless there is an explicitly supplied
    // temporary value
    nResHyphMinLeading      = nHyphMinLeading;
    nResHyphMinTrailing     = nHyphMinTrailing;
    nResHyphMinWordLength   = nHyphMinWordLength;

    sal_Int32 nLen = rPropVals.getLength();

    if (nLen)
    {
        const PropertyValue *pVal = rPropVals.getConstArray();
        for (sal_Int32 i = 0;  i < nLen;  ++i)
        {
            sal_Int16 *pnResVal = NULL;

            if ( pVal[i].Name == UPN_HYPH_MIN_LEADING )
                pnResVal = &nResHyphMinLeading;
            else if ( pVal[i].Name == UPN_HYPH_MIN_TRAILING )
                pnResVal = &nResHyphMinTrailing;
            else if ( pVal[i].Name == UPN_HYPH_MIN_WORD_LENGTH )
                pnResVal = &nResHyphMinWordLength;

            DBG_ASSERT( pnResVal, "unknown property" );

            if (pnResVal)
                pVal[i].Value >>= *pnResVal;
        }
    }
}

PropertyHelper_Thesaurus::PropertyHelper_Thesaurus(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > &rxPropSet )
{
    pInst = new PropertyHelper_Thes( rxSource, rxPropSet );
    xPropHelper = pInst;
}

PropertyHelper_Thesaurus::~PropertyHelper_Thesaurus()
{
}

void PropertyHelper_Thesaurus::AddAsPropListener()
{
    pInst->AddAsPropListener();
}

void PropertyHelper_Thesaurus::RemoveAsPropListener()
{
    pInst->RemoveAsPropListener();
}

void PropertyHelper_Thesaurus::SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals )
{
    pInst->SetTmpPropVals( rPropVals );
}

PropertyHelper_Hyphenation::PropertyHelper_Hyphenation(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > &rxPropSet)
{
    pInst = new PropertyHelper_Hyphen( rxSource, rxPropSet );
    xPropHelper = pInst;
}

PropertyHelper_Hyphenation::~PropertyHelper_Hyphenation()
{
}

void PropertyHelper_Hyphenation::AddAsPropListener()
{
    pInst->AddAsPropListener();
}

void PropertyHelper_Hyphenation::RemoveAsPropListener()
{
    pInst->RemoveAsPropListener();
}

void PropertyHelper_Hyphenation::SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals )
{
    pInst->SetTmpPropVals( rPropVals );
}

sal_Int16 PropertyHelper_Hyphenation::GetMinLeading() const
{
    return pInst->GetMinLeading();
}

sal_Int16 PropertyHelper_Hyphenation::GetMinTrailing() const
{
    return pInst->GetMinTrailing();
}

sal_Int16 PropertyHelper_Hyphenation::GetMinWordLength() const
{
    return pInst->GetMinWordLength();
}

sal_Bool PropertyHelper_Hyphenation::addLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException)
{
    return pInst->addLinguServiceEventListener( rxListener );
}

sal_Bool PropertyHelper_Hyphenation::removeLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException)
{
    return pInst->removeLinguServiceEventListener( rxListener );
}

PropertyHelper_Spelling::PropertyHelper_Spelling(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > &rxSource,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > &rxPropSet )
{
    pInst = new PropertyHelper_Spell( rxSource, rxPropSet );
    xPropHelper = pInst;
}

PropertyHelper_Spelling::~PropertyHelper_Spelling()
{
}

void PropertyHelper_Spelling::AddAsPropListener()
{
    pInst->AddAsPropListener();
}

void PropertyHelper_Spelling::RemoveAsPropListener()
{
    pInst->RemoveAsPropListener();
}

void PropertyHelper_Spelling::SetTmpPropVals( const com::sun::star::beans::PropertyValues &rPropVals )
{
    pInst->SetTmpPropVals( rPropVals );
}

sal_Bool PropertyHelper_Spelling::IsSpellUpperCase() const
{
    return pInst->IsSpellUpperCase();
}

sal_Bool PropertyHelper_Spelling::IsSpellWithDigits() const
{
    return pInst->IsSpellWithDigits();
}

sal_Bool PropertyHelper_Spelling::IsSpellCapitalization() const
{
    return pInst->IsSpellCapitalization();
}

sal_Bool PropertyHelper_Spelling::addLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException)
{
    return pInst->addLinguServiceEventListener( rxListener );
}

sal_Bool PropertyHelper_Spelling::removeLinguServiceEventListener(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XLinguServiceEventListener >& rxListener )
            throw(::com::sun::star::uno::RuntimeException)
{
    return pInst->removeLinguServiceEventListener( rxListener );
}

}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
