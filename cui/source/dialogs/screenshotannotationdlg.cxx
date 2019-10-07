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

#include <screenshotannotationdlg.hxx>

#include <strings.hrc>
#include <dialmgr.hxx>

#include <basegfx/range/b2irange.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>

#include <comphelper/random.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vcl/customweld.hxx>
#include <vcl/event.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <svtools/optionsdrawinglayer.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <set>

using namespace com::sun::star;

namespace
{
    OUString lcl_genRandom( const OUString &rId )
    {
        //FIXME: plus timestamp
        unsigned int nRand = comphelper::rng::uniform_uint_distribution(0, 0xFFFF);
        return OUString( rId + OUString::number( nRand ) );
    }


    OUString lcl_AltDescr()
    {
        OUString aTempl("<alt id=\"%1\">"
                        " " //FIXME real dialog title or something
                        "</alt>");
        aTempl = aTempl.replaceFirst( "%1", lcl_genRandom("alt_id") );

        return aTempl;
    }

    OUString lcl_Image( const OUString& rScreenshotId, const Size& rSize )
    {
        OUString aTempl("<image id=\"%1\" src=\"media/screenshots/%2.png\""
                           " width=\"%3cm\"  height=\"%4cm\">"
                           "%5"
                        "</image>");
        aTempl = aTempl.replaceFirst( "%1", lcl_genRandom("img_id") );
        aTempl = aTempl.replaceFirst( "%2", rScreenshotId );
        aTempl = aTempl.replaceFirst( "%3", OUString::number( rSize.Width() ) );
        aTempl = aTempl.replaceFirst( "%4", OUString::number( rSize.Height() ) );
        aTempl = aTempl.replaceFirst( "%5", lcl_AltDescr() );

        return aTempl;
    }

    OUString lcl_ParagraphWithImage( const OUString& rScreenshotId, const Size& rSize )
    {
        OUString aTempl( "<paragraph id=\"%1\" role=\"paragraph\">%2"
                         "</paragraph>"  SAL_NEWLINE_STRING );
        aTempl = aTempl.replaceFirst( "%1", lcl_genRandom("par_id") );
        aTempl = aTempl.replaceFirst( "%2", lcl_Image(rScreenshotId, rSize) );

        return aTempl;
    }

    OUString lcl_Bookmark( const OUString& rWidgetId )
    {
        OUString aTempl = "<!-- Bookmark for widget %1 -->" SAL_NEWLINE_STRING
                          "<bookmark branch=\"hid/%2\" id=\"%3\" localize=\"false\"/>" SAL_NEWLINE_STRING;
        aTempl = aTempl.replaceFirst( "%1", rWidgetId );
        aTempl = aTempl.replaceFirst( "%2", rWidgetId );
        aTempl = aTempl.replaceFirst( "%3", lcl_genRandom("bm_id") );

        return aTempl;
    }
}

class ControlDataEntry
{
public:
    ControlDataEntry(
        const vcl::Window& rControl,
        const basegfx::B2IRange& rB2IRange)
        : mrControl(rControl),
        maB2IRange(rB2IRange)
    {
    }

    const basegfx::B2IRange& getB2IRange() const
    {
        return maB2IRange;
    }

    OString const & GetHelpId() const { return mrControl.GetHelpId(); }

private:
    const vcl::Window&  mrControl;
    basegfx::B2IRange   maB2IRange;
};

typedef std::vector< ControlDataEntry > ControlDataCollection;

class Picture : public weld::CustomWidgetController
{
private:
    ScreenshotAnnotationDlg_Impl *m_pDialog;
    bool m_bMouseOver;
private:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual bool MouseMove(const MouseEvent& rMouseEvent) override;
    virtual bool MouseButtonUp(const MouseEvent& rMouseEvent) override;
public:
    Picture(ScreenshotAnnotationDlg_Impl* pDialog)
        : m_pDialog(pDialog)
        , m_bMouseOver(false)
    {
    }

    bool IsMouseOver() const
    {
        return m_bMouseOver;
    }
};

class ScreenshotAnnotationDlg_Impl
{
public:
    ScreenshotAnnotationDlg_Impl(
        weld::Builder& rParent,
        Dialog& rParentDialog);
    ~ScreenshotAnnotationDlg_Impl();

private:
    // Handler for click on save
    DECL_LINK(saveButtonHandler, weld::Button&, void);

    // helper methods
    void CollectChildren(
        const vcl::Window& rCurrent,
        const basegfx::B2IPoint& rTopLeft,
        ControlDataCollection& rControlDataCollection);
    ControlDataEntry* CheckHit(const basegfx::B2IPoint& rPosition);
    void PaintControlDataEntry(
        const ControlDataEntry& rEntry,
        const Color& rColor,
        double fLineWidth,
        double fTransparency);
    void RepaintToBuffer(
        bool bUseDimmed = false,
        bool bPaintHilight = false);
    void RepaintPictureElement();
    Point GetOffsetInPicture() const;

    // local variables
    Dialog&                     mrParentDialog;
    BitmapEx                    maParentDialogBitmap;
    BitmapEx                    maDimmedDialogBitmap;
    Size                        maParentDialogSize;

    // VirtualDevice for buffered interaction paints
    VclPtr<VirtualDevice>       mxVirtualBufferDevice;

    // all detected children
    ControlDataCollection       maAllChildren;

    // highlighted/selected children
    ControlDataEntry*           mpHilighted;
    std::set< ControlDataEntry* >
                                maSelected;

    // list of detected controls
    Picture maPicture;
    std::unique_ptr<weld::CustomWeld> mxPicture;
    std::unique_ptr<weld::TextView> mxText;
    std::unique_ptr<weld::Button> mxSave;

    // save as text
    OUString                    maSaveAsText;
    OUString                    maMainMarkupText;

    // folder URL
    static OUString             maLastFolderURL;
public:
    void Paint(vcl::RenderContext& rRenderContext);
    bool MouseMove(const MouseEvent& rMouseEvent);
    bool MouseButtonUp();
};

OUString ScreenshotAnnotationDlg_Impl::maLastFolderURL = OUString();

ScreenshotAnnotationDlg_Impl::ScreenshotAnnotationDlg_Impl(
    weld::Builder& rParent,
    Dialog& rParentDialog)
:   mrParentDialog(rParentDialog),
    maParentDialogBitmap(rParentDialog.createScreenshot()),
    maDimmedDialogBitmap(maParentDialogBitmap),
    maParentDialogSize(maParentDialogBitmap.GetSizePixel()),
    mxVirtualBufferDevice(nullptr),
    maAllChildren(),
    mpHilighted(nullptr),
    maSelected(),
    maPicture(this),
    maSaveAsText(CuiResId(RID_SVXSTR_SAVE_SCREENSHOT_AS))
{
    // image ain't empty
    assert(!maParentDialogBitmap.IsEmpty());
    assert(0 != maParentDialogBitmap.GetSizePixel().Width());
    assert(0 != maParentDialogBitmap.GetSizePixel().Height());

    // get needed widgets
    mxPicture.reset(new weld::CustomWeld(rParent, "picture", maPicture));
    assert(mxPicture.get());
    mxText = rParent.weld_text_view("text");
    assert(mxText.get());
    mxSave = rParent.weld_button("save");
    assert(mxSave.get());

    // set screenshot image at FixedImage, resize, set event listener
    if (mxPicture)
    {
        // collect all children. Choose start pos to be negative
        // of target dialog's position to get all positions relative to (0,0)
        const Point aParentPos(mrParentDialog.GetPosPixel());
        const basegfx::B2IPoint aTopLeft(-aParentPos.X(), -aParentPos.Y());

        CollectChildren(
            mrParentDialog,
            aTopLeft,
            maAllChildren);

        // to make clear that maParentDialogBitmap is a background image, adjust
        // luminance a bit for maDimmedDialogBitmap - other methods may be applied
        maDimmedDialogBitmap.Adjust(-15, 0, 0, 0, 0);

        // init paint buffering VirtualDevice
        mxVirtualBufferDevice = VclPtr<VirtualDevice>::Create(*Application::GetDefaultDevice(), DeviceFormat::DEFAULT, DeviceFormat::BITMASK);
        mxVirtualBufferDevice->SetOutputSizePixel(maParentDialogSize);
        mxVirtualBufferDevice->SetFillColor(COL_TRANSPARENT);

        // initially set image for picture control
        mxVirtualBufferDevice->DrawBitmapEx(Point(0, 0), maDimmedDialogBitmap);

        // set size for picture control, this will re-layout so that
        // the picture control shows the whole dialog
        maPicture.SetOutputSizePixel(maParentDialogSize);
        mxPicture->set_size_request(maParentDialogSize.Width(), maParentDialogSize.Height());

        mxPicture->queue_draw();
    }

    // set some test text at VclMultiLineEdit and make read-only - only
    // copying content to clipboard is allowed
    if (mxText)
    {
        mxText->set_size_request(400, mxText->get_height_rows(10));
        OUString aHelpId = OStringToOUString( mrParentDialog.GetHelpId(), RTL_TEXTENCODING_UTF8 );
        Size aSizeCm = mrParentDialog.PixelToLogic(maParentDialogSize, MapMode(MapUnit::MapCM));
        maMainMarkupText = lcl_ParagraphWithImage( aHelpId, aSizeCm );
        mxText->set_text( maMainMarkupText );
        mxText->set_editable(false);
    }

    // set click handler for save button
    if (mxSave)
    {
        mxSave->connect_clicked(LINK(this, ScreenshotAnnotationDlg_Impl, saveButtonHandler));
    }
}

void ScreenshotAnnotationDlg_Impl::CollectChildren(
    const vcl::Window& rCurrent,
    const basegfx::B2IPoint& rTopLeft,
    ControlDataCollection& rControlDataCollection)
{
    if (rCurrent.IsVisible())
    {
        const Point aCurrentPos(rCurrent.GetPosPixel());
        const Size aCurrentSize(rCurrent.GetSizePixel());
        const basegfx::B2IPoint aCurrentTopLeft(rTopLeft.getX() + aCurrentPos.X(), rTopLeft.getY() + aCurrentPos.Y());
        const basegfx::B2IRange aCurrentRange(aCurrentTopLeft, aCurrentTopLeft + basegfx::B2IPoint(aCurrentSize.Width(), aCurrentSize.Height()));

        if (!aCurrentRange.isEmpty())
        {
            rControlDataCollection.emplace_back(rCurrent, aCurrentRange);
        }

        for (sal_uInt16 a(0); a < rCurrent.GetChildCount(); a++)
        {
            vcl::Window* pChild = rCurrent.GetChild(a);

            if (nullptr != pChild)
            {
                CollectChildren(*pChild, aCurrentTopLeft, rControlDataCollection);
            }
        }
    }
}

ScreenshotAnnotationDlg_Impl::~ScreenshotAnnotationDlg_Impl()
{
    mxVirtualBufferDevice.disposeAndClear();
}

IMPL_LINK_NOARG(ScreenshotAnnotationDlg_Impl, saveButtonHandler, weld::Button&, void)
{
    // 'save screenshot...' pressed, offer to save maParentDialogBitmap
    // as PNG image, use *.id file name as screenshot file name offering
    OString aDerivedFileName;

    // get a suggestion for the filename from ui file name
    {
        const OString& rUIFileName = mrParentDialog.getUIFile();
        sal_Int32 nIndex(0);

        do
        {
            const OString aToken(rUIFileName.getToken(0, '/', nIndex));

            if (!aToken.isEmpty())
            {
                aDerivedFileName = aToken;
            }
        } while (nIndex >= 0);
    }

    uno::Reference< uno::XComponentContext > xContext = cppu::defaultBootstrap_InitialComponentContext();
    const uno::Reference< ui::dialogs::XFilePicker3 > xFilePicker =
        ui::dialogs::FilePicker::createWithMode(xContext, ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION);

    xFilePicker->setTitle(maSaveAsText);

    if (!maLastFolderURL.isEmpty())
    {
        xFilePicker->setDisplayDirectory(maLastFolderURL);
    }

    xFilePicker->appendFilter("*.png", "*.png");
    xFilePicker->setCurrentFilter("*.png");
    xFilePicker->setDefaultName(OStringToOUString(aDerivedFileName, RTL_TEXTENCODING_UTF8));
    xFilePicker->setMultiSelectionMode(false);

    if (xFilePicker->execute() == ui::dialogs::ExecutableDialogResults::OK)
    {
        maLastFolderURL = xFilePicker->getDisplayDirectory();
        const uno::Sequence< OUString > files(xFilePicker->getSelectedFiles());

        if (files.hasElements())
        {
            OUString aConfirmedName = files[0];

            if (!aConfirmedName.isEmpty())
            {
                INetURLObject aConfirmedURL(aConfirmedName);
                OUString aCurrentExtension(aConfirmedURL.getExtension());

                if (!aCurrentExtension.isEmpty() && aCurrentExtension != "png")
                {
                    aConfirmedURL.removeExtension();
                    aCurrentExtension.clear();
                }

                if (aCurrentExtension.isEmpty())
                {
                    aConfirmedURL.setExtension("png");
                }

                // open stream
                SvFileStream aNew(aConfirmedURL.PathToFileName(), StreamMode::WRITE | StreamMode::TRUNC);

                if (aNew.IsOpen())
                {
                    // prepare bitmap to save - do use the original screenshot here,
                    // not the dimmed one
                    RepaintToBuffer();

                    // extract Bitmap
                    const BitmapEx aTargetBitmap(
                        mxVirtualBufferDevice->GetBitmapEx(
                        Point(0, 0),
                        mxVirtualBufferDevice->GetOutputSizePixel()));

                    // write as PNG
                    vcl::PNGWriter aPNGWriter(aTargetBitmap);
                    aPNGWriter.Write(aNew);
                }
            }
        }
    }
}

ControlDataEntry* ScreenshotAnnotationDlg_Impl::CheckHit(const basegfx::B2IPoint& rPosition)
{
    ControlDataEntry* pRetval = nullptr;

    for (auto&& rCandidate : maAllChildren)
    {
        if (rCandidate.getB2IRange().isInside(rPosition))
        {
            if (pRetval)
            {
                if (pRetval->getB2IRange().isInside(rCandidate.getB2IRange().getMinimum())
                    && pRetval->getB2IRange().isInside(rCandidate.getB2IRange().getMaximum()))
                {
                    pRetval = &rCandidate;
                }
            }
            else
            {
                pRetval = &rCandidate;
            }
        }
    }

    return pRetval;
}

void ScreenshotAnnotationDlg_Impl::PaintControlDataEntry(
    const ControlDataEntry& rEntry,
    const Color& rColor,
    double fLineWidth,
    double fTransparency)
{
    if (mxPicture && mxVirtualBufferDevice)
    {
        basegfx::B2DRange aB2DRange(rEntry.getB2IRange());

        // grow in pixels to be a little bit 'outside'. This also
        // ensures that getWidth()/getHeight() ain't 0.0 (see division below)
        static const double fGrowTopLeft(1.5);
        static const double fGrowBottomRight(0.5);
        aB2DRange.expand(aB2DRange.getMinimum() - basegfx::B2DPoint(fGrowTopLeft, fGrowTopLeft));
        aB2DRange.expand(aB2DRange.getMaximum() + basegfx::B2DPoint(fGrowBottomRight, fGrowBottomRight));

        // edge rounding in pixel. Need to convert, value for
        // createPolygonFromRect is relative [0.0 .. 1.0]
        static const double fEdgeRoundPixel(8.0);
        const basegfx::B2DPolygon aPolygon(
            basegfx::utils::createPolygonFromRect(
            aB2DRange,
            fEdgeRoundPixel / aB2DRange.getWidth(),
            fEdgeRoundPixel / aB2DRange.getHeight()));

        mxVirtualBufferDevice->SetLineColor(rColor);

        // try to use transparency
        if (!mxVirtualBufferDevice->DrawPolyLineDirect(
            basegfx::B2DHomMatrix(),
            aPolygon,
            fLineWidth,
            fTransparency,
            basegfx::B2DLineJoin::Round))
        {
            // no transparency, draw without
            mxVirtualBufferDevice->DrawPolyLine(
                aPolygon,
                fLineWidth);
        }
    }
}

Point ScreenshotAnnotationDlg_Impl::GetOffsetInPicture() const
{
    const Size aPixelSizeTarget(maPicture.GetOutputSizePixel());

    return Point(
        aPixelSizeTarget.Width() > maParentDialogSize.Width() ? (aPixelSizeTarget.Width() - maParentDialogSize.Width()) >> 1 : 0,
        aPixelSizeTarget.Height() > maParentDialogSize.Height() ? (aPixelSizeTarget.Height() - maParentDialogSize.Height()) >> 1 : 0);
}

void ScreenshotAnnotationDlg_Impl::RepaintToBuffer(
    bool bUseDimmed,
    bool bPaintHilight)
{
    if (mxVirtualBufferDevice)
    {
        // reset with original screenshot bitmap
        mxVirtualBufferDevice->DrawBitmapEx(
            Point(0, 0),
            bUseDimmed ? maDimmedDialogBitmap : maParentDialogBitmap);

        // get various options
        const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
        const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
        const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);
        const bool bIsAntiAliasing(aSvtOptionsDrawinglayer.IsAntiAliasing());
        const AntialiasingFlags nOldAA(mxVirtualBufferDevice->GetAntialiasing());

        if (bIsAntiAliasing)
        {
            mxVirtualBufferDevice->SetAntialiasing(AntialiasingFlags::EnableB2dDraw);
        }

        // paint selected entries
        for (auto&& rCandidate : maSelected)
        {
            static const double fLineWidthEntries(5.0);
            PaintControlDataEntry(*rCandidate, COL_LIGHTRED, fLineWidthEntries, fTransparence * 0.2);
        }

        // paint highlighted entry
        if (mpHilighted && bPaintHilight)
        {
            static const double fLineWidthHilight(7.0);
            PaintControlDataEntry(*mpHilighted, aHilightColor, fLineWidthHilight, fTransparence);
        }

        if (bIsAntiAliasing)
        {
            mxVirtualBufferDevice->SetAntialiasing(nOldAA);
        }
    }
}

void ScreenshotAnnotationDlg_Impl::RepaintPictureElement()
{
    if (mxPicture && mxVirtualBufferDevice)
    {
        // reset image in buffer, use dimmed version and allow highlight
        RepaintToBuffer(true, true);
        mxPicture->queue_draw();
    }
}

void ScreenshotAnnotationDlg_Impl::Paint(vcl::RenderContext& rRenderContext)
{
    Point aPos(GetOffsetInPicture());
    Size aSize(mxVirtualBufferDevice->GetOutputSizePixel());
    rRenderContext.DrawOutDev(aPos, aSize, Point(), aSize, *mxVirtualBufferDevice);
}

void Picture::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    m_pDialog->Paint(rRenderContext);
}

bool ScreenshotAnnotationDlg_Impl::MouseMove(const MouseEvent& rMouseEvent)
{
    bool bRepaint(false);

    if (maPicture.IsMouseOver())
    {
        const ControlDataEntry* pOldHit = mpHilighted;
        const Point aOffset(GetOffsetInPicture());
        const basegfx::B2IPoint aMousePos(
            rMouseEvent.GetPosPixel().X() - aOffset.X(),
            rMouseEvent.GetPosPixel().Y() - aOffset.Y());
        const ControlDataEntry* pHit = CheckHit(aMousePos);

        if (pHit && pOldHit != pHit)
        {
            mpHilighted = const_cast<ControlDataEntry*>(pHit);
            bRepaint = true;
        }
    }
    else if (mpHilighted)
    {
        mpHilighted = nullptr;
        bRepaint = true;
    }

    if (bRepaint)
    {
        RepaintPictureElement();
    }

    return true;
}

bool Picture::MouseMove(const MouseEvent& rMouseEvent)
{
    if (rMouseEvent.IsEnterWindow())
        m_bMouseOver = true;
    if (rMouseEvent.IsLeaveWindow())
        m_bMouseOver = false;
    return m_pDialog->MouseMove(rMouseEvent);
}

bool ScreenshotAnnotationDlg_Impl::MouseButtonUp()
{
    // event in picture frame
    bool bRepaint(false);

    if (maPicture.IsMouseOver() && mpHilighted)
    {
        if (maSelected.erase(mpHilighted) == 0)
        {
            maSelected.insert(mpHilighted);
        }

        OUStringBuffer aBookmarks(maMainMarkupText);
        for (auto&& rCandidate : maSelected)
        {
            OUString aHelpId = OStringToOUString( rCandidate->GetHelpId(), RTL_TEXTENCODING_UTF8 );
            aBookmarks.append(lcl_Bookmark( aHelpId ));
        }

        mxText->set_text( aBookmarks.makeStringAndClear() );
        bRepaint = true;
    }

    if (bRepaint)
    {
        RepaintPictureElement();
    }

    return true;
}

bool Picture::MouseButtonUp(const MouseEvent&)
{
    return m_pDialog->MouseButtonUp();
}

ScreenshotAnnotationDlg::ScreenshotAnnotationDlg(weld::Window* pParent, Dialog& rParentDialog)
    : GenericDialogController(pParent, "cui/ui/screenshotannotationdialog.ui", "ScreenshotAnnotationDialog")
{
    m_pImpl.reset(new ScreenshotAnnotationDlg_Impl(*m_xBuilder, rParentDialog));
}

ScreenshotAnnotationDlg::~ScreenshotAnnotationDlg()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
