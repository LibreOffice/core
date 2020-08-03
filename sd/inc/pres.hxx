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

enum class PresObjKind
{
    NONE = 0,
    Title,
    Outline,
    Text,
    Graphic,
    Object,
    Chart,
    OrgChart,
    Table,
    Page,
    Handout,
    Notes,
    Header,
    Footer,
    DateTime,
    SlideNumber,
    Calc,
    Media,
    LAST = Media
};

enum class PageKind
{
    Standard,
    Notes,
    Handout,
    LAST = Handout
};

enum class EditMode
{
    Page,
    MasterPage
};

enum class DocumentType
{
    Impress,
    Draw
};

enum NavigatorDragType
{
    NAVIGATOR_DRAGTYPE_NONE,
    NAVIGATOR_DRAGTYPE_URL,
    NAVIGATOR_DRAGTYPE_LINK,
    NAVIGATOR_DRAGTYPE_EMBEDDED
};
#define NAVIGATOR_DRAGTYPE_COUNT 4

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
