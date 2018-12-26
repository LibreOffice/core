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

#include "TableFieldDescWin.hxx"
#include <osl/diagnose.h>
#include <FieldDescriptions.hxx>
#include <strings.hrc>
#include <TableDesignHelpBar.hxx>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/settings.hxx>
#include <helpids.h>
#include <core_resource.hxx>

#define STANDARD_MARGIN                  6
#define DETAILS_HEADER_HEIGHT           25
#define CONTROL_SPACING_X   18  // 6
#define CONTROL_SPACING_Y   5
#define CONTROL_HEIGHT      20
#define CONTROL_WIDTH_1     140 // 100
#define CONTROL_WIDTH_3     250
#define CONTROL_WIDTH_4     (CONTROL_WIDTH_3 - CONTROL_HEIGHT - 5)
#define DETAILS_OPT_PAGE_WIDTH          (CONTROL_WIDTH_1 + CONTROL_SPACING_X + CONTROL_WIDTH_4 + 50)
#define DETAILS_OPT_PAGE_HEIGHT         ((CONTROL_HEIGHT + CONTROL_SPACING_Y) * 5)
#define DETAILS_MIN_HELP_WIDTH          100
#define DETAILS_OPT_HELP_WIDTH          200
#define DETAILS_MIN_HELP_HEIGHT         50
#define DETAILS_OPT_HELP_HEIGHT         100

using namespace dbaui;
// class OTableFieldDescWin
OTableFieldDescWin::OTableFieldDescWin( vcl::Window* pParent)
    : TabPage(pParent, WB_3DLOOK)
    , m_eChildFocus(NONE)
{
    // Header
    m_pHeader = VclPtr<FixedText>::Create( this, WB_CENTER );
    m_pHeader->SetText(DBA_RES(STR_TAB_PROPERTIES));
    m_pHeader->Show();

    // HelpBar
    m_pHelpBar = VclPtr<OTableDesignHelpBar>::Create( this );
    m_pHelpBar->SetHelpId(HID_TAB_DESIGN_HELP_TEXT_FRAME);
    m_pHelpBar->Show();

    m_pGenPage = VclPtr<OFieldDescGenWin>::Create( this, m_pHelpBar );
    getGenPage()->SetHelpId( HID_TABLE_DESIGN_TABPAGE_GENERAL );
    getGenPage()->Show();
}

OTableFieldDescWin::~OTableFieldDescWin()
{
    disposeOnce();
}

void OTableFieldDescWin::dispose()
{
    // destroy children
    m_pHelpBar->Hide();
    getGenPage()->Hide();
    m_pHeader->Hide();

    m_pGenPage.disposeAndClear();
    m_pHeader.disposeAndClear();
    m_pHelpBar.disposeAndClear();
    TabPage::dispose();
}

void OTableFieldDescWin::Init()
{
    OSL_ENSURE(getGenPage() != nullptr, "OTableFieldDescWin::Init : ups ... no GenericPage ... this will crash ...");
    getGenPage()->Init();
}

void OTableFieldDescWin::SetReadOnly( bool bRead )
{
    getGenPage()->SetReadOnly( bRead );
}

void OTableFieldDescWin::DisplayData( OFieldDescription* pFieldDescr )
{
    getGenPage()->DisplayData( pFieldDescr );
}

void OTableFieldDescWin::SaveData( OFieldDescription* pFieldDescr )
{
    getGenPage()->SaveData( pFieldDescr );
}

void OTableFieldDescWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    // 3D-line at the top window border
    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    rRenderContext.SetLineColor(rStyleSettings.GetLightColor());
    rRenderContext.DrawLine(Point(0,0), Point(GetSizePixel().Width(), 0));

    // 3D-line for the separation of the header
    rRenderContext.DrawLine(Point(3, DETAILS_HEADER_HEIGHT), Point(GetSizePixel().Width() - 6, DETAILS_HEADER_HEIGHT));
    rRenderContext.SetLineColor(rStyleSettings.GetShadowColor());
    rRenderContext.DrawLine(Point(3, DETAILS_HEADER_HEIGHT - 1), Point(GetSizePixel().Width() - 6, DETAILS_HEADER_HEIGHT - 1));
}

void OTableFieldDescWin::Resize()
{
    // dimensions of the parent window
    Size aOutputSize( GetOutputSizePixel() );
    long nOutputWidth = aOutputSize.Width();
    long nOutputHeight = aOutputSize.Height();

    // since the GenPage can scroll, but I can't, I position the HelpWindow, in case I become too slim,
    // _below_ the Genpage, not on the right side. But before that I try to make it a bit smaller

    long nHelpX, nHelpY;
    long nHelpWidth, nHelpHeight;
    long nPageWidth, nPageHeight;

    // do both fit next to each other (margin + page + margin + help)?
    if (STANDARD_MARGIN + DETAILS_OPT_PAGE_WIDTH + STANDARD_MARGIN + DETAILS_MIN_HELP_WIDTH <= nOutputWidth)
    {   // yes -> then we wonder if can give the help its optimum width
        nHelpWidth = DETAILS_OPT_HELP_WIDTH;
        nPageWidth = nOutputWidth - nHelpWidth - STANDARD_MARGIN - STANDARD_MARGIN;
        if (nPageWidth < DETAILS_OPT_PAGE_WIDTH)
        {   // rather resize the help from its optimal width to its minimum width
            long nTransfer = DETAILS_OPT_PAGE_WIDTH - nPageWidth;
            nPageWidth += nTransfer;
            nHelpWidth -= nTransfer;
        }
        nHelpX = nOutputWidth - nHelpWidth;
        // the heights are simple in that case...
        nHelpY = DETAILS_HEADER_HEIGHT + 1;
        nHelpHeight = nOutputHeight - nHelpY;
        nPageHeight = nOutputHeight - STANDARD_MARGIN - DETAILS_HEADER_HEIGHT - STANDARD_MARGIN;
    }
    else
    {   // doesn't work next to each other, thus below each other (margin + header + page + help)
        if (STANDARD_MARGIN + DETAILS_HEADER_HEIGHT + DETAILS_OPT_PAGE_HEIGHT + DETAILS_MIN_HELP_HEIGHT <= nOutputHeight)
        {   // it's at least enough, to fit both below each other (page optimal, help minimal)
            nHelpHeight = DETAILS_OPT_HELP_HEIGHT;
            nPageHeight = nOutputHeight - nHelpHeight - DETAILS_HEADER_HEIGHT - STANDARD_MARGIN;
            if (nPageHeight < DETAILS_OPT_PAGE_HEIGHT)
            {   // like above: page optimal, help gets whatever is left (which is bigger/equal to its minimum)
                long nTransfer = DETAILS_OPT_PAGE_HEIGHT - nPageHeight;
                nPageHeight += nTransfer;
                nHelpHeight -= nTransfer;
            }
            nHelpY = nOutputHeight - nHelpHeight;
            // and across the entire width
            nHelpX = 0;                 // without margin, since the HelpCtrl has its own one
            nHelpWidth = nOutputWidth;  // dito
            nPageWidth = nOutputWidth - STANDARD_MARGIN - STANDARD_MARGIN;
        }
        else
        {   // unfortunately that's not even enough, to show page at its optimum and help with minimum width
            nHelpX = nHelpY = nHelpWidth = nHelpHeight = 0; // thus no help window
            nPageWidth = nOutputWidth - STANDARD_MARGIN - STANDARD_MARGIN;
            nPageHeight = nOutputHeight - STANDARD_MARGIN - DETAILS_HEADER_HEIGHT - STANDARD_MARGIN;
        }
    }

    m_pHeader->SetPosSizePixel( Point(0, STANDARD_MARGIN), Size(nOutputWidth, 15) );

    getGenPage()->SetPosSizePixel(Point (   STANDARD_MARGIN,
                                        STANDARD_MARGIN + DETAILS_HEADER_HEIGHT
                                    ),
                              Size  (   nPageWidth,
                                        nPageHeight
                                    )
                             );
    if (nHelpHeight)
    {
        m_pHelpBar->Show();
        m_pHelpBar->SetPosSizePixel(Point   (   nHelpX,
                                            nHelpY
                                        ),
                                  Size  (   nHelpWidth,
                                            nHelpHeight
                                        )
                                 );
    }
    else
    {
        m_pHelpBar->Hide();
    }
    Invalidate();
}

IClipboardTest* OTableFieldDescWin::getActiveChild() const
{
    IClipboardTest* pTest = nullptr;
    switch(m_eChildFocus)
    {
        case DESCRIPTION:
            pTest = getGenPage();
            break;
        default:
            pTest = m_pHelpBar;
            break;
    }
    return pTest;
}

bool OTableFieldDescWin::isCopyAllowed()
{
    return getActiveChild() && getActiveChild()->isCopyAllowed();
}

bool OTableFieldDescWin::isCutAllowed()
{
    return (getGenPage() && getGenPage()->HasChildPathFocus() && getGenPage()->isCutAllowed());
}

bool OTableFieldDescWin::isPasteAllowed()
{
    return (getGenPage() && getGenPage()->HasChildPathFocus() && getGenPage()->isPasteAllowed());
}

void OTableFieldDescWin::cut()
{
    if ( getGenPage() && getGenPage()->HasChildPathFocus() )
        getGenPage()->cut();
}

void OTableFieldDescWin::copy()
{
    if ( getActiveChild() )
        getActiveChild()->copy();
}

void OTableFieldDescWin::paste()
{
    if ( getGenPage() && getGenPage()->HasChildPathFocus() )
        getGenPage()->paste();
}

void OTableFieldDescWin::GetFocus()
{
    if ( getGenPage() )
        getGenPage()->GetFocus();
}

void OTableFieldDescWin::LoseFocus()
{
    if ( getGenPage() )
        getGenPage()->LoseFocus();
}

bool OTableFieldDescWin::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS)
    {
        if( getGenPage() && getGenPage()->HasChildPathFocus() )
            m_eChildFocus = DESCRIPTION;
        else
            m_eChildFocus = HELP;
    }
    return TabPage::PreNotify(rNEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
