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

#ifndef SC_SELECT_HXX
#define SC_SELECT_HXX

#include <vcl/seleng.hxx>

#include "viewdata.hxx"     // ScSplitPos

// ---------------------------------------------------------------------------

class ScTabView;
class ScViewData;


class ScViewSelectionEngine : public SelectionEngine
{
private:
    ScSplitPos      eWhich;
public:
                    ScViewSelectionEngine( Window* pWindow, ScTabView* pView,
                                                    ScSplitPos eSplitPos );

    ScSplitPos      GetWhich() const            { return eWhich; }
    void            SetWhich(ScSplitPos eNew)   { eWhich = eNew; }
};


class ScViewFunctionSet : public FunctionSet            // View (Gridwin / keyboard)
{
private:
    ScViewData*             pViewData;
    ScViewSelectionEngine*  pEngine;

    sal_Bool            bAnchor;
    sal_Bool            bStarted;
    ScAddress       aAnchorPos;

    ScSplitPos      GetWhich();

    sal_uLong           CalcUpdateInterval( const Size& rWinSize, const Point& rEffPos,
                                        bool bLeftScroll, bool bTopScroll, bool bRightScroll, bool bBottomScroll );

public:
                    ScViewFunctionSet( ScViewData* pNewViewData );

    void            SetSelectionEngine( ScViewSelectionEngine* pSelEngine );

    void            SetAnchor( SCCOL nPosX, SCROW nPosY );
    void            SetAnchorFlag( sal_Bool bSet );

    virtual void    BeginDrag();
    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();
    virtual sal_Bool    SetCursorAtPoint( const Point& rPointPixel, sal_Bool bDontSelectAtCursor = false );
    virtual sal_Bool    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    sal_Bool            SetCursorAtCell( SCsCOL nPosX, SCsROW nPosY, sal_Bool bScroll );
};


// ---------------------------------------------------------------------------


class ScHeaderFunctionSet : public FunctionSet          // Column / row headers
{
private:
    ScViewData*     pViewData;
    sal_Bool            bColumn;                // Col- / Rowbar
    ScSplitPos      eWhich;

    sal_Bool            bAnchor;
    SCCOLROW        nCursorPos;

public:
                    ScHeaderFunctionSet( ScViewData* pNewViewData );

    void            SetColumn( sal_Bool bSet );
    void            SetWhich( ScSplitPos eNew );

    virtual void    BeginDrag();
    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();
    virtual sal_Bool    SetCursorAtPoint( const Point& rPointPixel, sal_Bool bDontSelectAtCursor = false );
    virtual sal_Bool    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    void            SetAnchorFlag(sal_Bool bSet)    { bAnchor = bSet; }
};


class ScHeaderSelectionEngine : public SelectionEngine
{
public:
                    ScHeaderSelectionEngine( Window* pWindow, ScHeaderFunctionSet* pFuncSet );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
