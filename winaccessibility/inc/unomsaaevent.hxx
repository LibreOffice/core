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

//STATE_CHANGED with get FOCUSED -> EVENT_OBJECT_FOCUS
const short UM_EVENT_STATE_FOCUSED = 0;

//STATE_CHANGED --> EVENT_OBJECT_STATECHANGE
const short UM_EVENT_STATE_CHECKED = 1;
const short UM_EVENT_STATE_ARMED = 2;
const short UM_EVENT_STATE_PRESSED = 3;
const short UM_EVENT_STATE_SELECTED = 4;
const short UM_EVENT_STATE_SHOWING = 5;

//if acc role is MENU_BAR, STATE_CHANGED with get FOCUSED -> EVENT_SYSTEM_MENUSTART
const short UM_EVENT_MENU_START = 6;

//if acc role is MENU_BAR, STATE_CHANGED with lose FOCUSED -> EVENT_SYSTEM_MENUEND
const short UM_EVENT_MENU_END = 7;

//if acc role is POPUP_MENU, STATE_CHANGED with get FOCUSED -> EVENT_SYSTEM_MENUPOPUPSTART
const short UM_EVENT_MENUPOPUPSTART = 8;

//if acc role is POPUP_MENU, STATE_CHANGED with lose FOCUSED -> EVENT_SYSTEM_MENUPOPUPEND
const short UM_EVENT_MENUPOPUPEND = 9;

//SELECTION_CHANGED -> EVENT_OBJECT_SELECTION
const short UM_EVENT_SELECTION_CHANGED = 10;

//INVALIDATE_ALL_CHILDREN --> EVENT_OBJECT_SHOW
const short UM_EVENT_INVALIDATE_ALL_CHILDREN = 11;

//VALUE_CHANGED --> EVENT_OBJECT_VALUECHANGE
const short UM_EVENT_OBJECT_VALUECHANGE = 12;

//NAME_CHANGED --> EVENT_OBJECT_NAMECHANGE
const short UM_EVENT_OBJECT_NAMECHANGE = 13;

//DESCRIPTION_CHANGED --> EVENT_OBJECT_DESCRIPTIONCHANGE
const short UM_EVENT_OBJECT_DESCRIPTIONCHANGE = 14;

//ACTION_CHANGED --> EVENT_OBJECT_DEFACTIONCHANGE
const short UM_EVENT_OBJECT_DEFACTIONCHANGE = 15;

//CARET_CHANGED --> EVENT_OBJECT_LOCATIONCHANGED
const short UM_EVENT_OBJECT_CARETCHANGE = 16;

//TEXT_CHANGED --> EVENT_OBJECT_VALUECHANGE
const short UM_EVENT_OBJECT_TEXTCHANGE = 17;

//ACTIVE_DESCENDANT_CHANGED --> EVENT_OBJECT_FOCUS
const short UM_EVENT_ACTIVE_DESCENDANT_CHANGED = 18;

//BOUNDRECT_CHANGED --> EVENT_OBJECT_LOCATIONCHANGE
const short UM_EVENT_BOUNDRECT_CHANGED = 19;

//VISIBLE_DATA_CHANGED --> EVENT_OBJECT_VALUECHANGE
const short UM_EVENT_VISIBLE_DATA_CHANGED = 20;

//to enable SHOW on dialogs, windows, frames
const short UM_EVENT_SHOW = 21;

const short UM_EVENT_STATE_BUSY = 22;

const short UM_EVENT_TABLE_CAPTION_CHANGED = 24;
const short UM_EVENT_TABLE_COLUMN_DESCRIPTION_CHANGED = 25;
const short UM_EVENT_TABLE_COLUMN_HEADER_CHANGED = 26;
const short UM_EVENT_TABLE_MODEL_CHANGED = 27;
const short UM_EVENT_TABLE_ROW_HEADER_CHANGED = 28;
const short UM_EVENT_TABLE_SUMMARY_CHANGED = 29;
const short UM_EVENT_OBJECT_REORDER = 30;
const short UM_EVENT_PAGE_CHANGED = 31;
const short UM_EVENT_CHILD_ADDED = 32;
const short UM_EVENT_CHILD_REMOVED = 33;
const short UM_EVENT_TABLE_ROW_DESCRIPTION_CHANGED = 34;
const short UM_EVENT_SELECTION_CHANGED_ADD = 35;
const short UM_EVENT_SELECTION_CHANGED_REMOVE = 36;
const short UM_EVENT_SELECTION_CHANGED_WITHIN = 37;

//support for PAGE_CHANGED event
const short UM_EVENT_OBJECT_PAGECHANGED = 38;

//to add TEXT_SELECTION_CHANGED event
const short UM_EVENT_TEXT_SELECTION_CHANGED = 39;

//for section change and column change event
const short UM_EVENT_SECTION_CHANGED = 40;
const short UM_EVENT_COLUMN_CHANGED = 41;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
