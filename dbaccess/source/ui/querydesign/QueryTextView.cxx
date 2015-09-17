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

#include "QueryTextView.hxx"
#include "querycontainerwindow.hxx"
#include "QueryViewSwitch.hxx"
#include "sqledit.hxx"
#include "undosqledit.hxx"
#include "browserids.hxx"
#include "querycontroller.hxx"
#include "dbu_qry.hrc"
#include "dbustrings.hrc"
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/split.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/types.hxx>
#include "QueryDesignView.hxx"

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;

// end of temp classes
OQueryTextView::OQueryTextView(OQueryContainerWindow* _pParent)
    :Window(_pParent)
{
    m_pEdit = VclPtr<OSqlEdit>::Create(this);
    m_pEdit->SetRightToLeft(false);
    m_pEdit->ClearModifyFlag();
    m_pEdit->SaveValue();
    m_pEdit->SetPosPixel( Point( 0, 0 ) );
    m_pEdit->Show();
}

OQueryTextView::~OQueryTextView()
{
    disposeOnce();
}

void OQueryTextView::dispose()
{
    m_pEdit.disposeAndClear();
    vcl::Window::dispose();
}

void OQueryTextView::GetFocus()
{
    if ( m_pEdit )
        m_pEdit->GrabFocus();
}

void OQueryTextView::Resize()
{
    Window::Resize();
    m_pEdit->SetSizePixel( GetOutputSizePixel() );
}

OUString OQueryTextView::getStatement()
{
    return m_pEdit->GetText();
}

void OQueryTextView::clear()
{
    OSqlEditUndoAct* pUndoAct = new OSqlEditUndoAct( m_pEdit );

    pUndoAct->SetOriginalText( m_pEdit->GetText() );
    getContainerWindow()->getDesignView()->getController().addUndoActionAndInvalidate( pUndoAct );

    m_pEdit->SetText(OUString());
}

void OQueryTextView::setStatement(const OUString& _rsStatement)
{
    m_pEdit->SetText(_rsStatement);
}

void OQueryTextView::copy()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Copy();
}

bool OQueryTextView::isCutAllowed()
{
    return !m_pEdit->GetSelected().isEmpty();
}

void OQueryTextView::cut()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Cut();
    getContainerWindow()->getDesignView()->getController().setModified(sal_True);
}

void OQueryTextView::paste()
{
    if(!m_pEdit->IsInAccelAct() )
        m_pEdit->Paste();
    getContainerWindow()->getDesignView()->getController().setModified(sal_True);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
