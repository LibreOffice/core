/*************************************************************************
 *
 *  $RCSfile: optlingu.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-22 11:43:09 $
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
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
#include <svx/unolingu.hxx>
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
#ifndef _COM_SUN_STAR_LINGUISTIC2_XOTHERLINGU_HPP_
#include <com/sun/star/linguistic2/XOtherLingu.hpp>
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

static BOOL lcl_SeqHasEntry(
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
    return i < nLen ? TRUE : FALSE;
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

#define TYPE_SPELL  1
#define TYPE_HYPH   2
#define TYPE_THES   3

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
    GroupBox        aValGB;
    NumericField    aValNF;

public:
    OptionsBreakSet(Window* pParent, int nRID) :
            ModalDialog(pParent, ResId(RID_SVXDLG_LNG_ED_NUM_PREBREAK, DIALOG_MGR() )),
            aOKPB       (this, ResId(BT_OK_PREBREAK)),
            aCancelPB   (this, ResId(BT_CANCEL_PREBREAK)),
            aValGB      (this, ResId(GB_NUMVAL_PREBREAK)),
            aValNF      (this, ResId(ED_PREBREAK))
    {
        DBG_ASSERT( STR_NUM_PRE_BREAK_DLG   == nRID ||
                    STR_NUM_POST_BREAK_DLG  == nRID ||
                    STR_NUM_MIN_WORDLEN_DLG == nRID, "unexpected RID" );

        if (nRID != -1)
            aValGB.SetText( String( ResId(nRID) ) );
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

// SvxLinguData_Impl ----------------------------------------------------

struct ServiceInfo_Impl
{
    OUString                    sDisplayName;
    OUString                    sSpellImplName;
    OUString                    sHyphImplName;
    OUString                    sThesImplName;
    Sequence <Locale>           aSuppLocales;
    Reference <XSpellChecker>   xSpell;
    Reference <XHyphenator>     xHyph;
    Reference <XThesaurus>      xThes;
    BOOL                        bConfigured;

    ServiceInfo_Impl() :
    bConfigured(sal_False){}
};
typedef ServiceInfo_Impl* ServiceInfo_ImplPtr;
SV_DECL_PTRARR_DEL(ServiceInfoArr, ServiceInfo_ImplPtr, 2, 2);
SV_IMPL_PTRARR(ServiceInfoArr, ServiceInfo_Impl*);

class SvxLinguData_Impl
{
    Reference< XMultiServiceFactory >   xMSF;
    Reference<XLinguServiceManager>     xLinguSrvcMgr;
    ServiceInfoArr                      aAvailSpellInfoArr;
    ServiceInfoArr                      aAvailHyphInfoArr;
    ServiceInfoArr                      aAvailThesInfoArr;

    Sequence<OUString>                  aAvailSrvcDisplayNames;
    Sequence<OUString>                  aAvailSrvcNames;
//  Sequence<sal_Bool>                  aAvailSrvcConfigured;

    Sequence<Locale>                    aAllServiceLocales;

    //contains services an implementation names sorted by implementation names
    ServiceInfoArr                      aDisplayServiceArr;

public:
    SvxLinguData_Impl();
    ~SvxLinguData_Impl();

    Reference<XLinguServiceManager>&    GetManager() {return xLinguSrvcMgr;}

    void AddServiceName(const ServiceInfo_Impl*);
    void SetChecked(const Sequence<OUString>& rConfiguredServices);
    void Reconfigure(const Sequence<OUString>& aRemove, const Sequence<OUString>& aInsert);

    const Sequence<OUString>&           GetAvailSrvcDisplayNames(){return  aAvailSrvcDisplayNames;}
    const Sequence<OUString>&           GetAvailSrvcNames()     {return    aAvailSrvcNames;}
//  const Sequence<sal_Bool>&           GetAvailSrvcConfigured(){return    aAvailSrvcConfigured;}
    const Sequence<Locale>&             GetAllSupportedLocales(){return aAllServiceLocales;}
    const ServiceInfoArr&               GetDisplayServiceArray() const {return aDisplayServiceArr;}
};
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
void lcl_MergeDisplayArray(ServiceInfoArr& rDisplayServiceArr, const ServiceInfo_Impl* pToAdd)
{
    for(int i = 0; i < rDisplayServiceArr.Count(); i++)
    {
        ServiceInfo_Impl* pEntry = rDisplayServiceArr[i];
        if(pEntry->sDisplayName == pToAdd->sDisplayName)
        {
            if(pToAdd->xSpell.is())
            {
                pEntry->sSpellImplName = pToAdd->sSpellImplName;
                pEntry->xSpell = pToAdd->xSpell;
            }
            if(pToAdd->xHyph.is())
            {
                pEntry->sHyphImplName = pToAdd->sHyphImplName;
                pEntry->xHyph = pToAdd->xHyph;
            }
            if(pToAdd->xThes.is())
            {
                pEntry->sThesImplName = pToAdd->sThesImplName;
                pEntry->xThes = pToAdd->xThes;
            }
            lcl_MergeLocales(pEntry->aSuppLocales, pToAdd->aSuppLocales);
            return ;
        }
    }
    ServiceInfo_Impl* pInsert = new ServiceInfo_Impl(*pToAdd);
    rDisplayServiceArr.Insert(pInsert, rDisplayServiceArr.Count());
}
/* -----------------------------26.11.00 18:07--------------------------------

 ---------------------------------------------------------------------------*/
SvxLinguData_Impl::SvxLinguData_Impl()
{
    xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XInterface > xI = xMSF->createInstance(
        C2U( "com.sun.star.linguistic2.LinguServiceManager" ) );
    xLinguSrvcMgr = Reference<XLinguServiceManager>(xI, UNO_QUERY);
    DBG_ASSERT(xLinguSrvcMgr.is(), "No linguistic service available!")
    if(xLinguSrvcMgr.is())
    {
        Locale aCurrentLocale;
        LanguageType eLang = Application::GetAppInternational().GetLanguage();
        if(LANGUAGE_SYSTEM == eLang)
        {
            eLang = GetSystemLanguage();
        }
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
            {
                pInfo->sDisplayName = xDispName->getServiceDisplayName(
                        aCurrentLocale );
                pInfo->aSuppLocales = pInfo->xSpell->getLocales();
                lcl_MergeLocales(aAllServiceLocales, pInfo->aSuppLocales);
            }

            aAvailSpellInfoArr.Insert(pInfo, aAvailSpellInfoArr.Count());

            AddServiceName(pInfo);
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
            {
                pInfo->sDisplayName = xDispName->getServiceDisplayName(
                        aCurrentLocale );
                pInfo->aSuppLocales = pInfo->xHyph->getLocales();
                lcl_MergeLocales(aAllServiceLocales, pInfo->aSuppLocales);
            }

            aAvailHyphInfoArr.Insert(pInfo, aAvailHyphInfoArr.Count());
            AddServiceName(pInfo);
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
            {
                pInfo->sDisplayName = xDispName->getServiceDisplayName(
                        aCurrentLocale );
                pInfo->aSuppLocales = pInfo->xThes->getLocales();
                //collect all supported locales
                lcl_MergeLocales(aAllServiceLocales, pInfo->aSuppLocales);
            }

            aAvailThesInfoArr.Insert(pInfo, aAvailThesInfoArr.Count());
            AddServiceName(pInfo);
        }

        const Locale* pAllLocales = aAllServiceLocales.getConstArray();
        for(sal_Int32 nLocale = 0; nLocale < aAllServiceLocales.getLength(); nLocale++)
        {
            SetChecked(xLinguSrvcMgr->getConfiguredServices(C2U(cSpell), pAllLocales[nLocale]));
            SetChecked(xLinguSrvcMgr->getConfiguredServices(C2U(cHyph), pAllLocales[nLocale]));
            SetChecked(xLinguSrvcMgr->getConfiguredServices(C2U(cThes), pAllLocales[nLocale]));
        }
    }
}
/* -----------------------------26.11.00 18:08--------------------------------

 ---------------------------------------------------------------------------*/
SvxLinguData_Impl::~SvxLinguData_Impl()
{
}
/* -----------------------------26.11.00 20:19--------------------------------

 ---------------------------------------------------------------------------*/
void SvxLinguData_Impl::AddServiceName(const ServiceInfo_Impl* pInfo)
{
    lcl_MergeDisplayArray(aDisplayServiceArr, pInfo);
    const OUString* pAvailSrvcDisplayNames = aAvailSrvcDisplayNames.getConstArray();
    sal_Bool bFound = sal_False;
    OUString sImpl = pInfo->sSpellImplName.getLength() ? pInfo->sSpellImplName :
        pInfo->sHyphImplName.getLength() ? pInfo->sHyphImplName :
        pInfo->sThesImplName;
    for(sal_Int32 nAll = 0; nAll < aAvailSrvcDisplayNames.getLength() && !bFound; nAll++)
    {
        bFound = pAvailSrvcDisplayNames[nAll] == sImpl;
    }
    if(!bFound)
    {
        aAvailSrvcNames.realloc(aAvailSrvcNames.getLength() + 1);
        aAvailSrvcDisplayNames.realloc(aAvailSrvcDisplayNames.getLength() + 1);
        OUString* pAvailSrvcNames = aAvailSrvcNames.getArray();
        OUString* pAvailSrvcDisplayNames = aAvailSrvcDisplayNames.getArray();
        pAvailSrvcNames[aAvailSrvcNames.getLength() - 1] = sImpl;
        pAvailSrvcDisplayNames[aAvailSrvcDisplayNames.getLength() - 1] = pInfo->sDisplayName;
    }
}
/* -----------------------------26.11.00 19:42--------------------------------

 ---------------------------------------------------------------------------*/
void SvxLinguData_Impl::SetChecked(const Sequence<OUString>& rConfiguredServices)
{
    const OUString* pConfiguredServices = rConfiguredServices.getConstArray();
    for(sal_Int32 n = 0; n < rConfiguredServices.getLength(); n++)
    {
        for(int i = 0; i < aDisplayServiceArr.Count(); i++)
        {
            ServiceInfo_Impl* pEntry = aDisplayServiceArr[i];
//          if(pEntry->sDisplayName == pConfiguredServices[n])
            if (pEntry->sSpellImplName == pConfiguredServices[n]  ||
                pEntry->sHyphImplName  == pConfiguredServices[n]  ||
                pEntry->sThesImplName  == pConfiguredServices[n])
            {
                pEntry->bConfigured = sal_True;
                break;
            }
        }
    }


/*  if(aAvailSrvcConfigured.getLength() != aAvailSrvcNames.getLength())
    {
        aAvailSrvcConfigured.realloc(aAvailSrvcDisplayNames.getLength());
        sal_Bool* pBool = aAvailSrvcConfigured.getArray();
        for(sal_Int32 nInit = 0; nInit < aAvailSrvcDisplayNames.getLength(); nInit++)
            pBool[nInit] = sal_False;
    }
    const OUString* pAvailSrvcNames = aAvailSrvcNames.getConstArray();
    sal_Bool* pIsConfigured = aAvailSrvcConfigured.getArray();
    const OUString* pConfiguredServices = rConfiguredServices.getConstArray();
    for(sal_Int32 n = 0; n < aAvailSrvcNames.getLength(); n++)
    {
        if(!pIsConfigured[n])
        {
            for(sal_Int32 nFind = 0; nFind < rConfiguredServices.getLength(); nFind++)
            {
                if(pConfiguredServices[nFind] == pAvailSrvcNames[n])
                {
                    pIsConfigured[n] = sal_True;
                    break;
                }
            }
        }
    }
*/}
/* -----------------------------26.11.00 20:43--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool lcl_AddRemove(Sequence<OUString>& rConfigured,
    const OUString* pRemove, const sal_Int32 nRemove,
    const OUString* pInsert, const sal_Int32 nInsert)
{
    sal_Bool bRet = sal_False;
    sal_Int32 nConfigured = rConfigured.getLength();
    rConfigured.realloc(rConfigured.getLength() + nInsert);
    OUString* pConfigured = rConfigured.getArray();
    sal_Int32 i, j;
    for(j = 0; j < nConfigured; j++)
    {
        for(i = 0; i < nRemove; i++)
        {
            if(pConfigured[j] == pRemove[i])
            {
                pConfigured[j] = OUString();
                bRet = sal_True;
                break;
            }
        }
    }
    sal_Int32 nAppend = nConfigured;
    for(i = 0; i < nInsert; i++)
    {
        sal_Bool bFound = sal_False;
        for(j = 0; j < nConfigured && !bFound; j++)
        {
            if(pConfigured[j] == pInsert[i])
                bFound = sal_True;
        }
        if(!bFound)
        {
            pConfigured[nAppend++] = pInsert[i];
            bRet = sal_True;
        }
    }
    sal_Int32 nRealloc = 0;
    for(i = 0; i < nInsert; i++)
        if(pConfigured[i].getLength())
            pConfigured[nRealloc++] = pConfigured[i];
    rConfigured.realloc(nRealloc);
    return bRet;
}

void SvxLinguData_Impl::Reconfigure(
            const Sequence<OUString>& aRemove, const Sequence<OUString>& aInsert)
{
    const OUString* pRemove = aRemove.getConstArray();
    const nRemove = aRemove.getLength();
    const OUString* pInsert = aInsert.getConstArray();
    const nInsert = aInsert.getLength();

    const Locale* pAllLocales = aAllServiceLocales.getConstArray();
    for(sal_Int32 nLocale = 0; nLocale < aAllServiceLocales.getLength(); nLocale++)
    {
        Sequence<OUString> aConfigured = xLinguSrvcMgr->getConfiguredServices(C2U(cSpell), pAllLocales[nLocale]);
        if(lcl_AddRemove(aConfigured, pRemove, nRemove, pInsert, nInsert))
            xLinguSrvcMgr->setConfiguredServices(C2U(cSpell), pAllLocales[nLocale], aConfigured);

        aConfigured = xLinguSrvcMgr->getConfiguredServices(C2U(cHyph), pAllLocales[nLocale]);
        if(lcl_AddRemove(aConfigured, pRemove, nRemove, pInsert, nInsert))
            xLinguSrvcMgr->setConfiguredServices(C2U(cHyph), pAllLocales[nLocale], aConfigured);

        aConfigured = xLinguSrvcMgr->getConfiguredServices(C2U(cThes), pAllLocales[nLocale]);
        if(lcl_AddRemove(aConfigured, pRemove, nRemove, pInsert, nInsert))
            xLinguSrvcMgr->setConfiguredServices(C2U(cThes), pAllLocales[nLocale], aConfigured);
    }
}
// SvxModulesData_Impl ---------------------------------------------------
struct SvxModulesData_Impl
{
    OUString    sServiceImplName;
    sal_Bool    bIsConfigured;
    SvxModulesData_Impl(const OUString& rSrvc, sal_Bool bSet) :
        sServiceImplName(rSrvc),
        bIsConfigured(bSet){}
};

// class SvxLinguTabPage -------------------------------------------------

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1
#define CBCOL_BOTH      2

SvxLinguTabPage::SvxLinguTabPage( Window* pParent,
                                  const SfxItemSet& rSet ):

    SfxTabPage( pParent, ResId( RID_SFXPAGE_LINGU, DIALOG_MGR() ), rSet ),

    aLinguisticGB       ( this, ResId( GB_LINGUISTIC ) ),
    aLinguModulesFT     ( this, ResId( FT_LINGU_MODULES ) ),
    aLinguModulesCLB    ( this, ResId( CLB_LINGU_MODULES ) ),
    aLinguModulesEditPB ( this, ResId( PB_LINGU_MODULES_EDIT ) ),
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

    aChkunBmp           ( ResId( BMP_CHKBUT_UNCHECKED ) ),
    aChkchBmp           ( ResId( BMP_CHKBUT_CHECKED ) ),
    pLinguData          ( NULL )
{
    pCheckButtonData = NULL;
    aLinguModulesCLB.SetWindowBits( WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguOptionsCLB.SetWindowBits( WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguModulesCLB.SetHighlightRange();
    aLinguOptionsCLB.SetHighlightRange();
    aLinguModulesCLB.SetHelpId(HID_CLB_LINGU_MODULES );
    aLinguOptionsCLB.SetHelpId(HID_CLB_LINGU_OPTIONS );

    xProp = Reference< XPropertySet >( SvxGetLinguPropertySet(), UNO_QUERY );

    aLinguModulesCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguOptionsCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguModulesEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguOptionsEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguOptionsCLB.SetDoubleClickHdl(LINK(this, SvxLinguTabPage, DoubleClickHdl_Impl));
    aLinguModulesCLB.SetDoubleClickHdl(LINK(this, SvxLinguTabPage, DoubleClickHdl_Impl));

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
    for(sal_uInt16 nEntry = 0; nEntry < aLinguModulesCLB.GetEntryCount(); nEntry++)
    {
        SvxModulesData_Impl* pData = (SvxModulesData_Impl*)aLinguModulesCLB.GetEntry(nEntry)->GetUserData();
        delete pData;
    }
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
        SvStringsDtor aToRemove;
        SvStringsDtor aToInsert;
        for(sal_uInt16 i = 0; i < aLinguModulesCLB.GetEntryCount(); i++)
        {
            SvLBoxEntry* pEntry = aLinguModulesCLB.GetEntry(i);
            SvxModulesData_Impl* pData = (SvxModulesData_Impl*) pEntry->GetUserData();
            if(pData->bIsConfigured != aLinguModulesCLB.IsChecked( i ))
            {
                if(pData->bIsConfigured)
                    aToRemove.Insert(new String(pData->sServiceImplName), aToRemove.Count());
                else if(!pData->bIsConfigured)
                    aToInsert.Insert(new String(pData->sServiceImplName), aToInsert.Count());
            }
        }
        if(aToRemove.Count() || aToInsert.Count())
        {
            Sequence<OUString> aRemove(aToRemove.Count());
            OUString* pRemove = aRemove.getArray();
            USHORT i;
            for(i = 0; i < aToRemove.Count(); i++)
                pRemove[i] = OUString(*aToRemove[i]);
            Sequence<OUString> aInsert(aToInsert.Count());
            OUString* pInsert = aInsert.getArray();
            for(i = 0; i < aToInsert.Count(); i++)
                pInsert[i] = OUString(*aToInsert[i]);

            if (!pLinguData)
                pLinguData =    new SvxLinguData_Impl;
            pLinguData->Reconfigure(aRemove, aInsert);
        }
    }


    BOOL bCheckAllLangChanged = FALSE;

    SvtLinguConfig aLngCfg;
    USHORT nEntries = (USHORT) aLinguOptionsCLB.GetEntryCount();
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

//------------------------------------------------------------------------

void SvxLinguTabPage::Reset( const SfxItemSet& rSet )
{
    // if not HideGroups was called with GROUP_MODULES...
    if (aLinguModulesCLB.IsVisible())
    {
        if (!pLinguData)
            pLinguData = new SvxLinguData_Impl;
        const ServiceInfoArr&   rAllDispSrvcArr = pLinguData->GetDisplayServiceArray();

        for(sal_uInt16 i = 0; i < rAllDispSrvcArr.Count(); i++)
        {
            ServiceInfo_Impl* pInfo = rAllDispSrvcArr[i];
            aLinguModulesCLB.InsertEntry(pInfo->sDisplayName);
            SvLBoxEntry* pEntry = aLinguModulesCLB.GetEntry(i);
            pEntry->SetUserData( new SvxModulesData_Impl(
                                 pInfo->sDisplayName, pInfo->bConfigured ) );
            aLinguModulesCLB.CheckEntryPos( i, pInfo->bConfigured );
        }
        aLinguModulesEditPB.Enable( rAllDispSrvcArr.Count() > 0 );
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
    if ( rSet.GetItemState( nWhich, FALSE ) >= SFX_ITEM_AVAILABLE )
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

IMPL_LINK( SvxLinguTabPage, DoubleClickHdl_Impl, SvxCheckListBox *, pBox )
{
    if(pBox == &aLinguOptionsCLB)
        ClickHdl_Impl(&aLinguOptionsEditPB);
    else
        ClickHdl_Impl(&aLinguModulesEditPB);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxLinguTabPage, ClickHdl_Impl, PushButton *, pBtn )
{
    if (&aLinguModulesEditPB == pBtn)
    {
        if (!pLinguData)
            pLinguData = new SvxLinguData_Impl;
        SvxEditModulesDlg aDlg( this, *pLinguData );
        aDlg.Execute();
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

SvLBoxEntry* SvxLinguTabPage::CreateEntry(String& rTxt, USHORT nCol)
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData;
        pCheckButtonData->aBmps[SV_BMP_UNCHECKED] = aChkunBmp;
        pCheckButtonData->aBmps[SV_BMP_CHECKED]   = aChkchBmp;
    }

    String sEmpty;
    if (CBCOL_FIRST == nCol)
        pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // Leerspalte
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!
    pEntry->AddItem( new BrwString_Impl( pEntry, 0, rTxt ) );

    return pEntry;
}

// ----------------------------------------------------------------------

#if SUPD <= 623
void SvxLinguTabPage::UpdateBox_Impl()
{
}
#endif

// -----------------------------------------------------------------------

void SvxLinguTabPage::HideGroups( sal_uInt16 nGrp )
{
    if ( 0 != ( GROUP_MODULES & nGrp ) )
    {
        aLinguModulesFT.Hide();
        aLinguModulesCLB.Hide();
        aLinguModulesEditPB.Hide();
    }

#ifdef NEVER
    if ( 0 != ( GROUP_SPELLING & nGrp ) )
    {
        aCapsBtn.Hide();
        aNumsBtn.Hide();
        aUpLowBtn.Hide();
        aAllLangBtn.Hide();
        aAuditBox.Hide();
    }

    if ( 0 != ( GROUP_HYPHEN & nGrp ) )
    {
        aPreBreakText.Hide();
        aPreBreakEdit.Hide();
        aAfterBreakText.Hide();
        aAfterBreakEdit.Hide();
        aAutoBtn.Hide();
        aSpecialBtn.Hide();
        aRulesBox.Hide();
    }
#endif
}
/*--------------------------------------------------
--------------------------------------------------*/

SvxEditModulesDlg::SvxEditModulesDlg(Window* pParent, SvxLinguData_Impl& rData) :
        ModalDialog(pParent, ResId(RID_SVXDLG_EDIT_MODULES, DIALOG_MGR() )),
        aClosePB    ( this, ResId( PB_OK ) ),
        aHelpPB     ( this, ResId( PB_HELP ) ),
        aModulesGB  ( this, ResId( GB_EDIT_MODULES_OPTIONS ) ),
        aLanguageFT ( this, ResId( FT_EDIT_MODULES_LANGUAGE ) ),
        aLanguageLB ( this, ResId( LB_EDIT_MODULES_LANGUAGE ), FALSE ),
        aModulesCLB ( this, ResId( CLB_EDIT_MODULES_MODULES ) ),
        aPrioUpPB   ( this, ResId( PB_EDIT_MODULES_PRIO_UP ) ),
        aPrioDownPB ( this, ResId( PB_EDIT_MODULES_PRIO_DOWN ) ),
        aBackPB     ( this, ResId( PB_EDIT_MODULES_PRIO_BACK ) ),
        aDicsGB     ( this, ResId( GB_EDIT_MODULES_DICS ) ),
        aDicsCLB    ( this, ResId( CLB_EDIT_MODULES_DICS ) ),
        aNewPB      ( this, ResId( PB_EDIT_MODULES_NEW_DIC ) ),
        aEditPB     ( this, ResId( PB_EDIT_MODULES_PRIO_EDIT_DIC ) ),
        aDeletePB   ( this, ResId( PB_EDIT_MODULES_PRIO_DEL_DIC ) ),
        aChkunBmp   ( ResId( BMP_CHKBUT_UNCHECKED ) ),
        aChkchBmp   ( ResId( BMP_CHKBUT_CHECKED ) ),
        sSpell( ResId( ST_SPELL)),
        sHyph( ResId( ST_HYPH)),
        sThes( ResId( ST_THES)),
        rLinguData(rData)
{
    pCheckButtonData = NULL;
    FreeResource();

    aModulesCLB.SetWindowBits( WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aDicsCLB.SetWindowBits( WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aModulesCLB.SetHighlightRange();
    aDicsCLB.SetHighlightRange();
    aModulesCLB.SetHelpId(HID_CLB_EDIT_MODULES_MODULES );
    aDicsCLB.SetHelpId(HID_CLB_EDIT_MODULES_DICS );

    aModulesCLB.SetSelectHdl( LINK( this, SvxEditModulesDlg, SelectHdl_Impl ));
    aDicsCLB.SetSelectHdl( LINK( this, SvxEditModulesDlg, SelectHdl_Impl ));
    aNewPB.SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aEditPB.SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aClosePB.SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aDeletePB.SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aPrioUpPB    .SetClickHdl( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    aPrioDownPB.SetClickHdl( LINK( this, SvxEditModulesDlg, UpDownHdl_Impl ));
    aBackPB.SetClickHdl(LINK( this, SvxEditModulesDlg, BackHdl_Impl));

    aModulesCLB.SetCheckButtonHdl( LINK( this, SvxEditModulesDlg, CheckButtonHdl_Impl) );

    xDicList = Reference< XDictionaryList >( SvxGetDictionaryList(), UNO_QUERY );
    if (xDicList.is())
    {
        UpdateDicBox_Impl();
    }
    else
    {
        aDicsGB.Disable();
        aDicsCLB.Disable();
        aNewPB.Disable();
        aEditPB.Disable();
        aDeletePB.Disable();
    }
    //fill option CheckListBox
    aLanguageLB.Clear();
    const Sequence<Locale>& rLoc = rLinguData.GetAllSupportedLocales();
    const Locale* pLocales = rLoc.getConstArray();
    for(long i = 0; i < rLoc.getLength(); i++)
    {
        aLanguageLB.InsertLanguage( SvxLocaleToLanguage( pLocales[i] ));
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
}


SvLBoxEntry* SvxEditModulesDlg::CreateEntry( const String& rTxt, USHORT nCol )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;
    if( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData;
        pCheckButtonData->aBmps[SV_BMP_UNCHECKED] = aChkunBmp;
        pCheckButtonData->aBmps[SV_BMP_CHECKED]   = aChkchBmp;
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

void SvxEditModulesDlg::UpdateDicBox_Impl()
{
    aDicsCLB.SetUpdateMode(FALSE);
    aDicsCLB.Clear();

    SvLBoxTreeList *pModel = aDicsCLB.GetModel();
    SvLBoxEntry* pEntry = NULL;

    aDics = xDicList->getDictionaries();
    INT32 nDics  = aDics.getLength();
    const Reference< XDictionary > *pDic = aDics.getConstArray();
    for (INT32 i = 0;  i < nDics;  ++i)
    {
        const Reference< XDictionary > &rDic = pDic[i];
        if (rDic.is())
        {
            Reference< frame::XStorable > xStor( rDic, UNO_QUERY );

            ULONG nUserData = 0;
            BOOL bChecked = rDic->isActive();
            BOOL bEditable = !xStor.is() || !xStor->isReadonly();
            BOOL bDeletable = bEditable;
            BOOL bNegativ = rDic->getDictionaryType() == DictionaryType_NEGATIVE;
            String aTxt( ::GetDicInfoStr( rDic->getName(),
                                SvxLocaleToLanguage( rDic->getLocale() ),
                                bNegativ ) );

            aDicsCLB.InsertEntry(aTxt);
            SvLBoxEntry* pEntry = aDicsCLB.GetEntry(i);

//          pEntry = CreateEntry( aTxt, CBCOL_FIRST );
            nUserData = DicUserData( (USHORT)i,
                    bChecked, bEditable, bDeletable ).GetUserData();
            pEntry->SetUserData( (void *)nUserData );
//  pModel->Insert( pEntry );
            lcl_SetCheckButton( pEntry, bChecked );
        }
    }

    aDicsCLB.SetUpdateMode(TRUE);
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
    else if (&aDicsCLB == pBox)
    {
        SvLBoxEntry *pEntry = pBox->GetCurEntry();
        if (pEntry)
        {
            DicUserData aData( (ULONG) pEntry->GetUserData() );
            aEditPB  .Enable( aData.IsEditable() );
            aDeletePB.Enable( aData.IsDeletable() );
        }
    }
    else
    {
        DBG_ERROR( "pBox unexpected value" );
    }

    return 0;
}
/* -----------------------------30.01.01 10:50--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SvxEditModulesDlg, CheckButtonHdl_Impl, SvTreeListBox *, pBox )
{
    if (pBox == &aModulesCLB)
    {
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
    }
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
    if(pBox)
    {
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
                    aChange.realloc(nStart);
                    xMgr->setConfiguredServices(
                        lcl_GetServiceName(pData->GetType() - 1), aLastLocale, aChange);
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
            xMgr->setConfiguredServices(lcl_GetServiceName(TYPE_THES), aLastLocale, aChange);
        }
    }

    for(ULONG i = 0; i < aModulesCLB.GetEntryCount(); i++)
        delete (ModuleUserData_Impl*)aModulesCLB.GetEntry(i)->GetUserData();

    aModulesCLB.Clear();
    if(LANGUAGE_DONTKNOW != eCurLanguage)
    {
        sal_Int32 nEntryPos = 1;
        sal_uInt16 n;

        SvLBoxEntry* pEntry = CreateEntry( sSpell,  CBCOL_SECOND );
        ModuleUserData_Impl* pUserData = new ModuleUserData_Impl( String(), TRUE, FALSE, TYPE_SPELL, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );

        const ServiceInfoArr&   rAllDispSrvcArr = rLinguData.GetDisplayServiceArray();

        Sequence<OUString> aSpellSrvc = xMgr->getConfiguredServices(C2U(cSpell), aCurLocale);
        sal_Int32 nLocalIndex = 0;  // index relative to parent
        for(n = 0; n < rAllDispSrvcArr.Count(); n++)
        {
            ServiceInfo_Impl* pInfo = rAllDispSrvcArr[n];

            const OUString &rImplName = pInfo->sSpellImplName;
            if (rImplName.getLength())
            {
                SvLBoxEntry* pEntry = CreateEntry( pInfo->sDisplayName, CBCOL_FIRST );
                BOOL bChecked = lcl_SeqHasEntry( aSpellSrvc, rImplName );
                lcl_SetCheckButton( pEntry, bChecked );
                pUserData = new ModuleUserData_Impl( rImplName, FALSE,
                                        bChecked, TYPE_SPELL, nLocalIndex++ );
                pEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pEntry );
            }
        }

        pEntry = CreateEntry( sHyph,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( String(), TRUE, FALSE, TYPE_HYPH, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );

        Sequence<OUString> aHyphSrvc = xMgr->getConfiguredServices(C2U(cHyph), aCurLocale);
        nLocalIndex = 0;
        for(n = 0; n < rAllDispSrvcArr.Count(); n++)
        {
            ServiceInfo_Impl* pInfo = rAllDispSrvcArr[n];

            const OUString &rImplName = pInfo->sHyphImplName;
            if (rImplName.getLength())
            {
                SvLBoxEntry* pEntry = CreateEntry( pInfo->sDisplayName, CBCOL_FIRST );
                BOOL bChecked = lcl_SeqHasEntry( aHyphSrvc, rImplName );
                lcl_SetCheckButton( pEntry, bChecked );
                pUserData = new ModuleUserData_Impl( rImplName, FALSE,
                                        bChecked, TYPE_HYPH, nLocalIndex++ );
                pEntry->SetUserData( (void *)pUserData );
                pModel->Insert( pEntry );
            }
        }

        pEntry = CreateEntry( sThes,    CBCOL_SECOND );
        pUserData = new ModuleUserData_Impl( String(), TRUE, FALSE, TYPE_THES, 0 );
        pEntry->SetUserData( (void *)pUserData );
        pModel->Insert( pEntry );

        Sequence<OUString> aThesSrvc = xMgr->getConfiguredServices(C2U(cThes), aCurLocale);
        nLocalIndex = 0;
        for(n = 0; n < rAllDispSrvcArr.Count(); n++)
        {
            ServiceInfo_Impl* pInfo = rAllDispSrvcArr[n];

            const OUString &rImplName = pInfo->sThesImplName;
            if (rImplName.getLength())
            {
                SvLBoxEntry* pEntry = CreateEntry( pInfo->sDisplayName, CBCOL_FIRST );
                BOOL bChecked = lcl_SeqHasEntry( aThesSrvc, rImplName );
                lcl_SetCheckButton( pEntry, bChecked );
                pUserData = new ModuleUserData_Impl( rImplName, FALSE,
                                        bChecked, TYPE_THES, nLocalIndex++ );
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
    aModulesCLB.SetUpdateMode(FALSE);
    sal_Bool bUp = &aPrioUpPB == pBtn;
    USHORT  nCurPos = aModulesCLB.GetSelectEntryPos();
    SvLBoxEntry* pEntry = aModulesCLB.GetEntry(nCurPos);
    SvLBoxTreeList *pModel = aModulesCLB.GetModel();

    ModuleUserData_Impl* pData = (ModuleUserData_Impl*)pEntry->GetUserData();
    SvLBoxEntry* pToInsert = CreateEntry( aModulesCLB.GetEntryText(pEntry), CBCOL_FIRST );
    pToInsert->SetUserData( (void *)pData);
    BOOL bIsChecked = aModulesCLB.IsChecked(nCurPos);

    pModel->Remove(pEntry);

    USHORT nDestPos = bUp ? nCurPos - 1 : nCurPos + 1;
    pModel->Insert(pToInsert, nDestPos);
    aModulesCLB.CheckEntryPos(nDestPos, bIsChecked );
    aModulesCLB.SelectEntryPos(nDestPos );
    aModulesCLB.SetUpdateMode(TRUE);
    SelectHdl_Impl(&aModulesCLB);

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

        Sequence< OUString > aActiveDics;
        INT32 nActiveDics = 0;

        // activate dictionaries according to checkbox state
        ULONG nEntries = aDicsCLB.GetEntryCount();
        for (ULONG i = 0;  i < nEntries;  ++i)
        {
            INT32 nDics = aDics.getLength();
            const Reference< XDictionary > *pDic = aDics.getConstArray();

            aActiveDics.realloc( nDics );
            OUString *pActiveDic = aActiveDics.getArray();

            SvLBoxEntry *pEntry = aDicsCLB.GetEntry( i );
            if (pEntry)
            {
                DicUserData aData( (ULONG)pEntry->GetUserData() );
                if (aData.GetEntryId() < nDics)
                {
                    BOOL bChecked = aDicsCLB.IsChecked( (USHORT) i );
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

        aActiveDics.realloc( nActiveDics );
        Any aTmp;
        aTmp <<= aActiveDics;
        SvtLinguConfig aLngCfg;
        aLngCfg.SetProperty( UPH_ACTIVE_DICTIONARIES, aTmp );

        EndDialog( RET_OK );
    }
    else if (&aNewPB == pBtn)
    {
        SvxNewDictionaryDialog aDlg( this, Reference< XSpellChecker1 >() );

        Reference< XDictionary1 >  xNewDic;
        if ( aDlg.Execute() == RET_OK )
            xNewDic = aDlg.GetNewDictionary();

        if ( xNewDic.is() )
        {
            UpdateDicBox_Impl();
        }
    }
    else if (&aEditPB == pBtn)
    {
        SvLBoxEntry *pEntry = aDicsCLB.GetCurEntry();
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
                    SvxEditDictionaryDialog aDlg( this,
                            xDic->getName(), Reference< XSpellChecker1 >() );
                    aDlg.Execute();

                    USHORT nOldPos = aDicsCLB.GetSelectEntryPos();
                    UpdateDicBox_Impl();
                    aDicsCLB.SelectEntryPos( nOldPos );
                }
            }
        }
    }
    else if (&aDeletePB == pBtn)
    {
        if ( RET_NO ==
             QueryBox( this, ResId( RID_SFXQB_DELDICT, DIALOG_MGR() ) ).Execute() )
            return 0;

        SvLBoxEntry *pEntry = aDicsCLB.GetCurEntry();
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
                                KillFile_Impl( aObj.GetMainURL() );
                            }
                        }
                        UpdateDicBox_Impl();
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
/* -----------------------------27.11.00 20:31--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK( SvxEditModulesDlg, BackHdl_Impl, PushButton *, pBtn )
{
    LangSelectHdl_Impl(0);
    return 0;
}

