/*************************************************************************
 *
 *  $RCSfile: select.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:00 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    ScTripel        aAnchorPos;

    ScSplitPos      GetWhich();

public:
                    ScViewFunctionSet( ScViewData* pNewViewData );

    void            SetSelectionEngine( ScViewSelectionEngine* pSelEngine );

    void            SetAnchor( USHORT nPosX, USHORT nPosY );
    void            SetAnchorFlag( BOOL bSet );

    virtual void    BeginDrag();
    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();
    virtual BOOL    SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE );
    virtual BOOL    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    BOOL            SetCursorAtCell( short nPosX, short nPosY, BOOL bScroll );
};


// ---------------------------------------------------------------------------


class ScHeaderFunctionSet : public FunctionSet          // Spalten- / Zeilenkoepfe
{
private:
    ScViewData*     pViewData;
    BOOL            bColumn;                // Col- / Rowbar
    ScSplitPos      eWhich;

    BOOL            bAnchor;
    USHORT          nCursorPos;

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
