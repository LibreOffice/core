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


#include "svx/ActionDescriptionProvider.hxx"
#include "svdglob.hxx"
#include "svx/strings.hrc"

OUString ActionDescriptionProvider::createDescription( ActionType eActionType
                        , const OUString& rObjectName )
{
    const char* pResID = nullptr;
    switch( eActionType )
    {
    case ActionType::Insert:
        pResID=STR_UndoInsertObj;
        break;
    case ActionType::Delete:
        pResID= STR_EditDelete;
        break;
    case ActionType::Move:
        pResID= STR_EditMove;
        break;
    case ActionType::Resize:
        pResID= STR_EditResize;
        break;
    case ActionType::Rotate:
        pResID= STR_EditRotate;
        break;
    case ActionType::Format:
        pResID= STR_EditSetAttributes;
        break;
    case ActionType::MoveToTop:
        pResID= STR_EditMovToTop;
        break;
    case ActionType::MoveToBottom:
        pResID= STR_EditMovToBtm;
        break;
    case ActionType::PosSize:
        pResID = STR_EditPosSize;
        break;
    }
    if (!pResID)
        return OUString();

    OUString aStr(ImpGetResStr(pResID));
    return aStr.replaceAll("%1", rObjectName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
