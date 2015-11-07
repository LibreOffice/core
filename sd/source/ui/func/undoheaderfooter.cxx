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

#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include "app.hrc"
#include "DrawViewShell.hxx"
#include "drawview.hxx"
#include "undoheaderfooter.hxx"


SdHeaderFooterUndoAction::SdHeaderFooterUndoAction( SdDrawDocument* pDoc, SdPage* pPage, const sd::HeaderFooterSettings& rNewSettings )
:   SdUndoAction(pDoc),
    mpPage(pPage),
    maOldSettings(pPage->getHeaderFooterSettings()),
    maNewSettings(rNewSettings)
{
}

SdHeaderFooterUndoAction::~SdHeaderFooterUndoAction()
{
}

void SdHeaderFooterUndoAction::Undo()
{
    mpPage->setHeaderFooterSettings( maOldSettings );
    SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SWITCHPAGE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
}

void SdHeaderFooterUndoAction::Redo()
{
    mpPage->setHeaderFooterSettings( maNewSettings );
    SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SWITCHPAGE, SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
