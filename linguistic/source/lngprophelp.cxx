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
#include <sal/log.hxx>

#include <com/sun/star/linguistic2/LinguServiceEvent.hpp>
#include <com/sun/star/linguistic2/LinguServiceEventFlags.hpp>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/mutex.hxx>
#include <unotools/linguprops.hxx>

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


namespace linguistic
{


PropertyChgHelper::PropertyChgHelper(
        const Reference< XInterface > &rxSource,
        Reference< XLinguProperties > const &rxPropSet,
        int nAllowedEvents ) :
    aPropNames          ({UPN_IS_IGNORE_CONTROL_CHARACTERS, UPN_IS_USE_DICTIONARY_LIST}),
    xMyEvtObj           (rxSource),
    aLngSvcEvtListeners (GetLinguMutex()),
    xPropSet            (rxPropSet),
    nEvtFlags           (nAllowedEvents)
{
    SetDefaultValues();
}

PropertyChgHelper::~PropertyChgHelper()
{
}

void PropertyChgHelper::SetDefaultValues()
{
    bResIsIgnoreControlCharacters   = bIsIgnoreControlCharacters    = true;
    bResIsUseDictionaryList         = bIsUseDictionaryList          = true;
}


void PropertyChgHelper::GetCurrentValues()
{
    const auto& rPropNames = GetPropNames();
    if (!GetPropSet().is() || rPropNames.empty())
        return;

    for (const OUString& rPropName : rPropNames)
    {
        bool *pbVal     = nullptr,
             *pbResVal  = nullptr;

        if ( rPropName == UPN_IS_IGNORE_CONTROL_CHARACTERS )
        {
            pbVal    = &bIsIgnoreControlCharacters;
            pbResVal = &bResIsIgnoreControlCharacters;
        }
        else if ( rPropName == UPN_IS_USE_DICTIONARY_LIST )
        {
            pbVal    = &bIsUseDictionaryList;
            pbResVal = &bResIsUseDictionaryList;
        }

        if (pbVal && pbResVal)
        {
            GetPropSet()->getPropertyValue( rPropName ) >>= *pbVal;
            *pbResVal = *pbVal;
        }
    }
}


void PropertyChgHelper::SetTmpPropVals( const PropertyValues &rPropVals )
{
    // return value is default value unless there is an explicitly supplied
    // temporary value
    bResIsIgnoreControlCharacters   = bIsIgnoreControlCharacters;
    bResIsUseDictionaryList         = bIsUseDictionaryList;

    for (const PropertyValue& rVal : rPropVals)
    {
        bool  *pbResVal = nullptr;
        switch (rVal.Handle)
        {
            case UPH_IS_IGNORE_CONTROL_CHARACTERS :
                    pbResVal = &bResIsIgnoreControlCharacters; break;
            case UPH_IS_USE_DICTIONARY_LIST     :
                    pbResVal = &bResIsUseDictionaryList; break;
            default:
                    ;
        }
        if (pbResVal)
            rVal.Value >>= *pbResVal;
    }
}


bool PropertyChgHelper::propertyChange_Impl( const PropertyChangeEvent& rEvt )
{
    bool bRes = false;

    if (GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        sal_Int16 nLngSvcFlags = (nEvtFlags & AE_HYPHENATOR) ?
                    LinguServiceEventFlags::HYPHENATE_AGAIN : 0;
        bool bSCWA = false, // SPELL_CORRECT_WORDS_AGAIN ?
             bSWWA = false; // SPELL_WRONG_WORDS_AGAIN ?

        bool  *pbVal = nullptr;
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
                bSCWA = bSWWA = true;
                break;
            }
        }
        if (pbVal)
            rEvt.NewValue >>= *pbVal;

        bRes = nullptr != pbVal;  // sth changed?
        if (bRes)
        {
            bool bSpellEvts = (nEvtFlags & AE_SPELLCHECKER);
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
{
    MutexGuard  aGuard( GetLinguMutex() );
    propertyChange_Impl( rEvt );
}


void PropertyChgHelper::AddAsPropListener()
{
    if (xPropSet.is())
    {
        for (const OUString& rPropName : aPropNames)
        {
            if (!rPropName.isEmpty())
                xPropSet->addPropertyChangeListener( rPropName, this );
        }
    }
}

void PropertyChgHelper::RemoveAsPropListener()
{
    if (xPropSet.is())
    {
        for (const OUString& rPropName : aPropNames)
        {
            if (!rPropName.isEmpty())
                xPropSet->removePropertyChangeListener( rPropName, this );
        }
    }
}


void PropertyChgHelper::LaunchEvent( const LinguServiceEvent &rEvt )
{
    aLngSvcEvtListeners.notifyEach( &XLinguServiceEventListener::processLinguServiceEvent, rEvt );
}


void SAL_CALL PropertyChgHelper::disposing( const EventObject& rSource )
{
    MutexGuard  aGuard( GetLinguMutex() );
    if (rSource.Source == xPropSet)
    {
        RemoveAsPropListener();
        xPropSet = nullptr;
        aPropNames.clear();
    }
}


sal_Bool SAL_CALL
    PropertyChgHelper::addLinguServiceEventListener(
            const Reference< XLinguServiceEventListener >& rxListener )
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
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
{
    MutexGuard  aGuard( GetLinguMutex() );

    bool bRes = false;
    if (rxListener.is())
    {
        sal_Int32   nCount = aLngSvcEvtListeners.getLength();
        bRes = aLngSvcEvtListeners.removeInterface( rxListener ) != nCount;
    }
    return bRes;
}


PropertyHelper_Thes::PropertyHelper_Thes(
        const Reference< XInterface > &rxSource,
        Reference< XLinguProperties > const &rxPropSet ) :
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
{
    MutexGuard  aGuard( GetLinguMutex() );
    PropertyChgHelper::propertyChange_Impl( rEvt );
}


PropertyHelper_Spell::PropertyHelper_Spell(
        const Reference< XInterface > & rxSource,
        Reference< XLinguProperties > const &rxPropSet ) :
    PropertyChgHelper   ( rxSource, rxPropSet, AE_SPELLCHECKER )
{
    auto& rPropNames = GetPropNames();
    rPropNames.push_back(UPN_IS_SPELL_UPPER_CASE);
    rPropNames.push_back(UPN_IS_SPELL_WITH_DIGITS);
    rPropNames.push_back(UPN_IS_SPELL_CLOSED_COMPOUND);
    rPropNames.push_back(UPN_IS_SPELL_HYPHENATED_COMPOUND);
    SetDefaultValues();
    GetCurrentValues();
}


PropertyHelper_Spell::~PropertyHelper_Spell()
{
}


void PropertyHelper_Spell::SetDefaultValues()
{
    PropertyChgHelper::SetDefaultValues();

    bResIsSpellUpperCase        = bIsSpellUpperCase         = false;
    bResIsSpellWithDigits       = bIsSpellWithDigits        = false;
    bResIsSpellClosedCompound   = bIsSpellClosedCompound    = true;
    bResIsSpellHyphenatedCompound = bIsSpellHyphenatedCompound = true;
}


void PropertyHelper_Spell::GetCurrentValues()
{
    PropertyChgHelper::GetCurrentValues();

    const auto& rPropNames = GetPropNames();
    if (!GetPropSet().is() || rPropNames.empty())
        return;

    for (const OUString& rPropName : rPropNames)
    {
        bool *pbVal     = nullptr,
             *pbResVal  = nullptr;

        if ( rPropName == UPN_IS_SPELL_UPPER_CASE )
        {
            pbVal    = &bIsSpellUpperCase;
            pbResVal = &bResIsSpellUpperCase;
        }
        else if ( rPropName == UPN_IS_SPELL_WITH_DIGITS )
        {
            pbVal    = &bIsSpellWithDigits;
            pbResVal = &bResIsSpellWithDigits;
        }
        else if ( rPropName == UPN_IS_SPELL_CLOSED_COMPOUND )
        {
            pbVal    = &bIsSpellClosedCompound;
            pbResVal = &bResIsSpellClosedCompound;
        }
        else if ( rPropName == UPN_IS_SPELL_HYPHENATED_COMPOUND )
        {
            pbVal    = &bIsSpellHyphenatedCompound;
            pbResVal = &bResIsSpellHyphenatedCompound;
        }

        if (pbVal && pbResVal)
        {
            GetPropSet()->getPropertyValue( rPropName ) >>= *pbVal;
            *pbResVal = *pbVal;
        }
    }
}


bool PropertyHelper_Spell::propertyChange_Impl( const PropertyChangeEvent& rEvt )
{
    bool bRes = PropertyChgHelper::propertyChange_Impl( rEvt );

    if (!bRes  &&  GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        bool bSCWA = false, // SPELL_CORRECT_WORDS_AGAIN ?
             bSWWA = false; // SPELL_WRONG_WORDS_AGAIN ?

        bool *pbVal = nullptr;
        switch (rEvt.PropertyHandle)
        {
            case UPH_IS_SPELL_UPPER_CASE          :
            {
                pbVal = &bIsSpellUpperCase;
                bSCWA = ! *pbVal;    // sal_False->sal_True change?
                bSWWA = !bSCWA;             // sal_True->sal_False change?
                break;
            }
            case UPH_IS_SPELL_WITH_DIGITS         :
            {
                pbVal = &bIsSpellWithDigits;
                bSCWA = ! *pbVal;    // sal_False->sal_True change?
                bSWWA = !bSCWA;             // sal_True->sal_False change?
                break;
            }
            case UPH_IS_SPELL_CLOSED_COMPOUND     :
            {
                pbVal = &bIsSpellClosedCompound;
                bSCWA = ! *pbVal;    // sal_False->sal_True change?
                bSWWA = !bSCWA;             // sal_True->sal_False change?
                break;
            }
            case UPH_IS_SPELL_HYPHENATED_COMPOUND :
            {
                pbVal = &bIsSpellHyphenatedCompound;
                bSCWA = ! *pbVal;    // sal_False->sal_True change?
                bSWWA = !bSCWA;             // sal_True->sal_False change?
                break;
            }
            default:
                SAL_WARN( "linguistic", "unknown property handle " << rEvt.PropertyHandle << " (check in include/unotools/linguprops.hxx)" );
        }
        if (pbVal)
            rEvt.NewValue >>= *pbVal;

        bRes = (pbVal != nullptr);
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
{
    MutexGuard  aGuard( GetLinguMutex() );
    propertyChange_Impl( rEvt );
}


void PropertyHelper_Spell::SetTmpPropVals( const PropertyValues &rPropVals )
{
    PropertyChgHelper::SetTmpPropVals( rPropVals );

    // return value is default value unless there is an explicitly supplied
    // temporary value
    bResIsSpellWithDigits       = bIsSpellWithDigits;
    bResIsSpellClosedCompound   = bIsSpellClosedCompound;
    bResIsSpellHyphenatedCompound = bIsSpellHyphenatedCompound;
    bResIsSpellUpperCase        = bIsSpellUpperCase;

    for (const PropertyValue& rVal : rPropVals)
    {
        if ( rVal.Name == UPN_MAX_NUMBER_OF_SUGGESTIONS )
        {
            // special value that is not part of the property set and thus needs to be handled differently
        }
        else
        {
            bool *pbResVal = nullptr;
            switch (rVal.Handle)
            {
                case UPH_IS_SPELL_UPPER_CASE     : pbResVal = &bResIsSpellUpperCase; break;
                case UPH_IS_SPELL_WITH_DIGITS    : pbResVal = &bResIsSpellWithDigits; break;
                case UPH_IS_SPELL_CLOSED_COMPOUND : pbResVal = &bResIsSpellClosedCompound; break;
                case UPH_IS_SPELL_HYPHENATED_COMPOUND : pbResVal = &bResIsSpellHyphenatedCompound; break;
                default:
                    SAL_WARN( "linguistic", "unknown property handle " << rVal.Handle << " (check in include/unotools/linguprops.hxx)" );
            }
            if (pbResVal)
                rVal.Value >>= *pbResVal;
        }
    }
}

PropertyHelper_Hyphen::PropertyHelper_Hyphen(
        const Reference< XInterface > & rxSource,
        Reference< XLinguProperties > const &rxPropSet ) :
    PropertyChgHelper   ( rxSource, rxPropSet, AE_HYPHENATOR )
{
    auto& rPropNames = GetPropNames();
    rPropNames.push_back(UPN_HYPH_MIN_LEADING);
    rPropNames.push_back(UPN_HYPH_MIN_TRAILING);
    rPropNames.push_back(UPN_HYPH_MIN_WORD_LENGTH);
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
    nResHyphCompoundMinLeading = nHyphCompoundMinLeading = 2;
    nResHyphMinWordLength   = nHyphMinWordLength    = 0;
    nResHyphTextHyphenZone  = nHyphTextHyphenZone   = 0;
    bResNoHyphenateCaps = bNoHyphenateCaps = false;
}


void PropertyHelper_Hyphen::GetCurrentValues()
{
    PropertyChgHelper::GetCurrentValues();

    const auto& rPropNames = GetPropNames();
    if (!GetPropSet().is() || rPropNames.empty())
        return;

    for (const OUString& rPropName : rPropNames)
    {
        sal_Int16  *pnVal    = nullptr,
               *pnResVal = nullptr;
        bool *pbVal = nullptr;
        bool *pbResVal = nullptr;

        if ( rPropName == UPN_HYPH_MIN_LEADING )
        {
            pnVal    = &nHyphMinLeading;
            pnResVal = &nResHyphMinLeading;
        }
        else if ( rPropName == UPN_HYPH_MIN_TRAILING )
        {
            pnVal    = &nHyphMinTrailing;
            pnResVal = &nResHyphMinTrailing;
        }
        if ( rPropName == UPN_HYPH_COMPOUND_MIN_LEADING )
        {
            pnVal    = &nHyphCompoundMinLeading;
            pnResVal = &nResHyphCompoundMinLeading;
        }
        else if ( rPropName == UPN_HYPH_MIN_WORD_LENGTH )
        {
            pnVal    = &nHyphMinWordLength;
            pnResVal = &nResHyphMinWordLength;
        }
        else if ( rPropName == UPN_HYPH_ZONE )
        {
            pnVal    = &nHyphTextHyphenZone;
            pnResVal = &nResHyphTextHyphenZone;
        }
        else if ( rPropName == UPN_HYPH_NO_CAPS )
        {
            pbVal    = &bNoHyphenateCaps;
            pbResVal = &bResNoHyphenateCaps;
        }

        if (pnVal && pnResVal)
        {
            GetPropSet()->getPropertyValue( rPropName ) >>= *pnVal;
            *pnResVal = *pnVal;
        }
        else if (pbVal && pbResVal)
        {
            GetPropSet()->getPropertyValue( rPropName ) >>= *pbVal;
            *pbResVal = *pbVal;
        }
    }
}


bool PropertyHelper_Hyphen::propertyChange_Impl( const PropertyChangeEvent& rEvt )
{
    bool bRes = PropertyChgHelper::propertyChange_Impl( rEvt );

    if (!bRes  &&  GetPropSet().is()  &&  rEvt.Source == GetPropSet())
    {
        sal_Int16   *pnVal = nullptr;
        bool *pbVal = nullptr;
        switch (rEvt.PropertyHandle)
        {
            case UPH_HYPH_MIN_LEADING     : pnVal = &nHyphMinLeading; break;
            case UPH_HYPH_MIN_TRAILING    : pnVal = &nHyphMinTrailing; break;
            case UPH_HYPH_COMPOUND_MIN_LEADING : pnVal = &nHyphCompoundMinLeading; break;
            case UPH_HYPH_MIN_WORD_LENGTH : pnVal = &nHyphMinWordLength; break;
            case UPH_HYPH_ZONE            : pnVal = &nHyphTextHyphenZone; break;
            case UPH_HYPH_NO_CAPS : pbVal = &bNoHyphenateCaps; break;
            default:
                SAL_WARN( "linguistic", "unknown property handle " << rEvt.PropertyHandle << " (check in include/unotools/linguprops.hxx)");
        }
        if (pnVal)
            rEvt.NewValue >>= *pnVal;
        else if (pbVal)
            rEvt.NewValue >>= *pbVal;

        bRes = (pnVal != nullptr || pbVal != nullptr);
        if (bRes)
        {
            LinguServiceEvent aEvt(GetEvtObj(), LinguServiceEventFlags::HYPHENATE_AGAIN);
            LaunchEvent(aEvt);
        }
    }

    return bRes;
}


void SAL_CALL
    PropertyHelper_Hyphen::propertyChange( const PropertyChangeEvent& rEvt )
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
    nResHyphCompoundMinLeading = nHyphCompoundMinLeading;
    nResHyphMinWordLength   = nHyphMinWordLength;
    nResHyphTextHyphenZone  = nHyphTextHyphenZone;
    bResNoHyphenateCaps = bNoHyphenateCaps;

    for (const PropertyValue& rVal : rPropVals)
    {
        sal_Int16 *pnResVal = nullptr;
        bool *pbResVal = nullptr;

        if ( rVal.Name == UPN_HYPH_MIN_LEADING )
            pnResVal = &nResHyphMinLeading;
        else if ( rVal.Name == UPN_HYPH_MIN_TRAILING )
            pnResVal = &nResHyphMinTrailing;
        if ( rVal.Name == UPN_HYPH_COMPOUND_MIN_LEADING )
            pnResVal = &nResHyphCompoundMinLeading;
        else if ( rVal.Name == UPN_HYPH_MIN_WORD_LENGTH )
            pnResVal = &nResHyphMinWordLength;
        else if ( rVal.Name == UPN_HYPH_ZONE )
            pnResVal = &nResHyphTextHyphenZone;
        else if ( rVal.Name == UPN_HYPH_NO_CAPS )
            pbResVal = &bResNoHyphenateCaps;

        SAL_WARN_IF( !(pnResVal || pbResVal), "linguistic", "unknown property '" << rVal.Name << "'");

        if (pnResVal)
            rVal.Value >>= *pnResVal;
        else if (pbResVal)
            rVal.Value >>= *pbResVal;
    }
}

PropertyHelper_Thesaurus::PropertyHelper_Thesaurus(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet )
{
    mxPropHelper = new PropertyHelper_Thes( rxSource, rxPropSet );
}

PropertyHelper_Thesaurus::~PropertyHelper_Thesaurus()
{
}

void PropertyHelper_Thesaurus::AddAsPropListener()
{
    mxPropHelper->AddAsPropListener();
}

void PropertyHelper_Thesaurus::RemoveAsPropListener()
{
    mxPropHelper->RemoveAsPropListener();
}

void PropertyHelper_Thesaurus::SetTmpPropVals( const css::beans::PropertyValues &rPropVals )
{
    mxPropHelper->SetTmpPropVals( rPropVals );
}

PropertyHelper_Hyphenation::PropertyHelper_Hyphenation(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet)
{
    mxPropHelper = new PropertyHelper_Hyphen( rxSource, rxPropSet );
}

PropertyHelper_Hyphenation::~PropertyHelper_Hyphenation()
{
}

void PropertyHelper_Hyphenation::AddAsPropListener()
{
    mxPropHelper->AddAsPropListener();
}

void PropertyHelper_Hyphenation::RemoveAsPropListener()
{
    mxPropHelper->RemoveAsPropListener();
}

void PropertyHelper_Hyphenation::SetTmpPropVals( const css::beans::PropertyValues &rPropVals )
{
    mxPropHelper->SetTmpPropVals( rPropVals );
}

sal_Int16 PropertyHelper_Hyphenation::GetMinLeading() const
{
    return mxPropHelper->GetMinLeading();
}

sal_Int16 PropertyHelper_Hyphenation::GetMinTrailing() const
{
    return mxPropHelper->GetMinTrailing();
}

sal_Int16 PropertyHelper_Hyphenation::GetCompoundMinLeading() const
{
    return mxPropHelper->GetCompoundMinLeading();
}

sal_Int16 PropertyHelper_Hyphenation::GetMinWordLength() const
{
    return mxPropHelper->GetMinWordLength();
}

sal_Int16 PropertyHelper_Hyphenation::GetTextHyphenZone() const
{
    return mxPropHelper->GetTextHyphenZone();
}

bool PropertyHelper_Hyphenation::IsNoHyphenateCaps() const
{
    return mxPropHelper->IsNoHyphenateCaps();
}

bool PropertyHelper_Hyphenation::addLinguServiceEventListener(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxListener )
{
    return mxPropHelper->addLinguServiceEventListener( rxListener );
}

bool PropertyHelper_Hyphenation::removeLinguServiceEventListener(
                const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >& rxListener )
{
    return mxPropHelper->removeLinguServiceEventListener( rxListener );
}

PropertyHelper_Spelling::PropertyHelper_Spelling(
            const css::uno::Reference< css::uno::XInterface > &rxSource,
            css::uno::Reference< css::linguistic2::XLinguProperties > const &rxPropSet )
{
    mxPropHelper = new PropertyHelper_Spell( rxSource, rxPropSet );
}

PropertyHelper_Spelling::~PropertyHelper_Spelling()
{
}

void PropertyHelper_Spelling::AddAsPropListener()
{
    mxPropHelper->AddAsPropListener();
}

void PropertyHelper_Spelling::RemoveAsPropListener()
{
    mxPropHelper->RemoveAsPropListener();
}

void PropertyHelper_Spelling::SetTmpPropVals( const css::beans::PropertyValues &rPropVals )
{
    mxPropHelper->SetTmpPropVals( rPropVals );
}

bool PropertyHelper_Spelling::IsSpellUpperCase() const
{
    return mxPropHelper->IsSpellUpperCase();
}

bool PropertyHelper_Spelling::IsSpellWithDigits() const
{
    return mxPropHelper->IsSpellWithDigits();
}

bool PropertyHelper_Spelling::IsSpellClosedCompound() const
{
    return mxPropHelper->IsSpellClosedCompound();
}

bool PropertyHelper_Spelling::IsSpellHyphenatedCompound() const
{
    return mxPropHelper->IsSpellHyphenatedCompound();
}

bool PropertyHelper_Spelling::addLinguServiceEventListener(
                const css::uno::Reference<
                    css::linguistic2::XLinguServiceEventListener >& rxListener )
{
    return mxPropHelper->addLinguServiceEventListener( rxListener );
}

bool PropertyHelper_Spelling::removeLinguServiceEventListener(
                const css::uno::Reference<
                    css::linguistic2::XLinguServiceEventListener >& rxListener )
{
    return mxPropHelper->removeLinguServiceEventListener( rxListener );
}

}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
