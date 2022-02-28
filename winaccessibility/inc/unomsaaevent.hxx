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

#pragma once

enum class UnoMSAAEvent
{
    STATE_FOCUSED,
    STATE_CHECKED,
    STATE_ARMED,
    STATE_PRESSED,
    STATE_SELECTED,
    STATE_SHOWING,
    MENU_START,
    MENU_END,
    MENUPOPUPSTART,
    MENUPOPUPEND,
    SELECTION_CHANGED,
    INVALIDATE_ALL_CHILDREN,
    OBJECT_VALUECHANGE,
    OBJECT_NAMECHANGE,
    OBJECT_DESCRIPTIONCHANGE,
    OBJECT_DEFACTIONCHANGE,
    OBJECT_CARETCHANGE,
    OBJECT_TEXTCHANGE,
    ACTIVE_DESCENDANT_CHANGED,
    BOUNDRECT_CHANGED,
    VISIBLE_DATA_CHANGED,
    SHOW,
    STATE_BUSY,
    TABLE_CAPTION_CHANGED,
    TABLE_COLUMN_DESCRIPTION_CHANGED,
    TABLE_COLUMN_HEADER_CHANGED,
    TABLE_MODEL_CHANGED,
    TABLE_ROW_HEADER_CHANGED,
    TABLE_SUMMARY_CHANGED,
    OBJECT_REORDER,
    PAGE_CHANGED,
    CHILD_ADDED,
    CHILD_REMOVED,
    TABLE_ROW_DESCRIPTION_CHANGED,
    SELECTION_CHANGED_ADD,
    SELECTION_CHANGED_REMOVE,
    SELECTION_CHANGED_WITHIN,
    OBJECT_PAGECHANGED,
    TEXT_SELECTION_CHANGED,
    SECTION_CHANGED,
    COLUMN_CHANGED
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
