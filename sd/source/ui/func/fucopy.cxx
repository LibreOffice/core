/*************************************************************************
 *
 *  $RCSfile: fucopy.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:35 $
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

#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif
#include <svx/svxids.hrc>

#include "sdresid.hxx"
#include "sdattr.hxx"
#include "strings.hrc"
#include "viewshel.hxx"
#include "sdview.hxx"
#include "fucopy.hxx"
#include "copydlg.hxx"
#include "drawdoc.hxx"
#include "docshell.hxx"

#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SVX_XCOLORITEM_HXX //autogen
#include <svx/xcolit.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
#endif
#ifndef _XDEF_HXX //autogen
#include <svx/xdef.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif


TYPEINIT1( FuCopy, FuPoor );

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

FuCopy::FuCopy(SdViewShell* pViewSh, SdWindow* pWin, SdView* pView,
                         SdDrawDocument* pDoc, SfxRequest& rReq)
    : FuPoor(pViewSh, pWin, pView, pDoc, rReq)
{
    if( pView->HasMarkedObj() )
    {
        // Undo
        String aString( pView->GetMarkDescription() );
        aString.Append( sal_Unicode(' ') );
        aString.Append( String( SdResId( STR_UNDO_COPYOBJECTS ) ) );
        pView->BegUndo( aString );

        const SfxItemSet* pArgs = rReq.GetArgs();

        if( !pArgs )
        {
            SfxItemSet aSet( pViewSh->GetPool(),
                                ATTR_COPY_START, ATTR_COPY_END, 0 );

            // Farb-Attribut angeben
            SfxItemSet aAttr( pDoc->GetPool() );
            pView->GetAttributes( aAttr );
            const SfxPoolItem*  pPoolItem = NULL;

            if( SFX_ITEM_SET == aAttr.GetItemState( XATTR_FILLSTYLE, TRUE, &pPoolItem ) )
            {
                XFillStyle eStyle = ( ( const XFillStyleItem* ) pPoolItem )->GetValue();

                if( eStyle == XFILL_SOLID &&
                    SFX_ITEM_SET == aAttr.GetItemState( XATTR_FILLCOLOR, TRUE, &pPoolItem ) )
                {
                    const XFillColorItem* pItem = ( const XFillColorItem* ) pPoolItem;
                    XColorItem aXColorItem( ATTR_COPY_START_COLOR, pItem->GetName(),
                                                        pItem->GetValue() );
                    aSet.Put( aXColorItem );

                }
            }

            SdCopyDlg* pDlg = new SdCopyDlg( NULL, aSet,
                                pDoc->GetColorTable(), pView );

            USHORT nResult = pDlg->Execute();

            switch( nResult )
            {
                case RET_OK:
                    pDlg->GetAttr( aSet );
                    rReq.Done( aSet );
                    pArgs = rReq.GetArgs();
                break;

                default:
                {
                    delete pDlg;
                    pView->EndUndo();
                }
                return; // Abbruch
            }
            delete( pDlg );
        }

        Rectangle   aRect;
        INT32       lWidth;
        INT32       lHeight;
        INT32       lSizeX = 0L;
        INT32       lSizeY = 0L;
        INT32       lAngle = 0L;
        UINT16      nNumber = 0;
        Color       aStartColor;
        Color       aEndColor;
        BOOL        bColor = FALSE;
        const SfxPoolItem *pPoolItem = NULL;

        // Anzahl
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_NUMBER, TRUE, &pPoolItem ) )
            nNumber = ( ( const SfxUInt16Item* ) pPoolItem )->GetValue();

        // Verschiebung
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_MOVE_X, TRUE, &pPoolItem ) )
            lSizeX = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_MOVE_Y, TRUE, &pPoolItem ) )
            lSizeY = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_ANGLE, TRUE, &pPoolItem ) )
            lAngle = ( ( const SfxInt32Item* )pPoolItem )->GetValue();

        // Verrgroesserung / Verkleinerung
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_WIDTH, TRUE, &pPoolItem ) )
            lWidth = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_HEIGHT, TRUE, &pPoolItem ) )
            lHeight = ( ( const SfxInt32Item* ) pPoolItem )->GetValue();

        // Startfarbe / Endfarbe
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_START_COLOR, TRUE, &pPoolItem ) )
        {
            aStartColor = ( ( const XColorItem* ) pPoolItem )->GetValue();
            bColor = TRUE;
        }
        if( SFX_ITEM_SET == pArgs->GetItemState( ATTR_COPY_END_COLOR, TRUE, &pPoolItem ) )
        {
            aEndColor = ( ( const XColorItem* ) pPoolItem )->GetValue();
            if( aStartColor == aEndColor )
                bColor = FALSE;
        }
        else
            bColor = FALSE;

        // Handles wegnehmen
        pView->HideMarkHdl( NULL );



        SfxProgress* pProgress = NULL;
        BOOL bWaiting = FALSE;

        if( nNumber > 1 )
        {
            String aStr( SdResId( STR_OBJECTS ) );
            aStr.Append( sal_Unicode(' ') );
            aStr.Append( String( SdResId( STR_UNDO_COPYOBJECTS ) ) );

            pProgress = new SfxProgress( pDocSh, aStr, nNumber );
            pDocSh->SetWaitCursor( TRUE );
            bWaiting = TRUE;
        }

        BOOL bFirst = TRUE;

// Removed long outcommented source ( Rev. 1.28 )

        SdrMarkList     aMarkList = pView->GetMarkList();
        ULONG           nMarkCount = aMarkList.GetMarkCount();
        SdrPageView*    pPageView = pView->GetPageViewPvNum( 0 );
        SdrObject*      pObj = NULL;

        // Anzahl moeglicher Kopien berechnen
        aRect = pView->GetAllMarkedRect();
        if( lWidth < 0L )
        {
            long nTmp = ( aRect.Right() - aRect.Left() ) / -lWidth;
            nNumber = (UINT16) Min( nTmp, (long)nNumber );
        }
        if( lHeight < 0L )
        {
            long nTmp = ( aRect.Bottom() - aRect.Top() ) / -lHeight;
            nNumber = (UINT16) Min( nTmp, (long)nNumber );
        }

        for( unsigned short i = 1; i <= nNumber; i++ )
        {
            if ( pProgress )
                pProgress->SetState( i );

            //
            if( !bFirst && pPageView )
            {
                pView->UnmarkAll();
                for( ULONG j = 0; j < nMarkCount; j++ )
                {
                    pObj = aMarkList.GetMark( j )->GetObj();
                    if( pObj )
                    {
                        pView->MarkObj( pObj, pPageView, FALSE );
                    }
                }
            }

            aRect = pView->GetAllMarkedRect();

            // Das erste mal wird evtl. die Farbe des Ursprungsobjektes gesetzt
            if( bFirst && bColor )
            {
                SfxItemSet aNewSet( pViewSh->GetPool(), XATTR_FILLSTYLE, XATTR_FILLCOLOR, 0L );
                aNewSet.Put( XFillStyleItem( XFILL_SOLID ) );
                aNewSet.Put( XFillColorItem( String(), aStartColor ) );
                pView->SetAttributes( aNewSet );
            }

            // Pruefen, ob Objekt groesser (oder gleich) Null wird
            /*
            if( ( aRect.GetWidth() + lWidth <= 0 ) ||
                ( aRect.GetHeight() + lHeight <= 0) )
                break;
            */
            pView->CopyMarked();

            // Das erste mal werden ggfs. die Protect-Flags entfernt
            if( bFirst )
            {
                for( ULONG j = 0; j < nMarkCount; j++ )
                {
                    pObj = aMarkList.GetMark( j )->GetObj();
                    if( pObj )
                    {
                        pObj->SetMoveProtect( FALSE );
                        pObj->SetResizeProtect( FALSE );
                    }
                }
                bFirst = FALSE;
            }

            Fraction aWidth( aRect.Right() - aRect.Left() + lWidth * i,
                                aRect.Right() - aRect.Left() );
            Fraction aHeight( aRect.Bottom() - aRect.Top() + lHeight * i,
                                aRect.Bottom() - aRect.Top() );

            if( pView->IsResizeAllowed() )
                pView->ResizeAllMarked( aRect.TopLeft(), aWidth, aHeight );

            if( pView->IsRotateAllowed() )
                pView->RotateAllMarked( aRect.Center(), lAngle * 100 * i );

            if( pView->IsMoveAllowed() )
                pView->MoveAllMarked( Size( lSizeX * i, lSizeY * i ) );

            if( bColor )
            {
                // Koennte man sicher noch optimieren, wuerde aber u.U.
                // zu Rundungsfehlern fuehren
                USHORT nRed = aStartColor.GetRed() + (USHORT) ( ( (long) aEndColor.GetRed() - (long) aStartColor.GetRed() ) * (long) i / (long) nNumber  );
                USHORT nGreen = aStartColor.GetGreen() + (USHORT) ( ( (long) aEndColor.GetGreen() - (long) aStartColor.GetGreen() ) *  (long) i / (long) nNumber );
                USHORT nBlue = aStartColor.GetBlue() + (USHORT) ( ( (long) aEndColor.GetBlue() - (long) aStartColor.GetBlue() ) * (long) i / (long) nNumber );
                Color aNewColor( nRed, nGreen, nBlue );
                SfxItemSet aNewSet( pViewSh->GetPool(), XATTR_FILLSTYLE, XATTR_FILLCOLOR, 0L );
                aNewSet.Put( XFillStyleItem( XFILL_SOLID ) );
                aNewSet.Put( XFillColorItem( String(), aNewColor ) );
                pView->SetAttributes( aNewSet );
            }
        }

        if ( pProgress )
            delete pProgress;

        if ( bWaiting )
            pDocSh->SetWaitCursor( FALSE );

        // Handles zeigen
        pView->AdjustMarkHdl( TRUE );
        pView->ShowMarkHdl( NULL );

        pView->EndUndo();
    }
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

FuCopy::~FuCopy()
{
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void FuCopy::Activate()
{
    FuPoor::Activate();
}

/*************************************************************************
|*
|* Function deaktivieren
|*
\************************************************************************/

void FuCopy::Deactivate()
{
    FuPoor::Deactivate();
}


