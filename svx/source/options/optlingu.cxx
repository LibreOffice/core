/*************************************************************************
 *
 *  $RCSfile: optlingu.cxx,v $
 *
 *  $Revision: 1.46 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 11:39:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop
// include ---------------------------------------------------------------

#define _SVSTDARR_STRINGSDTOR
#define _SVSTDARR_USHORTS
#ifndef _SVSTDARR_HXX
#include <svtools/svstdarr.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _DYNARY_HXX
#include <tools/dynary.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _SVTOOLS_LINGUCFG_HXX_
#include <svtools/lingucfg.hxx>
#endif

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <dlgutil.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SFX_SFXUNO_HXX
#include <sfx2/sfxuno.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSPELLCHECKER_HPP_
#include <com/sun/star/linguistic2/XSpellChecker.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XTHESAURUS_HPP_
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XAVAILABLELOCALES_HPP_
#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEDISPLAYNAME_HPP_
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_DICTIONARYLISTEVENTFLAGS_HPP_
#include <com/sun/star/linguistic2/DictionaryListEventFlags.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_DICTIONARYLISTEVENT_HPP_
#include <com/sun/star/linguistic2/DictionaryListEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLISTEVENTLISTENER_HPP_
#include <com/sun/star/linguistic2/XDictionaryListEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_COMMANDABORTEDEXCEPTION_HPP_
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#endif
#ifndef _SVLBOX_HXX
#include <svtools/svlbox.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#include <sfx2/viewfrm.hxx>

#define _SVX_OPTLINGU_CXX

#include "optlingu.hrc"

#define ITEMID_SPELLCHECK   0
#define ITEMID_HYPHENREGION 0

#include "optdict.hxx"
#include "optitems.hxx"
#include "optlingu.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "helpid.hrc"

#include <ucbhelper/content.hxx>

using namespace ::ucb;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::beans;

#define C2U(cChar) OUString::createFromAscii(cChar)
#define SVX_MAX_USERDICTS 20
#define CBCOL_FIRST     0
#define CBCOL_SECOND    1
#define CBCOL_BOTH      2

static const sal_Char cSpell[] = "com.sun.star.linguistic2.SpellChecker";
static const sal_Char cHyph[] = "com.sun.star.linguistic2.Hyphenator";
static const sal_Char cThes[] = "com.sun.star.linguistic2.Thesaurus";

// static ----------------------------------------------------------------

static Sequence< INT16 > lcl_LocaleSeqToLangSeq( const Sequence< Locale > &rSeq )
{
    INT32 nLen = rSeq.getLength();
    Sequence< INT16 > aRes( nLen );
    INT16 *pRes = aRes.getArray();
    const Locale *pSeq = rSeq.getConstArray();
    for (INT32 i = 0;  i < nLen;  ++i)
    {
        pRes[i] = SvxLocaleToLanguage( pSeq[i] );
    }
    return aRes;
}


static BOOL lcl_SeqHasLang( const Sequence< INT16 > &rSeq, INT16 nLang )
{
    INT32 nLen = rSeq.getLength();
    const INT16 *pLang = rSeq.getConstArray();
    INT32 nPos = -1;
    for (INT32 i = 0;  i < nLen  &&  nPos < 0;  ++i)
    {
        if (nLang == pLang[i])
            nPos = i;
    }
    return nPos < 0 ? FALSE : TRUE;
}


static INT32 lcl_SeqGetEntryPos(
    const Sequence< OUString > &rSeq, const OUString &rEntry )
{
    INT32 i;
    INT32 nLen = rSeq.getLength();
    const OUString *pItem = rSeq.getConstArray();
    for (i = 0;  i < nLen;  ++i)
    {
        if (rEntry == pItem[i])
            break;
    }
    return i < nLen ? i : -1;
}

/*--------------------------------------------------
--------------------------------------------------*/

static const sal_uInt16 nNameLen = 8;

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
        Content aCnt( rURL, Reference< ::com::sun::star::ucb::XCommandEnvironment > () );
        aCnt.executeCommand( OUString::createFromAscii( "delete" ), makeAny( sal_Bool( sal_True ) ) );
    }
    catch( ::com::sun::star::ucb::CommandAbortedException& )
    {
        DBG_ERRORFILE( "KillFile: CommandAbortedException" );
        bRet = sal_False;
    }
    catch( ... )
    {
        DBG_ERRORFILE( "KillFile: Any other exception" );
        bRet = sal_False;
    }

    return bRet;
}
/* -----------------------------27.11.00 14:07--------------------------------

 ---------------------------------------------------------------------------*/
// 0x 0p 0t 0c nn
// p: 1 -> parent
// t: 1 -> spell, 2 -> hyph, 3 -> thes
// c: 1 -> checked 0 -> unchecked
// n: index

#define TYPE_SPELL  (BYTE)1
#define TYPE_HYPH   (BYTE)2
#define TYPE_THES   (BYTE)3

class ModuleUserData_Impl
{
    BOOL bParent;
    BOOL bIsChecked;
    BYTE nType;
    BYTE nIndex;
    String  sImplName;

public:
    ModuleUserData_Impl( String sImpName, BOOL bIsParent, BOOL bChecked, BYTE nSetType, BYTE nSetIndex ) :
        bParent(bIsParent),
        bIsChecked(bChecked),
        nType(nSetType),
        nIndex(nSetIndex),
        sImplName(sImpName)
        {
        }
    BOOL IsParent() const {return bParent;}
    BYTE GetType() const {return nType;}
    BOOL IsChecked() const {return bIsChecked;}
    BYTE GetIndex() const {return nIndex;}
    void SetIndex(BYTE nSet)  {nIndex = nSet;}
    const String& GetImplName() const {return sImplName;}

};

/*--------------------------------------------------
--------------------------------------------------*/
class DicUserData
{
    ULONG   nVal;

public:
    DicUserData( ULONG nUserData ) : nVal( nUserData ) {}
    DicUserData( USHORT nEID,
                 BOOL bChecked, BOOL bEditable, BOOL bDeletable );

    ULONG   GetUserData() const         { return nVal; }
    USHORT  GetEntryId() const          { return (USHORT)(nVal >> 16); }
    BOOL    IsChecked() const           { return (BOOL)(nVal >>  8) & 0x01; }
    BOOL    IsEditable() const          { return (BOOL)(nVal >>  9) & 0x01; }
    BOOL    IsDeletable() const         { return (BOOL)(nVal >> 10) & 0x01; }

    void    SetChecked( BOOL bVal );
};


DicUserData::DicUserData(
        USHORT nEID,
        BOOL bChecked, BOOL bEditable, BOOL bDeletable )
{
    DBG_ASSERT( nEID < 65000, "Entry Id out of range" );
    nVal =  ((ULONG)(0xFFFF & nEID)         << 16) |
            ((ULONG)(bChecked ? 1 : 0)      <<  8) |
            ((ULONG)(bEditable ? 1 : 0)     <<  9) |
            ((ULONG)(bDeletable ? 1 : 0)    << 10);
}


void DicUserData::SetChecked( BOOL bVal )
{
    nVal &= ~(1UL << 8);
    nVal |=  (ULONG)(bVal ? 1 : 0) << 8;
}


// class BrwString_Impl -------------------------------------------------

void lcl_SetCheckButton( SvLBoxEntry* pEntry, BOOL bCheck )
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found")
    if (((SvLBoxItem*)pItem)->IsA() == SV_ITEM_ID_LBOXBUTTON)
    {
        if (bCheck)
            pItem->SetStateChecked();
        else
            pItem->SetStateUnchecked();
        //InvalidateEntry( pEntry );
    }
}


class BrwStringDic_Impl : public SvLBoxString
{
public:

    BrwStringDic_Impl( SvLBoxEntry* pEntry, USHORT nFlags,
        const String& rStr ) : SvLBoxString( pEntry, nFlags, rStr ) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
                                            SvLBoxEntry* pEntry);
};

void BrwStringDic_Impl::Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
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


/*--------------------------------------------------
--------------------------------------------------*/

class OptionsBreakSet : public ModalDialog
{
    OKButton        aOKPB;
    CancelButton    aCancelPB;
    FixedLine       aValFL;
    NumericField    aValNF;

public:
    OptionsBreakSet(Window* pParent, int nRID) :
            ModalDialog(pParent, ResId(RID_SVXDLG_LNG_ED_NUM_PREBREAK, DIALOG_MGR() )),
            aOKPB       (this, ResId(BT_OK_PREBREAK)),
            aCancelPB   (this, ResId(BT_CANCEL_PREBREAK)),
            aValFL      (this, ResId(FL_NUMVAL_PREBREAK)),
            aValNF      (this, ResId(ED_PREBREAK))
    {
        DBG_ASSERT( STR_NUM_PRE_BREAK_DLG   == nRID ||
                    STR_NUM_POST_BREAK_DLG  == nRID ||
                    STR_NUM_MIN_WORDLEN_DLG == nRID, "unexpected RID" );

        if (nRID != -1)
            aValFL.SetText( String( ResId(nRID) ) );
        FreeResource();
    }

    NumericField&   GetNumericFld() { return aValNF; }
};


/*--------------------------------------------------
    Entry IDs for options listbox of dialog
--------------------------------------------------*/

enum EID_OPTIONS
{
    EID_CAPITAL_WORDS,
    EID_WORDS_WITH_DIGITS,
    EID_CAPITALIZATION,
    EID_SPELL_SPECIAL,
    EID_ALL_LANGUAGES,
    EID_SPELL_AUTO,
    EID_HIDE_MARKINGS,
    EID_OLD_GERMAN,
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
    "IsSpellUpperCase",         // EID_CAPITAL_WORDS
    "IsSpellWithDigits",        // EID_WORDS_WITH_DIGITS
    "IsSpellCapitalization",    // EID_CAPITALIZATION
    "IsSpellSpecial",           // EID_SPELL_SPECIAL
    "IsSpellInAllLanguages",    // EID_ALL_LANGUAGES
    "IsSpellAuto",              // EID_SPELL_AUTO
    "IsSpellHide",              // EID_HIDE_MARKINGS
    "IsGermanPreReform",        // EID_OLD_GERMAN
    "HyphMinWordLength",        // EID_NUM_MIN_WORDLEN,
    "HyphMinLeading",           // EID_NUM_PRE_BREAK
    "HyphMinTrailing",          // EID_NUM_POST_BREAK
    "IsHyphAuto",               // EID_HYPH_AUTO
    "IsHyphSpecial"             // EID_HYPH_SPECIAL
};


static inline String lcl_GetPropertyName( EID_OPTIONS eEntryId )
{
    DBG_ASSERT( (int) eEntryId < sizeof(aEidToPropName) / sizeof(aEidToPropName[0]),
            "index out of range" );
    return String::CreateFromAscii( aEidToPropName[ (int) eEntryId ] );
}

// class OptionsUserData -------------------------------------------------

class OptionsUserData
{
    ULONG   nVal;

    void    SetModified();

public:
    OptionsUserData( ULONG nUserData ) : nVal( nUserData ) {}
    OptionsUserData( USHORT nEID,
                     BOOL bHasNV, USHORT nNumVal,
                     BOOL bCheckable, BOOL bChecked );

    ULONG   GetUserData() const         { return nVal; }
    USHORT  GetEntryId() const          { return (USHORT)(nVal >> 16); }
    BOOL    HasNumericValue() const     { return (BOOL)(nVal >> 10) & 0x01; }
    USHORT  GetNumericValue() const     { return (USHORT)(nVal & 0xFF); }
    BOOL    IsChecked() const           { return (BOOL)(nVal >> 8) & 0x01; }
    BOOL    IsCheckable() const         { return (BOOL)(nVal >> 9) & 0x01; }
    BOOL    IsModified() const          { return (BOOL)(nVal >> 11) & 0x01; }

    void    SetChecked( BOOL bVal );
    void    SetNumericValue( BYTE nNumVal );
};

OptionsUserData::OptionsUserData( USHORT nEID,
        BOOL bHasNV, USHORT nNumVal,
        BOOL bCheckable, BOOL bChecked )
{
    DBG_ASSERT( nEID < 65000, "Entry Id out of range" );
    DBG_ASSERT( nNumVal < 256, "value out of range" );
    nVal =  ((ULONG)(0xFFFF & nEID)         << 16) |
            ((ULONG)(bHasNV ? 1 : 0)        << 10) |
            ((ULONG)(bCheckable ? 1 : 0)    <<  9) |
            ((ULONG)(bChecked ? 1 : 0)      <<  8) |
            ((ULONG)(0xFF & nNumVal));
}

void OptionsUserData::SetChecked( BOOL bVal )
{
    if (IsCheckable()  &&  (IsChecked() != bVal))
    {
        nVal &= ~(1UL << 8);
        nVal |=  (ULONG)(bVal ? 1 : 0) << 8;
        SetModified();
    }
}

void OptionsUserData::SetNumericValue( BYTE nNumVal )
{
    DBG_ASSERT( nNumVal < 256, "value out of range" );
    if (HasNumericValue()  &&  (GetNumericValue() != nNumVal))
    {
        nVal &= 0xffffff00;
        nVal |= (nNumVal);
        SetModified();
    }
}

void OptionsUserData::SetModified()
{
    nVal |=  (ULONG)1 << 11;
}

// class BrwString_Impl -------------------------------------------------

class BrwString_Impl : public SvLBoxString
{
public:

    BrwString_Impl( SvLBoxEntry* pEntry, USHORT nFlags,
        const String& rStr ) : SvLBoxString( pEntry, nFlags, rStr ) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
                                            SvLBoxEntry* pEntry);
};

void BrwString_Impl::Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
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

        BOOL bFett = TRUE;
        USHORT nPos = 0;
        //??? das untere byte aus dem user data in string wandeln
        OptionsUserData aData( (ULONG) pEntry->GetUserData() );
        if(aData.HasNumericValue())
        {
            String sTxt( ' ' );
            sTxt +=  String::CreateFromInt32( aData.GetNumericValue() );
            rDev.SetFont( aFont );
            rDev.DrawText( aNewPos, sTxt );
        }

//          if( STRING_NOTFOUND != nPos )
//              aNewPos.X() += rDev.GetTextWidth( sTxt );

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
    Reference <XSpellChecker>   xSpell;
    Reference <XHyphenator>     xHyph;
    Reference <XThesaurus>      xThes;
    BOOL                        bConfigured;

    ServiceInfo_Impl() : bConfigured(sal_False) {}
};

DECLARE_DYNARRAY(ServiceInfoArr, ServiceInfo_Impl *);

DECLARE_TABLE( LangImplNameTable, Sequence< OUString > * );

// SvxLinguData_Impl ----------------------------------------------------

class SvxLinguData_Impl
{
    //contains services and implementation names sorted by implementation names
    ServiceInfoArr                      aDisplayServiceArr;
    ULONG                               nDisplayServices;

    Sequence< Locale >                  aAllServiceLocales;
    LangImplNameTable                   aCfgSpellTable;
    LangImplNameTable                   aCfgHyphTable;
    LangImplNameTable                   aCfgThesTable;
    Reference< XMultiServiceFactory >   xMSF;
    Reference< XLinguServiceManager >   xLinguSrvcMgr;


    sal_Bool    AddRemove( Sequence< OUString > &rConfigured,
                           const OUString &rImplName, BOOL bAdd );

public:
    SvxLinguData_Impl();
    SvxLinguData_Impl( const SvxLinguData_Impl &rData );
    ~SvxLinguData_Impl();

    SvxLinguData_Impl & operator = (const SvxLinguData_Impl &rData);

    Reference<XLinguServiceManager> &   GetManager() { return xLinguSrvcMgr; }

    void SetChecked( const Sequence< OUString > &rConfiguredServices );
    void Reconfigure( const OUString &rDisplayName, BOOL bEnable );

    const Sequence<Locale> &    GetAllSupportedLocales() { return aAllServiceLocales; }

    const LangImplNameTable &   GetSpellTable() const   { return aCfgSpellTable; }
    LangImplNameTable &         GetSpellTable()         { return aCfgSpellTable; }
    const LangImplNameTable &   GetHyphTable() const    { return aCfgHyphTable; }
    LangImplNameTable &         GetHyphTable()          { return aCfgHyphTable; }
    const LangImplNameTable &   GetThesTable() const    { return aCfgThesTable; }
    LangImplNameTable &         GetThesTable()          { return aCfgThesTable; }

    const ServiceInfoArr &      GetDisplayServiceArray() const  { return aDisplayServiceArr; }
    ServiceInfoArr &            GetDisplayServiceArray()        { return aDisplayServiceArr; }

    const ULONG &   GetDisplayServiceCount() const          { return nDisplayServices; }
    void            SetDisplayServiceCount( ULONG nVal )    { nDisplayServices = nVal; }

    // returns the list of service implementation names for the specified
    // language and service (TYPE_SPELL, TYPE_HYPH, TYPE_THES) sorted in
    // the proper order for the SvxEditModulesDlg (the ones from the
    // configuration (keeping that order!) first and then the other ones.
    // I.e. the ones available but not configured in arbitrary order).
    // They available ones may contain names that do not(!) support that
    // language.
    Sequence< OUString > GetSortedImplNames( INT16 nLang, BYTE nType );

    ServiceInfo_Impl * GetInfoByImplName( const OUString &rSvcImplName );
};


INT32 lcl_SeqGetIndex( const Sequence< OUString > &rSeq, const OUString &rTxt )
{
    INT32 nRes = -1;
    INT32 nLen = rSeq.getLength();
    const OUString *pString = rSeq.getConstArray();
    for (INT32 i = 0;  i < nLen  &&  nRes == -1;  ++i)
    {
        if (pString[i] == rTxt)
            nRes = i;
    }
    return nRes;
}


Sequence< OUString > SvxLinguData_Impl::GetSortedImplNames( INT16 nLang, BYTE nType )
{
    LangImplNameTable *pTable = 0;
    switch (nType)
    {
        case TYPE_SPELL : pTable = &aCfgSpellTable; break;
        case TYPE_HYPH  : pTable = &aCfgHyphTable; break;
        case TYPE_THES  : pTable = &aCfgThesTable; break;
    }
    Sequence< OUString > aRes;
    const Sequence< OUString > *pCfgImplNames = pTable->Get( nLang );  // get configured services
    if (pCfgImplNames)
        aRes = *pCfgImplNames;      // add configured services
    INT32 nIdx = aRes.getLength();
    DBG_ASSERT( (INT32) nDisplayServices >= nIdx, "size mismatch" );
    aRes.realloc( nDisplayServices );
    OUString *pRes = aRes.getArray();

    // add not configured services
    for (INT32 i = 0;  i < (INT32) nDisplayServices;  ++i)
    {
        ServiceInfo_Impl* pInfo = aDisplayServiceArr.Get(i);
        OUString aImplName;
        switch (nType)
        {
            case TYPE_SPELL : aImplName = pInfo->sSpellImplName; break;
            case TYPE_HYPH  : aImplName = pInfo->sHyphImplName; break;
            case TYPE_THES  : aImplName = pInfo->sThesImplName; break;
        }

        if (aImplName.getLength()  &&  lcl_SeqGetIndex( aRes, aImplName) == -1)    // name not yet added
        {
            DBG_ASSERT( nIdx < aRes.getLength(), "index out of range" )
            if (nIdx < aRes.getLength())
                pRes[ nIdx++ ] = aImplName;
        }
    }

    return aRes;
}


ServiceInfo_Impl * SvxLinguData_Impl::GetInfoByImplName( const OUString &rSvcImplName )
{
    ServiceInfo_Impl* pInfo = 0;
    for (ULONG i = 0;  i < nDisplayServices  &&  !pInfo;  ++i)
    {
        ServiceInfo_Impl *pTmp = aDisplayServiceArr.Get(i);
        if (pTmp->sSpellImplName == rSvcImplName ||
            pTmp->sHyphImplName  == rSvcImplName ||
            pTmp->sThesImplName  == rSvcImplName)
            pInfo = pTmp;
    }
    return pInfo;
}


//-----------------------------------------------------------------------------

void lcl_MergeLocales(Sequence<Locale>& aAllLocales, const Sequence<Locale>& rAdd)
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
/* -----------------------------27.11.00 16:48--------------------------------

 ---------------------------------------------------------------------------*/
void lcl_MergeDisplayArray(
        SvxLinguData_Impl &rData,
        const ServiceInfo_Impl* pToAdd )
{
    ULONG nCnt = 0;

    ServiceInfoArr &rSvcInfoArr = rData.GetDisplayServiceArray();
    ULONG nEntries = rData.GetDisplayServiceCount();

    ServiceInfo_Impl* pEntry;
    for (ULONG i = 0;  i < nEntries;  ++i)
    {
        pEntry = rSvcInfoArr.Get(i);
        if (pEntry  &&  pEntry->sDisplayName == pToAdd->sDisplayName)
        {
            if(pToAdd->xSpell.is())
            {
                DBG_ASSERT( !pEntry->xSpell.is() &&
                            pEntry->sSpellImplName.getLength() == 0,
                            "merge conflict" )
                pEntry->sSpellImplName = pToAdd->sSpellImplName;
                pEntry->xSpell = pToAdd->xSpell;
            }
            if(pToAdd->xHyph.is())
            {
                DBG_ASSERT( !pEntry->xHyph.is() &&
                            pEntry->sHyphImplName.getLength() == 0,
                            "merge conflict" )
                pEntry->sHyphImplName = pToAdd->sHyphImplName;
                pEntry->xHyph = pToAdd->xHyph;
            }
            if(pToAdd->xThes.is())
            {
                DBG_ASSERT( !pEntry->xThes.is() &&
                            pEntry->sThesImplName.getLength() == 0,
                            "merge conflict" )
                pEntry->sThesImplName = pToAdd->sThesImplName;
                pEntry->xThes = pToAdd->xThes;
            }
            return ;
        }
        ++nCnt;
    }
    ServiceInfo_Impl* pInsert = new ServiceInfo_Impl(*pToAdd);
    rData.GetDisplayServiceArray().Put( nCnt, pInsert );
    rData.SetDisplayServiceCount( nCnt + 1 );
}
/* -----------------------------26.11.00 18:07--------------------------------

 ---------------------------------------------------------------------------*/
SvxLinguData_Impl::SvxLinguData_Impl() :
    nDisplayServices    (0)
{
    xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xI = xMSF->createInstance(
        C2U( "com.sun.star.linguistic2.LinguServiceManager" ) );
    xLinguSrvcMgr = Reference<XLinguServiceManager>(xI, UNO_QUERY);
    DBG_ASSERT(xLinguSrvcMgr.is(), "No linguistic service available!")
    if(xLinguSrvcMgr.is())
    {
        Locale aCurrentLocale;
        LanguageType eLang = Application::GetSettings().GetLanguage();
        SvxLanguageToLocale(aCurrentLocale, eLang);
        Sequence<Any> aArgs(2);//second arguments has to be empty!
        aArgs.getArray()[0] <<= SvxGetLinguPropertySet();

        //read spell checker
        Sequence< OUString > aSpellNames = xLinguSrvcMgr->getAvailableServices(
                        C2U(cSpell),    Locale() );
        const OUString* pSpellNames = aSpellNames.getConstArray();

        sal_Int32 nIdx;
        for(nIdx = 0; nIdx < aSpellNames.getLength(); nIdx++)
        {
            ServiceInfo_Impl* pInfo = new ServiceInfo_Impl;
            pInfo->sSpellImplName = pSpellNames[nIdx];
            pInfo->xSpell = Reference<XSpellChecker>(
                            xMSF->createInstanceWithArguments(pInfo->sSpellImplName, aArgs), UNO_QUERY);

            Reference<XServiceDisplayName> xDispName(pInfo->xSpell, UNO_QUERY);
            if(xDispName.is())
                pInfo->sDisplayName = xDispName->getServiceDisplayName( aCurrentLocale );

            const Sequence< Locale > aLocales( pInfo->xSpell->getLocales() );
            //! suppress display of entries with no supported languages (see feature 110994)
            if (aLocales.getLength())
            {
                lcl_MergeLocales( aAllServiceLocales, aLocales );
                lcl_MergeDisplayArray( *this, pInfo );
            }
        }

        //read hyphenator
        Sequence< OUString > aHyphNames = xLinguSrvcMgr->getAvailableServices(
                        C2U(cHyph), Locale() );
        const OUString* pHyphNames = aHyphNames.getConstArray();
        for(nIdx = 0; nIdx < aHyphNames.getLength(); nIdx++)
        {
            ServiceInfo_Impl* pInfo = new ServiceInfo_Impl;
            pInfo->sHyphImplName = pHyphNames[nIdx];
            pInfo->xHyph = Reference<XHyphenator>(
                            xMSF->createInstanceWithArguments(pInfo->sHyphImplName, aArgs), UNO_QUERY);

            Reference<XServiceDisplayName> xDispName(pInfo->xHyph, UNO_QUERY);
            if(xDispName.is())
                pInfo->sDisplayName = xDispName->getServiceDisplayName( aCurrentLocale );

            const Sequence< Locale > aLocales( pInfo->xHyph->getLocales() );
            //! suppress display of entries with no supported languages (see feature 110994)
            if (aLocales.getLength())
            {
                lcl_MergeLocales( aAllServiceLocales, aLocales );
                lcl_MergeDisplayArray( *this, pInfo );
            }
        }

        //read thesauri
        Sequence< OUString > aThesNames = xLinguSrvcMgr->getAvailableServices(
                        C2U(cThes),     Locale() );
        const OUString* pThesNames = aThesNames.getConstArray();
        for(nIdx = 0; nIdx < aThesNames.getLength(); nIdx++)
        {
            ServiceInfo_Impl* pInfo = new ServiceInfo_Impl;
            pInfo->sThesImplName = pThesNames[nIdx];
            pInfo->xThes = Reference<XThesaurus>(
                            xMSF->createInstanceWithArguments(pInfo->sThesImplName, aArgs), UNO_QUERY);

            Reference<XServiceDisplayName> xDispName(pInfo->xThes, UNO_QUERY);
            if(xDispName.is())
                pInfo->sDisplayName = xDispName->getServiceDisplayName( aCurrentLocale );

            const Sequence< Locale > aLocales( pInfo->xThes->getLocales() );
            //! suppress display of entries with no supported languages (see feature 110994)
            if (aLocales.getLength())
            {
                lcl_MergeLocales( aAllServiceLocales, aLocales );
                lcl_MergeDisplayArray( *this, pInfo );
            }
        }

        Sequence< OUString > aCfgSvcs;
        const Locale* pAllLocales = aAllServiceLocales.getConstArray();
        for(sal_Int32 nLocale = 0; nLocale < aAllServiceLocales.getLength(); nLocale++)
        {
            INT16 nLang = SvxLocaleToLanguage( pAllLocales[nLocale] );

            aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(C2U(cSpell), pAllLocales[nLocale]);
            SetChecked( aCfgSvcs );
            if (aCfgSvcs.getLength())
                aCfgSpellTable.Insert( nLang, new Sequence< OUString >(aCfgSvcs) );

            aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(C2U(cHyph), pAllLocales[nLocale]);
            SetChecked( aCfgSvcs );
            if (aCfgSvcs.getLength())
                aCfgHyphTable.Insert( nLang, new Sequence< OUString >(aCfgSvcs) );

            aCfgSvcs = xLinguSrvcMgr->getConfiguredServices(C2U(cThes), pAllLocales[nLocale]);
            SetChecked( aCfgSvcs );
            if (aCfgSvcs.getLength())
                aCfgThesTable.Insert( nLang, new Sequence< OUString >(aCfgSvcs) );
        }
    }
}
/* -----------------------------22.05.01 10:43--------------------------------

 ---------------------------------------------------------------------------*/
SvxLinguData_Impl::SvxLinguData_Impl( const SvxLinguData_Impl &rData ) :
    xMSF                (rData.xMSF),
    xLinguSrvcMgr       (rData.xLinguSrvcMgr),
    aAllServiceLocales  (rData.aAllServiceLocales),
    aDisplayServiceArr  (rData.aDisplayServiceArr),
    nDisplayServices    (rData.nDisplayServices)
{
}
/* -----------------------------22.05.01 10:43--------------------------------

 ---------------------------------------------------------------------------*/
SvxLinguData_Impl & SvxLinguData_Impl::operator = (const SvxLinguData_Impl &rData)
{
    xMSF                = rData.xMSF;
    xLinguSrvcMgr       = rData.xLinguSrvcMgr;
    aAllServiceLocales  = rData.aAllServiceLocales;
    aDisplayServiceArr  = rData.aDisplayServiceArr;
    nDisplayServices    = rData.nDisplayServices;
    return *this;
}
/* -----------------------------26.11.00 18:08--------------------------------

 ---------------------------------------------------------------------------*/
SvxLinguData_Impl::~SvxLinguData_Impl()
{
    USHORT k;
    aDisplayServiceArr.Clear();

    LangImplNameTable *pTable[3] =
    {
        &aCfgSpellTable, &aCfgHyphTable, &aCfgThesTable
    };
    for (USHORT i = 0;  i < 3;  ++i)
    {
        LangImplNameTable &rTable = *pTable[i];
        for (k = 0;  k < rTable.Count();  ++k)
            delete rTable.GetObject(k);
        rTable.Clear();
    }
}
/* -----------------------------26.11.00 19:42--------------------------------

 ---------------------------------------------------------------------------*/
void SvxLinguData_Impl::SetChecked(const Sequence<OUString>& rConfiguredServices)
{
    const OUString* pConfiguredServices = rConfiguredServices.getConstArray();
    for(sal_Int32 n = 0; n < rConfiguredServices.getLength(); n++)
    {
        ServiceInfo_Impl* pEntry;
        for (ULONG i = 0;  i < nDisplayServices;  ++i)
        {
            pEntry = aDisplayServiceArr.Get(i);
            if (pEntry  &&  !pEntry->bConfigured)
            {
                const OUString &rSrvcImplName = pConfiguredServices[n];
                if (rSrvcImplName.getLength()  &&
                    (pEntry->sSpellImplName == rSrvcImplName  ||
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
/* -----------------------------26.11.00 20:43--------------------------------

 ---------------------------------------------------------------------------*/

sal_Bool SvxLinguData_Impl::AddRemove(
            Sequence< OUString > &rConfigured,
            const OUString &rImplName, BOOL bAdd )
{
    sal_Bool bRet = sal_False;  // modified?

    INT32 nEntries = rConfigured.getLength();
    INT32 nPos = lcl_SeqGetEntryPos(rConfigured, rImplName);
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
        for (INT32 i = nPos;  i < nEntries - 1;  ++i)
            pConfigured[i] = pConfigured[i + 1];
        rConfigured.realloc(--nEntries);
        bRet = sal_True;
    }

    return bRet;
}


void SvxLinguData_Impl::Reconfigure( const OUString &rDisplayName, BOOL bEnable )
{
    DBG_ASSERT( rDisplayName.getLength(), "empty DisplayName" );

    ServiceInfo_Impl *pInfo = 0;
    ServiceInfo_Impl *pTmp;
    for (ULONG i = 0;  i < nDisplayServices;  ++i)
    {
        pTmp = aDisplayServiceArr.Get(i);
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
        INT32 nLocales = 0;
        INT32 i;

        // update configured spellchecker entries
        if (pInfo->xSpell.is())
        {
            aLocales = pInfo->xSpell->getLocales();
            pLocale = aLocales.getConstArray();
            nLocales = aLocales.getLength();
            for (i = 0;  i < nLocales;  ++i)
            {
                INT16 nLang = SvxLocaleToLanguage( pLocale[i] );
                Sequence< OUString > *pCfgImplNames = aCfgSpellTable.Get( nLang );
                if (!pCfgImplNames && bEnable)
                {
                    pCfgImplNames = new Sequence< OUString >();
                    aCfgSpellTable.Insert( nLang, pCfgImplNames );
                }
                if (pCfgImplNames)
                    AddRemove( *pCfgImplNames, pInfo->sSpellImplName, bEnable );
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
                INT16 nLang = SvxLocaleToLanguage( pLocale[i] );
                Sequence< OUString > *pCfgImplNames = aCfgHyphTable.Get( nLang );
                if (!pCfgImplNames && bEnable)
                {
                    pCfgImplNames = new Sequence< OUString >();
                    aCfgHyphTable.Insert( nLang, pCfgImplNames );
                }
                if (pCfgImplNames)
                    AddRemove( *pCfgImplNames, pInfo->sHyphImplName, bEnable );
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
                INT16 nLang = SvxLocaleToLanguage( pLocale[i] );
                Sequence< OUString > *pCfgImplNames = aCfgThesTable.Get( nLang );
                if (!pCfgImplNames && bEnable)
                {
                    pCfgImplNames = new Sequence< OUString >();
                    aCfgThesTable.Insert( nLang, pCfgImplNames );
                }
                if (pCfgImplNames)
                    AddRemove( *pCfgImplNames, pInfo->sThesImplName, bEnable );
            }
        }
    }
}


// class SvxLinguTabPage -------------------------------------------------

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1
#define CBCOL_BOTH      2

SvxLinguTabPage::SvxLinguTabPage( Window* pParent,
                                  const SfxItemSet& rSet ):

    SfxTabPage( pParent, ResId( RID_SFXPAGE_LINGU, DIALOG_MGR() ), rSet ),

    aLinguisticFL       ( this, ResId( FL_LINGUISTIC ) ),
    aLinguModulesFT     ( this, ResId( FT_LINGU_MODULES ) ),
    aLinguModulesCLB    ( this, ResId( CLB_LINGU_MODULES ) ),
    aLinguModulesEditPB ( this, ResId( PB_LINGU_MODULES_EDIT ) ),
    aLinguDicsFT        ( this, ResId( FT_LINGU_DICS ) ),
    aLinguDicsCLB       ( this, ResId( CLB_LINGU_DICS ) ),
    aLinguDicsNewPB     ( this, ResId( PB_LINGU_DICS_NEW_DIC ) ),
    aLinguDicsEditPB    ( this, ResId( PB_LINGU_DICS_EDIT_DIC ) ),
    aLinguDicsDelPB     ( this, ResId( PB_LINGU_DICS_DEL_DIC ) ),
    aLinguOptionsFT     ( this, ResId( FT_LINGU_OPTIONS ) ),
    aLinguOptionsCLB    ( this, ResId( CLB_LINGU_OPTIONS ) ),
    aLinguOptionsEditPB ( this, ResId( PB_LINGU_OPTIONS_EDIT ) ),
    sCapitalWords       ( ResId( STR_CAPITAL_WORDS ) ),
    sWordsWithDigits    ( ResId( STR_WORDS_WITH_DIGITS ) ),
    sCapitalization     ( ResId( STR_CAPITALIZATION ) ),
    sSpellSpecial       ( ResId( STR_SPELL_SPECIAL ) ),
    sAllLanguages       ( ResId( STR_ALL_LANGUAGES ) ),
    sSpellAuto          ( ResId( STR_SPELL_AUTO ) ),
    sHideMarkings       ( ResId( STR_HIDE_MARKINGS ) ),
    sOldGerman          ( ResId( STR_OLD_GERMAN ) ),
    sNumMinWordlen      ( ResId( STR_NUM_MIN_WORDLEN ) ),
    sNumPreBreak        ( ResId( STR_NUM_PRE_BREAK ) ),
    sNumPostBreak       ( ResId( STR_NUM_POST_BREAK ) ),
    sHyphAuto           ( ResId( STR_HYPH_AUTO ) ),
    sHyphSpecial        ( ResId( STR_HYPH_SPECIAL ) ),

    pLinguData          ( NULL )
{
    pCheckButtonData = NULL;

    aLinguModulesCLB.SetWindowBits( WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguModulesCLB.SetHelpId(HID_CLB_LINGU_MODULES );
    aLinguModulesCLB.SetHighlightRange();
    aLinguModulesCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguModulesCLB.SetDoubleClickHdl(LINK(this, SvxLinguTabPage, BoxDoubleClickHdl_Impl));
    aLinguModulesCLB.SetCheckButtonHdl(LINK(this, SvxLinguTabPage, BoxCheckButtonHdl_Impl));

    aLinguModulesEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguOptionsEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));

    aLinguDicsCLB.SetWindowBits( WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguDicsCLB.SetHelpId(HID_CLB_EDIT_MODULES_DICS );
    aLinguDicsCLB.SetHighlightRange();
    aLinguDicsCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguDicsCLB.SetCheckButtonHdl(LINK(this, SvxLinguTabPage, BoxCheckButtonHdl_Impl));

    aLinguDicsNewPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguDicsEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguDicsDelPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));

    aLinguOptionsCLB.SetWindowBits( WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguOptionsCLB.SetHelpId(HID_CLB_LINGU_OPTIONS );
    aLinguOptionsCLB.SetHighlightRange();
    aLinguOptionsCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguOptionsCLB.SetDoubleClickHdl(LINK(this, SvxLinguTabPage, BoxDoubleClickHdl_Impl));

    xProp = Reference< XPropertySet >( SvxGetLinguPropertySet(), UNO_QUERY );

    xDicList = Reference< XDictionaryList >( SvxGetDictionaryList(), UNO_QUERY );
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

    SvxLinguConfigUpdate::UpdateAll();
}

// -----------------------------------------------------------------------

SvxLinguTabPage::~SvxLinguTabPage()
{
    if (pLinguData)
        delete pLinguData;
}

//------------------------------------------------------------------------

//nicht überladen wegschmeissen
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

Any lcl_Bool2Any(BOOL bVal)
{
    Any aRet(&bVal, ::getBooleanCppuType());
    return aRet;
}


sal_Bool lcl_Bool2Any(Any& rVal)
{
    return *(sal_Bool*)rVal.getValue();
}


sal_Bool SvxLinguTabPage::FillItemSet( SfxItemSet& rCoreSet )
{
    sal_Bool bModified = sal_True; // !!!!

    // if not HideGroups was called with GROUP_MODULES...
    if (aLinguModulesCLB.IsVisible())
    {
        DBG_ASSERT( pLinguData, "pLinguData not yet initialized" );
        if (!pLinguData)
            pLinguData =    new SvxLinguData_Impl;

        ULONG i;

        // update spellchecker configuration entries
        const LangImplNameTable *pTable = &pLinguData->GetSpellTable();
        for (i = 0;  i < pTable->Count();  ++i)
        {
            INT16 nLang = (INT16) pTable->GetObjectKey(i);
            const Sequence< OUString > *pImplNames = pTable->GetObject(i);
            DBG_ASSERT( pImplNames, "service implementation names missing" )
            if (pImplNames)
            {
#if OSL_DEBUG_LEVEL > 1
                const OUString *pTmpStr = pImplNames->getConstArray();
#endif
                Reference< XLinguServiceManager > xMgr( pLinguData->GetManager() );
                Locale aLocale( SvxCreateLocale(nLang) );
                if (xMgr.is())
                    xMgr->setConfiguredServices( C2U(cSpell), aLocale, *pImplNames );
            }
        }

        // update hyphenator configuration entries
        pTable = &pLinguData->GetHyphTable();
        for (i = 0;  i < pTable->Count();  ++i)
        {
            INT16 nLang = (INT16) pTable->GetObjectKey(i);
            const Sequence< OUString > *pImplNames = pTable->GetObject(i);
            DBG_ASSERT( pImplNames, "service implementation names missing" )
            if (pImplNames)
            {
                Reference< XLinguServiceManager > xMgr( pLinguData->GetManager() );
                Locale aLocale( SvxCreateLocale(nLang) );
                if (xMgr.is())
                    xMgr->setConfiguredServices( C2U(cHyph), aLocale, *pImplNames );
            }
        }

        // update thesaurus configuration entries
        pTable = &pLinguData->GetThesTable();
        for (i = 0;  i < pTable->Count();  ++i)
        {
            INT16 nLang = (INT16) pTable->GetObjectKey(i);
            const Sequence< OUString > *pImplNames = pTable->GetObject(i);
            DBG_ASSERT( pImplNames, "service implementation names missing" )
            if (pImplNames)
            {
                Reference< XLinguServiceManager > xMgr( pLinguData->GetManager() );
                Locale aLocale( SvxCreateLocale(nLang) );
                if (xMgr.is())
                    xMgr->setConfiguredServices( C2U(cThes), aLocale, *pImplNames );
            }
        }
    }


    //
    // activate dictionaries according to checkbox state
    //
    Sequence< OUString > aActiveDics;
    INT32 nActiveDics = 0;
    ULONG nEntries = aLinguDicsCLB.GetEntryCount();
    for (ULONG i = 0;  i < nEntries;  ++i)
    {
        INT32 nDics = aDics.getLength();
        const Reference< XDictionary > *pDic = aDics.getConstArray();

        aActiveDics.realloc( nDics );
        OUString *pActiveDic = aActiveDics.getArray();

        SvLBoxEntry *pEntry = aLinguDicsCLB.GetEntry( i );
        if (pEntry)
        {
            DicUserData aData( (ULONG)pEntry->GetUserData() );
            if (aData.GetEntryId() < nDics)
            {
                BOOL bChecked = aLinguDicsCLB.IsChecked( (USHORT) i );
                Reference< XDictionary > xDic( aDics.getConstArray()[ i ] );
                if (xDic.is())
                {
                    if (SvxGetIgnoreAllList() == xDic)
                        bChecked = TRUE;
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
    //
    aActiveDics.realloc( nActiveDics );
    Any aTmp;
    aTmp <<= aActiveDics;
    SvtLinguConfig aLngCfg;
    aLngCfg.SetProperty( UPH_ACTIVE_DICTIONARIES, aTmp );


    BOOL bCheckAllLangChanged = FALSE;

    nEntries = aLinguOptionsCLB.GetEntryCount();
    for (USHORT j = 0;  j < nEntries;  ++j)
    {
        SvLBoxEntry *pEntry = aLinguOptionsCLB.GetEntry( j );

        OptionsUserData aData( (ULONG)pEntry->GetUserData() );
        String aPropName( lcl_GetPropertyName( (EID_OPTIONS) aData.GetEntryId() ) );

        if (EID_ALL_LANGUAGES == (EID_OPTIONS) aData.GetEntryId())
        {
            bCheckAllLangChanged = aData.IsChecked() != aLinguOptionsCLB.IsChecked( j );
        }

        Any aAny;
        if (aData.IsCheckable())
        {
            BOOL bChecked = aLinguOptionsCLB.IsChecked( j );
            aAny <<= bChecked;
        }
        else if (aData.HasNumericValue())
        {
            INT16 nVal = aData.GetNumericValue();
            aAny <<= nVal;
        }

        if (xProp.is())
            xProp->setPropertyValue( aPropName, aAny );
        aLngCfg.SetProperty( aPropName, aAny );
    }

    rCoreSet.Put( SfxBoolItem( SID_SPELL_MODIFIED, bCheckAllLangChanged ) );

    SvLBoxEntry *pPreBreakEntry  = aLinguOptionsCLB.GetEntry( (USHORT) EID_NUM_PRE_BREAK );
    SvLBoxEntry *pPostBreakEntry = aLinguOptionsCLB.GetEntry( (USHORT) EID_NUM_POST_BREAK );
    DBG_ASSERT( pPreBreakEntry, "NULL Pointer" );
    DBG_ASSERT( pPostBreakEntry, "NULL Pointer" );
    if (pPreBreakEntry && pPostBreakEntry)
    {
        OptionsUserData aPreBreakData( (ULONG)pPreBreakEntry->GetUserData() );
        OptionsUserData aPostBreakData( (ULONG)pPostBreakEntry->GetUserData() );
        if ( aPreBreakData.IsModified() || aPostBreakData.IsModified() )
        {
            SfxHyphenRegionItem aHyp( GetWhich( SID_ATTR_HYPHENREGION ) );
            aHyp.GetMinLead()  = (UINT8) aPreBreakData.GetNumericValue();
            aHyp.GetMinTrail() = (UINT8) aPostBreakData.GetNumericValue();
            rCoreSet.Put( aHyp );
        }
    }


    // Autom. Rechtschreibung
    BOOL bNewAutoCheck = aLinguOptionsCLB.IsChecked( (USHORT) EID_SPELL_AUTO );
    const SfxPoolItem* pOld = GetOldItem( rCoreSet, SID_AUTOSPELL_CHECK );
    if ( !pOld || ( (SfxBoolItem*)pOld )->GetValue() != bNewAutoCheck )
    {
        rCoreSet.Put( SfxBoolItem( GetWhich( SID_AUTOSPELL_CHECK ),
                                bNewAutoCheck ) );
        bModified |= sal_True;
    }

    // pOld ist alte Wert wenn wert geändert mit neuen Wert ins Item Set
    BOOL bNewMarkOff = aLinguOptionsCLB.IsChecked( (USHORT) EID_HIDE_MARKINGS );
    pOld = GetItem( rCoreSet, SID_AUTOSPELL_MARKOFF );
    if ( !pOld || ( (SfxBoolItem*)pOld )->GetValue() != bNewMarkOff )
    {
        rCoreSet.Put( SfxBoolItem( GetWhich( SID_AUTOSPELL_MARKOFF ),
                                   bNewMarkOff ) );
        bModified |= sal_True;
    }

    return bModified;
}

// ----------------------------------------------------------------------

ULONG SvxLinguTabPage::GetDicUserData( const Reference< XDictionary > &rxDic, USHORT nIdx )
{
    ULONG nRes = 0;
    DBG_ASSERT( rxDic.is(), "dictionary not supplied" );
    if (rxDic.is())
    {
        Reference< frame::XStorable > xStor( rxDic, UNO_QUERY );

        ULONG nUserData = 0;
        BOOL bChecked = rxDic->isActive();
        BOOL bEditable = !xStor.is() || !xStor->isReadonly();
        BOOL bDeletable = bEditable;
        BOOL bNegativ = rxDic->getDictionaryType() == DictionaryType_NEGATIVE;

        nRes = DicUserData( nIdx,
                bChecked, bEditable, bDeletable ).GetUserData();
    }
    return nRes;
}


void SvxLinguTabPage::AddDicBoxEntry(
        const Reference< XDictionary > &rxDic,
        USHORT nIdx )
{
    aLinguDicsCLB.SetUpdateMode(FALSE);

    String aTxt( ::GetDicInfoStr( rxDic->getName(),
                        SvxLocaleToLanguage( rxDic->getLocale() ),
                        DictionaryType_NEGATIVE == rxDic->getDictionaryType() ) );
    aLinguDicsCLB.InsertEntry( aTxt );  // append at end
    SvLBoxEntry* pEntry = aLinguDicsCLB.GetEntry( aLinguDicsCLB.GetEntryCount() - 1 );
    DBG_ASSERT( pEntry, "failed to add entry" );
    if (pEntry)
    {
        DicUserData aData( GetDicUserData( rxDic, nIdx ) );
        pEntry->SetUserData( (void *) aData.GetUserData() );
        lcl_SetCheckButton( pEntry, aData.IsChecked() );
    }

    aLinguDicsCLB.SetUpdateMode(TRUE);
}

// ----------------------------------------------------------------------

void SvxLinguTabPage::UpdateDicBox_Impl()
{
    aLinguDicsCLB.SetUpdateMode(FALSE);
    aLinguDicsCLB.Clear();

    INT32 nDics  = aDics.getLength();
    const Reference< XDictionary > *pDic = aDics.getConstArray();
    for (INT32 i = 0;  i < nDics;  ++i)
    {
        const Reference< XDictionary > &rDic = pDic[i];
        if (rDic.is())
            AddDicBoxEntry( rDic, (USHORT)i );
    }

    aLinguDicsCLB.SetUpdateMode(TRUE);
}

// ----------------------------------------------------------------------

void SvxLinguTabPage::UpdateModulesBox_Impl()
{
    if (pLinguData)
    {
        const ServiceInfoArr &rAllDispSrvcArr = pLinguData->GetDisplayServiceArray();
        const ULONG nDispSrvcCount = pLinguData->GetDisplayServiceCount();

        aLinguModulesCLB.Clear();

        ServiceInfo_Impl* pInfo;
        for (USHORT i = 0;  i < nDispSrvcCount;  ++i)
        {
            pInfo = rAllDispSrvcArr.Get(i);
            aLinguModulesCLB.InsertEntry(pInfo->sDisplayName);
            SvLBoxEntry* pEntry = aLinguModulesCLB.GetEntry(i);
            pEntry->SetUserData( (void *) pInfo );
            aLinguModulesCLB.CheckEntryPos( i, pInfo->bConfigured );
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

    aLinguOptionsCLB.SetUpdateMode(FALSE);
    aLinguOptionsCLB.Clear();

    SvLBoxTreeList *pModel = aLinguOptionsCLB.GetModel();
    SvLBoxEntry* pEntry = NULL;

    INT16 nVal = 0;
    BOOL  bVal  = FALSE;
    ULONG nUserData = 0;

    pEntry = CreateEntry( sCapitalWords,    CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_SPELL_UPPER_CASE) ) >>= bVal;
    nUserData = OptionsUserData( EID_CAPITAL_WORDS, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sWordsWithDigits, CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_SPELL_WITH_DIGITS) ) >>= bVal;
    nUserData = OptionsUserData( EID_WORDS_WITH_DIGITS, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sCapitalization,  CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_SPELL_CAPITALIZATION) ) >>= bVal;
    nUserData = OptionsUserData( EID_CAPITALIZATION, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sSpellSpecial,    CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_SPELL_SPECIAL) ) >>= bVal;
    nUserData = OptionsUserData( EID_SPELL_SPECIAL, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sAllLanguages,    CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_SPELL_IN_ALL_LANGUAGES) ) >>= bVal;
    nUserData = OptionsUserData( EID_ALL_LANGUAGES, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sSpellAuto,       CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_SPELL_AUTO) ) >>= bVal;
    const SfxPoolItem* pItem = GetItem( rSet, SID_AUTOSPELL_CHECK );
    if (pItem)
        bVal = ((SfxBoolItem *) pItem)->GetValue();
    nUserData = OptionsUserData( EID_SPELL_AUTO, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sHideMarkings,    CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_SPELL_HIDE) ) >>= bVal;
    pItem = GetItem( rSet, SID_AUTOSPELL_MARKOFF );
    if (pItem)
        bVal = ((SfxBoolItem *) pItem)->GetValue();
    nUserData = OptionsUserData( EID_HIDE_MARKINGS, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sOldGerman,       CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_GERMAN_PRE_REFORM) ) >>= bVal;
    nUserData = OptionsUserData( EID_OLD_GERMAN, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sNumMinWordlen,   CBCOL_SECOND );
    aLngCfg.GetProperty( C2U(UPN_HYPH_MIN_WORD_LENGTH) ) >>= nVal;
    nUserData = OptionsUserData( EID_NUM_MIN_WORDLEN, TRUE, (USHORT)nVal, FALSE, FALSE).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );

    const SfxHyphenRegionItem *pHyp = NULL;
    USHORT nWhich = GetWhich( SID_ATTR_HYPHENREGION );
    if ( rSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_SET )
        pHyp = &( (const SfxHyphenRegionItem &) rSet.Get( nWhich ) );

    pEntry = CreateEntry( sNumPreBreak,     CBCOL_SECOND );
    aLngCfg.GetProperty( C2U(UPN_HYPH_MIN_LEADING) ) >>= nVal;
    if (pHyp)
        nVal = (INT16) pHyp->GetMinLead();
    nUserData = OptionsUserData( EID_NUM_PRE_BREAK, TRUE, (USHORT)nVal, FALSE, FALSE).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );

    pEntry = CreateEntry( sNumPostBreak,    CBCOL_SECOND );
    aLngCfg.GetProperty( C2U(UPN_HYPH_MIN_TRAILING) ) >>= nVal;
    if (pHyp)
        nVal = (INT16) pHyp->GetMinTrail();
    nUserData = OptionsUserData( EID_NUM_POST_BREAK, TRUE, (USHORT)nVal, FALSE, FALSE).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );

    pEntry = CreateEntry( sHyphAuto,        CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_HYPH_AUTO) ) >>= bVal;
    nUserData = OptionsUserData( EID_HYPH_AUTO, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    pEntry = CreateEntry( sHyphSpecial,     CBCOL_FIRST );
    aLngCfg.GetProperty( C2U(UPN_IS_HYPH_SPECIAL) ) >>= bVal;
    nUserData = OptionsUserData( EID_HYPH_SPECIAL, FALSE, 0, TRUE, bVal).GetUserData();
    pEntry->SetUserData( (void *)nUserData );
    pModel->Insert( pEntry );
    lcl_SetCheckButton( pEntry, bVal );

    aLinguOptionsCLB.SetUpdateMode(TRUE);
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

IMPL_LINK( SvxLinguTabPage, PostDblClickHdl_Impl, SvTreeListBox *, pBox )
{
    ClickHdl_Impl(&aLinguModulesEditPB);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, BoxCheckButtonHdl_Impl, SvTreeListBox *, pBox )
{
    if (pBox == &aLinguModulesCLB)
    {
        DBG_ASSERT( pLinguData, "NULL pointer, LinguData missing" );
        USHORT nPos = aLinguModulesCLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND  &&  pLinguData)
        {
            pLinguData->Reconfigure( aLinguModulesCLB.GetText( nPos ),
                                     aLinguModulesCLB.IsChecked( nPos ) );
        }
    }
    else if (pBox == &aLinguDicsCLB)
    {
        USHORT nPos = aLinguDicsCLB.GetSelectEntryPos();
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
        {
            const Reference< XDictionary > &rDic = aDics.getConstArray()[ nPos ];
            if (SvxGetIgnoreAllList() == rDic)
            {
                SvLBoxEntry* pEntry = aLinguDicsCLB.GetEntry( nPos );
                if (pEntry)
                    lcl_SetCheckButton( pEntry, TRUE );
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
        ULONG nLen = pLinguData->GetDisplayServiceCount();
        for (ULONG i = 0;  i < nLen;  ++i)
            pLinguData->GetDisplayServiceArray().Get(i)->bConfigured = FALSE;
        const Locale* pAllLocales = pLinguData->GetAllSupportedLocales().getConstArray();
        INT32 nLocales = pLinguData->GetAllSupportedLocales().getLength();
        for (INT32 k = 0;  k < nLocales;  ++k)
        {
            INT16 nLang = SvxLocaleToLanguage( pAllLocales[k] );
            Sequence< OUString > *pNames;
            pNames = pLinguData->GetSpellTable().Get( nLang );
            if (pNames)
                pLinguData->SetChecked( *pNames );
            pNames = pLinguData->GetHyphTable().Get( nLang );
            if (pNames)
                pLinguData->SetChecked( *pNames );
            pNames = pLinguData->GetThesTable().Get( nLang );
            if (pNames)
                pLinguData->SetChecked( *pNames );
        }

        // show new status of modules
        UpdateModulesBox_Impl();
    }
    else if (&aLinguDicsNewPB == pBtn)
    {
        Reference< XSpellChecker1 > xSpellChecker1;
        SvxNewDictionaryDialog aDlg( this,  xSpellChecker1);
        Reference< XDictionary >  xNewDic;
        if ( aDlg.Execute() == RET_OK )
            xNewDic = Reference< XDictionary >( aDlg.GetNewDictionary(), UNO_QUERY );
        if ( xNewDic.is() )
        {
            // add new dics to the end
            INT32 nLen = aDics.getLength();
            aDics.realloc( nLen + 1 );

            aDics.getArray()[ nLen ] = xNewDic;

            AddDicBoxEntry( xNewDic, (USHORT) nLen );
        }
    }
    else if (&aLinguDicsEditPB == pBtn)
    {
        SvLBoxEntry *pEntry = aLinguDicsCLB.GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( (ULONG) pEntry->GetUserData() );
            USHORT nDicPos = aData.GetEntryId();
            INT32 nDics = aDics.getLength();
            if (nDicPos < nDics)
            {
                Reference< XDictionary > xDic;
                xDic = aDics.getConstArray()[ nDicPos ];
                if (xDic.is())
                {
                    Reference< XSpellChecker1 > xSpellChecker1;
                    SvxEditDictionaryDialog aDlg( this,
                            xDic->getName(), xSpellChecker1 );
                    aDlg.Execute();
                }
            }
        }
    }
    else if (&aLinguDicsDelPB == pBtn)
    {
        if ( RET_NO ==
             QueryBox( this, ResId( RID_SFXQB_DELDICT, DIALOG_MGR() ) ).Execute() )
            return 0;

        SvLBoxEntry *pEntry = aLinguDicsCLB.GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( (ULONG) pEntry->GetUserData() );
            USHORT nDicPos = aData.GetEntryId();
            INT32 nDics = aDics.getLength();
            if (nDicPos < nDics)
            {
                Reference< XDictionary > xDic;
                xDic = aDics.getConstArray()[ nDicPos ];
                if (xDic.is())
                {
                    if (SvxGetIgnoreAllList() == xDic)
                        xDic->clear();
                    else
                    {
                        if (xDicList.is())
                            xDicList->removeDictionary( xDic );

                        Reference< frame::XStorable > xStor( xDic, UNO_QUERY );
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
                        ULONG nCnt = aLinguDicsCLB.GetEntryCount();
                        for (ULONG i = 0;  i < nCnt;  ++i)
                        {
                            SvLBoxEntry *pEntry = aLinguDicsCLB.GetEntry( i );
                            DBG_ASSERT( pEntry, "missing entry" );
                            if (pEntry)
                            {
                                DicUserData aData( (ULONG) pEntry->GetUserData() );
                                if (aData.GetEntryId() == nDicPos )
                                {
                                    aLinguDicsCLB.RemoveEntry( (USHORT) i );
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
            long nVal = -1;
            OptionsUserData aData( (ULONG)pEntry->GetUserData() );
            if(aData.HasNumericValue())
            {
                int nRID = -1;
                switch (aData.GetEntryId())
                {
                    case EID_NUM_PRE_BREAK  : nRID = STR_NUM_PRE_BREAK_DLG; break;
                    case EID_NUM_POST_BREAK : nRID = STR_NUM_POST_BREAK_DLG; break;
                    case EID_NUM_MIN_WORDLEN: nRID = STR_NUM_MIN_WORDLEN_DLG; break;
                    default:
                        DBG_ERROR( "unexpected case" );
                }

                OptionsBreakSet aDlg( this, nRID );
                aDlg.GetNumericFld().SetValue( aData.GetNumericValue() );
                if (RET_OK == aDlg.Execute() )
                {
                    nVal = aDlg.GetNumericFld().GetValue();
                    if (-1 != nVal && aData.GetNumericValue() != nVal)
                    {
                        aData.SetNumericValue( (BYTE)nVal ); //! sets IsModified !
                        pEntry->SetUserData( (void *) aData.GetUserData() );
                        aLinguOptionsCLB.Invalidate();
                    }
                }
            }
        }
    }
    else
    {
        DBG_ERROR( "pBtn unexpected value" );
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
            DicUserData aData( (ULONG) pEntry->GetUserData() );
            aLinguDicsEditPB.Enable( aData.IsEditable() );
            aLinguDicsDelPB .Enable( aData.IsDeletable() );
        }
    }
    else if (&aLinguOptionsCLB == pBox)
    {
        SvLBoxEntry *pEntry = pBox->GetCurEntry();
        if (pEntry)
        {
            OptionsUserData aData( (ULONG) pEntry->GetUserData() );
            aLinguOptionsEditPB.Enable( aData.HasNumericValue() );
        }
    }
    else
    {
        DBG_ERROR( "pBox unexpected value" );
    }

    return 0;
}

// -----------------------------------------------------------------------

SvLBoxEntry* SvxLinguTabPage::CreateEntry( String& rTxt, USHORT nCol )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if( !pCheckButtonData )
        pCheckButtonData = new SvLBoxButtonData( &aLinguOptionsCLB );

    String sEmpty;
    if (CBCOL_FIRST == nCol)
        pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // Leerspalte
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!
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
        //
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
        //
        aPos = aLinguOptionsFT.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguOptionsFT.SetPosPixel( aPos );
        aPos = aLinguOptionsCLB.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguOptionsCLB.SetPosPixel( aPos );
        aPos = aLinguOptionsEditPB.GetPosPixel();
        aPos.Y() -= nDeltaY;
        aLinguOptionsEditPB.SetPosPixel( aPos );
        //
        Size aSize( aLinguOptionsCLB.GetSizePixel() );
        aSize.Height() += nDeltaY;
        aLinguOptionsCLB.SetSizePixel( aSize );
    }
}
/*--------------------------------------------------
--------------------------------------------------*/

SvxEditModulesDlg::SvxEditModulesDlg(Window* pParent, SvxLinguData_Impl& rData) :
        ModalDialog(pParent, ResId(RID_SVXDLG_EDIT_MODULES, DIALOG_MGR() )),
        aClosePB    ( this, ResId( PB_OK ) ),
        aHelpPB     ( this, ResId( PB_HELP ) ),
        aModulesFL  ( this, ResId( FL_EDIT_MODULES_OPTIONS ) ),
        aLanguageFT ( this, ResId( FT_EDIT_MODULES_LANGUAGE ) ),
        aLanguageLB ( this, ResId( LB_EDIT_MODULES_LANGUAGE ), FALSE ),
        aModulesCLB ( this, ResId( CLB_EDIT_MODULES_MODULES ) ),
        aPrioUpPB   ( this, ResId( PB_EDIT_MODULES_PRIO_UP ) ),
        aPrioDownPB ( this, ResId( PB_EDIT_MODULES_PRIO_DOWN ) ),
        aBackPB     ( this, ResId( PB_EDIT_MODULES_PRIO_BACK ) ),
        sSpell( ResId( ST_SPELL)),
        sHyph( ResId( ST_HYPH)),
        sThes( ResId( ST_THES)),
        rLinguData(rData)
{
    pCheckButtonData = NULL;
    FreeResource();

    pDefaultLinguData = new SvxLinguData_Impl( rLinguData );

    aModulesCLB.SetWindowBits( WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aModulesCLB.SetHighlightRange();
    aModulesCLB.SetHelpId(HID_CLB_EDIT_MODULES_MODULES );
    aModulesCLB.SetSelectHdl( LINK( this, SvxEditModulesDlg, SelectHdl_Impl ));
    aModulesCLB.SetCheckButtonHdl( LINK( this, SvxEditModulesDlg, BoxCheckButtonHdl_Impl) );

    aClosePB   .SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aPrioUpPB  .SetClickHdl( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    aPrioDownPB.SetClickHdl( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    aBackPB    .SetClickHdl( LINK( this, SvxEditModulesDlg, BackHdl_Impl ));
    // in case of not installed language modules
    aPrioUpPB  .Enable( FALSE );
    aPrioDownPB.Enable( FALSE );

    //
    //fill language box
    //
    Sequence< INT16 > aAvailLang;
    Reference< XAvailableLocales > xAvail( rLinguData.GetManager(), UNO_QUERY );
    if (xAvail.is())
    {
        aAvailLang = lcl_LocaleSeqToLangSeq(
                        xAvail->getAvailableLocales( C2U(cSpell) ) );
    }
    const Sequence< Locale >& rLoc = rLinguData.GetAllSupportedLocales();
    const Locale* pLocales = rLoc.getConstArray();
    aLanguageLB.Clear();
    for(long i = 0; i < rLoc.getLength(); i++)
    {
        INT16 nLang = SvxLocaleToLanguage( pLocales[i] );
        aLanguageLB.InsertLanguage( nLang, lcl_SeqHasLang( aAvailLang, nLang ) );
    }
    LanguageType eSysLang = GetSystemLanguage();
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


SvLBoxEntry* SvxEditModulesDlg::CreateEntry( String& rTxt, USHORT nCol )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;
    if( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData( &aModulesCLB );
        pCheckButtonData->SetLink( aModulesCLB.GetCheckButtonHdl() );
    }

    String sEmpty;
    if (CBCOL_FIRST == nCol)
        pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // Leerspalte
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!
    pEntry->AddItem( new BrwStringDic_Impl( pEntry, 0, rTxt ) );

    return pEntry;
}

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/

static void lcl_UpdateLinguData()
{
}

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
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
                USHORT  nCurPos = pBox->GetSelectEntryPos();
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
        DBG_ERROR( "pBox unexpected value" );
    }

    return 0;
}
/* -----------------------------28.05.01 11:00--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SvxEditModulesDlg, BoxCheckButtonHdl_Impl, SvTreeListBox *, pBox )
{
//    if (pBox == (SvTreeListBox *) &aModulesCLB)
//    {
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
                        lcl_SetCheckButton( pEntry, FALSE );
                        pBox->InvalidateEntry( pEntry );
                    }
                    pEntry = pBox->Next( pEntry );
                }
            }
        }
//    }
    return 0;
}
/* -----------------------------27.11.00 14:00--------------------------------

 ---------------------------------------------------------------------------*/
OUString lcl_GetServiceName(BYTE nType)
{
    switch(nType)
    {
        case  TYPE_SPELL:   return C2U(cSpell);
        case  TYPE_HYPH:    return C2U(cHyph);
        case  TYPE_THES:    return C2U(cThes);
    }
    return OUString();
}


IMPL_LINK( SvxEditModulesDlg, LangSelectHdl_Impl, ListBox *, pBox )
{
    LanguageType  eCurLanguage = aLanguageLB.GetSelectLanguage();
    static Locale aLastLocale;
    Locale aCurLocale;
    SvxLanguageToLocale(aCurLocale, eCurLanguage);
    SvLBoxTreeList *pModel = aModulesCLB.GetModel();
    Reference<XLinguServiceManager>&   xMgr = rLinguData.GetManager();

    if (pBox)
    {
        // save old probably changed settings
        // before switching to new language entries

        ULONG nLang = SvxLocaleToLanguage( aLastLocale );

        sal_Int32 nStart = 0, nLocalIndex = 0;
        Sequence<OUString> aChange;
        sal_Bool bChanged = FALSE;
        for(USHORT i = 0; i < aModulesCLB.GetEntryCount(); i++)
        {
            SvLBoxEntry *pEntry = aModulesCLB.GetEntry(i);
            ModuleUserData_Impl* pData = (ModuleUserData_Impl*)pEntry->GetUserData();
            if(pData->IsParent())
            {
                if(bChanged)
                {
                    LangImplNameTable *pTable = 0;
                    BYTE nType = pData->GetType();
                    switch (nType - 1)
                    {
                        case  TYPE_SPELL : pTable = &rLinguData.GetSpellTable(); break;
                        case  TYPE_HYPH  : pTable = &rLinguData.GetHyphTable();  break;
                        case  TYPE_THES  : pTable = &rLinguData.GetThesTable();  break;
                    }
                    if (pTable)
                    {
                        aChange.realloc(nStart);
                        Sequence< OUString > *pImplNames = pTable->Get( nLang );
                        if (pImplNames)
                            *pImplNames = aChange;
                        else
                            pTable->Insert( nLang, new Sequence< OUString >(aChange) );
                    }
                }
                nLocalIndex = nStart = 0;
                aChange.realloc(aModulesCLB.GetEntryCount());
                bChanged = FALSE;
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
            LangImplNameTable &rTable = rLinguData.GetThesTable();
            Sequence< OUString > *pImplNames = rTable.Get( nLang );
            if (pImplNames)
                *pImplNames = aChange;
            else
                rTable.Insert( nLang, new Sequence< OUString >(aChange) );
        }
    }

    for(ULONG i = 0; i < aModulesCLB.GetEntryCount(); i++)
        delete (ModuleUserData_Impl*)aModulesCLB.GetEntry(i)->GetUserData();

    //
    // display entries for new selected language
    //
    aModulesCLB.Clear();
    if(LANGUAGE_DONTKNOW != eCurLanguage)
    {
        sal_Int32 nEntryPos = 1;

        ULONG n;
        ServiceInfo_Impl* pInfo;

        //
        // spellchecker entries
        //
        SvLBoxEntry* pEntry = CreateEntry( sSpell,  CBCOL_SECOND );
        ModuleUserData_Impl* pUserData = new ModuleUserData_Impl(
                                         String(), TRUE, FALSE, TYPE_SPELL, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );
        //
        Sequence< OUString > aNames( rLinguData.GetSortedImplNames( eCurLanguage, TYPE_SPELL ) );
        const OUString *pName = aNames.getConstArray();
        ULONG nNames = (ULONG) aNames.getLength();
        sal_Int32 nLocalIndex = 0;  // index relative to parent
        for (n = 0;  n < nNames;  ++n)
        {
            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            BOOL bIsSuppLang = pInfo->xSpell.is()  &&
                               pInfo->xSpell->hasLocale( aCurLocale );
            const OUString &rImplName = pInfo->sSpellImplName;
            if (rImplName.getLength()  &&  bIsSuppLang)
            {
                String aTxt( pInfo->sDisplayName );
                SvLBoxEntry* pEntry = CreateEntry( aTxt, CBCOL_FIRST );
                const Sequence< OUString > *pCfgImplNames = rLinguData.GetSpellTable().Get( eCurLanguage );
                DBG_ASSERT( pCfgImplNames, "pCfgImplNames missing" );
                BOOL bChecked = pCfgImplNames && lcl_SeqGetEntryPos( *pCfgImplNames, rImplName ) >= 0;
                lcl_SetCheckButton( pEntry, bChecked );
                pUserData = new ModuleUserData_Impl( rImplName, FALSE,
                                        bChecked, TYPE_SPELL, (BYTE)nLocalIndex++ );
                pEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pEntry );
            }
        }

        //
        // hyphenator entries
        //
        pEntry = CreateEntry( sHyph,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( String(), TRUE, FALSE, TYPE_HYPH, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );
        //
        aNames = rLinguData.GetSortedImplNames( eCurLanguage, TYPE_HYPH );
        pName = aNames.getConstArray();
        nNames = (ULONG) aNames.getLength();
        nLocalIndex = 0;
        for (n = 0;  n < nNames;  ++n)
        {
            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            BOOL bIsSuppLang = pInfo->xHyph.is()  &&
                               pInfo->xHyph->hasLocale( aCurLocale );
            const OUString &rImplName = pInfo->sHyphImplName;
            if (rImplName.getLength()  &&  bIsSuppLang)
            {
                String aTxt( pInfo->sDisplayName );
                SvLBoxEntry* pEntry = CreateEntry( aTxt, CBCOL_FIRST );
                const Sequence< OUString > *pCfgImplNames = rLinguData.GetHyphTable().Get( eCurLanguage );
                DBG_ASSERT( pCfgImplNames, "pCfgImplNames missing" );
                BOOL bChecked = pCfgImplNames && lcl_SeqGetEntryPos( *pCfgImplNames, rImplName ) >= 0;
                lcl_SetCheckButton( pEntry, bChecked );
                pUserData = new ModuleUserData_Impl( rImplName, FALSE,
                                        bChecked, TYPE_HYPH, (BYTE)nLocalIndex++ );
                pEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pEntry );
            }
        }

        //
        // thesaurus entries
        //
        pEntry = CreateEntry( sThes,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( String(), TRUE, FALSE, TYPE_THES, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );
        //
        aNames = rLinguData.GetSortedImplNames( eCurLanguage, TYPE_THES );
        pName = aNames.getConstArray();
        nNames = (ULONG) aNames.getLength();
        nLocalIndex = 0;
        for (n = 0;  n < nNames;  ++n)
        {
            pInfo = rLinguData.GetInfoByImplName( pName[n] );
            BOOL bIsSuppLang = pInfo->xThes.is()  &&
                               pInfo->xThes->hasLocale( aCurLocale );
            const OUString &rImplName = pInfo->sThesImplName;
            if (rImplName.getLength()  &&  bIsSuppLang)
            {
                String aTxt( pInfo->sDisplayName );
                SvLBoxEntry* pEntry = CreateEntry( aTxt, CBCOL_FIRST );
                const Sequence< OUString > *pCfgImplNames = rLinguData.GetThesTable().Get( eCurLanguage );
                DBG_ASSERT( pCfgImplNames, "pCfgImplNames missing" );
                BOOL bChecked = pCfgImplNames && lcl_SeqGetEntryPos( *pCfgImplNames, rImplName ) >= 0;
                lcl_SetCheckButton( pEntry, bChecked );
                pUserData = new ModuleUserData_Impl( rImplName, FALSE,
                                        bChecked, TYPE_THES, (BYTE)nLocalIndex++ );
                pEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pEntry );
            }
        }
    }
    aLastLocale.Language = aCurLocale.Language;
    aLastLocale.Country = aCurLocale.Country;
    return 0;
}
/* -----------------------------27.11.00 19:50--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SvxEditModulesDlg, UpDownHdl_Impl, PushButton *, pBtn )
{
    sal_Bool bUp = &aPrioUpPB == pBtn;
    USHORT  nCurPos = aModulesCLB.GetSelectEntryPos();
    SvLBoxEntry* pEntry;
    if (nCurPos != LISTBOX_ENTRY_NOTFOUND  &&
        0 != (pEntry = aModulesCLB.GetEntry(nCurPos)))
    {
        aModulesCLB.SetUpdateMode(FALSE);
        SvLBoxTreeList *pModel = aModulesCLB.GetModel();

        ModuleUserData_Impl* pData = (ModuleUserData_Impl*)pEntry->GetUserData();
        String aStr(aModulesCLB.GetEntryText(pEntry));
        SvLBoxEntry* pToInsert = CreateEntry( aStr, CBCOL_FIRST );
        pToInsert->SetUserData( (void *)pData);
        BOOL bIsChecked = aModulesCLB.IsChecked(nCurPos);

        pModel->Remove(pEntry);

        USHORT nDestPos = bUp ? nCurPos - 1 : nCurPos + 1;
        pModel->Insert(pToInsert, nDestPos);
        aModulesCLB.CheckEntryPos(nDestPos, bIsChecked );
        aModulesCLB.SelectEntryPos(nDestPos );
        SelectHdl_Impl(&aModulesCLB);
        aModulesCLB.SetUpdateMode(TRUE);
    }
    return 0;
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
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
        DBG_ERROR( "pBtn unexpected value" );
    }

    return 0;
}
/* -----------------------------27.11.00 20:31--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SvxEditModulesDlg, BackHdl_Impl, PushButton *, pBtn )
{
    rLinguData = *pDefaultLinguData;
    LangSelectHdl_Impl(0);
    return 0;
}

