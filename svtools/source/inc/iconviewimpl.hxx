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

#ifndef INCLUDED_SVTOOLS_SOURCE_INC_ICONVIEWIMPL_HXX
#define INCLUDED_SVTOOLS_SOURCE_INC_ICONVIEWIMPL_HXX

#include <svimpbox.hxx>

class SvTreeListBox;
class Point;

class IconViewImpl : public SvImpLBox
{
public:
    IconViewImpl( SvTreeListBox* pTreeListBox, SvTreeList* pTreeList, WinBits nWinStyle );

    void KeyDown( bool bPageDown ) SAL_OVERRIDE;

    void KeyUp( bool bPageUp ) SAL_OVERRIDE;

    Point GetEntryPosition( SvTreeListEntry* pEntry ) const SAL_OVERRIDE;

    SvTreeListEntry* GetClickedEntry( const Point& rPoint ) const SAL_OVERRIDE;

    bool IsEntryInView( SvTreeListEntry* pEntry ) const SAL_OVERRIDE;

    void Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) SAL_OVERRIDE;

    // returns 0 if position is just past the last entry
    SvTreeListEntry* GetEntry( const Point& rPoint ) const SAL_OVERRIDE;

    void UpdateAll( bool bInvalidateCompleteView ) SAL_OVERRIDE;

    bool KeyInput( const KeyEvent& ) SAL_OVERRIDE;

    void InvalidateEntry( long nId ) const SAL_OVERRIDE;

protected:
    long GetEntryLine( SvTreeListEntry* pEntry ) const SAL_OVERRIDE;

    void CursorUp() SAL_OVERRIDE;
    void CursorDown() SAL_OVERRIDE;
    void PageDown( sal_uInt16 nDelta ) SAL_OVERRIDE;
    void PageUp( sal_uInt16 nDelta ) SAL_OVERRIDE;

    void SyncVerThumb() SAL_OVERRIDE;
    void AdjustScrollBars( Size& rSize ) SAL_OVERRIDE;
};

#endif // INCLUDED_SVTOOLS_SOURCE_INC_ICONVIEWIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
