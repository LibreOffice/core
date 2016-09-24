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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SELECT_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SELECT_HXX

#include <vcl/seleng.hxx>

#include "viewdata.hxx"

class ScTabView;
class ScViewData;

class ScViewSelectionEngine : public SelectionEngine
{
private:
    ScSplitPos      eWhich;
public:
                    ScViewSelectionEngine( vcl::Window* pWindow, ScTabView* pView,
                                                    ScSplitPos eSplitPos );

    ScSplitPos      GetWhich() const            { return eWhich; }
    void            SetWhich(ScSplitPos eNew)   { eWhich = eNew; }
};

class ScViewFunctionSet : public FunctionSet            // View (Gridwin / keyboard)
{
private:
    ScViewData*             pViewData;
    ScViewSelectionEngine*  pEngine;

    bool            bAnchor;
    bool            bStarted;
    ScAddress       aAnchorPos;

    ScSplitPos      GetWhich();

    sal_uLong           CalcUpdateInterval( const Size& rWinSize, const Point& rEffPos,
                                        bool bLeftScroll, bool bTopScroll, bool bRightScroll, bool bBottomScroll );

public:
                    ScViewFunctionSet( ScViewData* pNewViewData );

    void            SetSelectionEngine( ScViewSelectionEngine* pSelEngine );

    void            SetAnchor( SCCOL nPosX, SCROW nPosY );
    void            SetAnchorFlag( bool bSet );

    virtual void    BeginDrag() override;
    virtual void    CreateAnchor() override;
    virtual void    DestroyAnchor() override;
    virtual bool    SetCursorAtPoint( const Point& rPointPixel, bool bDontSelectAtCursor = false ) override;
    virtual bool    IsSelectionAtPoint( const Point& rPointPixel ) override;
    virtual void    DeselectAtPoint( const Point& rPointPixel ) override;
    virtual void    DeselectAll() override;

    bool            SetCursorAtCell( SCsCOL nPosX, SCsROW nPosY, bool bScroll );
};

class ScHeaderFunctionSet : public FunctionSet          // Column / row headers
{
private:
    ScViewData*     pViewData;
    bool            bColumn;                // Col- / Rowbar
    ScSplitPos      eWhich;

    bool            bAnchor;
    SCCOLROW        nCursorPos;

public:
                    ScHeaderFunctionSet( ScViewData* pNewViewData );

    void            SetColumn( bool bSet );
    void            SetWhich( ScSplitPos eNew );

    virtual void    BeginDrag() override;
    virtual void    CreateAnchor() override;
    virtual void    DestroyAnchor() override;
    virtual bool    SetCursorAtPoint( const Point& rPointPixel, bool bDontSelectAtCursor = false ) override;
    virtual bool    IsSelectionAtPoint( const Point& rPointPixel ) override;
    virtual void    DeselectAtPoint( const Point& rPointPixel ) override;
    virtual void    DeselectAll() override;

    void            SetAnchorFlag(bool bSet)    { bAnchor = bSet; }
};

class ScHeaderSelectionEngine : public SelectionEngine
{
public:
                    ScHeaderSelectionEngine( vcl::Window* pWindow, ScHeaderFunctionSet* pFuncSet );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
