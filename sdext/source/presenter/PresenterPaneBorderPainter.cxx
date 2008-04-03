/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PresenterPaneBorderPainter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:01:46 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "PresenterPaneBorderPainter.hxx"
#include "PresenterConfigurationAccess.hxx"
#include "PresenterGeometryHelper.hxx"
#include "PresenterTheme.hxx"
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/SimpleFontMetric.hpp>
#include <com/sun/star/awt/XFont.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#include <com/sun/star/rendering/CompositeOperation.hpp>
#include <com/sun/star/rendering/FillRule.hpp>
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;

namespace sdext { namespace presenter {

namespace {
    class BitmapDescriptor
    {
    public:
        BitmapDescriptor(
            const ::boost::shared_ptr<PresenterTheme>& rpTheme,
            const OUString& rsStyleName,
            const OUString& rsBitmapName);

        Reference<rendering::XBitmap> mxBitmap;
        awt::Point maOffset;
        awt::Size maSize;
        util::Color maReplacementColor;
    };

    class BorderSize
    {
    public:
        BorderSize (void);
        BorderSize (const BorderSize& rBorderSize);
        BorderSize& operator= (const BorderSize& rBoderSize);
        sal_Int32 mnLeft;
        sal_Int32 mnTop;
        sal_Int32 mnRight;
        sal_Int32 mnBottom;
    };

    class RendererPaneStyle
    {
    public:
        RendererPaneStyle (
            const ::boost::shared_ptr<PresenterTheme>& rpTheme,
            const OUString& rsStyleName);

        awt::Rectangle AddBorder (
            const awt::Rectangle& rBox,
            drawing::framework::BorderType eBorderType) const;
        awt::Rectangle RemoveBorder (
            const awt::Rectangle& rBox,
            drawing::framework::BorderType eBorderType) const;
        const rendering::FontRequest& GetCanvasFontDescriptor (void) const;

        BitmapDescriptor maTopLeft;
        BitmapDescriptor maTop;
        BitmapDescriptor maTopRight;
        BitmapDescriptor maLeft;
        BitmapDescriptor maRight;
        BitmapDescriptor maBottomLeft;
        BitmapDescriptor maBottom;
        BitmapDescriptor maBottomRight;
        BitmapDescriptor maWindowBackground;
        rendering::FontRequest maCanvasFontDescriptor;
        util::Color maFontColor;
        enum Anchor { AnchorLeft, AnchorRight, AnchorCenter } meFontAnchor;
        BorderSize maInnerBorderSize;
        BorderSize maOuterBorderSize;
        BorderSize maTotalBorderSize;
        enum Side { Left, Top, Right, Bottom };
        sal_Int32 GetBorderSize (const Side eSide) const;
    private:
        void UpdateBorderSizes (void);
    };
}



class  PresenterPaneBorderPainter::Renderer
{
public:
    Renderer (
        const Reference<XComponentContext>& rxContext,
        const ::boost::shared_ptr<PresenterTheme>& rpTheme);
    ~Renderer (void);

    void SetCanvas (const Reference<rendering::XCanvas>& rxCanvas);
    void PaintBorder (
        const OUString& rsTitle,
        const awt::Rectangle& rBBox,
        const awt::Rectangle& rUpdateBox,
        const OUString& rsPaneURL,
        const double nBorderTransparency);
    void PaintTitle (
        const OUString& rsTitle,
        const ::boost::shared_ptr<RendererPaneStyle>& rpStyle,
        const awt::Rectangle& rUpdateBox,
        const awt::Rectangle& rOuterBox,
        const awt::Rectangle& rInnerBox,
        const bool bPaintBackground);
    void SetupClipping (const awt::Rectangle& rUpdateBox, bool bPaintOutline);
    ::boost::shared_ptr<RendererPaneStyle> GetRendererPaneStyle (const OUString& rsResourceURL);

private:
    ::boost::shared_ptr<PresenterTheme> mpTheme;
    typedef ::std::map<OUString, ::boost::shared_ptr<RendererPaneStyle> > RendererPaneStyleContainer;
    RendererPaneStyleContainer maRendererPaneStyles;
    Reference<awt::XFont> mxFont;
    Reference<rendering::XCanvasFont> mxCanvasFont;
    Reference<rendering::XCanvas> mxCanvas;
    css::rendering::ViewState maViewState;
    Reference<rendering::XPolyPolygon2D> mxViewStateClip;
    awt::Point maTopLeftTitleOffset;
    awt::Point maTopRightTitleOffset;
    awt::Point maTopLeftOffset;
    awt::Point maTopRightOffset;
    awt::Point maBottomLeftOffset;
    awt::Point maBottomRightOffset;

    void PaintFrameBackground (
        const awt::Rectangle& rInnerBox,
        const awt::Rectangle& rOuterBox);
    void PaintBitmap(
        const awt::Rectangle& rBox,
        const awt::Rectangle& rUpdateBox,
        const ::boost::shared_ptr<RendererPaneStyle>& rpStyle,
        const sal_Int32 nXPosition,
        const sal_Int32 nYPosition,
        const sal_Int32 nStartOffset,
        const sal_Int32 nEndOffset,
        const BitmapDescriptor& rBitmap,
        const double nBorderTransparency);
};




// ===== PresenterPaneBorderPainter ===========================================

PresenterPaneBorderPainter::PresenterPaneBorderPainter (
    const Reference<XComponentContext>& rxContext)
    : PresenterPaneBorderPainterInterfaceBase(m_aMutex),
      mxContext(rxContext),
      mpTheme(),
      mpRenderer()
{
}




PresenterPaneBorderPainter::~PresenterPaneBorderPainter (void)
{
}




//----- XPaneBorderPainter ----------------------------------------------------

awt::Rectangle SAL_CALL PresenterPaneBorderPainter::addBorder (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::awt::Rectangle& rRectangle,
    drawing::framework::BorderType eBorderType)
    throw(css::uno::RuntimeException)
{
    ThrowIfDisposed();

    ProvideTheme();

    return AddBorder(rsPaneBorderStyleName, rRectangle, eBorderType);
}




awt::Rectangle SAL_CALL PresenterPaneBorderPainter::removeBorder (
    const rtl::OUString& rsPaneBorderStyleName,
    const css::awt::Rectangle& rRectangle,
    drawing::framework::BorderType eBorderType)
    throw(css::uno::RuntimeException)
{
    ThrowIfDisposed();

    ProvideTheme();

    return RemoveBorder(rsPaneBorderStyleName, rRectangle, eBorderType);
}


void SAL_CALL PresenterPaneBorderPainter::paintBorder(
    const ::rtl::OUString& rsPaneBorderStyleName,
    const css::uno::Reference< css::rendering::XCanvas >& rxCanvas,
    const css::awt::Rectangle& rOuterBorderRectangle,
    const css::awt::Rectangle& rRepaintArea,
    const ::rtl::OUString& rsTitle )
    throw (css::uno::RuntimeException)
{
    ThrowIfDisposed();

    // Early reject paints completely outside the repaint area.
    if (rRepaintArea.X >= rOuterBorderRectangle.X+rOuterBorderRectangle.Width
        || rRepaintArea.Y >= rOuterBorderRectangle.Y+rOuterBorderRectangle.Height
        || rRepaintArea.X+rRepaintArea.Width <= rOuterBorderRectangle.X
        || rRepaintArea.Y+rRepaintArea.Height <= rOuterBorderRectangle.Y)
    {
        return;
    }
    ProvideTheme(rxCanvas);

    if (mpRenderer.get() != NULL)
    {
        mpRenderer->SetCanvas(rxCanvas);
        mpRenderer->SetupClipping(rRepaintArea, false);
        mpRenderer->PaintBorder(
            rsTitle,
            rOuterBorderRectangle,
            rRepaintArea,
            rsPaneBorderStyleName,
            0);
    }
}

void SAL_CALL PresenterPaneBorderPainter::paintBorderWithCallout(
    const ::rtl::OUString& /*sPaneBorderStyleName*/,
    const css::uno::Reference< css::rendering::XCanvas >& /*xCanvas*/,
    const css::awt::Rectangle& /*aOuterBorderRectangle*/,
    const css::awt::Rectangle& /*aRepaintArea*/,
    const ::rtl::OUString& /*sTitle*/,
    const css::awt::Point& /*aCalloutAnchor*/ )
    throw (css::uno::RuntimeException)
{
}

css::awt::Point SAL_CALL PresenterPaneBorderPainter::getCalloutOffset(
    const ::rtl::OUString& /*sPaneBorderStyleName*/ )
    throw (css::uno::RuntimeException)
{
    css::awt::Point aPoint;
    return aPoint;
}

//-----------------------------------------------------------------------------

bool PresenterPaneBorderPainter::ProvideTheme (const Reference<rendering::XCanvas>& rxCanvas)
{
    bool bModified (false);

    if ( ! mxContext.is())
        return false;

    if (mpTheme.get() != NULL)
    {
        // Check if the theme already has a canvas.
        if ( ! mpTheme->HasCanvas())
        {
            mpTheme->ProvideCanvas(rxCanvas);
            bModified = true;
        }
    }
    else
    {
        mpTheme.reset(new PresenterTheme(mxContext, OUString(), rxCanvas));
        bModified = true;
    }

    if (mpTheme.get() != NULL && bModified)
    {
        if (mpRenderer.get() == NULL)
            mpRenderer.reset(new Renderer(mxContext, mpTheme));
        else
            mpRenderer->SetCanvas(rxCanvas);
    }

    return bModified;
}




bool PresenterPaneBorderPainter::ProvideTheme (void)
{
    if (mpTheme.get() == NULL)
    {
        // Create a theme without bitmaps (no canvas => no bitmaps).
        return ProvideTheme(NULL);
    }
    else
    {
        // When there already is a theme then without a canvas we can not
        // add anything new.
        return false;
    }
}




bool PresenterPaneBorderPainter::HasTheme (void) const
{
    return mpTheme.get()!=NULL && mpRenderer.get()!=NULL;
}




void PresenterPaneBorderPainter::SetTheme (const ::boost::shared_ptr<PresenterTheme>& rpTheme)
{
    mpTheme = rpTheme;
    if (mpRenderer.get() == NULL)
        mpRenderer.reset(new Renderer(mxContext, mpTheme));
}




awt::Rectangle PresenterPaneBorderPainter::AddBorder (
    const ::rtl::OUString& rsPaneURL,
    const awt::Rectangle& rInnerBox,
    const css::drawing::framework::BorderType eBorderType) const
{
    if (mpRenderer.get() != NULL)
    {
        const ::boost::shared_ptr<RendererPaneStyle> pRendererPaneStyle(mpRenderer->GetRendererPaneStyle(rsPaneURL));
        if (pRendererPaneStyle.get() != NULL)
            return pRendererPaneStyle->AddBorder(rInnerBox, eBorderType);
    }
    return rInnerBox;
}




awt::Rectangle PresenterPaneBorderPainter::RemoveBorder (
    const ::rtl::OUString& rsPaneURL,
    const css::awt::Rectangle& rOuterBox,
    const css::drawing::framework::BorderType eBorderType) const
{
    if (mpRenderer.get() != NULL)
    {
        const ::boost::shared_ptr<RendererPaneStyle> pRendererPaneStyle(mpRenderer->GetRendererPaneStyle(rsPaneURL));
        if (pRendererPaneStyle.get() != NULL)
            return pRendererPaneStyle->RemoveBorder(rOuterBox, eBorderType);
    }
    return rOuterBox;
}




void PresenterPaneBorderPainter::ThrowIfDisposed (void) const
    throw (css::lang::DisposedException)
{
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        throw lang::DisposedException (
            OUString(RTL_CONSTASCII_USTRINGPARAM(
                "PresenterPaneBorderPainter object has already been disposed")),
            const_cast<uno::XWeak*>(static_cast<const uno::XWeak*>(this)));
    }
}




//===== PresenterPaneBorderPainter::Renderer =====================================


PresenterPaneBorderPainter::Renderer::Renderer (
    const Reference<XComponentContext>& rxContext,
    const ::boost::shared_ptr<PresenterTheme>& rpTheme)
    : mpTheme(rpTheme),
      maRendererPaneStyles(),
      mxFont(),
      mxCanvasFont(),
      mxCanvas(),
      maViewState(),
      mxViewStateClip(),
      maTopLeftTitleOffset(0,0),
      maTopRightTitleOffset(0,0),
      maTopLeftOffset(0,0),
      maTopRightOffset(0,0),
      maBottomLeftOffset(0,0),
      maBottomRightOffset(0,0)
{
    (void)rxContext;
    maViewState.AffineTransform = geometry::AffineMatrix2D(1,0,0, 0,1,0);
}




PresenterPaneBorderPainter::Renderer::~Renderer (void)
{
}




void PresenterPaneBorderPainter::Renderer::SetCanvas (const Reference<rendering::XCanvas>& rxCanvas)
{
    if (mxCanvas != rxCanvas)
    {
        mxCanvas = rxCanvas;
    }
}




void PresenterPaneBorderPainter::Renderer::PaintBorder (
    const OUString& rsTitle,
    const awt::Rectangle& rBBox,
    const awt::Rectangle& rUpdateBox,
    const OUString& rsPaneURL,
    const double nBorderTransparency)
{
    if ( ! mxCanvas.is())
        return;

    // Create the outer and inner border of the, ahm, border.
    ::boost::shared_ptr<RendererPaneStyle> pStyle (GetRendererPaneStyle(rsPaneURL));
    if (pStyle.get() == NULL)
        return;

    awt::Rectangle aOuterBox (rBBox);
    awt::Rectangle aCenterBox (
        pStyle->RemoveBorder(aOuterBox, drawing::framework::BorderType_OUTER_BORDER));
    awt::Rectangle aInnerBox (
        pStyle->RemoveBorder(aOuterBox, drawing::framework::BorderType_TOTAL_BORDER));

    //PaintTitle(rsTitle, pStyle, rUpdateBox, aOuterBox, aInnerBox, true);

    // Prepare references for all used bitmaps.
    BitmapDescriptor& rTop (pStyle->maTop);
    BitmapDescriptor& rTopLeft (pStyle->maTopLeft);
    BitmapDescriptor& rTopRight (pStyle->maTopRight);
    BitmapDescriptor& rLeft (pStyle->maLeft);
    BitmapDescriptor& rRight (pStyle->maRight);
    BitmapDescriptor& rBottomLeft (pStyle->maBottomLeft);
    BitmapDescriptor& rBottomRight (pStyle->maBottomRight);
    BitmapDescriptor& rBottom (pStyle->maBottom);

    // Paint the sides.
    PaintBitmap(aCenterBox, rUpdateBox, pStyle, 0,-1,
        rTopLeft.maOffset.X, rTopRight.maOffset.X, rTop, nBorderTransparency);
    PaintBitmap(aCenterBox, rUpdateBox, pStyle, 0,+1,
        rBottomLeft.maOffset.X, rBottomRight.maOffset.X, rBottom, nBorderTransparency);
    PaintBitmap(aCenterBox, rUpdateBox, pStyle, -1,0,
        rTopLeft.maOffset.Y, rBottomLeft.maOffset.Y, rLeft, nBorderTransparency);
    PaintBitmap(aCenterBox, rUpdateBox, pStyle, +1,0,
        rTopRight.maOffset.Y, rBottomRight.maOffset.Y, rRight, nBorderTransparency);

    // Paint the corners.
    PaintBitmap(aCenterBox, rUpdateBox, pStyle, -1,-1, 0,0, rTopLeft, nBorderTransparency);
    PaintBitmap(aCenterBox, rUpdateBox, pStyle, +1,-1, 0,0, rTopRight, nBorderTransparency);
    PaintBitmap(aCenterBox, rUpdateBox, pStyle, -1,+1, 0,0, rBottomLeft, nBorderTransparency);
    PaintBitmap(aCenterBox, rUpdateBox, pStyle, +1,+1, 0,0, rBottomRight, nBorderTransparency);

    // Paint the title.
    PaintTitle(rsTitle, pStyle, rUpdateBox, aOuterBox, aInnerBox, false);

    // In a double buffering environment request to make the changes visible.
    Reference<rendering::XSpriteCanvas> xSpriteCanvas (mxCanvas, UNO_QUERY);
    if (xSpriteCanvas.is())
        xSpriteCanvas->updateScreen(sal_False);
}




void PresenterPaneBorderPainter::Renderer::PaintTitle (
    const OUString& rsTitle,
    const ::boost::shared_ptr<RendererPaneStyle>& rpStyle,
    const awt::Rectangle& rUpdateBox,
    const awt::Rectangle& rOuterBox,
    const awt::Rectangle& rInnerBox,
    bool bPaintBackground)
{
    if ( ! mxCanvas.is())
        return;

    if (rsTitle.getLength() == 0)
        return;

    if ( ! mxCanvasFont.is())
    {
        mxCanvasFont = mxCanvas->createFont(
            rpStyle->GetCanvasFontDescriptor(),
            Sequence<beans::PropertyValue>(),
            geometry::Matrix2D(1,0,0,1));
    }

    if ( ! mxCanvasFont.is())
        return;

    rendering::StringContext aContext (
        rsTitle,
        0,
        rsTitle.getLength());
    Reference<rendering::XTextLayout> xLayout (mxCanvasFont->createTextLayout(
        aContext,
        rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
        0));
    if ( ! xLayout.is())
        return;

    geometry::RealRectangle2D aBox (xLayout->queryTextBounds());
    const double nTextHeight = aBox.Y2 - aBox.Y1;
    const double nTextWidth = aBox.X2 - aBox.X1;
    double nX = rInnerBox.X + (rInnerBox.Width - nTextWidth)/2;
    const sal_Int32 nTitleBarHeight = rInnerBox.Y - rOuterBox.Y - 1;
    double nY = rOuterBox.Y + (nTitleBarHeight - nTextHeight) / 2 + 3*nTextHeight/4;
    if (nY >= rInnerBox.Y)
        nY = rInnerBox.Y - 1;
    switch (rpStyle->meFontAnchor)
    {
        default:
        case RendererPaneStyle::AnchorLeft:
            nX = rInnerBox.X;
            break;
        case RendererPaneStyle::AnchorRight:
            nX = rInnerBox.X + rInnerBox.Width - nTextWidth;
            break;
        case RendererPaneStyle::AnchorCenter:
            nX = rInnerBox.X + (rInnerBox.Width - nTextWidth)/2;
            break;
    }

    if (rUpdateBox.X >= nX+nTextWidth
        || rUpdateBox.Y >= nY+nTextHeight
        || rUpdateBox.X+rUpdateBox.Width <= nX
        || rUpdateBox.Y+rUpdateBox.Height <= nY)
    {
        return;
    }

    rendering::RenderState aRenderState(
        geometry::AffineMatrix2D(1,0,nX, 0,1,nY),
        NULL,
        Sequence<double>(3),
        rendering::CompositeOperation::SOURCE);

    if (bPaintBackground)
    {
        aRenderState.DeviceColor[0] = 1;
        aRenderState.DeviceColor[1] = 1;
        aRenderState.DeviceColor[2] = 1;
        Sequence<Sequence<geometry::RealPoint2D> > aPolygons(1);
        aPolygons[0] = Sequence<geometry::RealPoint2D>(4);
        aPolygons[0][0] = geometry::RealPoint2D(0, -nTextHeight);
        aPolygons[0][1] = geometry::RealPoint2D(0, 0);
        aPolygons[0][2] = geometry::RealPoint2D(nTextWidth, 0);
        aPolygons[0][3] = geometry::RealPoint2D(nTextWidth, -nTextHeight);
        Reference<rendering::XPolyPolygon2D> xPolygon (
            mxCanvas->getDevice()->createCompatibleLinePolyPolygon(aPolygons), UNO_QUERY);
        if (xPolygon.is())
            xPolygon->setClosed(0, sal_True);
        mxCanvas->fillPolyPolygon(
            xPolygon,
            maViewState,
            aRenderState);
    }
    else
    {
        aRenderState.DeviceColor[0] = ((rpStyle->maFontColor&0x00ff0000)>>16) / 255.0;
        aRenderState.DeviceColor[1] = ((rpStyle->maFontColor&0x0000ff00)>>8) / 255.0;
        aRenderState.DeviceColor[2] = (rpStyle->maFontColor&0x000000ff) / 255.0;

        mxCanvas->drawText(
            aContext,
            mxCanvasFont,
            maViewState,
            aRenderState,
            rendering::TextDirection::WEAK_LEFT_TO_RIGHT);
    }
}



::boost::shared_ptr<RendererPaneStyle>
    PresenterPaneBorderPainter::Renderer::GetRendererPaneStyle (const OUString& rsResourceURL)
{
    OSL_ASSERT(mpTheme.get()!=NULL);

    RendererPaneStyleContainer::const_iterator iStyle (maRendererPaneStyles.find(rsResourceURL));
    if (iStyle == maRendererPaneStyles.end())
    {
        OUString sStyleName (OUString::createFromAscii("DefaultRendererPaneStyle"));

        try
        {
            // Get pane layout name for resource URL.
            mpTheme->getPropertyValue(rsResourceURL) >>= sStyleName;
        }
        catch (beans::UnknownPropertyException&)
        {
            OSL_ENSURE(false, "pane style not found");
        }

        // Create a new pane style object and initialize it with bitmaps.
        ::boost::shared_ptr<RendererPaneStyle> pStyle (new RendererPaneStyle(mpTheme,sStyleName));
        iStyle = maRendererPaneStyles.insert(
            RendererPaneStyleContainer::value_type(rsResourceURL, pStyle)).first;
    }
    if (iStyle != maRendererPaneStyles.end())
        return iStyle->second;
    else
        return ::boost::shared_ptr<RendererPaneStyle>();
}




void PresenterPaneBorderPainter::Renderer::PaintFrameBackground (
    const awt::Rectangle& rOuterBox,
    const awt::Rectangle& rInnerBox)
{
    (void)rOuterBox;
    (void)rInnerBox;
}




void PresenterPaneBorderPainter::Renderer::PaintBitmap(
    const awt::Rectangle& rBox,
    const awt::Rectangle& rUpdateBox,
    const ::boost::shared_ptr<RendererPaneStyle>& rpStyle,
    const sal_Int32 nXPosition,
    const sal_Int32 nYPosition,
    const sal_Int32 nStartOffset,
    const sal_Int32 nEndOffset,
    const BitmapDescriptor& rBitmap,
    const double nBorderTransparency)
{
    (void)nBorderTransparency;
    (void)rpStyle;

    bool bUseCanvas (mxCanvas.is());
    if ( ! bUseCanvas)
        return;

    if (rBitmap.maSize.Width<=0 || rBitmap.maSize.Height<=0)
        return;

    if ( ! rBitmap.mxBitmap.is())
        return;

    // Calculate position, and for side bitmaps, the size.
    sal_Int32 nX = 0;
    sal_Int32 nY = 0;
    sal_Int32 nW = rBitmap.maSize.Width;
    sal_Int32 nH = rBitmap.maSize.Height;
    if (nXPosition < 0)
    {
        nX = rBox.X - rBitmap.maSize.Width + rBitmap.maOffset.X;
    }
    else if (nXPosition > 0)
    {
        nX = rBox.X + rBox.Width + rBitmap.maOffset.X;
    }
    else
    {
        nX = rBox.X + nStartOffset;
        nW = rBox.Width - nStartOffset + nEndOffset;
    }

    if (nYPosition < 0)
    {
        nY = rBox.Y - rBitmap.maSize.Height + rBitmap.maOffset.Y;
    }
    else if (nYPosition > 0)
    {
        nY = rBox.Y + rBox.Height + rBitmap.maOffset.Y;
    }
    else
    {
        nY = rBox.Y + nStartOffset;
        nH = rBox.Height - nStartOffset + nEndOffset;
    }

    // Do not paint when bitmap area does not intersect with update box.
    if (nX >= rUpdateBox.X + rUpdateBox.Width
        || nX+nW <= rUpdateBox.X
        || nY >= rUpdateBox.Y + rUpdateBox.Height
        || nY+nH <= rUpdateBox.Y)
    {
        return;
    }

    rendering::RenderState aRenderState (
        geometry::AffineMatrix2D(
            double(nW)/rBitmap.maSize.Width, 0, nX,
            0, double(nH)/rBitmap.maSize.Height, nY),
        NULL,
        Sequence<double>(3),
        rendering::CompositeOperation::OVER);

    mxCanvas->drawBitmap(rBitmap.mxBitmap,maViewState,aRenderState);
}




void PresenterPaneBorderPainter::Renderer::SetupClipping (const awt::Rectangle& rUpdateBox,
    bool bPaintOutline)
{
    mxViewStateClip = NULL;
    maViewState.Clip = NULL;

    if ( ! mxCanvas.is())
        return;

    mxViewStateClip = PresenterGeometryHelper::CreatePolygon(rUpdateBox, mxCanvas->getDevice());

    if (bPaintOutline)
    {
        maViewState.Clip = NULL;
        rendering::RenderState aRenderState(
            geometry::AffineMatrix2D(1,0,0, 0,1,0),
            NULL,
            Sequence<double>(3),
            rendering::CompositeOperation::SOURCE);
        aRenderState.DeviceColor[0] = 0;
        aRenderState.DeviceColor[1] = 1;
        aRenderState.DeviceColor[2] = 0;
        mxCanvas->drawPolyPolygon(mxViewStateClip, maViewState, aRenderState);
    }

    maViewState.Clip = mxViewStateClip;
}



namespace {

//===== BitmapDescriptor ======================================================

BitmapDescriptor::BitmapDescriptor(
    const ::boost::shared_ptr<PresenterTheme>& rpTheme,
    const OUString& rsStyleName,
    const OUString& rsBitmapName)
    : mxBitmap(),
      maOffset(0,0),
      maSize(0,0),
      maReplacementColor(0x00ffffff)
{
    if (rpTheme.get() == NULL)
        return;

    const OUString sPrefix (rsStyleName
        +OUString::createFromAscii("_Border_")
        +rsBitmapName
        +OUString::createFromAscii("_"));

    try
    {
        rpTheme->getPropertyValue(sPrefix+OUString::createFromAscii("Bitmap")) >>= mxBitmap;
        if (mxBitmap.is())
        {
            const geometry::IntegerSize2D aSize (mxBitmap->getSize());
            maSize = awt::Size(aSize.Width, aSize.Height);
        }
    }
    catch(beans::UnknownPropertyException&)
    {
    }

    try
    {
        rpTheme->getPropertyValue(sPrefix+OUString::createFromAscii("Offset")) >>= maOffset;
    }
    catch(beans::UnknownPropertyException&)
    {
    }

    try
    {
        rpTheme->getPropertyValue(sPrefix+OUString::createFromAscii("Color")) >>= maReplacementColor;
    }
    catch(beans::UnknownPropertyException&)
    {
    }
}




//===== BorderSize ============================================================

BorderSize::BorderSize (void)
    : mnLeft(0),
      mnTop(0),
      mnRight(0),
      mnBottom(0)
{
}




BorderSize::BorderSize (const BorderSize& rBorderSize)
    : mnLeft(rBorderSize.mnLeft),
      mnTop(rBorderSize.mnTop),
      mnRight(rBorderSize.mnRight),
      mnBottom(rBorderSize.mnBottom)
{
}




BorderSize& BorderSize::operator= (const BorderSize& rBorderSize)
{
    if (&rBorderSize != this)
    {
        mnLeft = rBorderSize.mnLeft;
        mnTop = rBorderSize.mnTop;
        mnRight = rBorderSize.mnRight;
        mnBottom = rBorderSize.mnBottom;
    }
    return *this;
}




//===== RendererPaneStyle  ============================================================

RendererPaneStyle::RendererPaneStyle (
    const ::boost::shared_ptr<PresenterTheme>& rpTheme,
    const OUString& rsStyleName)
    : maTopLeft(rpTheme, rsStyleName, OUString::createFromAscii("TopLeft")),
      maTop(rpTheme, rsStyleName, OUString::createFromAscii("Top")),
      maTopRight(rpTheme, rsStyleName, OUString::createFromAscii("TopRight")),
      maLeft(rpTheme, rsStyleName, OUString::createFromAscii("Left")),
      maRight(rpTheme, rsStyleName, OUString::createFromAscii("Right")),
      maBottomLeft(rpTheme, rsStyleName, OUString::createFromAscii("BottomLeft")),
      maBottom(rpTheme, rsStyleName, OUString::createFromAscii("Bottom")),
      maBottomRight(rpTheme, rsStyleName, OUString::createFromAscii("BottomRight")),
      maWindowBackground(rpTheme, rsStyleName, OUString::createFromAscii("WindowBackground")),
      maCanvasFontDescriptor(),
      maFontColor(0),
      meFontAnchor(AnchorCenter),
      maInnerBorderSize(),
      maOuterBorderSize(),
      maTotalBorderSize()
{
    if (rpTheme.get() != NULL)
    {
        // Get font description.
        OUString sFontName (OUString::createFromAscii("Albany"));
        sal_Int32 nFontSize (17);
        maFontColor = 0x00000000;
        OUString sAnchor (OUString::createFromAscii("Left"));
        const OUString sFontPrefix (rsStyleName+OUString::createFromAscii("_Font_"));
        try
        {
            rpTheme->getPropertyValue(sFontPrefix+OUString::createFromAscii("Name")) >>= sFontName;
            rpTheme->getPropertyValue(sFontPrefix+OUString::createFromAscii("Size")) >>= nFontSize;
            rpTheme->getPropertyValue(sFontPrefix+OUString::createFromAscii("Color"))
                >>= maFontColor;
            rpTheme->getPropertyValue(sFontPrefix+OUString::createFromAscii("Anchor")) >>= sAnchor;
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        maCanvasFontDescriptor.FontDescription.FamilyName = sFontName;
        maCanvasFontDescriptor.CellSize = nFontSize;

        if (sAnchor == OUString::createFromAscii("Left"))
            meFontAnchor = AnchorLeft;
        else if (sAnchor == OUString::createFromAscii("Right"))
            meFontAnchor = AnchorRight;
        else if (sAnchor == OUString::createFromAscii("Center"))
            meFontAnchor = AnchorCenter;
        else
            meFontAnchor = AnchorCenter;

        // Get border sizes.
        Sequence<sal_Int32> aInnerBorder (4);
        Sequence<sal_Int32> aOuterBorder (4);
        try
        {
            rpTheme->getPropertyValue(rsStyleName+OUString::createFromAscii("_InnerBorderSize"))
                >>= aInnerBorder;
            maInnerBorderSize.mnLeft = aInnerBorder[0];
            maInnerBorderSize.mnTop = aInnerBorder[1];
            maInnerBorderSize.mnRight = aInnerBorder[2];
            maInnerBorderSize.mnBottom = aInnerBorder[3];

            rpTheme->getPropertyValue(rsStyleName+OUString::createFromAscii("_OuterBorderSize"))
                >>= aOuterBorder;
            maOuterBorderSize.mnLeft = aOuterBorder[0];
            maOuterBorderSize.mnTop = aOuterBorder[1];
            maOuterBorderSize.mnRight = aOuterBorder[2];
            maOuterBorderSize.mnBottom = aOuterBorder[3];
        }
        catch(beans::UnknownPropertyException&)
        {
            OSL_ASSERT(false);
        }

        UpdateBorderSizes();
    }
}





awt::Rectangle RendererPaneStyle::AddBorder (
    const awt::Rectangle& rBox,
    const drawing::framework::BorderType eBorderType) const
{
    const BorderSize* pBorderSize = NULL;
    switch (eBorderType)
    {
        case drawing::framework::BorderType_INNER_BORDER:
            pBorderSize = &maInnerBorderSize;
            break;
        case drawing::framework::BorderType_OUTER_BORDER:
            pBorderSize = &maOuterBorderSize;
            break;
        case drawing::framework::BorderType_TOTAL_BORDER:
            pBorderSize = &maTotalBorderSize;
            break;
        default:
            return rBox;
    }
    return awt::Rectangle (
        rBox.X - pBorderSize->mnLeft,
        rBox.Y - pBorderSize->mnTop,
        rBox.Width + pBorderSize->mnLeft + pBorderSize->mnRight,
        rBox.Height + pBorderSize->mnTop + pBorderSize->mnBottom);
}




awt::Rectangle RendererPaneStyle::RemoveBorder (
    const awt::Rectangle& rBox,
    const css::drawing::framework::BorderType eBorderType) const
{
    const BorderSize* pBorderSize = NULL;
    switch (eBorderType)
    {
        case drawing::framework::BorderType_INNER_BORDER:
            pBorderSize = &maInnerBorderSize;
            break;
        case drawing::framework::BorderType_OUTER_BORDER:
            pBorderSize = &maOuterBorderSize;
            break;
        case drawing::framework::BorderType_TOTAL_BORDER:
            pBorderSize = &maTotalBorderSize;
            break;
        default:
            return rBox;
    }
    return awt::Rectangle (
        rBox.X + pBorderSize->mnLeft,
        rBox.Y + pBorderSize->mnTop,
        rBox.Width - pBorderSize->mnLeft - pBorderSize->mnRight,
        rBox.Height - pBorderSize->mnTop - pBorderSize->mnBottom);
}




const rendering::FontRequest& RendererPaneStyle::GetCanvasFontDescriptor (void) const
{
    return maCanvasFontDescriptor;
}




sal_Int32 RendererPaneStyle::GetBorderSize (const Side eSide) const
{
    switch (eSide)
    {
        case Left : return maTotalBorderSize.mnLeft;
        case Right : return maTotalBorderSize.mnRight;
        case Top : return maTotalBorderSize.mnTop;
        case Bottom : return maTotalBorderSize.mnBottom;
    }
    return 0;
}




void RendererPaneStyle::UpdateBorderSizes (void)
{
    maTotalBorderSize.mnLeft = maInnerBorderSize.mnLeft + maOuterBorderSize.mnLeft;
    maTotalBorderSize.mnTop = maInnerBorderSize.mnTop + maOuterBorderSize.mnTop;
    maTotalBorderSize.mnRight = maInnerBorderSize.mnRight + maOuterBorderSize.mnRight;
    maTotalBorderSize.mnBottom = maInnerBorderSize.mnBottom + maOuterBorderSize.mnBottom;
}



} // end of anonymous namespace


} } // end of namespace ::sd::presenter
