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

#ifndef _SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif

#include "viewdata.hxx"		// ScSplitPos
namespace binfilter {

// ---------------------------------------------------------------------------

class ScTabView;
class ScViewData;


class ScViewSelectionEngine : public SelectionEngine
{
private:
    ScSplitPos		eWhich;
public:
                    ScViewSelectionEngine( Window* pWindow, ScTabView* pView,
                                                    ScSplitPos eSplitPos );

    ScSplitPos		GetWhich() const			{ return eWhich; }
    void			SetWhich(ScSplitPos eNew)	{ eWhich = eNew; }
};


class ScViewFunctionSet : public FunctionSet			// View (Gridwin / Tastatur)
{
private:
    ScViewData*				pViewData;
    ScViewSelectionEngine*	pEngine;

    BOOL			bAnchor;
    BOOL			bStarted;
    ScTripel		aAnchorPos;


public:
                    ScViewFunctionSet( ScViewData* pNewViewData );

    void			SetSelectionEngine( ScViewSelectionEngine* pSelEngine );


    virtual void	BeginDrag(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001  	virtual void	BeginDrag();
    virtual void	CreateAnchor(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void	CreateAnchor();
    virtual void	DestroyAnchor(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void	DestroyAnchor();
    virtual BOOL	SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE ){DBG_BF_ASSERT(0, "STRIP");return FALSE;} //STRIP001 virtual BOOL	SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE );
    virtual BOOL	IsSelectionAtPoint( const Point& rPointPixel ){DBG_BF_ASSERT(0, "STRIP"); return FALSE;} //STRIP001 virtual BOOL	IsSelectionAtPoint( const Point& rPointPixel );
    virtual void	DeselectAtPoint( const Point& rPointPixel ){DBG_ASSERT(0, "STRIP")}; //STRIP001 virtual void	DeselectAtPoint( const Point& rPointPixel );
    virtual void	DeselectAll(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void	DeselectAll();

};


// ---------------------------------------------------------------------------


class ScHeaderFunctionSet : public FunctionSet			// Spalten- / Zeilenkoepfe
{
private:
    ScViewData*		pViewData;
    BOOL			bColumn;				// Col- / Rowbar
    ScSplitPos		eWhich;

    BOOL			bAnchor;
    USHORT			nCursorPos;

public:
                    ScHeaderFunctionSet( ScViewData* pNewViewData );


    virtual void	BeginDrag(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void	BeginDrag();
    virtual void	CreateAnchor(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void	CreateAnchor();
    virtual void	DestroyAnchor(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void	DestroyAnchor();
    virtual BOOL	SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE ){DBG_BF_ASSERT(0, "STRIP");return FALSE;} //STRIP001 virtual BOOL	SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE );
    virtual BOOL	IsSelectionAtPoint( const Point& rPointPixel ){DBG_BF_ASSERT(0, "STRIP");return FALSE;}//STRIP001 	virtual BOOL	IsSelectionAtPoint( const Point& rPointPixel );
    virtual void	DeselectAtPoint( const Point& rPointPixel ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void	DeselectAtPoint( const Point& rPointPixel );
    virtual void	DeselectAll(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 virtual void	DeselectAll();

    void			SetAnchorFlag(BOOL bSet)	{ bAnchor = bSet; }
};


class ScHeaderSelectionEngine : public SelectionEngine
{
public:
                    ScHeaderSelectionEngine( Window* pWindow, ScHeaderFunctionSet* pFuncSet );
};



} //namespace binfilter
#endif
