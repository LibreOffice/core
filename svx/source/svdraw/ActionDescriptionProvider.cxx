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
#include "svx/svdglob.hxx"
#include "svx/svdstr.hrc"

::rtl::OUString ActionDescriptionProvider::createDescription( ActionType eActionType
                        , const ::rtl::OUString& rObjectName )
{
    sal_uInt16 nResID=0;
    switch( eActionType )
    {
    case INSERT:
        nResID=STR_UndoInsertObj;
        break;
    case DELETE:
        nResID= STR_EditDelete;
        break;
    case CUT:
        nResID= STR_ExchangeClpCut;
        break;
    case MOVE:
        nResID= STR_EditMove;
        break;
    case RESIZE:
        nResID= STR_EditResize;
        break;
    case ROTATE:
        nResID= STR_EditRotate;
        break;
    case TRANSFORM:
        nResID= STR_EditTransform;
        break;
    case FORMAT:
        nResID= STR_EditSetAttributes;
        break;
    case MOVE_TOTOP:
        nResID= STR_EditMovToTop;
        break;
    case MOVE_TOBOTTOM:
        nResID= STR_EditMovToBtm;
        break;
    case POS_SIZE:
        nResID = STR_EditPosSize;
        break;
    }
    if(!nResID)
        return rtl::OUString();

    OUString aStr(ImpGetResStr(nResID));
    return aStr.replaceAll("%1", rObjectName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
