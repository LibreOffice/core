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

#include <sfx2/sfxsids.hrc>
#include <svx/svxids.hrc>
#include <QueryTextView.hxx>
#include <querycontainerwindow.hxx>
#include <helpids.h>
#include <sqledit.hxx>
#include <undosqledit.hxx>
#include <QueryDesignView.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

// end of temp classes
OQueryTextView::OQueryTextView(OQueryContainerWindow* pParent)
    : InterimItemWindow(pParent, "dbaccess/ui/queryview.ui", "QueryView")
    , m_xSQL(new SQLEditView)
    , m_xSQLEd(new weld::CustomWeld(*m_xBuilder, "sql", *m_xSQL))
    , m_bStopTimer(false)
{
    m_xSQL->SetHelpId(HID_CTL_QRYSQLEDIT);
    m_xSQL->SetModifyHdl(LINK(this, OQueryTextView, ModifyHdl));

    m_timerUndoActionCreation.SetTimeout(1000);
    m_timerUndoActionCreation.SetInvokeHandler(LINK(this, OQueryTextView, OnUndoActionTimer));

    m_timerInvalidate.SetTimeout(200);
    m_timerInvalidate.SetInvokeHandler(LINK(this, OQueryTextView, OnInvalidateTimer));
    m_timerInvalidate.Start();
}

IMPL_LINK_NOARG(OQueryTextView, ModifyHdl, LinkParamNone*, void)
{
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();
    m_timerUndoActionCreation.Start();

    OJoinController& rController = getContainerWindow()->getDesignView()->getController();
    if (!rController.isModified())
        rController.setModified( true );

    rController.InvalidateFeature(SID_SBA_QRY_EXECUTE);
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);
}

IMPL_LINK_NOARG(OQueryTextView, OnUndoActionTimer, Timer*, void)
{
    OUString aText = m_xSQL->GetText();
    if (aText == m_strOrigText)
        return;

    OJoinController& rController = getContainerWindow()->getDesignView()->getController();
    SfxUndoManager& rUndoMgr = rController.GetUndoManager();
    std::unique_ptr<OSqlEditUndoAct> pUndoAct(new OSqlEditUndoAct(*m_xSQL));

    pUndoAct->SetOriginalText( m_strOrigText );
    rUndoMgr.AddUndoAction( std::move(pUndoAct) );

    rController.InvalidateFeature(SID_UNDO);
    rController.InvalidateFeature(SID_REDO);

    m_strOrigText = aText;
}

IMPL_LINK_NOARG(OQueryTextView, OnInvalidateTimer, Timer*, void)
{
    OJoinController& rController = getContainerWindow()->getDesignView()->getController();
    rController.InvalidateFeature(SID_CUT);
    rController.InvalidateFeature(SID_COPY);
    if(!m_bStopTimer)
        m_timerInvalidate.Start();
}

void OQueryTextView::startTimer()
{
    m_bStopTimer = false;
    if (!m_timerInvalidate.IsActive())
        m_timerInvalidate.Start();
}

void OQueryTextView::stopTimer()
{
    m_bStopTimer = true;
    if (m_timerInvalidate.IsActive())
        m_timerInvalidate.Stop();
}

OQueryTextView::~OQueryTextView()
{
    disposeOnce();
}

void OQueryTextView::dispose()
{
    if (m_timerUndoActionCreation.IsActive())
        m_timerUndoActionCreation.Stop();

    m_xSQLEd.reset();
    m_xSQL.reset();
    InterimItemWindow::dispose();
}

void OQueryTextView::GetFocus()
{
    if (m_xSQL)
    {
        m_xSQL->GrabFocus();
        m_strOrigText = m_xSQL->GetText();
    }
    InterimItemWindow::GetFocus();
}

OUString OQueryTextView::getStatement() const
{
    return m_xSQL->GetText();
}

void OQueryTextView::clear()
{
    std::unique_ptr<OSqlEditUndoAct> xUndoAct(new OSqlEditUndoAct(*m_xSQL));

    xUndoAct->SetOriginalText(m_xSQL->GetText());
    getContainerWindow()->getDesignView()->getController().addUndoActionAndInvalidate( std::move(xUndoAct) );

    SetSQLText(OUString());
}

void OQueryTextView::setStatement(const OUString& rsStatement)
{
    SetSQLText(rsStatement);
}

void OQueryTextView::SetSQLText(const OUString& rNewText)
{
    if (m_timerUndoActionCreation.IsActive())
    {
        // create the trailing undo-actions
        m_timerUndoActionCreation.Stop();
        OnUndoActionTimer(nullptr);
    }

    m_xSQL->SetTextAndUpdate(rNewText);

    m_strOrigText = rNewText;
}

void OQueryTextView::copy()
{
    m_xSQL->Copy();
}

bool OQueryTextView::isCutAllowed() const
{
    return m_xSQL->HasSelection();
}

void OQueryTextView::cut()
{
    m_xSQL->Cut();
    getContainerWindow()->getDesignView()->getController().setModified(true);
}

void OQueryTextView::paste()
{
    m_xSQL->Paste();
    getContainerWindow()->getDesignView()->getController().setModified(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
