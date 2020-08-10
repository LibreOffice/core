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

#include "dbtreeview.hxx"
#include <dbtreelistbox.hxx>
#include <helpids.h>

namespace dbaui
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

DBTreeView::DBTreeView( vcl::Window* pParent, WinBits nBits)
    : Window(pParent, nBits)
    , m_pTreeListBox(VclPtr<InterimDBTreeListBox>::Create(this))
{
    m_pTreeListBox->SetHelpId(HID_TLB_TREELISTBOX);
    m_pTreeListBox->Show();
}

DBTreeView::~DBTreeView()
{
    disposeOnce();
}

void DBTreeView::dispose()
{
    m_pTreeListBox.disposeAndClear();
    vcl::Window::dispose();
}

void DBTreeView::SetPreExpandHandler(const Link<const weld::TreeIter&,bool>& rHdl)
{
    m_pTreeListBox->GetWidget().connect_expanding(rHdl);
}

void    DBTreeView::setCopyHandler(const Link<LinkParamNone*,void>& _rHdl)
{
    m_pTreeListBox->setCopyHandler(_rHdl);
}

void DBTreeView::Resize()
{
    Window::Resize();
    m_pTreeListBox->SetPosSizePixel(Point(0,0),GetOutputSizePixel());
}

void DBTreeView::setSelChangeHdl( const Link<LinkParamNone*,void>& _rHdl )
{
    m_pTreeListBox->SetSelChangeHdl( _rHdl );
}

void DBTreeView::GetFocus()
{
    Window::GetFocus();
    if ( m_pTreeListBox )//&& !m_pTreeListBox->HasChildPathFocus())
        m_pTreeListBox->GrabFocus();
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
