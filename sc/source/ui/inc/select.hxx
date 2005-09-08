/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: select.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:49:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_SELECT_HXX
#define SC_SELECT_HXX

#ifndef _SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif

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


class ScViewFunctionSet : public FunctionSet            // View (Gridwin / Tastatur)
{
private:
    ScViewData*             pViewData;
    ScViewSelectionEngine*  pEngine;

    BOOL            bAnchor;
    BOOL            bStarted;
    ScAddress       aAnchorPos;

    ScSplitPos      GetWhich();

public:
                    ScViewFunctionSet( ScViewData* pNewViewData );

    void            SetSelectionEngine( ScViewSelectionEngine* pSelEngine );

    void            SetAnchor( SCCOL nPosX, SCROW nPosY );
    void            SetAnchorFlag( BOOL bSet );

    virtual void    BeginDrag();
    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();
    virtual BOOL    SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE );
    virtual BOOL    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    BOOL            SetCursorAtCell( SCsCOL nPosX, SCsROW nPosY, BOOL bScroll );
};


// ---------------------------------------------------------------------------


class ScHeaderFunctionSet : public FunctionSet          // Spalten- / Zeilenkoepfe
{
private:
    ScViewData*     pViewData;
    BOOL            bColumn;                // Col- / Rowbar
    ScSplitPos      eWhich;

    BOOL            bAnchor;
    SCCOLROW        nCursorPos;

public:
                    ScHeaderFunctionSet( ScViewData* pNewViewData );

    void            SetColumn( BOOL bSet );
    void            SetWhich( ScSplitPos eNew );

    virtual void    BeginDrag();
    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();
    virtual BOOL    SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE );
    virtual BOOL    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    void            SetAnchorFlag(BOOL bSet)    { bAnchor = bSet; }
};


class ScHeaderSelectionEngine : public SelectionEngine
{
public:
                    ScHeaderSelectionEngine( Window* pWindow, ScHeaderFunctionSet* pFuncSet );
};



#endif
