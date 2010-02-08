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

#include <tools/rcid.h>

#include <vcl/help.hxx>
#include <vcl/imagerepository.hxx>

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

// --------------------------------------------------------------------

Image ViewOverlayManager::maSmallButtonImages[ViewOverlayManager::ButtonCount];
Image ViewOverlayManager::maLargeButtonImages[ViewOverlayManager::ButtonCount];

static USHORT gButtonSlots[ViewOverlayManager::ButtonCount] = { SID_INSERT_TABLE, SID_INSERT_DIAGRAM, SID_INSERT_GRAPHIC, SID_INSERT_AVMEDIA };
static USHORT gButtonResId[ViewOverlayManager::ButtonCount] = { BMP_PLACEHOLDER_TABLE_57X71, BMP_PLACEHOLDER_CHART_57X71, BMP_PLACEHOLDER_IMAGE_57X71, BMP_PLACEHOLDER_MOVIE_57X71 };
//static USHORT gButtonResIdHc[ViewOverlayManager::ButtonCount] = { BMP_PLACEHOLDER_TABLE_57X71_H, BMP_PLACEHOLDER_CHART_57X71_H, BMP_PLACEHOLDER_IMAGE_57X71_H, BMP_PLACEHOLDER_MOVIE_57X71_H };

// --------------------------------------------------------------------

static Image loadImageResource( USHORT nId )
{
    /*
    OUString sURL( OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/sd/bitmap/" ) ) );
    sURL += OUString::valueOf( (sal_Int32)nId );

    BitmapEx aBmpEx;
    vcl::ImageRepository::loadImage( sURL, aBmpEx, false );
    return Image( aBmpEx );
    */

    SdResId aResId( nId );
    aResId.SetRT( RSC_BITMAP );

    return Image( BitmapEx( aResId ) );
}

// --------------------------------------------------------------------

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

protected:
    virtual void addCustomHandles( SdrHdlList& rHandlerList );
    virtual void disposing();
    virtual void select();
    virtual void deselect();

private:
    ViewOverlayManager& mrManager;
    SdrObjectWeakRef    mxPlaceholderObj;
};

class ImageButtonHdl : public SmartHdl
{
public:
    ImageButtonHdl( const SmartTagReference& xTag, USHORT nSID, const Image& rImage, const Point& rPnt );
    virtual ~ImageButtonHdl();
    virtual void CreateB2dIAObject();
    virtual BOOL IsFocusHdl() const;
    virtual Pointer GetPointer() const;
    virtual bool isMarkable() const;

    virtual void onMouseEnter();
    virtual void onMouseLeave();

    USHORT getSlotId() const { return mnSID; }

private:
    rtl::Reference< ChangePlaceholderTag > mxTag;
    Image                           maImage;
    USHORT mnSID;
};

// --------------------------------------------------------------------

ImageButtonHdl::ImageButtonHdl( const SmartTagReference& xTag, USHORT nSID, const Image& rImage, const Point& rPnt )
: SmartHdl( xTag, rPnt )
, mxTag( dynamic_cast< ChangePlaceholderTag* >( xTag.get() ) )
, maImage( rImage )
, mnSID( nSID )
{
}

// --------------------------------------------------------------------

ImageButtonHdl::~ImageButtonHdl()
{
}

// --------------------------------------------------------------------

void ImageButtonHdl::onMouseEnter()
{
    Touch();
}

// --------------------------------------------------------------------

void ImageButtonHdl::onMouseLeave()
{
    Touch();
}

// --------------------------------------------------------------------

void ImageButtonHdl::CreateB2dIAObject()
{
    // first throw away old one
    GetRidOfIAObject();

//  const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    const Point aTagPos( GetPos() );
    basegfx::B2DPoint aPosition( aTagPos.X(), aTagPos.Y() );

//    const bool bFocused = IsFocusHdl() && pHdlList && (pHdlList->GetFocusHdl() == this);

    BitmapEx aBitmapEx( maImage.GetBitmapEx() );

    const double fAlpha = isMouseOver() ? 0.0 : 0.6;

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
bool ChangePlaceholderTag::MouseButtonDown( const MouseEvent& /*rMEvt*/, SmartHdl& rHdl )
{
    USHORT nSID = static_cast< ImageButtonHdl& >(rHdl).getSlotId();

    if( mxPlaceholderObj.get() )
    {
        SdrPageView* pPV = mrView.GetSdrPageView();
        mrView.MarkObj(mxPlaceholderObj.get(), pPV, FALSE);
    }

    mrView.GetViewShell()->GetViewFrame()->GetDispatcher()->Execute( nSID, SFX_CALLMODE_ASYNCHRON);

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
bool ChangePlaceholderTag::RequestHelp( const HelpEvent& rHEvt )
{
    Rectangle aItemRect( rHEvt.GetMousePosPixel(), Size(1,1) );
    String aHelpText(RTL_CONSTASCII_USTRINGPARAM("I'm a help text"));
    if( rHEvt.GetMode() == HELPMODE_BALLOON )
        Help::ShowBalloon( static_cast< ::Window* >(mrView.GetFirstOutputDevice()), aItemRect.Center(), aItemRect, aHelpText);
    else
        Help::ShowQuickHelp( static_cast< ::Window* >(mrView.GetFirstOutputDevice()), aItemRect, aHelpText );

    return true;
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
        if( 50 > nShapeSizePix )
            return;

        Image* pImages = &ViewOverlayManager::maSmallButtonImages[0]; //(nShapeSizePix > 300) ? &ViewOverlayManager::maLargeButtonImages[0] : &ViewOverlayManager::maSmallButtonImages[0];

        Size aButtonSize( pDev->PixelToLogic(pImages[0].GetSizePixel()) );
        if( 100 > nShapeSizePix )
        {
            aButtonSize.Width() >>= 1; aButtonSize.Height() >>= 1;
        }

        const long nBorderPix = (nShapeSizePix > 100) ? 4 : (nShapeSizePix > 50) ? 2 : 1;
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
            if( 100 > nShapeSizePix )
            {
                BitmapEx b( aImg.GetBitmapEx() );
                const double scale = 0.5;
                b.Scale( scale, scale );
                aImg = Image(b);
            }

            ImageButtonHdl* pHdl = new ImageButtonHdl( xThis, gButtonSlots[i], aImg, aPoint );
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
/*
static Image lcl_getslotimage( ::com::sun::star::uno::Reference<com::sun::star::frame::XFrame>& xFrame, USHORT nSID, BOOL b1, BOOL b2 )
{
    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
    aSlotURL += rtl::OUString::valueOf( sal_Int32( nSID ));
    return ::GetImage( xFrame, aSlotURL, b1, b2 );
}
*/
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
            maSmallButtonImages[i] = loadImageResource( gButtonResId[i] );
//          maLargeButtonImages[i] = lcl_getslotimage( xFrame, mnButtonSlots[i], TRUE, FALSE );
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

    SdPage* pPage = mrBase.GetMainViewShell()->getCurrentPage();

    if( pPage )
    {
        if( !maSmallButtonImages[0] )
            UpdateImages();

        const std::list< SdrObject* >& rShapes = pPage->GetPresentationShapeList().getList();

        for( std::list< SdrObject* >::const_iterator iter( rShapes.begin() ); iter != rShapes.end(); iter++ )
        {
            if( (*iter)->IsEmptyPresObj() && ((*iter)->GetObjIdentifier() == OBJ_OUTLINETEXT) && !static_cast<SdrTextObj*>((*iter))->HasEditText() )
            {
                rtl::Reference< SmartTag > xTag( new ChangePlaceholderTag( *this, *mrBase.GetMainViewShell()->GetView(), *(*iter) ) );
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
        case tools::EventMultiplexerEvent::EID_CURRENT_PAGE:
            UpdateTags();
            break;
    }
    return 0;
}

}
