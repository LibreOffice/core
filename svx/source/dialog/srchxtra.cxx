/*************************************************************************
 *
 *  $RCSfile: srchxtra.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pb $ $Date: 2000-11-27 09:03:18 $
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

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#pragma hdrstop

#include "srchxtra.hxx"

#include "dialogs.hrc"
#include "srchxtra.hrc"
#include "svxitems.hrc"

#define ITEMID_FONTLIST     SID_ATTR_CHAR_FONTLIST
#include "flstitem.hxx"

#include "chardlg.hxx"
#include "paragrph.hxx"
#include "dialmgr.hxx"
#ifndef _SVX_BACKGRND_HXX //autogen
#include <backgrnd.hxx>
#endif

// class SvxSearchFormatDialog -------------------------------------------

SvxSearchFormatDialog::SvxSearchFormatDialog( Window* pParent,
                                              const SfxItemSet& rSet ) :

    SfxTabDialog( pParent, SVX_RES( RID_SVXDLG_SEARCHFORMAT ), &rSet )

{
    AddTabPage( RID_SVXPAGE_CHAR_STD,       SvxCharStdPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_EXT,       SvxCharExtPage::Create, 0 );
#ifdef DEBUG
    AddTabPage( RID_SVXPAGE_CHAR_NAME,      SvxCharNamePage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_EFFECTS,   SvxCharEffectsPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_POSITION,  SvxCharPositionPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_CHAR_TWOLINES,  SvxCharTwoLinesPage::Create, 0 );
#endif
    AddTabPage( RID_SVXPAGE_STD_PARAGRAPH,  SvxStdParagraphTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_ALIGN_PARAGRAPH,SvxParaAlignTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_EXT_PARAGRAPH,  SvxExtParagraphTabPage::Create, 0 );
    AddTabPage( RID_SVXPAGE_BACKGROUND,     SvxBackgroundTabPage::Create, 0 );

    pFontList = 0;
    FreeResource();
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
        case RID_SVXPAGE_CHAR_STD:
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
                ( (SvxCharStdPage&)rPage ).
                    SetFontList( SvxFontListItem( pList ) );
            ( (SvxCharStdPage&)rPage ).EnableSearchMode();
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

    aOKBtn  ( this, ResId( BTN_ATTR_OK ) ),
    aEscBtn ( this, ResId( BTN_ATTR_CANCEL ) ),
    aHelpBtn( this, ResId( BTN_ATTR_HELP ) ),
    aAttrLB ( this, ResId( LB_ATTR ) ),
    aAttrBox( this, ResId( GB_ATTR ) ),

    rList( rLst )

{
    FreeResource();

    aOKBtn.SetClickHdl( LINK( this, SvxSearchAttributeDialog, OKHdl ) );

    SfxObjectShell* pSh = SfxObjectShell::Current();
    DBG_ASSERT( pSh, "Where is the DocShell?" );


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

            if ( SID_ATTR_BRUSH_CHAR == nSlot )
                // Sonderbehandlung f"ur Zeichenhintergrund
                nResId = RID_SVXITEMS_BRUSH_CHAR;
            else if ( SID_ATTR_CHAR_CHARSETCOLOR == nSlot )
                // Sonderbehandlung f"ur Zeichensatzfarbe
                nResId = RID_ATTR_CHAR_CHARSETCOLOR;

            if ( ( nResId >= RID_ATTR_BEGIN && nResId < RID_ATTR_END ) ||
                 RID_SVXITEMS_BRUSH_CHAR == nResId || RID_ATTR_CHAR_CHARSETCOLOR == nResId )
                aAttrLB.InsertEntry( SVX_RESSTR( nResId ) );
            else
            {
                DBG_ERRORFILE( "no resource for slot id" );
            }

            USHORT nPos = (USHORT)aAttrLB.GetEntryCount() - 1;
            aAttrLB.CheckEntryPos( nPos, bChecked );
            aAttrLB.SetEntryData( nPos, (void*)(ULONG)nSlot );
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
        if( !j && bChecked )
        {
            aInvalidItem.nSlot = nSlot;
            rList.Insert( aInvalidItem );
        }
    }

    // ung"ultige Items entfernen (pItem == 0)
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

    aOtherTxt   ( this, ResId( FT_OTHER ) ),
    aOtherFld   ( this, ResId( NF_OTHER ) ),
    aShorterTxt ( this, ResId( FT_SHORTER ) ),
    aShorterFld ( this, ResId( NF_SHORTER ) ),
    aLongerTxt  ( this, ResId( FT_LONGER ) ),
    aLongerFld  ( this, ResId( NF_LONGER ) ),
    aRelaxBox   ( this, ResId( CB_RELAX ) ),
    aGroup      ( this, ResId( GB_SIMILARITY ) ),

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


