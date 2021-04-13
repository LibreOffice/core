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

#include <svimpbox.hxx>

class SvTreeListBox;
class Point;

class IconViewImpl : public SvImpLBox
{
public:
    IconViewImpl(SvTreeListBox* pTreeListBox, SvTreeList* pTreeList, WinBits nWinStyle);

    void KeyDown(bool bPageDown) override;

    void KeyUp(bool bPageUp) override;

    Point GetEntryPosition(const SvTreeListEntry* pEntry) const override;

    SvTreeListEntry* GetClickedEntry(const Point& rPoint) const override;

    bool IsEntryInView(SvTreeListEntry* pEntry) const override;

    void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    // returns 0 if position is just past the last entry
    SvTreeListEntry* GetEntry(const Point& rPoint) const override;

    void UpdateAll(bool bInvalidateCompleteView) override;

    bool KeyInput(const KeyEvent&) override;

    void InvalidateEntry(tools::Long nId) const override;

protected:
    tools::Long GetEntryLine(const SvTreeListEntry* pEntry) const override;

    void CursorUp() override;
    void CursorDown() override;
    void PageDown(sal_uInt16 nDelta) override;
    void PageUp(sal_uInt16 nDelta) override;

    void SyncVerThumb() override;
    void AdjustScrollBars(Size& rSize) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
