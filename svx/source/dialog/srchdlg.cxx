/*************************************************************************
 *
 *  $RCSfile: srchdlg.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: tl $ $Date: 2001-02-19 11:17:18 $
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

// include ---------------------------------------------------------------

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_MOREBTN_HXX //autogen wg. MoreButton
#include <vcl/morebtn.hxx>
#endif
#ifndef _SV_SYSTEM_HXX //autogen wg. System
#include <vcl/system.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen wg. RET_OK
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXSLSTITM_HXX
#include <svtools/slstitm.hxx>
#endif
#ifndef _SFXITEMITER_HXX
#include <svtools/itemiter.hxx>
#endif
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _SVT_SEARCHOPT_HXX_
#include <svtools/searchopt.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_TRANSLITERATIONMODULES_HPP_
#include <com/sun/star/i18n/transliterationmodules.hpp>
#endif

#pragma hdrstop

#define _SVX_SRCHDLG_CXX
#include "srchdlg.hxx"

#include "dialogs.hrc"
#include "svxitems.hrc"
#include "srchdlg.hrc"

#define ITEMID_SEARCH       SID_ATTR_SEARCH
#define ITEMID_SETITEM      0

#include "srchitem.hxx"
#include "pageitem.hxx"
#include "srchctrl.hxx"
#include "srchxtra.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#ifndef _SVX_OPTJSEARCH_HXX_
#include <optjsearch.hxx>
#endif
#define ITEMID_BRUSH SID_ATTR_BRUSH
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <brshitem.hxx>
#endif
#ifndef _SVX_BACKGRND_HXX //autogen
#include <backgrnd.hxx>
#endif

using namespace com::sun::star::i18n;

// -----------------------------------------------------------------------

#define REMEMBER_SIZE       10

#define MODIFY_SEARCH       0x00000001
#define MODIFY_REPLACE      0x00000002
#define MODIFY_WORD         0x00000004
#define MODIFY_EXACT        0x00000008
#define MODIFY_BACKWARDS    0x00000010
#define MODIFY_SELECTION    0x00000020
#define MODIFY_REGEXP       0x00000040
#define MODIFY_LAYOUT       0x00000080
#define MODIFY_SIMILARITY   0x00000100
#define MODIFY_FORMULAS     0x00000200
#define MODIFY_VALUES       0x00000400
#define MODIFY_NOTES        0x00000800
#define MODIFY_ROWS         0x00001000
#define MODIFY_COLUMNS      0x00002000
#define MODIFY_ALLTABLES    0x00004000

SV_IMPL_VARARR(SrchAttrItemList, SearchAttrItem);

#define NotifyApp( nId )                                        \
    rBindings.GetDispatcher()->Execute( nId, SFX_CALLMODE_SLOT,           \
                              pSearchItem, 0L )

#define GetCheckBoxValue( rBox )                                \
    rBox.IsEnabled() ? rBox.IsChecked() : FALSE

struct SearchDlg_Impl
{
    BOOL        bMultiLineEdit  : 1,
                bSaveToModule   : 1,
                bFocusOnSearch  : 1;
    USHORT*     pRanges;
    Timer       aSelectionTimer;

    SearchDlg_Impl() :
        bMultiLineEdit( FALSE ), bSaveToModule( TRUE ), bFocusOnSearch( TRUE ), pRanges( NULL ) {}
    ~SearchDlg_Impl() { delete pRanges; }
};

// -----------------------------------------------------------------------

void ListToStrArr_Impl( USHORT nId, SvStringsDtor& rStrLst, ComboBox& rCBox )
{
    SfxStringListItem* pSrchItem =
        (SfxStringListItem*)SFX_APP()->GetItem( nId );
    List* pLst = pSrchItem ? pSrchItem->GetList() : 0;

    if ( pLst )
        for ( USHORT i = 0; i < pLst->Count(); ++i )
        {
            String* pTmp = new String( *(String*)( pLst->GetObject(i) ) );
            rStrLst.Insert( pTmp, rStrLst.Count() );
            rCBox.InsertEntry( *pTmp );
        }
}

// -----------------------------------------------------------------------

void StrArrToList_Impl( USHORT nId, const SvStringsDtor& rStrLst )
{
    DBG_ASSERT( rStrLst.Count(), "vorher abpruefen!!" );
    List aLst;

    for ( USHORT i = 0; i < rStrLst.Count(); ++i )
        aLst.Insert( rStrLst[ i ], LIST_APPEND );

    SFX_APP()->PutItem( SfxStringListItem( nId, &aLst ) );
}

// class SearchAttrItemList ----------------------------------------------

SearchAttrItemList::SearchAttrItemList( const SearchAttrItemList& rList ) :

    SrchAttrItemList( (BYTE)rList.Count() )

{
    SrchAttrItemList::Insert( &rList, 0 );
    SearchAttrItem* pData = (SearchAttrItem*)GetData();

    for ( USHORT i = Count(); i; --i, ++pData )
        if ( !IsInvalidItem( pData->pItem ) )
            pData->pItem = pData->pItem->Clone();
}

// -----------------------------------------------------------------------

SearchAttrItemList::~SearchAttrItemList()
{
    Clear();
}

// -----------------------------------------------------------------------

void SearchAttrItemList::Put( const SfxItemSet& rSet )
{
    if ( !rSet.Count() )
        return;

    SfxItemPool* pPool = rSet.GetPool();
    SfxItemIter aIter( rSet );
    SearchAttrItem aItem;
    const SfxPoolItem* pItem = aIter.GetCurItem();
    USHORT nWhich;

    while ( TRUE )
    {
        // nur testen, ob vorhanden ist ?
        if( IsInvalidItem( pItem ) )
        {
            nWhich = rSet.GetWhichByPos( aIter.GetCurPos() );
            aItem.pItem = (SfxPoolItem*)pItem;
        }
        else
        {
            nWhich = pItem->Which();
            aItem.pItem = pItem->Clone();
        }

        aItem.nSlot = pPool->GetSlotId( nWhich );
        Insert( aItem );

        if ( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
    }
}

// -----------------------------------------------------------------------

SfxItemSet& SearchAttrItemList::Get( SfxItemSet& rSet )
{
    SfxItemPool* pPool = rSet.GetPool();
    SearchAttrItem* pData = (SearchAttrItem*)GetData();

    for ( USHORT i = Count(); i; --i, ++pData )
        if ( IsInvalidItem( pData->pItem ) )
            rSet.InvalidateItem( pPool->GetWhich( pData->nSlot ) );
        else
            rSet.Put( *pData->pItem );
    return rSet;
}

// -----------------------------------------------------------------------

void SearchAttrItemList::Clear()
{
    SearchAttrItem* pData = (SearchAttrItem*)GetData();

    for ( USHORT i = Count(); i; --i, ++pData )
        if ( !IsInvalidItem( pData->pItem ) )
            delete pData->pItem;
    SrchAttrItemList::Remove( 0, Count() );
}

// -----------------------------------------------------------------------

// l"oscht die Pointer auf die Items
void SearchAttrItemList::Remove( USHORT nPos, USHORT nLen )
{
    if ( nPos + nLen > Count() )
        nLen = Count() - nPos;
    SearchAttrItem* pData = (SearchAttrItem*)GetData() + nPos;

    for ( USHORT n = nLen; n; --n, ++pData )
        if ( !IsInvalidItem( pData->pItem ) )
            delete pData->pItem;

    SrchAttrItemList::Remove( nPos, nLen );
}

// class SvxSearchDialog -------------------------------------------------

SvxSearchDialog::SvxSearchDialog( Window* pParent, SfxBindings& rBind ) :

    ModelessDialog( pParent, SVX_RES( RID_SVXDLG_SEARCH ) ),

    aSearchText     ( this, ResId( FT_SEARCH ) ),
    aSearchLB       ( this, ResId( ED_SEARCH ) ),
    aSearchTmplLB   ( this, ResId( LB_SEARCH ) ),
    aSearchAttrText ( this, ResId( FT_SEARCH_ATTR ) ),
    aSearchFormatsED( this, ResId( ED_SEARCH_FORMATS ) ),
    aSearchAttrBox  ( this, ResId( GB_SEARCH_ATTR ) ),

    aReplaceText    ( this, ResId( FT_REPLACE ) ),
    aReplaceLB      ( this, ResId( ED_REPLACE ) ),
    aReplaceTmplLB  ( this, ResId( LB_REPLACE ) ),
    aReplaceAttrText( this, ResId( FT_REPLACE_ATTR ) ),
    aReplaceFormatsED( this, ResId( ED_REPLACE_FORMATS ) ),
    aReplaceAttrBox ( this, ResId( GB_REPLACE_ATTR ) ),

    aSearchAllBtn   ( this, ResId( BTN_SEARCH_ALL ) ),
    aSearchBtn      ( this, ResId( BTN_SEARCH ) ),
    aReplaceAllBtn  ( this, ResId( BTN_REPLACE_ALL ) ),
    aReplaceBtn     ( this, ResId( BTN_REPLACE ) ),
    aAttributeBtn   ( this, ResId( BTN_ATTRIBUTE ) ),
    aCloseBtn       ( this, ResId( BTN_CLOSE ) ),
    aFormatBtn      ( this, ResId( BTN_FORMAT ) ),
    aHelpBtn        ( this, ResId( BTN_HELP ) ),
    aNoFormatBtn    ( this, ResId( BTN_NOFORMAT ) ),
    pMoreBtn        ( new MoreButton( this, ResId( BTN_MORE ) ) ),

    aWordBtn        ( this, ResId( BTN_CELLS ) ),
    aExactBtn       ( this, ResId( BTN_EXACT ) ),
    aBackwardsBtn   ( this, ResId( BTN_BACKWARDS ) ),
    aSelectionBtn   ( this, ResId( BTN_SELECTIONS ) ),
    aRegExpBtn      ( this, ResId( BTN_REGEXP ) ),
    aLayoutBtn      ( this, ResId( BTN_LAYOUTS ) ),
    aSimilarityBox  ( this, ResId( CB_SIMILARITY) ),
    aSimilarityBtn  ( this, ResId( PB_SIMILARITY) ),
    aJapMatchFullHalfWidthCB( this, ResId( CB_JAP_MATCH_FULL_HALF_WIDTH ) ),
    aJapSoundsLikeCB( this, ResId( CB_JAP_SOUNDS_LIKE ) ),
    aJapOptionsBtn  ( this, ResId( PB_JAP_OPTIONS ) ),
    aOptionsBox     ( this, ResId( GB_OPTIONS ) ),

    aFormulasBtn    ( this, ResId( BTN_FORMULAS ) ),
    aValuesBtn      ( this, ResId( BTN_VALUES ) ),
    aNotesBtn       ( this, ResId( BTN_NOTES ) ),
    aSearchBox      ( this, ResId( GB_SEARCH ) ),

    aRowsBtn        ( this, ResId( BTN_ROWS ) ),
    aColumnsBtn     ( this, ResId( BTN_COLUMNS ) ),
    aSearchDirBox   ( this, ResId( GB_SEARCHDIR ) ),

    aAllTablesCB    ( this, ResId( CB_ALLTABLES ) ),
    aCalcExtrasBox  ( this, ResId( GB_CALCEXTRAS ) ),

    aCalcStr        (       ResId( STR_WORDCALC ) ),

    rBindings       ( rBind ),
    bWriter         ( FALSE ),
    bSearch         ( TRUE ),
    bFormat         ( FALSE ),
    nOptions        ( USHRT_MAX ),
    bSet            ( FALSE ),
    bReadOnly       ( FALSE ),
    bConstruct      ( TRUE ),
    nModifyFlag     ( 0 ),

    pImpl           ( NULL ),
    pSearchList     ( NULL ),
    pReplaceList    ( NULL ),
    pSearchItem     ( NULL ),

    pSearchController       ( NULL ),
    pOptionsController      ( NULL ),
    pFamilyController       ( NULL ),
    pSearchSetController    ( NULL ),
    pReplaceSetController   ( NULL ),
    nTransliterationSettings( 0x00000000 )

{
    Wallpaper aBackground = GetBackground();
    aSearchFormatsED.SetBackground( aBackground );
    aReplaceFormatsED.SetBackground( aBackground );
    FixedInfo* pInfo = new FixedInfo( this );
    aSearchFormatsED.SetFont( pInfo->GetFont() );
    aReplaceFormatsED.SetFont( pInfo->GetFont() );
    delete pInfo;
    pImpl = new SearchDlg_Impl;
    pImpl->aSelectionTimer.SetTimeout( 500 );
    pImpl->aSelectionTimer.SetTimeoutHdl(
        LINK( this, SvxSearchDialog, TimeoutHdl_Impl ) );

    EnableControls_Impl( 0 );

#ifdef OS2
    aSearchFormatsED.Hide();
    aSearchAttrText.Show();
    aReplaceFormatsED.Hide();
    aReplaceAttrText.Show();
#endif
    // alten Text des aWordBtn's merken
    ( aCalcStr += sal_Unicode('#') ) += aWordBtn.GetText();

/*! (pb) there is no AppWindow anymore
    // Position anpassen
    Point aPos;
    const Size aScreen( Application::GetAppWindow()->GetDesktopRectPixel().GetSize() );
    const Size MySize( GetSizePixel() );
    aPos.X() =  ( aScreen.Width() - MySize.Width() ) / 2;
    aPos.Y() =  aScreen.Height() - MySize.Height() - 60;
    SetPosPixel( aPos );
*/

    aLayoutStr = SVX_RESSTR( RID_SVXSTR_SEARCH_STYLES );
    aStylesStr = aLayoutBtn.GetText();

    // gemerkte Such-Strings von der Applikation holen
    ListToStrArr_Impl( SID_SEARCHDLG_SEARCHSTRINGS,
                       aSearchStrings, aSearchLB    );
    ListToStrArr_Impl( SID_SEARCHDLG_REPLACESTRINGS,
                       aReplaceStrings, aReplaceLB  );

    FreeResource();
    InitControls_Impl();

    // Attribut-Sets nur einmal im Ctor() besorgen
    const SvxSetItem* pSrchSetItem =
        (const SvxSetItem*)NotifyApp( FID_SEARCH_SEARCHSET );

    if ( pSrchSetItem )
        InitAttrList_Impl( &pSrchSetItem->GetItemSet(), 0 );

    const SvxSetItem* pReplSetItem =
        (const SvxSetItem*)NotifyApp( FID_SEARCH_REPLACESET );

    if ( pReplSetItem )
        InitAttrList_Impl( 0, &pReplSetItem->GetItemSet() );

    // Controller erzeugen und gleich aktualisieren
    rBindings.EnterRegistrations();
    pSearchController =
        new SvxSearchController( SID_SEARCH_ITEM, rBindings, *this );
    pOptionsController =
        new SvxSearchController( SID_SEARCH_OPTIONS, rBindings, *this );
    rBindings.LeaveRegistrations();
    NotifyApp( FID_SEARCH_ON );
    pImpl->aSelectionTimer.Start();
}

// -----------------------------------------------------------------------

SvxSearchDialog::~SvxSearchDialog()
{
    Hide();

    rBindings.EnterRegistrations();
    delete pSearchController;
    delete pOptionsController;
    delete pFamilyController;
    delete pSearchSetController;
    delete pReplaceSetController;
    rBindings.LeaveRegistrations();

    delete pSearchItem;
    delete pImpl;
    delete pSearchList;
    delete pReplaceList;
    delete pMoreBtn;
}

// -----------------------------------------------------------------------

BOOL SvxSearchDialog::Close()
{
    // remember strings speichern
    if ( aSearchStrings.Count() )
        StrArrToList_Impl( SID_SEARCHDLG_SEARCHSTRINGS, aSearchStrings );

    if ( aReplaceStrings.Count() )
        StrArrToList_Impl( SID_SEARCHDLG_REPLACESTRINGS, aReplaceStrings );

    // save settings to configuration
    SvtSearchOptions aOpt;
    aOpt.SetWholeWordsOnly      ( aWordBtn                .IsChecked() );
    aOpt.SetBackwards           ( aBackwardsBtn           .IsChecked() );
    aOpt.SetUseRegularExpression( aRegExpBtn              .IsChecked() );
    aOpt.SetMatchCase           ( aExactBtn               .IsChecked() );
    aOpt.SetSearchForStyles     ( aLayoutBtn              .IsChecked() );
    aOpt.SetSimilaritySearch    ( aSimilarityBox          .IsChecked() );
    aOpt.SetMatchFullHalfWidth  ( aJapMatchFullHalfWidthCB.IsChecked() );
    aOpt.SetSoundsLikeEnabled   ( aJapSoundsLikeCB        .IsChecked() );

    NotifyApp( FID_SEARCH_OFF );
    NotifyApp( SID_SEARCH_DLG );
    return TRUE;
}

// -----------------------------------------------------------------------

INT32 SvxSearchDialog::GetTransliterationSettings() const
{
    return nTransliterationSettings;
}

// -----------------------------------------------------------------------

void SvxSearchDialog::ApplyTransliterationSettings_Impl( INT32 nSettings )
{
    nTransliterationSettings = nSettings;
    BOOL bVal = 0 != (nSettings & TransliterationModules_IGNORE_CASE);
    aExactBtn               .Check( bVal );
    bVal = 0 != (nSettings & TransliterationModules_IGNORE_WIDTH);
    aJapMatchFullHalfWidthCB.Check( bVal );
}

// -----------------------------------------------------------------------

void SvxSearchDialog::InitControls_Impl()
{
    // CaseSensitives AutoComplete
    aSearchLB.EnableAutocomplete( TRUE, TRUE );
    aSearchLB.Show();
    aReplaceLB.EnableAutocomplete( TRUE, TRUE );
    aReplaceLB.Show();

    aFormatBtn.Disable();
    aAttributeBtn.Disable();

    aSearchLB.SetModifyHdl( LINK( this, SvxSearchDialog, ModifyHdl_Impl ) );
    aReplaceLB.SetModifyHdl( LINK( this, SvxSearchDialog, ModifyHdl_Impl ) );

    Link aLink = LINK( this, SvxSearchDialog, FocusHdl_Impl );
    aSearchLB.SetGetFocusHdl( aLink );
    aSearchFormatsED.SetGetFocusHdl( aLink );
    aReplaceLB.SetGetFocusHdl( aLink );
    aReplaceFormatsED.SetGetFocusHdl( aLink );

    aLink = LINK( this, SvxSearchDialog, LoseFocusHdl_Impl );
    aSearchLB.SetLoseFocusHdl( aLink );
    aReplaceLB.SetLoseFocusHdl( aLink );
    aSearchTmplLB.SetLoseFocusHdl( aLink );
    aReplaceTmplLB.SetLoseFocusHdl( aLink );

    aLink = LINK( this, SvxSearchDialog, CommandHdl_Impl );
    aSearchBtn.SetClickHdl( aLink );
    aSearchAllBtn.SetClickHdl( aLink );
    aReplaceBtn.SetClickHdl( aLink );
    aReplaceAllBtn.SetClickHdl( aLink );
    aCloseBtn.SetClickHdl( aLink );
    aSimilarityBtn.SetClickHdl( aLink );
    aJapOptionsBtn.SetClickHdl( aLink );

    aLink = LINK( this, SvxSearchDialog, FlagHdl_Impl );
    aWordBtn.SetClickHdl( aLink );
    aSelectionBtn.SetClickHdl( aLink );
    aExactBtn.SetClickHdl( aLink );
    aRegExpBtn.SetClickHdl( aLink );
    aBackwardsBtn.SetClickHdl( aLink );
    aSimilarityBox.SetClickHdl( aLink );
    aJapSoundsLikeCB.SetClickHdl( aLink );
    aJapMatchFullHalfWidthCB.SetClickHdl( aLink );

    aLayoutBtn.SetClickHdl( LINK( this, SvxSearchDialog, TemplateHdl_Impl ) );
    aFormatBtn.SetClickHdl( LINK( this, SvxSearchDialog, FormatHdl_Impl ) );
    aNoFormatBtn.SetClickHdl(
        LINK( this, SvxSearchDialog, NoFormatHdl_Impl ) );
    aAttributeBtn.SetClickHdl(
        LINK( this, SvxSearchDialog, AttributeHdl_Impl ) );

    // apply settings from configuration
    SvtSearchOptions aOpt;
    aWordBtn                .Check( aOpt.IsWholeWordsOnly() );
    aBackwardsBtn           .Check( aOpt.IsBackwards() );
    aRegExpBtn              .Check( aOpt.IsUseRegularExpression() );
    aExactBtn               .Check( aOpt.IsMatchCase() );
    aLayoutBtn              .Check( aOpt.IsSearchForStyles() );
    aSimilarityBox          .Check( aOpt.IsSimilaritySearch() );
    aJapMatchFullHalfWidthCB.Check( aOpt.IsMatchFullHalfWidth() );
    aJapSoundsLikeCB        .Check( aOpt.IsSoundsLikeEnabled() );
    ApplyTransliterationSettings_Impl( aOpt.GetTransliterationSettings() );
    FlagHdl_Impl( &aJapSoundsLikeCB );
}

// -----------------------------------------------------------------------

void SvxSearchDialog::Init_Impl( int bSearchPattern )
{
    DBG_ASSERT( pSearchItem, "SearchItem == 0" );
    bWriter = ( pSearchItem->GetAppFlag() == SVX_SEARCHAPP_WRITER );

#ifndef OS2
#ifndef VCL
    pImpl->bMultiLineEdit = bWriter;
#else
    // bei VCL kein MultiLineEdit mehr fuer die Format-Texte
    pImpl->bMultiLineEdit = FALSE;
#endif

    if ( !pImpl->bMultiLineEdit )
    {
        aSearchFormatsED.Hide();
        aSearchAttrText.Show();
        aReplaceFormatsED.Hide();
        aReplaceAttrText.Show();
    }
    else
    {
        String aText = aSearchAttrText.GetText();
        aSearchAttrText.Hide();

        if ( aText.Len() )
            aSearchFormatsED.SetText( aText );
        aSearchFormatsED.Show();
        aText = aReplaceAttrText.GetText();
        aReplaceAttrText.Hide();

        if ( aText.Len() )
            aReplaceFormatsED.SetText( aText );
        aReplaceFormatsED.Show();
    }
#endif

    if ( ( nModifyFlag & MODIFY_WORD ) == 0 )
         aWordBtn.Check( pSearchItem->GetWordOnly() );
    if ( ( nModifyFlag & MODIFY_EXACT ) == 0 )
        aExactBtn.Check( pSearchItem->GetExact() );
    if ( ( nModifyFlag & MODIFY_BACKWARDS ) == 0 )
        aBackwardsBtn.Check( pSearchItem->GetBackward() );
    if ( ( nModifyFlag & MODIFY_SELECTION ) == 0 )
        aSelectionBtn.Check( pSearchItem->GetSelection() );
    if ( ( nModifyFlag & MODIFY_REGEXP ) == 0 )
        aRegExpBtn.Check( pSearchItem->GetRegExp() );
    if ( ( nModifyFlag & MODIFY_LAYOUT ) == 0 )
        aLayoutBtn.Check( pSearchItem->GetPattern() );
    FASTBOOL bDraw = FALSE;

    if ( pSearchItem->GetAppFlag() == SVX_SEARCHAPP_CALC )
    {
        Link aLink = LINK( this, SvxSearchDialog, FlagHdl_Impl );
        aFormulasBtn.SetClickHdl( aLink );
        aNotesBtn.SetClickHdl( aLink );
        aValuesBtn.SetClickHdl( aLink );
        aRowsBtn.SetClickHdl( aLink );
        aColumnsBtn.SetClickHdl( aLink );
        aAllTablesCB.SetClickHdl( aLink );

        switch ( pSearchItem->GetCellType() )
        {
            case SVX_SEARCHIN_FORMULA:
                if ( ( nModifyFlag & MODIFY_FORMULAS ) == 0 )
                    aFormulasBtn.Check();
                break;

            case SVX_SEARCHIN_VALUE:
                if ( ( nModifyFlag & MODIFY_VALUES ) == 0 )
                    aValuesBtn.Check();
                break;

            case SVX_SEARCHIN_NOTE:
                if ( ( nModifyFlag & MODIFY_NOTES ) == 0 )
                    aNotesBtn.Check();
                break;
        }
        aWordBtn.SetText( aCalcStr.GetToken( 0, '#' ) );

        if ( pSearchItem->GetRowDirection() &&
             ( nModifyFlag & MODIFY_ROWS ) == 0 )
            aRowsBtn.Check();
        else if ( !pSearchItem->GetRowDirection() &&
                  ( nModifyFlag & MODIFY_COLUMNS ) == 0 )
            aColumnsBtn.Check();

        if ( ( nModifyFlag & MODIFY_ALLTABLES ) == 0 )
            aAllTablesCB.Check( pSearchItem->IsAllTables() );

        // nur im Writer Suche nach Formatierung
        aFormatBtn.Hide();
        aNoFormatBtn.Hide();
        aAttributeBtn.Hide();

        // Extras beim StarCalc
        pMoreBtn->AddWindow( &aFormulasBtn );
        pMoreBtn->AddWindow( &aNotesBtn );
        pMoreBtn->AddWindow( &aValuesBtn );
        pMoreBtn->AddWindow( &aSearchBox );
        pMoreBtn->AddWindow( &aRowsBtn );
        pMoreBtn->AddWindow( &aColumnsBtn );
        pMoreBtn->AddWindow( &aSearchDirBox );
        pMoreBtn->AddWindow( &aAllTablesCB );
        pMoreBtn->AddWindow( &aCalcExtrasBox );
        pMoreBtn->Show();
        pMoreBtn->Enable();
    }
    else
    {
        aWordBtn.SetText( aCalcStr.GetToken( 1, '#' ) );

        if ( pSearchItem->GetAppFlag() == SVX_SEARCHAPP_DRAW )
        {
            aRegExpBtn.Hide();
            aLayoutBtn.Hide();

            // nur im Writer Suche nach Formatierung
            aFormatBtn.Hide();
            aNoFormatBtn.Hide();
            aAttributeBtn.Hide();
            bDraw = TRUE;
        }
        else
        {
            if ( !pSearchList )
            {
                // Attribut-Sets besorgen, wenn noch nicht geschehen
                const SvxSetItem* pSrchSetItem =
                    (const SvxSetItem*)NotifyApp( FID_SEARCH_SEARCHSET );

                if ( pSrchSetItem )
                    InitAttrList_Impl( &pSrchSetItem->GetItemSet(), 0 );

                const SvxSetItem* pReplSetItem =
                    (const SvxSetItem*)NotifyApp( FID_SEARCH_REPLACESET );

                if ( pReplSetItem )
                    InitAttrList_Impl( 0, &pReplSetItem->GetItemSet() );
            }
            aFormatBtn.Show();
            aNoFormatBtn.Show();
            aAttributeBtn.Show();
        }
        pMoreBtn->SetState( FALSE );
        pMoreBtn->Hide();
    }

    if ( !bDraw )
    {
        aRegExpBtn.Show();
        aLayoutBtn.Show();
    }

    // "Ahnlichkeitssuche?
    if ( ( nModifyFlag & MODIFY_SIMILARITY ) == 0 )
        aSimilarityBox.Check( pSearchItem->IsLevenshtein() );
    bSet = TRUE;
    pImpl->bSaveToModule = FALSE;
    FlagHdl_Impl( &aSimilarityBox );
    pImpl->bSaveToModule = TRUE;
    FASTBOOL bDisableSearch = FALSE;
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if ( pViewShell )
    {
        BOOL bText = !bSearchPattern;

        if ( pViewShell->HasSelection( bText ) )
            EnableControl_Impl( &aSelectionBtn );
        else
        {
            aSelectionBtn.Check( FALSE );
            aSelectionBtn.Disable();
        }
    }

    // Patternsuche und es wurden keine AttrSets "ubergeben
    if ( bSearchPattern )
    {
        SfxObjectShell* pShell = SfxObjectShell::Current();

        if ( pShell && pShell->GetStyleSheetPool() )
        {
            // Vorlagen beschaffen
            aSearchTmplLB .Clear();
            aReplaceTmplLB.Clear();
            SfxStyleSheetBasePool* pStylePool = pShell->GetStyleSheetPool();
            pStylePool->SetSearchMask( pSearchItem->GetFamily(),
                                       SFXSTYLEBIT_ALL );
            SfxStyleSheetBase* pBase = pStylePool->First();

            while ( pBase )
            {
                if ( pBase->IsUsed() )
                    aSearchTmplLB.InsertEntry( pBase->GetName() );
                aReplaceTmplLB.InsertEntry( pBase->GetName() );
                pBase = pStylePool->Next();
            }
            aSearchTmplLB.SelectEntry( pSearchItem->GetSearchString() );
            aReplaceTmplLB.SelectEntry( pSearchItem->GetReplaceString() );

        }
        aSearchTmplLB.Show();

        if ( bConstruct )
            // nur nach dem Erzeugen den Fokus grappen
            aSearchTmplLB.GrabFocus();
        aReplaceTmplLB.Show();
        aSearchLB.Hide();
        aReplaceLB.Hide();

        aWordBtn.Disable();
        aRegExpBtn.Disable();
        aExactBtn.Disable();

        bDisableSearch = !aSearchTmplLB.GetEntryCount();
    }
    else
    {
        FASTBOOL bSetSearch = ( ( nModifyFlag & MODIFY_SEARCH ) == 0 );
        FASTBOOL bSetReplace = ( ( nModifyFlag & MODIFY_REPLACE ) == 0 );

        if ( pSearchItem->GetSearchString().Len() && bSetSearch )
            aSearchLB.SetText( pSearchItem->GetSearchString() );
        else if ( aSearchStrings.Count() )
        {
            FASTBOOL bAttributes =
                ( ( pSearchList && pSearchList->Count() ) ||
                  ( pReplaceList && pReplaceList->Count() ) );

            if ( bSetSearch && !bAttributes )
                aSearchLB.SetText( *aSearchStrings[ 0 ] );

            String aReplaceTxt = pSearchItem->GetReplaceString();

            if ( aReplaceStrings.Count() )
                aReplaceTxt = *aReplaceStrings[ 0 ];

            if ( bSetReplace && !bAttributes )
                aReplaceLB.SetText( aReplaceTxt );
        }
        aSearchLB.Show();

        if ( bConstruct )
            // nur nach dem Erzeugen den Fokus grappen
            aSearchLB.GrabFocus();
        aReplaceLB.Show();
        aSearchTmplLB.Hide();
        aReplaceTmplLB.Hide();

        EnableControl_Impl( &aRegExpBtn );
        EnableControl_Impl( &aExactBtn );

        if ( aRegExpBtn.IsChecked() )
            aWordBtn.Disable();
        else
            EnableControl_Impl( &aWordBtn );

        String aSrchAttrTxt;

        if ( pImpl->bMultiLineEdit )
            aSrchAttrTxt = aSearchFormatsED.GetText();
        else
            aSrchAttrTxt = aSearchAttrText.GetText();

        bDisableSearch = !aSearchLB.GetText().Len() && !aSrchAttrTxt.Len();
    }
    FocusHdl_Impl( &aSearchLB );

    if ( bDisableSearch )
    {
        aSearchBtn.Disable();
        aSearchAllBtn.Disable();
        aReplaceBtn.Disable();
        aReplaceAllBtn.Disable();
    }
    else
    {
        EnableControl_Impl( &aSearchBtn );
        EnableControl_Impl( &aSearchAllBtn );
        EnableControl_Impl( &aReplaceBtn );
        EnableControl_Impl( &aReplaceAllBtn );
    }

    if ( ( !pImpl->bMultiLineEdit && aSearchAttrText.GetText().Len() ) ||
         ( pImpl->bMultiLineEdit && aSearchFormatsED.GetText().Len() ) )
        EnableControl_Impl( &aNoFormatBtn );
    else
        aNoFormatBtn.Disable();

    if ( !pSearchList )
    {
        aAttributeBtn.Disable();
        aFormatBtn.Disable();
    }

    if ( aLayoutBtn.IsChecked() )
    {
        pImpl->bSaveToModule = FALSE;
        TemplateHdl_Impl( &aLayoutBtn );
        pImpl->bSaveToModule = TRUE;
    }
}

// -----------------------------------------------------------------------

void SvxSearchDialog::InitAttrList_Impl( const SfxItemSet* pSSet,
                                         const SfxItemSet* pRSet )
{
    if ( !pSSet && !pRSet )
        return;

    if ( !pImpl->pRanges && pSSet )
    {
        USHORT nCnt = 0;
        const USHORT* pPtr = pSSet->GetRanges();
        const USHORT* pTmp = pPtr;

        while( *pPtr )
        {
            nCnt += ( *(pPtr+1) - *pPtr ) + 1;
            pPtr += 2;
        }
        nCnt = pPtr - pTmp + 1;
        pImpl->pRanges = new USHORT[nCnt];
        memcpy( pImpl->pRanges, pTmp, sizeof(USHORT) * nCnt );
    }

    // sorge daf"ur, das die Texte der Attribute richtig stehen
    String aDesc;

    if ( pSSet )
    {
        delete pSearchList;
        pSearchList = new SearchAttrItemList;

        if ( pSSet->Count() )
        {
            pSearchList->Put( *pSSet );

            if ( !pImpl->bMultiLineEdit )
                aSearchAttrText.SetText( BuildAttrText_Impl( aDesc, TRUE ) );
            else
                aSearchFormatsED.SetText( BuildAttrText_Impl( aDesc, TRUE ) );

            if ( aDesc.Len() )
                bFormat |= TRUE;
        }
    }

    if ( pRSet )
    {
        delete pReplaceList;
        pReplaceList = new SearchAttrItemList;

        if ( pRSet->Count() )
        {
            pReplaceList->Put( *pRSet );

            if ( !pImpl->bMultiLineEdit )
                aReplaceAttrText.SetText( BuildAttrText_Impl( aDesc, FALSE ) );
            else
                aReplaceFormatsED.SetText( BuildAttrText_Impl( aDesc, FALSE ) );

            if ( aDesc.Len() )
                bFormat |= TRUE;
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, FlagHdl_Impl, Button *, pButton )
{
     if ( pButton && !bSet )
        SetModifyFlag_Impl( pButton );
    else
        bSet = FALSE;

    if ( pButton == &aSimilarityBox )
    {
        BOOL bIsChecked = aSimilarityBox.IsChecked();

        if ( bIsChecked )
        {
            aSimilarityBtn.Enable();
            aRegExpBtn.Check( FALSE );
            aRegExpBtn.Disable();
            EnableControl_Impl( &aWordBtn );

            if ( aLayoutBtn.IsChecked() )
            {
                EnableControl_Impl( &aExactBtn );
                aLayoutBtn.Check( FALSE );
            }
            aRegExpBtn.Disable();
            aLayoutBtn.Disable();
            aFormatBtn.Disable();
            aNoFormatBtn.Disable();
            aAttributeBtn.Disable();
        }
        else
        {
            EnableControl_Impl( &aRegExpBtn );
            EnableControl_Impl( &aLayoutBtn );
            EnableControl_Impl( &aFormatBtn );
            EnableControl_Impl( &aAttributeBtn );
            aSimilarityBtn.Disable();
        }
        pSearchItem->SetLevenshtein( bIsChecked );
    }
    else
    {
        if ( aLayoutBtn.IsChecked() && !bFormat )
        {
            aWordBtn.Check( FALSE );
            aWordBtn.Disable();
            aRegExpBtn.Check( FALSE );
            aRegExpBtn.Disable();
            aExactBtn.Check( FALSE );
            aExactBtn.Disable();

            if ( aSearchTmplLB.GetEntryCount() )
            {
                EnableControl_Impl( &aSearchBtn );
                EnableControl_Impl( &aSearchAllBtn );
                EnableControl_Impl( &aReplaceBtn );
                EnableControl_Impl( &aReplaceAllBtn );
            }
        }
        else
        {
            EnableControl_Impl( &aRegExpBtn );
            EnableControl_Impl( &aExactBtn );

            if ( aRegExpBtn.IsChecked() )
            {
                aWordBtn.Check( FALSE );
                aWordBtn.Disable();
                aSimilarityBox.Disable();
                aSimilarityBtn.Disable();
            }
            else
            {
                EnableControl_Impl( &aWordBtn );
                EnableControl_Impl( &aSimilarityBox );
            }

            // Such-String vorhanden? dann Buttons enablen
            bSet = TRUE;
            ModifyHdl_Impl( &aSearchLB );
        }
    }

    if ( &aAllTablesCB == pButton )
    {
        if ( aAllTablesCB.IsChecked() )
            aSearchAllBtn.Disable();
        else
        {
            bSet = TRUE;
            ModifyHdl_Impl( &aSearchLB );
        }
    }

    if ( &aJapSoundsLikeCB == pButton )
    {
        BOOL bEnableJapOpt = aJapSoundsLikeCB.IsChecked();
        aExactBtn               .Enable( !bEnableJapOpt );
        aJapMatchFullHalfWidthCB.Enable( !bEnableJapOpt );
        aJapOptionsBtn          .Enable( bEnableJapOpt );
    }

    if ( pImpl->bSaveToModule )
        SaveToModule_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, CommandHdl_Impl, Button *, pBtn )
{
    FASTBOOL bInclusive = ( aLayoutBtn.GetText() == aLayoutStr );

    if ( ( pBtn == &aSearchBtn )    ||
         ( pBtn == &aSearchAllBtn ) ||
         ( pBtn == &aReplaceBtn )   ||
         ( pBtn == &aReplaceAllBtn ) )
    {
        if ( aLayoutBtn.IsChecked() && !bInclusive )
        {
            pSearchItem->SetSearchString ( aSearchTmplLB.GetSelectEntry() );
            pSearchItem->SetReplaceString( aReplaceTmplLB.GetSelectEntry() );
        }
        else
        {
            pSearchItem->SetSearchString ( aSearchLB.GetText() );
            pSearchItem->SetReplaceString( aReplaceLB.GetText() );

            if ( pBtn == &aReplaceBtn )
                Remember_Impl( aReplaceLB.GetText(), FALSE );
            else
            {
                Remember_Impl( aSearchLB.GetText(), TRUE );

                if ( pBtn == &aReplaceAllBtn )
                    Remember_Impl( aReplaceLB.GetText(), FALSE );
            }
        }

        pSearchItem->SetWordOnly( GetCheckBoxValue( aWordBtn ) );
        pSearchItem->SetPattern( GetCheckBoxValue( aLayoutBtn ) );
        pSearchItem->SetSelection( GetCheckBoxValue( aSelectionBtn ) );
        pSearchItem->SetExact( GetCheckBoxValue( aExactBtn ) );
        pSearchItem->SetRegExp( GetCheckBoxValue( aRegExpBtn ) );
        pSearchItem->SetBackward( GetCheckBoxValue( aBackwardsBtn ) );

        if ( !bWriter )
        {
            if ( aFormulasBtn.IsChecked() )
                pSearchItem->SetCellType( SVX_SEARCHIN_FORMULA );
            else if ( aValuesBtn.IsChecked() )
                pSearchItem->SetCellType( SVX_SEARCHIN_VALUE );
            else if ( aNotesBtn.IsChecked() )
                pSearchItem->SetCellType( SVX_SEARCHIN_NOTE );

            pSearchItem->SetRowDirection( aRowsBtn.IsChecked() );
            pSearchItem->SetAllTables( aAllTablesCB.IsChecked() );
        }

        if ( pBtn == &aSearchBtn )
            pSearchItem->SetCommand( SVX_SEARCHCMD_FIND );
        else if ( pBtn == &aSearchAllBtn )
            pSearchItem->SetCommand( SVX_SEARCHCMD_FIND_ALL );
        else if ( pBtn == &aReplaceBtn )
            pSearchItem->SetCommand( SVX_SEARCHCMD_REPLACE );
        else if ( pBtn == &aReplaceAllBtn )
            pSearchItem->SetCommand( SVX_SEARCHCMD_REPLACE_ALL );

        // wenn nach Vorlagen gesucht wird, dann Format-Listen l"oschen
        if ( !bFormat && pSearchItem->GetPattern() )
        {
            if ( pSearchList )
                pSearchList->Clear();

            if ( pReplaceList )
                pReplaceList->Clear();
        }
        nModifyFlag = 0;
        NotifyApp( FID_SEARCH_NOW );
    }
    else if ( pBtn == &aCloseBtn )
    {
        if ( !aLayoutBtn.IsChecked() || bInclusive )
        {
            String aStr( aSearchLB.GetText() );

            if ( aStr.Len() )
                Remember_Impl( aStr, TRUE );
            aStr = aReplaceLB.GetText();

            if ( aStr.Len() )
                Remember_Impl( aStr, FALSE );
        }
        SaveToModule_Impl();
        Close();
    }
    else if ( pBtn == &aSimilarityBtn )
    {
        SvxSearchSimilarityDialog* pDlg =
            new SvxSearchSimilarityDialog( this,
                                           pSearchItem->IsLEVRelaxed(),
                                           pSearchItem->GetLEVOther(),
                                           pSearchItem->GetLEVShorter(),
                                           pSearchItem->GetLEVLonger() );

        if ( pDlg->Execute() == RET_OK )
        {
            pSearchItem->SetLEVRelaxed( pDlg->IsRelaxed() );
            pSearchItem->SetLEVOther( pDlg->GetOther() );
            pSearchItem->SetLEVShorter( pDlg->GetShorter() );
            pSearchItem->SetLEVLonger( pDlg->GetLonger() );
            SaveToModule_Impl();
        }
        delete pDlg;
    }
    else if ( pBtn == &aJapOptionsBtn )
    {
        SfxItemSet aSet( SFX_APP()->GetPool() );
        SfxSingleTabDialog aDlg( SfxSingleTabDialog( this, aSet, RID_SVXPAGE_JSEARCH_OPTIONS ) );
        SvxJSearchOptionsPage *pPage = (SvxJSearchOptionsPage *)
                    SvxJSearchOptionsPage::Create( &aDlg, aSet );
        aDlg.SetTabPage( pPage );   //! implicitly calls pPage->Reset(...)!
        pPage->EnableSaveOptions( FALSE );
        pPage->SetTransliterationSettings( GetTransliterationSettings() );
        aDlg.Execute();
        ApplyTransliterationSettings_Impl( pPage->GetTransliterationSettings() );
    }

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, ModifyHdl_Impl, ComboBox *, pEd )
{
    if ( !bSet )
        SetModifyFlag_Impl( pEd );
    else
        bSet = FALSE;

    if ( pEd == &aSearchLB || pEd == &aReplaceLB )
    {
        xub_StrLen nLBTxtLen = aSearchLB.GetText().Len(), nTxtLen;

        if ( !pImpl->bMultiLineEdit )
           nTxtLen = aSearchAttrText.GetText().Len();
        else
           nTxtLen = aSearchFormatsED.GetText().Len();

        if ( nLBTxtLen || nTxtLen )
        {
            EnableControl_Impl( &aSearchBtn );
            EnableControl_Impl( &aSearchAllBtn );
            EnableControl_Impl( &aReplaceBtn );
            EnableControl_Impl( &aReplaceAllBtn );
        }
        else
        {
            aSearchBtn.Disable();
            aSearchAllBtn.Disable();
            aReplaceBtn.Disable();
            aReplaceAllBtn.Disable();
        }
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, TemplateHdl_Impl, Button *, EMPTYARG )
{
    if ( pImpl->bSaveToModule )
        SaveToModule_Impl();

    if ( bFormat )
        return 0;
    String sDesc;

    if ( aLayoutBtn.IsChecked() )
    {
        if ( !pFamilyController )
        {
            USHORT nId = 0;

            // Vorlagen-Controller enablen
            switch ( pSearchItem->GetFamily() )
            {
                case SFX_STYLE_FAMILY_CHAR:
                    nId = SID_STYLE_FAMILY1; break;

                case SFX_STYLE_FAMILY_PARA:
                    nId = SID_STYLE_FAMILY2; break;

                case SFX_STYLE_FAMILY_FRAME:
                    nId = SID_STYLE_FAMILY3; break;

                case SFX_STYLE_FAMILY_PAGE:
                    nId = SID_STYLE_FAMILY4; break;

                case SFX_STYLE_FAMILY_ALL:
                    break;

                default:
                    DBG_ERROR( "StyleSheetFamily wurde geaendert?" );
            }

            rBindings.EnterRegistrations();
            pFamilyController =
                new SvxSearchController( nId, rBindings, *this );
            rBindings.LeaveRegistrations();
            aSearchTmplLB.Clear();
            aReplaceTmplLB.Clear();

            aSearchTmplLB.Show();
            aReplaceTmplLB.Show();
            aSearchLB.Hide();
            aReplaceLB.Hide();

            if ( !pImpl->bMultiLineEdit )
            {
                aSearchAttrText.SetText( sDesc );
                aReplaceAttrText.SetText( sDesc );
            }
            else
            {
                aSearchFormatsED.SetText( sDesc );
                aReplaceFormatsED.SetText( sDesc );
            }
        }
        aFormatBtn.Disable();
        aNoFormatBtn.Disable();
        aAttributeBtn.Disable();
        aSimilarityBox.Disable();
        aSimilarityBtn.Disable();
    }
    else
    {
        // Vorlagen-Controller disablen
        rBindings.EnterRegistrations();
        DELETEZ( pFamilyController );
        rBindings.LeaveRegistrations();

        aSearchLB.Show();
        aReplaceLB.Show();
        aSearchTmplLB.Hide();
        aReplaceTmplLB.Hide();

        if ( !pImpl->bMultiLineEdit )
        {
            aSearchAttrText.SetText( BuildAttrText_Impl( sDesc, TRUE ) );
            aReplaceAttrText.SetText( BuildAttrText_Impl( sDesc, FALSE ) );
        }
        else
        {
            aSearchFormatsED.SetText( BuildAttrText_Impl( sDesc, TRUE ) );
            aReplaceFormatsED.SetText( BuildAttrText_Impl( sDesc, FALSE ) );
        }

        EnableControl_Impl( &aFormatBtn );
        EnableControl_Impl( &aAttributeBtn );
        EnableControl_Impl( &aSimilarityBox );

        FocusHdl_Impl( bSearch ? &aSearchLB : &aReplaceLB );
    }
    bSet = TRUE;
    pImpl->bSaveToModule = FALSE;
    FlagHdl_Impl( &aLayoutBtn );
    pImpl->bSaveToModule = TRUE;
    return 0;
}

// -----------------------------------------------------------------------

void SvxSearchDialog::Remember_Impl( const String &rStr,BOOL bSearch )
{
    if ( !rStr.Len() )
        return;

    SvStringsDtor* pArr = bSearch ? &aSearchStrings : &aReplaceStrings;
    ComboBox* pListBox = bSearch ? &aSearchLB : &aReplaceLB;

    // identische Strings ignorieren
    for ( USHORT i = 0; i < pArr->Count(); ++i )
    {
        if ( COMPARE_EQUAL == (*pArr)[i]->CompareTo( rStr ) )
            return;
    }

    // bei maximaler Belegung "altesten Eintrag l"oschen (ListBox und Array)
    String* pInsStr;

    if ( pArr->Count() >= REMEMBER_SIZE )
    {
        pInsStr = (*pArr)[REMEMBER_SIZE - 1];
        pListBox->RemoveEntry( USHORT(REMEMBER_SIZE - 1) );
        pArr->Remove( REMEMBER_SIZE - 1 );
        *pInsStr = rStr;
    }
    else
        pInsStr = new String( rStr );

    pArr->Insert( pInsStr, 0 );
    pListBox->InsertEntry( *pInsStr, 0 );
}

// -----------------------------------------------------------------------

void SvxSearchDialog::TemplatesChanged_Impl( SfxStyleSheetBasePool& rPool )
{
//  SetUpdateMode( FALSE );
    String aOldSrch( aSearchTmplLB .GetSelectEntry() );
    String aOldRepl( aReplaceTmplLB.GetSelectEntry() );
    aSearchTmplLB .Clear();
    aReplaceTmplLB.Clear();
    rPool.SetSearchMask( pSearchItem->GetFamily(), SFXSTYLEBIT_ALL );
    aSearchTmplLB.SetUpdateMode( FALSE );
    aReplaceTmplLB.SetUpdateMode( FALSE );
    SfxStyleSheetBase* pBase = rPool.First();

    while ( pBase )
    {
        if ( pBase->IsUsed() )
            aSearchTmplLB.InsertEntry( pBase->GetName() );
        aReplaceTmplLB.InsertEntry( pBase->GetName() );
        pBase = rPool.Next();
    }
    aSearchTmplLB.SetUpdateMode( TRUE );
    aReplaceTmplLB.SetUpdateMode( TRUE );
    aSearchTmplLB.SelectEntryPos(0);

    if ( aOldSrch.Len() )
        aSearchTmplLB .SelectEntry( aOldSrch );
    aReplaceTmplLB.SelectEntryPos(0);

    if ( aOldRepl.Len() )
        aReplaceTmplLB.SelectEntry( aOldRepl );

    if ( aSearchTmplLB.GetEntryCount() )
    {
        EnableControl_Impl( &aSearchBtn );
        EnableControl_Impl( &aSearchAllBtn );
        EnableControl_Impl( &aReplaceBtn );
        EnableControl_Impl( &aReplaceAllBtn );
    }
//  FlagHdl_Impl(0);
//  SetUpdateMode( TRUE );
}

// -----------------------------------------------------------------------

void SvxSearchDialog::EnableControls_Impl( const USHORT nFlags )
{
    if ( nFlags == nOptions )
        return;
    else
        nOptions = nFlags;

    if ( !nOptions )
    {
        if ( IsVisible() )
        {
            Hide();
            return;
        }
    }
    else if ( !IsVisible() )
        Show();
    FASTBOOL bNoSearch = TRUE;

    if ( ( SEARCH_OPTIONS_SEARCH & nOptions ) != 0 )
    {
        aSearchBtn.Enable();
        bNoSearch = FALSE;
    }
    else
        aSearchBtn.Disable();
    if ( ( SEARCH_OPTIONS_SEARCH_ALL & nOptions ) != 0 )
    {
        aSearchAllBtn.Enable();
        bNoSearch = FALSE;
    }
    else
        aSearchAllBtn.Disable();
    if ( ( SEARCH_OPTIONS_REPLACE & nOptions ) != 0 )
    {
        aReplaceBtn.Enable();
        aReplaceText.Enable();
        aReplaceLB.Enable();
        aReplaceTmplLB.Enable();
        bNoSearch = FALSE;
    }
    else
    {
        aReplaceBtn.Disable();
        aReplaceText.Disable();
        aReplaceLB.Disable();
        aReplaceTmplLB.Disable();
    }
    if ( ( SEARCH_OPTIONS_REPLACE_ALL & nOptions ) != 0 )
    {
        aReplaceAllBtn.Enable();
        bNoSearch = FALSE;
    }
    else
        aReplaceAllBtn.Disable();
    aSearchBtn.Enable( !bNoSearch );
    aSearchText.Enable( !bNoSearch );
    aSearchLB.Enable( !bNoSearch );

    if ( ( SEARCH_OPTIONS_WHOLE_WORDS & nOptions ) != 0 )
        aWordBtn.Enable();
    else
        aWordBtn.Disable();
    if ( ( SEARCH_OPTIONS_BACKWARDS & nOptions ) != 0 )
        aBackwardsBtn.Enable();
    else
        aBackwardsBtn.Disable();
    if ( ( SEARCH_OPTIONS_REG_EXP & nOptions ) != 0 )
        aRegExpBtn.Enable();
    else
        aRegExpBtn.Disable();
    if ( ( SEARCH_OPTIONS_EXACT & nOptions ) != 0 )
        aExactBtn.Enable();
    else
        aExactBtn.Disable();
    if ( ( SEARCH_OPTIONS_SELECTION & nOptions ) != 0 )
        aSelectionBtn.Enable();
    else
        aSelectionBtn.Disable();
    if ( ( SEARCH_OPTIONS_FAMILIES & nOptions ) != 0 )
        aLayoutBtn.Enable();
    else
        aLayoutBtn.Disable();
    if ( ( SEARCH_OPTIONS_FORMAT & nOptions ) != 0 )
    {
        aAttributeBtn.Enable();
        aFormatBtn.Enable();
        aNoFormatBtn.Enable();
    }
    else
    {
        aAttributeBtn.Disable();
        aFormatBtn.Disable();
        aNoFormatBtn.Disable();
    }
    if ( ( SEARCH_OPTIONS_MORE & nOptions ) != 0 &&
         pSearchItem && pSearchItem->GetAppFlag() == SVX_SEARCHAPP_CALC )
        pMoreBtn->Enable();
    else
    {
        pMoreBtn->SetState( FALSE );
        pMoreBtn->Disable();
    }
    if ( ( SEARCH_OPTIONS_SIMILARITY & nOptions ) != 0 )
    {
        aSimilarityBox.Enable();
        aSimilarityBtn.Enable();
    }
    else
    {
        aSimilarityBox.Disable();
        aSimilarityBtn.Disable();
    }

    if ( pSearchItem )
        Init_Impl( pSearchItem->GetPattern() &&
                   ( !pSearchList || !pSearchList->Count() ) );
}

// -----------------------------------------------------------------------

void SvxSearchDialog::EnableControl_Impl( Control* pCtrl )
{
    if ( &aSearchBtn == pCtrl && ( SEARCH_OPTIONS_SEARCH & nOptions ) != 0 )
    {
        aSearchBtn.Enable();
        return;
    }
    if ( &aSearchAllBtn == pCtrl &&
         ( SEARCH_OPTIONS_SEARCH_ALL & nOptions ) != 0 )
    {
        aSearchAllBtn.Enable( ( bWriter || !aAllTablesCB.IsChecked() ) );
        return;
    }
    if ( &aReplaceBtn == pCtrl && ( SEARCH_OPTIONS_REPLACE & nOptions ) != 0 )
    {
        aReplaceBtn.Enable();
        return;
    }
    if ( &aReplaceAllBtn == pCtrl &&
         ( SEARCH_OPTIONS_REPLACE_ALL & nOptions ) != 0 )
    {
        aReplaceAllBtn.Enable();
        return;
    }
    if ( &aWordBtn == pCtrl && ( SEARCH_OPTIONS_WHOLE_WORDS & nOptions ) != 0 )
    {
        aWordBtn.Enable();
        return;
    }
    if ( &aBackwardsBtn == pCtrl && ( SEARCH_OPTIONS_BACKWARDS & nOptions ) != 0 )
    {
        aBackwardsBtn.Enable();
        return;
    }
    if ( &aRegExpBtn == pCtrl && ( SEARCH_OPTIONS_REG_EXP & nOptions ) != 0
        && !aSimilarityBox.IsChecked())
    {
        aRegExpBtn.Enable();
        return;
    }
    if ( &aExactBtn == pCtrl && ( SEARCH_OPTIONS_EXACT & nOptions ) != 0 )
    {
        aExactBtn.Enable();
        return;
    }
    if ( &aSelectionBtn == pCtrl && ( SEARCH_OPTIONS_SELECTION & nOptions ) != 0 )
    {
        aSelectionBtn.Enable();
        return;
    }
    if ( &aLayoutBtn == pCtrl && ( SEARCH_OPTIONS_FAMILIES & nOptions ) != 0 )
    {
        aLayoutBtn.Enable();
        return;
    }
    if (    &aAttributeBtn == pCtrl
         && ( SEARCH_OPTIONS_FORMAT & nOptions ) != 0
         && pSearchList )
    {
        aAttributeBtn.Enable( pImpl->bFocusOnSearch );
    }
    if ( &aFormatBtn == pCtrl && ( SEARCH_OPTIONS_FORMAT & nOptions ) != 0 )
    {
        aFormatBtn.Enable();
        return;
    }
    if ( &aNoFormatBtn == pCtrl && ( SEARCH_OPTIONS_FORMAT & nOptions ) != 0 )
    {
        aNoFormatBtn.Enable();
        return;
    }
    if ( &aSimilarityBox == pCtrl &&
         ( SEARCH_OPTIONS_SIMILARITY & nOptions ) != 0 )
    {
        aSimilarityBox.Enable();

        if ( aSimilarityBox.IsChecked() )
            aSimilarityBtn.Enable();
    }
}

// -----------------------------------------------------------------------

void SvxSearchDialog::SetItem_Impl( const SvxSearchItem* pItem )
{
    if ( pItem )
    {
        delete pSearchItem;
        pSearchItem = (SvxSearchItem*)pItem->Clone();
        Init_Impl( pSearchItem->GetPattern() &&
                   ( !pSearchList || !pSearchList->Count() ) );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, FocusHdl_Impl, Control *, pCtrl )
{
    xub_StrLen nTxtLen;

    if ( !pImpl->bMultiLineEdit )
        nTxtLen = aSearchAttrText.GetText().Len();
    else
        nTxtLen = aSearchFormatsED.GetText().Len();

    if ( pCtrl == &aSearchLB || pCtrl == &aSearchFormatsED )
    {
        if ( pCtrl->HasChildPathFocus() )
            pImpl->bFocusOnSearch = TRUE;
        pCtrl = &aSearchLB;
        bSearch = TRUE;

        if( nTxtLen )
            EnableControl_Impl( &aNoFormatBtn );
        else
            aNoFormatBtn.Disable();
        EnableControl_Impl( &aAttributeBtn );
    }
    else
    {
        pImpl->bFocusOnSearch = FALSE;
        pCtrl = &aReplaceLB;
        bSearch = FALSE;

        if ( ( !pImpl->bMultiLineEdit && aReplaceAttrText.GetText().Len() ) ||
             ( pImpl->bMultiLineEdit && aReplaceFormatsED.GetText().Len() ) )
            EnableControl_Impl( &aNoFormatBtn );
        else
            aNoFormatBtn.Disable();
        aAttributeBtn.Disable();
    }
    bSet = TRUE;
    ModifyHdl_Impl( (ComboBox*)pCtrl );
    aLayoutBtn.SetText( bFormat && nTxtLen ? aLayoutStr : aStylesStr );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, LoseFocusHdl_Impl, Control *, EMPTYARG )
{
    SaveToModule_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, FormatHdl_Impl, Button *, EMPTYARG )
{
    SfxObjectShell* pSh = SfxObjectShell::Current();

    DBG_ASSERT( pSh, "no DocShell" );

    if ( !pSh || !pImpl->pRanges )
        return 0;

    USHORT nCnt = 0;
    const USHORT* pPtr = pImpl->pRanges;
    const USHORT* pTmp = pPtr;

    while( *pTmp )
        pTmp++;
    nCnt = pTmp - pPtr + 7;
    USHORT* pWhRanges = new USHORT[nCnt];
    USHORT nPos = 0;

    while( *pPtr )
    {
        pWhRanges[nPos++] = *pPtr++;
    }

    pWhRanges[nPos++] = SID_ATTR_PARA_MODEL;
    pWhRanges[nPos++] = SID_ATTR_PARA_MODEL;

    USHORT nBrushWhich = pSh->GetPool().GetWhich(SID_ATTR_BRUSH);
    pWhRanges[nPos++] = nBrushWhich;
    pWhRanges[nPos++] = nBrushWhich;
    pWhRanges[nPos++] = SID_PARA_BACKGRND_DESTINATION;
    pWhRanges[nPos++] = SID_PARA_BACKGRND_DESTINATION;
    pWhRanges[nPos] = 0;
    SfxItemPool& rPool = pSh->GetPool();
    SfxItemSet aSet( rPool, pWhRanges );
    String aTxt;

    aSet.InvalidateAllItems();
    aSet.Put(SvxBrushItem(nBrushWhich));
    aSet.Put(SfxUInt16Item(SID_PARA_BACKGRND_DESTINATION, PARA_DEST_CHAR));

    if ( bSearch )
    {
        aTxt = SVX_RESSTR( RID_SVXSTR_SEARCH );
        pSearchList->Get( aSet );
    }
    else
    {
        aTxt = SVX_RESSTR( RID_SVXSTR_REPLACE );
        pReplaceList->Get( aSet );
    }
    aSet.DisableItem(SID_ATTR_PARA_MODEL);
    aSet.DisableItem(rPool.GetWhich(SID_ATTR_PARA_PAGEBREAK));
    aSet.DisableItem(rPool.GetWhich(SID_ATTR_PARA_KEEP));

    SvxSearchFormatDialog* pDlg = new SvxSearchFormatDialog( this, aSet );

    aTxt.Insert( pDlg->GetText(), 0 );
    pDlg->SetText( aTxt );

    if ( pDlg->Execute() == RET_OK )
    {
        DBG_ASSERT( pDlg->GetOutputItemSet(), "invalid Output-Set" );
        SfxItemSet aOutSet( *pDlg->GetOutputItemSet() );

        SearchAttrItemList* pList = bSearch ? pSearchList : pReplaceList;

        SearchAttrItem* pAItem;
        const SfxPoolItem* pItem;
        for( USHORT n = 0; n < pList->Count(); ++n )
            if( !IsInvalidItem( (pAItem = &pList->GetObject(n))->pItem ) &&
                SFX_ITEM_SET == aOutSet.GetItemState(
                    pAItem->pItem->Which(), FALSE, &pItem ) )
            {
                delete pAItem->pItem;
                pAItem->pItem = pItem->Clone();
                aOutSet.ClearItem( pAItem->pItem->Which() );
            }

        if( aOutSet.Count() )
            pList->Put( aOutSet );

        PaintAttrText_Impl(); // AttributText in GroupBox setzen
    }
    delete pDlg;
    delete pWhRanges;
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, NoFormatHdl_Impl, Button *, EMPTYARG )
{
    aLayoutBtn.SetText( aStylesStr );
    bFormat = FALSE;
    aLayoutBtn.Check( FALSE );

    if ( bSearch )
    {
        if ( !pImpl->bMultiLineEdit )
            aSearchAttrText.SetText( String() );
        else
            aSearchFormatsED.SetText( String() );
        pSearchList->Clear();
    }
    else
    {
        if ( !pImpl->bMultiLineEdit )
            aReplaceAttrText.SetText( String() );
        else
            aReplaceFormatsED.SetText( String() );
        pReplaceList->Clear();
    }
    pImpl->bSaveToModule = FALSE;
    TemplateHdl_Impl( &aLayoutBtn );
    pImpl->bSaveToModule = TRUE;
    aNoFormatBtn.Disable();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, AttributeHdl_Impl, Button *, EMPTYARG )
{
    if ( !pSearchList || !pImpl->pRanges )
        return 0;

    SvxSearchAttributeDialog* pDlg = new SvxSearchAttributeDialog( this, *pSearchList, pImpl->pRanges );
    pDlg->Execute();
    delete pDlg;
    PaintAttrText_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchDialog, TimeoutHdl_Impl, Timer *, pTimer )
{
    SfxViewShell* pViewShell = SfxViewShell::Current();

    if ( pViewShell )
    {
        if ( pViewShell->HasSelection( aSearchLB.IsVisible() ) )
            EnableControl_Impl( &aSelectionBtn );
        else
        {
            aSelectionBtn.Check( FALSE );
            aSelectionBtn.Disable();
        }
    }

    pTimer->Start();
    return 0;
}

// -----------------------------------------------------------------------

void SvxSearchDialog::GetSearchItems( SfxItemSet& rSet )
{
    xub_StrLen nLen;

    if ( !pImpl->bMultiLineEdit )
        nLen = aSearchAttrText.GetText().Len();
    else
        nLen = aSearchFormatsED.GetText().Len();

    if ( nLen && pSearchList )
        pSearchList->Get( rSet );
}

// -----------------------------------------------------------------------

void SvxSearchDialog::GetReplaceItems( SfxItemSet& rSet )
{
    xub_StrLen nLen;

    if ( !pImpl->bMultiLineEdit )
        nLen = aReplaceAttrText.GetText().Len();
    else
        nLen = aReplaceFormatsED.GetText().Len();

    if ( nLen && pReplaceList )
        pReplaceList->Get( rSet );
}

// -----------------------------------------------------------------------

String& SvxSearchDialog::BuildAttrText_Impl( String& rStr,
                                             BOOL bSrchFlag ) const
{
    if ( rStr.Len() )
        rStr.Erase();

    SfxObjectShell* pSh = SfxObjectShell::Current();
    DBG_ASSERT( pSh, "no DocShell" );

    if ( !pSh )
        return rStr;

    SfxItemPool& rPool = pSh->GetPool();
    SearchAttrItemList* pList = bSrchFlag ? pSearchList : pReplaceList;

    if ( !pList )
        return rStr;

    // Metrik abfragen
    SfxMapUnit eMapUnit = SFX_MAPUNIT_CM;
    FieldUnit eFieldUnit = GetModuleFieldUnit();

    switch ( eFieldUnit )
    {
        case FUNIT_MM:          eMapUnit = SFX_MAPUNIT_MM; break;
        case FUNIT_CM:
        case FUNIT_M:
        case FUNIT_KM:          eMapUnit = SFX_MAPUNIT_CM; break;
        case FUNIT_TWIP:        eMapUnit = SFX_MAPUNIT_TWIP; break;
        case FUNIT_POINT:
        case FUNIT_PICA:        eMapUnit = SFX_MAPUNIT_POINT; break;
        case FUNIT_INCH:
        case FUNIT_FOOT:
        case FUNIT_MILE:        eMapUnit = SFX_MAPUNIT_INCH; break;
        case FUNIT_100TH_MM:    eMapUnit = SFX_MAPUNIT_100TH_MM; break;
    }

    for ( USHORT i = 0; i < pList->Count(); ++i )
    {
        const SearchAttrItem& rItem = pList->GetObject(i);

        if ( rStr.Len() )
            rStr.AppendAscii( RTL_CONSTASCII_STRINGPARAM( ", " ) );

        if ( !IsInvalidItem( rItem.pItem ) )
        {
            String aStr;
            rPool.GetPresentation( *rItem.pItem,
                                    SFX_ITEM_PRESENTATION_COMPLETE,
                                    eMapUnit, aStr );
            rStr += aStr;
        }
        else
        {
            //Sonderbehandlung fuer Zeichenhintergrund
            USHORT nId = rItem.nSlot == SID_ATTR_BRUSH_CHAR ?
                                RID_SVXITEMS_BRUSH_CHAR :
                                    rItem.nSlot - SID_SVX_START + RID_ATTR_BEGIN;
            rStr += SVX_RESSTR( nId );
        }
    }
    return rStr;
}

// -----------------------------------------------------------------------

void SvxSearchDialog::PaintAttrText_Impl()
{
    String aDesc;
    BuildAttrText_Impl( aDesc, bSearch );

    if ( !bFormat && aDesc.Len() )
        bFormat = TRUE;

    if ( bSearch )
    {
        if ( !pImpl->bMultiLineEdit )
            aSearchAttrText.SetText( aDesc );
        else
            aSearchFormatsED.SetText( aDesc );
        FocusHdl_Impl( &aSearchLB );
    }
    else
    {
        if ( !pImpl->bMultiLineEdit )
            aReplaceAttrText.SetText( aDesc );
        else
            aReplaceFormatsED.SetText( aDesc );
        FocusHdl_Impl( &aReplaceLB );
    }
}

// -----------------------------------------------------------------------

void SvxSearchDialog::SetModifyFlag_Impl( const Control* pCtrl )
{
    if ( &aSearchLB == (ComboBox*)pCtrl )
        nModifyFlag |= MODIFY_SEARCH;
    else if ( &aReplaceLB == (ComboBox*)pCtrl )
        nModifyFlag |= MODIFY_REPLACE;
    else if ( &aWordBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_WORD;
    else if ( &aExactBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_EXACT;
    else if ( &aBackwardsBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_BACKWARDS;
    else if ( &aSelectionBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_SELECTION;
    else if ( &aRegExpBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_REGEXP;
    else if ( &aLayoutBtn == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_LAYOUT;
    else if ( &aSimilarityBox == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_SIMILARITY;
    else if ( &aFormulasBtn == (RadioButton*)pCtrl )
        nModifyFlag |= MODIFY_FORMULAS;
    else if ( &aValuesBtn == (RadioButton*)pCtrl )
        nModifyFlag |= MODIFY_VALUES;
    else if ( &aNotesBtn == (RadioButton*)pCtrl )
        nModifyFlag |= MODIFY_NOTES;
    else if ( &aRowsBtn == (RadioButton*)pCtrl )
        nModifyFlag |= MODIFY_ROWS;
    else if ( &aColumnsBtn == (RadioButton*)pCtrl )
        nModifyFlag |= MODIFY_COLUMNS;
    else if ( &aAllTablesCB == (CheckBox*)pCtrl )
        nModifyFlag |= MODIFY_ALLTABLES;
}

// -----------------------------------------------------------------------

void SvxSearchDialog::SaveToModule_Impl()
{
    if ( !pSearchItem )
        return;

    if ( aLayoutBtn.IsChecked() )
    {
        pSearchItem->SetSearchString ( aSearchTmplLB.GetSelectEntry() );
        pSearchItem->SetReplaceString( aReplaceTmplLB.GetSelectEntry() );
    }
    else
    {
        pSearchItem->SetSearchString ( aSearchLB.GetText() );
        pSearchItem->SetReplaceString( aReplaceLB.GetText() );
        Remember_Impl( aSearchLB.GetText(), TRUE );
    }

    pSearchItem->SetWordOnly( GetCheckBoxValue( aWordBtn ) );
    pSearchItem->SetPattern( GetCheckBoxValue( aLayoutBtn ) );
    pSearchItem->SetSelection( GetCheckBoxValue( aSelectionBtn ) );
    pSearchItem->SetExact( GetCheckBoxValue( aExactBtn ) );
    pSearchItem->SetRegExp( GetCheckBoxValue( aRegExpBtn ) );
    pSearchItem->SetBackward( GetCheckBoxValue( aBackwardsBtn ) );

    if ( !bWriter )
    {
        if ( aFormulasBtn.IsChecked() )
            pSearchItem->SetCellType( SVX_SEARCHIN_FORMULA );
        else if ( aValuesBtn.IsChecked() )
            pSearchItem->SetCellType( SVX_SEARCHIN_VALUE );
        else if ( aNotesBtn.IsChecked() )
            pSearchItem->SetCellType( SVX_SEARCHIN_NOTE );

        pSearchItem->SetRowDirection( aRowsBtn.IsChecked() );
        pSearchItem->SetAllTables( aAllTablesCB.IsChecked() );
    }

    pSearchItem->SetCommand( SVX_SEARCHCMD_FIND );
    nModifyFlag = 0;
    NotifyApp( SID_SEARCH_ITEM );
}

// class SvxSearchDialogWrapper ------------------------------------------

SFX_IMPL_CHILDWINDOW(SvxSearchDialogWrapper, SID_SEARCH_DLG)

// -----------------------------------------------------------------------

SvxSearchDialogWrapper::SvxSearchDialogWrapper( Window* pParent, USHORT nId,
                                                SfxBindings* pBindings,
                                                SfxChildWinInfo* pInfo ) :
    SfxChildWindow( pParent, nId )

{
    pWindow = new SvxSearchDialog( pParent, *pBindings );

    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
        pWindow->SetPosPixel( pInfo->aPos );
    pBindings->Update( SID_SEARCH_ITEM );
    pBindings->Update( SID_SEARCH_OPTIONS );
    pBindings->Update( SID_SEARCH_SEARCHSET );
    pBindings->Update( SID_SEARCH_REPLACESET );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ( (SvxSearchDialog*)pWindow )->bConstruct = FALSE;
}

// -----------------------------------------------------------------------

SfxChildWinInfo SvxSearchDialogWrapper::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}


