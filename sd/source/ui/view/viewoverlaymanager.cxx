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
#include <svx/svdlegacy.hxx>

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace sd {

class ImageButtonHdl;

// --------------------------------------------------------------------

static sal_uInt16 gButtonSlots[] = { SID_INSERT_TABLE, SID_INSERT_DIAGRAM, SID_INSERT_GRAPHIC, SID_INSERT_AVMEDIA };
static sal_uInt16 gButtonToolTips[] = { STR_INSERT_TABLE, STR_INSERT_CHART, STR_INSERT_PICTURE, STR_INSERT_MOVIE };

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
    ChangePlaceholderTag( ViewOverlayManager& rManager, ::sd::View& rView, SdrObject& rPlaceholderObj );
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
    ViewOverlayManager& mrManager;
    SdrObjectWeakRef    mxPlaceholderObj;
};

class ImageButtonHdl : public SmartHdl
{
public:
    ImageButtonHdl(
        SdrHdlList& rHdlList,
        const SmartTagReference& xTag,
        const basegfx::B2DPoint& rPnt );
    virtual bool IsFocusHdl() const;
    virtual Pointer GetPointer() const;
    virtual bool isMarkable() const;

    virtual void onMouseEnter(const MouseEvent& rMEvt);
    virtual void onMouseLeave();

    int getHighlightId() const { return mnHighlightId; }

    void HideTip();

protected:
    virtual void CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager);
    virtual ~ImageButtonHdl();

private:
    rtl::Reference< ChangePlaceholderTag > mxTag;

    int mnHighlightId;
    Size maImageSize;
    sal_uLong mnTip;
};

// --------------------------------------------------------------------

ImageButtonHdl::ImageButtonHdl(
    SdrHdlList& rHdlList,
    const SmartTagReference& xTag,
    const basegfx::B2DPoint& rPnt )
: SmartHdl( rHdlList, 0, xTag, HDL_SMARTTAG, rPnt )
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

extern ::rtl::OUString ImplRetrieveLabelFromCommand( const Reference< XFrame >& xFrame, const OUString& aCmdURL );

void ImageButtonHdl::onMouseEnter(const MouseEvent& rMEvt)
{
    int nHighlightId = 0;
    OutputDevice* pDev = mrHdlList.GetViewFromSdrHdlList().GetFirstOutputDevice();
    if( pDev == 0 )
        pDev = Application::GetDefaultDevice();

    Point aMDPos( rMEvt.GetPosPixel() );
    const basegfx::B2DPoint aPosPixel(pDev->GetViewTransformation() * getPosition());
    const Point aPointPixel(basegfx::fround(aPosPixel.getX()), basegfx::fround(aPosPixel.getY()));
    aMDPos -= aPointPixel;

    nHighlightId += aMDPos.X() > maImageSize.Width() ? 1 : 0;
    nHighlightId += aMDPos.Y() > maImageSize.Height() ? 2 : 0;

    if( mnHighlightId != nHighlightId )
    {
        HideTip();

        mnHighlightId = nHighlightId;
        SdResId aResId( gButtonToolTips[mnHighlightId] );
        aResId.SetRT( RSC_STRING );

        String aHelpText( aResId );
        Rectangle aScreenRect( aPointPixel, maImageSize );
        mnTip = Help::ShowTip( static_cast< ::Window* >( mrHdlList.GetViewFromSdrHdlList().GetFirstOutputDevice() ), aScreenRect, aHelpText, 0 ) ;
        Touch();
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

void ImageButtonHdl::CreateB2dIAObject(::sdr::overlay::OverlayManager& rOverlayManager)
{
    BitmapEx aBitmapEx( mxTag->createOverlayImage( mnHighlightId ) );
    maImageSize = aBitmapEx.GetSizePixel();
    maImageSize.Width() >>= 1;
    maImageSize.Height() >>= 1;

    ::sdr::overlay::OverlayObject* pOverlayObject = new ::sdr::overlay::OverlayBitmapEx( getPosition(), aBitmapEx, 0, 0 );

    rOverlayManager.add(*pOverlayObject);
                        maOverlayGroup.append(*pOverlayObject);
}

// --------------------------------------------------------------------

bool ImageButtonHdl::IsFocusHdl() const
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
    int nHighlightId = static_cast< ImageButtonHdl& >(rHdl).getHighlightId();
    if( nHighlightId >= 0 )
    {
        sal_uInt16 nSID = gButtonSlots[nHighlightId];

        if( mxPlaceholderObj.get() )
        {
            // mark placeholder if it is not currently marked (or if also others are marked)
            const SdrObject* pSingleSelected = mrView.getSelectedIfSingle();

            if( pSingleSelected != mxPlaceholderObj.get() )
            {
                mrView.UnmarkAllObj();
                mrView.MarkObj(*mxPlaceholderObj.get(), false);
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
        const Rectangle aSnapRect(sdr::legacy::GetSnapRect(*pPlaceholder));
        OutputDevice* pDev = mrView.GetFirstOutputDevice();
        if( pDev == 0 )
            pDev = Application::GetDefaultDevice();

        Size aShapeSizePix = pDev->LogicToPixel(aSnapRect.GetSize());
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
        const Rectangle aSnapRect(sdr::legacy::GetSnapRect(*pPlaceholder));

        OutputDevice* pDev = mrView.GetFirstOutputDevice();
        if( pDev == 0 )
            pDev = Application::GetDefaultDevice();

        const Size aShapeSizePix(pDev->LogicToPixel(aSnapRect.GetSize()));
        const long nShapeSizePix(std::min(aShapeSizePix.Width(),aShapeSizePix.Height()));

        if( 50 > nShapeSizePix )
            return;

        const bool bLarge(nShapeSizePix > 250);
        const Size aButtonSize( pDev->PixelToLogic( getButtonImage(0, bLarge )->GetSizePixel()) );

        const int nColumns = 2;
        const int nRows = 2;

        long all_width = nColumns * aButtonSize.Width();
        long all_height = nRows * aButtonSize.Height();

        Point aPos( aSnapRect.Center() );
        aPos.X() -= all_width >> 1;
        aPos.Y() -= all_height >> 1;

        ImageButtonHdl* pHdl = new ImageButtonHdl(rHandlerList, xThis, basegfx::B2DPoint(aPos.X(), aPos.Y()) );
        pHdl->SetObjHdlNum( SMART_TAG_HDL_NUM );
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

IMPL_LINK(ViewOverlayManager,UpdateTagsHdl, void *, EMPTYARG)
{
    OSL_TRACE("ViewOverlayManager::UpdateTagsHdl");

    mnUpdateTagsEvent  = 0;
    bool bChanges = DisposeTags();
    bChanges |= CreateTags();

    if( bChanges && mrBase.GetDrawView() )
        static_cast< ::sd::View* >( mrBase.GetDrawView() )->SetMarkHandles();
    return 0;
}

bool ViewOverlayManager::CreateTags()
{
    bool bChanges = false;

    SdPage* pPage = mrBase.GetMainViewShell()->getCurrentPage();

    if( pPage && !pPage->IsMasterPage() && (pPage->GetPageKind() == PK_STANDARD) )
    {
        const std::list< const SdrObject* >& rShapes = pPage->GetPresentationShapeList().getList();

        for( std::list< const SdrObject* >::const_iterator iter( rShapes.begin() ); iter != rShapes.end(); iter++ )
        {
            if( (*iter)->IsEmptyPresObj() && ((*iter)->GetObjIdentifier() == OBJ_OUTLINETEXT) && (mrBase.GetDrawView()->GetTextEditObject() != (*iter)) )
            {
                rtl::Reference< SmartTag > xTag(
                    new ChangePlaceholderTag(
                        *this, *mrBase.GetMainViewShell()->GetView(), *const_cast< SdrObject* >(*iter) ) );
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
