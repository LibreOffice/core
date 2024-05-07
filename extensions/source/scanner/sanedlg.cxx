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

#include <stdlib.h>
#include <o3tl/sprintf.hxx>
#include <tools/config.hxx>
#include <unotools/resmgr.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/customweld.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/event.hxx>
#include "sanedlg.hxx"
#include "grid.hxx"
#include <math.h>
#include <sal/macros.h>
#include <sal/log.hxx>
#include <rtl/strbuf.hxx>
#include <memory>
#include <strings.hrc>

#define PREVIEW_WIDTH       113
#define PREVIEW_HEIGHT      160

#define RECT_SIZE_PIX 7

namespace {

void DrawRect(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    tools::Rectangle aRect(rRect);
    rRenderContext.SetFillColor(COL_BLACK);
    rRenderContext.SetLineColor();
    rRenderContext.DrawRect(aRect);
    aRect.Move(1, 1);
    aRect.AdjustRight(-2);
    aRect.AdjustBottom(-2);
    rRenderContext.SetFillColor();
    rRenderContext.SetLineColor(COL_WHITE);
    rRenderContext.DrawRect(aRect);
}

void DrawRectangles(vcl::RenderContext& rRenderContext, Point const & rUL, Point const & rBR)
{
    Point aUR(rBR.X(), rUL.Y());
    Point aBL(rUL.X(), rBR.Y());
    int nMiddleX = (rBR.X() - rUL.X()) / 2 + rUL.X();
    int nMiddleY = (rBR.Y() - rUL.Y()) / 2 + rUL.Y();

    rRenderContext.SetLineColor(COL_WHITE);
    rRenderContext.DrawLine(rUL, aBL);
    rRenderContext.DrawLine(aBL, rBR);
    rRenderContext.DrawLine(rBR, aUR);
    rRenderContext.DrawLine(aUR, rUL);

    rRenderContext.SetLineColor(COL_BLACK);
    LineInfo aInfo(LineStyle::Dash, 1);
    aInfo.SetDistance(8);
    aInfo.SetDotLen(4);
    aInfo.SetDotCount(1);
    rRenderContext.DrawLine(rUL, aBL, aInfo);
    rRenderContext.DrawLine(aBL, rBR, aInfo);
    rRenderContext.DrawLine(rBR, aUR, aInfo);
    rRenderContext.DrawLine(aUR, rUL, aInfo);

    Size aSize(RECT_SIZE_PIX, RECT_SIZE_PIX);
    DrawRect(rRenderContext, tools::Rectangle(rUL, aSize));
    DrawRect(rRenderContext, tools::Rectangle(Point(aBL.X(), aBL.Y() - RECT_SIZE_PIX), aSize));
    DrawRect(rRenderContext, tools::Rectangle(Point(rBR.X() - RECT_SIZE_PIX, rBR.Y() - RECT_SIZE_PIX), aSize));
    DrawRect(rRenderContext, tools::Rectangle(Point(aUR.X() - RECT_SIZE_PIX, aUR.Y()), aSize));
    DrawRect(rRenderContext, tools::Rectangle(Point(nMiddleX - RECT_SIZE_PIX / 2, rUL.Y()), aSize));
    DrawRect(rRenderContext, tools::Rectangle(Point(nMiddleX - RECT_SIZE_PIX / 2, rBR.Y() - RECT_SIZE_PIX), aSize));
    DrawRect(rRenderContext, tools::Rectangle(Point(rUL.X(), nMiddleY - RECT_SIZE_PIX / 2), aSize));
    DrawRect(rRenderContext, tools::Rectangle(Point(rBR.X() - RECT_SIZE_PIX, nMiddleY - RECT_SIZE_PIX / 2), aSize));
}

}

class ScanPreview : public weld::CustomWidgetController
{
private:
    enum DragDirection { TopLeft, Top, TopRight, Right, BottomRight, Bottom,
                         BottomLeft, Left };

    BitmapEx  maPreviewBitmapEx;
    tools::Rectangle maPreviewRect;
    Point     maTopLeft, maBottomRight;
    Point     maMinTopLeft, maMaxBottomRight;
    SaneDlg*  mpParentDialog;
    DragDirection meDragDirection;
    bool      mbDragEnable;
    bool      mbIsDragging;

public:
    ScanPreview()
        : maMaxBottomRight(PREVIEW_WIDTH,  PREVIEW_HEIGHT)
        , mpParentDialog(nullptr)
        , meDragDirection(TopLeft)
        , mbDragEnable(false)
        , mbIsDragging(false)
    {
    }

    void Init(SaneDlg *pParent)
    {
        mpParentDialog = pParent;
    }

    void ResetForNewScanner()
    {
        maTopLeft = Point();
        maBottomRight = Point();
        maMinTopLeft = Point();
        maMaxBottomRight = Point(PREVIEW_WIDTH,  PREVIEW_HEIGHT);
    }

    void EnableDrag()
    {
        mbDragEnable = true;
    }

    void DisableDrag()
    {
        mbDragEnable = false;
    }

    bool IsDragEnabled() const
    {
        return mbDragEnable;
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    Point GetPixelPos(const Point& rIn) const;
    Point GetLogicPos(const Point& rIn) const;

    void GetPreviewLogicRect(Point& rTopLeft, Point &rBottomRight) const
    {
        rTopLeft = GetLogicPos(maTopLeft);
        rBottomRight = GetLogicPos(maBottomRight);
    }
    void GetMaxLogicRect(Point& rTopLeft, Point &rBottomRight) const
    {
        rTopLeft = maMinTopLeft;
        rBottomRight = maMaxBottomRight;

    }
    void ChangePreviewLogicTopLeftY(tools::Long Y)
    {
        Point aPoint(0, Y);
        aPoint = GetPixelPos(aPoint);
        maTopLeft.setY( aPoint.Y() );
    }
    void ChangePreviewLogicTopLeftX(tools::Long X)
    {
        Point aPoint(X, 0);
        aPoint = GetPixelPos(aPoint);
        maTopLeft.setX( aPoint.X() );
    }
    void ChangePreviewLogicBottomRightY(tools::Long Y)
    {
        Point aPoint(0, Y);
        aPoint = GetPixelPos(aPoint);
        maBottomRight.setY( aPoint.Y() );
    }
    void ChangePreviewLogicBottomRightX(tools::Long X)
    {
        Point aPoint(X, 0);
        aPoint = GetPixelPos(aPoint);
        maBottomRight.setX( aPoint.X() );
    }
    void SetPreviewLogicRect(const Point& rTopLeft, const Point &rBottomRight)
    {
        maTopLeft = GetPixelPos(rTopLeft);
        maBottomRight = GetPixelPos(rBottomRight);
        maPreviewRect = tools::Rectangle(maTopLeft,
                                  Size(maBottomRight.X() - maTopLeft.X(),
                                       maBottomRight.Y() - maTopLeft.Y()));
    }
    void SetPreviewMaxRect(const Point& rTopLeft, const Point &rBottomRight)
    {
        maMinTopLeft = rTopLeft;
        maMaxBottomRight = rBottomRight;
    }
    void DrawDrag(vcl::RenderContext& rRenderContext);
    void UpdatePreviewBounds();
    void SetBitmap(SvStream &rStream)
    {
        ReadDIBBitmapEx(maPreviewBitmapEx, rStream, true);
    }
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override
    {
        Size aSize(pDrawingArea->get_ref_device().LogicToPixel(Size(PREVIEW_WIDTH, PREVIEW_HEIGHT), MapMode(MapUnit::MapAppFont)));
        aSize.setWidth(aSize.getWidth()+1);
        aSize.setHeight(aSize.getHeight()+1);
        pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
        CustomWidgetController::SetDrawingArea(pDrawingArea);
        SetOutputSizePixel(aSize);
    }
};

SaneDlg::SaneDlg(weld::Window* pParent, Sane& rSane, bool bScanEnabled)
    : GenericDialogController(pParent, u"modules/scanner/ui/sanedialog.ui"_ustr, u"SaneDialog"_ustr)
    , mpParent(pParent)
    , mrSane(rSane)
    , mbScanEnabled(bScanEnabled)
    , mnCurrentOption(0)
    , mnCurrentElement(0)
    , mfMin(0.0)
    , mfMax(0.0)
    , doScan(false)
    , mxCancelButton(m_xBuilder->weld_button(u"cancel"_ustr))
    , mxDeviceInfoButton(m_xBuilder->weld_button(u"deviceInfoButton"_ustr))
    , mxPreviewButton(m_xBuilder->weld_button(u"previewButton"_ustr))
    , mxScanButton(m_xBuilder->weld_button(u"ok"_ustr))
    , mxButtonOption(m_xBuilder->weld_button(u"optionsButton"_ustr))
    , mxOptionTitle(m_xBuilder->weld_label(u"optionTitleLabel"_ustr))
    , mxOptionDescTxt(m_xBuilder->weld_label(u"optionsDescLabel"_ustr))
    , mxVectorTxt(m_xBuilder->weld_label(u"vectorLabel"_ustr))
    , mxLeftField(m_xBuilder->weld_metric_spin_button(u"leftSpinbutton"_ustr, FieldUnit::PIXEL))
    , mxTopField(m_xBuilder->weld_metric_spin_button(u"topSpinbutton"_ustr, FieldUnit::PIXEL))
    , mxRightField(m_xBuilder->weld_metric_spin_button(u"rightSpinbutton"_ustr, FieldUnit::PIXEL))
    , mxBottomField(m_xBuilder->weld_metric_spin_button(u"bottomSpinbutton"_ustr, FieldUnit::PIXEL))
    , mxDeviceBox(m_xBuilder->weld_combo_box(u"deviceCombobox"_ustr))
    , mxReslBox(m_xBuilder->weld_combo_box(u"reslCombobox"_ustr))
    , mxAdvancedBox(m_xBuilder->weld_check_button(u"advancedCheckbutton"_ustr))
    , mxVectorBox(m_xBuilder->weld_spin_button(u"vectorSpinbutton"_ustr))
    , mxQuantumRangeBox(m_xBuilder->weld_combo_box(u"quantumRangeCombobox"_ustr))
    , mxStringRangeBox(m_xBuilder->weld_combo_box(u"stringRangeCombobox"_ustr))
    , mxBoolCheckBox(m_xBuilder->weld_check_button(u"boolCheckbutton"_ustr))
    , mxStringEdit(m_xBuilder->weld_entry(u"stringEntry"_ustr))
    , mxNumericEdit(m_xBuilder->weld_entry(u"numericEntry"_ustr))
    , mxOptionBox(m_xBuilder->weld_tree_view(u"optionSvTreeListBox"_ustr))
    , mxPreview(new ScanPreview)
    , mxPreviewWnd(new weld::CustomWeld(*m_xBuilder, u"preview"_ustr, *mxPreview))
{
    Size aSize(mxOptionBox->get_approximate_digit_width() * 32, mxOptionBox->get_height_rows(8));
    mxOptionTitle->set_size_request(aSize.Width(), aSize.Height() / 2);
    mxOptionBox->set_size_request(aSize.Width(), aSize.Height());
    mxPreview->Init(this);
    if( Sane::IsSane() )
    {
        InitDevices(); // opens first sane device
        DisableOption();
        InitFields();
    }

    mxDeviceInfoButton->connect_clicked( LINK( this, SaneDlg, ClickBtnHdl ) );
    mxPreviewButton->connect_clicked( LINK( this, SaneDlg, ClickBtnHdl ) );
    mxScanButton->connect_clicked( LINK( this, SaneDlg, ClickBtnHdl ) );
    mxButtonOption->connect_clicked( LINK( this, SaneDlg, ClickBtnHdl ) );
    mxDeviceBox->connect_changed( LINK( this, SaneDlg, SelectHdl ) );
    mxOptionBox->connect_changed( LINK( this, SaneDlg, OptionsBoxSelectHdl ) );
    mxCancelButton->connect_clicked( LINK( this, SaneDlg, ClickBtnHdl ) );
    mxBoolCheckBox->connect_toggled( LINK( this, SaneDlg, ToggleBtnHdl ) );
    mxStringEdit->connect_changed( LINK( this, SaneDlg, ModifyHdl ) );
    mxNumericEdit->connect_changed( LINK( this, SaneDlg, ModifyHdl ) );
    mxVectorBox->connect_changed( LINK( this, SaneDlg, ModifyHdl ) );
    mxReslBox->connect_changed( LINK( this, SaneDlg, ValueModifyHdl ) );
    mxStringRangeBox->connect_changed( LINK( this, SaneDlg, SelectHdl ) );
    mxQuantumRangeBox->connect_changed( LINK( this, SaneDlg, SelectHdl ) );
    mxLeftField->connect_value_changed( LINK( this, SaneDlg, MetricValueModifyHdl ) );
    mxRightField->connect_value_changed( LINK( this, SaneDlg, MetricValueModifyHdl) );
    mxTopField->connect_value_changed( LINK( this, SaneDlg, MetricValueModifyHdl) );
    mxBottomField->connect_value_changed( LINK( this, SaneDlg, MetricValueModifyHdl) );
    mxAdvancedBox->connect_toggled( LINK( this, SaneDlg, ToggleBtnHdl ) );

    maOldLink = mrSane.SetReloadOptionsHdl( LINK( this, SaneDlg, ReloadSaneOptionsHdl ) );
}

SaneDlg::~SaneDlg()
{
    mrSane.SetReloadOptionsHdl(maOldLink);
}

namespace {

OUString SaneResId(TranslateId aID)
{
    return Translate::get(aID, Translate::Create("pcr"));
}

}

short SaneDlg::run()
{
    if (!Sane::IsSane())
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(mpParent,
                                                       VclMessageType::Warning, VclButtonsType::Ok,
                                                       SaneResId(STR_COULD_NOT_BE_INIT)));
        xErrorBox->run();
        return RET_CANCEL;
    }
    LoadState();
    return GenericDialogController::run();
}

void SaneDlg::InitDevices()
{
    if( ! Sane::IsSane() )
        return;

    if( mrSane.IsOpen() )
        mrSane.Close();
    mrSane.ReloadDevices();
    mxDeviceBox->clear();
    for (int i = 0; i < Sane::CountDevices(); ++i)
        mxDeviceBox->append_text(Sane::GetName(i));
    if( Sane::CountDevices() )
    {
        mrSane.Open(0);
        mxDeviceBox->set_active(0);
    }
}

void SaneDlg::InitFields()
{
    if( ! Sane::IsSane() )
        return;

    int nOption, i, nValue;
    double fValue;
    const char *ppSpecialOptions[] = {
        "resolution",
        "tl-x",
        "tl-y",
        "br-x",
        "br-y",
        "preview"
    };

    mxPreview->EnableDrag();
    mxReslBox->clear();
    Point aTopLeft, aBottomRight;
    mxPreview->GetPreviewLogicRect(aTopLeft, aBottomRight);
    Point aMinTopLeft, aMaxBottomRight;
    mxPreview->GetMaxLogicRect(aMinTopLeft, aMaxBottomRight);
    mxScanButton->set_visible( mbScanEnabled );

    if( ! mrSane.IsOpen() )
        return;

    // set Resolution
    nOption = mrSane.GetOptionByName( "resolution" );
    if( nOption != -1 )
    {
        double fRes;

        if( mrSane.GetOptionValue( nOption, fRes ) )
        {
            mxReslBox->set_sensitive(true);

            mxReslBox->set_entry_text(OUString::number(static_cast<sal_uInt32>(fRes)));
            std::unique_ptr<double[]> pDouble;
            nValue = mrSane.GetRange( nOption, pDouble );
            if( nValue > -1 )
            {
                assert(pDouble);
                if( nValue )
                {
                    for( i=0; i<nValue; i++ )
                    {
                        if( i == 0 || i == nValue-1 || ! ( static_cast<int>(pDouble[i]) % 20) )
                            mxReslBox->append_text(OUString::number(static_cast<sal_uInt32>(pDouble[i])));
                    }
                }
                else
                {
                    mxReslBox->append_text(OUString::number(static_cast<sal_uInt32>(pDouble[0])));
                    // Can only select 75 and 2400 dpi in Scanner dialogue
                    // scanner allows random setting of dpi resolution, a slider might be useful
                    // support that
                    // workaround: offer at least some more standard dpi resolution between
                    // min and max value
                    int bGot300 = 0;
                    for (sal_uInt32 nRes = static_cast<sal_uInt32>(pDouble[0]) * 2; nRes < static_cast<sal_uInt32>(pDouble[1]); nRes = nRes * 2)
                    {
                        if ( !bGot300 && nRes > 300 ) {
                            nRes = 300; bGot300 = 1;
                        }
                        mxReslBox->append_text(OUString::number(nRes));
                    }
                    mxReslBox->append_text(OUString::number(static_cast<sal_uInt32>(pDouble[1])));
                }
            }
            else
                mxReslBox->set_sensitive( false );
        }
    }
    else
        mxReslBox->set_sensitive( false );

    // set scan area
    for( i = 0; i < 4; i++ )
    {
        char const *pOptionName = nullptr;
        weld::MetricSpinButton* pField = nullptr;
        switch( i )
        {
            case 0:
                pOptionName = "tl-x";
                pField = mxLeftField.get();
                break;
            case 1:
                pOptionName = "tl-y";
                pField = mxTopField.get();
                break;
            case 2:
                pOptionName = "br-x";
                pField = mxRightField.get();
                break;
            case 3:
                pOptionName = "br-y";
                pField = mxBottomField.get();
        }
        nOption = pOptionName ? mrSane.GetOptionByName( pOptionName ) : -1;
        if( nOption != -1 )
        {
            if( mrSane.GetOptionValue( nOption, fValue ) )
            {
                if( mrSane.GetOptionUnit( nOption ) == SANE_UNIT_MM )
                {
                    pField->set_unit( FieldUnit::MM );
                    pField->set_value( static_cast<int>(fValue), FieldUnit::MM );
                }
                else // SANE_UNIT_PIXEL
                {
                    pField->set_unit( FieldUnit::PIXEL );
                    pField->set_value( static_cast<int>(fValue), FieldUnit::PIXEL );
                }
                switch( i ) {
                    case 0: aTopLeft.setX( static_cast<int>(fValue) );break;
                    case 1: aTopLeft.setY( static_cast<int>(fValue) );break;
                    case 2: aBottomRight.setX( static_cast<int>(fValue) );break;
                    case 3: aBottomRight.setY( static_cast<int>(fValue) );break;
                }
            }
            std::unique_ptr<double[]> pDouble;
            nValue = mrSane.GetRange( nOption, pDouble );
            if( nValue > -1 )
            {
                if( pDouble )
                {
                    pField->set_min( static_cast<tools::Long>(pDouble[0]), FieldUnit::NONE );
                    if( nValue )
                        pField->set_max( static_cast<tools::Long>(pDouble[ nValue-1 ]), FieldUnit::NONE );
                    else
                        pField->set_max( static_cast<tools::Long>(pDouble[ 1 ]), FieldUnit::NONE );
                }
                switch( i ) {
                    case 0: aMinTopLeft.setX( pField->get_min(FieldUnit::NONE) );break;
                    case 1: aMinTopLeft.setY( pField->get_min(FieldUnit::NONE) );break;
                    case 2: aMaxBottomRight.setX( pField->get_max(FieldUnit::NONE) );break;
                    case 3: aMaxBottomRight.setY( pField->get_max(FieldUnit::NONE) );break;
                }
            }
            else
            {
                switch( i ) {
                    case 0: aMinTopLeft.setX( static_cast<int>(fValue) );break;
                    case 1: aMinTopLeft.setY( static_cast<int>(fValue) );break;
                    case 2: aMaxBottomRight.setX( static_cast<int>(fValue) );break;
                    case 3: aMaxBottomRight.setY( static_cast<int>(fValue) );break;
                }
            }
            pField->set_sensitive(true);
        }
        else
        {
            mxPreview->DisableDrag();
            pField->set_min( 0, FieldUnit::NONE );
            switch( i ) {
                case 0:
                    aMinTopLeft.setX( 0 );
                    aTopLeft.setX( 0 );
                    pField->set_max( PREVIEW_WIDTH, FieldUnit::NONE );
                    pField->set_value( 0, FieldUnit::NONE );
                    break;
                case 1:
                    aMinTopLeft.setY( 0 );
                    aTopLeft.setY( 0 );
                    pField->set_max( PREVIEW_HEIGHT, FieldUnit::NONE );
                    pField->set_value( 0, FieldUnit::NONE );
                    break;
                case 2:
                    aMaxBottomRight.setX( PREVIEW_WIDTH );
                    aBottomRight.setX( PREVIEW_WIDTH );
                    pField->set_max( PREVIEW_WIDTH, FieldUnit::NONE );
                    pField->set_value( PREVIEW_WIDTH, FieldUnit::NONE );
                    break;
                case 3:
                    aMaxBottomRight.setY( PREVIEW_HEIGHT );
                    aBottomRight.setY( PREVIEW_HEIGHT );
                    pField->set_max( PREVIEW_HEIGHT, FieldUnit::NONE );
                    pField->set_value( PREVIEW_HEIGHT, FieldUnit::NONE );
                    break;
            }
            pField->set_sensitive(false);
        }
    }

    mxPreview->SetPreviewMaxRect(aMinTopLeft, aMaxBottomRight);
    mxPreview->SetPreviewLogicRect(aTopLeft, aBottomRight);
    mxPreview->Invalidate();

    // fill OptionBox
    mxOptionBox->clear();
    std::unique_ptr<weld::TreeIter> xParentEntry(mxOptionBox->make_iterator());
    bool bGroupRejected = false;
    for( i = 1; i < mrSane.CountOptions(); i++ )
    {
        OUString aOption=mrSane.GetOptionName( i );
        bool bInsertAdvanced =
            (mrSane.GetOptionCap( i ) & SANE_CAP_ADVANCED) == 0 ||
            mxAdvancedBox->get_active();
        if( mrSane.GetOptionType( i ) == SANE_TYPE_GROUP )
        {
            if( bInsertAdvanced )
            {
                aOption = mrSane.GetOptionTitle( i );
                mxOptionBox->append(xParentEntry.get());
                mxOptionBox->set_text(*xParentEntry, aOption, 0);
                bGroupRejected = false;
            }
            else
                bGroupRejected = true;
        }
        else if( !aOption.isEmpty() &&
                 ! ( mrSane.GetOptionCap( i ) &
                     (
                         SANE_CAP_HARD_SELECT |
                         SANE_CAP_INACTIVE
                         ) ) &&
                 bInsertAdvanced && ! bGroupRejected )
        {
            bool bIsSpecial = false;
            for( size_t n = 0; !bIsSpecial &&
                     n < SAL_N_ELEMENTS(ppSpecialOptions); n++ )
            {
                if( aOption == OUString::createFromAscii(ppSpecialOptions[n]) )
                    bIsSpecial=true;
            }
            if( ! bIsSpecial )
            {
                if (xParentEntry)
                    mxOptionBox->append(xParentEntry.get(), aOption);
                else
                    mxOptionBox->append_text(aOption);
            }
        }
    }
}

IMPL_LINK( SaneDlg, ClickBtnHdl, weld::Button&, rButton, void )
{
    if( mrSane.IsOpen() )
    {
        if( &rButton == mxDeviceInfoButton.get() )
        {
            OUString aString(SaneResId(STR_DEVICE_DESC));
            aString = aString.replaceFirst( "%s", Sane::GetName( mrSane.GetDeviceNumber() ) );
            aString = aString.replaceFirst( "%s", Sane::GetVendor( mrSane.GetDeviceNumber() ) );
            aString = aString.replaceFirst( "%s", Sane::GetModel( mrSane.GetDeviceNumber() ) );
            aString = aString.replaceFirst( "%s", Sane::GetType( mrSane.GetDeviceNumber() ) );
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          aString));
            xInfoBox->run();
        }
        else if( &rButton == mxPreviewButton.get() )
            AcquirePreview();
        else if( &rButton == mxButtonOption.get() )
        {

            SANE_Value_Type nType = mrSane.GetOptionType( mnCurrentOption );
            switch( nType )
            {
                case SANE_TYPE_BUTTON:
                    mrSane.ActivateButtonOption( mnCurrentOption );
                    break;
                case SANE_TYPE_FIXED:
                case SANE_TYPE_INT:
                {
                    int nElements = mrSane.GetOptionElements( mnCurrentOption );
                    std::unique_ptr<double[]> x(new double[ nElements ]);
                    std::unique_ptr<double[]> y(new double[ nElements ]);
                    for( int i = 0; i < nElements; i++ )
                        x[ i ] = static_cast<double>(i);
                    mrSane.GetOptionValue( mnCurrentOption, y.get() );

                    GridDialog aGrid(m_xDialog.get(), x.get(), y.get(), nElements);
                    aGrid.set_title( mrSane.GetOptionName( mnCurrentOption ) );
                    aGrid.setBoundings( 0, mfMin, nElements, mfMax );
                    if (aGrid.run() && aGrid.getNewYValues())
                        mrSane.SetOptionValue( mnCurrentOption, aGrid.getNewYValues() );
                }
                break;
                case SANE_TYPE_BOOL:
                case SANE_TYPE_STRING:
                case SANE_TYPE_GROUP:
                    break;
            }
        }
    }
    if (&rButton == mxScanButton.get())
    {
        double fRes = static_cast<double>(mxReslBox->get_active_text().toUInt32());
        SetAdjustedNumericalValue( "resolution", fRes );
        UpdateScanArea(true);
        SaveState();
        m_xDialog->response(mrSane.IsOpen() ? RET_OK : RET_CANCEL);
        doScan = mrSane.IsOpen();
    }
    else if( &rButton == mxCancelButton.get() )
    {
        mrSane.Close();
        m_xDialog->response(RET_CANCEL);
    }
}

IMPL_LINK( SaneDlg, ToggleBtnHdl, weld::Toggleable&, rButton, void )
{
    if( mrSane.IsOpen() )
    {
        if( &rButton == mxBoolCheckBox.get() )
        {
            mrSane.SetOptionValue( mnCurrentOption,
                                   mxBoolCheckBox->get_active() );
        }
        else if( &rButton == mxAdvancedBox.get() )
        {
            ReloadSaneOptionsHdl( mrSane );
        }
    }
}

IMPL_LINK( SaneDlg, SelectHdl, weld::ComboBox&, rListBox, void )
{
    if( &rListBox == mxDeviceBox.get() && Sane::IsSane() && Sane::CountDevices() )
    {
        int nNewNumber = mxDeviceBox->get_active();
        int nOldNumber = mrSane.GetDeviceNumber();
        if (nNewNumber != nOldNumber)
        {
            mrSane.Close();
            mrSane.Open(nNewNumber);
            mxPreview->ResetForNewScanner();
            InitFields();
        }
    }
    if( mrSane.IsOpen() )
    {
        if( &rListBox == mxQuantumRangeBox.get() )
        {
            double fValue = mxQuantumRangeBox->get_active_text().toDouble();
            mrSane.SetOptionValue(mnCurrentOption, fValue, mnCurrentElement);
        }
        else if( &rListBox == mxStringRangeBox.get() )
        {
            mrSane.SetOptionValue(mnCurrentOption, mxStringRangeBox->get_active_text());
        }
    }
}

IMPL_LINK_NOARG(SaneDlg, OptionsBoxSelectHdl, weld::TreeView&, void)
{
    if (!Sane::IsSane())
        return;

    OUString aOption = mxOptionBox->get_selected_text();
    int nOption = mrSane.GetOptionByName(OUStringToOString(aOption,
        osl_getThreadTextEncoding()).getStr());
    if( nOption == -1 || nOption == mnCurrentOption )
        return;

    DisableOption();
    mnCurrentOption = nOption;
    mxOptionTitle->set_label(mrSane.GetOptionTitle(mnCurrentOption));
    SANE_Value_Type nType = mrSane.GetOptionType( mnCurrentOption );
    SANE_Constraint_Type nConstraint;
    switch( nType )
    {
        case SANE_TYPE_BOOL:    EstablishBoolOption();break;
        case SANE_TYPE_STRING:
            nConstraint = mrSane.GetOptionConstraintType( mnCurrentOption );
            if( nConstraint == SANE_CONSTRAINT_STRING_LIST )
                EstablishStringRange();
            else
                EstablishStringOption();
            break;
        case SANE_TYPE_FIXED:
        case SANE_TYPE_INT:
        {
            nConstraint = mrSane.GetOptionConstraintType( mnCurrentOption );
            int nElements = mrSane.GetOptionElements( mnCurrentOption );
            mnCurrentElement = 0;
            if( nConstraint == SANE_CONSTRAINT_RANGE ||
                nConstraint == SANE_CONSTRAINT_WORD_LIST )
                EstablishQuantumRange();
            else
            {
                mfMin = mfMax = 0.0;
                EstablishNumericOption();
            }
            if( nElements > 1 )
            {
                if( nElements <= 10 )
                {
                    mxVectorBox->set_range(1, mrSane.GetOptionElements(mnCurrentOption));
                    mxVectorBox->set_value(1);
                    mxVectorBox->show();
                    mxVectorTxt->show();
                }
                else
                {
                    DisableOption();
                    // bring up dialog only on button click
                    EstablishButtonOption();
                }
            }
        }
        break;
        case SANE_TYPE_BUTTON:
            EstablishButtonOption();
            break;
        default: break;
    }
}

IMPL_LINK(SaneDlg, ModifyHdl, weld::Entry&, rEdit, void)
{
    if( !mrSane.IsOpen() )
        return;

    if (&rEdit == mxStringEdit.get())
    {
        mrSane.SetOptionValue( mnCurrentOption, mxStringEdit->get_text() );
    }
    else if (&rEdit == mxNumericEdit.get())
    {
        double fValue = mxNumericEdit->get_text().toDouble();
        if( mfMin != mfMax && ( fValue < mfMin || fValue > mfMax ) )
        {
            char pBuf[256];
            if( fValue < mfMin )
                fValue = mfMin;
            else if( fValue > mfMax )
                fValue = mfMax;
            o3tl::sprintf( pBuf, "%g", fValue );
            mxNumericEdit->set_text( OUString( pBuf, strlen(pBuf), osl_getThreadTextEncoding() ) );
        }
        mrSane.SetOptionValue( mnCurrentOption, fValue, mnCurrentElement );
    }
    else if (&rEdit == mxVectorBox.get())
    {
        mnCurrentElement = mxVectorBox->get_value() - 1;
        double fValue;
        if( mrSane.GetOptionValue( mnCurrentOption, fValue, mnCurrentElement ))
        {
            char pBuf[256];
            o3tl::sprintf( pBuf, "%g", fValue );
            OUString aValue( pBuf, strlen(pBuf), osl_getThreadTextEncoding() );
            mxNumericEdit->set_text( aValue );
            mxQuantumRangeBox->set_active_text( aValue );
        }
    }
}

IMPL_LINK(SaneDlg, ValueModifyHdl, weld::ComboBox&, rEdit, void)
{
    if( !mrSane.IsOpen() )
        return;

    if (&rEdit != mxReslBox.get())
        return;

    double fRes = static_cast<double>(mxReslBox->get_active_text().toUInt32());
    int nOption = mrSane.GetOptionByName( "resolution" );
    if( nOption == -1 )
        return;

    std::unique_ptr<double[]> pDouble;
    int nValues = mrSane.GetRange( nOption, pDouble );
    if( nValues > 0 )
    {
        int i;
        for( i = 0; i < nValues; i++ )
        {
            if( fRes == pDouble[i] )
                break;
        }
        if( i >= nValues )
            fRes = pDouble[0];
    }
    else if( nValues == 0 )
    {
        if( fRes < pDouble[ 0 ] )
            fRes = pDouble[ 0 ];
        if( fRes > pDouble[ 1 ] )
            fRes = pDouble[ 1 ];
    }
    mxReslBox->set_entry_text(OUString::number(static_cast<sal_uInt32>(fRes)));
}

IMPL_LINK(SaneDlg, MetricValueModifyHdl, weld::MetricSpinButton&, rEdit, void)
{
    if( !mrSane.IsOpen() )
        return;

    if (&rEdit == mxTopField.get())
    {
        mxPreview->ChangePreviewLogicTopLeftY(mxTopField->get_value(FieldUnit::NONE));
        mxPreview->Invalidate();
    }
    else if (&rEdit == mxLeftField.get())
    {
        mxPreview->ChangePreviewLogicTopLeftX(mxLeftField->get_value(FieldUnit::NONE));
        mxPreview->Invalidate();
    }
    else if (&rEdit == mxBottomField.get())
    {
        mxPreview->ChangePreviewLogicBottomRightY(mxBottomField->get_value(FieldUnit::NONE));
        mxPreview->Invalidate();
    }
    else if (&rEdit == mxRightField.get())
    {
        mxPreview->ChangePreviewLogicBottomRightX(mxRightField->get_value(FieldUnit::NONE));
        mxPreview->Invalidate();
    }
}

IMPL_LINK_NOARG( SaneDlg, ReloadSaneOptionsHdl, Sane&, void )
{
    mnCurrentOption = -1;
    mnCurrentElement = 0;
    DisableOption();
    InitFields();
    mxPreview->Invalidate();
}

void SaneDlg::AcquirePreview()
{
    if( ! mrSane.IsOpen() )
        return;

    UpdateScanArea( true );
    // set small resolution for preview
    double fResl = static_cast<double>(mxReslBox->get_active_text().toUInt32());
    SetAdjustedNumericalValue( "resolution", 30.0 );

    int nOption = mrSane.GetOptionByName( "preview" );
    if( nOption == -1 )
    {
        OUString aString(SaneResId(STR_SLOW_PREVIEW));
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                  VclMessageType::Warning, VclButtonsType::OkCancel,
                                                  aString));
        if (xBox->run() == RET_CANCEL)
            return;
    }
    else
        mrSane.SetOptionValue( nOption, true );

    rtl::Reference<BitmapTransporter> xTransporter(new BitmapTransporter);
    if (!mrSane.Start(*xTransporter))
    {
        std::unique_ptr<weld::MessageDialog> xErrorBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                       VclMessageType::Warning, VclButtonsType::Ok,
                                                       SaneResId(STR_ERROR_SCAN)));
        xErrorBox->run();
    }
    else
    {
#if OSL_DEBUG_LEVEL > 0
        SAL_INFO("extensions.scanner", "Previewbitmapstream contains " << xTransporter->getStream().TellEnd() << "bytes");
#endif
        xTransporter->getStream().Seek( STREAM_SEEK_TO_BEGIN );
        mxPreview->SetBitmap(xTransporter->getStream());
    }

    SetAdjustedNumericalValue( "resolution", fResl );
    mxReslBox->set_entry_text(OUString::number(static_cast<sal_uInt32>(fResl)));

    mxPreview->UpdatePreviewBounds();
    mxPreview->Invalidate();
}

void ScanPreview::UpdatePreviewBounds()
{
    if( mbDragEnable )
    {
        maPreviewRect = tools::Rectangle( maTopLeft,
                                   Size( maBottomRight.X() - maTopLeft.X(),
                                         maBottomRight.Y() - maTopLeft.Y() )
                                   );
    }
    else
    {
        Size aBMSize( maPreviewBitmapEx.GetSizePixel() );
        if( aBMSize.Width() > aBMSize.Height() && aBMSize.Width() )
        {
            int nVHeight = (maBottomRight.X() - maTopLeft.X()) * aBMSize.Height() / aBMSize.Width();
            maPreviewRect = tools::Rectangle( Point( maTopLeft.X(), ( maTopLeft.Y() + maBottomRight.Y() )/2 - nVHeight/2 ),
                                       Size( maBottomRight.X() - maTopLeft.X(),
                                             nVHeight ) );
        }
        else if (aBMSize.Height())
        {
            int nVWidth = (maBottomRight.Y() - maTopLeft.Y()) * aBMSize.Width() / aBMSize.Height();
            maPreviewRect = tools::Rectangle( Point( ( maTopLeft.X() + maBottomRight.X() )/2 - nVWidth/2, maTopLeft.Y() ),
                                       Size( nVWidth,
                                             maBottomRight.Y() - maTopLeft.Y() ) );
        }
    }
}

void ScanPreview::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    rRenderContext.SetMapMode(MapMode(MapUnit::MapAppFont));
    rRenderContext.SetFillColor(COL_WHITE);
    rRenderContext.SetLineColor(COL_WHITE);
    rRenderContext.DrawRect(tools::Rectangle(Point(0, 0),
                                      Size(PREVIEW_WIDTH, PREVIEW_HEIGHT)));
    rRenderContext.SetMapMode(MapMode(MapUnit::MapPixel));
    // check for sane values
    rRenderContext.DrawBitmapEx(maPreviewRect.TopLeft(), maPreviewRect.GetSize(), maPreviewBitmapEx);

    DrawDrag(rRenderContext);
}

void SaneDlg::DisableOption()
{
    mxBoolCheckBox->hide();
    mxStringEdit->hide();
    mxNumericEdit->hide();
    mxQuantumRangeBox->hide();
    mxStringRangeBox->hide();
    mxButtonOption->hide();
    mxVectorBox->hide();
    mxVectorTxt->hide();
    mxOptionDescTxt->hide();
}

void SaneDlg::EstablishBoolOption()
{
    bool bSuccess, bValue;

    bSuccess = mrSane.GetOptionValue( mnCurrentOption, bValue );
    if( bSuccess )
    {
        mxBoolCheckBox->set_label( mrSane.GetOptionName( mnCurrentOption ) );
        mxBoolCheckBox->set_active( bValue );
        mxBoolCheckBox->show();
    }
}

void SaneDlg::EstablishStringOption()
{
    bool bSuccess;
    OString aValue;

    bSuccess = mrSane.GetOptionValue( mnCurrentOption, aValue );
    if( bSuccess )
    {
        mxOptionDescTxt->set_label( mrSane.GetOptionName( mnCurrentOption ) );
        mxOptionDescTxt->show();
        mxStringEdit->set_text(OStringToOUString(aValue, osl_getThreadTextEncoding()));
        mxStringEdit->show();
    }
}

void SaneDlg::EstablishStringRange()
{
    const char** ppStrings = mrSane.GetStringConstraint( mnCurrentOption );
    mxStringRangeBox->clear();
    for( int i = 0; ppStrings[i] != nullptr; i++ )
        mxStringRangeBox->append_text( OUString( ppStrings[i], strlen(ppStrings[i]), osl_getThreadTextEncoding() ) );
    OString aValue;
    mrSane.GetOptionValue( mnCurrentOption, aValue );
    mxStringRangeBox->set_active_text(OStringToOUString(aValue, osl_getThreadTextEncoding()));
    mxStringRangeBox->show();
    mxOptionDescTxt->set_label( mrSane.GetOptionName( mnCurrentOption ) );
    mxOptionDescTxt->show();
}

void SaneDlg::EstablishQuantumRange()
{
    mpRange.reset();
    int nValues = mrSane.GetRange( mnCurrentOption, mpRange );
    if( nValues == 0 )
    {
        mfMin = mpRange[ 0 ];
        mfMax = mpRange[ 1 ];
        mpRange.reset();
        EstablishNumericOption();
    }
    else if( nValues > 0 )
    {
        char pBuf[ 256 ];
        mxQuantumRangeBox->clear();
        mfMin = mpRange[ 0 ];
        mfMax = mpRange[ nValues-1 ];
        for( int i = 0; i < nValues; i++ )
        {
            o3tl::sprintf( pBuf, "%g", mpRange[ i ] );
            mxQuantumRangeBox->append_text( OUString( pBuf, strlen(pBuf), osl_getThreadTextEncoding() ) );
        }
        double fValue;
        if( mrSane.GetOptionValue( mnCurrentOption, fValue, mnCurrentElement ) )
        {
            o3tl::sprintf( pBuf, "%g", fValue );
            mxQuantumRangeBox->set_active_text( OUString( pBuf, strlen(pBuf), osl_getThreadTextEncoding() ) );
        }
        mxQuantumRangeBox->show();
        OUString aText = mrSane.GetOptionName( mnCurrentOption ) + " "
            + mrSane.GetOptionUnitName( mnCurrentOption );
        mxOptionDescTxt->set_label(aText);
        mxOptionDescTxt->show();
    }
}

void SaneDlg::EstablishNumericOption()
{
    bool bSuccess;
    double fValue;

    bSuccess = mrSane.GetOptionValue( mnCurrentOption, fValue );
    if( ! bSuccess )
        return;

    char pBuf[256];
    OUString aText = mrSane.GetOptionName( mnCurrentOption ) + " "
        + mrSane.GetOptionUnitName( mnCurrentOption );
    if( mfMin != mfMax )
    {
        o3tl::sprintf( pBuf, " < %g ; %g >", mfMin, mfMax );
        aText += OUString( pBuf, strlen(pBuf), osl_getThreadTextEncoding() );
    }
    mxOptionDescTxt->set_label( aText );
    mxOptionDescTxt->show();
    o3tl::sprintf( pBuf, "%g", fValue );
    mxNumericEdit->set_text( OUString( pBuf, strlen(pBuf), osl_getThreadTextEncoding() ) );
    mxNumericEdit->show();
}

void SaneDlg::EstablishButtonOption()
{
    mxOptionDescTxt->set_label(mrSane.GetOptionName(mnCurrentOption));
    mxOptionDescTxt->show();
    mxButtonOption->show();
}

bool ScanPreview::MouseMove(const MouseEvent& rMEvt)
{
    if( !mbIsDragging )
        return false;

    Point aMousePos = rMEvt.GetPosPixel();
    // move into valid area
    Point aLogicPos = GetLogicPos( aMousePos );
    aMousePos = GetPixelPos( aLogicPos );
    switch( meDragDirection )
    {
        case TopLeft:       maTopLeft = aMousePos; break;
        case Top:           maTopLeft.setY( aMousePos.Y() ); break;
        case TopRight:
            maTopLeft.setY( aMousePos.Y() );
            maBottomRight.setX( aMousePos.X() );
            break;
        case Right:         maBottomRight.setX( aMousePos.X() ); break;
        case BottomRight:   maBottomRight = aMousePos; break;
        case Bottom:        maBottomRight.setY( aMousePos.Y() ); break;
        case BottomLeft:
            maTopLeft.setX( aMousePos.X() );
            maBottomRight.setY( aMousePos.Y() );
            break;
        case Left:          maTopLeft.setX( aMousePos.X() ); break;
        default: break;
    }
    int nSwap;
    if( maTopLeft.X() > maBottomRight.X() )
    {
        nSwap = maTopLeft.X();
        maTopLeft.setX( maBottomRight.X() );
        maBottomRight.setX( nSwap );
    }
    if( maTopLeft.Y() > maBottomRight.Y() )
    {
        nSwap = maTopLeft.Y();
        maTopLeft.setY( maBottomRight.Y() );
        maBottomRight.setY( nSwap );
    }
    Invalidate();
    mpParentDialog->UpdateScanArea(false);
    return false;
}

bool ScanPreview::MouseButtonDown( const MouseEvent& rMEvt )
{
    if (!mbIsDragging && mbDragEnable)
    {
        Point aMousePixel = rMEvt.GetPosPixel();

        int nMiddleX = ( maBottomRight.X() - maTopLeft.X() ) / 2 - RECT_SIZE_PIX/2 + maTopLeft.X();
        int nMiddleY = ( maBottomRight.Y() - maTopLeft.Y() ) / 2 - RECT_SIZE_PIX/2 + maTopLeft.Y();
        if( aMousePixel.Y() >= maTopLeft.Y() &&
            aMousePixel.Y() < maTopLeft.Y() + RECT_SIZE_PIX )
        {
            if( aMousePixel.X() >= maTopLeft.X() &&
                aMousePixel.X() < maTopLeft.X() + RECT_SIZE_PIX )
            {
                meDragDirection = TopLeft;
                mbIsDragging = true;
            }
            else if( aMousePixel.X() >= nMiddleX &&
                     aMousePixel.X() < nMiddleX + RECT_SIZE_PIX )
            {
                meDragDirection = Top;
                mbIsDragging = true;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = TopRight;
                mbIsDragging = true;
            }
        }
        else if( aMousePixel.Y() >= nMiddleY &&
                 aMousePixel.Y() < nMiddleY + RECT_SIZE_PIX )
        {
            if( aMousePixel.X() >= maTopLeft.X() &&
                aMousePixel.X() < maTopLeft.X() + RECT_SIZE_PIX )
            {
                meDragDirection = Left;
                mbIsDragging = true;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = Right;
                mbIsDragging = true;
            }
        }
        else if( aMousePixel.Y() <= maBottomRight.Y() &&
                 aMousePixel.Y() > maBottomRight.Y() - RECT_SIZE_PIX )
        {
            if( aMousePixel.X() >= maTopLeft.X() &&
                aMousePixel.X() < maTopLeft.X() + RECT_SIZE_PIX )
            {
                meDragDirection = BottomLeft;
                mbIsDragging = true;
            }
            else if( aMousePixel.X() >= nMiddleX &&
                     aMousePixel.X() < nMiddleX + RECT_SIZE_PIX )
            {
                meDragDirection = Bottom;
                mbIsDragging = true;
            }
            else if( aMousePixel.X() > maBottomRight.X() - RECT_SIZE_PIX &&
                     aMousePixel.X() <= maBottomRight.X() )
            {
                meDragDirection = BottomRight;
                mbIsDragging = true;
            }
        }
    }

    if( mbIsDragging )
        Invalidate();

    return false;
}

bool ScanPreview::MouseButtonUp(const MouseEvent&)
{
    if( mbIsDragging )
        mpParentDialog->UpdateScanArea(true);
    mbIsDragging = false;

    return false;
}

void ScanPreview::DrawDrag(vcl::RenderContext& rRenderContext)
{
    if (!mbDragEnable)
        return;

    rRenderContext.SetMapMode(MapMode(MapUnit::MapPixel));

    DrawRectangles(rRenderContext, maTopLeft, maBottomRight);

    rRenderContext.SetMapMode(MapMode(MapUnit::MapAppFont));
}

Point ScanPreview::GetPixelPos( const Point& rIn) const
{
    Point aConvert(
        ( ( rIn.X() * PREVIEW_WIDTH ) /
          ( maMaxBottomRight.X() - maMinTopLeft.X() ) )
        ,
        ( ( rIn.Y() * PREVIEW_HEIGHT )
          / ( maMaxBottomRight.Y() - maMinTopLeft.Y() ) )
        );

    return GetDrawingArea()->get_ref_device().LogicToPixel(aConvert, MapMode(MapUnit::MapAppFont));
}

Point ScanPreview::GetLogicPos(const Point& rIn) const
{
    Point aConvert = GetDrawingArea()->get_ref_device().PixelToLogic(rIn, MapMode(MapUnit::MapAppFont));
    if( aConvert.X() < 0 )
        aConvert.setX( 0 );
    if( aConvert.X() >= PREVIEW_WIDTH )
        aConvert.setX( PREVIEW_WIDTH-1 );
    if( aConvert.Y() < 0 )
        aConvert.setY( 0 );
    if( aConvert.Y() >= PREVIEW_HEIGHT )
        aConvert.setY( PREVIEW_HEIGHT-1 );

    aConvert.setX( aConvert.X() * ( maMaxBottomRight.X() - maMinTopLeft.X() ) );
    aConvert.setX( aConvert.X() / ( PREVIEW_WIDTH) );
    aConvert.setY( aConvert.Y() * ( maMaxBottomRight.Y() - maMinTopLeft.Y() ) );
    aConvert.setY( aConvert.Y() / ( PREVIEW_HEIGHT) );
    return aConvert;
}

void SaneDlg::UpdateScanArea(bool bSend)
{
    if (!mxPreview->IsDragEnabled())
        return;

    Point aUL, aBR;
    mxPreview->GetPreviewLogicRect(aUL, aBR);

    mxLeftField->set_value(aUL.X(), FieldUnit::NONE);
    mxTopField->set_value(aUL.Y(), FieldUnit::NONE);
    mxRightField->set_value(aBR.X(), FieldUnit::NONE);
    mxBottomField->set_value(aBR.Y(), FieldUnit::NONE);

    if (!bSend)
        return;

    if( mrSane.IsOpen() )
    {
        SetAdjustedNumericalValue( "tl-x", static_cast<double>(aUL.X()) );
        SetAdjustedNumericalValue( "tl-y", static_cast<double>(aUL.Y()) );
        SetAdjustedNumericalValue( "br-x", static_cast<double>(aBR.X()) );
        SetAdjustedNumericalValue( "br-y", static_cast<double>(aBR.Y()) );
    }
}

bool SaneDlg::LoadState()
{
    int i;

    if( ! Sane::IsSane() )
        return false;

    const char* pEnv = getenv("HOME");
    OUString aFileName = (pEnv ? OUString(pEnv, strlen(pEnv), osl_getThreadTextEncoding() ) : OUString()) + "/.so_sane_state";
    Config aConfig( aFileName );
    if( ! aConfig.HasGroup( "SANE" ) )
        return false;

    aConfig.SetGroup( "SANE"_ostr );
    OString aString = aConfig.ReadKey( "SO_LastSaneDevice"_ostr );
    for( i = 0; i < Sane::CountDevices() && aString != OUStringToOString(Sane::GetName(i), osl_getThreadTextEncoding()); i++ ) ;
    if( i == Sane::CountDevices() )
        return false;

    mrSane.Close();
    mrSane.Open( aString.getStr() );

    DisableOption();
    InitFields();

    if( mrSane.IsOpen() )
    {
        int iMax = aConfig.GetKeyCount();
        for (i = 0; i < iMax; ++i)
        {
            aString = aConfig.GetKeyName( i );
            OString aValue = aConfig.ReadKey( i );
            int nOption = mrSane.GetOptionByName( aString.getStr() );
            if( nOption == -1 )
                continue;

            if (aValue.startsWith("BOOL="))
            {
                aValue = aValue.copy(RTL_CONSTASCII_LENGTH("BOOL="));
                bool aBOOL = aValue.toInt32() != 0;
                mrSane.SetOptionValue( nOption, aBOOL );
            }
            else if (aValue.startsWith("STRING="))
            {
                aValue = aValue.copy(RTL_CONSTASCII_LENGTH("STRING="));
                mrSane.SetOptionValue(nOption,OStringToOUString(aValue, osl_getThreadTextEncoding()) );
            }
            else if (aValue.startsWith("NUMERIC="))
            {
                aValue = aValue.copy(RTL_CONSTASCII_LENGTH("NUMERIC="));

                sal_Int32 nIndex = 0;
                int n = 0;
                do
                {
                    OString aSub = aValue.getToken(0, ':', nIndex);
                    double fValue=0.0;
                    sscanf(aSub.getStr(), "%lg", &fValue);
                    SetAdjustedNumericalValue(aString.getStr(), fValue, n++);
                }
                while ( nIndex >= 0 );
            }
        }
    }

    DisableOption();
    InitFields();

    return true;
}

void SaneDlg::SaveState()
{
    if( ! Sane::IsSane() )
        return;

    const char* pEnv = getenv( "HOME" );
    OUString aFileName;

    if( pEnv )
        aFileName = OUString::createFromAscii(pEnv) + "/.so_sane_state";
    else
        aFileName = OStringToOUString("", osl_getThreadTextEncoding()) + "/.so_sane_state";

    Config aConfig( aFileName );
    aConfig.DeleteGroup( "SANE" );
    aConfig.SetGroup( "SANE"_ostr );
    aConfig.WriteKey( "SO_LastSANEDevice"_ostr,
        OUStringToOString(mxDeviceBox->get_active_text(), RTL_TEXTENCODING_UTF8) );

    static char const* pSaveOptions[] = {
        "resolution",
        "tl-x",
        "tl-y",
        "br-x",
        "br-y"
    };
    for(const char * pSaveOption : pSaveOptions)
    {
        OString aOption = pSaveOption;
        int nOption = mrSane.GetOptionByName( pSaveOption );
        if( nOption > -1 )
        {
            SANE_Value_Type nType = mrSane.GetOptionType( nOption );
            switch( nType )
            {
                case SANE_TYPE_BOOL:
                {
                    bool bValue;
                    if( mrSane.GetOptionValue( nOption, bValue ) )
                    {
                        OString aString = "BOOL=" + OString::number(static_cast<sal_Int32>(bValue));
                        aConfig.WriteKey(aOption, aString);
                    }
                }
                break;
                case SANE_TYPE_STRING:
                {
                    OString aValue;
                    if( mrSane.GetOptionValue( nOption, aValue ) )
                    {
                        OString aString = "STRING=" + aValue;
                        aConfig.WriteKey( aOption, aString );
                    }
                }
                break;
                case SANE_TYPE_FIXED:
                case SANE_TYPE_INT:
                {
                    OStringBuffer aString("NUMERIC=");
                    double fValue;
                    char buf[256];
                    int n;

                    for( n = 0; n < mrSane.GetOptionElements( nOption ); n++ )
                    {
                        if( ! mrSane.GetOptionValue( nOption, fValue, n ) )
                            break;
                        if( n > 0 )
                            aString.append(':');
                        o3tl::sprintf( buf, "%lg", fValue );
                        aString.append(buf);
                    }
                    if( n >= mrSane.GetOptionElements( nOption ) )
                        aConfig.WriteKey( aOption, aString.makeStringAndClear() );
                }
                break;
                default:
                    break;
            }
        }
    }
}

bool SaneDlg::SetAdjustedNumericalValue(
    const char* pOption,
    double fValue,
    int nElement )
{
    if (! Sane::IsSane() || ! mrSane.IsOpen())
        return false;
    int const nOption(mrSane.GetOptionByName(pOption));
    if (nOption == -1)
        return false;

    if( nElement < 0 || nElement >= mrSane.GetOptionElements( nOption ) )
        return false;

    std::unique_ptr<double[]> pValues;
    int nValues;
    if( ( nValues = mrSane.GetRange( nOption, pValues ) ) < 0 )
    {
        return false;
    }

    SAL_INFO("extensions.scanner", "SaneDlg::SetAdjustedNumericalValue(\"" << pOption << "\", " << fValue << ") ");

    if( nValues )
    {
        int nNearest = 0;
        double fNearest = 1e6;
        for( int i = 0; i < nValues; i++ )
        {
            if( fabs( fValue - pValues[ i ] ) < fNearest )
            {
                fNearest = fabs( fValue - pValues[ i ] );
                nNearest = i;
            }
        }
        fValue = pValues[ nNearest ];
    }
    else
    {
        if( fValue < pValues[0] )
            fValue = pValues[0];
        if( fValue > pValues[1] )
            fValue = pValues[1];
    }
    mrSane.SetOptionValue( nOption, fValue, nElement );
    SAL_INFO("extensions.scanner", "yields " << fValue);


    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
