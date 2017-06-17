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

#include "lokgloballistener.hxx"
#include "lokinteractionhandler.hxx"

#include <sfx2/app.hxx>
#include <sfx2/event.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <rtl/ref.hxx>
#include <../../inc/lib/init.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <com/sun/star/document/XUndoManagerSupplier.hpp>
#include <com/sun/star/document/XUndoManager.hpp>

LOKGlobalListener& LOKGlobalListener::Get()
{
    static LOKGlobalListener aUniqueLOKListener;
    return aUniqueLOKListener;
}

LOKGlobalListener::LOKGlobalListener()
{
    StartListening( *SfxGetpApp() );
}

LOKGlobalListener::~LOKGlobalListener()
{
}

void LOKGlobalListener::Notify(SfxBroadcaster&, const SfxHint& rHint )
{
    const SfxViewEventHint* pViewHint = dynamic_cast<const SfxViewEventHint*>(&rHint);
    if (!pViewHint)
        return;

    switch (pViewHint->GetEventId())
    {
        case SfxEventHintId::ViewCreated:
        case SfxEventHintId::LOKViewCallbackRegistered:
        {
            SfxObjectShell* pDocShell = pViewHint->GetObjShell();
            ::css::uno::Reference<css::frame::XController> xController(pViewHint->GetController(), ::css::uno::UNO_QUERY);
            ::css::uno::Reference<::css::document::XUndoManagerSupplier> xSuppUndo(pDocShell ? pDocShell->GetModel() : nullptr, ::css::uno::UNO_QUERY);
            ::css::uno::Reference<::css::document::XUndoManager> xUndoManager(xSuppUndo.is() ? xSuppUndo->getUndoManager() : nullptr, ::css::uno::UNO_QUERY);

            SfxViewShell* pViewShell = SfxViewShell::Get(xController);
            if (pViewShell && xController.is() && xUndoManager.is())
            {
                OString sPayload(xUndoManager->isUndoPossible() || xUndoManager->isRedoPossible() ? ".uno:DocumentRepair=enabled" : ".uno:DocumentRepair=disabled");
                pViewShell->libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED, sPayload.getStr());
            }
        }
        break;
        default: break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
