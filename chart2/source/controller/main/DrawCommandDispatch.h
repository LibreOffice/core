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
#ifndef CHART_DRAWCOMMANDDISPATCH_HRC
#define CHART_DRAWCOMMANDDISPATCH_HRC

//Draw Command Ids:
enum DrawCommandID
{
    COMMAND_ID_OBJECT_SELECT              = 1,
    COMMAND_ID_DRAW_LINE                  = 2,
    COMMAND_ID_LINE_ARROW_END             = 3,
    COMMAND_ID_DRAW_RECT                  = 4,
    COMMAND_ID_DRAW_ELLIPSE               = 5,
    COMMAND_ID_DRAW_FREELINE_NOFILL       = 6,
    COMMAND_ID_DRAW_TEXT                  = 7,
    COMMAND_ID_DRAW_TEXT_VERTICAL         = 8,
    COMMAND_ID_DRAW_CAPTION               = 9,
    COMMAND_ID_DRAW_CAPTION_VERTICAL      = 10,
    COMMAND_ID_DRAWTBX_CS_BASIC           = 11,
    COMMAND_ID_DRAWTBX_CS_SYMBOL          = 12,
    COMMAND_ID_DRAWTBX_CS_ARROW           = 13,
    COMMAND_ID_DRAWTBX_CS_FLOWCHART       = 14,
    COMMAND_ID_DRAWTBX_CS_CALLOUT         = 15,
    COMMAND_ID_DRAWTBX_CS_STAR            = 16
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
