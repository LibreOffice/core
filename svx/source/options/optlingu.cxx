/*************************************************************************
 *
 *  $RCSfile: optlingu.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tl $ $Date: 2000-11-27 09:22:28 $
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

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX
#include <dlgutil.hxx>
#endif
#ifndef _LINGU_LNGPROPS_HHX_
#include <lingu/lngprops.hxx>
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
    SvLBoxButton* pItem = (SvLBoxButton*)pEntry->GetItem( 1 );

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
    Point aPos(rPos);
    aPos.X() += 20;
    rDev.DrawText( aPos, GetText() );
}

/*--------------------------------------------------
--------------------------------------------------*/

SvxEditModulesDlg::SvxEditModulesDlg(Window* pParent) :
        ModalDialog(pParent, ResId(RID_SVXDLG_EDIT_MODULES, DIALOG_MGR() )),
        aClosePB    ( this, ResId( PB_OK ) ),
        aHelpPB     ( this, ResId( PB_HELP ) ),
        aModulesGB  ( this, ResId( GB_EDIT_MODULES_OPTIONS ) ),
        aLanguageFT ( this, ResId( FT_EDIT_MODULES_LANGUAGE ) ),
        aLanguageLB ( this, ResId( LB_EDIT_MODULES_LANGUAGE ) ),
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
        aChkchBmp   ( ResId( BMP_CHKBUT_CHECKED ) )
{
    pCheckButtonData = NULL;
    FreeResource();
    aModulesCLB.SetHighlightRange();
    aDicsCLB.SetHighlightRange();
    aModulesCLB.SetHelpId(HID_CLB_EDIT_MODULES_MODULES );
    aDicsCLB.SetHelpId(HID_CLB_EDIT_MODULES_DICS );

    aModulesCLB.SetSelectHdl( LINK( this, SvxEditModulesDlg, SelectHdl_Impl ));
    aDicsCLB.SetSelectHdl( LINK( this, SvxEditModulesDlg, SelectHdl_Impl ));
    aNewPB.SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aEditPB.SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aDeletePB.SetClickHdl( LINK( this, SvxEditModulesDlg, ClickHdl_Impl ));
    aLanguageLB.SetSelectHdl( LINK( this, SvxEditModulesDlg, SelectHdlLB_Impl ));

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
}


SvxEditModulesDlg::~SvxEditModulesDlg()
{
}


BOOL SvxEditModulesDlg::Close()
{
    BOOL bAllowClose = TRUE;

    ULONG nEntries = aDicsCLB.GetEntryCount();
    for (ULONG i = 0;  i < nEntries;  ++i)
    {
        INT32 nDics = aDics.getLength();
        const Reference< XDictionary > *pDic = aDics.getConstArray();

        SvLBoxEntry *pEntry = aDicsCLB.GetEntry( i );
        if (pEntry)
        {
            DicUserData aData( (ULONG)pEntry->GetUserData() );
            if (aData.GetEntryId() < nDics)
            {
                BOOL bChecked = aDicsCLB.IsChecked( i );
                Reference< XDictionary > xDic( aDics.getConstArray()[ i ] );
                if (xDic.is())
                {
                    String aDicName( xDic->getName() );
                    xDic->setActive( bChecked );
                }
            }
        }
    }

    return bAllowClose;
}


SvLBoxEntry* SvxEditModulesDlg::CreateEntry( String& rTxt, USHORT nCol )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData;
        pCheckButtonData->aBmps[SV_BMP_UNCHECKED] = aChkunBmp;
        pCheckButtonData->aBmps[SV_BMP_CHECKED]   = aChkchBmp;
    }

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!

    String sEmpty;
    if (CBCOL_FIRST == nCol)
        pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // Leerspalte
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

            pEntry = CreateEntry( aTxt, CBCOL_FIRST );
            nUserData = DicUserData( (USHORT)i,
                    bChecked, bEditable, bDeletable ).GetUserData();
            pEntry->SetUserData( (void *)nUserData );
            pModel->Insert( pEntry );
            lcl_SetCheckButton( pEntry, bChecked );
        }
    }

    aDicsCLB.SetUpdateMode(TRUE);
}

IMPL_LINK( SvxEditModulesDlg, SelectHdl_Impl, SvxCheckListBox *, pBox )
{
    if (&aModulesCLB == pBox)
    {
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


IMPL_LINK( SvxEditModulesDlg, SelectHdlLB_Impl, ListBox *, pBox )
{
    if (&aLanguageLB == pBox)
    {
    }
    else
    {
        DBG_ERROR( "pBox unexpected value" );
    }

    return 0;
}


IMPL_LINK( SvxEditModulesDlg, ClickHdl_Impl, PushButton *, pBtn )
{
    if (&aNewPB == pBtn)
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

/*--------------------------------------------------
--------------------------------------------------*/

class OptionsBreakSet : public ModalDialog
{
    OKButton        aOKPB;
    CancelButton    aCancelPB;
    GroupBox        aValGB;
    NumericField    aValNF;

public:
    OptionsBreakSet(Window* pParent, const String& rGroupBoxTitle) :
            ModalDialog(pParent, ResId(RID_SVXDLG_LNG_ED_NUM_PREBREAK, DIALOG_MGR() )),
            aOKPB       (this, ResId(BT_OK_PREBREAK)),
            aCancelPB   (this, ResId(BT_CANCEL_PREBREAK)),
            aValGB      (this, ResId(GB_NUMVAL_PREBREAK)),
            aValNF      (this, ResId(ED_PREBREAK))
    {
        FreeResource();
        aValGB.SetText(rGroupBoxTitle);
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
    EID_NUM_PRE_BREAK,
    EID_NUM_POST_BREAK,
    EID_HYPH_AUTO,
    EID_HYPH_SPECIAL
};

// class OptionsUserData -------------------------------------------------

class OptionsUserData
{
    ULONG   nVal;

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
    if (IsCheckable())
    {
        nVal &= ~(1UL << 8);
        nVal |=  (ULONG)(bVal ? 1 : 0) << 8;
    }
}

void OptionsUserData::SetNumericValue( BYTE nNumVal )
{
    DBG_ASSERT( nNumVal < 256, "value out of range" );
    if (HasNumericValue())
    {
        nVal &= 0xffffff00;
        nVal |= (nNumVal);
    }
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
    OUString                    sImplName;
    OUString                    sDisplayName;
    Sequence <Locale>           aSuppLocales;
    Reference <XSpellChecker>   xSpell;
    Reference <XHyphenator>     xHyph;
    Reference <XThesaurus>      xThes;
};
SV_DECL_PTRARR_DEL(ServiceInfoArr, ServiceInfo_Impl*, 2, 2);
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
    Sequence<sal_Bool>                  aAvailSrvcConfigured;

    Sequence<Locale>                    aAllServiceLocales;

public:
    SvxLinguData_Impl();
    ~SvxLinguData_Impl();

    Reference<XLinguServiceManager>&    GetManager() {return xLinguSrvcMgr;}

    void AddServiceName(const ServiceInfo_Impl*);
    void SetChecked(const Sequence<OUString>& rConfiguredServices);
    void Reconfigure(const Sequence<OUString>& aRemove, const Sequence<OUString>& aInsert);

    const Sequence<OUString>&           GetAvailSrvcDisplayNames(){return  aAvailSrvcDisplayNames;}
    const Sequence<OUString>&           GetAvailSrvcNames()     {return    aAvailSrvcNames;}
    const Sequence<sal_Bool>&           GetAvailSrvcConfigured(){return    aAvailSrvcConfigured;}

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
            pInfo->sImplName = pSpellNames[nIdx];
            pInfo->xSpell = Reference<XSpellChecker>(
                            xMSF->createInstanceWithArguments(pInfo->sImplName, aArgs), UNO_QUERY);
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
            pInfo->sImplName = pHyphNames[nIdx];
            pInfo->xHyph = Reference<XHyphenator>(
                            xMSF->createInstanceWithArguments(pInfo->sImplName, aArgs), UNO_QUERY);
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
            pInfo->sImplName = pThesNames[nIdx];
            pInfo->xThes = Reference<XThesaurus>(
                            xMSF->createInstanceWithArguments(pInfo->sImplName, aArgs), UNO_QUERY);
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
    const OUString* pAvailSrvcDisplayNames = aAvailSrvcDisplayNames.getConstArray();
    sal_Bool bFound = sal_False;
    for(sal_Int32 nAll = 0; nAll < aAvailSrvcDisplayNames.getLength() && !bFound; nAll++)
    {
        bFound = pAvailSrvcDisplayNames[nAll] == pInfo->sImplName;
    }
    if(!bFound)
    {
        aAvailSrvcNames.realloc(aAvailSrvcNames.getLength() + 1);
        aAvailSrvcDisplayNames.realloc(aAvailSrvcDisplayNames.getLength() + 1);
        OUString* pAvailSrvcNames = aAvailSrvcNames.getArray();
        OUString* pAvailSrvcDisplayNames = aAvailSrvcDisplayNames.getArray();
        pAvailSrvcNames[aAvailSrvcNames.getLength() - 1] = pInfo->sImplName;
        pAvailSrvcDisplayNames[aAvailSrvcDisplayNames.getLength() - 1] = pInfo->sDisplayName;
    }
}
/* -----------------------------26.11.00 19:42--------------------------------

 ---------------------------------------------------------------------------*/
void SvxLinguData_Impl::SetChecked(const Sequence<OUString>& rConfiguredServices)
{
    if(aAvailSrvcConfigured.getLength() != aAvailSrvcNames.getLength())
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
}
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
            xLinguSrvcMgr->setConfiguredServices(C2U(cSpell), pAllLocales[nLocale], aConfigured);

        aConfigured = xLinguSrvcMgr->getConfiguredServices(C2U(cThes), pAllLocales[nLocale]);
        if(lcl_AddRemove(aConfigured, pRemove, nRemove, pInsert, nInsert))
            xLinguSrvcMgr->setConfiguredServices(C2U(cSpell), pAllLocales[nLocale], aConfigured);
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
    sNumPreBreak        ( ResId( STR_NUM_PRE_BREAK ) ),
    sNumPostBreak       ( ResId( STR_NUM_POST_BREAK ) ),
    sHyphAuto           ( ResId( STR_HYPH_AUTO ) ),
    sHyphSpecial        ( ResId( STR_HYPH_SPECIAL ) ),

    aChkunBmp           ( ResId( BMP_CHKBUT_UNCHECKED ) ),
    aChkchBmp           ( ResId( BMP_CHKBUT_CHECKED ) ),
    pLinguData          (new SvxLinguData_Impl)

#ifdef NEVER
    aPreBreakEdit       ( this, ResId( ED_PREBREAK ) ),
    aAfterBreakEdit     ( this, ResId( ED_AFTERBREAK ) ),
#endif
{
    pCheckButtonData = NULL;
    aLinguModulesCLB.SetWindowBits( WB_HASBUTTONS|WB_HASBUTTONSATROOT|
                            WB_CLIPCHILDREN|WB_HSCROLL|WB_FORCE_MAKEVISIBLE );
    aLinguModulesCLB.SetHighlightRange();
    aLinguOptionsCLB.SetHighlightRange();
    aLinguModulesCLB.SetHelpId(HID_CLB_LINGU_MODULES );
    aLinguOptionsCLB.SetHelpId(HID_CLB_LINGU_OPTIONS );

    xProp = Reference< XPropertySet >( SvxGetLinguPropertySet(), UNO_QUERY );
    if (!xProp.is())
    {
        aLinguOptionsFT.Disable();
        aLinguOptionsCLB.Disable();
        aLinguOptionsEditPB.Disable();
    }

    aLinguModulesCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguOptionsCLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectHdl_Impl ));
    aLinguModulesEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguOptionsEditPB.SetClickHdl( LINK( this, SvxLinguTabPage, ClickHdl_Impl ));
    aLinguOptionsCLB.SetDoubleClickHdl(LINK(this, SvxLinguTabPage, DoubleClickHdl_Impl));
    aLinguModulesCLB.SetDoubleClickHdl(LINK(this, SvxLinguTabPage, DoubleClickHdl_Impl));

#ifdef NEVER
    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
    if (xDicList.is())
        aDics = xDicList->getDictionaries();
    aDictsTbx.SetSelectHdl( LINK( this, SvxLinguTabPage, BoxSelectHdl_Impl ) );
    Size aSiz = aDictsTbx.CalcWindowSizePixel();
    aDictsTbx.SetSizePixel( aSiz );

    const SfxSpellCheckItem* pItem = 0;
    SfxItemState eItemState = SFX_ITEM_UNKNOWN;

    eItemState = rSet.GetItemState( GetWhich( SID_ATTR_SPELL ),
                                    sal_False, (const SfxPoolItem**)&pItem );

    // handelt es sich um ein Default-Item?
    if ( eItemState == SFX_ITEM_DEFAULT )
        pItem = (const SfxSpellCheckItem*)&(rSet.Get( GetWhich( SID_ATTR_SPELL ) ) );
    else if ( eItemState == SFX_ITEM_DONTCARE )
        pItem = NULL;


    // fill box for DefaultLanguage (?) with possible languages
    const sal_uInt16 nLangCnt = SvxGetSelectableLanguages().getLength();
    const util::Language *pLang = SvxGetSelectableLanguages().getConstArray();
    // start with 1 to skip LANGUAGE_NONE
    for ( sal_uInt16 i = 1; i < nLangCnt; i++ )
    {
        sal_uInt16 nPos = aLanguageLB.InsertEntry( GetLanguageString( pLang[i] ) );
        aLanguageLB.SetEntryData( nPos, (void*)(sal_uInt32)i );
    }

    // disable button if IgnoreAllList has no elements

    aAutoCheckBtn.SetClickHdl( LINK( this, SvxLinguTabPage, CheckHdl_Impl ) );
    aDictsLB.SetCheckButtonHdl( LINK( this, SvxLinguTabPage, CheckDicHdl_Impl));
    aDictsLB.SetSelectHdl( LINK( this, SvxLinguTabPage, SelectDicHdl_Impl));

    // disable controls that can't be used due to missing services
    if (!SvxGetLinguPropertySet().is())
        Enable( sal_False );    // disable everything
    else if (!xDicList.is())
    {   // disable dictionary and dictionary list controls
        aDictsLB .Enable( sal_False );
        aDictsTbx.Enable( sal_False );
        aDictsBox.Enable( sal_False );
    }
#endif

    FreeResource();
}

// -----------------------------------------------------------------------

SvxLinguTabPage::~SvxLinguTabPage()
{
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

        pLinguData->Reconfigure(aRemove, aInsert);
    }

    if (!SvxGetLinguPropertySet().is())
        return sal_False;

#ifdef NEVER
    //
    // build list of active dictionaries
    // (There has to be at least one active positive dictionary
    // this usually is the "standard" dictionary. It will be created
    // if it does not already exist.)
    //

    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );

    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    const sal_uInt16 nCount = aDics.getLength();
    const sal_uInt16 nSize  = (sal_uInt16)aDictsLB.GetEntryCount();
    //
    sal_Bool bIsAnyPosDicActive = sal_False;
    for (sal_uInt16 j = 0;  j < nCount;  j++)
    {
        Reference< XDictionary1 >  xDic( pDic[j], UNO_QUERY );
        if (!xDic.is())
            continue;

        sal_Bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
        const String aName( ::GetDicInfoStr( xDic->getName(),
                                             xDic->getLanguage(),
                                             bNegativ ) );

        sal_Bool bActivate = sal_False;
        for ( sal_uInt16 i = 0; i < nSize; ++i )
        {
            // Dictionary in der awt::Selection -> Aktivieren
            if ( aDictsLB.IsChecked(i) && aName == aDictsLB.GetText(i) )
            {
                // any dictionary is persistent not readonly and may hold
                // positive entries?
                Reference< frame::XStorable >  xStor( xDic, UNO_QUERY );
                if (xStor.is() && xStor->hasLocation() && !xStor->isReadonly())
                    bIsAnyPosDicActive =
                            xDic->getDictionaryType() != DictionaryType_NEGATIVE;

                bActivate = sal_True;
                break;
            }
        }
        if (xDic != SvxGetIgnoreAllList())
            xDic->setActive( bActivate );
    }
#endif

    // force new spelling and flushing of spell caches
    //! current implementation is a workaround until the correct
    //! interface is implemented.
    //TL:TODO: use XPropertyChangeListener mechanism to do this
    Reference< XDictionary1 >  xDic( SvxGetIgnoreAllList() );
    if (xDic.is())
    {
        OUString aTmp( C2U("v_7xj4") );
        sal_Bool bOk = xDic->add( aTmp, sal_False, OUString() );
        if (bOk)
            xDic->remove( aTmp );
    }

#ifdef NOT_YET_IMPLEMENTED
    //!!! functionality needs to be implemented via XPropertySet listeners!

    sal_Bool bSpellAllAgain   = sal_False;
    sal_Bool bSpellWrongAgain = sal_False;
    if( aNumsBtn.IsChecked() != aNumsBtn.GetSavedValue() )
    {
        if( aNumsBtn.IsChecked() )
            bSpellAllAgain = sal_True;
        else
            bSpellWrongAgain = sal_True;
    }
    if( aCapsBtn.IsChecked() != aCapsBtn.GetSavedValue() )
    {
        if( aCapsBtn.IsChecked() )
            bSpellAllAgain = sal_True;
        else
            bSpellWrongAgain = sal_True;
    }
    if( aUpLowBtn.IsChecked() != aUpLowBtn.GetSavedValue() )
    {
        if( aUpLowBtn.IsChecked() )
            bSpellAllAgain = sal_True;
        else
            bSpellWrongAgain = sal_True;
    }
    if( aAllLangBtn.IsChecked() != aAllLangBtn.GetSavedValue() )
    {
        if( aAllLangBtn.IsChecked() )
            bSpellWrongAgain = sal_True;
        else
            bSpellAllAgain = sal_True;
    }
    pSpell->SetSpellWrongAgain( bSpellWrongAgain );
    pSpell->SetSpellAllAgain( bSpellAllAgain );
#endif //NOT_YET_IMPLEMENTED

#ifdef NEVER
    Reference< XPropertySet >  xProp( SvxGetLinguPropertySet() );
    if (xProp.is())
    {
        // set spellchecking properties
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_WITH_DIGITS), lcl_Bool2Any(aNumsBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_UPPER_CASE), lcl_Bool2Any(aCapsBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_CAPITALIZATION), lcl_Bool2Any(aUpLowBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_IN_ALL_LANGUAGES), lcl_Bool2Any(aAllLangBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_SPELL_SPECIAL), lcl_Bool2Any(aSpellSpecialBtn.IsChecked()) );

        // set hyphenation properties
        xProp->setPropertyValue( C2U(UPN_IS_HYPH_AUTO), lcl_Bool2Any(aAutoBtn.IsChecked()) );
        xProp->setPropertyValue( C2U(UPN_IS_HYPH_SPECIAL), lcl_Bool2Any(aSpecialBtn.IsChecked()) );

        xProp->setPropertyValue( C2U(UPN_IS_GERMAN_PRE_REFORM), lcl_Bool2Any(aGermanPreReformBtn.IsChecked()) );
    }

    // erstmal immer putten!
    rCoreSet.Put( SfxBoolItem( SID_SPELL_MODIFIED, bModified ) );

    const String    &rPreStr    = aPreBreakEdit.GetText(),
                    &rAfterStr  = aAfterBreakEdit.GetText();

    const String    &rSavedPreStr   = aPreBreakEdit.GetSavedValue(),
                    &rSavedAfterStr = aAfterBreakEdit.GetSavedValue();

    if ( rPreStr != rSavedPreStr || rAfterStr != rSavedAfterStr )
    {
        SfxHyphenRegionItem aHyp( GetWhich( SID_ATTR_HYPHENREGION ) );
        aHyp.GetMinLead() =
            (sal_uInt8)aPreBreakEdit.Denormalize( aPreBreakEdit.GetValue() );
        aHyp.GetMinTrail() =
            (sal_uInt8)aAfterBreakEdit.Denormalize( aAfterBreakEdit.GetValue() );
        rCoreSet.Put( aHyp );
    }

    // language
    sal_uInt16 nPos = aLanguageLB.GetSelectEntryPos();
    sal_uInt16 nLang = (sal_uInt16)(sal_uInt32)aLanguageLB.GetEntryData( nPos );

    if ( nPos != aLanguageLB.GetSavedValue() &&
         nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        SfxUInt16Item aFmtLang( GetWhich( SID_ATTR_LANGUAGE ),
                                SvxGetSelectableLanguages().getConstArray()[ nLang ] );
        rCoreSet.Put( aFmtLang );
    }


    // Autom. Rechtschreibung
    const SfxPoolItem* pOld = GetOldItem( rCoreSet, SID_AUTOSPELL_CHECK );

    if ( !pOld || ( (SfxBoolItem*)pOld )->GetValue() !=
                    aAutoCheckBtn.IsChecked() )
    {
        rCoreSet.Put( SfxBoolItem( GetWhich( SID_AUTOSPELL_CHECK ),
                                   aAutoCheckBtn.IsChecked() ) );
        bModified |= sal_True;
    }
    pOld = GetItem( rCoreSet, SID_AUTOSPELL_MARKOFF );

    if ( aMarkOffBtn.IsEnabled() &&
         ( !pOld || ( (SfxBoolItem*)pOld )->GetValue() !=
                      aMarkOffBtn.IsChecked() ) )
    {
        rCoreSet.Put( SfxBoolItem( GetWhich( SID_AUTOSPELL_MARKOFF ),
                                   aMarkOffBtn.IsChecked() ) );
        bModified |= sal_True;
    }
#endif

    return bModified;
}

//------------------------------------------------------------------------

void SvxLinguTabPage::Reset( const SfxItemSet& rSet )
{
    // nur Initialisierung
    UpdateBox_Impl();

#ifdef NEVER
    // #44483#
    // form::component::CheckBox f"ur die Benutzerw"orterb"ucher
    // setzen
    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );

    const sal_uInt16 nCount = aDics.getLength();
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    const sal_uInt16 nSize  = (sal_uInt16)aDictsLB.GetEntryCount();
    //
    for ( sal_uInt16 i = 0; i < nSize; ++i )
    {
        for ( sal_uInt16 j = 0; j < nCount; ++j )
        {
            Reference< XDictionary1 >  xDic( pDic[j], UNO_QUERY );
            if (!xDic.is())
                continue;

            sal_Bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
            const String aName( ::GetDicInfoStr( xDic->getName(),
                                                 xDic->getLanguage(),
                                                 bNegativ ) );

            if ( xDic->isActive()  &&  aName == aDictsLB.GetText(i) )
                break;
        }
        aDictsLB.CheckEntryPos( i, j != nCount );
    }
    // #44483#

    Reference< XPropertySet >  xProp ( SvxGetLinguPropertySet() );
    if (xProp.is())
    {
        // Optionen aus dem Speller einstellen
        aCapsBtn        .Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_UPPER_CASE) ).getValue() );
        aNumsBtn        .Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_WITH_DIGITS) ).getValue() );
        aUpLowBtn       .Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_CAPITALIZATION) ).getValue() );
        aAllLangBtn     .Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_IN_ALL_LANGUAGES) ).getValue() );
        aSpellSpecialBtn.Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_SPELL_SPECIAL) ).getValue() );

        aAutoBtn.Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_HYPH_AUTO) ).getValue() );
        aSpecialBtn.Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_HYPH_SPECIAL) ).getValue() );

        aGermanPreReformBtn.Check( *(sal_Bool*)xProp->getPropertyValue( C2U(UPN_IS_GERMAN_PRE_REFORM) ).getValue() );
    }

    sal_uInt16 nWhich = GetWhich( SID_ATTR_HYPHENREGION );

    if ( rSet.GetItemState( nWhich, sal_False ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxHyphenRegionItem& rHyp =
            (const SfxHyphenRegionItem&)rSet.Get( nWhich );
        aPreBreakEdit.SetValue(
            aPreBreakEdit.Normalize( rHyp.GetMinLead() ) ); //TL???
        aAfterBreakEdit.SetValue(
            aAfterBreakEdit.Normalize( rHyp.GetMinTrail() ) );
    }

    // Sprache
    nWhich = GetWhich( SID_ATTR_LANGUAGE );
    aLanguageLB.SetNoSelection();

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_AVAILABLE )
    {
        const SfxUInt16Item& rItem = (SfxUInt16Item&)rSet.Get( nWhich );
        DBG_ASSERT( (LanguageType)rItem.GetValue() != LANGUAGE_SYSTEM,
                    "LANGUAGE_SYSTEM nicht erlaubt!" );

        if ( (LanguageType)rItem.GetValue() != LANGUAGE_DONTKNOW )
        {
            sal_uInt16 nLang = SvxGetLanguagePos( SvxGetSelectableLanguages(),
                                              rItem.GetValue() );

            for ( sal_uInt16 i = 0; i < aLanguageLB.GetEntryCount(); ++i )
            {
                if ( (sal_uInt16)(sal_uInt32)aLanguageLB.GetEntryData(i) == nLang )
                {
                    aLanguageLB.SelectEntryPos(i);
                    break;
                }
            }
        }
    }

    // Autom. Rechtschreibung
    const SfxPoolItem* pItem = GetItem( rSet, SID_AUTOSPELL_CHECK );

    if ( pItem )
        aAutoCheckBtn.Check( ( (SfxBoolItem*)pItem )->GetValue() );
    pItem = GetItem( rSet, SID_AUTOSPELL_MARKOFF );

    if ( pItem )
        aMarkOffBtn.Check( ( (SfxBoolItem*)pItem )->GetValue() );

    CheckHdl_Impl( 0 );

    aNumsBtn.SaveValue();
    aCapsBtn.SaveValue();
    aUpLowBtn.SaveValue();
    aAllLangBtn.SaveValue();
    aPreBreakEdit.SaveValue();
    aAfterBreakEdit.SaveValue();
    aLanguageLB.SaveValue();
#endif
}

// -----------------------------------------------------------------------

#ifdef NEVER
IMPL_LINK( SvxLinguTabPage, NewHdl_Impl, Button *, EMPTYARG )
{
    SvxNewDictionaryDialog* pDlg = new SvxNewDictionaryDialog( this, xSpell );

    Reference< XDictionary1 >  xNewDic;

    if ( pDlg->Execute() == RET_OK )
        xNewDic = pDlg->GetNewDictionary();
    delete pDlg;

    if ( xNewDic.is() )
    {
        // add new Dictionary to snapshot list
        sal_uInt16 nNewIdx = aDics.getLength();
        aDics.realloc( nNewIdx + 1 );
        aDics.getArray()[ nNewIdx ] = Reference< XDictionary > ( xNewDic, UNO_QUERY );
        const Reference< XDictionary >  *pDic = aDics.getConstArray();

        sal_Bool bNegativ = xNewDic->getDictionaryType() == DictionaryType_NEGATIVE;
        const String aName( ::GetDicInfoStr( xNewDic->getName(),
                                             xNewDic->getLanguage(),
                                             bNegativ ) );
        aDictsLB.InsertEntry( aName, nNewIdx );
        if ( aDictsLB.GetCheckedEntryCount() < SVX_MAX_USERDICTS )
            aDictsLB.CheckEntryPos( nNewIdx );
        aDictsLB.SelectEntryPos( nNewIdx );
        aDictsLB.MakeVisible( aDictsLB.GetEntry( nNewIdx ) );
    }
    return 1;
}
#endif
// -----------------------------------------------------------------------

#ifdef NEVER
IMPL_LINK( SvxLinguTabPage, EditHdl_Impl, Button *, EMPTYARG )
{
    String sName;
    sal_uInt16 nId = aDictsLB.GetSelectEntryPos();

    if ( nId != LISTBOX_ENTRY_NOTFOUND )
        sName = aDictsLB.GetText( nId );

    SvxEditDictionaryDialog* pDlg =
        new SvxEditDictionaryDialog( this, sName, xSpell );
    pDlg->Execute();

    UpdateBox_Impl();

    aDictsLB.MakeVisible(aDictsLB.GetEntry(pDlg->GetSelectedDict()));
    aDictsLB.SelectEntryPos(pDlg->GetSelectedDict());

    delete pDlg;
    return 1;
}
#endif

// -----------------------------------------------------------------------
#ifdef NEVER
void lcl_SequenceRemoveElementAt(
    Sequence<Reference<XDictionary > >& rEntries, int nPos )
{
    //TODO: helper for SequenceRemoveElementAt available?
    if(nPos >= rEntries.getLength())
        return;
    Sequence<Reference<XDictionary> > aTmp(rEntries.getLength() - 1);
    Reference<XDictionary >* pOrig = rEntries.getArray();
    Reference<XDictionary >* pTemp = aTmp.getArray();
    int nOffset = 0;
    for(int i = 0; i < aTmp.getLength(); i++)
    {
        if(nPos == i)
            nOffset++;
        pTemp[i] = pOrig[i + nOffset];
    }

    rEntries = aTmp;
}
#endif
// -----------------------------------------------------------------------

#ifdef NEVER
IMPL_LINK( SvxLinguTabPage, CheckHdl_Impl, CheckBox *, EMPTYARG )
{
    if ( !aAutoCheckBtn.IsChecked() )
        aMarkOffBtn.Disable();
    else
        aMarkOffBtn.Enable();
    return 1;
}
#endif

// -----------------------------------------------------------------------

#ifdef NEVER
IMPL_LINK( SvxLinguTabPage, CheckDicHdl_Impl, SvxCheckListBox *, EMPTYARG )
{
    sal_uInt16 nMax=0;
    sal_Bool bFlag=sal_False;

    if(aDictsLB.GetCheckedEntryCount()>SVX_MAX_USERDICTS)
    {
        InfoBox aInfoBox(this,aMaxWarning);
        aInfoBox.Execute();

        SvLBoxEntry* pEntry=aDictsLB.GetCurEntry();
        SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));

        if(pItem!=NULL)
        {
            pItem->SetStateHilighted(sal_False);

        }

        aDictsLB.EndSelection();
        aDictsLB.CheckEntryPos( aDictsLB.GetSelectEntryPos(),sal_False);
    }

    // let CheckBox for IgnoreAllList always be checked
    sal_uInt16 nSel = aDictsLB.GetSelectEntryPos();
    if (nSel != LISTBOX_ENTRY_NOTFOUND
        &&  aDictsLB.GetEntryData( nSel ) == (void *) 1)
    {
        aDictsLB.CheckEntryPos( nSel, sal_True );
    }

    return 0;
}
#endif

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
        SvxEditModulesDlg aDlg( this );
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
                OptionsBreakSet aDlg( this, aData.GetEntryId() == EID_NUM_PRE_BREAK ? sNumPreBreak : sNumPostBreak);
                aDlg.Execute();
                nVal = aDlg.GetNumericFld().GetValue();
                if (-1 != nVal)
                {
                    aData.SetNumericValue( (BYTE)nVal );
                    pEntry->SetUserData( (void *) aData.GetUserData() );
                    aLinguOptionsCLB.Invalidate();
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

#ifdef NEVER
IMPL_LINK( SvxLinguTabPage, BoxSelectHdl_Impl, ToolBox *, pBox )
{
    const sal_uInt16 nEntry = pBox->GetCurItemId();

    if ( TID_NEW == nEntry )
        NewHdl_Impl( 0 );
    else if ( TID_EDIT == nEntry )
        EditHdl_Impl( 0 );
    else if ( TID_DELETE == nEntry )
        DeleteHdl_Impl( 0 );
    return 1;
}
#endif

// ----------------------------------------------------------------------

SvLBoxEntry* SvxLinguTabPage::CreateEntry(String& rTxt, USHORT nCol)
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    if( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData;
        pCheckButtonData->aBmps[SV_BMP_UNCHECKED] = aChkunBmp;
        pCheckButtonData->aBmps[SV_BMP_CHECKED]   = aChkchBmp;
    }

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));    // Sonst Puff!

    String sEmpty;
    if (CBCOL_FIRST == nCol)
        pEntry->AddItem( new SvLBoxButton( pEntry, 0, pCheckButtonData ) );
    if (CBCOL_SECOND == nCol)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );    // Leerspalte
    pEntry->AddItem( new BrwString_Impl( pEntry, 0, rTxt ) );

    return pEntry;
}

// ----------------------------------------------------------------------

void SvxLinguTabPage::UpdateBox_Impl()
{
    const Sequence<OUString>&  rAllNames      = pLinguData->GetAvailSrvcNames();
    const OUString* pNames = rAllNames.getConstArray();
    const Sequence<OUString>&  rAllDispNames  = pLinguData->GetAvailSrvcDisplayNames();
    const OUString* pDispNames = rAllDispNames.getConstArray();
    const Sequence<sal_Bool>&  rAllConfigured = pLinguData->GetAvailSrvcConfigured();
    const sal_Bool* pConfigured = rAllConfigured.getConstArray();
    for(sal_uInt16 i = 0; i < rAllNames.getLength(); i++)
    {
        aLinguModulesCLB.InsertEntry(pDispNames[i]);
        SvLBoxEntry* pEntry = aLinguModulesCLB.GetEntry(i);
        pEntry->SetUserData(new SvxModulesData_Impl(pNames[i], pConfigured[i]));
        aLinguModulesCLB.CheckEntryPos( i, pConfigured[i] );
    }
    aLinguModulesEditPB.Enable(rAllNames.getLength() > 0);

    if (xProp.is())
    {
        aLinguOptionsCLB.SetUpdateMode(FALSE);
        aLinguOptionsCLB.Clear();

        SvLBoxTreeList *pModel = aLinguOptionsCLB.GetModel();
        SvLBoxEntry* pEntry = NULL;

        INT16 nVal = 0;
        BOOL  bVal  = FALSE;
        ULONG nUserData = 0;

        pEntry = CreateEntry( sCapitalWords,    CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_SPELL_UPPER_CASE) ) >>= bVal;
        nUserData = OptionsUserData( EID_CAPITAL_WORDS, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sWordsWithDigits, CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_SPELL_WITH_DIGITS) ) >>= bVal;
        nUserData = OptionsUserData( EID_WORDS_WITH_DIGITS, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sCapitalization,  CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_SPELL_CAPITALIZATION) ) >>= bVal;
        nUserData = OptionsUserData( EID_CAPITALIZATION, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sSpellSpecial,    CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_SPELL_SPECIAL) ) >>= bVal;
        nUserData = OptionsUserData( EID_SPELL_SPECIAL, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sAllLanguages,    CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_SPELL_IN_ALL_LANGUAGES) ) >>= bVal;
        nUserData = OptionsUserData( EID_ALL_LANGUAGES, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sSpellAuto,       CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_SPELL_AUTO) ) >>= bVal;
        nUserData = OptionsUserData( EID_SPELL_AUTO, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sHideMarkings,    CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_SPELL_HIDE) ) >>= bVal;
        nUserData = OptionsUserData( EID_HIDE_MARKINGS, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sOldGerman,       CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_GERMAN_PRE_REFORM) ) >>= bVal;
        nUserData = OptionsUserData( EID_OLD_GERMAN, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sNumPreBreak,     CBCOL_SECOND );
        xProp->getPropertyValue( C2U(UPN_HYPH_MIN_LEADING) ) >>= nVal;
        nUserData = OptionsUserData( EID_NUM_PRE_BREAK, TRUE, (USHORT)nVal, FALSE, FALSE).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );

        pEntry = CreateEntry( sNumPostBreak,    CBCOL_SECOND );
        xProp->getPropertyValue( C2U(UPN_HYPH_MIN_TRAILING) ) >>= nVal;
        nUserData = OptionsUserData( EID_NUM_POST_BREAK, TRUE, (USHORT)nVal, FALSE, FALSE).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );

        pEntry = CreateEntry( sHyphAuto,        CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_HYPH_AUTO) ) >>= bVal;
        nUserData = OptionsUserData( EID_HYPH_AUTO, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        pEntry = CreateEntry( sHyphSpecial,     CBCOL_FIRST );
        xProp->getPropertyValue( C2U(UPN_IS_HYPH_SPECIAL) ) >>= bVal;
        nUserData = OptionsUserData( EID_HYPH_SPECIAL, FALSE, 0, TRUE, bVal).GetUserData();
        pEntry->SetUserData( (void *)nUserData );
        pModel->Insert( pEntry );
        lcl_SetCheckButton( pEntry, bVal );

        aLinguOptionsCLB.SetUpdateMode(TRUE);
    }

#ifdef NEVER
    // gecheckte Eintr"age merken
    SvUShorts aArr;

    sal_uInt16 i;
    sal_uInt16 nOldEntryCount = (sal_uInt16)aDictsLB.GetEntryCount();
    for ( i = 0; i < nOldEntryCount; ++i )
        if ( aDictsLB.IsChecked(i) )
            aArr.Insert( i, aArr.Count() );

    const sal_Int32 nCount = aDics.getLength();
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    aDictsLB.Clear();
    for ( i = 0; i < nCount; ++i )
    {
        Reference< XDictionary1 >  xDic( pDic[i], UNO_QUERY );
        if (!xDic.is())
            continue;

        sal_Bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
        const String aName( ::GetDicInfoStr( xDic->getName(),
                                             xDic->getLanguage(),
                                             bNegativ ) );
        aDictsLB.InsertEntry( aName );

        // Neue Eintr"age evtl. checken
        if ( nOldEntryCount && i >= nOldEntryCount
            && xDic->isActive())
            aDictsLB.CheckEntryPos( (sal_uInt16)aDictsLB.GetEntryCount() - 1  );

        // mark entry for IgnoreAllList as not being allowed to uncheck
        if (xDic == SvxGetIgnoreAllList())
        {
            aDictsLB.SetEntryData( aDictsLB.GetEntryCount() - 1, (void *) 1 );
        }
    }

    // gemerkte Eintr"age wieder checken
    for ( i = 0; i < aArr.Count(); ++i )
        aDictsLB.CheckEntryPos( aArr[i] );

    if ( aDictsLB.GetEntryCount() )
    {
        aDictsTbx.EnableItem( TID_EDIT, sal_True );
        aDictsTbx.EnableItem( TID_DELETE, sal_True );
    }
    else
    {
        aDictsTbx.EnableItem( TID_EDIT, sal_False );
        aDictsTbx.EnableItem( TID_DELETE, sal_False );
    }

    aDictsLB.SetHighlightRange();
    aDictsLB.SelectEntryPos( 0 );   //! may implicitly change TID_DELETE
#endif
}

// -----------------------------------------------------------------------

void SvxLinguTabPage::HideGroups( sal_uInt16 nGrp )
{
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

    if ( 0 != ( GROUP_LANGUAGE & nGrp ) )
    {
        aLanguageText.Hide();
        aLanguageLB.Hide();
    }
#endif
}

// -----------------------------------------------------------------------

void SvxLinguTabPage::HideSpellCheckSpecial()
{
    //TL???
    //aSpellSpecialBtn.Hide();
}

/* -----------------------------27.01.00 12:14--------------------------------

 ---------------------------------------------------------------------------*/
void SvxLinguTabPage::EnableAutoSpelling()
{
#ifdef NEVER
    aAutoCheckBtn.Show();
    aMarkOffBtn.Show();
    aAutoSpellBox.Show();
#endif
}

/*-- 27.01.00 12:17:13---------------------------------------------------

  -----------------------------------------------------------------------*/

SvxExternalLinguTabPage::SvxExternalLinguTabPage(Window* pParent, const SfxItemSet& rSet) :
    SfxTabPage(pParent, ResId( RID_SVXPAGE_EXTERNAL_LINGU, DIALOG_MGR() ), rSet),
    aOptionsPB( this, ResId(PB_OPTIONS)),
    aLinguGB(   this, ResId(GB_LINGU    )),
    aLinguLB(   this, ResId(LB_LINGU    )),
    aStdSpellCB(this, ResId(CB_STDSPELL)),
    aStdThesCB( this, ResId(CB_STDTHES)),
    aStdHyphCB( this, ResId(CB_STDHYPH)),
    aSettingsGB(this, ResId(GB_SETTINGS))
{
    FreeResource();
    Reference< lang::XMultiServiceFactory >  xMgr = ::comphelper::getProcessServiceFactory();
    Reference< XInterface >  xInst = xMgr->createInstance(
            C2U("com.sun.star.linguistic2.OtherLingu") );
    xOtherLingu = Reference< XOtherLingu > (xInst, UNO_QUERY);
    if(xOtherLingu.is())
    {
        sal_Int16 nLinguCount = xOtherLingu->getCount();
        for(sal_uInt16 i = 0; i < nLinguCount; i++)
        {
            aLinguLB.InsertEntry(xOtherLingu->getIdentifier(i));
        }
        aLinguLB.SelectEntryPos(0);

        aLinguLB.SetSelectHdl(LINK(this, SvxExternalLinguTabPage, LBSelectHdl_Impl));
        aOptionsPB.SetClickHdl(LINK(this, SvxExternalLinguTabPage, OptDlgHdl_Impl));

        xInst = xMgr->createInstance(
                C2U("com.sun.star.linguistic2.LinguProperties") );
        xLinguProps = Reference< XPropertySet > (xInst, UNO_QUERY);
        if(xLinguProps.is())
        {
            Any aIndex = xLinguProps->getPropertyValue(C2U("OtherLinguIndex"));
            sal_Int16 nIndex;
            aIndex >>= nIndex;;
            if(nIndex >= 0 && nIndex < nLinguCount)
            {
                aLinguLB.SelectEntryPos(nIndex + 1);

                Any aStdSpell = xLinguProps->getPropertyValue(C2U("IsStandardSpellChecker"));
                aStdSpellCB.Check(lcl_Bool2Any(aStdSpell));

                Any aStdThes = xLinguProps->getPropertyValue(C2U("IsStandardThesaurus"));
                aStdThesCB.Check(lcl_Bool2Any(aStdThes));

                Any aStdHyph = xLinguProps->getPropertyValue(C2U("IsStandardHyphenator"));
                aStdHyphCB.Check(lcl_Bool2Any(aStdHyph));
            }

        }
        LBSelectHdl_Impl(&aLinguLB);
/*
    virtual sal_Int16           getCount() throw( RuntimeException );
    virtual ::rtl::OUString         getIdentifier(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );
    virtual sal_Bool            hasOptionDialog(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );
    virtual sal_Int32           startOptionDialog(sal_Int16 nIndex, sal_Int32 pParent) throw( IllegalArgumentException, RuntimeException );
    virtual Reference< XHyphenator >    getHyphenator(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );
    virtual Reference< XSpellChecker >      getSpellChecker(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );
    virtual Reference< XThesaurus >     getThesaurus(sal_Int16 nIndex) throw( IllegalArgumentException, RuntimeException );

*/
    }
    else
    {
        aOptionsPB  .Enable(sal_False);
        aLinguGB    .Enable(sal_False);
        aLinguLB    .Enable(sal_False);
        aStdSpellCB .Enable(sal_False);
        aStdThesCB  .Enable(sal_False);
        aStdHyphCB  .Enable(sal_False);
        aSettingsGB .Enable(sal_False);
    }
}
/*-- 27.01.00 12:17:13---------------------------------------------------

  -----------------------------------------------------------------------*/
SvxExternalLinguTabPage::~SvxExternalLinguTabPage()
{
}
/*-- 27.01.00 12:17:14---------------------------------------------------

  -----------------------------------------------------------------------*/
SfxTabPage* SvxExternalLinguTabPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxExternalLinguTabPage(pParent, rSet);
}
/*-- 27.01.00 12:17:14---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool    SvxExternalLinguTabPage::FillItemSet( SfxItemSet& rSet )
{
    sal_uInt16 nSelectPos = aLinguLB.GetSelectEntryPos();
    if( xLinguProps.is() &&
        nSelectPos != aLinguLB.GetSavedValue() ||
        (nSelectPos &&
            (   aStdSpellCB.IsChecked() != aStdSpellCB.GetSavedValue() ||
                aStdHyphCB.IsChecked() != aStdHyphCB.GetSavedValue() ||
                aStdThesCB.IsChecked() != aStdThesCB.GetSavedValue())
            )
        )
    {
        sal_Int16 nTmpIndex = nSelectPos ? nSelectPos - 1: -1;

        Any aOldIndex = xLinguProps->getPropertyValue(C2U("OtherLinguIndex"));
        sal_Int16 nOldIndex;
        aOldIndex >>= nOldIndex;
        if (nOldIndex != nTmpIndex)
        {
            // force new spelling and flushing of spell caches
            //! current implementation is a workaround until the correct
            //! interface is implemented.
            //TL:TODO: use XPropertyChangeListener mechanism to do this
            Reference< XDictionary1 >  xDic( SvxGetIgnoreAllList() );
            if (xDic.is())
            {
                OUString aTmp( C2U("v_7xj4") );
                sal_Bool bOk = xDic->add( aTmp, sal_False, OUString() );
                if (bOk)
                    xDic->remove( aTmp );
            }
        }

        if (xOtherLingu.is())
            xOtherLingu->selectLinguisticByIndex( nTmpIndex );

        Any aIndex;
        aIndex <<= nTmpIndex;
        xLinguProps->setPropertyValue(C2U("OtherLinguIndex"), aIndex);
        if(aStdSpellCB.IsEnabled())
        {
            xLinguProps->setPropertyValue(C2U("IsStandardSpellChecker"),
                    lcl_Bool2Any(aStdSpellCB.IsChecked()));
        }

        if(aStdThesCB.IsEnabled())
        {
            xLinguProps->setPropertyValue(C2U("IsStandardThesaurus"),
                                lcl_Bool2Any(aStdThesCB.IsChecked()));
        }

        if(aStdHyphCB.IsEnabled())
        {
            xLinguProps->setPropertyValue(C2U("IsStandardHyphenator"),
                lcl_Bool2Any(aStdHyphCB.IsChecked()));
        }

    }

    // The spellchecker may have changed or an external linguistics option
    // page may have changed relevant options or have added/deleted words from
    // their dictionaries.
    // Thus the only safe choice is to trigger spellchecking again.
    //TL:TODO: should only be called if the external linguistics
    //  option dialog was used or the item-state has changed.
    if ( SfxViewFrame::Current() && SfxViewFrame::Current()->GetDispatcher() )
        SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SPELLCHECKER_CHANGED, SFX_CALLMODE_ASYNCHRON );

    return sal_True;
}
/*-- 27.01.00 12:17:14---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SvxExternalLinguTabPage::Reset( const SfxItemSet& rSet )
{
    aStdHyphCB.SaveValue();
    aStdThesCB.SaveValue();
    aStdSpellCB.SaveValue();
    aLinguLB.SaveValue();
}
/* -----------------------------27.01.00 13:26--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxExternalLinguTabPage, LBSelectHdl_Impl, ListBox*, pLB)
{
    if(!xOtherLingu.is())
        return 0;

    sal_uInt16 nSel = pLB->GetSelectEntryPos();

    aOptionsPB.Enable(nSel > 0 && xOtherLingu->hasOptionDialog( nSel - 1));
    sal_uInt8 nUse = 0;
    if( nSel > 0 )
    {
        --nSel;
        aStdSpellCB.Enable( xOtherLingu->hasSpellChecker(nSel) );
        aStdThesCB.Enable( xOtherLingu->hasThesaurus(nSel) );
        aStdHyphCB.Enable( xOtherLingu->hasHyphenator(nSel) );
    }
    else
    {
        aStdSpellCB.Enable( sal_False );
        aStdThesCB.Enable( sal_False );
        aStdHyphCB.Enable( sal_False );
    }
    return 0;
}
/* -----------------------------27.01.00 13:26--------------------------------

 ---------------------------------------------------------------------------*/
IMPL_LINK(SvxExternalLinguTabPage, OptDlgHdl_Impl, PushButton*, pButton)
{
    sal_uInt16 nSel = aLinguLB.GetSelectEntryPos();
    if(nSel && xOtherLingu.is())
    {
        if(xOtherLingu->hasOptionDialog( nSel - 1 ))
        {
            xOtherLingu->selectLinguisticByIndex( nSel - 1 );
            xOtherLingu->startOptionDialog( 0 );
        }
    }
    return 0;
}



