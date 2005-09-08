/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editsel.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:28:50 $
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

#ifndef _EDITSEL_HXX
#define _EDITSEL_HXX

#ifndef _SV_SELENG_HXX //autogen
#include <vcl/seleng.hxx>
#endif

class EditView;

//  ----------------------------------------------------------------------
//  class EditSelFunctionSet
//  ----------------------------------------------------------------------
class EditSelFunctionSet: public FunctionSet
{
private:
    EditView*       pCurView;

public:
                    EditSelFunctionSet();

    virtual void    BeginDrag();

    virtual void    CreateAnchor();
    virtual void    DestroyAnchor();

    virtual BOOL    SetCursorAtPoint( const Point& rPointPixel, BOOL bDontSelectAtCursor = FALSE );

    virtual BOOL    IsSelectionAtPoint( const Point& rPointPixel );
    virtual void    DeselectAtPoint( const Point& rPointPixel );
    virtual void    DeselectAll();

    void            SetCurView( EditView* pView )       { pCurView = pView; }
    EditView*       GetCurView()                        { return pCurView; }
};

// ----------------------------------------------------------------------
//  class EditSelectionEngine
//  ----------------------------------------------------------------------
class EditSelectionEngine : public SelectionEngine
{
private:

public:
                    EditSelectionEngine();

    void            SetCurView( EditView* pNewView );
    EditView*       GetCurView();
};

#endif // _EDITSEL_HXX
