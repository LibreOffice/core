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
#include <vcl/pngwrite.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclmedit.hxx>
#include <vcl/button.hxx>
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
        OUString aTempl = OUString("<alt id=\"%1\">"
                                   " " //FIXME real dialog title or something
                                  "</alt>");
        aTempl = aTempl.replaceFirst( "%1", lcl_genRandom("alt_id") );

        return aTempl;
    }

    OUString lcl_Image( const OUString& rScreenshotId, const Size& rSize )
    {
        OUString aTempl = OUString("<image id=\"%1\" src=\"media/screenshots/%2.png\""
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
        OUString aTempl = OUString( "<paragraph id=\"%1\" role=\"paragraph\">%2"
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

class ScreenshotAnnotationDlg_Impl // : public ModalDialog
{
public:
    ScreenshotAnnotationDlg_Impl(
        ScreenshotAnnotationDlg& rParent,
        Dialog& rParentDialog);
    ~ScreenshotAnnotationDlg_Impl();

private:
    // Handler for click on save
    DECL_LINK(saveButtonHandler, Button*, void);

    // Handler for clicks on picture frame
    DECL_LINK(pictureFrameListener, VclWindowEvent&, void);

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
    VclPtr<VirtualDevice>       mpVirtualBufferDevice;

    // all detected children
    ControlDataCollection       maAllChildren;

    // hilighted/selected children
    ControlDataEntry*           mpHilighted;
    std::set< ControlDataEntry* >
                                maSelected;

    // list of detected controls
    VclPtr<FixedImage>          mpPicture;
    VclPtr<VclMultiLineEdit>    mpText;
    VclPtr<PushButton>          mpSave;

    // save as text
    OUString                    maSaveAsText;
    OUString                    maMainMarkupText;

    // folder URL
    static OUString             maLastFolderURL;
};

OUString ScreenshotAnnotationDlg_Impl::maLastFolderURL = OUString();

ScreenshotAnnotationDlg_Impl::ScreenshotAnnotationDlg_Impl(
    ScreenshotAnnotationDlg& rParent,
    Dialog& rParentDialog)
:   mrParentDialog(rParentDialog),
    maParentDialogBitmap(rParentDialog.createScreenshot()),
    maDimmedDialogBitmap(maParentDialogBitmap),
    maParentDialogSize(maParentDialogBitmap.GetSizePixel()),
    mpVirtualBufferDevice(nullptr),
    maAllChildren(),
    mpHilighted(nullptr),
    maSelected(),
    mpPicture(nullptr),
    mpText(nullptr),
    mpSave(nullptr),
    maSaveAsText(CuiResId(RID_SVXSTR_SAVE_SCREENSHOT_AS))
{
    // image ain't empty
    assert(!maParentDialogBitmap.IsEmpty());
    assert(0 != maParentDialogBitmap.GetSizePixel().Width());
    assert(0 != maParentDialogBitmap.GetSizePixel().Height());

    // get needed widgets
    rParent.get(mpPicture, "picture");
    assert(mpPicture.get());
    rParent.get(mpText, "text");
    assert(mpText.get());
    rParent.get(mpSave, "save");
    assert(mpSave.get());

    // set screenshot image at FixedImage, resize, set event listener
    if (mpPicture)
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
        mpVirtualBufferDevice = VclPtr<VirtualDevice>::Create(*Application::GetDefaultDevice(), DeviceFormat::DEFAULT, DeviceFormat::BITMASK);
        mpVirtualBufferDevice->SetOutputSizePixel(maParentDialogSize);
        mpVirtualBufferDevice->SetFillColor(COL_TRANSPARENT);

        // initially set image for picture control
        mpPicture->SetImage(Image(maDimmedDialogBitmap));

        // set size for picture control, this will re-layout so that
        // the picture control shows the whole dialog
        mpPicture->set_width_request(maParentDialogSize.Width());
        mpPicture->set_height_request(maParentDialogSize.Height());

        // add local event listener to allow interactions with mouse
        mpPicture->AddEventListener(LINK(this, ScreenshotAnnotationDlg_Impl, pictureFrameListener));

        // avoid image scaling, this is needed for images smaller than the
        // minimal dialog size
        const WinBits aWinBits(mpPicture->GetStyle());
        mpPicture->SetStyle(aWinBits & ~WB_SCALE);
    }

    // set some test text at VclMultiLineEdit and make read-only - only
    // copying content to clipboard is allowed
    if (mpText)
    {
        mpText->set_width_request(400);
        mpText->set_height_request( mpText->GetTextHeight() * 10 );
        OUString aHelpId = OStringToOUString( mrParentDialog.GetHelpId(), RTL_TEXTENCODING_UTF8 );
        Size aSizeCm = mrParentDialog.PixelToLogic(maParentDialogSize, MapMode(MapUnit::MapCM));
        maMainMarkupText = lcl_ParagraphWithImage( aHelpId, aSizeCm );
        mpText->SetText( maMainMarkupText );
        mpText->SetReadOnly();
    }

    // set click handler for save button
    if (mpSave)
    {
        mpSave->SetClickHdl(LINK(this, ScreenshotAnnotationDlg_Impl, saveButtonHandler));
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
    mpVirtualBufferDevice.disposeAndClear();
}

IMPL_LINK_NOARG(ScreenshotAnnotationDlg_Impl, saveButtonHandler, Button*, void)
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

        if (files.getLength())
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
                        mpVirtualBufferDevice->GetBitmapEx(
                        Point(0, 0),
                        mpVirtualBufferDevice->GetOutputSizePixel()));

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
    if (mpPicture && mpVirtualBufferDevice)
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

        mpVirtualBufferDevice->SetLineColor(rColor);

        // try to use transparency
        if (!mpVirtualBufferDevice->DrawPolyLineDirect(
            basegfx::B2DHomMatrix(),
            aPolygon,
            fLineWidth,
            fTransparency,
            basegfx::B2DLineJoin::Round))
        {
            // no transparency, draw without
            mpVirtualBufferDevice->DrawPolyLine(
                aPolygon,
                fLineWidth);
        }
    }
}

Point ScreenshotAnnotationDlg_Impl::GetOffsetInPicture() const
{
    if (!mpPicture)
    {
        return Point(0, 0);
    }

    const Size aPixelSizeTarget(mpPicture->GetOutputSizePixel());

    return Point(
        aPixelSizeTarget.Width() > maParentDialogSize.Width() ? (aPixelSizeTarget.Width() - maParentDialogSize.Width()) >> 1 : 0,
        aPixelSizeTarget.Height() > maParentDialogSize.Height() ? (aPixelSizeTarget.Height() - maParentDialogSize.Height()) >> 1 : 0);
}

void ScreenshotAnnotationDlg_Impl::RepaintToBuffer(
    bool bUseDimmed,
    bool bPaintHilight)
{
    if (mpVirtualBufferDevice)
    {
        // reset with original screenshot bitmap
        mpVirtualBufferDevice->DrawBitmapEx(
            Point(0, 0),
            bUseDimmed ? maDimmedDialogBitmap : maParentDialogBitmap);

        // get various options
        const SvtOptionsDrawinglayer aSvtOptionsDrawinglayer;
        const Color aHilightColor(aSvtOptionsDrawinglayer.getHilightColor());
        const double fTransparence(aSvtOptionsDrawinglayer.GetTransparentSelectionPercent() * 0.01);
        const bool bIsAntiAliasing(aSvtOptionsDrawinglayer.IsAntiAliasing());
        const AntialiasingFlags nOldAA(mpVirtualBufferDevice->GetAntialiasing());

        if (bIsAntiAliasing)
        {
            mpVirtualBufferDevice->SetAntialiasing(AntialiasingFlags::EnableB2dDraw);
        }

        // paint selected entries
        for (auto&& rCandidate : maSelected)
        {
            static const double fLineWidthEntries(5.0);
            PaintControlDataEntry(*rCandidate, COL_LIGHTRED, fLineWidthEntries, fTransparence * 0.2);
        }

        // paint hilighted entry
        if (mpHilighted && bPaintHilight)
        {
            static const double fLineWidthHilight(7.0);
            PaintControlDataEntry(*mpHilighted, aHilightColor, fLineWidthHilight, fTransparence);
        }

        if (bIsAntiAliasing)
        {
            mpVirtualBufferDevice->SetAntialiasing(nOldAA);
        }
    }
}

void ScreenshotAnnotationDlg_Impl::RepaintPictureElement()
{
    if (mpPicture && mpVirtualBufferDevice)
    {
        // reset image in buffer, use dimmed version and allow hilight
        RepaintToBuffer(true, true);

        // copy new content to picture control (hard paint)
        mpPicture->DrawOutDev(
            GetOffsetInPicture(),
            maParentDialogSize,
            Point(0, 0),
            maParentDialogSize,
            *mpVirtualBufferDevice);

        // also set image to get repaints right, but trigger no repaint
        mpPicture->SetImage(
            Image(
            mpVirtualBufferDevice->GetBitmapEx(
            Point(0, 0),
            mpVirtualBufferDevice->GetOutputSizePixel())));
        mpPicture->Validate();
    }
}

IMPL_LINK(ScreenshotAnnotationDlg_Impl, pictureFrameListener, VclWindowEvent&, rEvent, void)
{
    // event in picture frame
    bool bRepaint(false);

    switch (rEvent.GetId())
    {
    case VclEventId::WindowMouseMove:
    case VclEventId::WindowMouseButtonUp:
    {
        MouseEvent* pMouseEvent = static_cast< MouseEvent* >(rEvent.GetData());

        if (pMouseEvent)
        {
            switch (rEvent.GetId())
            {
            case VclEventId::WindowMouseMove:
            {
                if (mpPicture->IsMouseOver())
                {
                    const ControlDataEntry* pOldHit = mpHilighted;
                    const Point aOffset(GetOffsetInPicture());
                    const basegfx::B2IPoint aMousePos(
                        pMouseEvent->GetPosPixel().X() - aOffset.X(),
                        pMouseEvent->GetPosPixel().Y() - aOffset.Y());
                    const ControlDataEntry* pHit = CheckHit(aMousePos);

                    if (pHit && pOldHit != pHit)
                    {
                        mpHilighted = const_cast< ControlDataEntry* >(pHit);
                        bRepaint = true;
                    }
                }
                else if (mpHilighted)
                {
                    mpHilighted = nullptr;
                    bRepaint = true;
                }
                break;
            }
            case VclEventId::WindowMouseButtonUp:
            {
                if (mpPicture->IsMouseOver() && mpHilighted)
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

                    mpText->SetText( aBookmarks.makeStringAndClear() );
                    bRepaint = true;
                }
                break;
            }
            default:
            {
                break;
            }
            }
        }
        break;
    }
    default:
    {
        break;
    }
    }

    if (bRepaint)
    {
        RepaintPictureElement();
    }
}

ScreenshotAnnotationDlg::ScreenshotAnnotationDlg(
    vcl::Window* pParent,
    Dialog& rParentDialog)
:   SfxModalDialog(pParent, "ScreenshotAnnotationDialog", "cui/ui/screenshotannotationdialog.ui")
{
    m_pImpl.reset(new ScreenshotAnnotationDlg_Impl(*this, rParentDialog));
}


ScreenshotAnnotationDlg::~ScreenshotAnnotationDlg()
{
    disposeOnce();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
