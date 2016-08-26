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

#include "screenshotannotationdlg.hxx"

#include "cuires.hrc"
#include "dialmgr.hxx"

#include <basegfx/range/b2irange.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/ui/dialogs/FilePicker.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <vcl/pngwrite.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>

using namespace com::sun::star;

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

    const vcl::Window& getControl() const
    {
        return mrControl;
    }

    const basegfx::B2IRange& getB2IRange() const
    {
        return maB2IRange;
    }

private:
    const vcl::Window&  mrControl;
    basegfx::B2IRange   maB2IRange;
};

typedef ::std::vector< ControlDataEntry > ControlDataCollection;
typedef ::std::set< ControlDataEntry* > ControlDataSet;

class ScreenshotAnnotationDlg_Impl // : public ModalDialog
{
public:
    ScreenshotAnnotationDlg_Impl(
        ScreenshotAnnotationDlg& rParent,
        Dialog& rParentDialog);
    ~ScreenshotAnnotationDlg_Impl();

private:
    // Handler for click on save
    DECL_LINK_TYPED(saveButtonHandler, Button*, void);

    // Handler for clicks on picture frame
    DECL_LINK_TYPED(pictureFrameListener, VclWindowEvent&, void);

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
        double fTransparency = 0.0);
    void RepaintToBuffer(
        bool bUseDimmed = false,
        bool bPaintHilight = false);
    void RepaintPictureElement();
    Point GetOffsetInPicture() const;

    // local variables
    ScreenshotAnnotationDlg&    mrParent;
    Dialog&                     mrParentDialog;
    Bitmap                      maParentDialogBitmap;
    Bitmap                      maDimmedDialogBitmap;
    Size                        maParentDialogSize;

    // VirtualDevice for buffered interation paints
    VclPtr<VirtualDevice>       mpVirtualBufferDevice;

    // all detected children
    ControlDataCollection       maAllChildren;

    // hilighted/selected children
    ControlDataEntry*           mpHilighted;
    ControlDataSet              maSelected;

    // list of detected controls
    VclPtr<FixedImage>          mpPicture;
    VclPtr<VclMultiLineEdit>    mpText;
    VclPtr<PushButton>          mpSave;

    // save as text
    OUString                    maSaveAsText;

    // folder URL
    static OUString             maLastFolderURL;
};

OUString ScreenshotAnnotationDlg_Impl::maLastFolderURL = OUString();

ScreenshotAnnotationDlg_Impl::ScreenshotAnnotationDlg_Impl(
    ScreenshotAnnotationDlg& rParent,
    Dialog& rParentDialog)
:   mrParent(rParent),
    mrParentDialog(rParentDialog),
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
    maSaveAsText(CUI_RES(RID_SVXSTR_SAVE_SCREENSHOT_AS))
{
    // image ain't empty
    assert(!maParentDialogBitmap.IsEmpty());
    assert(0 != maParentDialogBitmap.GetSizePixel().Width());
    assert(0 != maParentDialogBitmap.GetSizePixel().Height());

    // get needed widgets
    mrParent.get(mpPicture, "picture");
    assert(mpPicture.get());
    mrParent.get(mpText, "text");
    assert(mpText.get());
    mrParent.get(mpSave, "save");
    assert(mpSave.get());

    // set screenshot image at FixedImage, resize, set event listener
    if (mpPicture)
    {
        // colelct all children. Choose start pos to be negative
        // of target dialog's position to get all positions relative to (0,0)
        const Point aParentPos(mrParentDialog.GetPosPixel());
        const basegfx::B2IPoint aTopLeft(-aParentPos.X(), -aParentPos.Y());

        CollectChildren(
            mrParentDialog,
            aTopLeft,
            maAllChildren);

        // to make clear that maParentDialogBitmap is a background image, adjust
        // luminance a bit for maDimmedDialogBitmap - other methods may be applied
        maDimmedDialogBitmap.Adjust(-15);

        // init paint buffering VirtualDevice
        mpVirtualBufferDevice = new VirtualDevice(*Application::GetDefaultDevice(), DeviceFormat::DEFAULT, DeviceFormat::BITMASK);
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
        mpPicture->SetStyle(aWinBits & (!WinBits(WB_SCALE)));
    }

    // set some test text at VclMultiLineEdit and make read-only - only
    // copying content to clipboard is allowed
    if (mpText)
    {
        mpText->SetText("The quick brown fox jumps over the lazy dog :)");
        mpText->SetReadOnly(true);
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
            rControlDataCollection.push_back(
                ControlDataEntry(
                rCurrent,
                aCurrentRange));
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

IMPL_LINK_TYPED(ScreenshotAnnotationDlg_Impl, saveButtonHandler, Button*, pButton, void)
{
    // suppress compiler warning
    (*pButton);

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
        ui::dialogs::FilePicker::createWithMode(xContext, ui::dialogs::TemplateDescription::FILESAVE_SIMPLE);

    xFilePicker->setTitle(maSaveAsText);

    if (!maLastFolderURL.isEmpty())
    {
        xFilePicker->setDisplayDirectory(maLastFolderURL);
    }

    xFilePicker->appendFilter("*.png", "*.PNG");
    xFilePicker->setCurrentFilter("*.png");
    xFilePicker->setDefaultName(OStringToOUString(aDerivedFileName, RTL_TEXTENCODING_UTF8)); // +".png");

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

                if (!aCurrentExtension.isEmpty() && 0 != aCurrentExtension.compareTo("png"))
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
                    const Bitmap aTargetBitmap(
                        mpVirtualBufferDevice->GetBitmap(
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

    for (auto aCandidate = maAllChildren.begin(); aCandidate != maAllChildren.end(); aCandidate++)
    {
        ControlDataEntry& rCandidate = *aCandidate;

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
        const basegfx::B2IRange& rRange = rEntry.getB2IRange();
        static double fRelativeEdgeRadius(0.1);
        basegfx::B2DRange aB2DRange(rRange);

        // grow one pixel to be a little bit outside
        aB2DRange.grow(1);

        const basegfx::B2DPolygon aPolygon(
            basegfx::tools::createPolygonFromRect(
            aB2DRange,
            fRelativeEdgeRadius,
            fRelativeEdgeRadius));
        mpVirtualBufferDevice->SetLineColor(rColor);

        if (!mpVirtualBufferDevice->DrawPolyLineDirect(
            aPolygon,
            fLineWidth,
            fTransparency,
            basegfx::B2DLineJoin::Round))
        {
            mpVirtualBufferDevice->DrawPolyLine(
                aPolygon,
                fLineWidth,
                basegfx::B2DLineJoin::Round);
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
        mpVirtualBufferDevice->DrawBitmap(
            Point(0, 0),
            bUseDimmed ? maDimmedDialogBitmap : maParentDialogBitmap);

        // get various options - sorry, no SvtOptionsDrawinglayer in vcl
        const Color aHilightColor(Application::GetSettings().GetStyleSettings().GetHighlightColor());
        const bool bIsAntiAliasing(true);
        const double fTransparence(0.4);
        const AntialiasingFlags nOldAA(mpVirtualBufferDevice->GetAntialiasing());

        if (bIsAntiAliasing)
        {
            mpVirtualBufferDevice->SetAntialiasing(AntialiasingFlags::EnableB2dDraw);
        }

        // paint selected entries
        for (auto candidate = maSelected.begin(); candidate != maSelected.end(); candidate++)
        {
            PaintControlDataEntry(**candidate, Color(COL_LIGHTRED), 3.0);
        }

        // paint hilighted entry
        if (mpHilighted && bPaintHilight)
        {
            PaintControlDataEntry(*mpHilighted, aHilightColor, 5.0, fTransparence);
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
            mpVirtualBufferDevice->GetBitmap(
            Point(0, 0),
            mpVirtualBufferDevice->GetOutputSizePixel())));
        mpPicture->Validate();
    }
}

IMPL_LINK_TYPED(ScreenshotAnnotationDlg_Impl, pictureFrameListener, VclWindowEvent&, rEvent, void)
{
    // event in picture frame
    bool bRepaint(false);

    switch (rEvent.GetId())
    {
    case VCLEVENT_WINDOW_MOUSEMOVE:
    case VCLEVENT_WINDOW_MOUSEBUTTONUP:
    {
        MouseEvent* pMouseEvent = static_cast< MouseEvent* >(rEvent.GetData());

        if (pMouseEvent)
        {
            switch (rEvent.GetId())
            {
            case VCLEVENT_WINDOW_MOUSEMOVE:
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
            case VCLEVENT_WINDOW_MOUSEBUTTONUP:
            {
                if (mpPicture->IsMouseOver() && mpHilighted)
                {
                    if (maSelected.find(mpHilighted) != maSelected.end())
                    {
                        maSelected.erase(mpHilighted);
                    }
                    else
                    {
                        maSelected.insert(mpHilighted);
                    }

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
