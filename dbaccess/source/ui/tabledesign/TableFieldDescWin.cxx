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
#include <FieldDescriptions.hxx>
#include <strings.hrc>
#include <TableDesignHelpBar.hxx>
#include <helpids.h>
#include <core_resource.hxx>

using namespace dbaui;

OTableFieldDescWin::OTableFieldDescWin(weld::Container* pParent, OTableDesignView* pView)
    : OChildWindow(pParent, u"dbaccess/ui/fielddescpanel.ui"_ustr, u"FieldDescPanel"_ustr)
    , m_aHelpBar(m_xBuilder->weld_text_view(u"textview"_ustr))
    , m_xBox(m_xBuilder->weld_container(u"box"_ustr))
    , m_xFieldControl(new OTableFieldControl(m_xBox.get(), &m_aHelpBar, pView))
    , m_xHeader(m_xBuilder->weld_label(u"header"_ustr))
    , m_eChildFocus(NONE)
{
    // Header
    m_xHeader->set_label(DBA_RES(STR_TAB_PROPERTIES));

    m_xFieldControl->SetHelpId(HID_TAB_DESIGN_FIELDCONTROL);

    m_aHelpBar.connect_focus_in(LINK(this, OTableFieldDescWin, HelpFocusIn));
    m_xFieldControl->connect_focus_in(LINK(this, OTableFieldDescWin, FieldFocusIn));
}

bool OTableFieldDescWin::HasChildPathFocus() const
{
    return m_xFieldControl->HasChildPathFocus() || m_aHelpBar.HasFocus();
}

OTableFieldDescWin::~OTableFieldDescWin()
{
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
            pTest = const_cast<OTableDesignHelpBar*>(&m_aHelpBar);
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
    return getActiveChild() && getActiveChild()->isCutAllowed();
}

bool OTableFieldDescWin::isPasteAllowed()
{
    return getActiveChild() && getActiveChild()->isPasteAllowed();
}

void OTableFieldDescWin::cut()
{
    if (getActiveChild())
        getActiveChild()->cut();
}

void OTableFieldDescWin::copy()
{
    if ( getActiveChild() )
        getActiveChild()->copy();
}

void OTableFieldDescWin::paste()
{
    if (getActiveChild())
        getActiveChild()->paste();
}

void OTableFieldDescWin::GrabFocus()
{
    m_xFieldControl->GrabFocus();
}

IMPL_LINK(OTableFieldDescWin, HelpFocusIn, weld::Widget&, rWidget, void)
{
    m_eChildFocus = HELP;
    m_aFocusInHdl.Call(rWidget);
}

IMPL_LINK(OTableFieldDescWin, FieldFocusIn, weld::Widget&, rWidget, void)
{
    m_eChildFocus = DESCRIPTION;
    m_aFocusInHdl.Call(rWidget);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
