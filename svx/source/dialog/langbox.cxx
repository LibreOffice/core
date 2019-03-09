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

#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <linguistic/misc.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <tools/urlobj.hxx>
#include <svtools/langtab.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/lang.h>
#include <editeng/scripttypeitem.hxx>
#include <editeng/unolingu.hxx>
#include <svx/langbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <bitmaps.hlst>
#include <vcl/builderfactory.hxx>
#include <vcl/i18nhelp.hxx>

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;


static_assert((LISTBOX_APPEND == COMBOBOX_APPEND) && (LISTBOX_ENTRY_NOTFOUND == COMBOBOX_ENTRY_NOTFOUND), "If these ever dispersed we'd need a solution");

OUString GetDicInfoStr( const OUString& rName, const LanguageType nLang, bool bNeg )
{
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INetProtocol::File );
    aURLObj.SetSmartURL( rName, INetURLObject::EncodeMechanism::All );
    OUString aTmp( aURLObj.GetBase() );
    aTmp += " ";

    if ( bNeg )
    {
        aTmp += " (-) ";
    }

    if ( LANGUAGE_NONE == nLang )
        aTmp += SvxResId(RID_SVXSTR_LANGUAGE_ALL);
    else
    {
        aTmp += "[";
        aTmp += SvtLanguageTable::GetLanguageString( nLang );
        aTmp += "]";
    }

    return aTmp;
}


//  misc local helper functions


static std::vector< LanguageType > lcl_LocaleSeqToLangSeq( Sequence< css::lang::Locale > const &rSeq )
{
    const css::lang::Locale *pLocale = rSeq.getConstArray();
    sal_Int32 nCount = rSeq.getLength();

    std::vector< LanguageType >   aLangs;
    aLangs.reserve(nCount);
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        aLangs.push_back( LanguageTag::convertToLanguageType( pLocale[i] ) );

    }

    return aLangs;
}


static bool lcl_SeqHasLang( const Sequence< sal_Int16 > & rLangSeq, sal_Int16 nLang )
{
    sal_Int32 i = -1;
    sal_Int32 nLen = rLangSeq.getLength();
    if (nLen)
    {
        const sal_Int16 *pLang = rLangSeq.getConstArray();
        for (i = 0;  i < nLen;  ++i)
        {
            if (nLang == pLang[i])
                break;
        }
    }
    return i >= 0  &&  i < nLen;
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSvxLanguageBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP;
    bool bDropdown = BuilderUtils::extractDropdown(rMap);
    if (bDropdown)
        nBits |= WB_DROPDOWN;
    else
        nBits |= WB_BORDER;
    VclPtrInstance<SvxLanguageBox> pLanguageBox(pParent, nBits);
    pLanguageBox->EnableAutoSize(true);
    rRet = pLanguageBox;
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSvxLanguageComboBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP;
    bool bDropdown = BuilderUtils::extractDropdown(rMap);
    if (bDropdown)
        nBits |= WB_DROPDOWN;
    else
        nBits |= WB_BORDER;
    VclPtrInstance<SvxLanguageComboBox> pLanguageBox(pParent, nBits);
    pLanguageBox->EnableAutoSize(true);
    rRet = pLanguageBox;
}

SvxLanguageBoxBase::SvxLanguageBoxBase()
    : m_bHasLangNone(false)
    , m_bLangNoneIsLangAll(false)
    , m_bWithCheckmark(false)
{
}

void SvxLanguageBoxBase::ImplLanguageBoxBaseInit()
{
    m_aNotCheckedImage = Image(StockImage::Yes, RID_SVXBMP_NOTCHECKED);
    m_aCheckedImage = Image(StockImage::Yes, RID_SVXBMP_CHECKED);
    m_aAllString            = SvxResId( RID_SVXSTR_LANGUAGE_ALL );
    m_bHasLangNone          = false;
    m_bLangNoneIsLangAll    = false;

    if ( m_bWithCheckmark )
    {
        sal_uInt32 nCount = SvtLanguageTable::GetLanguageEntryCount();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            LanguageType nLangType = SvtLanguageTable::GetLanguageTypeAtIndex( i );

            bool bInsert = true;
            if ((LANGUAGE_DONTKNOW == nLangType)  ||
                (LANGUAGE_SYSTEM   == nLangType))
            {
                bInsert = false;
            }

            if ( bInsert )
                InsertLanguage( nLangType );
        }
    }
}


SvxLanguageBoxBase::~SvxLanguageBoxBase()
{
}

namespace {

bool lcl_isPrerequisite( LanguageType nLangType )
{
    return
        nLangType != LANGUAGE_DONTKNOW &&
        nLangType != LANGUAGE_SYSTEM &&
        nLangType != LANGUAGE_NONE &&
        !MsLangId::isLegacy( nLangType) &&
        MsLangId::getSubLanguage( nLangType);
}

bool lcl_isScriptTypeRequested( LanguageType nLangType, SvxLanguageListFlags nLangList )
{
    return
        bool(nLangList & SvxLanguageListFlags::ALL) ||
        (bool(nLangList & SvxLanguageListFlags::WESTERN) &&
         (SvtLanguageOptions::GetScriptTypeOfLanguage(nLangType) == SvtScriptType::LATIN)) ||
        (bool(nLangList & SvxLanguageListFlags::CTL) &&
         (SvtLanguageOptions::GetScriptTypeOfLanguage(nLangType) == SvtScriptType::COMPLEX)) ||
        (bool(nLangList & SvxLanguageListFlags::CJK) &&
         (SvtLanguageOptions::GetScriptTypeOfLanguage(nLangType) == SvtScriptType::ASIAN));
}

}

void SvxLanguageBoxBase::AddLanguages( const std::vector< LanguageType >& rLanguageTypes,
        SvxLanguageListFlags nLangList )
{
    for ( auto const & nLangType : rLanguageTypes )
    {
        if (lcl_isPrerequisite( nLangType ))
        {
            LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( nLangType );
            if (lcl_isScriptTypeRequested( nLang, nLangList))
            {
                sal_Int32 nAt = ImplTypeToPos( nLang );
                if (nAt == LISTBOX_ENTRY_NOTFOUND)
                    InsertLanguage( nLang );
            }
        }
    }
}

void SvxLanguageBoxBase::SetLanguageList( SvxLanguageListFlags nLangList,
        bool bHasLangNone, bool bCheckSpellAvail )
{
    ImplClear();

    m_bHasLangNone          = bHasLangNone;
    m_bLangNoneIsLangAll    = false;
    m_bWithCheckmark        = bCheckSpellAvail;

    if ( SvxLanguageListFlags::EMPTY == nLangList )
        return;

    bool bAddAvailable = (!(nLangList & SvxLanguageListFlags::ONLY_KNOWN) &&
            ((nLangList & SvxLanguageListFlags::ALL) ||
             (nLangList & SvxLanguageListFlags::WESTERN) ||
             (nLangList & SvxLanguageListFlags::CTL) ||
             (nLangList & SvxLanguageListFlags::CJK)));
    std::vector< LanguageType > aSpellAvailLang;
    std::vector< LanguageType > aHyphAvailLang;
    std::vector< LanguageType > aThesAvailLang;
    Sequence< sal_Int16 > aSpellUsedLang;
    Reference< XAvailableLocales > xAvail( LinguMgr::GetLngSvcMgr(), UNO_QUERY );
    if (xAvail.is())
    {
        Sequence< css::lang::Locale > aTmp;

        if (bAddAvailable)
        {
            aTmp = xAvail->getAvailableLocales( SN_SPELLCHECKER );
            aSpellAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
        }
        if (bAddAvailable)
        {
            aTmp = xAvail->getAvailableLocales( SN_HYPHENATOR );
            aHyphAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
        }
        if (bAddAvailable)
        {
            aTmp = xAvail->getAvailableLocales( SN_THESAURUS );
            aThesAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
        }
    }
    if (SvxLanguageListFlags::SPELL_USED & nLangList)
    {
        Reference< XSpellChecker1 > xTmp1( LinguMgr::GetSpellChecker(), UNO_QUERY );
        if (xTmp1.is())
            aSpellUsedLang = xTmp1->getLanguages();
    }

    std::vector<LanguageType> aKnown;
    sal_uInt32 nCount;
    if ( nLangList & SvxLanguageListFlags::ONLY_KNOWN )
    {
        aKnown = LocaleDataWrapper::getInstalledLanguageTypes();
        nCount = aKnown.size();
    }
    else
    {
        nCount = SvtLanguageTable::GetLanguageEntryCount();
    }
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        LanguageType nLangType;
        if ( nLangList & SvxLanguageListFlags::ONLY_KNOWN )
            nLangType = aKnown[i];
        else
            nLangType = SvtLanguageTable::GetLanguageTypeAtIndex( i );
        if ( lcl_isPrerequisite( nLangType ) &&
             (lcl_isScriptTypeRequested( nLangType, nLangList) ||
              (bool(nLangList & SvxLanguageListFlags::FBD_CHARS) &&
               MsLangId::hasForbiddenCharacters(nLangType)) ||
              (bool(nLangList & SvxLanguageListFlags::SPELL_USED) &&
               lcl_SeqHasLang(aSpellUsedLang, static_cast<sal_uInt16>(nLangType)))
              ) )
            InsertLanguage( nLangType );
    }

    if (bAddAvailable)
    {
        // Spell checkers, hyphenators and thesauri may add language tags
        // unknown so far.
        AddLanguages( aSpellAvailLang, nLangList);
        AddLanguages( aHyphAvailLang, nLangList);
        AddLanguages( aThesAvailLang, nLangList);
    }

    if (bHasLangNone)
        InsertLanguage( LANGUAGE_NONE );
}


sal_Int32 SvxLanguageBoxBase::InsertLanguage( const LanguageType nLangType )
{
    return ImplInsertLanguage( nLangType, LISTBOX_APPEND, css::i18n::ScriptType::WEAK );
}


sal_Int32 SvxLanguageBoxBase::ImplInsertLanguage( const LanguageType nLangType, sal_Int32 nPos, sal_Int16 nType )
{
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( nLangType);
    // For obsolete and to be replaced languages check whether an entry of the
    // replacement already exists and if so don't add an entry with identical
    // string as would be returned by SvtLanguageTable::GetString().
    if (nLang != nLangType)
    {
        sal_Int32 nAt = ImplTypeToPos( nLang );
        if ( nAt != LISTBOX_ENTRY_NOTFOUND )
            return nAt;
    }

    OUString aStrEntry = SvtLanguageTable::GetLanguageString( nLang );
    if (LANGUAGE_NONE == nLang && m_bHasLangNone && m_bLangNoneIsLangAll)
        aStrEntry = m_aAllString;

    LanguageType nRealLang = nLang;
    if (nRealLang == LANGUAGE_SYSTEM)
    {
        nRealLang = MsLangId::resolveSystemLanguageByScriptType(nRealLang, nType);
        aStrEntry += " - ";
        aStrEntry += SvtLanguageTable::GetLanguageString( nRealLang );
    } else if (nRealLang == LANGUAGE_USER_SYSTEM_CONFIG) {
        nRealLang = MsLangId::getSystemLanguage();
        aStrEntry += " - ";
        aStrEntry += SvtLanguageTable::GetLanguageString( nRealLang );
    }

    aStrEntry = ApplyLreOrRleEmbedding( aStrEntry );

    sal_Int32 nAt = 0;
    if ( m_bWithCheckmark )
    {
        bool bFound = false;

        if (!m_pSpellUsedLang)
        {
            Reference< XSpellChecker1 > xSpell( LinguMgr::GetSpellChecker(), UNO_QUERY );
            if ( xSpell.is() )
                m_pSpellUsedLang.reset( new Sequence< sal_Int16 >( xSpell->getLanguages() ) );
        }
        bFound = m_pSpellUsedLang &&
            lcl_SeqHasLang( *m_pSpellUsedLang, static_cast<sal_uInt16>(nRealLang) );

        nAt = ImplInsertImgEntry( aStrEntry, nPos, bFound );
    }
    else
        nAt = ImplInsertEntry( aStrEntry, nPos );

    ImplSetEntryData( nAt, reinterpret_cast<void*>(static_cast<sal_uInt16>(nLangType)) );
    return nAt;
}


void SvxLanguageBoxBase::InsertDefaultLanguage( sal_Int16 nType )
{
    ImplInsertLanguage( LANGUAGE_SYSTEM, LISTBOX_APPEND, nType );
}


void SvxLanguageBoxBase::InsertSystemLanguage()
{
    ImplInsertLanguage( LANGUAGE_USER_SYSTEM_CONFIG, LISTBOX_APPEND, css::i18n::ScriptType::WEAK );
}


void SvxLanguageBoxBase::InsertLanguage( const LanguageType nLangType,
        bool bCheckEntry )
{
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( nLangType);
    // For obsolete and to be replaced languages check whether an entry of the
    // replacement already exists and if so don't add an entry with identical
    // string as would be returned by SvtLanguageTable::GetString().
    if (nLang != nLangType)
    {
        sal_Int32 nAt = ImplTypeToPos( nLang );
        if ( nAt != LISTBOX_ENTRY_NOTFOUND )
            return;
    }

    OUString aStrEntry = SvtLanguageTable::GetLanguageString( nLang );
    if (LANGUAGE_NONE == nLang && m_bHasLangNone && m_bLangNoneIsLangAll)
        aStrEntry = m_aAllString;

    sal_Int32 nAt = ImplInsertImgEntry( aStrEntry, LISTBOX_APPEND, bCheckEntry );
    ImplSetEntryData( nAt, reinterpret_cast<void*>(static_cast<sal_uInt16>(nLang)) );
}


LanguageType SvxLanguageBoxBase::GetSelectedLanguage() const
{
    sal_Int32     nPos   = ImplGetSelectedEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return LanguageType( reinterpret_cast<sal_uIntPtr>(ImplGetEntryData(nPos)) );
    else
        return LANGUAGE_DONTKNOW;
}


void SvxLanguageBoxBase::SelectLanguage( const LanguageType eLangType )
{
    // If the core uses a LangID of an imported MS document and wants to select
    // a language that is replaced, we need to select the replacement instead.
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( eLangType);

    sal_Int32 nAt = ImplTypeToPos( nLang );

    if ( nAt == LISTBOX_ENTRY_NOTFOUND )
        nAt = InsertLanguage( nLang );      // on-the-fly-ID

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        ImplSelectEntryPos( nAt, true/*bSelect*/ );
}


bool SvxLanguageBoxBase::IsLanguageSelected( const LanguageType eLangType ) const
{
    // Same here, work on the replacement if applicable.
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( eLangType);

    sal_Int32 nAt = ImplTypeToPos( nLang );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        return ImplIsEntryPosSelected( nAt );
    else
        return false;
}


sal_Int32 SvxLanguageBoxBase::ImplTypeToPos( LanguageType eType ) const
{
    return ImplGetEntryPos( reinterpret_cast<void*>(static_cast<sal_uInt16>(eType)) );
}


LanguageType LanguageBox::get_active_id() const
{
    OUString sLang = m_xControl->get_active_id();
    if (!sLang.isEmpty())
        return LanguageType(sLang.toInt32());
    else
        return LANGUAGE_DONTKNOW;
}

int LanguageBox::find_id(const LanguageType eLangType) const
{
    return m_xControl->find_id(OUString::number(static_cast<sal_uInt16>(eLangType)));
}

void LanguageBox::set_id(int pos, const LanguageType eLangType)
{
    m_xControl->set_id(pos, OUString::number(static_cast<sal_uInt16>(eLangType)));
}

LanguageType LanguageBox::get_id(int pos) const
{
    return LanguageType(m_xControl->get_id(pos).toInt32());
}

void LanguageBox::remove_id(const LanguageType eLangType)
{
    m_xControl->remove_id(OUString::number(static_cast<sal_uInt16>(eLangType)));
}

void LanguageBox::append(const LanguageType eLangType, const OUString& rStr)
{
    m_xControl->append(OUString::number(static_cast<sal_uInt16>(eLangType)), rStr);
}

void LanguageBox::set_active_id(const LanguageType eLangType)
{
    // If the core uses a LangID of an imported MS document and wants to select
    // a language that is replaced, we need to select the replacement instead.
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( eLangType);

    sal_Int32 nAt = ImplTypeToPos( nLang );

    if (nAt == -1)
    {
        InsertLanguage( nLang );      // on-the-fly-ID
        nAt = ImplTypeToPos( nLang );
    }

    if (nAt != -1)
        m_xControl->set_active(nAt);
}

void LanguageBox::AddLanguages(const std::vector< LanguageType >& rLanguageTypes,
        SvxLanguageListFlags nLangList, std::vector<weld::ComboBoxEntry>& rEntries)
{
    for ( auto const & nLangType : rLanguageTypes )
    {
        if (lcl_isPrerequisite( nLangType ))
        {
            LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( nLangType );
            if (lcl_isScriptTypeRequested( nLang, nLangList))
            {
                int nAt = ImplTypeToPos(nLang);
                if (nAt != -1)
                    continue;
                weld::ComboBoxEntry aNewEntry(BuildEntry(nLang));
                if (aNewEntry.sString.isEmpty())
                    continue;
                if (std::find_if(rEntries.begin(), rEntries.end(),
                                 [=](const weld::ComboBoxEntry& rEntry){ return rEntry.sId == aNewEntry.sId; }) != rEntries.end())
                    continue;
                rEntries.push_back(aNewEntry);
            }
        }
    }
}

void LanguageBox::ImplClear()
{
    m_xControl->clear();
}

void LanguageBox::SetLanguageList( SvxLanguageListFlags nLangList,
        bool bHasLangNone, bool bLangNoneIsLangAll, bool bCheckSpellAvail )
{
    m_bHasLangNone          = bHasLangNone;
    m_bLangNoneIsLangAll    = bLangNoneIsLangAll;
    m_bWithCheckmark        = bCheckSpellAvail;

    if (SvxLanguageListFlags::EMPTY == nLangList)
    {
        ImplClear();
        return;
    }

    bool bAddAvailable = (!(nLangList & SvxLanguageListFlags::ONLY_KNOWN) &&
            ((nLangList & SvxLanguageListFlags::ALL) ||
             (nLangList & SvxLanguageListFlags::WESTERN) ||
             (nLangList & SvxLanguageListFlags::CTL) ||
             (nLangList & SvxLanguageListFlags::CJK)));
    std::vector< LanguageType > aSpellAvailLang;
    std::vector< LanguageType > aHyphAvailLang;
    std::vector< LanguageType > aThesAvailLang;
    Sequence< sal_Int16 > aSpellUsedLang;
    Reference< XAvailableLocales > xAvail( LinguMgr::GetLngSvcMgr(), UNO_QUERY );
    if (xAvail.is())
    {
        Sequence< css::lang::Locale > aTmp;

        if (bAddAvailable)
        {
            aTmp = xAvail->getAvailableLocales( SN_SPELLCHECKER );
            aSpellAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
        }
        if (bAddAvailable)
        {
            aTmp = xAvail->getAvailableLocales( SN_HYPHENATOR );
            aHyphAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
        }
        if (bAddAvailable)
        {
            aTmp = xAvail->getAvailableLocales( SN_THESAURUS );
            aThesAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
        }
    }
    if (SvxLanguageListFlags::SPELL_USED & nLangList)
    {
        Reference< XSpellChecker1 > xTmp1( LinguMgr::GetSpellChecker(), UNO_QUERY );
        if (xTmp1.is())
            aSpellUsedLang = xTmp1->getLanguages();
    }

    std::vector<LanguageType> aKnown;
    sal_uInt32 nCount;
    if ( nLangList & SvxLanguageListFlags::ONLY_KNOWN )
    {
        aKnown = LocaleDataWrapper::getInstalledLanguageTypes();
        nCount = aKnown.size();
    }
    else
    {
        nCount = SvtLanguageTable::GetLanguageEntryCount();
    }

    std::vector<weld::ComboBoxEntry> aEntries;
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        LanguageType nLangType;
        if ( nLangList & SvxLanguageListFlags::ONLY_KNOWN )
            nLangType = aKnown[i];
        else
            nLangType = SvtLanguageTable::GetLanguageTypeAtIndex( i );
        if ( lcl_isPrerequisite( nLangType ) &&
             (lcl_isScriptTypeRequested( nLangType, nLangList) ||
              (bool(nLangList & SvxLanguageListFlags::FBD_CHARS) &&
               MsLangId::hasForbiddenCharacters(nLangType)) ||
              (bool(nLangList & SvxLanguageListFlags::SPELL_USED) &&
               lcl_SeqHasLang(aSpellUsedLang, static_cast<sal_uInt16>(nLangType)))
              ) )
        {
            aEntries.push_back(BuildEntry(nLangType));
            if (aEntries.back().sString.isEmpty())
                aEntries.pop_back();
        }
    }

    if (bAddAvailable)
    {
        // Spell checkers, hyphenators and thesauri may add language tags
        // unknown so far.
        AddLanguages(aSpellAvailLang, nLangList, aEntries);
        AddLanguages(aHyphAvailLang, nLangList, aEntries);
        AddLanguages(aThesAvailLang, nLangList, aEntries);
    }

    if (bHasLangNone)
       aEntries.push_back(BuildEntry(LANGUAGE_NONE));

    m_xControl->insert_vector(aEntries, false);
}

int LanguageBox::ImplTypeToPos(LanguageType eType) const
{
    return m_xControl->find_id(OUString::number(static_cast<sal_uInt16>(eType)));
}

void LanguageBox::InsertLanguage(const LanguageType nLangType)
{
    weld::ComboBoxEntry aEntry = BuildEntry(nLangType);
    if (aEntry.sString.isEmpty())
        return;
    if (aEntry.sImage.isEmpty())
        m_xControl->append(aEntry.sId, aEntry.sString);
    else
        m_xControl->append(aEntry.sId, aEntry.sString, aEntry.sImage);
}

weld::ComboBoxEntry LanguageBox::BuildEntry(const LanguageType nLangType)
{
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage(nLangType);
    // For obsolete and to be replaced languages check whether an entry of the
    // replacement already exists and if so don't add an entry with identical
    // string as would be returned by SvtLanguageTable::GetString().
    if (nLang != nLangType)
    {
        int nAt = ImplTypeToPos( nLang );
        if (nAt != -1)
            return weld::ComboBoxEntry("");
    }

    OUString aStrEntry = SvtLanguageTable::GetLanguageString( nLang );
    if (LANGUAGE_NONE == nLang && m_bHasLangNone && m_bLangNoneIsLangAll)
        aStrEntry = m_aAllString;

    LanguageType nRealLang = nLang;
    if (nRealLang == LANGUAGE_SYSTEM)
    {
        nRealLang = MsLangId::resolveSystemLanguageByScriptType(nRealLang, css::i18n::ScriptType::WEAK);
        aStrEntry += " - ";
        aStrEntry += SvtLanguageTable::GetLanguageString( nRealLang );
    }
    else if (nRealLang == LANGUAGE_USER_SYSTEM_CONFIG)
    {
        nRealLang = MsLangId::getSystemLanguage();
        aStrEntry += " - ";
        aStrEntry += SvtLanguageTable::GetLanguageString( nRealLang );
    }

    if (m_bWithCheckmark)
    {
        if (!m_xSpellUsedLang)
        {
            Reference<XSpellChecker1> xSpell(LinguMgr::GetSpellChecker(), UNO_QUERY);
            if (xSpell.is())
                m_xSpellUsedLang.reset(new Sequence<sal_Int16>(xSpell->getLanguages()));
        }

        bool bFound = m_xSpellUsedLang && lcl_SeqHasLang(*m_xSpellUsedLang, static_cast<sal_uInt16>(nRealLang));

        return weld::ComboBoxEntry(aStrEntry, OUString::number(static_cast<sal_uInt16>(nLangType)), bFound ? OUString(RID_SVXBMP_CHECKED) : OUString(RID_SVXBMP_NOTCHECKED));
    }
    else
        return weld::ComboBoxEntry(aStrEntry, OUString::number(static_cast<sal_uInt16>(nLangType)));
}

IMPL_LINK(LanguageBox, ChangeHdl, weld::ComboBox&, rControl, void)
{
    if (rControl.has_entry())
    {
        EditedAndValid eOldState = m_eEditedAndValid;
        OUString aStr(rControl.get_active_text());
        if (aStr.isEmpty())
            m_eEditedAndValid = EditedAndValid::Invalid;
        else
        {
            const int nPos = rControl.find_text(aStr);
            if (nPos != -1)
            {
                int nStartSelectPos, nEndSelectPos;
                rControl.get_entry_selection_bounds(nStartSelectPos, nEndSelectPos);

                // Select the corresponding listbox entry if not current. This
                // invalidates the Edit Selection thus has to happen between
                // obtaining the Selection and setting the new Selection.
                int nSelPos = m_xControl->get_active();
                bool bSetEditSelection;
                if (nSelPos == nPos)
                    bSetEditSelection = false;
                else
                {
                    m_xControl->set_active(nPos);
                    bSetEditSelection = true;
                }

                // If typing into the Edit control led us here, advance start of a
                // full selection by one so the next character will already
                // continue the string instead of having to type the same character
                // again to start a new string. The selection is in reverse
                // when obtained from the Edit control.
                if (nEndSelectPos == 0)
                {
                    OUString aText(m_xControl->get_active_text());
                    if (nStartSelectPos == aText.getLength())
                    {
                        ++nEndSelectPos;
                        bSetEditSelection = true;
                    }
                }

                if (bSetEditSelection)
                    rControl.select_entry_region(nStartSelectPos, nEndSelectPos);

                m_eEditedAndValid = EditedAndValid::No;
            }
            else
            {
                OUString aCanonicalized;
                bool bValid = LanguageTag::isValidBcp47( aStr, &aCanonicalized, true);
                m_eEditedAndValid = (bValid ? EditedAndValid::Valid : EditedAndValid::Invalid);
                if (bValid && aCanonicalized != aStr)
                {
                    m_xControl->set_entry_text(aCanonicalized);
                    const auto nCursorPos = aCanonicalized.getLength();
                    m_xControl->select_entry_region(nCursorPos, nCursorPos);
                }
            }
        }
        if (eOldState != m_eEditedAndValid)
        {
            if (m_eEditedAndValid == EditedAndValid::Invalid)
                rControl.set_entry_error(true);
            else
                rControl.set_entry_error(false);
        }
    }
    m_aChangeHdl.Call(rControl);
}

LanguageBox::LanguageBox(std::unique_ptr<weld::ComboBox> pControl)
    : m_xControl(std::move(pControl))
    , m_aAllString(SvxResId(RID_SVXSTR_LANGUAGE_ALL))
    , m_eSavedLanguage(LANGUAGE_DONTKNOW)
    , m_eEditedAndValid(EditedAndValid::No)
    , m_bHasLangNone(false)
    , m_bLangNoneIsLangAll(false)
    , m_bWithCheckmark(false)
{
    m_xControl->make_sorted();
    m_xControl->connect_changed(LINK(this, LanguageBox, ChangeHdl));
}

SvxLanguageBox::SvxLanguageBox( vcl::Window* pParent, WinBits nBits )
    : ListBox( pParent, nBits )
    , SvxLanguageBoxBase()
{
    // display entries sorted
    SetStyle( GetStyle() | WB_SORT );

    ImplLanguageBoxBaseInit();
}

SvxLanguageComboBox::SvxLanguageComboBox( vcl::Window* pParent, WinBits nBits )
    : ComboBox( pParent, nBits )
    , SvxLanguageBoxBase()
    , meEditedAndValid( EditedAndValid::No )
{
    // display entries sorted
    SetStyle( GetStyle() | WB_SORT );

    EnableMultiSelection( false );

    ImplLanguageBoxBaseInit();

    SetModifyHdl( LINK( this, SvxLanguageComboBox, EditModifyHdl ) );
}

sal_Int32 SvxLanguageBox::ImplInsertImgEntry( const OUString& rEntry, sal_Int32 nPos, bool bChecked )
{
    return InsertEntry( rEntry, (bChecked ? m_aCheckedImage : m_aNotCheckedImage), nPos );
}

sal_Int32 SvxLanguageComboBox::ImplInsertImgEntry( const OUString& rEntry, sal_Int32 nPos, bool bChecked )
{
    return InsertEntryWithImage( rEntry, (bChecked ? m_aCheckedImage : m_aNotCheckedImage), nPos );
}


void SvxLanguageBox::ImplClear()
{
    Clear();
}

void SvxLanguageComboBox::ImplClear()
{
    Clear();
}


sal_Int32 SvxLanguageBox::ImplInsertEntry( const OUString& rEntry, sal_Int32 nPos )
{
    return InsertEntry( rEntry, nPos);
}

sal_Int32 SvxLanguageComboBox::ImplInsertEntry( const OUString& rEntry, sal_Int32 nPos )
{
    return InsertEntry( rEntry, nPos);
}


void SvxLanguageBox::ImplSetEntryData( sal_Int32 nPos, void* pData )
{
    SetEntryData( nPos, pData);
}

void SvxLanguageComboBox::ImplSetEntryData( sal_Int32 nPos, void* pData )
{
    SetEntryData( nPos, pData);
}


sal_Int32 SvxLanguageBox::ImplGetSelectedEntryPos() const
{
    return GetSelectedEntryPos();
}

sal_Int32 SvxLanguageComboBox::ImplGetSelectedEntryPos() const
{
    return GetSelectedEntryPos();
}


void* SvxLanguageBox::ImplGetEntryData( sal_Int32 nPos ) const
{
    return GetEntryData( nPos);
}

void* SvxLanguageComboBox::ImplGetEntryData( sal_Int32 nPos ) const
{
    return GetEntryData( nPos);
}


void SvxLanguageBox::ImplSelectEntryPos( sal_Int32 nPos, bool bSelect )
{
    SelectEntryPos( nPos, bSelect);
}

void SvxLanguageComboBox::ImplSelectEntryPos( sal_Int32 nPos, bool bSelect )
{
    SelectEntryPos( nPos, bSelect);
}


bool SvxLanguageBox::ImplIsEntryPosSelected( sal_Int32 nPos ) const
{
    return IsEntryPosSelected( nPos);
}

bool SvxLanguageComboBox::ImplIsEntryPosSelected( sal_Int32 nPos ) const
{
    return IsEntryPosSelected( nPos);
}


sal_Int32 SvxLanguageBox::ImplGetEntryPos( const void* pData ) const
{
    return GetEntryPos( pData);
}

sal_Int32 SvxLanguageComboBox::ImplGetEntryPos( const void* pData ) const
{
    return GetEntryPos( pData);
}


IMPL_LINK_NOARG( SvxLanguageComboBox, EditModifyHdl, Edit&, void )
{
    EditedAndValid eOldState = meEditedAndValid;
    OUString aStr( vcl::I18nHelper::filterFormattingChars( GetText()));
    if (aStr.isEmpty())
        meEditedAndValid = EditedAndValid::Invalid;
    else
    {
        const sal_Int32 nPos = GetEntryPos( aStr);
        if (nPos != COMBOBOX_ENTRY_NOTFOUND)
        {
            Selection aSel( GetSelection());

            // Select the corresponding listbox entry if not current. This
            // invalidates the Edit Selection thus has to happen between
            // obtaining the Selection and setting the new Selection.
            sal_Int32 nSelPos = ImplGetSelectedEntryPos();
            bool bSetEditSelection;
            if (nSelPos == nPos)
                bSetEditSelection = false;
            else
            {
                ImplSelectEntryPos( nPos, true);
                bSetEditSelection = true;
            }

            // If typing into the Edit control led us here, advance start of a
            // full selection by one so the next character will already
            // continue the string instead of having to type the same character
            // again to start a new string. The selection includes formatting
            // characters and is reverse when obtained from the Edit control.
            if (aSel.Max() == 1)
            {
                OUString aText( GetText());
                if (aSel.Min() == aText.getLength())
                {
                    ++aSel.Max();
                    bSetEditSelection = true;
                }
            }

            if (bSetEditSelection)
                SetSelection( aSel);

            meEditedAndValid = EditedAndValid::No;
        }
        else
        {
            OUString aCanonicalized;
            bool bValid = LanguageTag::isValidBcp47( aStr, &aCanonicalized, true);
            meEditedAndValid = (bValid ? EditedAndValid::Valid : EditedAndValid::Invalid);
            if (bValid && aCanonicalized != aStr)
            {
                SetText( aCanonicalized);
                SetSelection( Selection( aCanonicalized.getLength()));
            }
        }
    }
    if (eOldState != meEditedAndValid)
    {
        if (meEditedAndValid == EditedAndValid::Invalid)
        {
#if 0
            //! Gives white on white!?! instead of white on reddish.
            SetControlBackground( ::Color( Color( 0xff, 0x65, 0x63)));
            SetControlForeground( ::COL_WHITE);
#else
            SetControlForeground( ::Color( 0xf0, 0, 0) );
#endif
        }
        else
        {
            SetControlForeground();
            SetControlBackground();
        }
    }
}


sal_Int32 LanguageBox::SaveEditedAsEntry()
{
    if (m_eEditedAndValid != EditedAndValid::Valid)
        return -1;

    LanguageTag aLanguageTag(m_xControl->get_active_text());
    LanguageType nLang = aLanguageTag.getLanguageType();
    if (nLang == LANGUAGE_DONTKNOW)
    {
        SAL_WARN( "svx.dialog", "SvxLanguageComboBox::SaveEditedAsEntry: unknown tag");
        return -1;
    }

    int nPos = ImplTypeToPos( nLang);
    if (nPos != -1)
        return nPos;    // Already present but with a different string.

    if (SvtLanguageTable::HasLanguageType( nLang))
    {
        // In SvtLanguageTable but not in SvxLanguageComboBox. On purpose? This
        // may be an entry with different settings or CTL instead of Western or
        // ... all things we don't handle yet.
        SAL_WARN( "svx.dialog", "SvxLanguageComboBox::SaveEditedAsEntry: already in SvtLanguageTable: " <<
                SvtLanguageTable::GetLanguageString( nLang) << ", " << nLang);
    }
    else
    {
        // Add to both, SvtLanguageTable and SvxLanguageComboBox.
        /* TODO: a descriptive user comment would be a nice to have here. */
        SvtLanguageTable::AddLanguageTag( aLanguageTag );
    }

    InsertLanguage(nLang);
    return ImplTypeToPos(nLang);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
