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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_PREVWPAGE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_PREVWPAGE_HXX

// classes <Point>, <Size> and <Rectangle>
#include <tools/gen.hxx>

class SwPageFrame;

/** data structure for a preview page in the current preview layout
*/
struct PreviewPage
{
    const SwPageFrame*  pPage;
    bool        bVisible;
    Size        aPageSize;
    Point       aPreviewWinPos;
    Point       aLogicPos;
    Point       aMapOffset;

    inline PreviewPage();
};

inline PreviewPage::PreviewPage()
    : pPage( nullptr ),
      bVisible( false ),
      aPageSize( Size(0,0) ),
      aPreviewWinPos( Point(0,0) ),
      aLogicPos( Point(0,0) ),
      aMapOffset( Point(0,0) )
{};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
