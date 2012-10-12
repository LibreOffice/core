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
#include <tools/shl.hxx>
#include <i18npool/mslangid.hxx>
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
#include <svtools/langhelp.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <sfx2/viewfrm.hxx>
#include <vcl/svapp.hxx>
#define _SVX_OPTLINGU_CXX
#include "optlingu.hrc"

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
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::beans;
namespace css = com::sun::star;

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1
#define CBCOL_BOTH      2

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
        pRes[i] = SvxLocaleToLanguage( pSeq[i] );
    }
    return aRes;
}


static sal_Bool lcl_SeqHasLang( const Sequence< sal_Int16 > &rSeq, sal_Int16 nLang )
{
    sal_Int32 nLen = rSeq.getLength();
    const sal_Int16 *pLang = rSeq.getConstArray();
    sal_Int32 nPos = -1;
    for (sal_Int32 i = 0;  i < nLen  &&  nPos < 0;  ++i)
    {
        if (nLang == pLang[i])
            nPos = i;
    }
    return nPos < 0 ? sal_False : sal_True;
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

static void lcl_OpenURL( ::rtl::OUString sURL )
{
    if ( !sURL.isEmpty() )
    {
        localizeWebserviceURI(sURL);
        try
        {
            uno::Reference< uno::XComponentContext > xContext =
                ::comphelper::getProcessComponentContext();
            uno::Reference< css::system::XSystemShellExecute > xSystemShell(
                css::system::SystemShellExecute::create(xContext) );
            xSystemShell->execute( sURL, ::rtl::OUString(), css::system::SystemShellExecuteFlags::URIS_ONLY );
        }
        catch( const uno::Exception& e )
        {
             OSL_TRACE( "Caught exception: %s\n thread terminated.\n",
                rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
        }
    }
}

static sal_uInt16 pRanges[] =
{
    SID_ATTR_SPELL,
    SID_ATTR_SPELL,
    0
};

sal_Bool KillFile_Impl( const String& rURL )
{
    sal_Bool bRet = sal_True;
    try
    {
        Content aCnt( rURL, uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(), comphelper::getProcessComponentContext() );
        aCnt.executeCommand( OUString("delete"), makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        SAL_WARN( "cui.options", "KillFile: CommandAbortedException" );
        bRet = sal_False;
    }
    catch( ... )
    {
        SAL_WARN( "cui.options", "KillFile: Any other exception" );
        bRet = sal_False;
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
    sal_Bool bParent;
    sal_Bool bIsChecked;
    sal_uInt8 nType;
    sal_uInt8 nIndex;
    String  sImplName;

public:
    ModuleUserData_Impl( String sImpName, sal_Bool bIsParent, sal_Bool bChecked, sal_uInt8 nSetType, sal_uInt8 nSetIndex ) :
        bParent(bIsParent),
        bIsChecked(bChecked),
        nType(nSetType),
        nIndex(nSetIndex),
        sImplName(sImpName)
        {
        }
    sal_Bool IsParent() const {return bParent;}
    sal_uInt8 GetType() const {return nType;}
    sal_Bool IsChecked() const {return bIsChecked;}
    sal_uInt8 GetIndex() const {return nIndex;}
    void SetIndex(sal_uInt8 nSet)  {nIndex = nSet;}
    const String& GetImplName() const {return sImplName;}

};

//
// User for user-dictionaries (XDictionary interface)
//
class DicUserData
{
    sal_uLong   nVal;

public:
    DicUserData( sal_uLong nUserData ) : nVal( nUserData ) {}
    DicUserData( sal_uInt16 nEID,
                 sal_Bool bChecked, sal_Bool bEditable, sal_Bool bDeletable );

    sal_uLong   GetUserData() const         { return nVal; }
    sal_uInt16  GetEntryId() const          { return (sal_uInt16)(nVal >> 16); }
    sal_Bool    IsChecked() const           { return (sal_Bool)(nVal >>  8) & 0x01; }
    sal_Bool    IsEditable() const          { return (sal_Bool)(nVal >>  9) & 0x01; }
    sal_Bool    IsDeletable() const         { return (sal_Bool)(nVal >> 10) & 0x01; }

    void    SetChecked( sal_Bool bVal );
};


DicUserData::DicUserData(
        sal_uInt16 nEID,
        sal_Bool bChecked, sal_Bool bEditable, sal_Bool bDeletable )
{
    DBG_ASSERT( nEID < 65000, "Entry Id out of range" );
    nVal =  ((sal_uLong)(0xFFFF & nEID)         << 16) |
            ((sal_uLong)(bChecked ? 1 : 0)      <<  8) |
            ((sal_uLong)(bEditable ? 1 : 0)     <<  9) |
            ((sal_uLong)(bDeletable ? 1 : 0)    << 10);
}


void DicUserData::SetChecked( sal_Bool bVal )
{
    nVal &= ~(1UL << 8);
    nVal |=  (sal_uLong)(bVal ? 1 : 0) << 8;
}


// class BrwString_Impl -------------------------------------------------

static void lcl_SetCheckButton( SvLBoxEntry* pEntry, sal_Bool bCheck )
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found");
    if (((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON)
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

    BrwStringDic_Impl( SvLBoxEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString( pEntry, nFlags, rStr ) {}

    virtual void Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags,
                                            SvLBoxEntry* pEntry);
};

void BrwStringDic_Impl::Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16,
    SvLBoxEntry* pEntry )
{
    ModuleUserData_Impl* pData = (ModuleUserData_Impl*)pEntry->GetUserData();
    Point aPos(rPos);
    Font aOldFont( rDev.GetFont());
    if(pData->IsParent())
    {
        Font aFont( aOldFont );
        aFont.SetWeight( WEIGHT_BOLD );
        rDev.SetFont( aFont );
        aPos.X() = 0;
    }
    else
        aPos.X() += 5;
    rDev.DrawText( aPos, GetText() );
    rDev.SetFont( aOldFont );
}

class OptionsBreakSet : public ModalDialog
{
    OKButton        aOKPB;
    CancelButton    aCancelPB;
    FixedLine       aValFL;
    NumericField    aValNF;

public:
    OptionsBreakSet(Window* pParent, int nRID) :
            ModalDialog(pParent, CUI_RES(RID_SVXDLG_LNG_ED_NUM_PREBREAK )),
            aOKPB       (this, CUI_RES(BT_OK_PREBREAK)),
            aCancelPB   (this, CUI_RES(BT_CANCEL_PREBREAK)),
            aValFL      (this, CUI_RES(FL_NUMVAL_PREBREAK)),
            aValNF      (this, CUI_RES(ED_PREBREAK))
    {
        DBG_ASSERT( STR_NUM_PRE_BREAK_DLG   == nRID ||
                    STR_NUM_POST_BREAK_DLG  == nRID ||
                    STR_NUM_MIN_WORDLEN_DLG == nRID, "unexpected RID" );

        if (nRID != -1)
            aValFL.SetText( String( CUI_RES(nRID) ) );
        FreeResource();
    }

    NumericField&   GetNumericFld() { return aValNF; }
};


/*--------------------------------------------------
    Entry IDs for options listbox of dialog
--------------------------------------------------*/

enum EID_OPTIONS
{
    EID_SPELL_AUTO,
    EID_GRAMMAR_AUTO,
    EID_CAPITAL_WORDS,
    EID_WORDS_WITH_DIGITS,
    EID_CAPITALIZATION,
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
    UPN_IS_SPELL_CAPITALIZATION,    // EID_CAPITALIZATION
    UPN_IS_SPELL_SPECIAL,           // EID_SPELL_SPECIAL
    UPN_HYPH_MIN_WORD_LENGTH,       // EID_NUM_MIN_WORDLEN,
    UPN_HYPH_MIN_LEADING,           // EID_NUM_PRE_BREAK
    UPN_HYPH_MIN_TRAILING,          // EID_NUM_POST_BREAK
    UPN_IS_HYPH_AUTO,               // EID_HYPH_AUTO
    UPN_IS_HYPH_SPECIAL             // EID_HYPH_SPECIAL
};


static inline String lcl_GetPropertyName( EID_OPTIONS eEntryId )
{
    DBG_ASSERT( (unsigned int) eEntryId < SAL_N_ELEMENTS(aEidToPropName), "index out of range" );
    return rtl::OUString::createFromAscii( aEidToPropName[ (int) eEntryId ] );
}

// class OptionsUserData -------------------------------------------------

class OptionsUserData
{
    sal_uLong   nVal;

    void    SetModified();

public:
    OptionsUserData( sal_uLong nUserData ) : nVal( nUserData ) {}
    OptionsUserData( sal_uInt16 nEID,
                     sal_Bool bHasNV, sal_uInt16 nNumVal,
                     sal_Bool bCheckable, sal_Bool bChecked );

    sal_uLong   GetUserData() const         { return nVal; }
    sal_uInt16  GetEntryId() const          { return (sal_uInt16)(nVal >> 16); }
    sal_Bool    HasNumericValue() const     { return (sal_Bool)(nVal >> 10) & 0x01; }
    sal_uInt16  GetNumericValue() const     { return (sal_uInt16)(nVal & 0xFF); }
    sal_Bool    IsChecked() const           { return (sal_Bool)(nVal >> 8) & 0x01; }
    sal_Bool    IsCheckable() const         { return (sal_Bool)(nVal >> 9) & 0x01; }
    sal_Bool    IsModified() const          { return (sal_Bool)(nVal >> 11) & 0x01; }

    void    SetChecked( sal_Bool bVal );
    void    SetNumericValue( sal_uInt8 nNumVal );
};

OptionsUserData::OptionsUserData( sal_uInt16 nEID,
        sal_Bool bHasNV, sal_uInt16 nNumVal,
        sal_Bool bCheckable, sal_Bool bChecked )
{
    DBG_ASSERT( nEID < 65000, "Entry Id out of range" );
    DBG_ASSERT( nNumVal < 256, "value out of range" );
    nVal =  ((sal_uLong)(0xFFFF & nEID)         << 16) |
            ((sal_uLong)(bHasNV ? 1 : 0)        << 10) |
            ((sal_uLong)(bCheckable ? 1 : 0)    <<  9) |
            ((sal_uLong)(bChecked ? 1 : 0)      <<  8) |
            ((sal_uLong)(0xFF & nNumVal));
}

void OptionsUserData::SetChecked( sal_Bool bVal )
{
    if (IsCheckable()  &&  (IsChecked() != bVal))
    {
        nVal &= ~(1UL << 8);
        nVal |=  (sal_uLong)(bVal ? 1 : 0) << 8;
        SetModified();
    }
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

    BrwString_Impl( SvLBoxEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString( pEntry, nFlags, rStr ) {}

    virtual void Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags,
                                            SvLBoxEntry* pEntry);
};

void BrwString_Impl::Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16,
    SvLBoxEntry* pEntry )
{
    Point aPos(rPos);
    aPos.X() += 20;
    rDev.DrawText( aPos, GetText() );
    if(pEntry->GetUserData())
    {
        Point aNewPos(aPos);
        aNewPos.X() += rDev.GetTextWidth(GetText());
        Font aOldFont( rDev.GetFont());
        Font aFont( aOldFont );
        aFont.SetWeight( WEIGHT_BOLD );

        //??? convert the lower byte from the user date into a string
        OptionsUserData aData( (sal_uLong) pEntry->GetUserData() );
        if(aData.HasNumericValue())
        {
            rtl::OUStringBuffer sTxt;
            sTxt.append(' ').append(static_cast<sal_Int32>(aData.GetNumericValue()));
            rDev.SetFont( aFont );
            rDev.DrawText( aNewPos, sTxt.makeStringAndClear() );
        }

        rDev.SetFont( aOldFont );
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
    sal_Bool                        bConfigured;

    ServiceInfo_Impl() : bConfigured(sal_False) {}
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
    uno::Reference< XMultiServiceFactory >   xMSF;
    uno::Reference< XLinguServiceManager >   xLinguSrvcMgr;


    sal_Bool    AddRemove( Sequence< OUString > &rConfigured,
                           const OUString &rImplName, sal_Bool bAdd );

public:
    SvxLinguData_Impl();
    SvxLinguData_Impl( const SvxLinguData_Impl &rData );
    ~SvxLinguData_Impl();

    SvxLinguData_Impl & operator = (const SvxLinguData_Impl &rData);

    uno::Reference<XLinguServiceManager> &   GetManager() { return xLinguSrvcMgr; }

    void SetChecked( const Sequence< OUString > &rConfiguredServices );
    void Reconfigure( const OUString &rDisplayName, sal_Bool bEnable );

    const Sequence<Locale> &    GetAllSupportedLocales() const { return aAllServiceLocales; }

    const LangImplNameTable &   GetSpellTable() const   { return aCfgSpellTable; }
    LangImplNameTable &         GetSpellTable()         { return aCfgSpellTable; }
    const LangImplNameTable &   GetHyphTable() const    { return aCfgHyphTable; }
    LangImplNameTable &         GetHyphTable()          { return aCfgHyphTable; }
    const LangImplNameTable &   GetThesTable() const    { return aCfgThesTable; }
    LangImplNameTable &         GetThesTable()          { return aCfgThesTable; }
    const LangImplNameTable &   GetGrammarTable() const { return aCfgGrammarTable; }
    LangImplNameTable &         GetGrammarTable()       { return aCfgGrammarTable; }

    const ServiceInfoArr &      GetDisplayServiceArray() const  { return aDisplayServiceArr; }
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


//-----------------------------------------------------------------------------

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
        sal_Bool bFound = sal_False;
        for(sal_Int32 j = 0; j < aAllLocales.getLength() && !bFound; j++)
        {
            bFound = pAdd[i].Language == pAllLocales[j].Language &&
                pAdd[i].Country == pAllLocales[j].Country;
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

    ServiceInfo_Impl* pEntry;
    for (sal_uLong i = 0;  i < nEntries;  ++i)
    {
        pEntry = &rSvcInfoArr[i];
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
    xMSF = ::comphelper::getProcessServiceFactory();
    uno::Reference < XInterface > xI = xMSF->createInstance(
        "com.sun.star.linguistic2.LinguServiceManager" );
    xLinguSrvcMgr = uno::Reference<XLinguServiceManager>(xI, UNO_QUERY);
    DBG_ASSERT(xLinguSrvcMgr.is(), "No linguistic service available!");
    if(xLinguSrvcMgr.is())
    {
        Locale aCurrentLocale;
        LanguageType eLang = Application::GetSettings().GetLanguage();
        SvxLanguageToLocale(aCurrentLocale, eLang);
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
                            xMSF->createInstanceWithArguments(aInfo.sSpellImplName, aArgs), UNO_QUERY);

            uno::Reference<XServiceDisplayName> xDispName(aInfo.xSpell, UNO_QUERY);
            if(xDispName.is())
                aInfo.sDisplayName = xDispName->getServiceDisplayName( aCurrentLocale );

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
                            xMSF->createInstanceWithArguments(aInfo.sGrammarImplName, aArgs), UNO_QUERY);

            uno::Reference<XServiceDisplayName> xDispName(aInfo.xGrammar, UNO_QUERY);
            if(xDispName.is())
                aInfo.sDisplayName = xDispName->getServiceDisplayName( aCurrentLocale );

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
                            xMSF->createInstanceWithArguments(aInfo.sHyphImplName, aArgs), UNO_QUERY);

            uno::Reference<XServiceDisplayName> xDispName(aInfo.xHyph, UNO_QUERY);
            if(xDispName.is())
                aInfo.sDisplayName = xDispName->getServiceDisplayName( aCurrentLocale );

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
                            xMSF->createInstanceWithArguments(aInfo.sThesImplName, aArgs), UNO_QUERY);

            uno::Reference<XServiceDisplayName> xDispName(aInfo.xThes, UNO_QUERY);
            if(xDispName.is())
                aInfo.sDisplayName = xDispName->getServiceDisplayName( aCurrentLocale );

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
            sal_Int16 nLang = SvxLocaleToLanguage( pAllLocales[nLocale] );

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
}

SvxLinguData_Impl::SvxLinguData_Impl( const SvxLinguData_Impl &rData ) :
    aDisplayServiceArr  (rData.aDisplayServiceArr),
    nDisplayServices    (rData.nDisplayServices),
    aAllServiceLocales  (rData.aAllServiceLocales),
    aCfgSpellTable      (rData.aCfgSpellTable),
    aCfgHyphTable       (rData.aCfgHyphTable),
    aCfgThesTable       (rData.aCfgThesTable),
    aCfgGrammarTable    (rData.aCfgGrammarTable),
    xMSF                (rData.xMSF),
    xLinguSrvcMgr       (rData.xLinguSrvcMgr)
{
}

SvxLinguData_Impl & SvxLinguData_Impl::operator = (const SvxLinguData_Impl &rData)
{
    xMSF                = rData.xMSF;
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
        ServiceInfo_Impl* pEntry;
        for (sal_uLong i = 0;  i < nDisplayServices;  ++i)
        {
            pEntry = &aDisplayServiceArr[i];
            if (pEntry  &&  !pEntry->bConfigured)
            {
                const OUString &rSrvcImplName = pConfiguredServices[n];
                if (!rSrvcImplName.isEmpty()  &&
                    (pEntry->sSpellImplName == rSrvcImplName  ||
                        pEntry->sGrammarImplName  == rSrvcImplName  ||
                        pEntry->sHyphImplName  == rSrvcImplName  ||
                        pEntry->sThesImplName  == rSrvcImplName))
                {
                    pEntry->bConfigured = sal_True;
                    break;
                }
            }
        }
    }
}

sal_Bool SvxLinguData_Impl::AddRemove(
            Sequence< OUString > &rConfigured,
            const OUString &rImplName, sal_Bool bAdd )
{
    sal_Bool bRet = sal_False;  // modified?

    sal_Int32 nEntries = rConfigured.getLength();
    sal_Int32 nPos = lcl_SeqGetEntryPos(rConfigured, rImplName);
    if (bAdd  &&  nPos < 0)         // add new entry
    {
        rConfigured.realloc( ++nEntries );
        OUString *pConfigured = rConfigured.getArray();
        pConfigured = rConfigured.getArray();
        pConfigured[nEntries - 1] = rImplName;
        bRet = sal_True;
    }
    else if (!bAdd  &&  nPos >= 0)  // remove existing entry
    {
        OUString *pConfigured = rConfigured.getArray();
        for (sal_Int32 i = nPos;  i < nEntries - 1;  ++i)
            pConfigured[i] = pConfigured[i + 1];
        rConfigured.realloc(--nEntries);
        bRet = sal_True;
    }

    return bRet;
}


void SvxLinguData_Impl::Reconfigure( const OUString &rDisplayName, sal_Bool bEnable )
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
                sal_Int16 nLang = SvxLocaleToLanguage( pLocale[i] );
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
                sal_Int16 nLang = SvxLocaleToLanguage( pLocale[i] );
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
                sal_Int16 nLang = SvxLocaleToLanguage( pLocale[i] );
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
                sal_Int16 nLang = SvxLocaleToLanguage( pLocale[i] );
                if (!aCfgThesTable.count( nLang ) && bEnable)
                    aCfgThesTable[ nLang ] = Sequence< OUString >();
                if (aCfgThesTable.count( nLang ))
                    AddRemove( aCfgThesTable[ nLang ], pInfo->sThesImplName, bEnable );
            }
        }
    }
}


// class SvxLinguTabPage -------------------------------------------------

SvxLinguTabPage::SvxLinguTabPage( Window* pParent,
                                  const SfxItemSet& rSet ):

    SfxTabPage( pParent, CUI_RES( RID_SFXPAGE_LINGU ), rSet ),

    aLinguisticFL       ( this, CUI_RES( FL_LINGUISTIC ) ),
    aLinguModulesFT     ( this, CUI_RES( FT_LINGU_MODULES ) ),
    aLinguModulesCLB    ( this, CUI_RES( CLB_LINGU_MODULES ) ),
    aLinguModulesEditPB ( this, CUI_RES( PB_LINGU_MODULES_EDIT ) ),
    aLinguDicsFT        ( this, CUI_RES( FT_LINGU_DICS ) ),
    aLinguDicsCLB       ( this, CUI_RES( CLB_LINGU_DICS ) ),
    aLinguDicsNewPB     ( this, CUI_RES( PB_LINGU_DICS_NEW_DIC ) ),
    aLinguDicsEditPB    ( this, CUI_RES( PB_LINGU_DICS_EDIT_DIC ) ),
    aLinguDicsDelPB     ( this, CUI_RES( PB_LINGU_DICS_DEL_DIC ) ),
    aLinguOptionsFT     ( this, CUI_RES( FT_LINGU_OPTIONS ) ),
    aLinguOptionsCLB    ( this, CUI_RES( CLB_LINGU_OPTIONS ) ),
    aLinguOptionsEditPB ( this, CUI_RES( PB_LINGU_OPTIONS_EDIT ) ),
    aMoreDictsLink      ( this, CUI_RES( FT_LINGU_OPTIONS_MOREDICTS ) ),
    sCapitalWords       ( CUI_RES( STR_CAPITAL_WORDS ) ),
    sWordsWithDigits    ( CUI_RES( STR_WORDS_WITH_DIGITS ) ),
    sCapitalization     ( CUI_RES( STR_CAPITALIZATION ) ),
    sSpellSpecial       ( CUI_RES( STR_SPELL_SPECIAL ) ),
    sSpellAuto          ( CUI_RES( STR_SPELL_AUTO ) ),
    sGrammarAuto        ( CUI_RES( STR_GRAMMAR_AUTO ) ),
    sNumMinWordlen      ( CUI_RES( STR_NUM_MIN_WORDLEN ) ),
    sNumPreBreak        ( CUI_RES( STR_NUM_PRE_BREAK ) ),
    sNumPostBreak       ( CUI_RES( STR_NUM_POST_BREAK ) ),
    sHyphAuto           ( CUI_RES( STR_HYPH_AUTO ) ),
    sHyphSpecial        ( CUI_RES( STR_HYPH_SPECIAL ) ),

    pLinguData          ( NULL )
{
    pCheckButtonData = NULL;

    aLinguModulesCLB.SetStyle( aLinguModulesCLB.GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguModulesCLB.SetHelpId(HID_CLB_LINGU_MODULES );
    aLinguModulesCLB.SetHighlightRange();
    aLinguModulesCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguModulesCLB.SetDoubleClickHdl(LINK(this, SvxLinguTabPage, BoxDoubleClickHdl_Impl));
    aLinguModulesCLB.SetCheckButtonHdl(LINK(this, SvxLinguTabPage, BoxCheckButtonHdl_Impl));

    aLinguModulesEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguOptionsEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));

    aLinguDicsCLB.SetStyle( aLinguDicsCLB.GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguDicsCLB.SetHelpId(HID_CLB_EDIT_MODULES_DICS );
    aLinguDicsCLB.SetHighlightRange();
    aLinguDicsCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguDicsCLB.SetCheckButtonHdl(LINK(this, SvxLinguTabPage, BoxCheckButtonHdl_Impl));

    aLinguDicsNewPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguDicsEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguDicsDelPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));

    aLinguOptionsCLB.SetStyle( aLinguOptionsCLB.GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguOptionsCLB.SetHelpId(HID_CLB_LINGU_OPTIONS );
    aLinguOptionsCLB.SetHighlightRange();
    aLinguOptionsCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguOptionsCLB.SetDoubleClickHdl(LINK(this, SvxLinguTabPage, BoxDoubleClickHdl_Impl));

    if ( SvtExtendedSecurityOptions().GetOpenHyperlinkMode()
            != SvtExtendedSecurityOptions::OPEN_NEVER )
    {
        aMoreDictsLink.SetURL( String(
            RTL_CONSTASCII_USTRINGPARAM( "http://extensions.libreoffice.org/dictionaries/" ) ) );
        aMoreDictsLink.SetClickHdl( LINK( this, SvxLinguTabPage, OpenURLHdl_Impl ) );
    }
    else
        aMoreDictsLink.Hide();

    String sAccessibleNameModuleEdit( CUI_RES( STR_LINGU_MODULES_EDIT ) );
    String sAccessibleNameDicsEdit  ( CUI_RES( STR_LINGU_DICS_EDIT_DIC ) );
    String sAccessibleNameOptionEdit( CUI_RES( STR_LINGU_OPTIONS_EDIT ) );

    aLinguModulesEditPB.SetAccessibleName(sAccessibleNameModuleEdit);
    aLinguDicsEditPB.SetAccessibleName(sAccessibleNameDicsEdit);
    aLinguOptionsEditPB.SetAccessibleName(sAccessibleNameOptionEdit);

    xProp = uno::Reference< XPropertySet >( SvxGetLinguPropertySet(), UNO_QUERY );
    xDicList = uno::Reference< XDictionaryList >( SvxGetDictionaryList(), UNO_QUERY );
    if (xDicList.is())
    {
        // keep references to all **currently** available dictionaries,
        // since the diclist may get changed meanwhile (e.g. through the API).
        // We want the dialog to operate on the same set of dictionaries it
        // was started with.
        // Also we have to take care to not loose the last reference when
        // someone else removes a dictionary from the list.
        // removed dics will be replaced by NULL new entries be added to the end
        // Thus we may use indizes as consistent references.
        aDics = xDicList->getDictionaries();

        UpdateDicBox_Impl();
    }
    else
    {
        aLinguDicsFT.Disable();
        aLinguDicsCLB.Disable();
        aLinguDicsNewPB.Disable();
        aLinguDicsEditPB.Disable();
        aLinguDicsDelPB.Disable();
    }

    const SfxSpellCheckItem* pItem = 0;
    SfxItemState eItemState = SFX_ITEM_UNKNOWN;

    eItemState = rSet.GetItemState( GetWhich( SID_ATTR_SPELL ),
                                    sal_False, (const SfxPoolItem**)&pItem );

    // handelt es sich um ein Default-Item?
    if ( eItemState == SFX_ITEM_DEFAULT )
        pItem = (const SfxSpellCheckItem*)&(rSet.Get( GetWhich( SID_ATTR_SPELL ) ) );
    else if ( eItemState == SFX_ITEM_DONTCARE )
        pItem = NULL;

    FreeResource();
}

// -----------------------------------------------------------------------

SvxLinguTabPage::~SvxLinguTabPage()
{
    if (pLinguData)
        delete pLinguData;
}

//------------------------------------------------------------------------

// don't throw away overloaded
sal_uInt16* SvxLinguTabPage::GetRanges()
{
    //TL???
    return pRanges;
}

//------------------------------------------------------------------------

SfxTabPage* SvxLinguTabPage::Create( Window* pParent,
                                     const SfxItemSet& rAttrSet )
{
    return ( new SvxLinguTabPage( pParent, rAttrSet ) );
}

//------------------------------------------------------------------------

sal_Bool SvxLinguTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    sal_Bool bModified = sal_True; // !!!!

    // if not HideGroups was called with GROUP_MODULES...
    if (aLinguModulesCLB.IsVisible())
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
            uno::Reference< XLinguServiceManager > xMgr( pLinguData->GetManager() );
            Locale aLocale( SvxCreateLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cSpell, aLocale, aImplNames );
        }

        // update grammar checker configuration entries
        pTable = &pLinguData->GetGrammarTable();
        for (aIt = pTable->begin();  aIt != pTable->end();  ++aIt)
        {
            sal_Int16 nLang = aIt->first;
            const Sequence< OUString > aImplNames( aIt->second );
            uno::Reference< XLinguServiceManager > xMgr( pLinguData->GetManager() );
            Locale aLocale( SvxCreateLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cGrammar, aLocale, aImplNames );
        }

        // update hyphenator configuration entries
        pTable = &pLinguData->GetHyphTable();
        for (aIt = pTable->begin();  aIt != pTable->end();  ++aIt)
        {
            sal_Int16 nLang = aIt->first;
            const Sequence< OUString > aImplNames( aIt->second );
            uno::Reference< XLinguServiceManager > xMgr( pLinguData->GetManager() );
            Locale aLocale( SvxCreateLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cHyph, aLocale, aImplNames );
        }

        // update thesaurus configuration entries
        pTable = &pLinguData->GetThesTable();
        for (aIt = pTable->begin();  aIt != pTable->end();  ++aIt)
        {
            sal_Int16 nLang = aIt->first;
            const Sequence< OUString > aImplNames( aIt->second );
            uno::Reference< XLinguServiceManager > xMgr( pLinguData->GetManager() );
            Locale aLocale( SvxCreateLocale(nLang) );
            if (xMgr.is())
                xMgr->setConfiguredServices( cThes, aLocale, aImplNames );
        }
    }


    //
    // activate dictionaries according to checkbox state
    //
    Sequence< OUString > aActiveDics;
    sal_Int32 nActiveDics = 0;
    sal_uLong nEntries = aLinguDicsCLB.GetEntryCount();
    for (sal_uLong i = 0;  i < nEntries;  ++i)
    {
        sal_Int32 nDics = aDics.getLength();

        aActiveDics.realloc( nDics );
        OUString *pActiveDic = aActiveDics.getArray();

        SvLBoxEntry *pEntry = aLinguDicsCLB.GetEntry( i );
        if (pEntry)
        {
            DicUserData aData( (sal_uLong)pEntry->GetUserData() );
            if (aData.GetEntryId() < nDics)
            {
                sal_Bool bChecked = aLinguDicsCLB.IsChecked( (sal_uInt16) i );
                uno::Reference< XDictionary > xDic( aDics.getConstArray()[ i ] );
                if (xDic.is())
                {
                    if (SvxGetIgnoreAllList() == xDic)
                        bChecked = sal_True;
                    xDic->setActive( bChecked );

                    if (bChecked)
                    {
                        String aDicName( xDic->getName() );
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


    nEntries = aLinguOptionsCLB.GetEntryCount();
    for (sal_uInt16 j = 0;  j < nEntries;  ++j)
    {
        SvLBoxEntry *pEntry = aLinguOptionsCLB.GetEntry( j );

        OptionsUserData aData( (sal_uLong)pEntry->GetUserData() );
        String aPropName( lcl_GetPropertyName( (EID_OPTIONS) aData.GetEntryId() ) );

        Any aAny;
        if (aData.IsCheckable())
        {
            sal_Bool bChecked = aLinguOptionsCLB.IsChecked( j );
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

    SvLBoxEntry *pPreBreakEntry  = aLinguOptionsCLB.GetEntry( (sal_uInt16) EID_NUM_PRE_BREAK );
    SvLBoxEntry *pPostBreakEntry = aLinguOptionsCLB.GetEntry( (sal_uInt16) EID_NUM_POST_BREAK );
    DBG_ASSERT( pPreBreakEntry, "NULL Pointer" );
    DBG_ASSERT( pPostBreakEntry, "NULL Pointer" );
    if (pPreBreakEntry && pPostBreakEntry)
    {
        OptionsUserData aPreBreakData( (sal_uLong)pPreBreakEntry->GetUserData() );
        OptionsUserData aPostBreakData( (sal_uLong)pPostBreakEntry->GetUserData() );
        if ( aPreBreakData.IsModified() || aPostBreakData.IsModified() )
        {
            SfxHyphenRegionItem aHyp( GetWhich( SID_ATTR_HYPHENREGION ) );
            aHyp.GetMinLead()  = (sal_uInt8) aPreBreakData.GetNumericValue();
            aHyp.GetMinTrail() = (sal_uInt8) aPostBreakData.GetNumericValue();
            rCoreSet.Put( aHyp );
        }
    }


    // automatic spell checking
    sal_Bool bNewAutoCheck = aLinguOptionsCLB.IsChecked( (sal_uInt16) EID_SPELL_AUTO );
    const SfxPoolItem* pOld = GetOldItem( rCoreSet, SID_AUTOSPELL_CHECK );
    if ( !pOld || ( (SfxBoolItem*)pOld )->GetValue() != bNewAutoCheck )
    {
        rCoreSet.Put( SfxBoolItem( GetWhich( SID_AUTOSPELL_CHECK ),
                                bNewAutoCheck ) );
        bModified |= sal_True;
    }

    return bModified;
}

// ----------------------------------------------------------------------

sal_uLong SvxLinguTabPage::GetDicUserData( const uno::Reference< XDictionary > &rxDic, sal_uInt16 nIdx )
{
    sal_uLong nRes = 0;
    DBG_ASSERT( rxDic.is(), "dictionary not supplied" );
    if (rxDic.is())
    {
        uno::Reference< frame::XStorable > xStor( rxDic, UNO_QUERY );

        sal_Bool bChecked = rxDic->isActive();
        sal_Bool bEditable = !xStor.is() || !xStor->isReadonly();
        sal_Bool bDeletable = bEditable;

        nRes = DicUserData( nIdx,
                bChecked, bEditable, bDeletable ).GetUserData();
    }
    return nRes;
}


void SvxLinguTabPage::AddDicBoxEntry(
        const uno::Reference< XDictionary > &rxDic,
        sal_uInt16 nIdx )
{
    aLinguDicsCLB.SetUpdateMode(sal_False);

    String aTxt( ::GetDicInfoStr( rxDic->getName(),
                        SvxLocaleToLanguage( rxDic->getLocale() ),
                        DictionaryType_NEGATIVE == rxDic->getDictionaryType() ) );
    aLinguDicsCLB.InsertEntry( aTxt, (sal_uInt16)LISTBOX_APPEND );  // append at end
    SvLBoxEntry* pEntry = aLinguDicsCLB.GetEntry( aLinguDicsCLB.GetEntryCount() - 1 );
    DBG_ASSERT( pEntry, "failed to add entry" );
    if (pEntry)
    {
        DicUserData aData( GetDicUserData( rxDic, nIdx ) );
        pEntry->SetUserData( (void *) aData.GetUserData() );
        lcl_SetCheckButton( pEntry, aData.IsChecked() );
    }

    aLinguDicsCLB.SetUpdateMode(sal_True);
}

// ----------------------------------------------------------------------

void SvxLinguTabPage::UpdateDicBox_Impl()
{
    aLinguDicsCLB.SetUpdateMode(sal_False);
    aLinguDicsCLB.Clear();

    sal_Int32 nDics  = aDics.getLength();
    const uno::Reference< XDictionary > *pDic = aDics.getConstArray();
    for (sal_Int32 i = 0;  i < nDics;  ++i)
    {
        const uno::Reference< XDictionary > &rDic = pDic[i];
        if (rDic.is())
            AddDicBoxEntry( rDic, (sal_uInt16)i );
    }

    aLinguDicsCLB.SetUpdateMode(sal_True);
}

// ----------------------------------------------------------------------

void SvxLinguTabPage::UpdateModulesBox_Impl()
{
    if (pLinguData)
    {
        const ServiceInfoArr &rAllDispSrvcArr = pLinguData->GetDisplayServiceArray();
        const sal_uLong nDispSrvcCount = pLinguData->GetDisplayServiceCount();

        aLinguModulesCLB.Clear();

        for (sal_uInt16 i = 0;  i < nDispSrvcCount;  ++i)
        {
            const ServiceInfo_Impl &rInfo = rAllDispSrvcArr[i];
            aLinguModulesCLB.InsertEntry( rInfo.sDisplayName, (sal_uInt16)LISTBOX_APPEND );
            SvLBoxEntry* pEntry = aLinguModulesCLB.GetEntry(i);
            pEntry->SetUserData( (void *) &rInfo );
            aLinguModulesCLB.CheckEntryPos( i, rInfo.bConfigured );
        }
        aLinguModulesEditPB.Enable( nDispSrvcCount > 0 );
    }
}

//------------------------------------------------------------------------

void SvxLinguTabPage::Reset( const SfxItemSet& rSet )
{
    // if not HideGroups was called with GROUP_MODULES...
    if (aLinguModulesCLB.IsVisible())
    {
        if (!pLinguData)
            pLinguData = new SvxLinguData_Impl;
        UpdateModulesBox_Impl();
    }


    //
    //  get data from configuration
    //

    SvtLinguConfig aLngCfg;

    aLinguOptionsCLB.SetUpdateMode(sal_False);
    aLinguOptionsCLB.Clear();

    SvLBoxTreeList *pModel = aLinguOptionsCLB.GetModel();
    SvLBoxEntry* pEntry = NULL;

    sal_Int16 nVal = 0;
    sal_Bool  bVal  = sal_False;
    sal_uLong nUserData = 0;

    pEntry = CreateEntry( sSpellAuto,       CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_AUTO ) >>= bVal;
    const SfxPoolItem* pItem = GetItem( rSet, SID_AUTOSPELL_CHECK );
    if (pItem)
        bVal = ((SfxBoolItem *) pItem)->GetValue();
    nUserData = OptionsUserData( EID_SPELL_AUTO, sal_False, 0, sal_True, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sGrammarAuto,       CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_GRAMMAR_AUTO ) >>= bVal;
    nUserData = OptionsUserData( EID_GRAMMAR_AUTO, sal_False, 0, sal_True, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sCapitalWords,    CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_UPPER_CASE ) >>= bVal;
    nUserData = OptionsUserData( EID_CAPITAL_WORDS, sal_False, 0, sal_True, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sWordsWithDigits, CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_WITH_DIGITS ) >>= bVal;
    nUserData = OptionsUserData( EID_WORDS_WITH_DIGITS, sal_False, 0, sal_True, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sCapitalization,  CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_CAPITALIZATION ) >>= bVal;
    nUserData = OptionsUserData( EID_CAPITALIZATION, sal_False, 0, sal_True, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sSpellSpecial,    CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_SPELL_SPECIAL ) >>= bVal;
    nUserData = OptionsUserData( EID_SPELL_SPECIAL, sal_False, 0, sal_True, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sNumMinWordlen,   CBCOL_SECOND );
    aLngCfg.GetProperty( UPN_HYPH_MIN_WORD_LENGTH ) >>= nVal;
    nUserData = OptionsUserData( EID_NUM_MIN_WORDLEN, sal_True, (sal_uInt16)nVal, sal_False, sal_False).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );

    const SfxHyphenRegionItem *pHyp = NULL;
    sal_uInt16 nWhich = GetWhich( SID_ATTR_HYPHENREGION );
    if ( rSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_SET )
        pHyp = &( (const SfxHyphenRegionItem &) rSet.Get( nWhich ) );

    pEntry = CreateEntry( sNumPreBreak,     CBCOL_SECOND );
    aLngCfg.GetProperty( UPN_HYPH_MIN_LEADING ) >>= nVal;
    if (pHyp)
        nVal = (sal_Int16) pHyp->GetMinLead();
    nUserData = OptionsUserData( EID_NUM_PRE_BREAK, sal_True, (sal_uInt16)nVal, sal_False, sal_False).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );

    pEntry = CreateEntry( sNumPostBreak,    CBCOL_SECOND );
    aLngCfg.GetProperty( UPN_HYPH_MIN_TRAILING ) >>= nVal;
    if (pHyp)
        nVal = (sal_Int16) pHyp->GetMinTrail();
    nUserData = OptionsUserData( EID_NUM_POST_BREAK, sal_True, (sal_uInt16)nVal, sal_False, sal_False).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );

    pEntry = CreateEntry( sHyphAuto,        CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_HYPH_AUTO ) >>= bVal;
    nUserData = OptionsUserData( EID_HYPH_AUTO, sal_False, 0, sal_True, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sHyphSpecial,     CBCOL_FIRST );
    aLngCfg.GetProperty( UPN_IS_HYPH_SPECIAL ) >>= bVal;
    nUserData = OptionsUserData( EID_HYPH_SPECIAL, sal_False, 0, sal_True, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    aLinguOptionsCLB.SetUpdateMode(sal_True);
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, BoxDoubleClickHdl_Impl, SvTreeListBox *, pBox )
{
    if (pBox == &aLinguModulesCLB)
    {
        //! in order to avoid a bug causing a GPF when double clicking
        //! on a module entry and exiting the "Edit Modules" dialog
        //! after that.
        Application::PostUserEvent( LINK(
                    this, SvxLinguTabPage, PostDblClickHdl_Impl ) );
    }
    else if (pBox == &aLinguOptionsCLB)
    {
        ClickHdl_Impl(&aLinguOptionsEditPB);
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLinguTabPage, PostDblClickHdl_Impl)
{
    ClickHdl_Impl(&aLinguModulesEditPB);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxLinguTabPage, OpenURLHdl_Impl)
{
    ::rtl::OUString sURL( aMoreDictsLink.GetURL() );
    lcl_OpenURL( sURL );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, BoxCheckButtonHdl_Impl, SvTreeListBox *, pBox )
{
    if (pBox == &aLinguModulesCLB)
    {
        DBG_ASSERT( pLinguData, "NULL pointer, LinguData missing" );
        sal_uInt16 nPos = aLinguModulesCLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND  &&  pLinguData)
        {
            pLinguData->Reconfigure( aLinguModulesCLB.GetText( nPos ),
                                     aLinguModulesCLB.IsChecked( nPos ) );
        }
    }
    else if (pBox == &aLinguDicsCLB)
    {
        sal_uInt16 nPos = aLinguDicsCLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
        {
            const uno::Reference< XDictionary > &rDic = aDics.getConstArray()[ nPos ];
            if (SvxGetIgnoreAllList() == rDic)
            {
                SvLBoxEntry* pEntry = aLinguDicsCLB.GetEntry( nPos );
                if (pEntry)
                    lcl_SetCheckButton( pEntry, sal_True );
            }
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, ClickHdl_Impl, PushButton *, pBtn )
{
    if (&aLinguModulesEditPB == pBtn)
    {
        if (!pLinguData)
            pLinguData = new SvxLinguData_Impl;

        SvxLinguData_Impl   aOldLinguData( *pLinguData );
        SvxEditModulesDlg   aDlg( this, *pLinguData );
        if (aDlg.Execute() != RET_OK)
            *pLinguData = aOldLinguData;

        // evaluate new status of 'bConfigured' flag
        sal_uLong nLen = pLinguData->GetDisplayServiceCount();
        for (sal_uLong i = 0;  i < nLen;  ++i)
            pLinguData->GetDisplayServiceArray()[i].bConfigured = sal_False;
        const Locale* pAllLocales = pLinguData->GetAllSupportedLocales().getConstArray();
        sal_Int32 nLocales = pLinguData->GetAllSupportedLocales().getLength();
        for (sal_Int32 k = 0;  k < nLocales;  ++k)
        {
            sal_Int16 nLang = SvxLocaleToLanguage( pAllLocales[k] );
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
    else if (&aLinguDicsNewPB == pBtn)
    {
        uno::Reference< XSpellChecker1 > xSpellChecker1;
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        if(pFact)
        {
            AbstractSvxNewDictionaryDialog* aDlg = pFact->CreateSvxNewDictionaryDialog( this, xSpellChecker1, RID_SFXDLG_NEWDICT );
            DBG_ASSERT(aDlg, "Dialogdiet fail!");
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
            delete aDlg;
        }
    }
    else if (&aLinguDicsEditPB == pBtn)
    {
        SvLBoxEntry *pEntry = aLinguDicsCLB.GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( (sal_uLong) pEntry->GetUserData() );
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
                        VclAbstractDialog* aDlg = pFact->CreateSvxEditDictionaryDialog( this, xDic->getName(), xSpellChecker1, RID_SFXDLG_EDITDICT );
                        DBG_ASSERT(aDlg, "Dialogdiet fail!");
                        aDlg->Execute();
                        delete aDlg;
                    }
                }
            }
        }
    }
    else if (&aLinguDicsDelPB == pBtn)
    {
        if ( RET_NO ==
             QueryBox( this, CUI_RES( RID_SFXQB_DELDICT ) ).Execute() )
            return 0;

        SvLBoxEntry *pEntry = aLinguDicsCLB.GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( (sal_uLong) pEntry->GetUserData() );
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
                            String sURL = xStor->getLocation();
                            INetURLObject aObj(sURL);
                            DBG_ASSERT( aObj.GetProtocol() == INET_PROT_FILE,
                                    "non-file URLs cannot be deleted" );
                            if ( aObj.GetProtocol() == INET_PROT_FILE )
                            {
                                KillFile_Impl( aObj.GetMainURL( INetURLObject::NO_DECODE ) );
                            }
                        }

                        aDics.getArray()[ nDicPos ] = 0;

                        // remove entry from checklistbox
                        sal_uLong nCnt = aLinguDicsCLB.GetEntryCount();
                        for (sal_uLong i = 0;  i < nCnt;  ++i)
                        {
                            SvLBoxEntry *pDicEntry = aLinguDicsCLB.GetEntry( i );
                            DBG_ASSERT( pDicEntry, "missing entry" );
                            if (pDicEntry)
                            {
                                DicUserData aDicData( (sal_uLong) pDicEntry->GetUserData() );
                                if (aDicData.GetEntryId() == nDicPos )
                                {
                                    aLinguDicsCLB.RemoveEntry( (sal_uInt16) i );
                                    break;
                                }
                            }
                        }
                        DBG_ASSERT( nCnt > aLinguDicsCLB.GetEntryCount(),
                                "remove failed ?");
                    }
                }
            }
        }
    }
    else if (&aLinguOptionsEditPB == pBtn)
    {
        SvLBoxEntry *pEntry = aLinguOptionsCLB.GetCurEntry();
        DBG_ASSERT( pEntry, "no entry selected" );
        if (pEntry)
        {
            OptionsUserData aData( (sal_uLong)pEntry->GetUserData() );
            if(aData.HasNumericValue())
            {
                int nRID = -1;
                switch (aData.GetEntryId())
                {
                    case EID_NUM_PRE_BREAK  : nRID = STR_NUM_PRE_BREAK_DLG; break;
                    case EID_NUM_POST_BREAK : nRID = STR_NUM_POST_BREAK_DLG; break;
                    case EID_NUM_MIN_WORDLEN: nRID = STR_NUM_MIN_WORDLEN_DLG; break;
                    default:
                        OSL_FAIL( "unexpected case" );
                }

                OptionsBreakSet aDlg( this, nRID );
                aDlg.GetNumericFld().SetValue( aData.GetNumericValue() );
                if (RET_OK == aDlg.Execute() )
                {
                    long nVal = static_cast<long>(aDlg.GetNumericFld().GetValue());
                    if (-1 != nVal && aData.GetNumericValue() != nVal)
                    {
                        aData.SetNumericValue( (sal_uInt8)nVal ); //! sets IsModified !
                        pEntry->SetUserData( (void *) aData.GetUserData() );
                        aLinguOptionsCLB.Invalidate();
                    }
                }
            }
        }
    }
    else
    {
        OSL_FAIL( "pBtn unexpected value" );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, SelectHdl_Impl, SvxCheckListBox *, pBox )
{
    if (&aLinguModulesCLB == pBox)
    {
    }
    else if (&aLinguDicsCLB == pBox)
    {
        SvLBoxEntry *pEntry = pBox->GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( (sal_uLong) pEntry->GetUserData() );

            // always allow to edit (i.e. at least view the content of the dictionary)
            aLinguDicsEditPB.Enable( true/*aData.IsEditable()*/ );
            aLinguDicsDelPB .Enable( aData.IsDeletable() );
        }
    }
    else if (&aLinguOptionsCLB == pBox)
    {
        SvLBoxEntry *pEntry = pBox->GetCurEntry();
        if (pEntry)
        {
            OptionsUserData aData( (sal_uLong) pEntry->GetUserData() );
            aLinguOptionsEditPB.Enable( aData.HasNumericValue() );
        }
    }
    else
    {
        OSL_FAIL( "pBox unexpected value" );
    }

    return 0;
}

// -----------------------------------------------------------------------

SvLBoxEntry* SvxLinguTabPage::CreateEntry( String& rTxt, sal_uInt16 nCol )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if( !pCheckButtonData )
        pCheckButtonData = new SvLBoxButtonData( &aLinguOptionsCLB );

    String sEmpty;
    if (CBCOL_FIRST == nCol)
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // empty column
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));
    pEntry->AddItem( new BrwString_Impl( pEntry, 0, rTxt ) );

    return pEntry;
}

// -----------------------------------------------------------------------

void SvxLinguTabPage::HideGroups( sal_uInt16 nGrp )
{
    if ( 0 != ( GROUP_MODULES & nGrp ) )
    {
        aLinguModulesFT.Hide();
        aLinguModulesCLB.Hide();
        aLinguModulesEditPB.Hide();

        // reposition / resize remaining controls
        long nDeltaY = aLinguDicsFT.GetPosPixel().Y() -
                       aLinguModulesFT.GetPosPixel().Y();
        DBG_ASSERT( nDeltaY >= 0, "move/resize value is negative" );
        Point   aPos;

        aPos = aLinguDicsFT.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguDicsFT.SetPosPixel( aPos );
        aPos = aLinguDicsCLB.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguDicsCLB.SetPosPixel( aPos );
        aPos = aLinguDicsNewPB.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguDicsNewPB.SetPosPixel( aPos );
        aPos = aLinguDicsEditPB.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguDicsEditPB.SetPosPixel( aPos );
        aPos = aLinguDicsDelPB.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguDicsDelPB.SetPosPixel( aPos );

        aPos = aLinguOptionsFT.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguOptionsFT.SetPosPixel( aPos );
        aPos = aLinguOptionsCLB.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguOptionsCLB.SetPosPixel( aPos );
        aPos = aLinguOptionsEditPB.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguOptionsEditPB.SetPosPixel( aPos );

        Size aSize( aLinguOptionsCLB.GetSizePixel() );
        aSize.Height() += nDeltaY;
        aLinguOptionsCLB.SetSizePixel( aSize );

        if ( SvtExtendedSecurityOptions().GetOpenHyperlinkMode()
                != SvtExtendedSecurityOptions::OPEN_NEVER )
        {
            aSize = GetOutputSizePixel();
            aSize.Height() += ( aMoreDictsLink.GetSizePixel().Height() * 11 / 8 );
            SetSizePixel( aSize );
            aMoreDictsLink.Show();
        }
    }
}

SvxEditModulesDlg::SvxEditModulesDlg(Window* pParent, SvxLinguData_Impl& rData) :
    ModalDialog( pParent, CUI_RES(RID_SVXDLG_EDIT_MODULES ) ),
    aModulesFL      ( this, CUI_RES( FL_EDIT_MODULES_OPTIONS ) ),
    aLanguageFT     ( this, CUI_RES( FT_EDIT_MODULES_LANGUAGE ) ),
    aLanguageLB     ( this, CUI_RES( LB_EDIT_MODULES_LANGUAGE ), sal_False ),
    aModulesCLB     ( this, CUI_RES( CLB_EDIT_MODULES_MODULES ) ),
    aPrioUpPB       ( this, CUI_RES( PB_EDIT_MODULES_PRIO_UP ) ),
    aPrioDownPB     ( this, CUI_RES( PB_EDIT_MODULES_PRIO_DOWN ) ),
    aBackPB         ( this, CUI_RES( PB_EDIT_MODULES_PRIO_BACK ) ),
    aMoreDictsLink  ( this, CUI_RES( FT_EDIT_MODULES_NEWDICTSLINK ) ),
    aButtonsFL      ( this, CUI_RES( FL_EDIT_MODULES_BUTTONS ) ),
    aHelpPB         ( this, CUI_RES( PB_HELP ) ),
    aClosePB        ( this, CUI_RES( PB_OK ) ),
    sSpell          (       CUI_RES( ST_SPELL ) ),
    sHyph           (       CUI_RES( ST_HYPH ) ),
    sThes           (       CUI_RES( ST_THES ) ),
    sGrammar        (       CUI_RES( ST_GRAMMAR ) ),
    rLinguData      ( rData )
{
    pCheckButtonData = NULL;
    FreeResource();

    pDefaultLinguData = new SvxLinguData_Impl( rLinguData );

    aModulesCLB.SetStyle( aModulesCLB.GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aModulesCLB.SetHighlightRange();
    aModulesCLB.SetHelpId(HID_CLB_EDIT_MODULES_MODULES );
    aModulesCLB.SetSelectHdl( LINK( this, SvxEditModulesDlg, SelectHdl_Impl ));
    aModulesCLB.SetCheckButtonHdl( LINK( this, SvxEditModulesDlg, BoxCheckButtonHdl_Impl) );

    aClosePB   .SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aPrioUpPB  .SetClickHdl( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    aPrioDownPB.SetClickHdl( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    aBackPB    .SetClickHdl( LINK( this, SvxEditModulesDlg, BackHdl_Impl ));
    // in case of not installed language modules
    aPrioUpPB  .Enable( sal_False );
    aPrioDownPB.Enable( sal_False );

    if ( SvtExtendedSecurityOptions().GetOpenHyperlinkMode()
            != SvtExtendedSecurityOptions::OPEN_NEVER )
    {
        aMoreDictsLink.SetURL( String(
            RTL_CONSTASCII_USTRINGPARAM( "http://extensions.libreoffice.org/dictionaries/" ) ) );
        aMoreDictsLink.SetClickHdl( LINK( this, SvxEditModulesDlg, OpenURLHdl_Impl ) );
    }
    else
    {
        aMoreDictsLink.Hide();
        long nPos = aMoreDictsLink.GetPosPixel().Y() + aMoreDictsLink.GetSizePixel().Height();
        Size aSize = aModulesCLB.GetSizePixel();
        aSize.Height() += ( nPos - ( aModulesCLB.GetPosPixel().Y() + aSize.Height() ) );
        aModulesCLB.SetSizePixel( aSize );
    }

    //
    //fill language box
    //
    Sequence< sal_Int16 > aAvailLang;
    uno::Reference< XAvailableLocales > xAvail( rLinguData.GetManager(), UNO_QUERY );
    if (xAvail.is())
    {
        aAvailLang = lcl_LocaleSeqToLangSeq(
                        xAvail->getAvailableLocales( cSpell ) );
    }
    const Sequence< Locale >& rLoc = rLinguData.GetAllSupportedLocales();
    const Locale* pLocales = rLoc.getConstArray();
    aLanguageLB.Clear();
    for(long i = 0; i < rLoc.getLength(); i++)
    {
        sal_Int16 nLang = SvxLocaleToLanguage( pLocales[i] );
        aLanguageLB.InsertLanguage( nLang, lcl_SeqHasLang( aAvailLang, nLang ) );
    }
    LanguageType eSysLang = MsLangId::getSystemLanguage();
    aLanguageLB.SelectLanguage( eSysLang );
    if(!aLanguageLB.IsLanguageSelected( eSysLang ) )
        aLanguageLB.SelectEntryPos(0);

    aLanguageLB.SetSelectHdl( LINK( this, SvxEditModulesDlg, LangSelectHdl_Impl ));
    LangSelectHdl_Impl(&aLanguageLB);
}


SvxEditModulesDlg::~SvxEditModulesDlg()
{
    delete pDefaultLinguData;
}


SvLBoxEntry* SvxEditModulesDlg::CreateEntry( String& rTxt, sal_uInt16 nCol )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;
    if( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData( &aModulesCLB );
        pCheckButtonData->SetLink( aModulesCLB.GetCheckButtonHdl() );
    }

    String sEmpty;
    if (CBCOL_FIRST == nCol)
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // empty column
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));
    pEntry->AddItem( new BrwStringDic_Impl( pEntry, 0, rTxt ) );

    return pEntry;
}

IMPL_LINK( SvxEditModulesDlg, SelectHdl_Impl, SvxCheckListBox *, pBox )
{
    if (&aModulesCLB == pBox)
    {
        sal_Bool bDisableUp = sal_True;
        sal_Bool bDisableDown = sal_True;
        SvLBoxEntry *pEntry = pBox->GetCurEntry();
        if (pEntry)
        {
            ModuleUserData_Impl* pData = (ModuleUserData_Impl*)pEntry->GetUserData();
            if(!pData->IsParent() && pData->GetType() != TYPE_HYPH)
            {
                sal_uInt16  nCurPos = pBox->GetSelectEntryPos();
                if(nCurPos < pBox->GetEntryCount() - 1)
                {
                    bDisableDown = ((ModuleUserData_Impl*)pBox->
                            GetEntry(nCurPos + 1)->GetUserData())->IsParent();
                }
                if(nCurPos > 1)
                {
                                bDisableUp = ((ModuleUserData_Impl*)pBox->
                            GetEntry(nCurPos - 1)->GetUserData())->IsParent();
                }
            }
            aPrioUpPB.Enable(!bDisableUp);
            aPrioDownPB.Enable(!bDisableDown);
        }
    }
    else
    {
        OSL_FAIL( "pBox unexpected value" );
    }

    return 0;
}

IMPL_LINK( SvxEditModulesDlg, BoxCheckButtonHdl_Impl, SvTreeListBox *, pBox )
{
        pBox = &aModulesCLB;
        SvLBoxEntry *pCurEntry = pBox->GetCurEntry();
        if (pCurEntry)
        {
            ModuleUserData_Impl* pData = (ModuleUserData_Impl *)
                                                pCurEntry->GetUserData();
            if (!pData->IsParent()  &&  pData->GetType() == TYPE_HYPH)
            {
                // make hyphenator checkboxes function as radio-buttons
                // (at most one box may be checked)
                SvLBoxEntry *pEntry = pBox->First();
                while (pEntry)
                {
                    pData = (ModuleUserData_Impl *) pEntry->GetUserData();
                    if (!pData->IsParent()  &&
                         pData->GetType() == TYPE_HYPH  &&
                         pEntry != pCurEntry)
                    {
                        lcl_SetCheckButton( pEntry, sal_False );
                        pBox->InvalidateEntry( pEntry );
                    }
                    pEntry = pBox->Next( pEntry );
                }
            }
        }
    return 0;
}

IMPL_LINK( SvxEditModulesDlg, LangSelectHdl_Impl, ListBox *, pBox )
{
    LanguageType  eCurLanguage = aLanguageLB.GetSelectLanguage();
    static Locale aLastLocale;
    Locale aCurLocale;
    SvxLanguageToLocale(aCurLocale, eCurLanguage);
    SvLBoxTreeList *pModel = aModulesCLB.GetModel();

    if (pBox)
    {
        // save old probably changed settings
        // before switching to new language entries

        sal_Int16 nLang = SvxLocaleToLanguage( aLastLocale );

        sal_Int32 nStart = 0, nLocalIndex = 0;
        Sequence< OUString > aChange;
        sal_Bool bChanged = sal_False;
        for(sal_uInt16 i = 0; i < aModulesCLB.GetEntryCount(); i++)
        {
            SvLBoxEntry *pEntry = aModulesCLB.GetEntry(i);
            ModuleUserData_Impl* pData = (ModuleUserData_Impl*)pEntry->GetUserData();
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
                aChange.realloc(aModulesCLB.GetEntryCount());
                bChanged = sal_False;
            }
            else
            {
                OUString* pChange = aChange.getArray();
                pChange[nStart] = pData->GetImplName();
                bChanged |= pData->GetIndex() != nLocalIndex ||
                    pData->IsChecked() != aModulesCLB.IsChecked(i);
                if(aModulesCLB.IsChecked(i))
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

    for(sal_uLong i = 0; i < aModulesCLB.GetEntryCount(); i++)
        delete (ModuleUserData_Impl*)aModulesCLB.GetEntry(i)->GetUserData();

    //
    // display entries for new selected language
    //
    aModulesCLB.Clear();
    if(LANGUAGE_DONTKNOW != eCurLanguage)
    {
        sal_uLong n;
        ServiceInfo_Impl* pInfo;

        //
        // spellchecker entries
        //
        SvLBoxEntry* pEntry = CreateEntry( sSpell,  CBCOL_SECOND );
        ModuleUserData_Impl* pUserData = new ModuleUserData_Impl(
                                         String(), sal_True, sal_False, TYPE_SPELL, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );

        Sequence< OUString > aNames( rLinguData.GetSortedImplNames( eCurLanguage, TYPE_SPELL ) );
        const OUString *pName = aNames.getConstArray();
        sal_uLong nNames = (sal_uLong) aNames.getLength();
        sal_Int32 nLocalIndex = 0;  // index relative to parent
        for (n = 0;  n < nNames;  ++n)
        {
            OUString aImplName;
            sal_Bool     bIsSuppLang = sal_False;

            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            if (pInfo)
            {
                bIsSuppLang = pInfo->xSpell.is()  &&
                              pInfo->xSpell->hasLocale( aCurLocale );
                aImplName = pInfo->sSpellImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                String aTxt( pInfo->sDisplayName );
                SvLBoxEntry* pNewEntry = CreateEntry( aTxt, CBCOL_FIRST );

                LangImplNameTable &rTable = rLinguData.GetSpellTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    DBG_WARNING( "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                lcl_SetCheckButton( pNewEntry, bCheck );
                pUserData = new ModuleUserData_Impl( aImplName, sal_False,
                                        bCheck, TYPE_SPELL, (sal_uInt8)nLocalIndex++ );
                pNewEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pNewEntry );
            }
        }

        //
        // grammar checker entries
        //
        pEntry = CreateEntry( sGrammar,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( String(), sal_True, sal_False, TYPE_GRAMMAR, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );

        aNames = rLinguData.GetSortedImplNames( eCurLanguage, TYPE_GRAMMAR );
        pName = aNames.getConstArray();
        nNames = (sal_uLong) aNames.getLength();
        nLocalIndex = 0;
        for (n = 0;  n < nNames;  ++n)
        {
            OUString aImplName;
            sal_Bool     bIsSuppLang = sal_False;

            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            if (pInfo)
            {
                bIsSuppLang = pInfo->xGrammar.is()  &&
                              pInfo->xGrammar->hasLocale( aCurLocale );
                aImplName = pInfo->sGrammarImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                String aTxt( pInfo->sDisplayName );
                SvLBoxEntry* pNewEntry = CreateEntry( aTxt, CBCOL_FIRST );

                LangImplNameTable &rTable = rLinguData.GetGrammarTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    DBG_WARNING( "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                lcl_SetCheckButton( pNewEntry, bCheck );
                pUserData = new ModuleUserData_Impl( aImplName, sal_False,
                                        bCheck, TYPE_GRAMMAR, (sal_uInt8)nLocalIndex++ );
                pNewEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pNewEntry );
            }
        }

        //
        // hyphenator entries
        //
        pEntry = CreateEntry( sHyph,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( String(), sal_True, sal_False, TYPE_HYPH, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );

        aNames = rLinguData.GetSortedImplNames( eCurLanguage, TYPE_HYPH );
        pName = aNames.getConstArray();
        nNames = (sal_uLong) aNames.getLength();
        nLocalIndex = 0;
        for (n = 0;  n < nNames;  ++n)
        {
            OUString aImplName;
            sal_Bool     bIsSuppLang = sal_False;

            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            if (pInfo)
            {
                bIsSuppLang = pInfo->xHyph.is()  &&
                              pInfo->xHyph->hasLocale( aCurLocale );
                aImplName = pInfo->sHyphImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                String aTxt( pInfo->sDisplayName );
                SvLBoxEntry* pNewEntry = CreateEntry( aTxt, CBCOL_FIRST );

                LangImplNameTable &rTable = rLinguData.GetHyphTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    DBG_WARNING( "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                lcl_SetCheckButton( pNewEntry, bCheck );
                pUserData = new ModuleUserData_Impl( aImplName, sal_False,
                                        bCheck, TYPE_HYPH, (sal_uInt8)nLocalIndex++ );
                pNewEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pNewEntry );
            }
        }

        //
        // thesaurus entries
        //
        pEntry = CreateEntry( sThes,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( String(), sal_True, sal_False, TYPE_THES, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );

        aNames = rLinguData.GetSortedImplNames( eCurLanguage, TYPE_THES );
        pName = aNames.getConstArray();
        nNames = (sal_uLong) aNames.getLength();
        nLocalIndex = 0;
        for (n = 0;  n < nNames;  ++n)
        {
            OUString aImplName;
            sal_Bool     bIsSuppLang = sal_False;

            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            if (pInfo)
            {
                bIsSuppLang = pInfo->xThes.is()  &&
                              pInfo->xThes->hasLocale( aCurLocale );
                aImplName = pInfo->sThesImplName;
            }
            if (!aImplName.isEmpty() && bIsSuppLang)
            {
                String aTxt( pInfo->sDisplayName );
                SvLBoxEntry* pNewEntry = CreateEntry( aTxt, CBCOL_FIRST );

                LangImplNameTable &rTable = rLinguData.GetThesTable();
                const bool bHasLang = rTable.count( eCurLanguage );
                if (!bHasLang)
                {
                    DBG_WARNING( "language entry missing" );    // only relevant if all languages found should be supported
                }
                const bool bCheck = bHasLang && lcl_SeqGetEntryPos( rTable[ eCurLanguage ], aImplName ) >= 0;
                lcl_SetCheckButton( pNewEntry, bCheck );
                pUserData = new ModuleUserData_Impl( aImplName, sal_False,
                                        bCheck, TYPE_THES, (sal_uInt8)nLocalIndex++ );
                pNewEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pNewEntry );
            }
        }
    }
    aLastLocale.Language = aCurLocale.Language;
    aLastLocale.Country = aCurLocale.Country;
    return 0;
}

IMPL_LINK( SvxEditModulesDlg, UpDownHdl_Impl, PushButton *, pBtn )
{
    sal_Bool bUp = &aPrioUpPB == pBtn;
    sal_uInt16  nCurPos = aModulesCLB.GetSelectEntryPos();
    SvLBoxEntry* pEntry;
    if (nCurPos != LISTBOX_ENTRY_NOTFOUND  &&
        0 != (pEntry = aModulesCLB.GetEntry(nCurPos)))
    {
        aModulesCLB.SetUpdateMode(sal_False);
        SvLBoxTreeList *pModel = aModulesCLB.GetModel();

        ModuleUserData_Impl* pData = (ModuleUserData_Impl*)pEntry->GetUserData();
        String aStr(aModulesCLB.GetEntryText(pEntry));
        SvLBoxEntry* pToInsert = CreateEntry( aStr, CBCOL_FIRST );
        pToInsert->SetUserData( (void *)pData);
        sal_Bool bIsChecked = aModulesCLB.IsChecked(nCurPos);

        pModel->Remove(pEntry);

        sal_uInt16 nDestPos = bUp ? nCurPos - 1 : nCurPos + 1;
        pModel->Insert(pToInsert, nDestPos);
        aModulesCLB.CheckEntryPos(nDestPos, bIsChecked );
        aModulesCLB.SelectEntryPos(nDestPos );
        SelectHdl_Impl(&aModulesCLB);
        aModulesCLB.SetUpdateMode(sal_True);
    }
    return 0;
}

IMPL_LINK( SvxEditModulesDlg, ClickHdl_Impl, PushButton *, pBtn )
{
    if (&aClosePB == pBtn)
    {
        // store language config
        LangSelectHdl_Impl(&aLanguageLB);
        EndDialog( RET_OK );
    }
    else
    {
        OSL_FAIL( "pBtn unexpected value" );
    }

    return 0;
}

IMPL_LINK_NOARG(SvxEditModulesDlg, BackHdl_Impl)
{
    rLinguData = *pDefaultLinguData;
    LangSelectHdl_Impl(0);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxEditModulesDlg, OpenURLHdl_Impl)
{
    ::rtl::OUString sURL( aMoreDictsLink.GetURL() );
    lcl_OpenURL( sURL );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
