/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: $
 * $Revision: $
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
#include "precompiled_sd.hxx"

#include "sddll.hxx"

#include <com/sun/star/frame/XFrame.hpp>
#include <sfx2/imagemgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/app.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>

#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/overlay/overlayanimatedbitmapex.hxx>
#include <svx/sdr/overlay/overlaybitmapex.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svxids.hrc>

#include "view/viewoverlaymanager.hxx"

#include "DrawDocShell.hxx"
#include "DrawViewShell.hxx"
#include "DrawController.hxx"
#include "glob.hrc"
#include "sdresid.hxx"
#include "EventMultiplexer.hxx"
#include "ViewShellManager.hxx"
#include "helpids.h"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "smarttag.hxx"

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace sd {

class ImageButtonHdl;

const sal_uInt32 SMART_TAG_HDL_NUM = SAL_MAX_UINT32;

class ChangePlaceholderTag : public SmartTag
{
    friend class ImageButtonHdl;
public:
    ChangePlaceholderTag( ViewOverlayManager& rManager, ::sd::View& rView, SdrObject& rPlaceholderObj );
    virtual ~ChangePlaceholderTag();

    /** returns true if the SmartTag handled the event. */
    virtual bool MouseButtonDown( const MouseEvent&, SmartHdl& );

    /** returns true if the SmartTag consumes this event. */
    virtual bool KeyInput( const KeyEvent& rKEvt );

    /** returns true if the SmartTag consumes this event. */
    virtual bool RequestHelp( const HelpEvent& rHEvt );

    /** returns true if the SmartTag consumes this event. */
    virtual bool Command( const CommandEvent& rCEvt );

    /** is called once if the mouse enters this tag */
    virtual void onMouseEnter(SmartHdl& rHdl);

    /** is called once if the mouse leaves this tag */
    virtual void onMouseLeave();

protected:
    virtual void addCustomHandles( SdrHdlList& rHandlerList );
    virtual void disposing();
    virtual void select();
    virtual void deselect();

private:
    ViewOverlayManager& mrManager;
    SdrObjectWeakRef    mxPlaceholderObj;
    ImageButtonHdl*     mpSelectedHdl;
};

class ImageButtonHdl : public SmartHdl
{
public:
    ImageButtonHdl( const SmartTagReference& xTag, const Image& rImage, const Point& rPnt );
    virtual ~ImageButtonHdl();
    virtual void CreateB2dIAObject();
    virtual BOOL IsFocusHdl() const;
    virtual Pointer GetSdrDragPointer() const;
    virtual bool isMarkable() const;

    void select( bool bSelect );
private:
    rtl::Reference< ChangePlaceholderTag > mxTag;
    bool mbSelected;
    Image                           maImage;
};

// --------------------------------------------------------------------

ImageButtonHdl::ImageButtonHdl( const SmartTagReference& xTag, const Image& rImage, const Point& rPnt )
: SmartHdl( xTag, rPnt )
, mxTag( dynamic_cast< ChangePlaceholderTag* >( xTag.get() ) )
, mbSelected(false)
, maImage( rImage )
{
}

// --------------------------------------------------------------------

ImageButtonHdl::~ImageButtonHdl()
{
    if( mxTag.is() && (mxTag->mpSelectedHdl == this) )
        mxTag->mpSelectedHdl = 0;
}

// --------------------------------------------------------------------

void ImageButtonHdl::select( bool bSelect )
{
    if( bSelect != mbSelected )
    {
        mbSelected = bSelect;
        Touch();
    }
}

// --------------------------------------------------------------------

void ImageButtonHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    const Point aTagPos( GetPos() );
    basegfx::B2DPoint aPosition( aTagPos.X(), aTagPos.Y() );

    const bool bFocused = IsFocusHdl() && pHdlList && (pHdlList->GetFocusHdl() == this);

    BitmapEx aBitmapEx( maImage.GetBitmapEx() );

    const double fAlpha = mbSelected ? 0.0 : 0.5;

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
                    if(rPaintWindow.OutputToWindow() && rPageWindow.GetOverlayManager() )
                    {
                        ::sdr::overlay::OverlayObject* pOverlayObject = 0;

                        // animate focused handles
                        pOverlayObject = new ::sdr::overlay::OverlayBitmapEx( aPosition, aBitmapEx, 0, 0, fAlpha );
                        rPageWindow.GetOverlayManager()->add(*pOverlayObject);
                        maOverlayGroup.append(*pOverlayObject);
                    }
                }
            }
        }
    }
}

// --------------------------------------------------------------------

BOOL ImageButtonHdl::IsFocusHdl() const
{
    return TRUE;
}

// --------------------------------------------------------------------

bool ImageButtonHdl::isMarkable() const
{
    return false;
}

// --------------------------------------------------------------------

Pointer ImageButtonHdl::GetSdrDragPointer() const
{
    return Pointer( POINTER_ARROW );
}

// ====================================================================

ChangePlaceholderTag::ChangePlaceholderTag( ViewOverlayManager& rManager, ::sd::View& rView, SdrObject& rPlaceholderObj )
: SmartTag( rView )
, mrManager( rManager )
, mxPlaceholderObj( &rPlaceholderObj )
{
}

// --------------------------------------------------------------------

ChangePlaceholderTag::~ChangePlaceholderTag()
{
}

// --------------------------------------------------------------------

/** returns true if the ChangePlaceholderTag handled the event. */
bool ChangePlaceholderTag::MouseButtonDown( const MouseEvent& rMEvt, SmartHdl& /*rHdl*/ )
{
    return false;
}

// --------------------------------------------------------------------

/** returns true if the SmartTag consumes this event. */
bool ChangePlaceholderTag::KeyInput( const KeyEvent& rKEvt )
{
    USHORT nCode = rKEvt.GetKeyCode().GetCode();
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

/** returns true if the SmartTag consumes this event. */
bool ChangePlaceholderTag::RequestHelp( const HelpEvent& /*rHEvt*/ )
{
   return false;
}

/** returns true if the SmartTag consumes this event. */
bool ChangePlaceholderTag::Command( const CommandEvent& rCEvt )
{
    return false;
}

/** is called once if the mouse enters this tag */
void ChangePlaceholderTag::onMouseEnter(SmartHdl& rHdl)
{
    ImageButtonHdl* pHdl = dynamic_cast< ImageButtonHdl* >( &rHdl );
    if( mpSelectedHdl != 0 )
    {
        if( mpSelectedHdl == pHdl )
            return;

        mpSelectedHdl->select(false);
    }

    mpSelectedHdl = pHdl;

    if( mpSelectedHdl )
    {
        mpSelectedHdl->select( true );
    }
}

/** is called once if the mouse leaves this tag */
void ChangePlaceholderTag::onMouseLeave()
{
    if( mpSelectedHdl )
    {
        mpSelectedHdl->select( false );
        mpSelectedHdl = 0;
    }
}

// --------------------------------------------------------------------

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

        Image* pImages = (nShapeSizePix > 300) ? &ViewOverlayManager::maLargeButtonImages[0] : &ViewOverlayManager::maSmallButtonImages[0];

        Size aButtonSize( pDev->PixelToLogic(pImages[0].GetSizePixel()) );
        if( 200 > nShapeSizePix )
        {
            aButtonSize.Width() >>= 1; aButtonSize.Height() >>= 1;
        }

        const long nBorderPix = (nShapeSizePix > 300) ? 4 : (nShapeSizePix > 100) ? 2 : 1;
        Size aBorder( pDev->PixelToLogic(Size(nBorderPix,nBorderPix)) );

        const int nColumns = (ViewOverlayManager::ButtonCount+1) >> 1;
        const int nRows = (ViewOverlayManager::ButtonCount + nColumns - 1) / nColumns;

        long all_width = (nColumns * aButtonSize.Width()) + ((nColumns-1) * aBorder.Width());
        long all_height = (nRows * aButtonSize.Height()) + ((nRows-1) * aBorder.Height());

        Point aPos( rSnapRect.Center() );
        aPos.X() -= all_width >> 1;
        aPos.Y() -= all_height >> 1;

        long nStartX = aPos.X();

        for( int i = 0, c = 0; i < ViewOverlayManager::ButtonCount; i++ )
        {
            Image aImg( pImages[i] );
            if( 200 > nShapeSizePix )
            {
                BitmapEx b( aImg.GetBitmapEx() );
                const double scale = 0.5;
                b.Scale( scale, scale );
                aImg = Image(b);
            }

            ImageButtonHdl* pHdl = new ImageButtonHdl( xThis, aImg, aPoint );
            pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
            pHdl->SetPageView( mrView.GetSdrPageView() );

            pHdl->SetPos( aPos );

            rHandlerList.AddHdl( pHdl );

            if( ++c == nColumns )
            {
                aPos.X() = nStartX;
                aPos.Y() += aButtonSize.Height() + aBorder.Height();
                c = 0;
            }
            else
            {
                aPos.X() += aButtonSize.Width() + aBorder.Width();
            }
        }
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

Image ViewOverlayManager::maSmallButtonImages[ViewOverlayManager::ButtonCount];
Image ViewOverlayManager::maLargeButtonImages[ViewOverlayManager::ButtonCount];
USHORT ViewOverlayManager::mnButtonSlots[ViewOverlayManager::ButtonCount] = { SID_INSERT_TABLE, SID_INSERT_DIAGRAM, SID_INSERT_GRAPHIC, SID_INSERT_AVMEDIA };

// --------------------------------------------------------------------

static Image lcl_getslotimage( ::com::sun::star::uno::Reference<com::sun::star::frame::XFrame>& xFrame, USHORT nSID, BOOL b1, BOOL b2 )
{
    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
    aSlotURL += rtl::OUString::valueOf( sal_Int32( nSID ));
    return ::GetImage( xFrame, aSlotURL, b1, b2 );
}

ViewOverlayManager::ViewOverlayManager( ViewShellBase& rViewShellBase )
: mrBase( rViewShellBase )
, mnUpdateTagsEvent( 0 )
{
    Link aLink( LINK(this,ViewOverlayManager,EventMultiplexerListener) );
    mrBase.GetEventMultiplexer()->AddEventListener(aLink, tools::EventMultiplexerEvent::EID_CURRENT_PAGE);

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

void ViewOverlayManager::UpdateImages()
{
    try
    {
        Reference<XFrame> xFrame(mrBase.GetFrame()->GetTopFrame()->GetFrameInterface(), UNO_QUERY_THROW);
        for( int i = 0; i < ButtonCount; i++ )
        {
            maSmallButtonImages[i] = lcl_getslotimage( xFrame, mnButtonSlots[i], FALSE, FALSE );
            maLargeButtonImages[i] = lcl_getslotimage( xFrame, mnButtonSlots[i], TRUE, FALSE );
        }
    }
    catch( Exception& )
    {
        DBG_ERROR( "ViewOverlayManager::ViewOverlayManager(), exception caught!" );
    }
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

IMPL_LINK(ViewOverlayManager,UpdateTagsHdl, void *, EMPTYARG)
{
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

    SdPage* pPage = mrBase.GetMainViewShell()->getCurrentPage();

    if( pPage )
    {
        if( !maSmallButtonImages[0] )
            UpdateImages();

        const std::list< SdrObject* >& rShapes = pPage->GetPresentationShapeList().getList();

        for( std::list< SdrObject* >::const_iterator iter( rShapes.begin() ); iter != rShapes.end(); iter++ )
        {
            if( (*iter)->IsEmptyPresObj() && ((*iter)->GetObjIdentifier() == OBJ_OUTLINETEXT) )
            {
                rtl::Reference< SmartTag > xTag( new ChangePlaceholderTag( *this, *mrBase.GetMainViewShell()->GetView(), *(*iter) ) );
                maTagVector.push_back(xTag);
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
        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
            UpdateTags();
            break;
    }
    return 0;
}

}
