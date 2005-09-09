/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwUndoTOXChange.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:30:47 $
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
#ifndef _SW_UNDO_TOX_CHANGE_HXX
#define _SW_UNDO_TOX_CHANGE_HXX
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif

#ifndef _TOX_HXX
#include <tox.hxx>
#endif

class SwUndoTOXChange : public SwUndo
{
    SwTOXBase * pTOX, aOld, aNew;

    void UpdateTOXBaseSection();

public:
    SwUndoTOXChange(SwTOXBase * pTOX, const SwTOXBase & rNew);
    virtual ~SwUndoTOXChange();

    virtual void Undo(SwUndoIter & rIter);
    virtual void Redo(SwUndoIter & rIter);
    virtual void Repeat(SwUndoIter & rIter);
};

#endif //_SW_UNDO_TOX_CHANGE_HXX
