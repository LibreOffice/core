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
#include "FieldDescriptions.hxx"
#include "dbu_tbl.hrc"
#include "TableDesignHelpBar.hxx"
#include <vcl/fixed.hxx>
#include "dbaccess_helpid.hrc"
#include "moduledbu.hxx"
#include <memory>

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
DBG_NAME(OTableFieldDescWin)
OTableFieldDescWin::OTableFieldDescWin( Window* pParent)
    :TabPage(pParent, WB_3DLOOK)
{
    DBG_CTOR(OTableFieldDescWin,NULL);
    // Header
    m_pHeader = new FixedText( this, WB_CENTER | WB_INFO ); //  | WB_3DLOOK
    m_pHeader->SetText( String(ModuleRes(STR_TAB_PROPERTIES)) );
    m_pHeader->Show();

    // HelpBar
    m_pHelpBar = new OTableDesignHelpBar( this );
    m_pHelpBar->SetHelpId(HID_TAB_DESIGN_HELP_TEXT_FRAME);
    m_pHelpBar->Show();

    m_pGenPage = new OFieldDescGenWin( this, m_pHelpBar );
    getGenPage()->SetHelpId( HID_TABLE_DESIGN_TABPAGE_GENERAL );
    getGenPage()->Show();
}

OTableFieldDescWin::~OTableFieldDescWin()
{
    DBG_DTOR(OTableFieldDescWin,NULL);
    // Children zerstoeren
    m_pHelpBar->Hide();
    getGenPage()->Hide();
    m_pHeader->Hide();

    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pGenPage);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pGenPage = NULL;
    }
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pHeader);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pHeader = NULL;
    }
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pHelpBar);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pHelpBar = NULL;
    }
}

void OTableFieldDescWin::Init()
{
    OSL_ENSURE(getGenPage() != NULL, "OTableFieldDescWin::Init : ups ... no GenericPage ... this will crash ...");
    getGenPage()->Init();
}

void OTableFieldDescWin::SetReadOnly( sal_Bool bRead )
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    getGenPage()->SetReadOnly( bRead );
}

void OTableFieldDescWin::DisplayData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    getGenPage()->DisplayData( pFieldDescr );
}

void OTableFieldDescWin::SaveData( OFieldDescription* pFieldDescr )
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    getGenPage()->SaveData( pFieldDescr );
}

void OTableFieldDescWin::Paint( const Rectangle& /*rRect*/ )
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    // 3D-Linie am oberen Fensterrand
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

    SetLineColor( rStyleSettings.GetLightColor() );
    DrawLine( Point(0,0), Point(GetSizePixel().Width(),0) );

    // 3D-Linie zum Abtrennen des Headers
    DrawLine( Point(3, DETAILS_HEADER_HEIGHT), Point(GetSizePixel().Width()-6, DETAILS_HEADER_HEIGHT) );
    SetLineColor( rStyleSettings.GetShadowColor() );
    DrawLine( Point(3, DETAILS_HEADER_HEIGHT-1), Point(GetSizePixel().Width()-6, DETAILS_HEADER_HEIGHT-1) );
}

void OTableFieldDescWin::Resize()
{
    DBG_CHKTHIS(OTableFieldDescWin,NULL);
    // Abmessungen parent window
    Size aOutputSize( GetOutputSizePixel() );
    long nOutputWidth = aOutputSize.Width();
    long nOutputHeight = aOutputSize.Height();

    // da die GenPage scrollen kann, ich selber aber nicht, positioniere ich das HelpFenster, wenn ich zu schmal werde,
    // _unter_ der Genpage, nicht rechts daneben. Zuvor versuche ich aber noch, es etwas schmaler zu machen

    long nHelpX, nHelpY;
    long nHelpWidth, nHelpHeight;
    long nPageWidth, nPageHeight;

    // passen beide nebeneinander (Rand + Page + Rand + Help) ?
    if (STANDARD_MARGIN + DETAILS_OPT_PAGE_WIDTH + STANDARD_MARGIN + DETAILS_MIN_HELP_WIDTH <= nOutputWidth)
    {   // ja -> dann ist die Frage, ob man der Hilfe ihre Optimal-Breite geben kann
        nHelpWidth = DETAILS_OPT_HELP_WIDTH;
        nPageWidth = nOutputWidth - nHelpWidth - STANDARD_MARGIN - STANDARD_MARGIN;
        if (nPageWidth < DETAILS_OPT_PAGE_WIDTH)
        {   // dann doch lieber die Hilfe von ihrer optimalen in Richtung auf die minimale Groesse
            long nTransfer = DETAILS_OPT_PAGE_WIDTH - nPageWidth;
            nPageWidth += nTransfer;
            nHelpWidth -= nTransfer;
        }
        nHelpX = nOutputWidth - nHelpWidth;
        // die Hoehen sind dann einfach ...
        nHelpY = DETAILS_HEADER_HEIGHT + 1;
        nHelpHeight = nOutputHeight - nHelpY;
        nPageHeight = nOutputHeight - STANDARD_MARGIN - DETAILS_HEADER_HEIGHT - STANDARD_MARGIN;
    }
    else
    {   // nebeneinander geht nicht, also untereinander (Rand + Header + Page + Help)
        if (STANDARD_MARGIN + DETAILS_HEADER_HEIGHT + DETAILS_OPT_PAGE_HEIGHT + DETAILS_MIN_HELP_HEIGHT <= nOutputHeight)
        {   // es reicht zumindest, um beide untereinander (Page optimal, Help minimal) unterzubringen
            nHelpHeight = DETAILS_OPT_HELP_HEIGHT;
            nPageHeight = nOutputHeight - nHelpHeight - DETAILS_HEADER_HEIGHT - STANDARD_MARGIN;
            if (nPageHeight < DETAILS_OPT_PAGE_HEIGHT)
            {   // wie oben : Page optimal, Hilfe soviel wie eben bleibt (das ist groesser/gleich ihrem Minimum)
                long nTransfer = DETAILS_OPT_PAGE_HEIGHT - nPageHeight;
                nPageHeight += nTransfer;
                nHelpHeight -= nTransfer;
            }
            nHelpY = nOutputHeight - nHelpHeight;
            // und ueber die ganze Breite
            nHelpX = 0;                 // ohne Margin, da das HelpCtrl einen eigenen hat
            nHelpWidth = nOutputWidth;  // dito
            nPageWidth = nOutputWidth - STANDARD_MARGIN - STANDARD_MARGIN;
        }
        else
        {   // dummerweise reicht es nicht mal, um Page optimal und Help minimal zu zeigen
            nHelpX = nHelpY = nHelpWidth = nHelpHeight = 0; // -> kein Help-Fenster
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
    IClipboardTest* pTest = NULL;
    switch(m_eChildFocus)
    {
        case DESCRIPTION:
            pTest = getGenPage();
            break;
        default:
            pTest = getHelpBar();
            break;
    }
    return pTest;
}

sal_Bool OTableFieldDescWin::isCopyAllowed()
{
    return getActiveChild() && getActiveChild()->isCopyAllowed();
}

sal_Bool OTableFieldDescWin::isCutAllowed()
{
    return (getGenPage() && getGenPage()->HasChildPathFocus() && getGenPage()->isCutAllowed());
}

sal_Bool OTableFieldDescWin::isPasteAllowed()
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

long OTableFieldDescWin::PreNotify( NotifyEvent& rNEvt )
{
    sal_Bool bHandled = sal_False;
    switch(rNEvt.GetType())
    {
        case EVENT_GETFOCUS:
            if( getGenPage() && getGenPage()->HasChildPathFocus() )
                m_eChildFocus = DESCRIPTION;
            else
                m_eChildFocus = HELP;
            break;
    }

    return bHandled ? 1L : TabPage::PreNotify(rNEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
