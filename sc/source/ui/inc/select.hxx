/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
