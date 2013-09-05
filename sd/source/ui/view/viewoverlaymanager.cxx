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


#include "sddll.hxx"

#include <com/sun/star/frame/XFrame.hpp>
#include <sfx2/imagemgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <tools/rcid.h>

#include <vcl/help.hxx>
#include <vcl/lazydelete.hxx>

#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svxids.hrc>

#include "view/viewoverlaymanager.hxx"

#include "res_bmp.hrc"
#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "DrawController.hxx"
#include "glob.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "EventMultiplexer.hxx"
#include "ViewShellManager.hxx"
#include "helpids.h"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "smarttag.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace sd {

class ImageButtonHdl;

// --------------------------------------------------------------------

static const sal_uInt16 gButtonSlots[] = { SID_INSERT_TABLE, SID_INSERT_DIAGRAM, SID_INSERT_GRAPHIC, SID_INSERT_AVMEDIA };
static const sal_uInt16 gButtonToolTips[] = { STR_INSERT_TABLE, STR_INSERT_CHART, STR_INSERT_PICTURE, STR_INSERT_MOVIE };

// --------------------------------------------------------------------

static BitmapEx loadImageResource( sal_uInt16 nId )
{
    SdResId aResId( nId );
    aResId.SetRT( RSC_BITMAP );

    return BitmapEx( aResId );
}

// --------------------------------------------------------------------

static BitmapEx* getButtonImage( int index, bool large )
{
    static vcl::DeleteOnDeinit< BitmapEx > gSmallButtonImages[BMP_PLACEHOLDER_SMALL_END - BMP_PLACEHOLDER_SMALL_START] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    static vcl::DeleteOnDeinit< BitmapEx > gLargeButtonImages[BMP_PLACEHOLDER_LARGE_END - BMP_PLACEHOLDER_LARGE_START] = { 0, 0, 0, 0, 0, 0, 0, 0 };

    if( !gSmallButtonImages[0].get() )
    {
        for( sal_uInt16 i = 0; i < (BMP_PLACEHOLDER_SMALL_END-BMP_PLACEHOLDER_SMALL_START); i++ )
        {
            gSmallButtonImages[i].set( new BitmapEx( loadImageResource( BMP_PLACEHOLDER_SMALL_START + i ) ) );
            gLargeButtonImages[i].set( new BitmapEx( loadImageResource( BMP_PLACEHOLDER_LARGE_START + i ) ) );
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

// --------------------------------------------------------------------

const sal_uInt32 SMART_TAG_HDL_NUM = SAL_MAX_UINT32;

class ChangePlaceholderTag : public SmartTag
{
    friend class ImageButtonHdl;
public:
    ChangePlaceholderTag( ::sd::View& rView, SdrObject& rPlaceholderObj );
    virtual ~ChangePlaceholderTag();

    /** returns true if the SmartTag handled the event. */
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& );

    /** returns true if the SmartTag consumes this event. */
    virtual bool KeyInput( const KeyEvent& rKEvt );

    BitmapEx createOverlayImage( int nHighlight = -1 );

protected:
    virtual void addCustomHandles( SdrHdlList& rHandlerList );
    virtual void disposing();
    virtual void select();
    virtual void deselect();

private:
    SdrObjectWeakRef    mxPlaceholderObj;
};

class ImageButtonHdl : public SmartHdl
{
public:
    ImageButtonHdl( const SmartTagReference& xTag, /* sal_uInt16 nSID, const Image& rImage, const Image& rImageMO, */ const Point& rPnt );
    virtual ~ImageButtonHdl();
    virtual void CreateB2dIAObject();
    virtual sal_Bool IsFocusHdl() const;
    virtual Pointer GetPointer() const;
    virtual bool isMarkable() const;

    virtual void onMouseEnter(const MouseEvent& rMEvt);
    virtual void onMouseLeave();

    int getHighlightId() const { return mnHighlightId; }

    void HideTip();

private:
    rtl::Reference< ChangePlaceholderTag > mxTag;

    int mnHighlightId;
    Size maImageSize;
    sal_uLong mnTip;
};

// --------------------------------------------------------------------

ImageButtonHdl::ImageButtonHdl( const SmartTagReference& xTag /*, sal_uInt16 nSID, const Image& rImage, const Image& rImageMO*/, const Point& rPnt )
: SmartHdl( xTag, rPnt )
, mxTag( dynamic_cast< ChangePlaceholderTag* >( xTag.get() ) )
, mnHighlightId( -1 )
, maImageSize( 42, 42 )
, mnTip( 0 )
{
}

// --------------------------------------------------------------------

ImageButtonHdl::~ImageButtonHdl()
{
    HideTip();
}

// --------------------------------------------------------------------

void ImageButtonHdl::HideTip()
{
    if( mnTip )
    {
        Help::HideTip( mnTip );
        mnTip = 0;
    }
}

// --------------------------------------------------------------------

extern OUString ImplRetrieveLabelFromCommand( const Reference< XFrame >& xFrame, const OUString& aCmdURL );

void ImageButtonHdl::onMouseEnter(const MouseEvent& rMEvt)
{

    if( pHdlList && pHdlList->GetView())
    {
        int nHighlightId = 0;
        OutputDevice* pDev = pHdlList->GetView()->GetFirstOutputDevice();
        if( pDev == 0 )
            pDev = Application::GetDefaultDevice();

        Point aMDPos( rMEvt.GetPosPixel() );
        aMDPos -= pDev->LogicToPixel( GetPos() );

        nHighlightId += aMDPos.X() > maImageSize.Width() ? 1 : 0;
        nHighlightId += aMDPos.Y() > maImageSize.Height() ? 2 : 0;

        if( mnHighlightId != nHighlightId )
        {
            HideTip();

            mnHighlightId = nHighlightId;

            if( pHdlList )
            {
                SdResId aResId( gButtonToolTips[mnHighlightId] );
                aResId.SetRT( RSC_STRING );

                OUString aHelpText( aResId );
                Rectangle aScreenRect( pDev->LogicToPixel( GetPos() ), maImageSize );
                mnTip = Help::ShowTip( static_cast< ::Window* >( pHdlList->GetView()->GetFirstOutputDevice() ), aScreenRect, aHelpText, 0 ) ;
            }
            Touch();
        }
    }
}

// --------------------------------------------------------------------

void ImageButtonHdl::onMouseLeave()
{
    mnHighlightId = -1;
    HideTip();
    Touch();
}

// --------------------------------------------------------------------

void ImageButtonHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    const Point aTagPos( GetPos() );
    basegfx::B2DPoint aPosition( aTagPos.X(), aTagPos.Y() );

    BitmapEx aBitmapEx( mxTag->createOverlayImage( mnHighlightId ) ); // maImageMO.GetBitmapEx() : maImage.GetBitmapEx() );
    maImageSize = aBitmapEx.GetSizePixel();
    maImageSize.Width() >>= 1;
    maImageSize.Height() >>= 1;

    if(pHdlList)
    {
        SdrMarkView* pView = pHdlList->GetView();

        if(pView && !pView->areMarkHandlesHidden())
        {
            SdrPageView* pPageView = pView->GetSdrPageView();

            if(pPageView)
            {
                for(sal_uInt32 b = 0; b < pPageView->PageWindowCount(); b++)
                {
                    const SdrPageWindow& rPageWindow = *pPageView->GetPageWindow(b);

                    SdrPaintWindow& rPaintWindow = rPageWindow.GetPaintWindow();
                    rtl::Reference< ::sdr::overlay::OverlayManager > xManager = rPageWindow.GetOverlayManager();
                    if(rPaintWindow.OutputToWindow() && xManager.is() )
                    {
                        ::sdr::overlay::OverlayObject* pOverlayObject = 0;

                        pOverlayObject = new ::sdr::overlay::OverlayBitmapEx( aPosition, aBitmapEx, 0, 0 );
                        xManager->add(*pOverlayObject);
                        maOverlayGroup.append(*pOverlayObject);
                    }
                }
            }
        }
    }
}

// --------------------------------------------------------------------

sal_Bool ImageButtonHdl::IsFocusHdl() const
{
    return false;
}

// --------------------------------------------------------------------

bool ImageButtonHdl::isMarkable() const
{
    return false;
}

// --------------------------------------------------------------------

Pointer ImageButtonHdl::GetPointer() const
{
    return Pointer( POINTER_ARROW );
}

// ====================================================================

ChangePlaceholderTag::ChangePlaceholderTag( ::sd::View& rView, SdrObject& rPlaceholderObj )
: SmartTag( rView )
, mxPlaceholderObj( &rPlaceholderObj )
{
}

// --------------------------------------------------------------------

ChangePlaceholderTag::~ChangePlaceholderTag()
{
}

// --------------------------------------------------------------------

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
                mrView.MarkObj(mxPlaceholderObj.get(), pPV, sal_False);
            }
        }

        mrView.GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( nSID, SFX_CALLMODE_ASYNCHRON);
    }
    return false;
}

// --------------------------------------------------------------------

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

// --------------------------------------------------------------------

BitmapEx ChangePlaceholderTag::createOverlayImage( int nHighlight )
{
    BitmapEx aRet;
    if( mxPlaceholderObj.is() )
    {
        SdrObject* pPlaceholder = mxPlaceholderObj.get();
        SmartTagReference xThis( this );
        const Rectangle& rSnapRect = pPlaceholder->GetSnapRect();

        OutputDevice* pDev = mrView.GetFirstOutputDevice();
        if( pDev == 0 )
            pDev = Application::GetDefaultDevice();

        Size aShapeSizePix = pDev->LogicToPixel(rSnapRect.GetSize());
        long nShapeSizePix = std::min(aShapeSizePix.Width(),aShapeSizePix.Height());

        bool bLarge = nShapeSizePix > 250;

        Size aSize( getButtonImage( 0, bLarge )->GetSizePixel() );

        aRet.SetSizePixel( Size( aSize.Width() << 1, aSize.Height() << 1 ) );

        const Rectangle aRectSrc( Point( 0, 0 ), aSize );

        aRet = *(getButtonImage((nHighlight == 0) ? 4 : 0, bLarge));
        aRet.Expand( aSize.Width(), aSize.Height(), NULL, sal_True );

        aRet.CopyPixel( Rectangle( Point( aSize.Width(), 0              ), aSize ), aRectSrc, getButtonImage((nHighlight == 1) ? 5 : 1, bLarge) );
        aRet.CopyPixel( Rectangle( Point( 0,             aSize.Height() ), aSize ), aRectSrc, getButtonImage((nHighlight == 2) ? 6 : 2, bLarge) );
        aRet.CopyPixel( Rectangle( Point( aSize.Width(), aSize.Height() ), aSize ), aRectSrc, getButtonImage((nHighlight == 3) ? 7 : 3, bLarge) );
    }

    return aRet;
}

void ChangePlaceholderTag::addCustomHandles( SdrHdlList& rHandlerList )
{
    if( mxPlaceholderObj.is() )
    {
        SdrObject* pPlaceholder = mxPlaceholderObj.get();
        SmartTagReference xThis( this );
        const Rectangle& rSnapRect = pPlaceholder->GetSnapRect();
        const Point aPoint;

        OutputDevice* pDev = mrView.GetFirstOutputDevice();
        if( pDev == 0 )
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
        aPos.X() -= all_width >> 1;
        aPos.Y() -= all_height >> 1;

        ImageButtonHdl* pHdl = new ImageButtonHdl( xThis, aPoint );
        pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
        pHdl->SetPageView( mrView.GetSdrPageView() );

        pHdl->SetPos( aPos );

        rHandlerList.AddHdl( pHdl );
    }
}

// --------------------------------------------------------------------

void ChangePlaceholderTag::disposing()
{
    SmartTag::disposing();
}

// --------------------------------------------------------------------

void ChangePlaceholderTag::select()
{
    SmartTag::select();
}

// --------------------------------------------------------------------

void ChangePlaceholderTag::deselect()
{
    SmartTag::deselect();
}

// --------------------------------------------------------------------

ViewOverlayManager::ViewOverlayManager( ViewShellBase& rViewShellBase )
: mrBase( rViewShellBase )
, mnUpdateTagsEvent( 0 )
{
    Link aLink( LINK(this,ViewOverlayManager,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener(aLink, tools::EventMultiplexerEvent::EID_CURRENT_PAGE
        | tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED
        | tools::EventMultiplexerEvent::EID_VIEW_ADDED
        | tools::EventMultiplexerEvent::EID_BEGIN_TEXT_EDIT
        | tools::EventMultiplexerEvent::EID_END_TEXT_EDIT );

    StartListening( *mrBase.GetDocShell() );
}

// --------------------------------------------------------------------

ViewOverlayManager::~ViewOverlayManager()
{
    Link aLink( LINK(this,ViewOverlayManager,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->RemoveEventListener( aLink );

    if( mnUpdateTagsEvent )
    {
        Application::RemoveUserEvent( mnUpdateTagsEvent );
        mnUpdateTagsEvent = 0;
    }

    DisposeTags();
}

// --------------------------------------------------------------------

void ViewOverlayManager::Notify(SfxBroadcaster&, const SfxHint& rHint)
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (pSimpleHint != NULL)
    {
        if (pSimpleHint->GetId() == SFX_HINT_DOCCHANGED)
        {
            UpdateTags();
        }
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

IMPL_LINK_NOARG(ViewOverlayManager, UpdateTagsHdl)
{
    OSL_TRACE("ViewOverlayManager::UpdateTagsHdl");

    mnUpdateTagsEvent  = 0;
    bool bChanges = DisposeTags();
    bChanges |= CreateTags();

    if( bChanges && mrBase.GetDrawView() )
        static_cast< ::sd::View* >( mrBase.GetDrawView() )->updateHandles();
    return 0;
}

bool ViewOverlayManager::CreateTags()
{
    bool bChanges = false;

    ::boost::shared_ptr<ViewShell> aMainShell = mrBase.GetMainViewShell();

    SdPage* pPage = aMainShell.get() ? aMainShell->getCurrentPage() : NULL;

    if( pPage && !pPage->IsMasterPage() && (pPage->GetPageKind() == PK_STANDARD) )
    {
        const std::list< SdrObject* >& rShapes = pPage->GetPresentationShapeList().getList();

        for( std::list< SdrObject* >::const_iterator iter( rShapes.begin() ); iter != rShapes.end(); ++iter )
        {
            if( (*iter)->IsEmptyPresObj() && ((*iter)->GetObjIdentifier() == OBJ_OUTLINETEXT) && (mrBase.GetDrawView()->GetTextEditObject() != (*iter)) )
            {
                rtl::Reference< SmartTag > xTag( new ChangePlaceholderTag( *mrBase.GetMainViewShell()->GetView(), *(*iter) ) );
                maTagVector.push_back(xTag);
                bChanges = true;
            }
        }
    }

    return bChanges;
}

// --------------------------------------------------------------------

bool ViewOverlayManager::DisposeTags()
{
    if( !maTagVector.empty() )
    {
        ViewTagVector vec;
        vec.swap( maTagVector );

        ViewTagVector::iterator iter = vec.begin();
        do
        {
            (*iter++)->Dispose();
        }
        while( iter != vec.end() );
        return true;
    }

    return false;
}

// --------------------------------------------------------------------

IMPL_LINK(ViewOverlayManager,EventMultiplexerListener,
    tools::EventMultiplexerEvent*,pEvent)
{
    switch (pEvent->meEventId)
    {
        case tools::EventMultiplexerEvent::EID_MAIN_VIEW_ADDED:
        case tools::EventMultiplexerEvent::EID_VIEW_ADDED:
        case tools::EventMultiplexerEvent::EID_BEGIN_TEXT_EDIT:
        case tools::EventMultiplexerEvent::EID_END_TEXT_EDIT:
        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
            UpdateTags();
            break;
    }
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
