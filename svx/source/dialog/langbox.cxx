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

#include <sal/config.h>

#include <map>
#include <unordered_map>

#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager2.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <linguistic/misc.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <unotools/localedatawrapper.hxx>
#include <tools/urlobj.hxx>
#include <svtools/langtab.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetagicu.hxx>
#include <editeng/unolingu.hxx>
#include <svl/languageoptions.hxx>
#include <svx/langbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <bitmaps.hlst>
#include <o3tl/sorted_vector.hxx>

#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/scopeguard.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;

OUString GetDicInfoStr( std::u16string_view rName, const LanguageType nLang, bool bNeg )
{
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INetProtocol::File );
    aURLObj.SetSmartURL( rName, INetURLObject::EncodeMechanism::All );
    OUString aTmp( aURLObj.GetBase() + " " );

    if ( bNeg )
    {
        aTmp += " (-) ";
    }

    if ( LANGUAGE_NONE == nLang )
        aTmp += SvxResId(RID_SVXSTR_LANGUAGE_ALL);
    else
    {
        aTmp += "[" + SvtLanguageTable::GetLanguageString( nLang ) + "]";
    }

    return aTmp;
}

//  misc local helper functions
static void appendLocaleSeqToLangs(Sequence<css::lang::Locale> const& rSeq,
                                   std::vector<LanguageType>& aLangs)
{
    sal_Int32 nCount = rSeq.getLength();

    aLangs.reserve(aLangs.size() + nCount);

    std::transform(rSeq.begin(), rSeq.end(), std::back_inserter(aLangs),
        [](const css::lang::Locale& rLocale) -> LanguageType {
            return LanguageTag::convertToLanguageType(rLocale); });
}

static bool lcl_SeqHasLang( const Sequence< sal_Int16 > & rLangSeq, sal_Int16 nLang )
{
    return rLangSeq.hasElements()
        && std::find(rLangSeq.begin(), rLangSeq.end(), nLang) != rLangSeq.end();
}

namespace {

bool lcl_isPrerequisite(LanguageType nLangType, bool requireSublang)
{
    return
        nLangType != LANGUAGE_DONTKNOW &&
        nLangType != LANGUAGE_SYSTEM &&
        nLangType != LANGUAGE_NONE &&
        nLangType != LANGUAGE_MULTIPLE &&
        nLangType != LANGUAGE_UNDETERMINED &&
        nLangType != LANGUAGE_USER_KEYID &&
        !MsLangId::isLegacy( nLangType) &&
        (!requireSublang || MsLangId::getSubLanguage( nLangType));
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


LanguageType SvxLanguageBox::get_active_id() const
{
    OUString sLang = m_xControl->get_active_id();
    if (!sLang.isEmpty())
        return LanguageType(sLang.toInt32());
    else
        return LANGUAGE_DONTKNOW;
}

int SvxLanguageBox::find_id(const LanguageType eLangType) const
{
    return m_xControl->find_id(OUString::number(static_cast<sal_uInt16>(eLangType)));
}

void SvxLanguageBox::set_id(int pos, const LanguageType eLangType)
{
    m_xControl->set_id(pos, OUString::number(static_cast<sal_uInt16>(eLangType)));
}

LanguageType SvxLanguageBox::get_id(int pos) const
{
    return LanguageType(m_xControl->get_id(pos).toInt32());
}

void SvxLanguageBox::remove_id(const LanguageType eLangType)
{
    m_xControl->remove_id(OUString::number(static_cast<sal_uInt16>(eLangType)));
}

void SvxLanguageBox::append(const LanguageType eLangType, const OUString& rStr)
{
    m_xControl->append(OUString::number(static_cast<sal_uInt16>(eLangType)), rStr);
}

void SvxLanguageBox::set_active_id(const LanguageType eLangType)
{
    // If the core uses a LangID of an imported MS document and wants to select
    // a language that is replaced, we need to select the replacement instead.
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( eLangType);

    sal_Int32 nAt = find_id( nLang );

    if (nAt == -1)
    {
        InsertLanguage( nLang );      // on-the-fly-ID
        nAt = find_id( nLang );
    }

    if (nAt != -1)
        m_xControl->set_active(nAt);
}

void SvxLanguageBox::AddLanguages(const std::vector< LanguageType >& rLanguageTypes,
        SvxLanguageListFlags nLangList, std::vector<weld::ComboBoxEntry>& rEntries, bool requireSublang)
{
    for ( auto const & nLangType : rLanguageTypes )
    {
        if (lcl_isPrerequisite(nLangType, requireSublang))
        {
            LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( nLangType );
            if (lcl_isScriptTypeRequested( nLang, nLangList))
            {
                int nAt = find_id(nLang);
                if (nAt != -1)
                    continue;
                weld::ComboBoxEntry aNewEntry(BuildEntry(nLang));
                if (aNewEntry.sString.isEmpty())
                    continue;
                rEntries.push_back(aNewEntry);
            }
        }
    }
}

static void SortLanguages(std::vector<weld::ComboBoxEntry>& rEntries)
{
    struct NaturalStringSorterCompare
    {
        bool operator()(const OUString& rLHS, const OUString& rRHS) const
        {
            static const auto aSorter = comphelper::string::NaturalStringSorter(
                comphelper::getProcessComponentContext(),
                Application::GetSettings().GetUILanguageTag().getLocale());
            return aSorter.compare(rLHS, rRHS) < 0;
        }
    };

    struct EntryData
    {
        LanguageTag tag;
        weld::ComboBoxEntry entry;
    };

    struct GenericFirst
    {
        bool operator()(const EntryData& e1, const EntryData& e2) const
        {
            assert(e1.tag.getLanguage() == e2.tag.getLanguage());
            if (e1.entry.sId == e2.entry.sId)
                return false; // shortcut

            // Make sure that e.g. generic 'Spanish {es}' goes before 'Spanish (Argentina)'.
            // We can't depend on MsLangId::getPrimaryLanguage/getSubLanguage, because e.g.
            // for generic Bosnian {bs}, the MS-LCID is 0x781A, and getSubLanguage is not 0.
            // So we have to do the expensive LanguageTag construction in EntryData.

            const bool isLangOnly1 = e1.tag.isIsoLocale() && e1.tag.getCountry().isEmpty();
            const bool isLangOnly2 = e2.tag.isIsoLocale() && e2.tag.getCountry().isEmpty();
            assert(!(isLangOnly1 && isLangOnly2));

            if (isLangOnly1)
            {
                // e1.tag is a generic language-only tag, e2.tag is not
                return true;
            }
            else if (isLangOnly2)
            {
                // e2.tag is a generic language-only tag, e1.tag is not
                return false;
            }

            // Do a normal string comparison for other cases
            return NaturalStringSorterCompare()(e1.entry.sString, e2.entry.sString);
        }
    };
    using SortedLangEntries = o3tl::sorted_vector<EntryData, GenericFirst>;

    // It is impossible to sort using only GenericFirst comparison: it would fail the strict weak
    // ordering requirement, where the following simplified example would fail the last assertion:
    //
    //  weld::ComboBoxEntry nn{ u"노르웨이어(니노르스크) {nn}"_ustr, "30740", "" }
    //  weld::ComboBoxEntry nn_NO{ u"노르웨이어 뉘노르스크>"_ustr, "2068", "" };
    //  weld::ComboBoxEntry nb_NO{ u"노르웨이어 부크몰"_ustr, "1044", "" };
    //
    //  assert(GenericFirst(nn, nn_NO));
    //  assert(GenericFirst(nn_NO, nb_NO));
    //  assert(GenericFirst(nn, nb_NO));
    //
    // So only sort this way inside language groups, where the data set itself guarantees the
    // comparison's strict weak ordering.

    // 1. Create lang-to-set-of-ComboBoxEntry map
    std::unordered_map<OUString, SortedLangEntries> langToEntriesMap;

    for (const auto& entry : rEntries)
    {
        LanguageType languageType(entry.sId.toInt32());
        // Remove LANGUAGE_USER_SYSTEM_CONFIG special entry and friends from the list
        if (languageType >= LanguageType(0xFFE0))
            continue;
        LanguageTag tag(languageType);
        langToEntriesMap[tag.getLanguage()].insert({ tag, entry }); // also makes unique
    }

    // 2. Sort using generic language's translated name, plus ISO language tag appended just in case
    std::map<OUString, const SortedLangEntries&, NaturalStringSorterCompare> finalSort;
    const LanguageTag& uiLang = Application::GetSettings().GetUILanguageTag();
    for (const auto& [lang, lang_entries] : langToEntriesMap)
    {
        OUString translatedLangName = LanguageTagIcu::getDisplayName(LanguageTag(lang), uiLang);
        finalSort.emplace(translatedLangName + "_" + lang, lang_entries);
    }

    rEntries.clear();
    for ([[maybe_unused]] const auto& [lang, lang_entries] : finalSort)
        for (auto& entryData : lang_entries)
            rEntries.push_back(entryData.entry);
}

void SvxLanguageBox::SetLanguageList(SvxLanguageListFlags nLangList, bool bHasLangNone,
                                     bool bLangNoneIsLangAll, bool bCheckSpellAvail,
                                     bool bDefaultLangExist, LanguageType eDefaultLangType,
                                     sal_Int16 nDefaultType)
{
    m_bHasLangNone          = bHasLangNone;
    m_bLangNoneIsLangAll    = bLangNoneIsLangAll;
    m_bWithCheckmark        = bCheckSpellAvail;

    m_xControl->freeze();
    comphelper::ScopeGuard aThawGuard([this]() { m_xControl->thaw(); });
    m_xControl->clear();

    if (SvxLanguageListFlags::EMPTY == nLangList)
        return;

    bool bAddSeparator = false;

    if (bHasLangNone)
    {
        m_xControl->append(BuildEntry(LANGUAGE_NONE));
        m_xControl->append(BuildEntry(LANGUAGE_MULTIPLE));
        m_xControl->append(BuildEntry(LANGUAGE_UNDETERMINED));
        bAddSeparator = true;
    }

    if (bDefaultLangExist)
    {
        m_xControl->append(BuildEntry(eDefaultLangType, nDefaultType));
        bAddSeparator = true;
    }

    if (bAddSeparator)
        m_xControl->append_separator(u""_ustr);

    bool bAddAvailable = (!(nLangList & SvxLanguageListFlags::ONLY_KNOWN) &&
            ((nLangList & SvxLanguageListFlags::ALL) ||
             (nLangList & SvxLanguageListFlags::WESTERN) ||
             (nLangList & SvxLanguageListFlags::CTL) ||
             (nLangList & SvxLanguageListFlags::CJK)));
    std::vector< LanguageType > aAvailLang;
    Sequence< sal_Int16 > aSpellUsedLang;
    if (bAddAvailable)
    {
        if (auto xAvail = LinguMgr::GetLngSvcMgr())
        {
            appendLocaleSeqToLangs(xAvail->getAvailableLocales(SN_SPELLCHECKER), aAvailLang);
            appendLocaleSeqToLangs(xAvail->getAvailableLocales(SN_HYPHENATOR), aAvailLang);
            appendLocaleSeqToLangs(xAvail->getAvailableLocales(SN_THESAURUS), aAvailLang);
        }
    }
    if (SvxLanguageListFlags::SPELL_USED & nLangList)
    {
        Reference< XSpellChecker1 > xTmp1 = LinguMgr::GetSpellChecker();
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
        if ( lcl_isPrerequisite( nLangType, true ) &&
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
        AddLanguages(aAvailLang, nLangList, aEntries, true);
    }

    SortLanguages(aEntries);
    m_xControl->insert_vector(aEntries, true);
}

void SvxLanguageBox::InsertLanguage(const LanguageType nLangType)
{
    if (find_id(nLangType) != -1)
        return;
    weld::ComboBoxEntry aEntry = BuildEntry(nLangType);
    if (aEntry.sString.isEmpty())
        return;
    m_xControl->append(aEntry);
}

void SvxLanguageBox::InsertLanguages(const std::vector<LanguageType>& rLanguageTypes)
{
    std::vector<weld::ComboBoxEntry> entries;
    AddLanguages(rLanguageTypes, SvxLanguageListFlags::ALL, entries, false);
    SortLanguages(entries);
    m_xControl->insert_vector(entries, true);
}

weld::ComboBoxEntry SvxLanguageBox::BuildEntry(const LanguageType nLangType, sal_Int16 nType)
{
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage(nLangType);
    // For obsolete and to be replaced languages check whether an entry of the
    // replacement already exists and if so don't add an entry with identical
    // string as would be returned by SvtLanguageTable::GetString().
    if (nLang != nLangType)
    {
        int nAt = find_id( nLang );
        if (nAt != -1)
            return weld::ComboBoxEntry(u""_ustr);
    }

    OUString aStrEntry = (LANGUAGE_NONE == nLang && m_bHasLangNone && m_bLangNoneIsLangAll)
                             ? SvxResId(RID_SVXSTR_LANGUAGE_ALL)
                             : SvtLanguageTable::GetLanguageString(nLang);

    LanguageType nRealLang = nLang;
    if (nRealLang == LANGUAGE_SYSTEM)
    {
        nRealLang = MsLangId::resolveSystemLanguageByScriptType(nRealLang, nType);
        aStrEntry += " - " + SvtLanguageTable::GetLanguageString( nRealLang );
    }
    else if (nRealLang == LANGUAGE_USER_SYSTEM_CONFIG)
    {
        nRealLang = MsLangId::getSystemLanguage();
        // Whatever we obtained, ensure a known supported locale.
        nRealLang = LanguageTag(nRealLang).makeFallback().getLanguageType();
        aStrEntry += " - " + SvtLanguageTable::GetLanguageString( nRealLang );
    }

    if (m_bWithCheckmark)
    {
        if (!m_xSpellUsedLang)
        {
            Reference<XSpellChecker1> xSpell = LinguMgr::GetSpellChecker();
            if (xSpell.is())
                m_xSpellUsedLang.reset(new Sequence<sal_Int16>(xSpell->getLanguages()));
        }

        bool bFound = m_xSpellUsedLang && lcl_SeqHasLang(*m_xSpellUsedLang, static_cast<sal_uInt16>(nRealLang));

        return weld::ComboBoxEntry(aStrEntry, OUString::number(static_cast<sal_uInt16>(nLang)), bFound ? RID_SVXBMP_CHECKED : RID_SVXBMP_NOTCHECKED);
    }
    else
        return weld::ComboBoxEntry(aStrEntry, OUString::number(static_cast<sal_uInt16>(nLang)));
}

IMPL_LINK(SvxLanguageBox, ChangeHdl, weld::ComboBox&, rControl, void)
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
                bool bValid = LanguageTag::isValidBcp47( aStr, &aCanonicalized, LanguageTag::PrivateUse::ALLOW_ART_X);
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
                rControl.set_entry_message_type(weld::EntryMessageType::Error);
            else
                rControl.set_entry_message_type(weld::EntryMessageType::Normal);
        }
    }
    m_aChangeHdl.Call(rControl);
}

SvxLanguageBox::SvxLanguageBox(std::unique_ptr<weld::ComboBox> pControl)
    : m_xControl(std::move(pControl))
    , m_eSavedLanguage(LANGUAGE_DONTKNOW)
    , m_eEditedAndValid(EditedAndValid::No)
    , m_bHasLangNone(false)
    , m_bLangNoneIsLangAll(false)
    , m_bWithCheckmark(false)
{
    m_xControl->connect_changed(LINK(this, SvxLanguageBox, ChangeHdl));
}

SvxLanguageBox* SvxLanguageBox::SaveEditedAsEntry(SvxLanguageBox* ppBoxes[3])
{
    if (m_eEditedAndValid != EditedAndValid::Valid)
        return this;

    LanguageTag aLanguageTag(m_xControl->get_active_text());
    LanguageType nLang = aLanguageTag.getLanguageType();
    if (nLang == LANGUAGE_DONTKNOW)
    {
        SAL_WARN( "svx.dialog", "SvxLanguageBox::SaveEditedAsEntry: unknown tag");
        return this;
    }

    for (size_t i = 0; i < 3; ++i)
    {
        SvxLanguageBox* pBox = ppBoxes[i];
        if (!pBox)
            continue;

        const int nPos = pBox->find_id( nLang);
        if (nPos != -1)
        {
            // Already present but with a different string or in another list.
            pBox->m_xControl->set_active(nPos);
            return pBox;
        }
    }

    if (SvtLanguageTable::HasLanguageType( nLang))
    {
        // In SvtLanguageTable but not in SvxLanguageBox. On purpose? This
        // may be an entry with different settings.
        SAL_WARN( "svx.dialog", "SvxLanguageBox::SaveEditedAsEntry: already in SvtLanguageTable: " <<
                SvtLanguageTable::GetLanguageString( nLang) << ", " << nLang);
    }
    else
    {
        // Add to SvtLanguageTable first. This at an on-the-fly LanguageTag
        // also sets the ScriptType needed below.
        SvtLanguageTable::AddLanguageTag( aLanguageTag );
    }

    // Add to the proper list.
    SvxLanguageBox* pBox = nullptr;
    switch (MsLangId::getScriptType(nLang))
    {
        default:
        case css::i18n::ScriptType::LATIN:
            pBox = ppBoxes[0];
        break;
        case css::i18n::ScriptType::ASIAN:
            pBox = ppBoxes[1];
        break;
        case css::i18n::ScriptType::COMPLEX:
            pBox = ppBoxes[2];
        break;
    }
    if (!pBox)
        pBox = this;
    pBox->InsertLanguage(nLang);

    // Select it.
    const int nPos = pBox->find_id(nLang);
    if (nPos != -1)
        pBox->m_xControl->set_active(nPos);

    return pBox;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
