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

#include <vcl/msgbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/settings.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <unotools/lingucfg.hxx>
#include <editeng/unolingu.hxx>
#include <svx/dlgutil.hxx>
#include <linguistic/lngprops.hxx>
#include <linguistic/misc.hxx>
#include <sfx2/sfxuno.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/urlobj.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/linguistic2/LinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#include <com/sun/star/linguistic2/XProofreader.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#include <com/sun/star/linguistic2/DictionaryListEvent.hpp>
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <unotools/extendedsecurityoptions.hxx>
#include <svtools/treelistbox.hxx>
#include "svtools/treelistentry.hxx"
#include <svtools/langhelp.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>

#include <svx/svxdlg.hxx>
#include <editeng/optitems.hxx>
#include "optlingu.hxx"
#include <dialmgr.hxx>
#include <cuires.hrc>
#include "helpid.hrc"

#include <ucbhelper/content.hxx>

#include <vector>
#include <map>

using namespace ::ucbhelper;
using namespace ::com::sun::star;
using namespace css::lang;
using namespace css::uno;
using namespace css::linguistic2;
using namespace css::beans;

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1

static const sal_Char cSpell[]   = SN_SPELLCHECKER;
static const sal_Char cGrammar[] = SN_GRAMMARCHECKER;
static const sal_Char cHyph[]    = SN_HYPHENATOR;
static const sal_Char cThes[]    = SN_THESAURUS;

// static ----------------------------------------------------------------

static Sequence< sal_Int16 > lcl_LocaleSeqToLangSeq( const Sequence< Locale > &rSeq )
{
    sal_Int32 nLen = rSeq.getLength();
    Sequence< sal_Int16 > aRes( nLen );
    sal_Int16 *pRes = aRes.getArray();
    const Locale *pSeq = rSeq.getConstArray();
    for (sal_Int32 i = 0;  i < nLen;  ++i)
    {
        pRes[i] = LanguageTag::convertToLanguageType( pSeq[i] );
    }
    return aRes;
}


static bool lcl_SeqHasLang( const Sequence< sal_Int16 > &rSeq, sal_Int16 nLang )
{
    sal_Int32 nLen = rSeq.getLength();
    const sal_Int16 *pLang = rSeq.getConstArray();
    sal_Int32 nPos = -1;
    for (sal_Int32 i = 0;  i < nLen  &&  nPos < 0;  ++i)
    {
        if (nLang == pLang[i])
            nPos = i;
    }
    return nPos >= 0;
}


static sal_Int32 lcl_SeqGetEntryPos(
    const Sequence< OUString > &rSeq, const OUString &rEntry )
{
    sal_Int32 i;
    sal_Int32 nLen = rSeq.getLength();
    const OUString *pItem = rSeq.getConstArray();
    for (i = 0;  i < nLen;  ++i)
    {
        if (rEntry == pItem[i])
            break;
    }
    return i < nLen ? i : -1;
}

static void lcl_OpenURL( const OUString& _sURL )
{
    if ( !_sURL.isEmpty() )
    {
        OUString sURL = _sURL;
        localizeWebserviceURI(sURL);
        try
        {
            uno::Reference< uno::XComponentContext > xContext =
                ::comphelper::getProcessComponentContext();
            uno::Reference< css::system::XSystemShellExecute > xSystemShell(
                css::system::SystemShellExecute::create(xContext) );
            xSystemShell->execute( sURL, OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY );
        }
        catch( const uno::Exception& e )
        {
             OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
                OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
}

bool KillFile_Impl( const OUString& rURL )
{
    bool bRet = true;
    try
    {
        Content aCnt( rURL, uno::Reference< css::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        aCnt.executeCommand( OUString("delete"), makeAny( true ) );
    }
    catch( css::ucb::CommandAbortedException& )
    {
        SAL_WARN( "cui.options", "KillFile: CommandAbortedException" );
        bRet = false;
    }
    catch( ... )
    {
        SAL_WARN( "cui.options", "KillFile: Any other exception" );
        bRet = false;
    }

    return bRet;
}

// 0x 0p 0t 0c nn
// p: 1 -> parent
// t: 1 -> spell, 2 -> hyph, 3 -> thes, 4 -> grammar
// c: 1 -> checked 0 -> unchecked
// n: index

#define TYPE_SPELL      (sal_uInt8)1
#define TYPE_GRAMMAR    (sal_uInt8)2
#define TYPE_HYPH       (sal_uInt8)3
#define TYPE_THES       (sal_uInt8)4

class ModuleUserData_Impl
{
    bool bParent;
    bool bIsChecked;
    sal_uInt8 nType;
    sal_uInt8 nIndex;
    OUString  sImplName;

public:
    ModuleUserData_Impl( const OUString& sImpName, bool bIsParent, bool bChecked, sal_uInt8 nSetType, sal_uInt8 nSetIndex ) :
        bParent(bIsParent),
        bIsChecked(bChecked),
        nType(nSetType),
        nIndex(nSetIndex),
        sImplName(sImpName)
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
    sal_uLong   nVal;

public:
    explicit DicUserData( sal_uLong nUserData ) : nVal( nUserData ) {}
    DicUserData( sal_uInt16 nEID,
                 bool bChecked, bool bEditable, bool bDeletable );

    sal_uLong   GetUserData() const         { return nVal; }
    sal_uInt16  GetEntryId() const          { return (sal_uInt16)(nVal >> 16); }
    bool        IsChecked() const           { return (bool)((nVal >>  8) & 0x01); }
    bool        IsDeletable() const         { return (bool)((nVal >> 10) & 0x01); }
};


DicUserData::DicUserData(
        sal_uInt16 nEID,
        bool bChecked, bool bEditable, bool bDeletable )
{
    DBG_ASSERT( nEID < 65000, "Entry Id out of range" );
    nVal =  ((sal_uLong)(0xFFFF & nEID)         << 16) |
            ((sal_uLong)(bChecked ? 1 : 0)      <<  8) |
            ((sal_uLong)(bEditable ? 1 : 0)     <<  9) |
            ((sal_uLong)(bDeletable ? 1 : 0)    << 10);
}


// class BrwString_Impl -------------------------------------------------

static void lcl_SetCheckButton( SvTreeListEntry* pEntry, bool bCheck )
{
    SvLBoxButton* pItem = static_cast<SvLBoxButton*>(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found");
    if (pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        if (bCheck)
            pItem->SetStateChecked();
        else
            pItem->SetStateUnchecked();
    }
}


class BrwStringDic_Impl : public SvLBoxString
{
public:

    BrwStringDic_Impl( SvTreeListEntry* pEntry, sal_uInt16 nFlags,
        const OUString& rStr ) : SvLBoxString( pEntry, nFlags, rStr ) {}

    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};

void BrwStringDic_Impl::Paint(const Point& rPos, SvTreeListBox& /*rDev*/, vcl::RenderContext& rRenderContext,
                              const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    ModuleUserData_Impl* pData = static_cast<ModuleUserData_Impl*>(rEntry.GetUserData());
    Point aPos(rPos);
    rRenderContext.Push(PushFlags::FONT);
    if (pData->IsParent())
    {
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetWeight(WEIGHT_BOLD);
        rRenderContext.SetFont(aFont);
        aPos.X() = 0;
    }
    else
        aPos.X() += 5;
    rRenderContext.DrawText(aPos, GetText());
    rRenderContext.Pop();
}

/*--------------------------------------------------
    Entry IDs for options listbox of dialog
--------------------------------------------------*/

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
    EID_HYPH_SPECIAL
};

//! this array must have an entry for every value of EID_OPTIONS.
//  It is used to get the respective property name.
static const char * aEidToPropName[] =
{
    UPN_IS_SPELL_AUTO,              // EID_SPELL_AUTO
    UPN_IS_GRAMMAR_AUTO,            // EID_GRAMMAR_AUTO
    UPN_IS_SPELL_UPPER_CASE,        // EID_CAPITAL_WORDS
    UPN_IS_SPELL_WITH_DIGITS,       // EID_WORDS_WITH_DIGITS
    UPN_IS_SPELL_SPECIAL,           // EID_SPELL_SPECIAL
    UPN_HYPH_MIN_WORD_LENGTH,       // EID_NUM_MIN_WORDLEN,
    UPN_HYPH_MIN_LEADING,           // EID_NUM_PRE_BREAK
    UPN_HYPH_MIN_TRAILING,          // EID_NUM_POST_BREAK
    UPN_IS_HYPH_AUTO,               // EID_HYPH_AUTO
    UPN_IS_HYPH_SPECIAL             // EID_HYPH_SPECIAL
};

static inline OUString lcl_GetPropertyName( EID_OPTIONS eEntryId )
{
    DBG_ASSERT( (unsigned int) eEntryId < SAL_N_ELEMENTS(aEidToPropName), "index out of range" );
    return OUString::createFromAscii( aEidToPropName[ (int) eEntryId ] );
}

class OptionsBreakSet : public ModalDialog
{
    VclPtr<VclFrame>       m_pBeforeFrame;
    VclPtr<VclFrame>       m_pAfterFrame;
    VclPtr<VclFrame>       m_pMinimalFrame;
    VclPtr<NumericField>   m_pBreakNF;

public:
    OptionsBreakSet(vcl::Window* pParent, sal_uInt16 nRID)
        : ModalDialog(pParent, "BreakNumberOption",
            "cui/ui/breaknumberoption.ui")
        , m_pBreakNF(NULL)
    {
        get(m_pBeforeFrame, "beforeframe");
        get(m_pAfterFrame, "afterframe");
        get(m_pMinimalFrame, "miniframe");

        assert(EID_NUM_PRE_BREAK == nRID ||
               EID_NUM_POST_BREAK == nRID ||
               EID_NUM_MIN_WORDLEN == nRID); //unexpected ID

        if (nRID == EID_NUM_PRE_BREAK)
        {
            m_pBeforeFrame->Show();
            get(m_pBreakNF, "beforebreak");
        }
        else if(nRID == EID_NUM_POST_BREAK)
        {
            m_pAfterFrame->Show();
            get(m_pBreakNF, "afterbreak");
        }
        else if(nRID == EID_NUM_MIN_WORDLEN)
        {
            m_pMinimalFrame->Show();
            get(m_pBreakNF, "wordlength");
        }
    }
    virtual ~OptionsBreakSet() { disposeOnce(); }
    virtual void dispose() override
    {
        m_pBeforeFrame.clear();
        m_pAfterFrame.clear();
        m_pMinimalFrame.clear();
        m_pBreakNF.clear();
        ModalDialog::dispose();
    }

    NumericField&   GetNumericFld()
    {
        return *m_pBreakNF;
    }
};

// class OptionsUserData -------------------------------------------------

class OptionsUserData
{
    sal_uLong   nVal;

    void    SetModified();

public:
    explicit OptionsUserData( sal_uLong nUserData ) : nVal( nUserData ) {}
    OptionsUserData( sal_uInt16 nEID,
                     bool bHasNV, sal_uInt16 nNumVal,
                     bool bCheckable, bool bChecked );

    sal_uLong   GetUserData() const         { return nVal; }
    sal_uInt16  GetEntryId() const          { return (sal_uInt16)(nVal >> 16); }
    bool        HasNumericValue() const     { return (bool)((nVal >> 10) & 0x01); }
    sal_uInt16  GetNumericValue() const     { return (sal_uInt16)(nVal & 0xFF); }
    bool        IsCheckable() const         { return (bool)((nVal >> 9) & 0x01); }
    bool        IsModified() const          { return (bool)((nVal >> 11) & 0x01); }

    void        SetNumericValue( sal_uInt8 nNumVal );
};

OptionsUserData::OptionsUserData( sal_uInt16 nEID,
        bool bHasNV, sal_uInt16 nNumVal,
        bool bCheckable, bool bChecked )
{
    DBG_ASSERT( nEID < 65000, "Entry Id out of range" );
    DBG_ASSERT( nNumVal < 256, "value out of range" );
    nVal =  ((sal_uLong)(0xFFFF & nEID)         << 16) |
            ((sal_uLong)(bHasNV ? 1 : 0)        << 10) |
            ((sal_uLong)(bCheckable ? 1 : 0)    <<  9) |
            ((sal_uLong)(bChecked ? 1 : 0)      <<  8) |
            ((sal_uLong)(0xFF & nNumVal));
}

void OptionsUserData::SetNumericValue( sal_uInt8 nNumVal )
{
    if (HasNumericValue()  &&  (GetNumericValue() != nNumVal))
    {
        nVal &= 0xffffff00;
        nVal |= (nNumVal);
        SetModified();
    }
}

void OptionsUserData::SetModified()
{
    nVal |=  (sal_uLong)1 << 11;
}

// class BrwString_Impl -------------------------------------------------

class BrwString_Impl : public SvLBoxString
{
public:

    BrwString_Impl( SvTreeListEntry* pEntry, sal_uInt16 nFlags,
        const OUString& rStr ) : SvLBoxString( pEntry, nFlags, rStr ) {}

    virtual void Paint(const Point& rPos, SvTreeListBox& rOutDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};

void BrwString_Impl::Paint(const Point& rPos, SvTreeListBox& /*rDev*/, vcl::RenderContext& rRenderContext,
                           const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    Point aPos(rPos);
    aPos.X() += 20;
    rRenderContext.DrawText(aPos, GetText());
    if (rEntry.GetUserData())
    {
        Point aNewPos(aPos);
        aNewPos.X() += rRenderContext.GetTextWidth(GetText());
        rRenderContext.Push(PushFlags::FONT);
        vcl::Font aFont(rRenderContext.GetFont());
        aFont.SetWeight(WEIGHT_BOLD);

        //??? convert the lower byte from the user date into a string
        OptionsUserData aData(reinterpret_cast<sal_uLong>(rEntry.GetUserData()));
        if (aData.HasNumericValue())
        {
            OUStringBuffer sTxt;
            sTxt.append(' ').append(static_cast<sal_Int32>(aData.GetNumericValue()));
            rRenderContext.SetFont(aFont);
            rRenderContext.DrawText(aNewPos, sTxt.makeStringAndClear());
        }

        rRenderContext.Pop();
    }
}

// ServiceInfo_Impl ----------------------------------------------------

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

typedef std::vector< ServiceInfo_Impl >                             ServiceInfoArr;
typedef std::map< sal_Int16 /*LanguageType*/, Sequence< OUString > >    LangImplNameTable;


// SvxLinguData_Impl ----------------------------------------------------

class SvxLinguData_Impl
{
    //contains services and implementation names sorted by implementation names
    ServiceInfoArr                      aDisplayServiceArr;
    sal_uLong                               nDisplayServices;

    Sequence< Locale >                  aAllServiceLocales;
    LangImplNameTable                   aCfgSpellTable;
    LangImplNameTable                   aCfgHyphTable;
    LangImplNameTable                   aCfgThesTable;
    LangImplNameTable                   aCfgGrammarTable;
    uno::Reference< XLinguServiceManager2 >  xLinguSrvcMgr;


    static bool AddRemove( Sequence< OUString > &rConfigured,
                           const OUString &rImplName, bool bAdd );

public:
    SvxLinguData_Impl();
    SvxLinguData_Impl( const SvxLinguData_Impl &rData );
    ~SvxLinguData_Impl();

    SvxLinguData_Impl & operator = (const SvxLinguData_Impl &rData);

    uno::Reference<XLinguServiceManager2> &   GetManager() { return xLinguSrvcMgr; }

    void SetChecked( const Sequence< OUString > &rConfiguredServices );
    void Reconfigure( const OUString &rDisplayName, bool bEnable );

    const Sequence<Locale> &    GetAllSupportedLocales() const { return aAllServiceLocales; }

    LangImplNameTable &         GetSpellTable()         { return aCfgSpellTable; }
    LangImplNameTable &         GetHyphTable()          { return aCfgHyphTable; }
    LangImplNameTable &         GetThesTable()          { return aCfgThesTable; }
    LangImplNameTable &         GetGrammarTable()       { return aCfgGrammarTable; }

    ServiceInfoArr &            GetDisplayServiceArray()        { return aDisplayServiceArr; }

    const sal_uLong &   GetDisplayServiceCount() const          { return nDisplayServices; }
    void            SetDisplayServiceCount( sal_uLong nVal )    { nDisplayServices = nVal; }

    // returns the list of service implementation names for the specified
    // language and service (TYPE_SPELL, TYPE_HYPH, TYPE_THES) sorted in
    // the proper order for the SvxEditModulesDlg (the ones from the
    // configuration (keeping that order!) first and then the other ones.
    // I.e. the ones available but not configured in arbitrary order).
    // They available ones may contain names that do not(!) support that
    // language.
    Sequence< OUString > GetSortedImplNames( sal_Int16 nLang, sal_uInt8 nType );

    ServiceInfo_Impl * GetInfoByImplName( const OUString &rSvcImplName );
};


static sal_Int32 lcl_SeqGetIndex( const Sequence< OUString > &rSeq, const OUString &rTxt )
{
    sal_Int32 nRes = -1;
    sal_Int32 nLen = rSeq.getLength();
    const OUString *pString = rSeq.getConstArray();
    for (sal_Int32 i = 0;  i < nLen  &&  nRes == -1;  ++i)
    {
        if (pString[i] == rTxt)
            nRes = i;
    }
    return nRes;
}


Sequence< OUString > SvxLinguData_Impl::GetSortedImplNames( sal_Int16 nLang, sal_uInt8 nType )
{
    LangImplNameTable *pTable = 0;
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
    DBG_ASSERT( (sal_Int32) nDisplayServices >= nIdx, "size mismatch" );
    aRes.realloc( nDisplayServices );
    OUString *pRes = aRes.getArray();

    // add not configured services
    for (sal_Int32 i = 0;  i < (sal_Int32) nDisplayServices;  ++i)
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

        if (!aImplName.isEmpty()  &&  (lcl_SeqGetIndex( aRes, aImplName) == -1))    // name not yet added
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


ServiceInfo_Impl * SvxLinguData_Impl::GetInfoByImplName( const OUString &rSvcImplName )
{
    ServiceInfo_Impl* pInfo = 0;
    for (sal_uLong i = 0;  i < nDisplayServices  &&  !pInfo;  ++i)
    {
        ServiceInfo_Impl &rTmp = aDisplayServiceArr[ i ];
        if (rTmp.sSpellImplName == rSvcImplName ||
            rTmp.sHyphImplName  == rSvcImplName ||
            rTmp.sThesImplName  == rSvcImplName ||
            rTmp.sGrammarImplName == rSvcImplName)
            pInfo = &rTmp;
    }
    return pInfo;
}




static void lcl_MergeLocales(Sequence< Locale >& aAllLocales, const Sequence< Locale >& rAdd)
{
    const Locale* pAdd = rAdd.getConstArray();
    Sequence<Locale> aLocToAdd(rAdd.getLength());
    const Locale* pAllLocales = aAllLocales.getConstArray();
    Locale* pLocToAdd = aLocToAdd.getArray();
    sal_Int32 nFound = 0;
    sal_Int32 i;
    for(i = 0; i < rAdd.getLength(); i++)
    {
        bool bFound = false;
        for(sal_Int32 j = 0; j < aAllLocales.getLength() && !bFound; j++)
        {
            bFound = pAdd[i].Language == pAllLocales[j].Language &&
                pAdd[i].Country == pAllLocales[j].Country &&
                pAdd[i].Variant == pAllLocales[j].Variant;
        }
        if(!bFound)
        {
            pLocToAdd[nFound++] = pAdd[i];
        }
    }
    sal_Int32 nLength = aAllLocales.getLength();
    aAllLocales.realloc( nLength + nFound);
    Locale* pAllLocales2 = aAllLocales.getArray();
    for(i = 0; i < nFound; i++)
        pAllLocales2[nLength++] = pLocToAdd[i];
}

static void lcl_MergeDisplayArray(
        SvxLinguData_Impl &rData,
        const ServiceInfo_Impl &rToAdd )
{
    sal_uLong nCnt = 0;

    ServiceInfoArr &rSvcInfoArr = rData.GetDisplayServiceArray();
    sal_uLong nEntries = rData.GetDisplayServiceCount();

    for (sal_uLong i = 0;  i < nEntries;  ++i)
    {
        ServiceInfo_Impl* pEntry = &rSvcInfoArr[i];
        if (pEntry  &&  pEntry->sDisplayName == rToAdd.sDisplayName)
        {
            if(rToAdd.xSpell.is())
            {
                DBG_ASSERT( !pEntry->xSpell.is() &&
                            pEntry->sSpellImplName.isEmpty(),
                            "merge conflict" );
                pEntry->sSpellImplName = rToAdd.sSpellImplName;
                pEntry->xSpell = rToAdd.xSpell;
            }
            if(rToAdd.xGrammar.is())
            {
                DBG_ASSERT( !pEntry->xGrammar.is() &&
                            pEntry->sGrammarImplName.isEmpty(),
                            "merge conflict" );
                pEntry->sGrammarImplName = rToAdd.sGrammarImplName;
                pEntry->xGrammar = rToAdd.xGrammar;
            }
            if(rToAdd.xHyph.is())
            {
                DBG_ASSERT( !pEntry->xHyph.is() &&
                            pEntry->sHyphImplName.isEmpty(),
                            "merge conflict" );
                pEntry->sHyphImplName = rToAdd.sHyphImplName;
                pEntry->xHyph = rToAdd.xHyph;
            }
            if(rToAdd.xThes.is())
            {
                DBG_ASSERT( !pEntry->xThes.is() &&
                            pEntry->sThesImplName.isEmpty(),
                            "merge conflict" );
                pEntry->sThesImplName = rToAdd.sThesImplName;
                pEntry->xThes = rToAdd.xThes;
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

    const Locale& rCurrentLocale = Application::GetSettings().GetLanguageTag().getLocale();
    Sequence<Any> aArgs(2);//second arguments has to be empty!
    aArgs.getArray()[0] <<= SvxGetLinguPropertySet();

    //read spell checker
    Sequence< OUString > aSpellNames = xLinguSrvcMgr->getAvailableServices(
                    cSpell,    Locale() );
    const OUString* pSpellNames = aSpellNames.getConstArray();

    sal_Int32 nIdx;
    for(nIdx = 0; nIdx < aSpellNames.getLength(); nIdx++)
    {
        ServiceInfo_Impl aInfo;
        aInfo.sSpellImplName = pSpellNames[nIdx];
        aInfo.xSpell = uno::Reference<XSpellChecker>(
                        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aInfo.sSpellImplName, aArgs, xContext), UNO_QUERY);

        uno::Reference<XServiceDisplayName> xDispName(aInfo.xSpell, UNO_QUERY);
        if(xDispName.is())
            aInfo.sDisplayName = xDispName->getServiceDisplayName( rCurrentLocale );

        const Sequence< Locale > aLocales( aInfo.xSpell->getLocales() );
        //! suppress display of entries with no supported languages (see feature 110994)
        if (aLocales.getLength())
        {
            lcl_MergeLocales( aAllServiceLocales, aLocales );
            lcl_MergeDisplayArray( *this, aInfo );
        }
    }

    //read grammar checker
    Sequence< OUString > aGrammarNames = xLinguSrvcMgr->getAvailableServices(
                    cGrammar, Locale() );
    const OUString* pGrammarNames = aGrammarNames.getConstArray();
    for(nIdx = 0; nIdx < aGrammarNames.getLength(); nIdx++)
    {
        ServiceInfo_Impl aInfo;
        aInfo.sGrammarImplName = pGrammarNames[nIdx];
        aInfo.xGrammar = uno::Reference<XProofreader>(
                        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aInfo.sGrammarImplName, aArgs, xContext), UNO_QUERY);

        uno::Reference<XServiceDisplayName> xDispName(aInfo.xGrammar, UNO_QUERY);
        if(xDispName.is())
            aInfo.sDisplayName = xDispName->getServiceDisplayName( rCurrentLocale );

        const Sequence< Locale > aLocales( aInfo.xGrammar->getLocales() );
        //! suppress display of entries with no supported languages (see feature 110994)
        if (aLocales.getLength())
        {
            lcl_MergeLocales( aAllServiceLocales, aLocales );
            lcl_MergeDisplayArray( *this, aInfo );
        }
    }

    //read hyphenator
    Sequence< OUString > aHyphNames = xLinguSrvcMgr->getAvailableServices(
                    cHyph, Locale() );
    const OUString* pHyphNames = aHyphNames.getConstArray();
    for(nIdx = 0; nIdx < aHyphNames.getLength(); nIdx++)
    {
        ServiceInfo_Impl aInfo;
        aInfo.sHyphImplName = pHyphNames[nIdx];
        aInfo.xHyph = uno::Reference<XHyphenator>(
                        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aInfo.sHyphImplName, aArgs, xContext), UNO_QUERY);

        uno::Reference<XServiceDisplayName> xDispName(aInfo.xHyph, UNO_QUERY);
        if(xDispName.is())
            aInfo.sDisplayName = xDispName->getServiceDisplayName( rCurrentLocale );

        const Sequence< Locale > aLocales( aInfo.xHyph->getLocales() );
        //! suppress display of entries with no supported languages (see feature 110994)
        if (aLocales.getLength())
        {
            lcl_MergeLocales( aAllServiceLocales, aLocales );
            lcl_MergeDisplayArray( *this, aInfo );
        }
    }

    //read thesauri
    Sequence< OUString > aThesNames = xLinguSrvcMgr->getAvailableServices(
                    cThes,     Locale() );
    const OUString* pThesNames = aThesNames.getConstArray();
    for(nIdx = 0; nIdx < aThesNames.getLength(); nIdx++)
    {
        ServiceInfo_Impl aInfo;
        aInfo.sThesImplName = pThesNames[nIdx];
        aInfo.xThes = uno::Reference<XThesaurus>(
                        xContext->getServiceManager()->createInstanceWithArgumentsAndContext(aInfo.sThesImplName, aArgs, xContext), UNO_QUERY);

        uno::Reference<XServiceDisplayName> xDispName(aInfo.xThes, UNO_QUERY);
        if(xDispName.is())
            aInfo.sDisplayName = xDispName->getServiceDisplayName( rCurrentLocale );

        const Sequence< Locale > aLocales( aInfo.xThes->getLocales() );
        //! suppress display of entries with no supported languages (see feature 110994)
        if (aLocales.getLength())
        {
            lcl_MergeLocales( aAllServiceLocales, aLocales );
            lcl_MergeDisplayArray( *this, aInfo );
        }
    }

    Sequence< OUString > aCfgSvcs;
    const Locale* pAllLocales = aAllServiceLocales.getConstArray();
    for(sal_Int32 nLocale = 0; nLocale < aAllServiceLocales.getLength(); nLocale++)
    {
        sal_Int16 nLang = LanguageTag::convertToLanguageType( pAllLocales[nLocale] );

        aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(cSpell, pAllLocales[nLocale]);
        SetChecked( aCfgSvcs );
        if (aCfgSvcs.getLength())
            aCfgSpellTable[ nLang ] = aCfgSvcs;

        aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(cGrammar, pAllLocales[nLocale]);
        SetChecked( aCfgSvcs );
        if (aCfgSvcs.getLength())
            aCfgGrammarTable[ nLang ] = aCfgSvcs;

        aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(cHyph, pAllLocales[nLocale]);
        SetChecked( aCfgSvcs );
        if (aCfgSvcs.getLength())
            aCfgHyphTable[ nLang ] = aCfgSvcs;

        aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(cThes, pAllLocales[nLocale]);
        SetChecked( aCfgSvcs );
        if (aCfgSvcs.getLength())
            aCfgThesTable[ nLang ] = aCfgSvcs;
    }
}

SvxLinguData_Impl::SvxLinguData_Impl( const SvxLinguData_Impl &rData ) :
    aDisplayServiceArr  (rData.aDisplayServiceArr),
    nDisplayServices    (rData.nDisplayServices),
    aAllServiceLocales  (rData.aAllServiceLocales),
    aCfgSpellTable      (rData.aCfgSpellTable),
    aCfgHyphTable       (rData.aCfgHyphTable),
    aCfgThesTable       (rData.aCfgThesTable),
    aCfgGrammarTable    (rData.aCfgGrammarTable),
    xLinguSrvcMgr       (rData.xLinguSrvcMgr)
{
}

SvxLinguData_Impl & SvxLinguData_Impl::operator = (const SvxLinguData_Impl &rData)
{
    xLinguSrvcMgr       = rData.xLinguSrvcMgr;
    aAllServiceLocales  = rData.aAllServiceLocales;
    aCfgSpellTable      = rData.aCfgSpellTable;
    aCfgHyphTable       = rData.aCfgHyphTable;
    aCfgThesTable       = rData.aCfgThesTable;
    aCfgGrammarTable    = rData.aCfgGrammarTable;
    aDisplayServiceArr  = rData.aDisplayServiceArr;
    nDisplayServices    = rData.nDisplayServices;
    return *this;
}

SvxLinguData_Impl::~SvxLinguData_Impl()
{
}

void SvxLinguData_Impl::SetChecked(const Sequence<OUString>& rConfiguredServices)
{
    const OUString* pConfiguredServices = rConfiguredServices.getConstArray();
    for(sal_Int32 n = 0; n < rConfiguredServices.getLength(); n++)
    {
        for (sal_uLong i = 0;  i < nDisplayServices;  ++i)
        {
            ServiceInfo_Impl* pEntry = &aDisplayServiceArr[i];
            if (pEntry  &&  !pEntry->bConfigured)
            {
                const OUString &rSrvcImplName = pConfiguredServices[n];
                if (!rSrvcImplName.isEmpty()  &&
                    (pEntry->sSpellImplName == rSrvcImplName  ||
                        pEntry->sGrammarImplName  == rSrvcImplName  ||
                        pEntry->sHyphImplName  == rSrvcImplName  ||
                        pEntry->sThesImplName  == rSrvcImplName))
                {
                    pEntry->bConfigured = true;
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


void SvxLinguData_Impl::Reconfigure( const OUString &rDisplayName, bool bEnable )
{
    DBG_ASSERT( !rDisplayName.isEmpty(), "empty DisplayName" );

    ServiceInfo_Impl *pInfo = 0;
    ServiceInfo_Impl *pTmp  = 0;
    for (sal_uLong i = 0;  i < nDisplayServices;  ++i)
    {
        pTmp = &aDisplayServiceArr[i];
        if (pTmp  &&  pTmp->sDisplayName == rDisplayName)
        {
            pInfo = pTmp;
            break;
        }
    }
    DBG_ASSERT( pInfo, "DisplayName entry not found" );
    if (pInfo)
    {
        pInfo->bConfigured = bEnable;

        Sequence< Locale > aLocales;
        const Locale *pLocale = 0;
        sal_Int32 nLocales = 0;
        sal_Int32 i;

        // update configured spellchecker entries
        if (pInfo->xSpell.is())
        {
            aLocales = pInfo->xSpell->getLocales();
            pLocale = aLocales.getConstArray();
            nLocales = aLocales.getLength();
            for (i = 0;  i < nLocales;  ++i)
            {
                sal_Int16 nLang = LanguageTag::convertToLanguageType( pLocale[i] );
                if (!aCfgSpellTable.count( nLang ) && bEnable)
                    aCfgSpellTable[ nLang ] = Sequence< OUString >();
                if (aCfgSpellTable.count( nLang ))
                    AddRemove( aCfgSpellTable[ nLang ], pInfo->sSpellImplName, bEnable );
            }
        }

        // update configured grammar checker entries
        if (pInfo->xGrammar.is())
        {
            aLocales = pInfo->xGrammar->getLocales();
            pLocale = aLocales.getConstArray();
            nLocales = aLocales.getLength();
            for (i = 0;  i < nLocales;  ++i)
            {
                sal_Int16 nLang = LanguageTag::convertToLanguageType( pLocale[i] );
                if (!aCfgGrammarTable.count( nLang ) && bEnable)
                    aCfgGrammarTable[ nLang ] = Sequence< OUString >();
                if (aCfgGrammarTable.count( nLang ))
                    AddRemove( aCfgGrammarTable[ nLang ], pInfo->sGrammarImplName, bEnable );
            }
        }

        // update configured hyphenator entries
        if (pInfo->xHyph.is())
        {
            aLocales = pInfo->xHyph->getLocales();
            pLocale = aLocales.getConstArray();
            nLocales = aLocales.getLength();
            for (i = 0;  i < nLocales;  ++i)
            {
                sal_Int16 nLang = LanguageTag::convertToLanguageType( pLocale[i] );
                if (!aCfgHyphTable.count( nLang ) && bEnable)
                    aCfgHyphTable[ nLang ] = Sequence< OUString >();
                if (aCfgHyphTable.count( nLang ))
                    AddRemove( aCfgHyphTable[ nLang ], pInfo->sHyphImplName, bEnable );
            }
        }

        // update configured spellchecker entries
        if (pInfo->xThes.is())
        {
            aLocales = pInfo->xThes->getLocales();
            pLocale = aLocales.getConstArray();
            nLocales = aLocales.getLength();
            for (i = 0;  i < nLocales;  ++i)
            {
                sal_Int16 nLang = LanguageTag::convertToLanguageType( pLocale[i] );
                if (!aCfgThesTable.count( nLang ) && bEnable)
                    aCfgThesTable[ nLang ] = Sequence< OUString >();
                if (aCfgThesTable.count( nLang ))
                    AddRemove( aCfgThesTable[ nLang ], pInfo->sThesImplName, bEnable );
            }
        }
    }
}


// class SvxLinguTabPage -------------------------------------------------

SvxLinguTabPage::SvxLinguTabPage( vcl::Window* pParent, const SfxItemSet& rSet ) :
    SfxTabPage(pParent, "OptLinguPage", "cui/ui/optlingupage.ui", &rSet),

    sCapitalWords   (CUI_RES(RID_SVXSTR_CAPITAL_WORDS)),
    sWordsWithDigits(CUI_RES(RID_SVXSTR_WORDS_WITH_DIGITS)),
    sSpellSpecial   (CUI_RES(RID_SVXSTR_SPELL_SPECIAL)),
    sSpellAuto      (CUI_RES(RID_SVXSTR_SPELL_AUTO)),
    sGrammarAuto    (CUI_RES(RID_SVXSTR_GRAMMAR_AUTO)),
    sNumMinWordlen  (CUI_RES(RID_SVXSTR_NUM_MIN_WORDLEN)),
    sNumPreBreak    (CUI_RES(RID_SVXSTR_NUM_PRE_BREAK)),
    sNumPostBreak   (CUI_RES(RID_SVXSTR_NUM_POST_BREAK)),
    sHyphAuto       (CUI_RES(RID_SVXSTR_HYPH_AUTO)),
    sHyphSpecial    (CUI_RES(RID_SVXSTR_HYPH_SPECIAL)),

    pLinguData(NULL)
{
    get(m_pLinguModulesFT, "lingumodulesft");
    get(m_pLinguModulesCLB, "lingumodules");
    get(m_pLinguModulesEditPB, "lingumodulesedit");
    get(m_pLinguDicsFT, "lingudictsft");
    get(m_pLinguDicsCLB, "lingudicts");
    get(m_pLinguDicsNewPB, "lingudictsnew");
    get(m_pLinguDicsEditPB, "lingudictsedit");
    get(m_pLinguDicsDelPB, "lingudictsdelete");
    get(m_pLinguOptionsCLB, "linguoptions");
    get(m_pLinguOptionsEditPB, "linguoptionsedit");
    get(m_pMoreDictsLink, "moredictslink");

    m_pLinguModulesCLB->set_height_request(m_pLinguModulesCLB->GetTextHeight() * 3);
    m_pLinguDicsCLB->set_height_request(m_pLinguDicsCLB->GetTextHeight() * 5);
    m_pLinguOptionsCLB->set_height_request(m_pLinguOptionsCLB->GetTextHeight() * 5);

    pCheckButtonData = NULL;

    m_pLinguModulesCLB->SetStyle( m_pLinguModulesCLB->GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    m_pLinguModulesCLB->SetHighlightRange();
    m_pLinguModulesCLB->SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    m_pLinguModulesCLB->SetDoubleClickHdl(LINK(this, SvxLinguTabPage, BoxDoubleClickHdl_Impl));
    m_pLinguModulesCLB->SetCheckButtonHdl(LINK(this, SvxLinguTabPage, BoxCheckButtonHdl_Impl));

    m_pLinguModulesEditPB->SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    m_pLinguOptionsEditPB->SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));

    m_pLinguDicsCLB->SetStyle( m_pLinguDicsCLB->GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    m_pLinguDicsCLB->SetHighlightRange();
    m_pLinguDicsCLB->SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    m_pLinguDicsCLB->SetCheckButtonHdl(LINK(this, SvxLinguTabPage, BoxCheckButtonHdl_Impl));

    m_pLinguDicsNewPB->SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    m_pLinguDicsEditPB->SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    m_pLinguDicsDelPB->SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));

    m_pLinguOptionsCLB->SetStyle( m_pLinguOptionsCLB->GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    m_pLinguOptionsCLB->SetHighlightRange();
    m_pLinguOptionsCLB->SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    m_pLinguOptionsCLB->SetDoubleClickHdl(LINK(this, SvxLinguTabPage, BoxDoubleClickHdl_Impl));

    if ( SvtExtendedSecurityOptions().GetOpenHyperlinkMode()
            != SvtExtendedSecurityOptions::OPEN_NEVER )
    {
        m_pMoreDictsLink->SetClickHdl( LINK( this, SvxLinguTabPage, OpenURLHdl_Impl ) );
    }
    else
        m_pMoreDictsLink->Hide();

    OUString sAccessibleNameModuleEdit(CUI_RES(RID_SVXSTR_LINGU_MODULES_EDIT));
    OUString sAccessibleNameDicsEdit  (CUI_RES(RID_SVXSTR_LINGU_DICS_EDIT_DIC));
    OUString sAccessibleNameOptionEdit(CUI_RES(RID_SVXSTR_LINGU_OPTIONS_EDIT));

    m_pLinguModulesEditPB->SetAccessibleName(sAccessibleNameModuleEdit);
    m_pLinguDicsEditPB->SetAccessibleName(sAccessibleNameDicsEdit);
    m_pLinguOptionsEditPB->SetAccessibleName(sAccessibleNameOptionEdit);

    xProp = SvxGetLinguPropertySet();
    xDicList = uno::Reference< XDictionaryList >( SvxGetDictionaryList(), UNO_QUERY );
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
        m_pLinguDicsFT->Disable();
        m_pLinguDicsCLB->Disable();
        m_pLinguDicsNewPB->Disable();
        m_pLinguDicsEditPB->Disable();
        m_pLinguDicsDelPB->Disable();
    }
}

SvxLinguTabPage::~SvxLinguTabPage()
{
    disposeOnce();
}

void SvxLinguTabPage::dispose()
{
    delete pLinguData;
    pLinguData = NULL;
    m_pLinguModulesFT.clear();
    m_pLinguModulesCLB.clear();
    m_pLinguModulesEditPB.clear();
    m_pLinguDicsFT.clear();
    m_pLinguDicsCLB.clear();
    m_pLinguDicsNewPB.clear();
    m_pLinguDicsEditPB.clear();
    m_pLinguDicsDelPB.clear();
    m_pLinguOptionsCLB.clear();
    m_pLinguOptionsEditPB.clear();
    m_pMoreDictsLink.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> SvxLinguTabPage::Create( vcl::Window* pParent,
                                            const SfxItemSet* rAttrSet )
{
    return VclPtr<SvxLinguTabPage>::Create( pParent, *rAttrSet );
}

bool SvxLinguTabPage::FillItemSet( SfxItemSet* rCoreSet )
{
    bool bModified = true; // !!!!

    // if not HideGroups was called with GROUP_MODULES...
    if (m_pLinguModulesCLB->IsVisible())
    {
        DBG_ASSERT( pLinguData, "pLinguData not yet initialized" );
        if (!pLinguData)
            pLinguData = new SvxLinguData_Impl;

        LangImplNameTable::const_iterator aIt;

        // update spellchecker configuration entries
        const LangImplNameTable *pTable = &pLinguData->GetSpellTable();
        for (aIt = pTable->begin();  aIt != pTable->end();  ++aIt)
        {
            sal_Int16 nLang = aIt->first;
            const Sequence< OUString > aImplNames( aIt->second );
            uno::Reference< XLinguServiceManager2 > xMgr( pLinguData->GetManager() );
            Locale aLocale( LanguageTag::convertToLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cSpell, aLocale, aImplNames );
        }

        // update grammar checker configuration entries
        pTable = &pLinguData->GetGrammarTable();
        for (aIt = pTable->begin();  aIt != pTable->end();  ++aIt)
        {
            sal_Int16 nLang = aIt->first;
            const Sequence< OUString > aImplNames( aIt->second );
            uno::Reference< XLinguServiceManager2 > xMgr( pLinguData->GetManager() );
            Locale aLocale( LanguageTag::convertToLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cGrammar, aLocale, aImplNames );
        }

        // update hyphenator configuration entries
        pTable = &pLinguData->GetHyphTable();
        for (aIt = pTable->begin();  aIt != pTable->end();  ++aIt)
        {
            sal_Int16 nLang = aIt->first;
            const Sequence< OUString > aImplNames( aIt->second );
            uno::Reference< XLinguServiceManager2 > xMgr( pLinguData->GetManager() );
            Locale aLocale( LanguageTag::convertToLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cHyph, aLocale, aImplNames );
        }

        // update thesaurus configuration entries
        pTable = &pLinguData->GetThesTable();
        for (aIt = pTable->begin();  aIt != pTable->end();  ++aIt)
        {
            sal_Int16 nLang = aIt->first;
            const Sequence< OUString > aImplNames( aIt->second );
            uno::Reference< XLinguServiceManager2 > xMgr( pLinguData->GetManager() );
            Locale aLocale( LanguageTag::convertToLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cThes, aLocale, aImplNames );
        }
    }



    // activate dictionaries according to checkbox state

    Sequence< OUString > aActiveDics;
    sal_Int32 nActiveDics = 0;
    sal_uLong nEntries = m_pLinguDicsCLB->GetEntryCount();
    for (sal_uLong i = 0;  i < nEntries;  ++i)
    {
        sal_Int32 nDics = aDics.getLength();

        aActiveDics.realloc( nDics );
        OUString *pActiveDic = aActiveDics.getArray();

        SvTreeListEntry *pEntry = m_pLinguDicsCLB->GetEntry( i );
        if (pEntry)
        {
            DicUserData aData( reinterpret_cast<sal_uLong>(pEntry->GetUserData()) );
            if (aData.GetEntryId() < nDics)
            {
                bool bChecked = m_pLinguDicsCLB->IsChecked( i );
                uno::Reference< XDictionary > xDic( aDics.getConstArray()[ i ] );
                if (xDic.is())
                {
                    if (SvxGetIgnoreAllList() == xDic)
                        bChecked = true;
                    xDic->setActive( bChecked );

                    if (bChecked)
                    {
                        OUString aDicName( xDic->getName() );
                        pActiveDic[ nActiveDics++ ] = aDicName;
                    }
                }
            }
        }
    }

    aActiveDics.realloc( nActiveDics );
    Any aTmp;
    aTmp <<= aActiveDics;
    SvtLinguConfig aLngCfg;
    aLngCfg.SetProperty( UPH_ACTIVE_DICTIONARIES, aTmp );


    nEntries = m_pLinguOptionsCLB->GetEntryCount();
    for (sal_uLong j = 0;  j < nEntries;  ++j)
    {
        SvTreeListEntry *pEntry = m_pLinguOptionsCLB->GetEntry( j );

        OptionsUserData aData( reinterpret_cast<sal_uLong>(pEntry->GetUserData()) );
        OUString aPropName( lcl_GetPropertyName( (EID_OPTIONS) aData.GetEntryId() ) );

        Any aAny;
        if (aData.IsCheckable())
        {
            bool bChecked = m_pLinguOptionsCLB->IsChecked( j );
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

    SvTreeListEntry *pPreBreakEntry  = m_pLinguOptionsCLB->GetEntry( (sal_uLong) EID_NUM_PRE_BREAK );
    SvTreeListEntry *pPostBreakEntry = m_pLinguOptionsCLB->GetEntry( (sal_uLong) EID_NUM_POST_BREAK );
    DBG_ASSERT( pPreBreakEntry, "NULL Pointer" );
    DBG_ASSERT( pPostBreakEntry, "NULL Pointer" );
    if (pPreBreakEntry && pPostBreakEntry)
    {
        OptionsUserData aPreBreakData( reinterpret_cast<sal_uLong>(pPreBreakEntry->GetUserData()) );
        OptionsUserData aPostBreakData( reinterpret_cast<sal_uLong>(pPostBreakEntry->GetUserData()) );
        if ( aPreBreakData.IsModified() || aPostBreakData.IsModified() )
        {
            SfxHyphenRegionItem aHyp( GetWhich( SID_ATTR_HYPHENREGION ) );
            aHyp.GetMinLead()  = (sal_uInt8) aPreBreakData.GetNumericValue();
            aHyp.GetMinTrail() = (sal_uInt8) aPostBreakData.GetNumericValue();
            rCoreSet->Put( aHyp );
        }
    }


    // automatic spell checking
    bool bNewAutoCheck = m_pLinguOptionsCLB->IsChecked( (sal_uLong) EID_SPELL_AUTO );
    const SfxPoolItem* pOld = GetOldItem( *rCoreSet, SID_AUTOSPELL_CHECK );
    if ( !pOld || static_cast<const SfxBoolItem*>(pOld)->GetValue() != bNewAutoCheck )
    {
        rCoreSet->Put( SfxBoolItem( GetWhich( SID_AUTOSPELL_CHECK ),
                                bNewAutoCheck ) );
        bModified = true;
    }

    return bModified;
}

sal_uLong SvxLinguTabPage::GetDicUserData( const uno::Reference< XDictionary > &rxDic, sal_uInt16 nIdx )
{
    sal_uLong nRes = 0;
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
    m_pLinguDicsCLB->SetUpdateMode(false);

    OUString aTxt( ::GetDicInfoStr( rxDic->getName(),
                        LanguageTag( rxDic->getLocale() ).getLanguageType(),
                        DictionaryType_NEGATIVE == rxDic->getDictionaryType() ) );
    m_pLinguDicsCLB->InsertEntry( aTxt, TREELIST_APPEND );  // append at end
    SvTreeListEntry* pEntry = m_pLinguDicsCLB->GetEntry( m_pLinguDicsCLB->GetEntryCount() - 1 );
    DBG_ASSERT( pEntry, "failed to add entry" );
    if (pEntry)
    {
        DicUserData aData( GetDicUserData( rxDic, nIdx ) );
        pEntry->SetUserData( reinterpret_cast<void *>(aData.GetUserData()) );
        lcl_SetCheckButton( pEntry, aData.IsChecked() );
    }

    m_pLinguDicsCLB->SetUpdateMode(true);
}



void SvxLinguTabPage::UpdateDicBox_Impl()
{
    m_pLinguDicsCLB->SetUpdateMode(false);
    m_pLinguDicsCLB->Clear();

    sal_Int32 nDics  = aDics.getLength();
    const uno::Reference< XDictionary > *pDic = aDics.getConstArray();
    for (sal_Int32 i = 0;  i < nDics;  ++i)
    {
        const uno::Reference< XDictionary > &rDic = pDic[i];
        if (rDic.is())
            AddDicBoxEntry( rDic, (sal_uInt16)i );
    }

    m_pLinguDicsCLB->SetUpdateMode(true);
}



void SvxLinguTabPage::UpdateModulesBox_Impl()
{
    if (pLinguData)
    {
        const ServiceInfoArr &rAllDispSrvcArr = pLinguData->GetDisplayServiceArray();
        const sal_uLong nDispSrvcCount = pLinguData->GetDisplayServiceCount();

        m_pLinguModulesCLB->Clear();

        for (sal_uLong i = 0;  i < nDispSrvcCount;  ++i)
        {
            const ServiceInfo_Impl &rInfo = rAllDispSrvcArr[i];
            m_pLinguModulesCLB->InsertEntry( rInfo.sDisplayName, TREELIST_APPEND );
            SvTreeListEntry* pEntry = m_pLinguModulesCLB->GetEntry(i);
            pEntry->SetUserData( const_cast<ServiceInfo_Impl *>(&rInfo) );
            m_pLinguModulesCLB->CheckEntryPos( i, rInfo.bConfigured );
        }
        m_pLinguModulesEditPB->Enable( nDispSrvcCount > 0 );
    }
}



void SvxLinguTabPage::Reset( const SfxItemSet* rSet )
{
    // if not HideGroups was called with GROUP_MODULES...
    if (m_pLinguModulesCLB->IsVisible())
    {
        if (!pLinguData)
            pLinguData = new SvxLinguData_Impl;
        UpdateModulesBox_Impl();
    }



    //  get data from configuration


    SvtLinguConfig aLngCfg;

    m_pLinguOptionsCLB->SetUpdateMode(false);
    m_pLinguOptionsCLB->Clear();

    SvTreeList *pModel = m_pLinguOptionsCLB->GetModel();
    SvTreeListEntry* pEntry = NULL;

    sal_Int16 nVal = 0;
    bool  bVal  = false;
    sal_uLong nUserData = 0;

    pEntry = CreateEntry( sSpellAuto,       CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_AUTO ) >>= bVal;
    const SfxPoolItem* pItem = GetItem( *rSet, SID_AUTOSPELL_CHECK );
    if (pItem)
        bVal = static_cast<const SfxBoolItem *>(pItem)->GetValue();
    nUserData = OptionsUserData( EID_SPELL_AUTO, false, 0, true, bVal).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sGrammarAuto,       CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_GRAMMAR_AUTO ) >>= bVal;
    nUserData = OptionsUserData( EID_GRAMMAR_AUTO, false, 0, true, bVal).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sCapitalWords,    CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_UPPER_CASE ) >>= bVal;
    nUserData = OptionsUserData( EID_CAPITAL_WORDS, false, 0, true, bVal).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sWordsWithDigits, CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_WITH_DIGITS ) >>= bVal;
    nUserData = OptionsUserData( EID_WORDS_WITH_DIGITS, false, 0, true, bVal).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sSpellSpecial,    CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_SPECIAL ) >>= bVal;
    nUserData = OptionsUserData( EID_SPELL_SPECIAL, false, 0, true, bVal).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sNumMinWordlen,   CBCOL_SECOND );
    aLngCfg.GetProperty( UPN_HYPH_MIN_WORD_LENGTH ) >>= nVal;
    nUserData = OptionsUserData( EID_NUM_MIN_WORDLEN, true, (sal_uInt16)nVal, false, false).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );

    const SfxHyphenRegionItem *pHyp = NULL;
    sal_uInt16 nWhich = GetWhich( SID_ATTR_HYPHENREGION );
    if ( rSet->GetItemState( nWhich, false ) == SfxItemState::SET )
        pHyp = &static_cast<const SfxHyphenRegionItem &>( rSet->Get( nWhich ) );

    pEntry = CreateEntry( sNumPreBreak,     CBCOL_SECOND );
    aLngCfg.GetProperty( UPN_HYPH_MIN_LEADING ) >>= nVal;
    if (pHyp)
        nVal = (sal_Int16) pHyp->GetMinLead();
    nUserData = OptionsUserData( EID_NUM_PRE_BREAK, true, (sal_uInt16)nVal, false, false).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );

    pEntry = CreateEntry( sNumPostBreak,    CBCOL_SECOND );
    aLngCfg.GetProperty( UPN_HYPH_MIN_TRAILING ) >>= nVal;
    if (pHyp)
        nVal = (sal_Int16) pHyp->GetMinTrail();
    nUserData = OptionsUserData( EID_NUM_POST_BREAK, true, (sal_uInt16)nVal, false, false).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );

    pEntry = CreateEntry( sHyphAuto,        CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_HYPH_AUTO ) >>= bVal;
    nUserData = OptionsUserData( EID_HYPH_AUTO, false, 0, true, bVal).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sHyphSpecial,     CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_HYPH_SPECIAL ) >>= bVal;
    nUserData = OptionsUserData( EID_HYPH_SPECIAL, false, 0, true, bVal).GetUserData();
    pEntry->SetUserData( reinterpret_cast<void *>(nUserData) );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    m_pLinguOptionsCLB->SetUpdateMode(true);
}



IMPL_LINK_TYPED( SvxLinguTabPage, BoxDoubleClickHdl_Impl, SvTreeListBox *, pBox, bool )
{
    if (pBox == m_pLinguModulesCLB)
    {
        //! in order to avoid a bug causing a GPF when double clicking
        //! on a module entry and exiting the "Edit Modules" dialog
        //! after that.
        Application::PostUserEvent( LINK(
                    this, SvxLinguTabPage, PostDblClickHdl_Impl ), NULL, true);
    }
    else if (pBox == m_pLinguOptionsCLB)
    {
        ClickHdl_Impl(m_pLinguOptionsEditPB);
    }
    return false;
}



IMPL_LINK_NOARG_TYPED(SvxLinguTabPage, PostDblClickHdl_Impl, void*, void)
{
    ClickHdl_Impl(m_pLinguModulesEditPB);
}



IMPL_LINK_NOARG_TYPED(SvxLinguTabPage, OpenURLHdl_Impl, FixedHyperlink&, void)
{
    OUString sURL( m_pMoreDictsLink->GetURL() );
    lcl_OpenURL( sURL );
}



IMPL_LINK_TYPED( SvxLinguTabPage, BoxCheckButtonHdl_Impl, SvTreeListBox *, pBox, void )
{
    if (pBox == m_pLinguModulesCLB)
    {
        DBG_ASSERT( pLinguData, "NULL pointer, LinguData missing" );
        sal_uLong nPos = m_pLinguModulesCLB->GetSelectEntryPos();
        if (nPos != TREELIST_ENTRY_NOTFOUND  &&  pLinguData)
        {
            pLinguData->Reconfigure( m_pLinguModulesCLB->GetText( nPos ),
                                     m_pLinguModulesCLB->IsChecked( nPos ) );
        }
    }
    else if (pBox == m_pLinguDicsCLB)
    {
        sal_uLong nPos = m_pLinguDicsCLB->GetSelectEntryPos();
        if (nPos != TREELIST_ENTRY_NOTFOUND)
        {
            const uno::Reference< XDictionary > &rDic = aDics.getConstArray()[ nPos ];
            if (SvxGetIgnoreAllList() == rDic)
            {
                SvTreeListEntry* pEntry = m_pLinguDicsCLB->GetEntry( nPos );
                if (pEntry)
                    lcl_SetCheckButton( pEntry, true );
            }
        }
    }
}



IMPL_LINK_TYPED( SvxLinguTabPage, ClickHdl_Impl, Button *, pBtn, void )
{
    if (m_pLinguModulesEditPB == pBtn)
    {
        if (!pLinguData)
            pLinguData = new SvxLinguData_Impl;

        SvxLinguData_Impl   aOldLinguData( *pLinguData );
        ScopedVclPtrInstance< SvxEditModulesDlg > aDlg( this, *pLinguData );
        if (aDlg->Execute() != RET_OK)
            *pLinguData = aOldLinguData;

        // evaluate new status of 'bConfigured' flag
        sal_uLong nLen = pLinguData->GetDisplayServiceCount();
        for (sal_uLong i = 0;  i < nLen;  ++i)
            pLinguData->GetDisplayServiceArray()[i].bConfigured = false;
        const Locale* pAllLocales = pLinguData->GetAllSupportedLocales().getConstArray();
        sal_Int32 nLocales = pLinguData->GetAllSupportedLocales().getLength();
        for (sal_Int32 k = 0;  k < nLocales;  ++k)
        {
            sal_Int16 nLang = LanguageTag::convertToLanguageType( pAllLocales[k] );
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
    else if (m_pLinguDicsNewPB == pBtn)
    {
        uno::Reference< XSpellChecker1 > xSpellChecker1;
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            std::unique_ptr<AbstractSvxNewDictionaryDialog> aDlg(pFact->CreateSvxNewDictionaryDialog( this, xSpellChecker1 ));
            DBG_ASSERT(aDlg, "Dialog creation failed!");
            uno::Reference< XDictionary >  xNewDic;
            if ( aDlg->Execute() == RET_OK )
                xNewDic = uno::Reference< XDictionary >( aDlg->GetNewDictionary(), UNO_QUERY );
            if ( xNewDic.is() )
            {
                // add new dics to the end
                sal_Int32 nLen = aDics.getLength();
                aDics.realloc( nLen + 1 );

                aDics.getArray()[ nLen ] = xNewDic;

                AddDicBoxEntry( xNewDic, (sal_uInt16) nLen );
            }
        }
    }
    else if (m_pLinguDicsEditPB == pBtn)
    {
        SvTreeListEntry *pEntry = m_pLinguDicsCLB->GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( reinterpret_cast<sal_uLong>(pEntry->GetUserData()) );
            sal_uInt16 nDicPos = aData.GetEntryId();
            sal_Int32 nDics = aDics.getLength();
            if (nDicPos < nDics)
            {
                uno::Reference< XDictionary > xDic;
                xDic = aDics.getConstArray()[ nDicPos ];
                if (xDic.is())
                {
                    uno::Reference< XSpellChecker1 > xSpellChecker1;
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    if(pFact)
                    {
                        std::unique_ptr<VclAbstractDialog> aDlg(pFact->CreateSvxEditDictionaryDialog( this, xDic->getName(), xSpellChecker1, RID_SFXDLG_EDITDICT ));
                        DBG_ASSERT(aDlg, "Dialog creation failed!");
                        aDlg->Execute();
                    }
                }
            }
        }
    }
    else if (m_pLinguDicsDelPB == pBtn)
    {
        ScopedVclPtrInstance<MessageDialog> aQuery(this, "QueryDeleteDictionaryDialog",
                                                   "cui/ui/querydeletedictionarydialog.ui");
        if (RET_NO == aQuery->Execute())
            return;

        SvTreeListEntry *pEntry = m_pLinguDicsCLB->GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( reinterpret_cast<sal_uLong>(pEntry->GetUserData()) );
            sal_uInt16 nDicPos = aData.GetEntryId();
            sal_Int32 nDics = aDics.getLength();
            if (nDicPos < nDics)
            {
                uno::Reference< XDictionary > xDic;
                xDic = aDics.getConstArray()[ nDicPos ];
                if (xDic.is())
                {
                    if (SvxGetIgnoreAllList() == xDic)
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
                                KillFile_Impl( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
                            }
                        }

                        aDics.getArray()[ nDicPos ] = 0;

                        // remove entry from checklistbox
                        sal_uLong nCnt = m_pLinguDicsCLB->GetEntryCount();
                        for (sal_uLong i = 0;  i < nCnt;  ++i)
                        {
                            SvTreeListEntry *pDicEntry = m_pLinguDicsCLB->GetEntry( i );
                            DBG_ASSERT( pDicEntry, "missing entry" );
                            if (pDicEntry)
                            {
                                DicUserData aDicData( reinterpret_cast<sal_uLong>(pDicEntry->GetUserData()) );
                                if (aDicData.GetEntryId() == nDicPos )
                                {
                                    m_pLinguDicsCLB->RemoveEntry( i );
                                    break;
                                }
                            }
                        }
                        DBG_ASSERT( nCnt > m_pLinguDicsCLB->GetEntryCount(),
                                "remove failed ?");
                    }
                }
            }
        }
    }
    else if (m_pLinguOptionsEditPB == pBtn)
    {
        SvTreeListEntry *pEntry = m_pLinguOptionsCLB->GetCurEntry();
        DBG_ASSERT( pEntry, "no entry selected" );
        if (pEntry)
        {
            OptionsUserData aData( reinterpret_cast<sal_uLong>(pEntry->GetUserData()) );
            if(aData.HasNumericValue())
            {
                sal_uInt16 nRID = aData.GetEntryId();
                ScopedVclPtrInstance< OptionsBreakSet > aDlg(this, nRID);
                aDlg->GetNumericFld().SetValue( aData.GetNumericValue() );
                if (RET_OK == aDlg->Execute() )
                {
                    long nVal = static_cast<long>(aDlg->GetNumericFld().GetValue());
                    if (-1 != nVal && aData.GetNumericValue() != nVal)
                    {
                        aData.SetNumericValue( (sal_uInt8)nVal ); //! sets IsModified !
                        pEntry->SetUserData( reinterpret_cast<void *>(aData.GetUserData()) );
                        m_pLinguOptionsCLB->Invalidate();
                    }
                }
            }
        }
    }
    else
    {
        OSL_FAIL( "pBtn unexpected value" );
    }
}



IMPL_LINK_TYPED( SvxLinguTabPage, SelectHdl_Impl, SvTreeListBox*, pBox, void )
{
    if (m_pLinguModulesCLB == pBox)
    {
    }
    else if (m_pLinguDicsCLB == pBox)
    {
        SvTreeListEntry *pEntry = pBox->GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( reinterpret_cast<sal_uLong>( pEntry->GetUserData() ) );

            // always allow to edit (i.e. at least view the content of the dictionary)
            m_pLinguDicsEditPB->Enable( true/*aData.IsEditable()*/ );
            m_pLinguDicsDelPB->Enable( aData.IsDeletable() );
        }
    }
    else if (m_pLinguOptionsCLB == pBox)
    {
        SvTreeListEntry *pEntry = pBox->GetCurEntry();
        if (pEntry)
        {
            OptionsUserData aData( reinterpret_cast<sal_uLong>( pEntry->GetUserData() ) );
            m_pLinguOptionsEditPB->Enable( aData.HasNumericValue() );
        }
    }
    else
    {
        OSL_FAIL( "pBox unexpected value" );
    }
}



SvTreeListEntry* SvxLinguTabPage::CreateEntry( OUString& rTxt, sal_uInt16 nCol )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if( !pCheckButtonData )
        pCheckButtonData = new SvLBoxButtonData(m_pLinguOptionsCLB);

    if (CBCOL_FIRST == nCol)
        pEntry->AddItem(std::unique_ptr<SvLBoxButton>(new SvLBoxButton(
            pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData)));
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem(std::unique_ptr<SvLBoxString>(new SvLBoxString(
            pEntry, 0, "")));    // empty column
    pEntry->AddItem(std::unique_ptr<SvLBoxContextBmp>(new SvLBoxContextBmp(
            pEntry, 0, Image(), Image(), false)));
    pEntry->AddItem(std::unique_ptr<BrwString_Impl>(new BrwString_Impl(
            pEntry, 0, rTxt)));

    return pEntry;
}



void SvxLinguTabPage::HideGroups( sal_uInt16 nGrp )
{
    if ( 0 != ( GROUP_MODULES & nGrp ) )
    {
        m_pLinguModulesFT->Hide();
        m_pLinguModulesCLB->Hide();
        m_pLinguModulesEditPB->Hide();

        if ( SvtExtendedSecurityOptions().GetOpenHyperlinkMode()
                != SvtExtendedSecurityOptions::OPEN_NEVER )
        {
            m_pMoreDictsLink->Show();
        }
    }
}

SvxEditModulesDlg::SvxEditModulesDlg(vcl::Window* pParent, SvxLinguData_Impl& rData)
    : ModalDialog( pParent, "EditModulesDialog",
        "cui/ui/editmodulesdialog.ui")
    , sSpell(CUI_RES(RID_SVXSTR_SPELL))
    , sHyph(CUI_RES(RID_SVXSTR_HYPH))
    , sThes(CUI_RES(RID_SVXSTR_THES))
    , sGrammar(CUI_RES(RID_SVXSTR_GRAMMAR))
    , rLinguData(rData)
{
    get(m_pClosePB, "close");
    get(m_pMoreDictsLink, "moredictslink");
    get(m_pBackPB, "back");
    get(m_pPrioDownPB, "down");
    get(m_pPrioUpPB, "up");
    get(m_pModulesCLB, "lingudicts");
    Size aListSize(m_pModulesCLB->LogicToPixel(Size(166, 120), MAP_APPFONT));
    m_pModulesCLB->set_height_request(aListSize.Height());
    m_pModulesCLB->set_width_request(aListSize.Width());
    get(m_pLanguageLB, "language");
    m_pLanguageLB->SetStyle(m_pLanguageLB->GetStyle() | WB_SORT);

    pCheckButtonData = NULL;

    pDefaultLinguData = new SvxLinguData_Impl( rLinguData );

    m_pModulesCLB->SetStyle( m_pModulesCLB->GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    m_pModulesCLB->SetHighlightRange();
    m_pModulesCLB->SetSelectHdl( LINK( this, SvxEditModulesDlg, SelectHdl_Impl ));
    m_pModulesCLB->SetCheckButtonHdl( LINK( this, SvxEditModulesDlg, BoxCheckButtonHdl_Impl) );

    m_pClosePB->SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    m_pPrioUpPB->SetClickHdl( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    m_pPrioDownPB->SetClickHdl( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    m_pBackPB->SetClickHdl( LINK( this, SvxEditModulesDlg, BackHdl_Impl ));
    // in case of not installed language modules
    m_pPrioUpPB->Enable( false );
    m_pPrioDownPB->Enable( false );

    if ( SvtExtendedSecurityOptions().GetOpenHyperlinkMode()
            != SvtExtendedSecurityOptions::OPEN_NEVER )
    {
        m_pMoreDictsLink->SetClickHdl( LINK( this, SvxEditModulesDlg, OpenURLHdl_Impl ) );
    }
    else
    {
        m_pMoreDictsLink->Hide();
    }

    //fill language box
    Sequence< sal_Int16 > aAvailLang;
    uno::Reference< XAvailableLocales > xAvail( rLinguData.GetManager(), UNO_QUERY );
    if (xAvail.is())
    {
        aAvailLang = lcl_LocaleSeqToLangSeq(
                        xAvail->getAvailableLocales( cSpell ) );
    }
    const Sequence< Locale >& rLoc = rLinguData.GetAllSupportedLocales();
    const Locale* pLocales = rLoc.getConstArray();
    m_pLanguageLB->Clear();
    for(long i = 0; i < rLoc.getLength(); i++)
    {
        sal_Int16 nLang = LanguageTag::convertToLanguageType( pLocales[i] );
        m_pLanguageLB->InsertLanguage( nLang, lcl_SeqHasLang( aAvailLang, nLang ) );
    }
    LanguageType eSysLang = MsLangId::getSystemLanguage();
    m_pLanguageLB->SelectLanguage( eSysLang );
    if(!m_pLanguageLB->IsLanguageSelected( eSysLang ) )
        m_pLanguageLB->SelectEntryPos(0);

    m_pLanguageLB->SetSelectHdl( LINK( this, SvxEditModulesDlg, LangSelectListBoxHdl_Impl ));
    LangSelectHdl_Impl(m_pLanguageLB);
}


SvxEditModulesDlg::~SvxEditModulesDlg()
{
    disposeOnce();
}

void SvxEditModulesDlg::dispose()
{
    delete pDefaultLinguData;
    pDefaultLinguData = NULL;
    m_pLanguageLB.clear();
    m_pModulesCLB.clear();
    m_pPrioUpPB.clear();
    m_pPrioDownPB.clear();
    m_pBackPB.clear();
    m_pMoreDictsLink.clear();
    m_pClosePB.clear();
    ModalDialog::dispose();
}

SvTreeListEntry* SvxEditModulesDlg::CreateEntry( OUString& rTxt, sal_uInt16 nCol )
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;
    if( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData(m_pModulesCLB);
        pCheckButtonData->SetLink( LINK( this, SvxEditModulesDlg, BoxCheckButtonHdl_Impl2 ) );
    }

    if (CBCOL_FIRST == nCol)
        pEntry->AddItem(std::unique_ptr<SvLBoxButton>(new SvLBoxButton(pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData)));
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem(std::unique_ptr<SvLBoxString>(new SvLBoxString(pEntry, 0, "")));    // empty column
    pEntry->AddItem(std::unique_ptr<SvLBoxContextBmp>(new SvLBoxContextBmp( pEntry, 0, Image(), Image(), false)));
    pEntry->AddItem(std::unique_ptr<BrwStringDic_Impl>(new BrwStringDic_Impl(pEntry, 0, rTxt)));

    return pEntry;
}

IMPL_LINK_TYPED( SvxEditModulesDlg, SelectHdl_Impl, SvTreeListBox*, pBox, void )
{
    if (m_pModulesCLB == pBox)
    {
        SvTreeListEntry *pEntry = pBox->GetCurEntry();
        if (pEntry)
        {
            bool bDisableUp = true;
            bool bDisableDown = true;
            ModuleUserData_Impl* pData = static_cast<ModuleUserData_Impl*>(pEntry->GetUserData());
            if(!pData->IsParent() && pData->GetType() != TYPE_HYPH)
            {
                sal_uLong  nCurPos = static_cast<SvxCheckListBox*>(pBox)->GetSelectEntryPos();
                if(nCurPos < pBox->GetEntryCount() - 1)
                {
                    bDisableDown = static_cast<ModuleUserData_Impl*>(pBox->
                            GetEntry(nCurPos + 1)->GetUserData())->IsParent();
                }
                if(nCurPos > 1)
                {
                    bDisableUp = static_cast<ModuleUserData_Impl*>(pBox->
                            GetEntry(nCurPos - 1)->GetUserData())->IsParent();
                }
            }
            m_pPrioUpPB->Enable(!bDisableUp);
            m_pPrioDownPB->Enable(!bDisableDown);
        }
    }
    else
    {
        OSL_FAIL( "pBox unexpected value" );
    }
}

IMPL_LINK_NOARG_TYPED( SvxEditModulesDlg, BoxCheckButtonHdl_Impl2, SvLBoxButtonData*, void )
{
    BoxCheckButtonHdl_Impl(NULL);
}
IMPL_LINK_NOARG_TYPED( SvxEditModulesDlg, BoxCheckButtonHdl_Impl, SvTreeListBox *, void )
{
    SvTreeListEntry *pCurEntry = m_pModulesCLB->GetCurEntry();
    if (pCurEntry)
    {
        ModuleUserData_Impl* pData = static_cast<ModuleUserData_Impl *>(
                                            pCurEntry->GetUserData());
        if (!pData->IsParent()  &&  pData->GetType() == TYPE_HYPH)
        {
            // make hyphenator checkboxes function as radio-buttons
            // (at most one box may be checked)
            SvTreeListEntry *pEntry = m_pModulesCLB->First();
            while (pEntry)
            {
                pData = static_cast<ModuleUserData_Impl*>(pEntry->GetUserData());
                if (!pData->IsParent()  &&
                     pData->GetType() == TYPE_HYPH  &&
                     pEntry != pCurEntry)
                {
                    lcl_SetCheckButton( pEntry, false );
                    m_pModulesCLB->InvalidateEntry( pEntry );
                }
                pEntry = m_pModulesCLB->Next( pEntry );
            }
        }
    }
}

IMPL_LINK_TYPED( SvxEditModulesDlg, LangSelectListBoxHdl_Impl, ListBox&, rBox, void )
{
    LangSelectHdl_Impl(&rBox);
}

void SvxEditModulesDlg::LangSelectHdl_Impl(ListBox* pBox)
{
    LanguageType  eCurLanguage = m_pLanguageLB->GetSelectLanguage();
    static Locale aLastLocale;
    Locale aCurLocale( LanguageTag::convertToLocale( eCurLanguage));
    SvTreeList *pModel = m_pModulesCLB->GetModel();

    if (pBox)
    {
        // save old probably changed settings
        // before switching to new language entries

        sal_Int16 nLang = LanguageTag::convertToLanguageType( aLastLocale );

        sal_Int32 nStart = 0, nLocalIndex = 0;
        Sequence< OUString > aChange;
        bool bChanged = false;
        for(sal_uLong i = 0; i < m_pModulesCLB->GetEntryCount(); i++)
        {
            SvTreeListEntry *pEntry = m_pModulesCLB->GetEntry(i);
            ModuleUserData_Impl* pData = static_cast<ModuleUserData_Impl*>(pEntry->GetUserData());
            if(pData->IsParent())
            {
                if(bChanged)
                {
                    LangImplNameTable *pTable = 0;
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
                aChange.realloc(m_pModulesCLB->GetEntryCount());
                bChanged = false;
            }
            else
            {
                OUString* pChange = aChange.getArray();
                pChange[nStart] = pData->GetImplName();
                bChanged |= pData->GetIndex() != nLocalIndex ||
                    pData->IsChecked() != m_pModulesCLB->IsChecked(i);
                if(m_pModulesCLB->IsChecked(i))
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

    for(sal_uLong i = 0; i < m_pModulesCLB->GetEntryCount(); i++)
        delete static_cast<ModuleUserData_Impl*>(m_pModulesCLB->GetEntry(i)->GetUserData());


    // display entries for new selected language

    m_pModulesCLB->Clear();
    if(LANGUAGE_DONTKNOW != eCurLanguage)
    {
        sal_uLong n;
        ServiceInfo_Impl* pInfo;


        // spellchecker entries

        SvTreeListEntry* pEntry = CreateEntry( sSpell,  CBCOL_SECOND );
        ModuleUserData_Impl* pUserData = new ModuleUserData_Impl(
                                         OUString(), true, false, TYPE_SPELL, 0 );
        pEntry->SetUserData( static_cast<void *>(pUserData) );
        pModel->Insert( pEntry );

        Sequence< OUString > aNames( rLinguData.GetSortedImplNames( eCurLanguage, TYPE_SPELL ) );
        const OUString *pName = aNames.getConstArray();
        sal_uLong nNames = (sal_uLong) aNames.getLength();
        sal_Int32 nLocalIndex = 0;  // index relative to parent
        for (n = 0;  n < nNames;  ++n)
        {
            OUString aImplName;
            bool     bIsSuppLang = false;

            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            if (pInfo)
            {
                bIsSuppLang = pInfo->xSpell.is()  &&
                              pInfo->xSpell->hasLocale( aCurLocale );
                aImplName = pInfo->sSpellImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                OUString aTxt( pInfo->sDisplayName );
                SvTreeListEntry* pNewEntry = CreateEntry( aTxt, CBCOL_FIRST );

                LangImplNameTable &rTable = rLinguData.GetSpellTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    DBG_WARNING( "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                lcl_SetCheckButton( pNewEntry, bCheck );
                pUserData = new ModuleUserData_Impl( aImplName, false,
                                        bCheck, TYPE_SPELL, (sal_uInt8)nLocalIndex++ );
                pNewEntry->SetUserData( static_cast<void *>(pUserData) );
                pModel->Insert( pNewEntry );
            }
        }


        // grammar checker entries

        pEntry = CreateEntry( sGrammar,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( OUString(), true, false, TYPE_GRAMMAR, 0 );
        pEntry->SetUserData( static_cast<void *>(pUserData) );
        pModel->Insert( pEntry );

        aNames = rLinguData.GetSortedImplNames( eCurLanguage, TYPE_GRAMMAR );
        pName = aNames.getConstArray();
        nNames = (sal_uLong) aNames.getLength();
        nLocalIndex = 0;
        for (n = 0;  n < nNames;  ++n)
        {
            OUString aImplName;
            bool     bIsSuppLang = false;

            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            if (pInfo)
            {
                bIsSuppLang = pInfo->xGrammar.is()  &&
                              pInfo->xGrammar->hasLocale( aCurLocale );
                aImplName = pInfo->sGrammarImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                OUString aTxt( pInfo->sDisplayName );
                SvTreeListEntry* pNewEntry = CreateEntry( aTxt, CBCOL_FIRST );

                LangImplNameTable &rTable = rLinguData.GetGrammarTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    DBG_WARNING( "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                lcl_SetCheckButton( pNewEntry, bCheck );
                pUserData = new ModuleUserData_Impl( aImplName, false,
                                        bCheck, TYPE_GRAMMAR, (sal_uInt8)nLocalIndex++ );
                pNewEntry->SetUserData( static_cast<void *>(pUserData) );
                pModel->Insert( pNewEntry );
            }
        }


        // hyphenator entries

        pEntry = CreateEntry( sHyph,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( OUString(), true, false, TYPE_HYPH, 0 );
        pEntry->SetUserData( static_cast<void *>(pUserData) );
        pModel->Insert( pEntry );

        aNames = rLinguData.GetSortedImplNames( eCurLanguage, TYPE_HYPH );
        pName = aNames.getConstArray();
        nNames = (sal_uLong) aNames.getLength();
        nLocalIndex = 0;
        for (n = 0;  n < nNames;  ++n)
        {
            OUString aImplName;
            bool     bIsSuppLang = false;

            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            if (pInfo)
            {
                bIsSuppLang = pInfo->xHyph.is()  &&
                              pInfo->xHyph->hasLocale( aCurLocale );
                aImplName = pInfo->sHyphImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                OUString aTxt( pInfo->sDisplayName );
                SvTreeListEntry* pNewEntry = CreateEntry( aTxt, CBCOL_FIRST );

                LangImplNameTable &rTable = rLinguData.GetHyphTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    DBG_WARNING( "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                lcl_SetCheckButton( pNewEntry, bCheck );
                pUserData = new ModuleUserData_Impl( aImplName, false,
                                        bCheck, TYPE_HYPH, (sal_uInt8)nLocalIndex++ );
                pNewEntry->SetUserData( static_cast<void *>(pUserData) );
                pModel->Insert( pNewEntry );
            }
        }


        // thesaurus entries

        pEntry = CreateEntry( sThes,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( OUString(), true, false, TYPE_THES, 0 );
        pEntry->SetUserData( static_cast<void *>(pUserData) );
        pModel->Insert( pEntry );

        aNames = rLinguData.GetSortedImplNames( eCurLanguage, TYPE_THES );
        pName = aNames.getConstArray();
        nNames = (sal_uLong) aNames.getLength();
        nLocalIndex = 0;
        for (n = 0;  n < nNames;  ++n)
        {
            OUString aImplName;
            bool     bIsSuppLang = false;

            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            if (pInfo)
            {
                bIsSuppLang = pInfo->xThes.is()  &&
                              pInfo->xThes->hasLocale( aCurLocale );
                aImplName = pInfo->sThesImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                OUString aTxt( pInfo->sDisplayName );
                SvTreeListEntry* pNewEntry = CreateEntry( aTxt, CBCOL_FIRST );

                LangImplNameTable &rTable = rLinguData.GetThesTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    DBG_WARNING( "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                lcl_SetCheckButton( pNewEntry, bCheck );
                pUserData = new ModuleUserData_Impl( aImplName, false,
                                        bCheck, TYPE_THES, (sal_uInt8)nLocalIndex++ );
                pNewEntry->SetUserData( static_cast<void *>(pUserData) );
                pModel->Insert( pNewEntry );
            }
        }
    }
    aLastLocale = aCurLocale;
}

IMPL_LINK_TYPED( SvxEditModulesDlg, UpDownHdl_Impl, Button *, pBtn, void )
{
    bool bUp = m_pPrioUpPB == pBtn;
    sal_uLong  nCurPos = m_pModulesCLB->GetSelectEntryPos();
    SvTreeListEntry* pEntry;
    if (nCurPos != TREELIST_ENTRY_NOTFOUND  &&
        0 != (pEntry = m_pModulesCLB->GetEntry(nCurPos)))
    {
        m_pModulesCLB->SetUpdateMode(false);
        SvTreeList *pModel = m_pModulesCLB->GetModel();

        ModuleUserData_Impl* pData = static_cast<ModuleUserData_Impl*>(pEntry->GetUserData());
        OUString aStr(m_pModulesCLB->GetEntryText(pEntry));
        SvTreeListEntry* pToInsert = CreateEntry( aStr, CBCOL_FIRST );
        pToInsert->SetUserData( static_cast<void *>(pData));
        bool bIsChecked = m_pModulesCLB->IsChecked(nCurPos);

        pModel->Remove(pEntry);

        sal_uLong nDestPos = bUp ? nCurPos - 1 : nCurPos + 1;
        pModel->Insert(pToInsert, nDestPos);
        m_pModulesCLB->CheckEntryPos(nDestPos, bIsChecked );
        m_pModulesCLB->SelectEntryPos(nDestPos );
        SelectHdl_Impl(m_pModulesCLB);
        m_pModulesCLB->SetUpdateMode(true);
    }
}

IMPL_LINK_NOARG_TYPED(SvxEditModulesDlg, ClickHdl_Impl, Button*, void)
{
    // store language config
    LangSelectHdl_Impl(m_pLanguageLB);
    EndDialog( RET_OK );
}

IMPL_LINK_NOARG_TYPED(SvxEditModulesDlg, BackHdl_Impl, Button*, void)
{
    rLinguData = *pDefaultLinguData;
    LangSelectHdl_Impl(0);
}



IMPL_LINK_NOARG_TYPED(SvxEditModulesDlg, OpenURLHdl_Impl, FixedHyperlink&, void)
{
    OUString sURL( m_pMoreDictsLink->GetURL() );
    lcl_OpenURL( sURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
