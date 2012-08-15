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

#include "sdundogr.hxx"


TYPEINIT1(SdUndoGroup, SdUndoAction);

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdUndoGroup::~SdUndoGroup()
{
    size_t nLast = aCtn.size();
    for (size_t nAction = 0; nAction < nLast; nAction++)
    {
        delete aCtn[nAction];
    }
    aCtn.clear();
}

/*************************************************************************
|*
|* Merge
|*
\************************************************************************/

sal_Bool SdUndoGroup::Merge( SfxUndoAction* pNextAction )
{
    sal_Bool bRet = sal_False;

    if( pNextAction && pNextAction->ISA( SdUndoAction ) )
    {
        SdUndoAction* pClone = static_cast< SdUndoAction* >( pNextAction )->Clone();

        if( pClone )
        {
            AddAction( pClone );
            bRet = sal_True;
        }
    }

    return bRet;
}

/*************************************************************************
|*
|* Undo, umgekehrte Reihenfolge der Ausfuehrung
|*
\************************************************************************/

void SdUndoGroup::Undo()
{
    long nLast = aCtn.size();
    for (long nAction = nLast - 1; nAction >= 0; nAction--)
    {
        aCtn[nAction]->Undo();
    }

}

/*************************************************************************
|*
|* Redo
|*
\************************************************************************/

void SdUndoGroup::Redo()
{
    size_t nLast = aCtn.size();
    for (size_t nAction = 0; nAction < nLast; nAction++)
    {
        aCtn[nAction]->Redo();
    }

}

/*************************************************************************
|*
|* eine Aktion hinzufuegen
|*
\************************************************************************/

void SdUndoGroup::AddAction(SdUndoAction* pAction)
{
    aCtn.push_back(pAction);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
