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
#include "lokundomanagerlistener.hxx"
#include "lokinteractionhandler.hxx"

#include <rtl/ref.hxx>
#include <vector>

#include <../../inc/lib/init.hxx>
#include <sfx2/lokhelper.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>


LOKUndoManagerListener::LOKUndoManagerListener(
    const ::css::uno::Reference<::css::document::XUndoManager>& xUndoManager,
    desktop::LibLibreOffice_Impl *const pLOKit)
    : m_xUndoManager(xUndoManager),
      m_pLOKit(pLOKit),
      m_bDocRepairState(false)
{
}

LOKUndoManagerListener::~LOKUndoManagerListener()
{

}

void LOKUndoManagerListener::broadcastViews()
{
    bool bDocRepairState = m_xUndoManager->isUndoPossible() || m_xUndoManager->isRedoPossible();
    if (m_bDocRepairState != bDocRepairState)
    {
        m_bDocRepairState = bDocRepairState;
        OString sPayload = m_bDocRepairState ? OString(".uno:DocumentRepair=enabled") : OString(".uno:DocumentRepair=disabled");
        if (m_pLOKit->mpCallback)
            m_pLOKit->mpCallback(LOK_CALLBACK_STATE_CHANGED, sPayload.getStr(), m_pLOKit->mpCallbackData);
    }
}

void SAL_CALL LOKUndoManagerListener::undoActionAdded(const ::com::sun::star::document::UndoManagerEvent&)
{
    broadcastViews();
}

void SAL_CALL LOKUndoManagerListener::actionUndone(const ::com::sun::star::document::UndoManagerEvent&)
{
    broadcastViews();
}

void SAL_CALL LOKUndoManagerListener::actionRedone(const ::com::sun::star::document::UndoManagerEvent&)
{
    broadcastViews();
}

void SAL_CALL LOKUndoManagerListener::allActionsCleared(const ::com::sun::star::lang::EventObject&)
{
    broadcastViews();
}

void SAL_CALL LOKUndoManagerListener::redoActionsCleared(const ::com::sun::star::lang::EventObject&)
{
    broadcastViews();
}

void SAL_CALL LOKUndoManagerListener::resetAll(const ::com::sun::star::lang::EventObject&)
{
    broadcastViews();
}

void SAL_CALL LOKUndoManagerListener::enteredContext(const ::com::sun::star::document::UndoManagerEvent&)
{
}

void SAL_CALL LOKUndoManagerListener::enteredHiddenContext(const ::com::sun::star::document::UndoManagerEvent&)
{
}

void SAL_CALL LOKUndoManagerListener::leftContext(const ::com::sun::star::document::UndoManagerEvent&)
{
    broadcastViews();
}

void SAL_CALL LOKUndoManagerListener::leftHiddenContext(const ::com::sun::star::document::UndoManagerEvent&)
{
    broadcastViews();
}

void SAL_CALL LOKUndoManagerListener::cancelledContext(const ::com::sun::star::document::UndoManagerEvent&)
{
}

void SAL_CALL LOKUndoManagerListener::disposing(const ::com::sun::star::lang::EventObject&)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
