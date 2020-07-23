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

using namespace dbaui;

OTableFieldDescWin::OTableFieldDescWin(vcl::Window* pParent, OTableDesignView* pView)
    : InterimItemWindow(pParent, "dbaccess/ui/fielddescpanel.ui", "FieldDescPanel")
    , m_xHelpBar(new OTableDesignHelpBar(m_xBuilder->weld_text_view("textview")))
    , m_xBox(m_xBuilder->weld_container("box"))
    , m_xFieldControl(VclPtr<OTableFieldControl>::Create(m_xBox.get(), m_xHelpBar.get(), pView))
    , m_xHeader(m_xBuilder->weld_label("header"))
    , m_eChildFocus(NONE)
{
    // Header
    m_xHeader->set_label(DBA_RES(STR_TAB_PROPERTIES));

    m_xFieldControl->SetHelpId(HID_TAB_DESIGN_FIELDCONTROL);
}

OTableFieldDescWin::~OTableFieldDescWin()
{
    disposeOnce();
}

void OTableFieldDescWin::dispose()
{
    // destroy children
    m_xFieldControl.disposeAndClear();
    m_xBox.reset();
    m_xHeader.reset();
    m_xHelpBar.reset();
    InterimItemWindow::dispose();
}

void OTableFieldDescWin::Init()
{
    m_xFieldControl->Init();
}

void OTableFieldDescWin::SetReadOnly( bool bRead )
{
    m_xFieldControl->SetReadOnly( bRead );
}

void OTableFieldDescWin::DisplayData( OFieldDescription* pFieldDescr )
{
    m_xFieldControl->DisplayData( pFieldDescr );
}

void OTableFieldDescWin::SaveData( OFieldDescription* pFieldDescr )
{
    m_xFieldControl->SaveData( pFieldDescr );
}

IClipboardTest* OTableFieldDescWin::getActiveChild() const
{
    IClipboardTest* pTest = nullptr;
    switch(m_eChildFocus)
    {
        case DESCRIPTION:
            pTest = m_xFieldControl.get();
            break;
        default:
            pTest = m_xHelpBar.get();
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
    return m_xFieldControl->HasChildPathFocus() && m_xFieldControl->isCutAllowed();
}

bool OTableFieldDescWin::isPasteAllowed()
{
    return m_xFieldControl->HasChildPathFocus() && m_xFieldControl->isPasteAllowed();
}

void OTableFieldDescWin::cut()
{
    if (m_xFieldControl->HasChildPathFocus())
        m_xFieldControl->cut();
}

void OTableFieldDescWin::copy()
{
    if ( getActiveChild() )
        getActiveChild()->copy();
}

void OTableFieldDescWin::paste()
{
    if (m_xFieldControl->HasChildPathFocus())
        m_xFieldControl->paste();
}

#if 0
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
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
