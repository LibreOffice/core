/*************************************************************************
 *
 *  $RCSfile: impprn.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:57:57 $
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

#ifndef REMOTE_APPSERVER

#define _SV_IMPPRN_CXX
#define _SPOOLPRINTER_EXT

#ifndef _QUEUE_HXX
#include <tools/queue.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_GDIMTF_HXX
#include <gdimtf.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_IMPPRN_HXX
#include <impprn.hxx>
#endif

// -----------
// - Defines -
// -----------

#define OPTIMAL_BMP_RESOLUTION  300
#define NORMAL_BMP_RESOLUTION   200

// =======================================================================

struct QueuePage
{
    GDIMetaFile*    mpMtf;
    JobSetup*       mpSetup;
    USHORT          mnPage;
    BOOL            mbEndJob;

                    QueuePage() { mpMtf = NULL; mpSetup = NULL; }
                    ~QueuePage() { delete mpMtf; if ( mpSetup ) delete mpSetup; }
};

// =======================================================================

ImplQPrinter::ImplQPrinter( Printer* pParent ) :
    Printer( pParent->GetName() )
{
    SetSelfAsQueuePrinter( TRUE );
    SetPrinterProps( pParent );
    SetPageQueueSize( 0 );
    mpParent        = pParent;
    mnCopyCount     = pParent->mnCopyCount;
    mbCollateCopy   = pParent->mbCollateCopy;
    mpQueue         = new Queue( mpParent->GetPageQueueSize() );
    mbAborted       = FALSE;
    mbUserCopy      = FALSE;
    mbDestroyAllowed= TRUE;
    mbDestroyed     = FALSE;
}

// -----------------------------------------------------------------------

ImplQPrinter::~ImplQPrinter()
{
    QueuePage* pQueuePage = (QueuePage*)mpQueue->Get();
    while ( pQueuePage )
    {
        delete pQueuePage;
        pQueuePage = (QueuePage*)mpQueue->Get();
    }

    delete mpQueue;
}

// -----------------------------------------------------------------------------

void ImplQPrinter::Destroy()
{
    if( mbDestroyAllowed )
        delete this;
    else
        mbDestroyed = TRUE;
}

// -----------------------------------------------------------------------

void ImplQPrinter::ImplPrintMtf( GDIMetaFile& rMtf, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    const PrinterOptions& rPrinterOptions = GetPrinterOptions();

    for( MetaAction* pAct = rMtf.FirstAction(); pAct && !mbAborted; pAct = rMtf.NextAction() )
    {
        const ULONG     nType = pAct->GetType();
        sal_Bool        bExecuted = sal_False;

        if( nType == META_COMMENT_ACTION )
        {
            // search for special comments ( ..._BEGIN/..._END )
            MetaCommentAction* pComment = (MetaCommentAction*) pAct;

            if( pComment->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
            {
                pAct = rMtf.NextAction();

                // if next action is a GradientEx action, execute this and
                // skip actions until a XGRAD_SEQ_END comment is found
                if( pAct && ( pAct->GetType() == META_GRADIENTEX_ACTION ) )
                {
                    MetaGradientExAction* pGradientExAction = (MetaGradientExAction*) pAct;
                    DrawGradientEx( this, pGradientExAction->GetPolyPolygon(), pGradientExAction->GetGradient() );

                    // seek to end of this comment
                    do
                    {
                        pAct = rMtf.NextAction();
                    }
                    while( pAct &&
                           ( ( pAct->GetType() != META_COMMENT_ACTION ) ||
                             ( ( (MetaCommentAction*) pAct )->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) != COMPARE_EQUAL ) ) );

                    bExecuted = sal_True;
                }
            }
            else if( pComment->GetComment().CompareIgnoreCaseToAscii( "PRNSPOOL_TRANSPARENTBITMAP_BEGIN" ) == COMPARE_EQUAL )
            {
                pAct = rMtf.NextAction();

                if( pAct && ( pAct->GetType() == META_BMPSCALE_ACTION ) )
                {
                    MetaBmpScaleAction* pBmpScaleAction = (MetaBmpScaleAction*) pAct;

                    // execute action here to avoid DPI processing of bitmap;
                    pAct->Execute( this );

                    // seek to end of this comment
                    do
                    {
                        pAct = rMtf.NextAction();
                    }
                    while( pAct &&
                           ( ( pAct->GetType() != META_COMMENT_ACTION ) ||
                             ( ( (MetaCommentAction*) pAct )->GetComment().CompareIgnoreCaseToAscii( "PRNSPOOL_TRANSPARENTBITMAP_END" ) != COMPARE_EQUAL ) ) );

                    bExecuted = sal_True;
                }
            }
        }
        else if( nType == META_GRADIENT_ACTION )
        {
            MetaGradientAction* pGradientAction = (MetaGradientAction*) pAct;
            DrawGradientEx( this, pGradientAction->GetRect(), pGradientAction->GetGradient() );
            bExecuted = sal_True;
        }
        else if( nType == META_BMPSCALE_ACTION )
        {
            MetaBmpScaleAction* pBmpScaleAction = (MetaBmpScaleAction*) pAct;
            const Bitmap&       rBmp = pBmpScaleAction->GetBitmap();

            DrawBitmap( pBmpScaleAction->GetPoint(), pBmpScaleAction->GetSize(),
                        GetPreparedBitmap( pBmpScaleAction->GetPoint(), pBmpScaleAction->GetSize(),
                                           Point(), rBmp.GetSizePixel(),
                                           rBmp, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPSCALEPART_ACTION )
        {
            MetaBmpScalePartAction* pBmpScalePartAction = (MetaBmpScalePartAction*) pAct;
            const Bitmap&           rBmp = pBmpScalePartAction->GetBitmap();

            DrawBitmap( pBmpScalePartAction->GetDestPoint(), pBmpScalePartAction->GetDestSize(),
                        GetPreparedBitmap( pBmpScalePartAction->GetDestPoint(), pBmpScalePartAction->GetDestSize(),
                                           pBmpScalePartAction->GetSrcPoint(), pBmpScalePartAction->GetSrcSize(),
                                           rBmp, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPEXSCALE_ACTION )
        {
            MetaBmpExScaleAction*   pBmpExScaleAction = (MetaBmpExScaleAction*) pAct;
            const BitmapEx&         rBmpEx = pBmpExScaleAction->GetBitmapEx();

            DrawBitmapEx( pBmpExScaleAction->GetPoint(), pBmpExScaleAction->GetSize(),
                          GetPreparedBitmapEx( pBmpExScaleAction->GetPoint(), pBmpExScaleAction->GetSize(),
                                               Point(), rBmpEx.GetSizePixel(),
                                               rBmpEx, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPEXSCALEPART_ACTION )
        {
            MetaBmpExScalePartAction*   pBmpExScalePartAction = (MetaBmpExScalePartAction*) pAct;
            const BitmapEx&             rBmpEx = pBmpExScalePartAction->GetBitmapEx();

            DrawBitmapEx( pBmpExScalePartAction->GetDestPoint(), pBmpExScalePartAction->GetDestSize(),
                          GetPreparedBitmapEx( pBmpExScalePartAction->GetDestPoint(), pBmpExScalePartAction->GetDestSize(),
                                               pBmpExScalePartAction->GetSrcPoint(), pBmpExScalePartAction->GetSrcSize(),
                                               rBmpEx, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_TRANSPARENT_ACTION )
        {
            MetaTransparentAction*  pTransAct = static_cast<MetaTransparentAction*>(pAct);
            USHORT                  nTransparency( pTransAct->GetTransparence() );

            // #i10613# Respect transparency for draw color
            if( nTransparency )
            {
                Push( PUSH_LINECOLOR|PUSH_FILLCOLOR );

                // assume white background for alpha blending
                Color aLineColor( GetLineColor() );
                aLineColor.SetRed( static_cast<UINT8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetRed()) / 100L ) );
                aLineColor.SetGreen( static_cast<UINT8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetGreen()) / 100L ) );
                aLineColor.SetBlue( static_cast<UINT8>( (255L*nTransparency + (100L - nTransparency)*aLineColor.GetBlue()) / 100L ) );
                SetLineColor( aLineColor );

                Color aFillColor( GetFillColor() );
                aFillColor.SetRed( static_cast<UINT8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetRed()) / 100L ) );
                aFillColor.SetGreen( static_cast<UINT8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetGreen()) / 100L ) );
                aFillColor.SetBlue( static_cast<UINT8>( (255L*nTransparency + (100L - nTransparency)*aFillColor.GetBlue()) / 100L ) );
                SetFillColor( aFillColor );
            }

            DrawPolyPolygon( pTransAct->GetPolyPolygon() );

            if( nTransparency )
                Pop();

            bExecuted = sal_True;
        }
        else if( nType == META_FLOATTRANSPARENT_ACTION )
        {
            MetaFloatTransparentAction* pFloatAction = (MetaFloatTransparentAction*) pAct;
            GDIMetaFile&                rMtf = (GDIMetaFile&) pFloatAction->GetGDIMetaFile();
            MapMode                     aDrawMap( rMtf.GetPrefMapMode() );
            Point                       aDestPoint( LogicToPixel( pFloatAction->GetPoint() ) );
            Size                        aDestSize( LogicToPixel( pFloatAction->GetSize() ) );

            if( aDestSize.Width() && aDestSize.Height() )
            {
                Size aTmpPrefSize( LogicToPixel( rMtf.GetPrefSize(), aDrawMap ) );

                if( !aTmpPrefSize.Width() )
                    aTmpPrefSize.Width() = aDestSize.Width();

                if( !aTmpPrefSize.Height() )
                    aTmpPrefSize.Height() = aDestSize.Height();

                Fraction aScaleX( aDestSize.Width(), aTmpPrefSize.Width() );
                Fraction aScaleY( aDestSize.Height(), aTmpPrefSize.Height() );

                aDrawMap.SetScaleX( aScaleX *= aDrawMap.GetScaleX() );
                aDrawMap.SetScaleY( aScaleY *= aDrawMap.GetScaleY() );
                aDrawMap.SetOrigin( PixelToLogic( aDestPoint, aDrawMap ) );

                Push();
                SetMapMode( aDrawMap );
                ImplPrintMtf( rMtf, nMaxBmpDPIX, nMaxBmpDPIY );
                Pop();
            }

            bExecuted = sal_True;
        }

        if( !bExecuted && pAct )
            pAct->Execute( this );

        Application::Reschedule();
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplQPrinter, ImplPrintHdl, Timer*, EMPTYARG )
{
    // Ist Drucken abgebrochen wurden?
    if( !IsPrinting() || ( mpParent->IsJobActive() && ( mpQueue->Count() < (ULONG)mpParent->GetPageQueueSize() ) ) )
        return 0;

    // Druck-Job zuende?
    QueuePage* pActPage = (QueuePage*) mpQueue->Get();

    if ( pActPage->mbEndJob )
    {
        maTimer.Stop();
        delete pActPage;
        EndJob();
        mpParent->ImplEndPrint();
    }
    else
    {
        GDIMetaFile             aMtf;
        const PrinterOptions&   rPrinterOptions = GetPrinterOptions();
        const ULONG             nOldDrawMode = GetDrawMode();
        long                    nMaxBmpDPIX = mnDPIX;
        long                    nMaxBmpDPIY = mnDPIY;
        USHORT                  nCopyCount = 1;

        if( rPrinterOptions.IsReduceBitmaps() )
        {
            // calculate maximum resolution for bitmap graphics
            if( PRINTER_BITMAP_OPTIMAL == rPrinterOptions.GetReducedBitmapMode() )
            {
                nMaxBmpDPIX = Min( (long) OPTIMAL_BMP_RESOLUTION, nMaxBmpDPIX );
                nMaxBmpDPIY = Min( (long) OPTIMAL_BMP_RESOLUTION, nMaxBmpDPIY );
            }
            else if( PRINTER_BITMAP_NORMAL == rPrinterOptions.GetReducedBitmapMode() )
            {
                nMaxBmpDPIX = Min( (long) NORMAL_BMP_RESOLUTION, nMaxBmpDPIX );
                nMaxBmpDPIY = Min( (long) NORMAL_BMP_RESOLUTION, nMaxBmpDPIY );
            }
            else
            {
                nMaxBmpDPIX = Min( (long) rPrinterOptions.GetReducedBitmapResolution(), nMaxBmpDPIX );
                nMaxBmpDPIY = Min( (long) rPrinterOptions.GetReducedBitmapResolution(), nMaxBmpDPIY );
            }
        }

        // convert to greysacles
        if( rPrinterOptions.IsConvertToGreyscales() )
        {
            SetDrawMode( GetDrawMode() | ( DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_GRAYTEXT |
                                           DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT ) );
        }

        // disable transparency output
        if( rPrinterOptions.IsReduceTransparency() && ( PRINTER_TRANSPARENCY_NONE == rPrinterOptions.GetReducedTransparencyMode() ) )
        {
            SetDrawMode( GetDrawMode() | DRAWMODE_NOTRANSPARENCY );
        }

        mbDestroyAllowed = FALSE;
        GetPreparedMetaFile( *pActPage->mpMtf, aMtf, nMaxBmpDPIX, nMaxBmpDPIY );

        if( mbUserCopy && !mbCollateCopy )
            nCopyCount = mnCopyCount;

        for ( USHORT i = 0; i < nCopyCount; i++ )
        {
            ULONG nActionPos = 0UL;

            if ( pActPage->mpSetup )
            {
                SetJobSetup( *pActPage->mpSetup );
                if ( mbAborted )
                     break;
            }

            StartPage();

            if ( mbAborted )
                break;

            ImplPrintMtf( aMtf, nMaxBmpDPIX, nMaxBmpDPIY );

            if( !mbAborted )
                EndPage();
            else
                break;
        }

        SetDrawMode( nOldDrawMode );

        delete pActPage;
        mbDestroyAllowed = TRUE;

        if( mbDestroyed )
            Destroy();
    }

    return 0;
}

// -----------------------------------------------------------------------

void ImplQPrinter::StartQueuePrint()
{
    maTimer.SetTimeout( 50 );
    maTimer.SetTimeoutHdl( LINK( this, ImplQPrinter, ImplPrintHdl ) );
    maTimer.Start();
}

// -----------------------------------------------------------------------

void ImplQPrinter::EndQueuePrint()
{
    QueuePage* pQueuePage   = new QueuePage;
    pQueuePage->mbEndJob    = TRUE;
    mpQueue->Put( pQueuePage );
}

// -----------------------------------------------------------------------

void ImplQPrinter::AbortQueuePrint()
{
    maTimer.Stop();
    mbAborted = TRUE;
    AbortJob();
}

// -----------------------------------------------------------------------

void ImplQPrinter::AddQueuePage( GDIMetaFile* pPage, USHORT nPage, BOOL bNewJobSetup )
{
    QueuePage* pQueuePage   = new QueuePage;
    pQueuePage->mpMtf       = pPage;
    pQueuePage->mnPage      = nPage;
    pQueuePage->mbEndJob    = FALSE;
    if ( bNewJobSetup )
        pQueuePage->mpSetup = new JobSetup( mpParent->GetJobSetup() );
    mpQueue->Put( pQueuePage );
}

#endif
