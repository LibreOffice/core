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

#include <utility>
#include <vcl/settings.hxx>
#include <vcl/weld.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <o3tl/safeint.hxx>
#include <officecfg/Office/Security.hxx>
#include <officecfg/Office/Linguistic.hxx>
#include <unotools/lingucfg.hxx>
#include <unotools/linguprops.hxx>
#include <editeng/unolingu.hxx>
#include <linguistic/misc.hxx>
#include <sfx2/sfxsids.hrc>
#include <tools/debug.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/dispatchcommand.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#include <com/sun/star/linguistic2/XLinguProperties.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <unotools/extendedsecurityoptions.hxx>
#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <svx/svxdlg.hxx>
#include <editeng/optitems.hxx>
#include <optlingu.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>

#include <ucbhelper/content.hxx>

#include <set>
#include <vector>
#include <map>

using namespace ::ucbhelper;
using namespace ::com::sun::star;
using namespace css::lang;
using namespace css::uno;
using namespace css::linguistic2;
using namespace css::beans;

constexpr OUString cSpell(SN_SPELLCHECKER);
constexpr OUString cGrammar(SN_GRAMMARCHECKER);
constexpr OUString cHyph(SN_HYPHENATOR);
constexpr OUString cThes(SN_THESAURUS);

// static ----------------------------------------------------------------

static sal_Int32 lcl_SeqGetEntryPos(
    const Sequence< OUString > &rSeq, std::u16string_view rEntry )
{
    auto it = std::find(rSeq.begin(), rSeq.end(), rEntry);
    return it == rSeq.end() ? -1 : std::distance(rSeq.begin(), it);
}

static bool KillFile_Impl( const OUString& rURL )
{
    bool bRet = true;
    try
    {
        Content aCnt( rURL, uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        aCnt.executeCommand( u"delete"_ustr, Any( true ) );
    }
    catch( ... )
    {
        TOOLS_WARN_EXCEPTION( "cui.options", "KillFile" );
        bRet = false;
    }

    return bRet;
}

// 0x 0p 0t 0c nn
// p: 1 -> parent
// t: 1 -> spell, 2 -> hyph, 3 -> thes, 4 -> grammar
// c: 1 -> checked 0 -> unchecked
// n: index

#define TYPE_SPELL      sal_uInt8(1)
#define TYPE_GRAMMAR    sal_uInt8(2)
#define TYPE_HYPH       sal_uInt8(3)
#define TYPE_THES       sal_uInt8(4)

namespace {

class ModuleUserData_Impl
{
    bool bParent;
    bool bIsChecked;
    sal_uInt8 nType;
    sal_uInt8 nIndex;
    OUString  sImplName;

public:
    ModuleUserData_Impl( OUString sImpName, bool bIsParent, bool bChecked, sal_uInt8 nSetType, sal_uInt8 nSetIndex ) :
        bParent(bIsParent),
        bIsChecked(bChecked),
        nType(nSetType),
        nIndex(nSetIndex),
        sImplName(std::move(sImpName))
        {
        }
    bool IsParent() const {return bParent;}
    sal_uInt8 GetType() const {return nType;}
    bool IsChecked() const {return bIsChecked;}
    sal_uInt8 GetIndex() const {return nIndex;}
    const OUString& GetImplName() const {return sImplName;}

};


// User for user-dictionaries (XDictionary interface)

class DicUserData
{
    sal_uInt32 nVal;

public:
    explicit DicUserData(sal_uInt32 nUserData) : nVal( nUserData ) {}
    DicUserData( sal_uInt16 nEID,
                 bool bChecked, bool bEditable, bool bDeletable );

    sal_uInt32  GetUserData() const         { return nVal; }
    sal_uInt16  GetEntryId() const          { return static_cast<sal_uInt16>(nVal >> 16); }
    bool        IsChecked() const           { return static_cast<bool>((nVal >>  8) & 0x01); }
    bool        IsDeletable() const         { return static_cast<bool>((nVal >> 10) & 0x01); }
};

}

DicUserData::DicUserData(
        sal_uInt16 nEID,
        bool bChecked, bool bEditable, bool bDeletable )
{
    DBG_ASSERT( nEID < 65000, "Entry Id out of range" );
    nVal =  (static_cast<sal_uInt32>(0xFFFF & nEID)         << 16) |
            (static_cast<sal_uInt32>(bChecked ? 1 : 0)      <<  8) |
            (static_cast<sal_uInt32>(bEditable ? 1 : 0)     <<  9) |
            (static_cast<sal_uInt32>(bDeletable ? 1 : 0)    << 10);
}

/*--------------------------------------------------
    Entry IDs for options listbox of dialog
--------------------------------------------------*/

namespace {

enum EID_OPTIONS
{
    EID_SPELL_AUTO,
    EID_GRAMMAR_AUTO,
    EID_CAPITAL_WORDS,
    EID_WORDS_WITH_DIGITS,
    EID_SPELL_SPECIAL,
    EID_NUM_MIN_WORDLEN,
    EID_NUM_PRE_BREAK,
    EID_NUM_POST_BREAK,
    EID_HYPH_AUTO,
    EID_HYPH_SPECIAL,
    EID_SPELL_CLOSED_COMPOUND,
    EID_SPELL_HYPHENATED_COMPOUND
};

}

static OUString lcl_GetPropertyName( EID_OPTIONS eEntryId )
{
    switch (eEntryId)
    {
        case EID_SPELL_AUTO: return UPN_IS_SPELL_AUTO;
        case EID_GRAMMAR_AUTO: return UPN_IS_GRAMMAR_AUTO;
        case EID_CAPITAL_WORDS: return UPN_IS_SPELL_UPPER_CASE;
        case EID_SPELL_CLOSED_COMPOUND: return UPN_IS_SPELL_CLOSED_COMPOUND;
        case EID_SPELL_HYPHENATED_COMPOUND: return UPN_IS_SPELL_HYPHENATED_COMPOUND;
        case EID_WORDS_WITH_DIGITS: return UPN_IS_SPELL_WITH_DIGITS;
        case EID_SPELL_SPECIAL: return UPN_IS_SPELL_SPECIAL;
        case EID_NUM_MIN_WORDLEN: return UPN_HYPH_MIN_WORD_LENGTH;
        case EID_NUM_PRE_BREAK: return UPN_HYPH_MIN_LEADING;
        case EID_NUM_POST_BREAK: return UPN_HYPH_MIN_TRAILING;
        case EID_HYPH_AUTO: return UPN_IS_HYPH_AUTO;
        case EID_HYPH_SPECIAL: return UPN_IS_HYPH_SPECIAL;
        default: assert (false); abort();
    }
}

namespace {

class OptionsBreakSet : public weld::GenericDialogController
{
    std::unique_ptr<weld::Widget> m_xBeforeFrame;
    std::unique_ptr<weld::Widget> m_xAfterFrame;
    std::unique_ptr<weld::Widget> m_xMinimalFrame;
    std::unique_ptr<weld::SpinButton> m_xBreakNF;

public:
    OptionsBreakSet(weld::Window* pParent, sal_uInt16 nRID)
        : GenericDialogController(pParent, u"cui/ui/breaknumberoption.ui"_ustr, u"BreakNumberOption"_ustr)
        , m_xBeforeFrame(m_xBuilder->weld_widget(u"beforeframe"_ustr))
        , m_xAfterFrame(m_xBuilder->weld_widget(u"afterframe"_ustr))
        , m_xMinimalFrame(m_xBuilder->weld_widget(u"miniframe"_ustr))
    {
        assert(EID_NUM_PRE_BREAK == nRID || EID_NUM_POST_BREAK == nRID || EID_NUM_MIN_WORDLEN == nRID); //unexpected ID

        if (nRID == EID_NUM_PRE_BREAK)
        {
            m_xBeforeFrame->show();
            m_xBreakNF = m_xBuilder->weld_spin_button(u"beforebreak"_ustr);
        }
        else if(nRID == EID_NUM_POST_BREAK)
        {
            m_xAfterFrame->show();
            m_xBreakNF = m_xBuilder->weld_spin_button(u"afterbreak"_ustr);
        }
        else if(nRID == EID_NUM_MIN_WORDLEN)
        {
            m_xMinimalFrame->show();
            m_xBreakNF = m_xBuilder->weld_spin_button(u"wordlength"_ustr);
        }
    }

    weld::SpinButton& GetNumericFld()
    {
        return *m_xBreakNF;
    }
};

// class OptionsUserData -------------------------------------------------

class OptionsUserData
{
    sal_uInt32   nVal;

public:
    explicit OptionsUserData( sal_uInt32 nUserData ) : nVal( nUserData ) {}
    OptionsUserData( sal_uInt16 nEID,
                     bool bHasNV, sal_uInt16 nNumVal,
                     bool bCheckable, bool bChecked );

    sal_uInt32  GetUserData() const         { return nVal; }
    sal_uInt16  GetEntryId() const          { return static_cast<sal_uInt16>(nVal >> 16); }
    bool        HasNumericValue() const     { return static_cast<bool>((nVal >> 10) & 0x01); }
    sal_uInt16  GetNumericValue() const     { return static_cast<sal_uInt16>(nVal & 0xFF); }
    bool        IsCheckable() const         { return static_cast<bool>((nVal >> 9) & 0x01); }
    bool        IsModified() const          { return static_cast<bool>((nVal >> 11) & 0x01); }

    void        SetNumericValue( sal_uInt8 nNumVal );
};

}

OptionsUserData::OptionsUserData( sal_uInt16 nEID,
        bool bHasNV, sal_uInt16 nNumVal,
        bool bCheckable, bool bChecked )
{
    DBG_ASSERT( nEID < 65000, "Entry Id out of range" );
    DBG_ASSERT( nNumVal < 256, "value out of range" );
    nVal =  (static_cast<sal_uInt32>(0xFFFF & nEID)         << 16) |
            (static_cast<sal_uInt32>(bHasNV ? 1 : 0)        << 10) |
            (static_cast<sal_uInt32>(bCheckable ? 1 : 0)    <<  9) |
            (static_cast<sal_uInt32>(bChecked ? 1 : 0)      <<  8) |
            static_cast<sal_uInt32>(0xFF & nNumVal);
}

void OptionsUserData::SetNumericValue( sal_uInt8 nNumVal )
{
    if (HasNumericValue()  &&  (GetNumericValue() != nNumVal))
    {
        nVal &= 0xffffff00;
        nVal |= nNumVal;
        nVal |= sal_uInt32(1) << 11; // mark as modified
    }
}

// ServiceInfo_Impl ----------------------------------------------------

namespace {

struct ServiceInfo_Impl
{
    OUString                    sDisplayName;
    OUString                    sSpellImplName;
    OUString                    sHyphImplName;
    OUString                    sThesImplName;
    OUString                    sGrammarImplName;
    uno::Reference< XSpellChecker >     xSpell;
    uno::Reference< XHyphenator >       xHyph;
    uno::Reference< XThesaurus >        xThes;
    uno::Reference< XProofreader >      xGrammar;
    bool                        bConfigured;

    ServiceInfo_Impl() : bConfigured(false) {}
};

struct Locale_less
{
    bool operator()(const css::lang::Locale& lhs, const css::lang::Locale& rhs) const
    {
        if (lhs.Language < rhs.Language)
            return true;
        if (lhs.Language > rhs.Language)
            return false;
        if (lhs.Country < rhs.Country)
            return true;
        if (lhs.Country > rhs.Country)
            return false;
        return lhs.Variant < rhs.Variant;
    }
};

}

typedef std::vector< ServiceInfo_Impl >                   ServiceInfoArr;
typedef std::map< LanguageType, Sequence< OUString > >    LangImplNameTable;


// SvxLinguData_Impl ----------------------------------------------------

class SvxLinguData_Impl
{
    //contains services and implementation names sorted by implementation names
    ServiceInfoArr                      aDisplayServiceArr;
    sal_uInt32                          nDisplayServices;

    std::set<Locale, Locale_less>       aAllServiceLocales;
    LangImplNameTable                   aCfgSpellTable;
    LangImplNameTable                   aCfgHyphTable;
    LangImplNameTable                   aCfgThesTable;
    LangImplNameTable                   aCfgGrammarTable;
    uno::Reference< XLinguServiceManager2 >  xLinguSrvcMgr;


    static bool AddRemove( Sequence< OUString > &rConfigured,
                           const OUString &rImplName, bool bAdd );

public:
    SvxLinguData_Impl();

    uno::Reference<XLinguServiceManager2> &   GetManager() { return xLinguSrvcMgr; }

    void SetChecked( const Sequence< OUString > &rConfiguredServices );
    void Reconfigure( std::u16string_view rDisplayName, bool bEnable );

    const auto&                 GetAllSupportedLocales() const { return aAllServiceLocales; }

    LangImplNameTable &         GetSpellTable()         { return aCfgSpellTable; }
    LangImplNameTable &         GetHyphTable()          { return aCfgHyphTable; }
    LangImplNameTable &         GetThesTable()          { return aCfgThesTable; }
    LangImplNameTable &         GetGrammarTable()       { return aCfgGrammarTable; }

    ServiceInfoArr &            GetDisplayServiceArray()        { return aDisplayServiceArr; }

    const sal_uInt32 &   GetDisplayServiceCount() const          { return nDisplayServices; }
    void            SetDisplayServiceCount( sal_uInt32 nVal )    { nDisplayServices = nVal; }

    // returns the list of service implementation names for the specified
    // language and service (TYPE_SPELL, TYPE_HYPH, TYPE_THES) sorted in
    // the proper order for the SvxEditModulesDlg (the ones from the
    // configuration (keeping that order!) first and then the other ones.
    // I.e. the ones available but not configured in arbitrary order).
    // They available ones may contain names that do not(!) support that
    // language.
    Sequence< OUString > GetSortedImplNames( LanguageType nLang, sal_uInt8 nType );

    ServiceInfo_Impl * GetInfoByImplName( std::u16string_view rSvcImplName );
};


Sequence< OUString > SvxLinguData_Impl::GetSortedImplNames( LanguageType nLang, sal_uInt8 nType )
{
    LangImplNameTable *pTable = nullptr;
    switch (nType)
    {
        case TYPE_SPELL     : pTable = &aCfgSpellTable; break;
        case TYPE_HYPH      : pTable = &aCfgHyphTable; break;
        case TYPE_THES      : pTable = &aCfgThesTable; break;
        case TYPE_GRAMMAR   : pTable = &aCfgGrammarTable; break;
    }
    Sequence< OUString > aRes;
    if (!pTable)
    {
        SAL_WARN( "cui.options", "unknown linguistic type" );
        return aRes;
    }
    if (pTable->count( nLang ))
        aRes = (*pTable)[ nLang ];      // add configured services
    sal_Int32 nIdx = aRes.getLength();
    DBG_ASSERT( nDisplayServices >= o3tl::make_unsigned(nIdx), "size mismatch" );
    aRes.realloc( nDisplayServices );
    OUString *pRes = aRes.getArray();

    // add not configured services
    for (sal_uInt32 i = 0;  i < nDisplayServices;  ++i)
    {
        const ServiceInfo_Impl &rInfo = aDisplayServiceArr[ i ];
        OUString aImplName;
        switch (nType)
        {
            case TYPE_SPELL     : aImplName = rInfo.sSpellImplName; break;
            case TYPE_HYPH      : aImplName = rInfo.sHyphImplName; break;
            case TYPE_THES      : aImplName = rInfo.sThesImplName; break;
            case TYPE_GRAMMAR   : aImplName = rInfo.sGrammarImplName; break;
        }

        if (!aImplName.isEmpty()  &&  (lcl_SeqGetEntryPos( aRes, aImplName) == -1))    // name not yet added
        {
            DBG_ASSERT( nIdx < aRes.getLength(), "index out of range" );
            if (nIdx < aRes.getLength())
                pRes[ nIdx++ ] = aImplName;
        }
    }
    // don't forget to put aRes back to its actual size just in case you allocated too much
    // since all of the names may have already been added
    // otherwise you get duplicate entries in the edit dialog
    aRes.realloc( nIdx );
    return aRes;
}


ServiceInfo_Impl * SvxLinguData_Impl::GetInfoByImplName( std::u16string_view rSvcImplName )
{
    for (sal_uInt32 i = 0;  i < nDisplayServices;  ++i)
    {
        ServiceInfo_Impl &rTmp = aDisplayServiceArr[ i ];
        if (rTmp.sSpellImplName == rSvcImplName ||
            rTmp.sHyphImplName  == rSvcImplName ||
            rTmp.sThesImplName  == rSvcImplName ||
            rTmp.sGrammarImplName == rSvcImplName)
        {
            return &rTmp;
        }
    }
    return nullptr;
}

static void lcl_MergeDisplayArray(
        SvxLinguData_Impl &rData,
        const ServiceInfo_Impl &rToAdd )
{
    sal_uInt32 nCnt = 0;

    ServiceInfoArr &rSvcInfoArr = rData.GetDisplayServiceArray();
    sal_uInt32 nEntries = rData.GetDisplayServiceCount();

    for (sal_uInt32 i = 0;  i < nEntries;  ++i)
    {
        ServiceInfo_Impl& rEntry = rSvcInfoArr[i];
        if (rEntry.sDisplayName == rToAdd.sDisplayName)
        {
            if(rToAdd.xSpell.is())
            {
                DBG_ASSERT( !rEntry.xSpell.is() &&
                            rEntry.sSpellImplName.isEmpty(),
                            "merge conflict" );
                rEntry.sSpellImplName = rToAdd.sSpellImplName;
                rEntry.xSpell = rToAdd.xSpell;
            }
            if(rToAdd.xGrammar.is())
            {
                DBG_ASSERT( !rEntry.xGrammar.is() &&
                            rEntry.sGrammarImplName.isEmpty(),
                            "merge conflict" );
                rEntry.sGrammarImplName = rToAdd.sGrammarImplName;
                rEntry.xGrammar = rToAdd.xGrammar;
            }
            if(rToAdd.xHyph.is())
            {
                DBG_ASSERT( !rEntry.xHyph.is() &&
                            rEntry.sHyphImplName.isEmpty(),
                            "merge conflict" );
                rEntry.sHyphImplName = rToAdd.sHyphImplName;
                rEntry.xHyph = rToAdd.xHyph;
            }
            if(rToAdd.xThes.is())
            {
                DBG_ASSERT( !rEntry.xThes.is() &&
                            rEntry.sThesImplName.isEmpty(),
                            "merge conflict" );
                rEntry.sThesImplName = rToAdd.sThesImplName;
                rEntry.xThes = rToAdd.xThes;
            }
            return ;
        }
        ++nCnt;
    }
    rData.GetDisplayServiceArray().push_back( rToAdd );
    rData.SetDisplayServiceCount( nCnt + 1 );
}

SvxLinguData_Impl::SvxLinguData_Impl() :
    nDisplayServices    (0)
{
    uno::Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    xLinguSrvcMgr = LinguServiceManager::create(xContext);

    const Locale& rCurrentLocale = Application::GetSettings().GetUILanguageTag().getLocale();
    Sequence<Any> aArgs
    {
        Any(LinguMgr::GetLinguPropertySet()),
        Any() // second argument has to be empty!
    };

    //read spell checker
    const Sequence< OUString > aSpellNames = xLinguSrvcMgr->getAvailableServices(
                    cSpell,    Locale() );

    for(const OUString& spellName : aSpellNames)
    {
        ServiceInfo_Impl aInfo;
        aInfo.sSpellImplName = spellName;
        aInfo.xSpell.set(
                        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aInfo.sSpellImplName, aArgs, xContext), UNO_QUERY);

        uno::Reference<XServiceDisplayName> xDispName(aInfo.xSpell, UNO_QUERY);
        if(xDispName.is())
            aInfo.sDisplayName = xDispName->getServiceDisplayName( rCurrentLocale );

        const Sequence< Locale > aLocales( aInfo.xSpell->getLocales() );
        //! suppress display of entries with no supported languages (see feature 110994)
        if (aLocales.hasElements())
        {
            aAllServiceLocales.insert(aLocales.begin(), aLocales.end());
            lcl_MergeDisplayArray( *this, aInfo );
        }
    }

    //read grammar checker
    const Sequence< OUString > aGrammarNames = xLinguSrvcMgr->getAvailableServices(
                    cGrammar, Locale() );
    for(const OUString& grammarName : aGrammarNames)
    {
        ServiceInfo_Impl aInfo;
        aInfo.sGrammarImplName = grammarName;
        aInfo.xGrammar.set(
                        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aInfo.sGrammarImplName, aArgs, xContext), UNO_QUERY);

        uno::Reference<XServiceDisplayName> xDispName(aInfo.xGrammar, UNO_QUERY);
        if(xDispName.is())
            aInfo.sDisplayName = xDispName->getServiceDisplayName( rCurrentLocale );

        const Sequence< Locale > aLocales( aInfo.xGrammar->getLocales() );
        //! suppress display of entries with no supported languages (see feature 110994)
        if (aLocales.hasElements())
        {
            aAllServiceLocales.insert(aLocales.begin(), aLocales.end());
            lcl_MergeDisplayArray( *this, aInfo );
        }
    }

    //read hyphenator
    const Sequence< OUString > aHyphNames = xLinguSrvcMgr->getAvailableServices(
                    cHyph, Locale() );
    for(const OUString& hyphName : aHyphNames)
    {
        ServiceInfo_Impl aInfo;
        aInfo.sHyphImplName = hyphName;
        aInfo.xHyph.set( xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aInfo.sHyphImplName, aArgs, xContext), UNO_QUERY);

        uno::Reference<XServiceDisplayName> xDispName(aInfo.xHyph, UNO_QUERY);
        if(xDispName.is())
            aInfo.sDisplayName = xDispName->getServiceDisplayName( rCurrentLocale );

        const Sequence< Locale > aLocales( aInfo.xHyph->getLocales() );
        //! suppress display of entries with no supported languages (see feature 110994)
        if (aLocales.hasElements())
        {
            aAllServiceLocales.insert(aLocales.begin(), aLocales.end());
            lcl_MergeDisplayArray( *this, aInfo );
        }
    }

    //read thesauri
    const Sequence< OUString > aThesNames = xLinguSrvcMgr->getAvailableServices(
                    cThes,     Locale() );
    for(const OUString& thesName : aThesNames)
    {
        ServiceInfo_Impl aInfo;
        aInfo.sThesImplName = thesName;
        aInfo.xThes.set( xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aInfo.sThesImplName, aArgs, xContext), UNO_QUERY);

        uno::Reference<XServiceDisplayName> xDispName(aInfo.xThes, UNO_QUERY);
        if(xDispName.is())
            aInfo.sDisplayName = xDispName->getServiceDisplayName( rCurrentLocale );

        const Sequence< Locale > aLocales( aInfo.xThes->getLocales() );
        //! suppress display of entries with no supported languages (see feature 110994)
        if (aLocales.hasElements())
        {
            aAllServiceLocales.insert(aLocales.begin(), aLocales.end());
            lcl_MergeDisplayArray( *this, aInfo );
        }
    }

    Sequence< OUString > aCfgSvcs;
    for (auto const& locale : aAllServiceLocales)
    {
        LanguageType nLang = LanguageTag::convertToLanguageType( locale );

        aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(cSpell, locale);
        SetChecked( aCfgSvcs );
        if (aCfgSvcs.hasElements())
            aCfgSpellTable[ nLang ] = aCfgSvcs;

        aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(cGrammar, locale);
        SetChecked( aCfgSvcs );
        if (aCfgSvcs.hasElements())
            aCfgGrammarTable[ nLang ] = aCfgSvcs;

        aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(cHyph, locale);
        SetChecked( aCfgSvcs );
        if (aCfgSvcs.hasElements())
            aCfgHyphTable[ nLang ] = aCfgSvcs;

        aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(cThes, locale);
        SetChecked( aCfgSvcs );
        if (aCfgSvcs.hasElements())
            aCfgThesTable[ nLang ] = aCfgSvcs;
    }
}

void SvxLinguData_Impl::SetChecked(const Sequence<OUString>& rConfiguredServices)
{
    for(OUString const & configService : rConfiguredServices)
    {
        for (sal_uInt32 i = 0;  i < nDisplayServices;  ++i)
        {
            ServiceInfo_Impl& rEntry = aDisplayServiceArr[i];
            if (!rEntry.bConfigured)
            {
                const OUString &rSrvcImplName = configService;
                if (!rSrvcImplName.isEmpty()  &&
                    (rEntry.sSpellImplName == rSrvcImplName ||
                        rEntry.sGrammarImplName == rSrvcImplName ||
                        rEntry.sHyphImplName == rSrvcImplName ||
                        rEntry.sThesImplName == rSrvcImplName))
                {
                    rEntry.bConfigured = true;
                    break;
                }
            }
        }
    }
}

bool SvxLinguData_Impl::AddRemove(
            Sequence< OUString > &rConfigured,
            const OUString &rImplName, bool bAdd )
{
    bool bRet = false;  // modified?

    sal_Int32 nEntries = rConfigured.getLength();
    sal_Int32 nPos = lcl_SeqGetEntryPos(rConfigured, rImplName);
    if (bAdd  &&  nPos < 0)         // add new entry
    {
        rConfigured.realloc( ++nEntries );
        OUString *pConfigured = rConfigured.getArray();
        pConfigured[nEntries - 1] = rImplName;
        bRet = true;
    }
    else if (!bAdd  &&  nPos >= 0)  // remove existing entry
    {
        OUString *pConfigured = rConfigured.getArray();
        for (sal_Int32 i = nPos;  i < nEntries - 1;  ++i)
            pConfigured[i] = pConfigured[i + 1];
        rConfigured.realloc(--nEntries);
        bRet = true;
    }

    return bRet;
}


void SvxLinguData_Impl::Reconfigure( std::u16string_view rDisplayName, bool bEnable )
{
    DBG_ASSERT( !rDisplayName.empty(), "empty DisplayName" );

    ServiceInfo_Impl *pInfo = nullptr;
    for (sal_uInt32 i = 0;  i < nDisplayServices;  ++i)
    {
        ServiceInfo_Impl& rTmp = aDisplayServiceArr[i];
        if (rTmp.sDisplayName == rDisplayName)
        {
            pInfo = &rTmp;
            break;
        }
    }
    DBG_ASSERT( pInfo, "DisplayName entry not found" );
    if (!pInfo)
        return;

    pInfo->bConfigured = bEnable;

    // update configured spellchecker entries
    if (pInfo->xSpell.is())
    {
        for (auto& locale : pInfo->xSpell->getLocales())
        {
            LanguageType nLang = LanguageTag::convertToLanguageType(locale);
            if (!aCfgSpellTable.count( nLang ) && bEnable)
                aCfgSpellTable[ nLang ] = Sequence< OUString >();
            if (aCfgSpellTable.count( nLang ))
                AddRemove( aCfgSpellTable[ nLang ], pInfo->sSpellImplName, bEnable );
        }
    }

    // update configured grammar checker entries
    if (pInfo->xGrammar.is())
    {
        for (auto& locale : pInfo->xGrammar->getLocales())
        {
            LanguageType nLang = LanguageTag::convertToLanguageType(locale);
            if (!aCfgGrammarTable.count( nLang ) && bEnable)
                aCfgGrammarTable[ nLang ] = Sequence< OUString >();
            if (aCfgGrammarTable.count( nLang ))
                AddRemove( aCfgGrammarTable[ nLang ], pInfo->sGrammarImplName, bEnable );
        }
    }

    // update configured hyphenator entries
    if (pInfo->xHyph.is())
    {
        for (auto& locale : pInfo->xHyph->getLocales())
        {
            LanguageType nLang = LanguageTag::convertToLanguageType(locale);
            if (!aCfgHyphTable.count( nLang ) && bEnable)
                aCfgHyphTable[ nLang ] = Sequence< OUString >();
            if (aCfgHyphTable.count( nLang ))
                AddRemove( aCfgHyphTable[ nLang ], pInfo->sHyphImplName, bEnable );
        }
    }

    // update configured spellchecker entries
    if (!pInfo->xThes.is())
        return;

    for (auto& locale : pInfo->xThes->getLocales())
    {
        LanguageType nLang = LanguageTag::convertToLanguageType(locale);
        if (!aCfgThesTable.count( nLang ) && bEnable)
            aCfgThesTable[ nLang ] = Sequence< OUString >();
        if (aCfgThesTable.count( nLang ))
            AddRemove( aCfgThesTable[ nLang ], pInfo->sThesImplName, bEnable );
    }
}


// class SvxLinguTabPage -------------------------------------------------

SvxLinguTabPage::SvxLinguTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, u"cui/ui/optlingupage.ui"_ustr, u"OptLinguPage"_ustr, &rSet)
    , sCapitalWords   (CuiResId(RID_CUISTR_CAPITAL_WORDS))
    , sWordsWithDigits(CuiResId(RID_CUISTR_WORDS_WITH_DIGITS))
    , sSpellSpecial   (CuiResId(RID_CUISTR_SPELL_SPECIAL))
    , sSpellAuto      (CuiResId(RID_CUISTR_SPELL_AUTO))
    , sSpellClosedCompound (CuiResId(RID_CUISTR_SPELL_CLOSED_COMPOUND))
    , sSpellHyphenatedCompound (CuiResId(RID_CUISTR_SPELL_HYPHENATED_COMPOUND))
    , sGrammarAuto    (CuiResId(RID_CUISTR_GRAMMAR_AUTO))
    , sNumMinWordlen  (CuiResId(RID_CUISTR_NUM_MIN_WORDLEN))
    , sNumPreBreak    (CuiResId(RID_CUISTR_NUM_PRE_BREAK))
    , sNumPostBreak   (CuiResId(RID_CUISTR_NUM_POST_BREAK))
    , sHyphAuto       (CuiResId(RID_CUISTR_HYPH_AUTO))
    , sHyphSpecial    (CuiResId(RID_CUISTR_HYPH_SPECIAL))
    , nUPN_HYPH_MIN_WORD_LENGTH(-1)
    , nUPN_HYPH_MIN_LEADING(-1)
    , nUPN_HYPH_MIN_TRAILING(-1)
    , m_nDlbClickEventId(nullptr)
    , m_xLinguModulesFT(m_xBuilder->weld_label(u"lingumodulesft"_ustr))
    , m_xLinguModulesCLB(m_xBuilder->weld_tree_view(u"lingumodules"_ustr))
    , m_xLinguModulesEditPB(m_xBuilder->weld_button(u"lingumodulesedit"_ustr))
    , m_xLinguDicsFT(m_xBuilder->weld_label(u"lingudictsft"_ustr))
    , m_xLinguDicsCLB(m_xBuilder->weld_tree_view(u"lingudicts"_ustr))
    , m_xLinguDicsNewPB(m_xBuilder->weld_button(u"lingudictsnew"_ustr))
    , m_xLinguDicsEditPB(m_xBuilder->weld_button(u"lingudictsedit"_ustr))
    , m_xLinguDicsDelPB(m_xBuilder->weld_button(u"lingudictsdelete"_ustr))
    , m_xLinguOptionsCLB(m_xBuilder->weld_tree_view(u"linguoptions"_ustr))
    , m_xLinguOptionsEditPB(m_xBuilder->weld_button(u"linguoptionsedit"_ustr))
    , m_xMoreDictsBox(m_xBuilder->weld_box(u"moredictsbox"_ustr))
    , m_xMoreDictsLink(m_xBuilder->weld_link_button(u"moredictslink"_ustr))
{
    m_xLinguModulesCLB->enable_toggle_buttons(weld::ColumnToggleType::Check);
    m_xLinguDicsCLB->enable_toggle_buttons(weld::ColumnToggleType::Check);
    m_xLinguOptionsCLB->enable_toggle_buttons(weld::ColumnToggleType::Check);

    m_xLinguModulesCLB->connect_changed( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    m_xLinguModulesCLB->connect_row_activated(LINK(this, SvxLinguTabPage, BoxDoubleClickHdl_Impl));
    m_xLinguModulesCLB->connect_toggled(LINK(this, SvxLinguTabPage, ModulesBoxCheckButtonHdl_Impl));

    m_xLinguModulesEditPB->connect_clicked( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    m_xLinguOptionsEditPB->connect_clicked( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));

    m_xLinguDicsCLB->connect_changed( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    m_xLinguDicsCLB->connect_toggled(LINK(this, SvxLinguTabPage, DicsBoxCheckButtonHdl_Impl));

    m_xLinguDicsNewPB->connect_clicked( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    m_xLinguDicsEditPB->connect_clicked( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    m_xLinguDicsDelPB->connect_clicked( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));

    m_xLinguOptionsCLB->connect_changed( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    m_xLinguOptionsCLB->connect_row_activated(LINK(this, SvxLinguTabPage, BoxDoubleClickHdl_Impl));

    m_xMoreDictsLink->connect_activate_link(LINK(this, SvxLinguTabPage, OnLinkClick));
    if (officecfg::Office::Security::Hyperlinks::Open::get() == SvtExtendedSecurityOptions::OPEN_NEVER)
        m_xMoreDictsBox->hide();

    if (comphelper::LibreOfficeKit::isActive())
    {
        // hide User-defined Dictionaries part
        m_xBuilder->weld_frame(u"dictsframe"_ustr)->hide();
        // hide Get more dictionaries URL + icon
        m_xMoreDictsBox->hide();
    }

    xProp = LinguMgr::GetLinguPropertySet();
    xDicList.set( LinguMgr::GetDictionaryList() );
    if (xDicList.is())
    {
        // keep references to all **currently** available dictionaries,
        // since the diclist may get changed meanwhile (e.g. through the API).
        // We want the dialog to operate on the same set of dictionaries it
        // was started with.
        // Also we have to take care to not lose the last reference when
        // someone else removes a dictionary from the list.
        // removed dics will be replaced by NULL new entries be added to the end
        // Thus we may use indices as consistent references.
        aDics = xDicList->getDictionaries();

        UpdateDicBox_Impl();
    }
    else
    {
        m_xLinguDicsFT->set_sensitive(false);
        m_xLinguDicsCLB->set_sensitive(false);
        m_xLinguDicsNewPB->set_sensitive(false);
        m_xLinguDicsEditPB->set_sensitive(false);
        m_xLinguDicsDelPB->set_sensitive(false);
    }
}

SvxLinguTabPage::~SvxLinguTabPage()
{
    if (m_nDlbClickEventId)
    {
        Application::RemoveUserEvent(m_nDlbClickEventId);
        m_nDlbClickEventId = nullptr;
    }
    pLinguData.reset();
}

std::unique_ptr<SfxTabPage> SvxLinguTabPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                            const SfxItemSet* rAttrSet )
{
    return std::make_unique<SvxLinguTabPage>( pPage, pController, *rAttrSet );
}

OUString SvxLinguTabPage::GetAllStrings()
{
    OUString sAllStrings;
    OUString labels[] = { u"lingumodulesft"_ustr, u"lingudictsft"_ustr, u"label4"_ustr };

    for (const auto& label : labels)
    {
        if (const auto& pString = m_xBuilder->weld_label(label))
            sAllStrings += pString->get_label() + " ";
    }

    sAllStrings += m_xMoreDictsLink->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool SvxLinguTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    bool bModified = true; // !!!!

    // if not HideGroups was called with GROUP_MODULES...
    if (m_xLinguModulesCLB->get_visible())
    {
        DBG_ASSERT( pLinguData, "pLinguData not yet initialized" );
        if (!pLinguData)
            pLinguData.reset( new SvxLinguData_Impl );

        // update spellchecker configuration entries
        const LangImplNameTable *pTable = &pLinguData->GetSpellTable();
        for (auto const& elem : *pTable)
        {
            LanguageType nLang = elem.first;
            const Sequence< OUString > aImplNames(elem.second);
            uno::Reference< XLinguServiceManager2 > xMgr( pLinguData->GetManager() );
            Locale aLocale( LanguageTag::convertToLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cSpell, aLocale, aImplNames );
        }

        // update grammar checker configuration entries
        pTable = &pLinguData->GetGrammarTable();
        for (auto const& elem : *pTable)
        {
            LanguageType nLang = elem.first;
            const Sequence< OUString > aImplNames(elem.second);
            uno::Reference< XLinguServiceManager2 > xMgr( pLinguData->GetManager() );
            Locale aLocale( LanguageTag::convertToLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cGrammar, aLocale, aImplNames );
        }

        // update hyphenator configuration entries
        pTable = &pLinguData->GetHyphTable();
        for (auto const& elem : *pTable)
        {
            LanguageType nLang = elem.first;
            const Sequence< OUString > aImplNames(elem.second);
            uno::Reference< XLinguServiceManager2 > xMgr( pLinguData->GetManager() );
            Locale aLocale( LanguageTag::convertToLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cHyph, aLocale, aImplNames );
        }

        // update thesaurus configuration entries
        pTable = &pLinguData->GetThesTable();
        for (auto const& elem : *pTable)
        {
            LanguageType nLang = elem.first;
            const Sequence< OUString > aImplNames(elem.second);
            uno::Reference< XLinguServiceManager2 > xMgr( pLinguData->GetManager() );
            Locale aLocale( LanguageTag::convertToLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cThes, aLocale, aImplNames );
        }
    }


    // activate dictionaries according to checkbox state

    Sequence< OUString > aActiveDics;
    sal_Int32 nActiveDics = 0;
    int nEntries = m_xLinguDicsCLB->n_children();
    for (int i = 0; i < nEntries; ++i)
    {
        sal_Int32 nDics = aDics.getLength();

        aActiveDics.realloc( nDics );
        OUString *pActiveDic = aActiveDics.getArray();

        DicUserData aData(m_xLinguDicsCLB->get_id(i).toUInt32());
        if (aData.GetEntryId() < nDics)
        {
            bool bChecked = m_xLinguDicsCLB->get_toggle(i) == TRISTATE_TRUE;
            uno::Reference<XDictionary> xDic(aDics[i]);
            if (xDic.is())
            {
                if (LinguMgr::GetIgnoreAllList() == xDic)
                    bChecked = true;
                xDic->setActive( bChecked );
                if (bChecked)
                    pActiveDic[nActiveDics++] = xDic->getName();
            }
        }
    }

    aActiveDics.realloc( nActiveDics );
    Any aTmp;
    aTmp <<= aActiveDics;
    SvtLinguConfig aLngCfg;
    aLngCfg.SetProperty( UPH_ACTIVE_DICTIONARIES, aTmp );


    nEntries = m_xLinguOptionsCLB->n_children();
    for (int j = 0; j < nEntries; ++j)
    {
        OptionsUserData aData(m_xLinguOptionsCLB->get_id(j).toUInt32());
        OUString aPropName( lcl_GetPropertyName( static_cast<EID_OPTIONS>(aData.GetEntryId()) ) );

        Any aAny;
        if (aData.IsCheckable())
        {
            bool bChecked = m_xLinguOptionsCLB->get_toggle(j) == TRISTATE_TRUE;
            aAny <<= bChecked;
        }
        else if (aData.HasNumericValue())
        {
            sal_Int16 nVal = aData.GetNumericValue();
            aAny <<= nVal;
        }

        if (xProp.is())
            xProp->setPropertyValue( aPropName, aAny );
        aLngCfg.SetProperty( aPropName, aAny );
    }

    OptionsUserData aPreBreakData(m_xLinguOptionsCLB->get_id(EID_NUM_PRE_BREAK).toUInt32());
    OptionsUserData aPostBreakData(m_xLinguOptionsCLB->get_id(EID_NUM_POST_BREAK).toUInt32());
    if ( aPreBreakData.IsModified() || aPostBreakData.IsModified() )
    {
        SfxHyphenRegionItem aHyp( SID_ATTR_HYPHENREGION );
        aHyp.GetMinLead()  = static_cast<sal_uInt8>(aPreBreakData.GetNumericValue());
        aHyp.GetMinTrail() = static_cast<sal_uInt8>(aPostBreakData.GetNumericValue());
        rCoreSet->Put( aHyp );
    }

    // automatic spell checking
    bool bNewAutoCheck = m_xLinguOptionsCLB->get_toggle(EID_SPELL_AUTO) == TRISTATE_TRUE;
    const SfxPoolItem* pOld = GetOldItem( *rCoreSet, SID_AUTOSPELL_CHECK );
    if ( !pOld || static_cast<const SfxBoolItem*>(pOld)->GetValue() != bNewAutoCheck )
    {
        rCoreSet->Put( SfxBoolItem( SID_AUTOSPELL_CHECK, bNewAutoCheck ) );
        bModified = true;
    }

    return bModified;
}

sal_uInt32 SvxLinguTabPage::GetDicUserData( const uno::Reference< XDictionary > &rxDic, sal_uInt16 nIdx )
{
    sal_uInt32 nRes = 0;
    DBG_ASSERT( rxDic.is(), "dictionary not supplied" );
    if (rxDic.is())
    {
        uno::Reference< frame::XStorable > xStor( rxDic, UNO_QUERY );

        bool bChecked = rxDic->isActive();
        bool bEditable = !xStor.is() || !xStor->isReadonly();
        bool bDeletable = bEditable;

        nRes = DicUserData( nIdx,
                bChecked, bEditable, bDeletable ).GetUserData();
    }
    return nRes;
}


void SvxLinguTabPage::AddDicBoxEntry(
        const uno::Reference< XDictionary > &rxDic,
        sal_uInt16 nIdx )
{
    m_xLinguDicsCLB->freeze();

    OUString aTxt( ::GetDicInfoStr( rxDic->getName(),
                        LanguageTag( rxDic->getLocale() ).getLanguageType(),
                        DictionaryType_NEGATIVE == rxDic->getDictionaryType() ) );
    m_xLinguDicsCLB->append();  // append at end
    int nEntry = m_xLinguDicsCLB->n_children() - 1;
    DicUserData aData( GetDicUserData( rxDic, nIdx ) );
    m_xLinguDicsCLB->set_id(nEntry, OUString::number(aData.GetUserData()));
    m_xLinguDicsCLB->set_toggle(nEntry, aData.IsChecked() ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguDicsCLB->set_text(nEntry, aTxt, 0);  // append at end

    m_xLinguDicsCLB->thaw();
}

void SvxLinguTabPage::UpdateDicBox_Impl()
{
    m_xLinguDicsCLB->freeze();
    m_xLinguDicsCLB->clear();

    for (sal_Int32 i = 0;  i < aDics.getLength();  ++i)
    {
        const uno::Reference<XDictionary>& rDic = aDics[i];
        if (rDic.is())
            AddDicBoxEntry( rDic, static_cast<sal_uInt16>(i) );
    }

    m_xLinguDicsCLB->thaw();
    if (m_xLinguDicsCLB->n_children())
    {
        m_xLinguDicsCLB->select(0);
        SelectHdl_Impl(*m_xLinguDicsCLB);
    }
}

void SvxLinguTabPage::UpdateModulesBox_Impl()
{
    if (!pLinguData)
        return;

    const ServiceInfoArr &rAllDispSrvcArr = pLinguData->GetDisplayServiceArray();
    const sal_uInt32 nDispSrvcCount = pLinguData->GetDisplayServiceCount();

    m_xLinguModulesCLB->clear();

    for (sal_uInt32 i = 0;  i < nDispSrvcCount;  ++i)
    {
        const ServiceInfo_Impl &rInfo = rAllDispSrvcArr[i];
        m_xLinguModulesCLB->append();
        m_xLinguModulesCLB->set_id(i, weld::toId(&rInfo));
        m_xLinguModulesCLB->set_toggle(i, rInfo.bConfigured ? TRISTATE_TRUE : TRISTATE_FALSE);
        m_xLinguModulesCLB->set_text(i, rInfo.sDisplayName, 0);
    }
    if (nDispSrvcCount)
    {
        m_xLinguModulesCLB->select(0);
        SelectHdl_Impl(*m_xLinguModulesCLB);
    }
    m_xLinguModulesEditPB->set_sensitive( nDispSrvcCount > 0 );
}

void SvxLinguTabPage::Reset( const SfxItemSet* rSet )
{
    // if not HideGroups was called with GROUP_MODULES...
    if (m_xLinguModulesCLB->get_visible())
    {
        if (!pLinguData)
            pLinguData.reset( new SvxLinguData_Impl );
        UpdateModulesBox_Impl();
    }


    //  get data from configuration
    SvtLinguConfig aLngCfg;

    m_xLinguOptionsCLB->freeze();
    m_xLinguOptionsCLB->clear();

    sal_Int16 nVal = 0;
    bool  bVal  = false;
    sal_uInt32 nUserData = 0;

    m_xLinguOptionsCLB->append();
    int nEntry = 0;

    aLngCfg.GetProperty( UPN_IS_SPELL_AUTO ) >>= bVal;
    const SfxPoolItem* pItem = GetItem( *rSet, SID_AUTOSPELL_CHECK );
    if (pItem)
        bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
    nUserData = OptionsUserData( EID_SPELL_AUTO, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sSpellAuto, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_SPELL_AUTO));

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_IS_GRAMMAR_AUTO ) >>= bVal;
    nUserData = OptionsUserData( EID_GRAMMAR_AUTO, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sGrammarAuto, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_GRAMMAR_AUTO));

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_IS_SPELL_UPPER_CASE ) >>= bVal;
    nUserData = OptionsUserData( EID_CAPITAL_WORDS, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sCapitalWords, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_SPELL_UPPER_CASE));

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_IS_SPELL_WITH_DIGITS ) >>= bVal;
    nUserData = OptionsUserData( EID_WORDS_WITH_DIGITS, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sWordsWithDigits, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_SPELL_WITH_DIGITS));

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_IS_SPELL_CLOSED_COMPOUND ) >>= bVal;
    nUserData = OptionsUserData( EID_SPELL_CLOSED_COMPOUND, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sSpellClosedCompound, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_SPELL_CLOSED_COMPOUND));

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_IS_SPELL_HYPHENATED_COMPOUND ) >>= bVal;
    nUserData = OptionsUserData( EID_SPELL_HYPHENATED_COMPOUND, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sSpellHyphenatedCompound, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_SPELL_HYPHENATED_COMPOUND));

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_IS_SPELL_SPECIAL ) >>= bVal;
    nUserData = OptionsUserData( EID_SPELL_SPECIAL, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sSpellSpecial, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_SPELL_SPECIAL));

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_HYPH_MIN_WORD_LENGTH ) >>= nVal;
    nUserData = OptionsUserData( EID_NUM_MIN_WORDLEN, true, static_cast<sal_uInt16>(nVal), false, false).GetUserData();
    m_xLinguOptionsCLB->set_text(nEntry, sNumMinWordlen + " " + OUString::number(nVal), 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_HYPH_MIN_WORD_LENGTH));
    nUPN_HYPH_MIN_WORD_LENGTH = nEntry;

    const SfxHyphenRegionItem *pHyp = nullptr;
    if ( rSet->GetItemState( SID_ATTR_HYPHENREGION, false ) == SfxItemState::SET )
        pHyp = & rSet->Get( SID_ATTR_HYPHENREGION );

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_HYPH_MIN_LEADING ) >>= nVal;
    if (pHyp)
        nVal = static_cast<sal_Int16>(pHyp->GetMinLead());
    nUserData = OptionsUserData( EID_NUM_PRE_BREAK, true, static_cast<sal_uInt16>(nVal), false, false).GetUserData();
    m_xLinguOptionsCLB->set_text(nEntry, sNumPreBreak + " " + OUString::number(nVal), 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_HYPH_MIN_LEADING));
    nUPN_HYPH_MIN_LEADING = nEntry;

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_HYPH_MIN_TRAILING ) >>= nVal;
    if (pHyp)
        nVal = static_cast<sal_Int16>(pHyp->GetMinTrail());
    nUserData = OptionsUserData( EID_NUM_POST_BREAK, true, static_cast<sal_uInt16>(nVal), false, false).GetUserData();
    m_xLinguOptionsCLB->set_text(nEntry, sNumPostBreak + " " + OUString::number(nVal), 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_HYPH_MIN_TRAILING));
    nUPN_HYPH_MIN_TRAILING = nEntry;

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_IS_HYPH_AUTO ) >>= bVal;
    nUserData = OptionsUserData( EID_HYPH_AUTO, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sHyphAuto, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_HYPH_AUTO));

    m_xLinguOptionsCLB->append();
    ++nEntry;

    aLngCfg.GetProperty( UPN_IS_HYPH_SPECIAL ) >>= bVal;
    nUserData = OptionsUserData( EID_HYPH_SPECIAL, false, 0, true, bVal).GetUserData();
    m_xLinguOptionsCLB->set_toggle(nEntry, bVal ? TRISTATE_TRUE : TRISTATE_FALSE);
    m_xLinguOptionsCLB->set_text(nEntry, sHyphSpecial, 0);
    m_xLinguOptionsCLB->set_id(nEntry, OUString::number(nUserData));
    m_xLinguOptionsCLB->set_sensitive(nEntry, !aLngCfg.IsReadOnly(UPN_IS_HYPH_SPECIAL));

    m_xLinguOptionsCLB->thaw();

    m_xLinguOptionsCLB->select(0);
    SelectHdl_Impl(*m_xLinguOptionsCLB);

    m_xLinguModulesCLB->set_size_request(m_xLinguModulesCLB->get_preferred_size().Width(),
                                         m_xLinguModulesCLB->get_height_rows(3));
    m_xLinguDicsCLB->set_size_request(m_xLinguDicsCLB->get_preferred_size().Width(),
                                      m_xLinguDicsCLB->get_height_rows(5));
    m_xLinguOptionsCLB->set_size_request(m_xLinguOptionsCLB->get_preferred_size().Width(),
                                         m_xLinguOptionsCLB->get_height_rows(5));

    if (officecfg::Office::Linguistic::General::DictionaryList::ActiveDictionaries::isReadOnly())
    {
        m_xLinguDicsFT->set_sensitive(false);
        m_xLinguDicsCLB->set_sensitive(false);
        m_xLinguDicsNewPB->set_sensitive(false);
        m_xLinguDicsEditPB->set_sensitive(false);
        m_xLinguDicsDelPB->set_sensitive(false);
    }
}

IMPL_LINK(SvxLinguTabPage, BoxDoubleClickHdl_Impl, weld::TreeView&, rBox, bool)
{
    if (&rBox == m_xLinguModulesCLB.get() && !m_nDlbClickEventId)
    {
        //! in order to avoid a bug causing a GPF when double clicking
        //! on a module entry and exiting the "Edit Modules" dialog
        //! after that.
        m_nDlbClickEventId = Application::PostUserEvent(LINK(this, SvxLinguTabPage, PostDblClickHdl_Impl));
    }
    else if (&rBox == m_xLinguOptionsCLB.get())
    {
        ClickHdl_Impl(*m_xLinguOptionsEditPB);
    }
    return true;
}

IMPL_LINK_NOARG(SvxLinguTabPage, PostDblClickHdl_Impl, void*, void)
{
    m_nDlbClickEventId = nullptr;
    ClickHdl_Impl(*m_xLinguModulesEditPB);
}

IMPL_LINK(SvxLinguTabPage, ModulesBoxCheckButtonHdl_Impl, const weld::TreeView::iter_col&, rRowCol, void)
{
    if (!pLinguData)
        return;
    pLinguData->Reconfigure(m_xLinguModulesCLB->get_text(rRowCol.first),
                            m_xLinguModulesCLB->get_toggle(rRowCol.first) == TRISTATE_TRUE);
}

IMPL_LINK(SvxLinguTabPage, DicsBoxCheckButtonHdl_Impl, const weld::TreeView::iter_col&, rRowCol, void)
{
    const uno::Reference<XDictionary> &rDic = aDics[m_xLinguDicsCLB->get_iter_index_in_parent(rRowCol.first)];
    if (LinguMgr::GetIgnoreAllList() == rDic)
        m_xLinguDicsCLB->set_toggle(rRowCol.first, TRISTATE_TRUE);
}

IMPL_LINK(SvxLinguTabPage, ClickHdl_Impl, weld::Button&, rBtn, void)
{
    if (m_xLinguModulesEditPB.get() == &rBtn)
    {
        if (!pLinguData)
            pLinguData.reset( new SvxLinguData_Impl );

        SvxLinguData_Impl aOldLinguData(*pLinguData);
        SvxEditModulesDlg aDlg(GetFrameWeld(), *pLinguData);
        if (aDlg.run() != RET_OK)
            *pLinguData = std::move(aOldLinguData);

        // evaluate new status of 'bConfigured' flag
        sal_uInt32 nLen = pLinguData->GetDisplayServiceCount();
        for (sal_uInt32 i = 0;  i < nLen;  ++i)
            pLinguData->GetDisplayServiceArray()[i].bConfigured = false;
        for (const auto& locale : pLinguData->GetAllSupportedLocales())
        {
            LanguageType nLang = LanguageTag::convertToLanguageType(locale);
            if (pLinguData->GetSpellTable().count( nLang ))
                pLinguData->SetChecked( pLinguData->GetSpellTable()[ nLang ] );
            if (pLinguData->GetGrammarTable().count( nLang ))
                pLinguData->SetChecked( pLinguData->GetGrammarTable()[ nLang ] );
            if (pLinguData->GetHyphTable().count( nLang ))
                pLinguData->SetChecked( pLinguData->GetHyphTable()[ nLang ] );
            if (pLinguData->GetThesTable().count( nLang ))
                pLinguData->SetChecked( pLinguData->GetThesTable()[ nLang ] );
        }

        // show new status of modules
        UpdateModulesBox_Impl();
    }
    else if (m_xLinguDicsNewPB.get() == &rBtn)
    {
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        ScopedVclPtr<AbstractSvxNewDictionaryDialog> aDlg(pFact->CreateSvxNewDictionaryDialog(GetFrameWeld()));
        uno::Reference< XDictionary >  xNewDic;
        if ( aDlg->Execute() == RET_OK )
            xNewDic = aDlg->GetNewDictionary();
        if ( xNewDic.is() )
        {
            // add new dics to the end
            sal_Int32 nLen = aDics.getLength();
            aDics.realloc( nLen + 1 );

            aDics.getArray()[ nLen ] = xNewDic;

            AddDicBoxEntry( xNewDic, static_cast<sal_uInt16>(nLen) );
        }
    }
    else if (m_xLinguDicsEditPB.get() == &rBtn)
    {
        int nEntry = m_xLinguDicsCLB->get_selected_index();
        if (nEntry != -1)
        {
            DicUserData aData(m_xLinguDicsCLB->get_id(nEntry).toUInt32());
            sal_uInt16 nDicPos = aData.GetEntryId();
            sal_Int32 nDics = aDics.getLength();
            if (nDicPos < nDics)
            {
                uno::Reference<XDictionary> xDic = aDics[nDicPos];
                if (xDic.is())
                {
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    ScopedVclPtr<VclAbstractDialog> aDlg(pFact->CreateSvxEditDictionaryDialog(GetFrameWeld(), xDic->getName()));
                    aDlg->Execute();
                }
            }
        }
    }
    else if (m_xLinguDicsDelPB.get() == &rBtn)
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(GetFrameWeld(), u"cui/ui/querydeletedictionarydialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQuery(xBuilder->weld_message_dialog(u"QueryDeleteDictionaryDialog"_ustr));
        if (RET_NO == xQuery->run())
            return;

        int nEntry = m_xLinguDicsCLB->get_selected_index();
        if (nEntry != -1)
        {
            DicUserData aData(m_xLinguDicsCLB->get_id(nEntry).toUInt32());
            sal_uInt16 nDicPos = aData.GetEntryId();
            sal_Int32 nDics = aDics.getLength();
            if (nDicPos < nDics)
            {
                uno::Reference<XDictionary> xDic = aDics[nDicPos];
                if (xDic.is())
                {
                    if (LinguMgr::GetIgnoreAllList() == xDic)
                        xDic->clear();
                    else
                    {
                        if (xDicList.is())
                            xDicList->removeDictionary( xDic );

                        uno::Reference< frame::XStorable > xStor( xDic, UNO_QUERY );
                        if ( xStor->hasLocation() && !xStor->isReadonly() )
                        {
                            OUString sURL = xStor->getLocation();
                            INetURLObject aObj(sURL);
                            DBG_ASSERT( aObj.GetProtocol() == INetProtocol::File,
                                    "non-file URLs cannot be deleted" );
                            if ( aObj.GetProtocol() == INetProtocol::File )
                            {
                                KillFile_Impl( aObj.GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
                            }
                        }

                        aDics.getArray()[ nDicPos ] = nullptr;

                        // remove entry from checklistbox
                        int nCnt = m_xLinguDicsCLB->n_children();
                        for (int i = 0;  i < nCnt;  ++i)
                        {
                            DicUserData aDicData(m_xLinguDicsCLB->get_id(i).toUInt32());
                            if (aDicData.GetEntryId() == nDicPos )
                            {
                                m_xLinguDicsCLB->remove(i);
                                break;
                            }
                        }
                        DBG_ASSERT( nCnt > m_xLinguDicsCLB->n_children(),
                                "remove failed ?");
                    }
                }
            }
        }
    }
    else if (m_xLinguOptionsEditPB.get() == &rBtn)
    {
        int nEntry = m_xLinguOptionsCLB->get_selected_index();
        DBG_ASSERT(nEntry != -1, "no entry selected");
        if (nEntry != -1)
        {
            OptionsUserData aData(m_xLinguOptionsCLB->get_id(nEntry).toUInt32());
            if (aData.HasNumericValue())
            {
                sal_uInt16 nRID = aData.GetEntryId();
                OptionsBreakSet aDlg(GetFrameWeld(), nRID);
                aDlg.GetNumericFld().set_value(aData.GetNumericValue());
                if (RET_OK == aDlg.run())
                {
                    int nVal = aDlg.GetNumericFld().get_value();
                    if (-1 != nVal && aData.GetNumericValue() != nVal)
                    {
                        aData.SetNumericValue( static_cast<sal_uInt8>(nVal) ); //! sets IsModified !
                        m_xLinguOptionsCLB->set_id(nEntry, OUString::number(aData.GetUserData()));
                        if (nEntry == nUPN_HYPH_MIN_WORD_LENGTH)
                            m_xLinguOptionsCLB->set_text(nEntry, sNumMinWordlen + " " + OUString::number(nVal), 0);
                        else if (nEntry == nUPN_HYPH_MIN_LEADING)
                            m_xLinguOptionsCLB->set_text(nEntry, sNumPreBreak + " " + OUString::number(nVal), 0);
                        else if (nEntry == nUPN_HYPH_MIN_TRAILING)
                            m_xLinguOptionsCLB->set_text(nEntry, sNumPostBreak + " " + OUString::number(nVal), 0);
                        m_xLinguOptionsCLB->set_id(nEntry, OUString::number(aData.GetUserData()));
                    }
                }
            }
        }
    }
    else
    {
        SAL_WARN("cui.options", "rBtn unexpected value");
    }
}

IMPL_LINK(SvxLinguTabPage, SelectHdl_Impl, weld::TreeView&, rBox, void)
{
    if (m_xLinguModulesCLB.get() == &rBox)
    {
    }
    else if (m_xLinguDicsCLB.get() == &rBox)
    {
        int nEntry = rBox.get_selected_index();
        if (nEntry != -1)
        {
            DicUserData aData(rBox.get_id(nEntry).toUInt32());

            // always allow to edit (i.e. at least view the content of the dictionary)
            m_xLinguDicsEditPB->set_sensitive( true );
            m_xLinguDicsDelPB->set_sensitive( aData.IsDeletable() );
        }
    }
    else if (m_xLinguOptionsCLB.get() == &rBox)
    {
        int nEntry = rBox.get_selected_index();
        if (nEntry != -1)
        {
            OptionsUserData aData(rBox.get_id(nEntry).toUInt32());
            m_xLinguOptionsEditPB->set_sensitive( aData.HasNumericValue() );
        }
    }
    else
    {
        SAL_WARN("cui.options", "rBtn unexpected value");
    }
}

void SvxLinguTabPage::HideGroups( sal_uInt16 nGrp )
{
    if ( 0 != ( GROUP_MODULES & nGrp ) )
    {
        m_xLinguModulesFT->hide();
        m_xLinguModulesCLB->hide();
        m_xLinguModulesEditPB->hide();

        if (officecfg::Office::Security::Hyperlinks::Open::get() != SvtExtendedSecurityOptions::OPEN_NEVER &&
            !comphelper::LibreOfficeKit::isActive())
        {
            m_xMoreDictsBox->show();
        }
    }
}

IMPL_STATIC_LINK_NOARG(SvxLinguTabPage, OnLinkClick, weld::LinkButton&, bool)
{
    comphelper::dispatchCommand(u".uno:MoreDictionaries"_ustr, {});
    return true;
}

SvxEditModulesDlg::SvxEditModulesDlg(weld::Window* pParent, SvxLinguData_Impl& rData)
    : GenericDialogController(pParent, u"cui/ui/editmodulesdialog.ui"_ustr, u"EditModulesDialog"_ustr)
    , sSpell(CuiResId(RID_CUISTR_SPELL))
    , sHyph(CuiResId(RID_CUISTR_HYPH))
    , sThes(CuiResId(RID_CUISTR_THES))
    , sGrammar(CuiResId(RID_CUISTR_GRAMMAR))
    , rLinguData(rData)
    , m_xModulesCLB(m_xBuilder->weld_tree_view(u"lingudicts"_ustr))
    , m_xPrioUpPB(m_xBuilder->weld_button(u"up"_ustr))
    , m_xPrioDownPB(m_xBuilder->weld_button(u"down"_ustr))
    , m_xBackPB(m_xBuilder->weld_button(u"back"_ustr))
    , m_xMoreDictsLink(m_xBuilder->weld_link_button(u"moredictslink"_ustr))
    , m_xClosePB(m_xBuilder->weld_button(u"close"_ustr))
    , m_xLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box(u"language"_ustr)))
{
    m_xModulesCLB->set_size_request(m_xModulesCLB->get_approximate_digit_width() * 40,
                                    m_xModulesCLB->get_height_rows(12));

    m_xModulesCLB->enable_toggle_buttons(weld::ColumnToggleType::Check);

    pDefaultLinguData.reset( new SvxLinguData_Impl( rLinguData ) );

    m_xModulesCLB->connect_changed( LINK( this, SvxEditModulesDlg, SelectHdl_Impl ));
    m_xModulesCLB->connect_toggled(LINK(this, SvxEditModulesDlg, BoxCheckButtonHdl_Impl));

    m_xClosePB->connect_clicked( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    m_xPrioUpPB->connect_clicked( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    m_xPrioDownPB->connect_clicked( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    m_xBackPB->connect_clicked( LINK( this, SvxEditModulesDlg, BackHdl_Impl ));
    // in case of not installed language modules
    m_xPrioUpPB->set_sensitive( false );
    m_xPrioDownPB->set_sensitive( false );

    m_xMoreDictsLink->connect_activate_link(LINK(this, SvxEditModulesDlg, OnLinkClick));
    if (officecfg::Office::Security::Hyperlinks::Open::get() == SvtExtendedSecurityOptions::OPEN_NEVER)
        m_xMoreDictsLink->hide();

    // set that we want the checkbox shown if spellchecking is available
    m_xLanguageLB->SetLanguageList(SvxLanguageListFlags::EMPTY, false, false, true);

    //fill language box
    const auto& rLoc = rLinguData.GetAllSupportedLocales();
    std::vector<LanguageType> aLanguages;
    aLanguages.reserve(rLoc.size());
    std::transform(rLoc.begin(), rLoc.end(), std::back_inserter(aLanguages),
                   [](Locale const& locale) { return LanguageTag::convertToLanguageType(locale); });
    m_xLanguageLB->InsertLanguages(aLanguages);
    LanguageType eSysLang = MsLangId::getConfiguredSystemLanguage();
    m_xLanguageLB->set_active_id( eSysLang );
    if (m_xLanguageLB->get_active_id() != eSysLang)
        m_xLanguageLB->set_active(0);

    css::uno::Reference < css::uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
    m_xReadWriteAccess = css::configuration::ReadWriteAccess::create(xContext, u"*"_ustr);

    m_xLanguageLB->connect_changed( LINK( this, SvxEditModulesDlg, LangSelectListBoxHdl_Impl ));
    LangSelectHdl_Impl(m_xLanguageLB.get());
}

SvxEditModulesDlg::~SvxEditModulesDlg()
{
    for (int i = 0, nEntryCount = m_xModulesCLB->n_children(); i < nEntryCount; ++i)
        delete weld::fromId<ModuleUserData_Impl*>(m_xModulesCLB->get_id(i));
}

IMPL_LINK( SvxEditModulesDlg, SelectHdl_Impl, weld::TreeView&, rBox, void )
{
    int nCurPos = rBox.get_selected_index();
    if (nCurPos == -1)
        return;

    bool bDisableUp = true;
    bool bDisableDown = true;
    ModuleUserData_Impl* pData = weld::fromId<ModuleUserData_Impl*>(rBox.get_id(nCurPos));
    if (!pData->IsParent() && pData->GetType() != TYPE_HYPH)
    {
        if (nCurPos < rBox.n_children() - 1)
        {
            bDisableDown = weld::fromId<ModuleUserData_Impl*>(rBox.get_id(nCurPos + 1))->IsParent();
        }
        if (nCurPos > 1)
        {
            bDisableUp = weld::fromId<ModuleUserData_Impl*>(rBox.get_id(nCurPos - 1))->IsParent();
        }
    }
    m_xPrioUpPB->set_sensitive(!bDisableUp);
    m_xPrioDownPB->set_sensitive(!bDisableDown);
}

IMPL_LINK( SvxEditModulesDlg, BoxCheckButtonHdl_Impl, const weld::TreeView::iter_col&, rRowCol, void )
{
    ModuleUserData_Impl* pData = weld::fromId<ModuleUserData_Impl*>(m_xModulesCLB->get_id(rRowCol.first));
    if (pData->IsParent() || pData->GetType() != TYPE_HYPH)
        return;

    // make hyphenator checkboxes function as radio-buttons
    // (at most one box may be checked)
    auto nPos = m_xModulesCLB->get_iter_index_in_parent(rRowCol.first);
    for (int i = 0, nEntryCount = m_xModulesCLB->n_children(); i < nEntryCount; ++i)
    {
        pData = weld::fromId<ModuleUserData_Impl*>(m_xModulesCLB->get_id(i));
        if (!pData->IsParent() && pData->GetType() == TYPE_HYPH && i != nPos)
        {
            m_xModulesCLB->set_toggle(i, TRISTATE_FALSE);
        }
    }
}

IMPL_LINK_NOARG(SvxEditModulesDlg, LangSelectListBoxHdl_Impl, weld::ComboBox&, void)
{
    LangSelectHdl_Impl(m_xLanguageLB.get());
}

void SvxEditModulesDlg::LangSelectHdl_Impl(const SvxLanguageBox* pBox)
{
    LanguageType  eCurLanguage = m_xLanguageLB->get_active_id();
    static Locale aLastLocale;
    Locale aCurLocale( LanguageTag::convertToLocale( eCurLanguage));

    if (pBox)
    {
        // save old probably changed settings
        // before switching to new language entries

        LanguageType nLang = LanguageTag::convertToLanguageType( aLastLocale );

        sal_Int32 nStart = 0, nLocalIndex = 0;
        Sequence< OUString > aChange;
        bool bChanged = false;
        for (int i = 0, nEntryCount = m_xModulesCLB->n_children(); i < nEntryCount; ++i)
        {
            ModuleUserData_Impl* pData = weld::fromId<ModuleUserData_Impl*>(m_xModulesCLB->get_id(i));
            if (pData->IsParent())
            {
                if (bChanged)
                {
                    LangImplNameTable *pTable = nullptr;
                    sal_uInt8 nType = pData->GetType();
                    switch (nType - 1)
                    {
                        case  TYPE_SPELL    : pTable = &rLinguData.GetSpellTable(); break;
                        case  TYPE_GRAMMAR  : pTable = &rLinguData.GetGrammarTable();  break;
                        case  TYPE_HYPH     : pTable = &rLinguData.GetHyphTable();  break;
                        case  TYPE_THES     : pTable = &rLinguData.GetThesTable();  break;
                    }
                    if (pTable)
                    {
                        aChange.realloc(nStart);
                        (*pTable)[ nLang ] = aChange;
                    }
                }
                nLocalIndex = nStart = 0;
                aChange.realloc(nEntryCount);
                bChanged = false;
            }
            else
            {
                OUString* pChange = aChange.getArray();
                pChange[nStart] = pData->GetImplName();
                bChanged |= pData->GetIndex() != nLocalIndex ||
                    static_cast<TriState>(pData->IsChecked()) != m_xModulesCLB->get_toggle(i);
                if (m_xModulesCLB->get_toggle(i))
                    nStart++;
                ++nLocalIndex;
            }
        }
        if(bChanged)
        {
            aChange.realloc(nStart);
            rLinguData.GetThesTable()[ nLang ] = aChange;
        }
    }

    for (int i = 0, nEntryCount = m_xModulesCLB->n_children(); i < nEntryCount; ++i)
        delete weld::fromId<ModuleUserData_Impl*>(m_xModulesCLB->get_id(i));
    m_xModulesCLB->clear();

    // display entries for new selected language

    if (LANGUAGE_DONTKNOW != eCurLanguage)
    {
        bool bReadOnly = false;

        int nRow = 0;
        // spellchecker entries

        ModuleUserData_Impl* pUserData = new ModuleUserData_Impl(
                                         OUString(), true, false, TYPE_SPELL, 0 );
        OUString sId(weld::toId(pUserData));
        m_xModulesCLB->append(nullptr);
        m_xModulesCLB->set_id(nRow, sId);
        m_xModulesCLB->set_text(nRow, sSpell, 0);
        m_xModulesCLB->set_text_emphasis(nRow, true, 0);
        ++nRow;

        OUString aLangNodeName = LanguageTag::convertToBcp47(aCurLocale);
        OUString aConfigPath = officecfg::Office::Linguistic::ServiceManager::path() + "/SpellCheckerList/" + aLangNodeName;
        if (m_xReadWriteAccess->hasPropertyByHierarchicalName(aConfigPath))
        {
            css::beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath);
            bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
        }

        sal_Int32 nLocalIndex = 0;  // index relative to parent
        for (auto& name : rLinguData.GetSortedImplNames(eCurLanguage, TYPE_SPELL))
        {
            OUString aImplName;
            bool     bIsSuppLang = false;

            ServiceInfo_Impl* pInfo = rLinguData.GetInfoByImplName(name);
            if (pInfo)
            {
                bIsSuppLang = pInfo->xSpell.is()  &&
                              pInfo->xSpell->hasLocale( aCurLocale );
                aImplName = pInfo->sSpellImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                OUString aTxt( pInfo->sDisplayName );

                LangImplNameTable &rTable = rLinguData.GetSpellTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    SAL_INFO( "cui.options", "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                pUserData = new ModuleUserData_Impl( aImplName, false,
                                        bCheck, TYPE_SPELL, static_cast<sal_uInt8>(nLocalIndex++) );
                sId = weld::toId(pUserData);

                m_xModulesCLB->append(nullptr);
                m_xModulesCLB->set_id(nRow, sId);
                m_xModulesCLB->set_toggle(nRow, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_xModulesCLB->set_text(nRow, aTxt, 0);
                m_xModulesCLB->set_text_emphasis(nRow, false, 0);
                m_xModulesCLB->set_sensitive(nRow, !bReadOnly);
                ++nRow;
            }
        }

        // grammar checker entries

        pUserData = new ModuleUserData_Impl( OUString(), true, false, TYPE_GRAMMAR, 0 );
        sId = weld::toId(pUserData);
        m_xModulesCLB->append(nullptr);
        m_xModulesCLB->set_id(nRow, sId);
        m_xModulesCLB->set_text(nRow, sGrammar, 0);
        m_xModulesCLB->set_text_emphasis(nRow, true, 0);
        ++nRow;

        aConfigPath = officecfg::Office::Linguistic::ServiceManager::path() + "/GrammarCheckerList/" + aLangNodeName;
        if (m_xReadWriteAccess->hasPropertyByHierarchicalName(aConfigPath))
        {
            css::beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath);
            bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
        }

        nLocalIndex = 0;
        for (auto& name : rLinguData.GetSortedImplNames(eCurLanguage, TYPE_GRAMMAR))
        {
            OUString aImplName;
            bool     bIsSuppLang = false;

            ServiceInfo_Impl* pInfo = rLinguData.GetInfoByImplName(name);
            if (pInfo)
            {
                bIsSuppLang = pInfo->xGrammar.is()  &&
                              pInfo->xGrammar->hasLocale( aCurLocale );
                aImplName = pInfo->sGrammarImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                OUString aTxt( pInfo->sDisplayName );

                LangImplNameTable &rTable = rLinguData.GetGrammarTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    SAL_INFO( "cui.options", "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                pUserData = new ModuleUserData_Impl( aImplName, false,
                                        bCheck, TYPE_GRAMMAR, static_cast<sal_uInt8>(nLocalIndex++) );

                sId = weld::toId(pUserData);

                m_xModulesCLB->append(nullptr);
                m_xModulesCLB->set_id(nRow, sId);
                m_xModulesCLB->set_toggle(nRow, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_xModulesCLB->set_text(nRow, aTxt, 0);
                m_xModulesCLB->set_text_emphasis(nRow, false, 0);
                m_xModulesCLB->set_sensitive(nRow, !bReadOnly);
                ++nRow;
            }
        }

        // hyphenator entries

        pUserData = new ModuleUserData_Impl( OUString(), true, false, TYPE_HYPH, 0 );
        sId = weld::toId(pUserData);
        m_xModulesCLB->append(nullptr);
        m_xModulesCLB->set_id(nRow, sId);
        m_xModulesCLB->set_text(nRow, sHyph, 0);
        m_xModulesCLB->set_text_emphasis(nRow, true, 0);
        ++nRow;

        aConfigPath = officecfg::Office::Linguistic::ServiceManager::path() + "/HyphenatorList/" + aLangNodeName;
        if (m_xReadWriteAccess->hasPropertyByHierarchicalName(aConfigPath))
        {
            css::beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath);
            bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
        }

        nLocalIndex = 0;
        for (auto& name : rLinguData.GetSortedImplNames(eCurLanguage, TYPE_HYPH))
        {
            OUString aImplName;
            bool     bIsSuppLang = false;

            ServiceInfo_Impl* pInfo = rLinguData.GetInfoByImplName(name);
            if (pInfo)
            {
                bIsSuppLang = pInfo->xHyph.is()  &&
                              pInfo->xHyph->hasLocale( aCurLocale );
                aImplName = pInfo->sHyphImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                OUString aTxt( pInfo->sDisplayName );

                LangImplNameTable &rTable = rLinguData.GetHyphTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    SAL_INFO( "cui.options", "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                pUserData = new ModuleUserData_Impl( aImplName, false,
                                        bCheck, TYPE_HYPH, static_cast<sal_uInt8>(nLocalIndex++) );
                sId = weld::toId(pUserData);

                m_xModulesCLB->append(nullptr);
                m_xModulesCLB->set_id(nRow, sId);
                m_xModulesCLB->set_toggle(nRow, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_xModulesCLB->set_text(nRow, aTxt, 0);
                m_xModulesCLB->set_text_emphasis(nRow, false, 0);
                m_xModulesCLB->set_sensitive(nRow, !bReadOnly);
                ++nRow;
            }
        }

        // thesaurus entries

        pUserData = new ModuleUserData_Impl( OUString(), true, false, TYPE_THES, 0 );
        sId = weld::toId(pUserData);
        m_xModulesCLB->append(nullptr);
        m_xModulesCLB->set_id(nRow, sId);
        m_xModulesCLB->set_text(nRow, sThes, 0);
        m_xModulesCLB->set_text_emphasis(nRow, true, 0);
        ++nRow;

        aConfigPath = officecfg::Office::Linguistic::ServiceManager::path() + "/ThesaurusList/" + aLangNodeName;
        if (m_xReadWriteAccess->hasPropertyByHierarchicalName(aConfigPath))
        {
            css::beans::Property aProperty = m_xReadWriteAccess->getPropertyByHierarchicalName(aConfigPath);
            bReadOnly = (aProperty.Attributes & css::beans::PropertyAttribute::READONLY) != 0;
        }

        nLocalIndex = 0;
        for (auto& name : rLinguData.GetSortedImplNames(eCurLanguage, TYPE_THES))
        {
            OUString aImplName;
            bool     bIsSuppLang = false;

            ServiceInfo_Impl* pInfo = rLinguData.GetInfoByImplName(name);
            if (pInfo)
            {
                bIsSuppLang = pInfo->xThes.is()  &&
                              pInfo->xThes->hasLocale( aCurLocale );
                aImplName = pInfo->sThesImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                OUString aTxt( pInfo->sDisplayName );

                LangImplNameTable &rTable = rLinguData.GetThesTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    SAL_INFO( "cui.options", "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                pUserData = new ModuleUserData_Impl( aImplName, false,
                                        bCheck, TYPE_THES, static_cast<sal_uInt8>(nLocalIndex++) );
                sId = weld::toId(pUserData);

                m_xModulesCLB->append(nullptr);
                m_xModulesCLB->set_id(nRow, sId);
                m_xModulesCLB->set_toggle(nRow, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
                m_xModulesCLB->set_text(nRow, aTxt, 0);
                m_xModulesCLB->set_text_emphasis(nRow, false, 0);
                m_xModulesCLB->set_sensitive(nRow, !bReadOnly);
                ++nRow;
            }
        }
    }
    aLastLocale = aCurLocale;
}

IMPL_LINK( SvxEditModulesDlg, UpDownHdl_Impl, weld::Button&, rBtn, void )
{
    bool bUp = m_xPrioUpPB.get() == &rBtn;
    int nCurPos = m_xModulesCLB->get_selected_index();
    if (nCurPos == -1)
        return;

    m_xModulesCLB->freeze();

    OUString sId(m_xModulesCLB->get_id(nCurPos));
    OUString sStr(m_xModulesCLB->get_text(nCurPos));
    bool bIsChecked = m_xModulesCLB->get_toggle(nCurPos);

    m_xModulesCLB->remove(nCurPos);

    int nDestPos = bUp ? nCurPos - 1 : nCurPos + 1;

    m_xModulesCLB->insert_text(nDestPos, sStr);
    m_xModulesCLB->set_id(nDestPos, sId);
    m_xModulesCLB->set_toggle(nDestPos, bIsChecked ? TRISTATE_TRUE : TRISTATE_FALSE);

    m_xModulesCLB->thaw();

    m_xModulesCLB->select(nDestPos);
    SelectHdl_Impl(*m_xModulesCLB);
}

IMPL_LINK_NOARG(SvxEditModulesDlg, ClickHdl_Impl, weld::Button&, void)
{
    // store language config
    LangSelectHdl_Impl(m_xLanguageLB.get());
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SvxEditModulesDlg, BackHdl_Impl, weld::Button&, void)
{
    rLinguData = *pDefaultLinguData;
    LangSelectHdl_Impl(nullptr);
}

IMPL_STATIC_LINK_NOARG(SvxEditModulesDlg, OnLinkClick, weld::LinkButton&, bool)
{
    comphelper::dispatchCommand(u".uno:MoreDictionaries"_ustr, {});
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
