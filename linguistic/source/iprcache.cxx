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


#include <string.h>

#include <iprcache.hxx>
#include <linguistic/misc.hxx>

#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <osl/mutex.hxx>
#include <linguistic/lngprops.hxx>
#include <unotools/linguprops.hxx>

using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::linguistic2;


namespace linguistic
{


#define NUM_FLUSH_PROPS     6

static const struct
{
    const char *pPropName;
    sal_Int32       nPropHdl;
} aFlushProperties[ NUM_FLUSH_PROPS ] =
{
    { UPN_IS_USE_DICTIONARY_LIST,         UPH_IS_USE_DICTIONARY_LIST },
    { UPN_IS_IGNORE_CONTROL_CHARACTERS,   UPH_IS_IGNORE_CONTROL_CHARACTERS },
    { UPN_IS_SPELL_UPPER_CASE,            UPH_IS_SPELL_UPPER_CASE },
    { UPN_IS_SPELL_WITH_DIGITS,           UPH_IS_SPELL_WITH_DIGITS },
    { UPN_IS_SPELL_CAPITALIZATION,        UPH_IS_SPELL_CAPITALIZATION }
};


static void lcl_AddAsPropertyChangeListener(
        const Reference< XPropertyChangeListener >& xListener,
        Reference< XLinguProperties > const &rPropSet )
{
    if (xListener.is() && rPropSet.is())
    {
        for (auto& aFlushPropertie : aFlushProperties)
        {
            rPropSet->addPropertyChangeListener(
                    OUString::createFromAscii(aFlushPropertie.pPropName), xListener );
        }
    }
}


static void lcl_RemoveAsPropertyChangeListener(
        const Reference< XPropertyChangeListener >& xListener,
        Reference< XLinguProperties > const &rPropSet )
{
    if (xListener.is() && rPropSet.is())
    {
        for (auto& aFlushPropertie : aFlushProperties)
        {
            rPropSet->removePropertyChangeListener(
                    OUString::createFromAscii(aFlushPropertie.pPropName), xListener );
        }
    }
}


static bool lcl_IsFlushProperty( sal_Int32 nHandle )
{
    int i;
    for (i = 0;  i < NUM_FLUSH_PROPS;  ++i)
    {
        if (nHandle == aFlushProperties[i].nPropHdl)
            break;
    }
    return i < NUM_FLUSH_PROPS;
}


void FlushListener::SetDicList( Reference<XSearchableDictionaryList> const &rDL )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (xDicList != rDL)
    {
        if (xDicList.is())
            xDicList->removeDictionaryListEventListener( this );

        xDicList = rDL;
        if (xDicList.is())
            xDicList->addDictionaryListEventListener( this, false );
    }
}


void FlushListener::SetPropSet( Reference< XLinguProperties > const &rPS )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (xPropSet != rPS)
    {
        if (xPropSet.is())
            lcl_RemoveAsPropertyChangeListener( this, xPropSet );

        xPropSet = rPS;
        if (xPropSet.is())
            lcl_AddAsPropertyChangeListener( this, xPropSet );
    }
}


void SAL_CALL FlushListener::disposing( const EventObject& rSource )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (xDicList.is()  &&  rSource.Source == xDicList)
    {
        xDicList->removeDictionaryListEventListener( this );
        xDicList = nullptr;    //! release reference
    }
    if (xPropSet.is()  &&  rSource.Source == xPropSet)
    {
        lcl_RemoveAsPropertyChangeListener( this, xPropSet );
        xPropSet = nullptr;    //! release reference
    }
}


void SAL_CALL FlushListener::processDictionaryListEvent(
            const DictionaryListEvent& rDicListEvent )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (rDicListEvent.Source == xDicList)
    {
        sal_Int16 nEvt = rDicListEvent.nCondensedEvent;
        sal_Int16 const nFlushFlags =
                DictionaryListEventFlags::ADD_NEG_ENTRY     |
                DictionaryListEventFlags::DEL_POS_ENTRY     |
                DictionaryListEventFlags::ACTIVATE_NEG_DIC  |
                DictionaryListEventFlags::DEACTIVATE_POS_DIC;
        bool bFlush = 0 != (nEvt & nFlushFlags);

        if (bFlush)
            mrSpellCache.Flush();
    }
}


void SAL_CALL FlushListener::propertyChange(
            const PropertyChangeEvent& rEvt )
{
    MutexGuard  aGuard( GetLinguMutex() );

    if (rEvt.Source == xPropSet)
    {
        bool bFlush = lcl_IsFlushProperty( rEvt.PropertyHandle );

        if (bFlush)
            mrSpellCache.Flush();
    }
}


SpellCache::SpellCache()
{
    mxFlushLstnr = new FlushListener( *this );
    Reference<XSearchableDictionaryList> aDictionaryList(GetDictionaryList());
    mxFlushLstnr->SetDicList( aDictionaryList ); //! after reference is established
    Reference<XLinguProperties> aPropertySet(GetLinguProperties());
    mxFlushLstnr->SetPropSet( aPropertySet );    //! after reference is established
}

SpellCache::~SpellCache()
{
    Reference<XSearchableDictionaryList>  aEmptyList;
    Reference<XLinguProperties>     aEmptySet;
    mxFlushLstnr->SetDicList( aEmptyList );
    mxFlushLstnr->SetPropSet( aEmptySet );
}

void SpellCache::Flush()
{
    MutexGuard  aGuard( GetLinguMutex() );
    // clear word list
    LangWordList_t aEmpty;
    aWordLists.swap( aEmpty );
}

bool SpellCache::CheckWord( const OUString& rWord, LanguageType nLang )
{
    MutexGuard  aGuard( GetLinguMutex() );
    WordList_t &rList = aWordLists[ nLang ];
    const WordList_t::const_iterator aIt = rList.find( rWord );
    return aIt != rList.end();
}

void SpellCache::AddWord( const OUString& rWord, LanguageType nLang )
{
    MutexGuard  aGuard( GetLinguMutex() );
    WordList_t & rList = aWordLists[ nLang ];
    // occasional clean-up...
    if (rList.size() > 500)
        rList.clear();
    rList.insert( rWord );
}

}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
