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
#include <svtools/treelistbox.hxx>
#include "dbtreelistbox.hxx"
#include "dbtreemodel.hxx"
#include "dbaccess_helpid.hrc"

namespace dbaui
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

DBG_NAME(DBTreeView)
// class DBTreeView
DBTreeView::DBTreeView( Window* pParent, WinBits nBits)
                    :   Window( pParent, nBits )
                    , m_pTreeListBox(NULL)
{
    DBG_CTOR(DBTreeView,NULL);

    m_pTreeListBox = new DBTreeListBox(this, WB_BORDER | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONS | WB_HSCROLL |WB_HASBUTTONSATROOT);
    m_pTreeListBox->EnableCheckButton(NULL);
    m_pTreeListBox->SetDragDropMode( 0 );
    m_pTreeListBox->EnableInplaceEditing( sal_True );
    m_pTreeListBox->SetHelpId(HID_TLB_TREELISTBOX);
    m_pTreeListBox->Show();
}

DBTreeView::~DBTreeView()
{
    DBG_DTOR(DBTreeView,NULL);
    if (m_pTreeListBox)
    {
        if (m_pTreeListBox->GetModel())
        {
            m_pTreeListBox->GetModel()->RemoveView(m_pTreeListBox);
            m_pTreeListBox->DisconnectFromModel();
        }
        ::std::auto_ptr<Window> aTemp(m_pTreeListBox);
        m_pTreeListBox = NULL;
    }
}

void DBTreeView::SetPreExpandHandler(const Link& _rHdl)
{
    m_pTreeListBox->SetPreExpandHandler(_rHdl);
}

void    DBTreeView::setCopyHandler(const Link& _rHdl)
{
    m_pTreeListBox->setCopyHandler(_rHdl);
}

void DBTreeView::Resize()
{
    Window::Resize();
    m_pTreeListBox->SetPosSizePixel(Point(0,0),GetOutputSizePixel());
}

void DBTreeView::setModel(SvTreeList* _pTreeModel)
{
    if (_pTreeModel)
        _pTreeModel->InsertView(m_pTreeListBox);
    m_pTreeListBox->SetModel(_pTreeModel);
}

void DBTreeView::setSelChangeHdl( const Link& _rHdl )
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
