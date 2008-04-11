/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: impprn.cxx,v $
 * $Revision: 1.18 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#define _SPOOLPRINTER_EXT
#include <tools/queue.hxx>
#include <vcl/svapp.hxx>
#include <vcl/metaact.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/timer.hxx>
#include <vcl/impprn.hxx>

#include <vcl/svdata.hxx>
#include <vcl/salprn.hxx>

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
    Printer( pParent->GetName() ),
    mpParent( pParent ),
    mbAborted( false ),
    mbUserCopy( false ),
    mbDestroyAllowed( true ),
    mbDestroyed( false ),
    mnMaxBmpDPIX( mnDPIX ),
    mnMaxBmpDPIY( mnDPIY ),
    mnCurCopyCount( 0 )
{
    SetSelfAsQueuePrinter( TRUE );
    SetPrinterProps( pParent );
    SetPageQueueSize( 0 );
    mnCopyCount     = pParent->mnCopyCount;
    mbCollateCopy   = pParent->mbCollateCopy;
}

// -----------------------------------------------------------------------

ImplQPrinter::~ImplQPrinter()
{
    for( std::vector< QueuePage* >::iterator it = maQueue.begin();
         it != maQueue.end(); ++it )
        delete (*it);
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

void ImplQPrinter::ImplPrintMtf( GDIMetaFile& rPrtMtf, long nMaxBmpDPIX, long nMaxBmpDPIY )
{
    for( MetaAction* pAct = rPrtMtf.FirstAction(); pAct && !mbAborted; pAct = rPrtMtf.NextAction() )
    {
        const ULONG     nType = pAct->GetType();
        sal_Bool        bExecuted = sal_False;

        if( nType == META_COMMENT_ACTION )
        {
            // search for special comments ( ..._BEGIN/..._END )
            MetaCommentAction* pComment = (MetaCommentAction*) pAct;

            if( pComment->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_BEGIN" ) == COMPARE_EQUAL )
            {
                pAct = rPrtMtf.NextAction();

                // if next action is a GradientEx action, execute this and
                // skip actions until a XGRAD_SEQ_END comment is found
                if( pAct && ( pAct->GetType() == META_GRADIENTEX_ACTION ) )
                {
                    MetaGradientExAction* pGradientExAction = (MetaGradientExAction*) pAct;
                    DrawGradientEx( this, pGradientExAction->GetPolyPolygon(), pGradientExAction->GetGradient() );

                    // seek to end of this comment
                    do
                    {
                        pAct = rPrtMtf.NextAction();
                    }
                    while( pAct &&
                           ( ( pAct->GetType() != META_COMMENT_ACTION ) ||
                             ( ( (MetaCommentAction*) pAct )->GetComment().CompareIgnoreCaseToAscii( "XGRAD_SEQ_END" ) != COMPARE_EQUAL ) ) );

                    bExecuted = sal_True;
                }
            }
            else if( pComment->GetComment().CompareIgnoreCaseToAscii( "PRNSPOOL_TRANSPARENTBITMAP_BEGIN" ) == COMPARE_EQUAL )
            {
                pAct = rPrtMtf.NextAction();

                if( pAct && ( pAct->GetType() == META_BMPSCALE_ACTION ) )
                {
                    // execute action here to avoid DPI processing of bitmap;
                    pAct->Execute( this );

                    // seek to end of this comment
                    do
                    {
                        pAct = rPrtMtf.NextAction();
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
                        GetPreparedBitmap( pBmpScaleAction->GetSize(),
                                           Point(), rBmp.GetSizePixel(),
                                           rBmp, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPSCALEPART_ACTION )
        {
            MetaBmpScalePartAction* pBmpScalePartAction = (MetaBmpScalePartAction*) pAct;
            const Bitmap&           rBmp = pBmpScalePartAction->GetBitmap();

            DrawBitmap( pBmpScalePartAction->GetDestPoint(), pBmpScalePartAction->GetDestSize(),
                        GetPreparedBitmap( pBmpScalePartAction->GetDestSize(),
                                           pBmpScalePartAction->GetSrcPoint(), pBmpScalePartAction->GetSrcSize(),
                                           rBmp, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPEXSCALE_ACTION )
        {
            MetaBmpExScaleAction*   pBmpExScaleAction = (MetaBmpExScaleAction*) pAct;
            const BitmapEx&         rBmpEx = pBmpExScaleAction->GetBitmapEx();

            DrawBitmapEx( pBmpExScaleAction->GetPoint(), pBmpExScaleAction->GetSize(),
                          GetPreparedBitmapEx( pBmpExScaleAction->GetSize(),
                                               Point(), rBmpEx.GetSizePixel(),
                                               rBmpEx, nMaxBmpDPIX, nMaxBmpDPIY ) );

            bExecuted = sal_True;
        }
        else if( nType == META_BMPEXSCALEPART_ACTION )
        {
            MetaBmpExScalePartAction*   pBmpExScalePartAction = (MetaBmpExScalePartAction*) pAct;
            const BitmapEx&             rBmpEx = pBmpExScalePartAction->GetBitmapEx();

            DrawBitmapEx( pBmpExScalePartAction->GetDestPoint(), pBmpExScalePartAction->GetDestSize(),
                          GetPreparedBitmapEx( pBmpExScalePartAction->GetDestSize(),
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

        if( ! ImplGetSVData()->maGDIData.mbPrinterPullModel )
            Application::Reschedule();
    }
}

// -----------------------------------------------------------------------

void ImplQPrinter::PrePrintPage( QueuePage* pPage )
{
    mnRestoreDrawMode = GetDrawMode();
    mnMaxBmpDPIX = mnDPIX;
    mnMaxBmpDPIY = mnDPIY;

    const PrinterOptions&   rPrinterOptions = GetPrinterOptions();

    if( rPrinterOptions.IsReduceBitmaps() )
    {
        // calculate maximum resolution for bitmap graphics
        if( PRINTER_BITMAP_OPTIMAL == rPrinterOptions.GetReducedBitmapMode() )
        {
            mnMaxBmpDPIX = Min( (long) OPTIMAL_BMP_RESOLUTION, mnMaxBmpDPIX );
            mnMaxBmpDPIY = Min( (long) OPTIMAL_BMP_RESOLUTION, mnMaxBmpDPIY );
        }
        else if( PRINTER_BITMAP_NORMAL == rPrinterOptions.GetReducedBitmapMode() )
        {
            mnMaxBmpDPIX = Min( (long) NORMAL_BMP_RESOLUTION, mnMaxBmpDPIX );
            mnMaxBmpDPIY = Min( (long) NORMAL_BMP_RESOLUTION, mnMaxBmpDPIY );
        }
        else
        {
            mnMaxBmpDPIX = Min( (long) rPrinterOptions.GetReducedBitmapResolution(), mnMaxBmpDPIX );
            mnMaxBmpDPIY = Min( (long) rPrinterOptions.GetReducedBitmapResolution(), mnMaxBmpDPIY );
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

    maCurPageMetaFile = GDIMetaFile();
    GetPreparedMetaFile( *pPage->mpMtf, maCurPageMetaFile, mnMaxBmpDPIX, mnMaxBmpDPIY );
}

void ImplQPrinter::PostPrintPage()
{
    SetDrawMode( mnRestoreDrawMode );
}

// -----------------------------------------------------------------------

void ImplQPrinter::PrintPage( unsigned int nPage )
{
    if( nPage >= maQueue.size() )
        return;
    mnCurCopyCount = (mbUserCopy && !mbCollateCopy) ? mnCopyCount : 1;
    QueuePage* pActPage = maQueue[nPage];
    PrePrintPage( pActPage );
    if ( pActPage->mpSetup )
        SetJobSetup( *pActPage->mpSetup );

    StartPage();
    ImplPrintMtf( maCurPageMetaFile, mnMaxBmpDPIX, mnMaxBmpDPIY );
    EndPage();

    mnCurCopyCount--;
    if( mnCurCopyCount == 0 )
        PostPrintPage();
}

// -----------------------------------------------------------------------

ULONG ImplQPrinter::GetPrintPageCount()
{
    ULONG nPageCount = maQueue.size() * ((mbUserCopy && !mbCollateCopy) ? mnCopyCount : 1);
    return nPageCount;
}

// -----------------------------------------------------------------------

IMPL_LINK( ImplQPrinter, ImplPrintHdl, Timer*, EMPTYARG )
{
    // Ist Drucken abgebrochen wurden?
    if( !IsPrinting() || ( mpParent->IsJobActive() && ( maQueue.size() < (ULONG)mpParent->GetPageQueueSize() ) ) )
        return 0;

    // Druck-Job zuende?
    QueuePage* pActPage = maQueue.front();
    maQueue.erase( maQueue.begin() );


    vcl::DeletionListener aDel( this );
    if ( pActPage->mbEndJob )
    {
        maTimer.Stop();
        delete pActPage;
        if( ! EndJob() )
            mpParent->Error();
        if( ! aDel.isDeleted() )
            mpParent->ImplEndPrint();
    }
    else
    {
        mbDestroyAllowed = FALSE;

        PrePrintPage( pActPage );

        USHORT nCopyCount = 1;
        if( mbUserCopy && !mbCollateCopy )
            nCopyCount = mnCopyCount;

        for ( USHORT i = 0; i < nCopyCount; i++ )
        {
            if ( pActPage->mpSetup )
            {
                SetJobSetup( *pActPage->mpSetup );
                if ( mbAborted )
                     break;
            }

            StartPage();

            if ( mbAborted )
                break;

            ImplPrintMtf( maCurPageMetaFile, mnMaxBmpDPIX, mnMaxBmpDPIY );

            if( !mbAborted )
                EndPage();
            else
                break;
        }

        PostPrintPage();

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
    if( ! ImplGetSVData()->maGDIData.mbPrinterPullModel )
    {
        maTimer.SetTimeout( 50 );
        maTimer.SetTimeoutHdl( LINK( this, ImplQPrinter, ImplPrintHdl ) );
        maTimer.Start();
    }
}

// -----------------------------------------------------------------------

void ImplQPrinter::EndQueuePrint()
{
    if( ImplGetSVData()->maGDIData.mbPrinterPullModel )
    {
        DBG_ASSERT( mpPrinter, "no SalPrinter in ImplQPrinter" );
        if( mpPrinter )
        {
            mpPrinter->StartJob( mbPrintFile ? &maPrintFile : NULL,
                                 Application::GetDisplayName(),
                                 maJobSetup.ImplGetConstData(),
                                 this );
            EndJob();
            mpParent->ImplEndPrint();
        }
    }
    else
    {
        QueuePage* pQueuePage   = new QueuePage;
        pQueuePage->mbEndJob    = TRUE;
        maQueue.push_back( pQueuePage );
    }
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
    maQueue.push_back( pQueuePage );
}
