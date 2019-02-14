/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sddll.hxx>

#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <vcl/help.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/ptrstyle.hxx>

#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/svxids.hrc>

#include <view/viewoverlaymanager.hxx>


#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <DrawController.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <sdresid.hxx>
#include <EventMultiplexer.hxx>
#include <ViewShellBase.hxx>
#include <ViewShellManager.hxx>
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <smarttag.hxx>

using namespace ::com::sun::star::uno;

namespace sd {

class ImageButtonHdl;

static const sal_uInt16 gButtonSlots[] = { SID_INSERT_TABLE, SID_INSERT_DIAGRAM, SID_INSERT_GRAPHIC, SID_INSERT_AVMEDIA };
static const char* gButtonToolTips[] = { STR_INSERT_TABLE, STR_INSERT_CHART, STR_INSERT_PICTURE, STR_INSERT_MOVIE };

static const OUStringLiteral aSmallPlaceHolders[] =
{
    BMP_PLACEHOLDER_TABLE_SMALL,
    BMP_PLACEHOLDER_CHART_SMALL,
    BMP_PLACEHOLDER_IMAGE_SMALL,
    BMP_PLACEHOLDER_MOVIE_SMALL,
    BMP_PLACEHOLDER_TABLE_SMALL_HOVER,
    BMP_PLACEHOLDER_CHART_SMALL_HOVER,
    BMP_PLACEHOLDER_IMAGE_SMALL_HOVER,
    BMP_PLACEHOLDER_MOVIE_SMALL_HOVER
};

static const OUStringLiteral aBigPlaceHolders[] =
{
    BMP_PLACEHOLDER_TABLE_LARGE,
    BMP_PLACEHOLDER_CHART_LARGE,
    BMP_PLACEHOLDER_IMAGE_LARGE,
    BMP_PLACEHOLDER_MOVIE_LARGE,
    BMP_PLACEHOLDER_TABLE_LARGE_HOVER,
    BMP_PLACEHOLDER_CHART_LARGE_HOVER,
    BMP_PLACEHOLDER_IMAGE_LARGE_HOVER,
    BMP_PLACEHOLDER_MOVIE_LARGE_HOVER
};

static BitmapEx* getButtonImage( int index, bool large )
{
    static vcl::DeleteOnDeinit< BitmapEx > gSmallButtonImages[SAL_N_ELEMENTS(aSmallPlaceHolders)] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    static vcl::DeleteOnDeinit< BitmapEx > gLargeButtonImages[SAL_N_ELEMENTS(aBigPlaceHolders)] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    assert(SAL_N_ELEMENTS(aSmallPlaceHolders) == SAL_N_ELEMENTS(aBigPlaceHolders));

    if( !gSmallButtonImages[0].get() )
    {
        for (size_t i = 0; i < SAL_N_ELEMENTS(aSmallPlaceHolders); i++ )
        {
            gSmallButtonImages[i].set(std::make_unique<BitmapEx>(aSmallPlaceHolders[i]));
            gLargeButtonImages[i].set(std::make_unique<BitmapEx>(aBigPlaceHolders[i]));
        }
    }

    if( large )
    {
        return gLargeButtonImages[index].get();
    }
    else
    {
        return gSmallButtonImages[index].get();
    }
}

const sal_uInt32 SMART_TAG_HDL_NUM = SAL_MAX_UINT32;

class ChangePlaceholderTag : public SmartTag
{
    friend class ImageButtonHdl;
public:
    ChangePlaceholderTag( ::sd::View& rView, SdrObject& rPlaceholderObj );

    /** returns true if the SmartTag handled the event. */
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& ) override;

    /** returns true if the SmartTag consumes this event. */
    virtual bool KeyInput( const KeyEvent& rKEvt ) override;

    BitmapEx createOverlayImage( int nHighlight );

protected:
    virtual void addCustomHandles( SdrHdlList& rHandlerList ) override;

private:
    ::tools::WeakReference<SdrObject>    mxPlaceholderObj;
};

class ImageButtonHdl : public SmartHdl
{
public:
    ImageButtonHdl( const SmartTagReference& xTag, /* sal_uInt16 nSID, const Image& rImage, const Image& rImageMO, */ const Point& rPnt );
    virtual ~ImageButtonHdl() override;
    virtual void CreateB2dIAObject() override;
    virtual bool IsFocusHdl() const override;
    virtual PointerStyle GetPointer() const override;
    virtual bool isMarkable() const override;

    virtual void onMouseEnter(const MouseEvent& rMEvt) override;
    virtual void onHelpRequest(const HelpEvent& rHEvt) override;
    virtual void onMouseLeave() override;

    int getHighlightId() const { return mnHighlightId; }

    void ShowTip();
    static void HideTip();

private:
    rtl::Reference< ChangePlaceholderTag > mxChangePlaceholderTag;

    int mnHighlightId;
    Size maImageSize;
};

ImageButtonHdl::ImageButtonHdl( const SmartTagReference& xTag /*, sal_uInt16 nSID, const Image& rImage, const Image& rImageMO*/, const Point& rPnt )
: SmartHdl( xTag, rPnt, SdrHdlKind::SmartTag )
, mxChangePlaceholderTag( dynamic_cast< ChangePlaceholderTag* >( xTag.get() ) )
, mnHighlightId( -1 )
, maImageSize( 42, 42 )
{
}

ImageButtonHdl::~ImageButtonHdl()
{
    HideTip();
}

void ImageButtonHdl::HideTip()
{
    Help::HideBalloonAndQuickHelp();
}

void ImageButtonHdl::ShowTip()
{
    if (!pHdlList || !pHdlList->GetView() || mnHighlightId == -1)
        return;

    OutputDevice* pDev = pHdlList->GetView()->GetFirstOutputDevice();
    if( pDev == nullptr )
        pDev = Application::GetDefaultDevice();

    OUString aHelpText(SdResId(gButtonToolTips[mnHighlightId]));
    Point aHelpPos(pDev->LogicToPixel(GetPos()));
    if (mnHighlightId == 1)
        aHelpPos.Move(maImageSize.Width(), 0);
    else if (mnHighlightId == 2)
        aHelpPos.Move(0, maImageSize.Height());
    else if (mnHighlightId == 3)
        aHelpPos.Move(maImageSize.Width(), maImageSize.Height());
    ::tools::Rectangle aLogicPix(aHelpPos, maImageSize);
    vcl::Window* pWindow = static_cast<vcl::Window*>(pHdlList->GetView()->GetFirstOutputDevice());
    ::tools::Rectangle aScreenRect(pWindow->OutputToScreenPixel(aLogicPix.TopLeft()),
                                   pWindow->OutputToScreenPixel(aLogicPix.BottomRight()));
    Help::ShowQuickHelp(pWindow, aScreenRect, aHelpText);
}

void ImageButtonHdl::onHelpRequest(const HelpEvent& /*rHEvt*/)
{
    ShowTip();
}

void ImageButtonHdl::onMouseEnter(const MouseEvent& rMEvt)
{
    if( !(pHdlList && pHdlList->GetView()))
        return;

    int nHighlightId = 0;
    OutputDevice* pDev = pHdlList->GetView()->GetFirstOutputDevice();
    if( pDev == nullptr )
        pDev = Application::GetDefaultDevice();

    Point aMDPos( rMEvt.GetPosPixel() );
    aMDPos -= pDev->LogicToPixel( GetPos() );

    nHighlightId += aMDPos.X() > maImageSize.Width() ? 1 : 0;
    nHighlightId += aMDPos.Y() > maImageSize.Height() ? 2 : 0;

    if( mnHighlightId != nHighlightId )
    {
        HideTip();

        mnHighlightId = nHighlightId;

        ShowTip();

        Touch();
    }
}

void ImageButtonHdl::onMouseLeave()
{
    mnHighlightId = -1;
    HideTip();
    Touch();
}

void ImageButtonHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    const Point aTagPos( GetPos() );
    basegfx::B2DPoint aPosition( aTagPos.X(), aTagPos.Y() );

    BitmapEx aBitmapEx( mxChangePlaceholderTag->createOverlayImage( mnHighlightId ) ); // maImageMO.GetBitmapEx() : maImage.GetBitmapEx() );
    maImageSize = aBitmapEx.GetSizePixel();
    maImageSize.setWidth( maImageSize.Width() >> 1 );
    maImageSize.setHeight( maImageSize.Height() >> 1 );

    if(!pHdlList)
        return;

    SdrMarkView* pView = pHdlList->GetView();

    if(!(pView && !pView->areMarkHandlesHidden()))
        return;

    SdrPageView* pPageView = pView->GetSdrPageView();

    if(!pPageView)
        return;

    for(sal_uInt32 b = 0; b < pPageView->PageWindowCount(); b++)
    {
        const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

        SdrPaintWindow& rPaintWindow = rPageWindow.GetPaintWindow();
        const rtl::Reference< sdr::overlay::OverlayManager >& xManager = rPageWindow.GetOverlayManager();
        if(rPaintWindow.OutputToWindow() && xManager.is() )
        {
            std::unique_ptr<sdr::overlay::OverlayObject> pOverlayObject(
                new sdr::overlay::OverlayBitmapEx( aPosition, aBitmapEx, 0, 0 ));

            // OVERLAYMANAGER
            insertNewlyCreatedOverlayObjectForSdrHdl(
                std::move(pOverlayObject),
                rPageWindow.GetObjectContact(),
                *xManager);
        }
    }
}

bool ImageButtonHdl::IsFocusHdl() const
{
    return false;
}

bool ImageButtonHdl::isMarkable() const
{
    return false;
}

PointerStyle ImageButtonHdl::GetPointer() const
{
    return PointerStyle::Arrow;
}

ChangePlaceholderTag::ChangePlaceholderTag( ::sd::View& rView, SdrObject& rPlaceholderObj )
: SmartTag( rView )
, mxPlaceholderObj( &rPlaceholderObj )
{
}

/** returns true if the ChangePlaceholderTag handled the event. */
bool ChangePlaceholderTag::MouseButtonDown( const MouseEvent& /*rMEvt*/, SmartHdl& rHdl )
{
    int nHighlightId = static_cast< ImageButtonHdl& >(rHdl).getHighlightId();
    if( nHighlightId >= 0 )
    {
        sal_uInt16 nSID = gButtonSlots[nHighlightId];

        if( mxPlaceholderObj.get() )
        {
            // mark placeholder if it is not currently marked (or if also others are marked)
            if( !mrView.IsObjMarked( mxPlaceholderObj.get() ) || (mrView.GetMarkedObjectList().GetMarkCount() != 1) )
            {
                SdrPageView* pPV = mrView.GetSdrPageView();
                mrView.UnmarkAllObj(pPV );
                mrView.MarkObj(mxPlaceholderObj.get(), pPV);
            }
        }

        mrView.GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( nSID, SfxCallMode::ASYNCHRON);
    }
    return false;
}

/** returns true if the SmartTag consumes this event. */
bool ChangePlaceholderTag::KeyInput( const KeyEvent& rKEvt )
{
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    switch( nCode )
    {
    case KEY_DOWN:
    case KEY_UP:
    case KEY_LEFT:
    case KEY_RIGHT:
    case KEY_ESCAPE:
    case KEY_TAB:
    case KEY_RETURN:
       case KEY_SPACE:
    default:
        return false;
    }
}

BitmapEx ChangePlaceholderTag::createOverlayImage( int nHighlight )
{
    BitmapEx aRet;
    if( mxPlaceholderObj.is() )
    {
        SdrObject* pPlaceholder = mxPlaceholderObj.get();
        SmartTagReference xThis( this );
        const ::tools::Rectangle& rSnapRect = pPlaceholder->GetSnapRect();

        OutputDevice* pDev = mrView.GetFirstOutputDevice();
        if( pDev == nullptr )
            pDev = Application::GetDefaultDevice();

        Size aShapeSizePix = pDev->LogicToPixel(rSnapRect.GetSize());
        long nShapeSizePix = std::min(aShapeSizePix.Width(),aShapeSizePix.Height());

        bool bLarge = nShapeSizePix > 250;

        Size aSize( getButtonImage( 0, bLarge )->GetSizePixel() );

        aRet.Scale(Size(aSize.Width() << 1, aSize.Height() << 1));

        const ::tools::Rectangle aRectSrc( Point( 0, 0 ), aSize );

        aRet = *(getButtonImage((nHighlight == 0) ? 4 : 0, bLarge));
        aRet.Expand( aSize.Width(), aSize.Height(), true );

        aRet.CopyPixel( ::tools::Rectangle( Point( aSize.Width(), 0              ), aSize ), aRectSrc, getButtonImage((nHighlight == 1) ? 5 : 1, bLarge) );
        aRet.CopyPixel( ::tools::Rectangle( Point( 0,             aSize.Height() ), aSize ), aRectSrc, getButtonImage((nHighlight == 2) ? 6 : 2, bLarge) );
        aRet.CopyPixel( ::tools::Rectangle( Point( aSize.Width(), aSize.Height() ), aSize ), aRectSrc, getButtonImage((nHighlight == 3) ? 7 : 3, bLarge) );
    }

    return aRet;
}

void ChangePlaceholderTag::addCustomHandles( SdrHdlList& rHandlerList )
{
    if( !mxPlaceholderObj.is() )
        return;

    SdrObject* pPlaceholder = mxPlaceholderObj.get();
    SmartTagReference xThis( this );
    const ::tools::Rectangle& rSnapRect = pPlaceholder->GetSnapRect();
    const Point aPoint;

    OutputDevice* pDev = mrView.GetFirstOutputDevice();
    if( pDev == nullptr )
        pDev = Application::GetDefaultDevice();

    Size aShapeSizePix = pDev->LogicToPixel(rSnapRect.GetSize());
    long nShapeSizePix = std::min(aShapeSizePix.Width(),aShapeSizePix.Height());
    if( 50 > nShapeSizePix )
        return;

    bool bLarge = nShapeSizePix > 250;

    Size aButtonSize( pDev->PixelToLogic( getButtonImage(0, bLarge )->GetSizePixel()) );

    const int nColumns = 2;
    const int nRows = 2;

    long all_width = nColumns * aButtonSize.Width();
    long all_height = nRows * aButtonSize.Height();

    Point aPos( rSnapRect.Center() );
    aPos.AdjustX( -(all_width >> 1) );
    aPos.AdjustY( -(all_height >> 1) );

    std::unique_ptr<ImageButtonHdl> pHdl(new ImageButtonHdl( xThis, aPoint ));
    pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
    pHdl->SetPageView( mrView.GetSdrPageView() );

    pHdl->SetPos( aPos );

    rHandlerList.AddHdl( std::move(pHdl) );
}

ViewOverlayManager::ViewOverlayManager( ViewShellBase& rViewShellBase )
: mrBase( rViewShellBase )
, mnUpdateTagsEvent( nullptr )
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,ViewOverlayManager,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener(aLink);

    StartListening( *mrBase.GetDocShell() );
}

ViewOverlayManager::~ViewOverlayManager()
{
    Link<tools::EventMultiplexerEvent&,void> aLink( LINK(this,ViewOverlayManager,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );

    if( mnUpdateTagsEvent )
    {
        Application::RemoveUserEvent( mnUpdateTagsEvent );
        mnUpdateTagsEvent = nullptr;
    }

    DisposeTags();
}

void ViewOverlayManager::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::DocChanged)
    {
        UpdateTags();
    }
}

void ViewOverlayManager::onZoomChanged()
{
    if( !maTagVector.empty() )
    {
        UpdateTags();
    }
}

void ViewOverlayManager::UpdateTags()
{
    if( !mnUpdateTagsEvent )
        mnUpdateTagsEvent = Application::PostUserEvent( LINK( this, ViewOverlayManager, UpdateTagsHdl ) );
}

IMPL_LINK_NOARG(ViewOverlayManager, UpdateTagsHdl, void*, void)
{
    mnUpdateTagsEvent  = nullptr;
    bool bChanges = DisposeTags();
    bChanges |= CreateTags();

    if( bChanges && mrBase.GetDrawView() )
        static_cast< ::sd::View* >( mrBase.GetDrawView() )->updateHandles();
}

bool ViewOverlayManager::CreateTags()
{
    bool bChanges = false;

    std::shared_ptr<ViewShell> aMainShell = mrBase.GetMainViewShell();

    SdPage* pPage = aMainShell.get() ? aMainShell->getCurrentPage() : nullptr;

    if( pPage && !pPage->IsMasterPage() && (pPage->GetPageKind() == PageKind::Standard) )
    {
        const std::list< SdrObject* >& rShapes = pPage->GetPresentationShapeList().getList();

        for( SdrObject* pShape : rShapes )
        {
            if( pShape->IsEmptyPresObj() && (pShape->GetObjIdentifier() == OBJ_OUTLINETEXT) && (mrBase.GetDrawView()->GetTextEditObject() != pShape) )
            {
                rtl::Reference< SmartTag > xTag( new ChangePlaceholderTag( *mrBase.GetMainViewShell()->GetView(), *pShape ) );
                maTagVector.push_back(xTag);
                bChanges = true;
            }
        }
    }

    return bChanges;
}

bool ViewOverlayManager::DisposeTags()
{
    if( !maTagVector.empty() )
    {
        ViewTagVector vec;
        vec.swap( maTagVector );

        for (auto& rxViewTag : vec)
            rxViewTag->Dispose();
        return true;
    }

    return false;
}

IMPL_LINK(ViewOverlayManager,EventMultiplexerListener,
    tools::EventMultiplexerEvent&, rEvent, void)
{
    switch (rEvent.meEventId)
    {
        case EventMultiplexerEventId::MainViewAdded:
        case EventMultiplexerEventId::ViewAdded:
        case EventMultiplexerEventId::BeginTextEdit:
        case EventMultiplexerEventId::EndTextEdit:
        case EventMultiplexerEventId::CurrentPageChanged:
            UpdateTags();
            break;
        default: break;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
