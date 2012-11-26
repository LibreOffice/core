/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include "slideshow.hxx"
#include <sfx2/objsh.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <fadedef.h>
#include <vcl/ctrl.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdorect.hxx>

#include "docprev.hxx"
#include "drawdoc.hxx"
#include "DrawDocShell.hxx"
#include "ViewShell.hxx"
#include "ViewShellBase.hxx"
#include "drawview.hxx"
#include "sdpage.hxx"
#include "sfx2/viewfrm.hxx"
#include <vcl/svapp.hxx>

#include <memory>

using ::com::sun::star::drawing::XDrawPage;
using ::com::sun::star::animations::XAnimationNode;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

const int SdDocPreviewWin::FRAME = 4;

void SdDocPreviewWin::SetObjectShell( SfxObjectShell* pObj, sal_uInt32 nShowPage )
{
    mpObj = pObj;
    mnShowPage = nShowPage;
    if (mxSlideShow.is())
    {
        mxSlideShow->end();
        mxSlideShow.clear();
    }
    updateViewSettings();
}

SdDocPreviewWin::SdDocPreviewWin( Window* pParent, const ResId& rResId )
: Control(pParent, rResId), pMetaFile( 0 ), bInEffect(false), mpObj(NULL), mnShowPage(0)
{
    SetBorderStyle( WINDOW_BORDER_MONO );
    svtools::ColorConfig aColorConfig;
    Wallpaper aEmpty;
    SetBackground( aEmpty );
}

SdDocPreviewWin::~SdDocPreviewWin()
{
    delete pMetaFile;
}

void SdDocPreviewWin::Resize()
{
    Invalidate();
    if( mxSlideShow.is() )
        mxSlideShow->resize( GetSizePixel() );
}

void SdDocPreviewWin::CalcSizeAndPos( GDIMetaFile* pFile, Size& rSize, Point& rPoint )
{
    Size aTmpSize = pFile ? pFile->GetPrefSize() : Size(1,1 );
    long nWidth = rSize.Width() - 2*FRAME;
    long nHeight = rSize.Height() - 2*FRAME;
    if( nWidth < 0 ) nWidth = 0;
    if( nHeight < 0 ) nHeight = 0;

    double dRatio=((double)aTmpSize.Width())/aTmpSize.Height();
    double dRatioPreV=((double) nWidth ) / nHeight;

    if (dRatio>dRatioPreV)
    {
        rSize=Size(nWidth, (sal_uInt16)(nWidth/dRatio));
        rPoint=Point( 0, (sal_uInt16)((nHeight-rSize.Height())/2));
    }
    else
    {
        rSize=Size((sal_uInt16)(nHeight*dRatio), nHeight);
        rPoint=Point((sal_uInt16)((nWidth-rSize.Width())/2),0);
    }
}

void SdDocPreviewWin::ImpPaint( GDIMetaFile* pFile, OutputDevice* pVDev )
{
    Point aPoint;
    Size aSize = pVDev->GetOutputSize();
    Point bPoint(aSize.Width()-2*FRAME, aSize.Height()-2*FRAME );
    CalcSizeAndPos( pFile, aSize, aPoint );
    bPoint -= aPoint;
    aPoint += Point( FRAME, FRAME );

    svtools::ColorConfig aColorConfig;

    pVDev->SetLineColor();
    pVDev->SetFillColor( Color( aColorConfig.GetColorValue( svtools::APPBACKGROUND ).nColor ) );
    pVDev->DrawRect(Rectangle( Point(0,0 ), pVDev->GetOutputSize()));
    if( pFile )
    {
        pVDev->SetFillColor( maDocumentColor );
        pVDev->DrawRect(Rectangle(aPoint, aSize));
        pFile->WindStart();
        pFile->Play( pVDev, aPoint, aSize  );
    }
}

void SdDocPreviewWin::Paint( const Rectangle& rRect )
{
    if( (!mxSlideShow.is()) || (!mxSlideShow->isRunning() ) )
    {
        SvtAccessibilityOptions aAccOptions;
        bool bUseContrast = aAccOptions.GetIsForPagePreviews() && Application::GetSettings().GetStyleSettings().GetHighContrastMode();
        SetDrawMode( bUseContrast
            ? SD_OUTPUT_DRAWMODE_CONTRAST
            : SD_OUTPUT_DRAWMODE_COLOR );

        ImpPaint( pMetaFile, (VirtualDevice*)this );
    }
    else
    {
        mxSlideShow->paint( rRect );
    }
}

void SdDocPreviewWin::startPreview()
{
    ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell * >( mpObj );
    if( mpObj )
    {
        SdDrawDocument* pDoc = pDocShell->GetDoc();

        if( pDoc )
        {
            SdPage* pPage = pDoc->GetSdPage( mnShowPage, PK_STANDARD );

            if( pPage && (pPage->getTransitionType() != 0) )
            {
                if( !mxSlideShow.is() )
                    mxSlideShow = sd::SlideShow::Create( pDoc );

                Reference< XDrawPage > xDrawPage( pPage->getUnoPage(), UNO_QUERY );
                Reference< XAnimationNode > xAnimationNode;

                mxSlideShow->startPreview( xDrawPage, xAnimationNode, this );
            }
        }
    }
}

long SdDocPreviewWin::Notify( NotifyEvent& rNEvt )
{
    if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
        if ( pMEvt->IsLeft() )
        {
            if( rNEvt.GetWindow() == this )
            {
                if(aClickHdl.IsSet())
                    aClickHdl.Call(this);
            }
        }
    }

    return Control::Notify( rNEvt );
}


void SdDocPreviewWin::updateViewSettings()
{
    ::sd::DrawDocShell* pDocShell = dynamic_cast< ::sd::DrawDocShell* >(mpObj);
    SdDrawDocument* pDoc = pDocShell?pDocShell->GetDoc():NULL;

    SvtAccessibilityOptions aAccOptions;
    bool bUseWhiteColor = !aAccOptions.GetIsForPagePreviews() && GetSettings().GetStyleSettings().GetHighContrastMode();
    if( bUseWhiteColor )
    {
        maDocumentColor = Color( COL_WHITE );
    }
    else
    {
        svtools::ColorConfig aColorConfig;
        maDocumentColor = Color( aColorConfig.GetColorValue( svtools::DOCCOLOR ).nColor );
    }

    GDIMetaFile* pMtf = NULL;

    if(pDoc)
    {
        SdPage * pPage = pDoc->GetSdPage( mnShowPage, PK_STANDARD );
        if( pPage )
        {
            SdrOutliner& rOutl = pDoc->GetDrawOutliner();
            Color aOldBackgroundColor = rOutl.GetBackgroundColor();
            rOutl.SetBackgroundColor( maDocumentColor );

            pMtf = new GDIMetaFile;

            VirtualDevice       aVDev;

            const Fraction      aFrac( pDoc->GetExchangeObjectScale() );
            const MapMode       aMap( pDoc->GetExchangeObjectUnit(), Point(), aFrac, aFrac );

            aVDev.SetMapMode( aMap );

            // #109058# Disable output, as we only want to record a metafile
            aVDev.EnableOutput( false );

            pMtf->Record( &aVDev );

            ::sd::DrawView* pView = new ::sd::DrawView(pDocShell, this, NULL);

            pView->SetBordVisible( false );
            pView->SetPageVisible( false );
            pView->ShowSdrPage( *pPage );

            const Point aNewOrg( pPage->GetLeftPageBorder(), pPage->GetTopPageBorder() );
            const Size aSize(basegfx::fround(pPage->GetPageScale().getX()), basegfx::fround(pPage->GetPageScale().getY()));
            const Size aNewSize( aSize.Width() - pPage->GetLeftPageBorder() - pPage->GetRightPageBorder(),
                                  aSize.Height() - pPage->GetTopPageBorder() - pPage->GetBottomPageBorder() );
            const Rectangle aClipRect( aNewOrg, aNewSize );
            MapMode         aVMap( aMap );

            aVDev.Push();
            aVMap.SetOrigin( Point( -aNewOrg.X(), -aNewOrg.Y() ) );
            aVDev.SetRelativeMapMode( aVMap );
            aVDev.IntersectClipRegion( aClipRect );

        // Use new StandardCheckVisisbilityRedirector
        StandardCheckVisisbilityRedirector aRedirector;
        const Rectangle aRedrawRectangle = Rectangle( Point(), aNewSize );
        Region aRedrawRegion(aRedrawRectangle);
        pView->SdrPaintView::CompleteRedraw(&aVDev,aRedrawRegion,&aRedirector);

            aVDev.Pop();

            pMtf->Stop();
            pMtf->WindStart();
            pMtf->SetPrefMapMode( aMap );
            pMtf->SetPrefSize( aNewSize );

            rOutl.SetBackgroundColor( aOldBackgroundColor );

            delete pView;
        }
    }

    delete pMetaFile;
    pMetaFile = pMtf;

    Invalidate();
}

void SdDocPreviewWin::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    const SfxSimpleHint* pSfxSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);

    if( pSfxSimpleHint && SFX_HINT_COLORS_CHANGED == pSfxSimpleHint->GetId() )
    {
        updateViewSettings();
    }
}
void SdDocPreviewWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        updateViewSettings();
    }
}
