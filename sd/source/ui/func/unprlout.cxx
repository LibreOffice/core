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

#include "unprlout.hxx"

#include "strings.hrc"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include "sdresid.hxx"


TYPEINIT1(SdPresentationLayoutUndoAction, SdUndoAction);


SdPresentationLayoutUndoAction::SdPresentationLayoutUndoAction(
                            SdDrawDocument* pTheDoc,
                            OUString        aTheOldLayoutName,
                            OUString        aTheNewLayoutName,
                            AutoLayout      eTheOldAutoLayout,
                            AutoLayout      eTheNewAutoLayout,
                            sal_Bool            bSet,
                            SdPage*         pThePage):
                      SdUndoAction(pTheDoc)
{
    aOldLayoutName = aTheOldLayoutName;
    aNewLayoutName = aTheNewLayoutName;
    eOldAutoLayout = eTheOldAutoLayout;
    eNewAutoLayout = eTheNewAutoLayout;
    bSetAutoLayout = bSet;

    DBG_ASSERT(pThePage, "No Page set!");
    pPage = pThePage;
    aComment = SD_RESSTR(STR_UNDO_SET_PRESLAYOUT);
}

void SdPresentationLayoutUndoAction::Undo()
{
    pPage->SetPresentationLayout(aOldLayoutName, sal_True, sal_True, sal_True);
    if (bSetAutoLayout)
        pPage->SetAutoLayout(eOldAutoLayout, sal_True);
}

void SdPresentationLayoutUndoAction::Redo()
{
    pPage->SetPresentationLayout(aNewLayoutName);
    if (bSetAutoLayout)
        pPage->SetAutoLayout(eNewAutoLayout, sal_True);
}

SdPresentationLayoutUndoAction::~SdPresentationLayoutUndoAction()
{
}

OUString SdPresentationLayoutUndoAction::GetComment() const
{
    return aComment;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
