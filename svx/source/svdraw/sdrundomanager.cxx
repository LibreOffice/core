/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdrundomanager.hxx>



SdrUndoManager::SdrUndoManager(sal_uInt16 nMaxUndoActionCount)
    : EditUndoManager(nMaxUndoActionCount)
    , maEndTextEditHdl()
    , mpLastUndoActionBeforeTextEdit(0)
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

        
        if(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction(0))
        {
            
            bRetval = EditUndoManager::Undo();
        }
        else
        {
            
            mbEndTextEditTriggeredFromUndo = true;
            maEndTextEditHdl.Call(this);
            mbEndTextEditTriggeredFromUndo = false;
        }

        return bRetval;
    }
    else
    {
        
        return SfxUndoManager::Undo();
    }
}

bool SdrUndoManager::Redo()
{
    bool bRetval(false);

    if(isTextEditActive())
    {
        
        bRetval = EditUndoManager::Redo();
    }

    if(!bRetval)
    {
        
        bRetval = SfxUndoManager::Redo();
    }

    return bRetval;
}

void SdrUndoManager::Clear()
{
    if(isTextEditActive())
    {
        while(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction(0))
        {
            RemoveLastUndoAction();
        }

        
        ClearRedo();
    }
    else
    {
        
        EditUndoManager::Clear();
    }
}

void SdrUndoManager::SetEndTextEditHdl(const Link& rLink)
{
    maEndTextEditHdl = rLink;

    if(isTextEditActive())
    {
        
        mpLastUndoActionBeforeTextEdit = GetUndoActionCount() ? GetUndoAction(0) : 0;
    }
    else
    {
        
        
        
        while(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction(0))
        {
            RemoveLastUndoAction();
        }

        
        ClearRedo();

        
        mpLastUndoActionBeforeTextEdit = 0;
    }
}

bool SdrUndoManager::isTextEditActive() const
{
    return maEndTextEditHdl.IsSet();
}



