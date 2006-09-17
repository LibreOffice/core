/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: srchxtra.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:39:41 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#include "srchxtra.hxx"

#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif

#ifndef _SVX_DIALOGS_HRC
#include "dialogs.hrc"
#endif
#ifndef _SVX_SRCHXTRA_HRC
#include "srchxtra.hrc"
#endif
#ifndef _SVXITEMS_HRC
#include "svxitems.hrc"
#endif

#ifndef _SVX_FLSTITEM_HXX
#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST
#include "flstitem.hxx"
#endif
#ifndef _SVX_CHARDLG_HXX
#include "chardlg.hxx"
#endif
#ifndef _SVX_PARAGRPH_HXX
#include "paragrph.hxx"
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _SVX_BACKGRND_HXX
#include "backgrnd.hxx"
#endif

// class SvxSearchFormatDialog -------------------------------------------

SvxSearchFormatDialog::SvxSearchFormatDialog( Window* pParent, const SfxItemSet& rSet ) :

    SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_SEARCHFORMAT ), &rSet ),

    pFontList( NULL )

{
    FreeResource();

    AddTabPage( RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_POSITION, SvxCharPositionPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_TWOLINES, SvxCharTwoLinesPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH, SvxStdParagraphTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH, SvxParaAlignTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_EXT_PARAGRAPH, SvxExtParagraphTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_PARA_ASIAN, SvxAsianTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BACKGROUND, SvxBackgroundTabPage::Create, 0 );

    // remove asian tabpages if necessary
    SvtCJKOptions aCJKOptions;
    if ( !aCJKOptions.IsDoubleLinesEnabled() )
        RemoveTabPage( RID_SVXPAGE_CHAR_TWOLINES );
    if ( !aCJKOptions.IsAsianTypographyEnabled() )
        RemoveTabPage( RID_SVXPAGE_PARA_ASIAN );
}

// -----------------------------------------------------------------------

SvxSearchFormatDialog::~SvxSearchFormatDialog()
{
    delete pFontList;
}

// -----------------------------------------------------------------------

void SvxSearchFormatDialog::PageCreated( USHORT nId, SfxTabPage& rPage )
{
    switch ( nId )
    {
        case RID_SVXPAGE_CHAR_NAME:
        {
            const FontList* pAppFontList = 0;
            SfxObjectShell* pSh = SfxObjectShell::Current();

            if ( pSh )
            {
                const SvxFontListItem* pFLItem = (const SvxFontListItem*)
                    pSh->GetItem( SID_ATTR_CHAR_FONTLIST );
                if ( pFLItem )
                    pAppFontList = pFLItem->GetFontList();
            }

            const FontList* pList = pAppFontList;

            if ( !pList )
            {
                if ( !pFontList )
                    pFontList = new FontList( this );
                pList = pFontList;
            }

            if ( pList )
                ( (SvxCharNamePage&)rPage ).
                    SetFontList( SvxFontListItem( pList ) );
            ( (SvxCharNamePage&)rPage ).EnableSearchMode();
            break;
        }

        case RID_SVXPAGE_STD_PARAGRAPH:
            ( (SvxStdParagraphTabPage&)rPage ).EnableAutoFirstLine();
            break;

        case RID_SVXPAGE_ALIGN_PARAGRAPH:
            ( (SvxParaAlignTabPage&)rPage ).EnableJustifyExt();
            break;
        case RID_SVXPAGE_BACKGROUND :
            ( (SvxBackgroundTabPage&)rPage ).ShowParaControl(TRUE);
            break;
    }
}

// class SvxSearchFormatDialog -------------------------------------------

SvxSearchAttributeDialog::SvxSearchAttributeDialog( Window* pParent,
                                                    SearchAttrItemList& rLst,
                                                    const USHORT* pWhRanges ) :

    ModalDialog( pParent, SVX_RES( RID_SVXDLG_SEARCHATTR )  ),

    aAttrFL ( this, ResId( FL_ATTR ) ),
    aAttrLB ( this, ResId( LB_ATTR ) ),
    aOKBtn  ( this, ResId( BTN_ATTR_OK ) ),
    aEscBtn ( this, ResId( BTN_ATTR_CANCEL ) ),
    aHelpBtn( this, ResId( BTN_ATTR_HELP ) ),

    rList( rLst )

{
    FreeResource();

    aAttrLB.SetWindowBits( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_SORT );
    aAttrLB.GetModel()->SetSortMode( SortAscending );

    aOKBtn.SetClickHdl( LINK( this, SvxSearchAttributeDialog, OKHdl ) );

    SfxObjectShell* pSh = SfxObjectShell::Current();
    DBG_ASSERT( pSh, "No DocShell" );

    SfxItemPool& rPool = pSh->GetPool();
    SfxItemSet aSet( rPool, pWhRanges );
    SfxWhichIter aIter( aSet );
    USHORT nWhich = aIter.FirstWhich();

    while ( nWhich )
    {
        USHORT nSlot = rPool.GetSlotId( nWhich );
        if ( nSlot >= SID_SVX_START )
        {
            BOOL bChecked = FALSE, bFound = FALSE;
            for ( USHORT i = 0; !bFound && i < rList.Count(); ++i )
            {
                if ( nSlot == rList[i].nSlot )
                {
                    bFound = TRUE;
                    if ( IsInvalidItem( rList[i].pItem ) )
                        bChecked = TRUE;
                }
            }

            USHORT nResId = nSlot - SID_SVX_START + RID_ATTR_BEGIN;
            SvLBoxEntry* pEntry = NULL;
            ResId aId( nResId );
            aId.SetRT( RSC_STRING );
            if ( DIALOG_MGR()->IsAvailable( aId ) )
                pEntry = aAttrLB.SvTreeListBox::InsertEntry( SVX_RESSTR( nResId ) );
            else
            {
                ByteString sError( "no resource for slot id\nslot = " );
                sError += ByteString::CreateFromInt32( nSlot );
                sError += ByteString( "\nresid = " );
                sError += ByteString::CreateFromInt32( nResId );
                DBG_ERRORFILE( sError.GetBuffer() );
            }

            if ( pEntry )
            {
                aAttrLB.SetCheckButtonState( pEntry, bChecked ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
                pEntry->SetUserData( (void*)(ULONG)nSlot );
            }
        }
        nWhich = aIter.NextWhich();
    }

    aAttrLB.SetHighlightRange();
    aAttrLB.SelectEntryPos( 0 );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxSearchAttributeDialog, OKHdl, Button *, EMPTYARG )
{
    SearchAttrItem aInvalidItem;
    aInvalidItem.pItem = (SfxPoolItem*)-1;

    for ( USHORT i = 0; i < aAttrLB.GetEntryCount(); ++i )
    {
        USHORT nSlot = (USHORT)(ULONG)aAttrLB.GetEntryData(i);
        BOOL bChecked = aAttrLB.IsChecked(i);

        USHORT j;
        for ( j = rList.Count(); j; )
        {
            SearchAttrItem& rItem = rList[ --j ];
            if( rItem.nSlot == nSlot )
            {
                if( bChecked )
                {
                    if( !IsInvalidItem( rItem.pItem ) )
                        delete rItem.pItem;
                    rItem.pItem = (SfxPoolItem*)-1;
                }
                else if( IsInvalidItem( rItem.pItem ) )
                    rItem.pItem = 0;
                j = 1;
                break;
            }
        }

        if ( !j && bChecked )
        {
            aInvalidItem.nSlot = nSlot;
            rList.Insert( aInvalidItem );
        }
    }

    // remove invalid items (pItem == NULL)
    for ( USHORT n = rList.Count(); n; )
        if ( !rList[ --n ].pItem )
            rList.Remove( n );

    EndDialog( RET_OK );
    return 0;
}

// class SvxSearchSimilarityDialog ---------------------------------------

SvxSearchSimilarityDialog::SvxSearchSimilarityDialog
(
    Window* pParent,
    BOOL bRelax,
    USHORT nOther,
    USHORT nShorter,
    USHORT nLonger
) :
    ModalDialog( pParent, SVX_RES( RID_SVXDLG_SEARCHSIMILARITY ) ),

    aFixedLine  ( this, ResId( FL_SIMILARITY ) ),
    aOtherTxt   ( this, ResId( FT_OTHER ) ),
    aOtherFld   ( this, ResId( NF_OTHER ) ),
    aLongerTxt  ( this, ResId( FT_LONGER ) ),
    aLongerFld  ( this, ResId( NF_LONGER ) ),
    aShorterTxt ( this, ResId( FT_SHORTER ) ),
    aShorterFld ( this, ResId( NF_SHORTER ) ),
    aRelaxBox   ( this, ResId( CB_RELAX ) ),

    aOKBtn      ( this, ResId( BTN_ATTR_OK ) ),
    aEscBtn     ( this, ResId( BTN_ATTR_CANCEL ) ),
    aHelpBtn    ( this, ResId( BTN_ATTR_HELP ) )

{
    FreeResource();

    aOtherFld.SetValue( nOther );
    aShorterFld.SetValue( nShorter );
    aLongerFld.SetValue( nLonger );
    aRelaxBox.Check( bRelax );
}

