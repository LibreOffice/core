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
#ifndef CHART_SHAPECONTROLLER_H
#define CHART_SHAPECONTROLLER_H

//Shape Controller Command Ids:
enum ShapeControllerCommandID
{
    COMMAND_ID_FORMAT_LINE              =  1,
    COMMAND_ID_FORMAT_AREA              =  2,
    COMMAND_ID_TEXT_ATTRIBUTES          =  3,
    COMMAND_ID_TRANSFORM_DIALOG         =  4,
    COMMAND_ID_OBJECT_TITLE_DESCRIPTION =  5,
    COMMAND_ID_RENAME_OBJECT            =  6,
    COMMAND_ID_BRING_TO_FRONT           =  8,
    COMMAND_ID_FORWARD                  =  9,
    COMMAND_ID_BACKWARD                 = 10,
    COMMAND_ID_SEND_TO_BACK             = 11,
    COMMAND_ID_FONT_DIALOG              = 15,
    COMMAND_ID_PARAGRAPH_DIALOG         = 16
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
