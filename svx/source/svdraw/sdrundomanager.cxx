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

#include <svx/sdrundomanager.hxx>



SdrUndoManager::SdrUndoManager(sal_uInt16 nMaxUndoActionCount)
    : EditUndoManager(nMaxUndoActionCount)
    , maEndTextEditHdl()
    , mpLastUndoActionBeforeTextEdit(nullptr)
    , mbEndTextEditTriggeredFromUndo(false)
{
}

SdrUndoManager::~SdrUndoManager()
{
}

bool SdrUndoManager::Undo()
{
    if(isTextEditActive())
    {
        bool bRetval(false);

        // we are in text edit mode
        if(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction())
        {
            // there is an undo action for text edit, trigger it
            bRetval = EditUndoManager::Undo();
        }
        else
        {
            // no more text edit undo, end text edit
            mbEndTextEditTriggeredFromUndo = true;
            maEndTextEditHdl.Call(this);
            mbEndTextEditTriggeredFromUndo = false;
        }

        return bRetval;
    }
    else
    {
        // no undo triggered up to now, trigger local one
        return SfxUndoManager::Undo();
    }
}

bool SdrUndoManager::Redo()
{
    bool bRetval(false);

    if(isTextEditActive())
    {
        // we are in text edit mode
        bRetval = EditUndoManager::Redo();
    }

    if(!bRetval)
    {
        // no redo triggered up to now, trigger local one
        bRetval = SfxUndoManager::Redo();
    }

    return bRetval;
}

void SdrUndoManager::Clear()
{
    if(isTextEditActive())
    {
        while(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction())
        {
            RemoveLastUndoAction();
        }

        // urgently needed: RemoveLastUndoAction does NOT correct the Redo stack by itself (!)
        ClearRedo();
    }
    else
    {
        // call parent
        EditUndoManager::Clear();
    }
}

void SdrUndoManager::SetEndTextEditHdl(const Link<SdrUndoManager*,void>& rLink)
{
    maEndTextEditHdl = rLink;

    if(isTextEditActive())
    {
        // text edit start, remember last non-textedit action for later cleanup
        mpLastUndoActionBeforeTextEdit = GetUndoActionCount() ? GetUndoAction() : nullptr;
    }
    else
    {
        // text edit ends, pop all textedit actions up to the remembered non-textedit action from the start
        // to set back the UndoManager to the state before text edit started. If that action is already gone
        // (due to being removed from the undo stack in the meantime), all need to be removed anyways
        while(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction())
        {
            RemoveLastUndoAction();
        }

        // urgently needed: RemoveLastUndoAction does NOT correct the Redo stack by itself (!)
        ClearRedo();

        // forget marker again
        mpLastUndoActionBeforeTextEdit = nullptr;
    }
}

bool SdrUndoManager::isTextEditActive() const
{
    return maEndTextEditHdl.IsSet();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
