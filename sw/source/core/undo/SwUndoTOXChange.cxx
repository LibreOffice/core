/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwUndoTOXChange.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:16:34 $
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
#include <SwUndoTOXChange.hxx>
#include <swundo.hxx>
#include <doctxm.hxx>

SwUndoTOXChange::SwUndoTOXChange(SwTOXBase * _pTOX, const SwTOXBase & rNew)
    : SwUndo(UNDO_TOXCHANGE), pTOX(_pTOX), aOld(*_pTOX), aNew(rNew)
{
}

SwUndoTOXChange::~SwUndoTOXChange()
{
}

void SwUndoTOXChange::UpdateTOXBaseSection()
{
    if (pTOX->ISA(SwTOXBaseSection))
    {
        SwTOXBaseSection * pTOXBase = static_cast<SwTOXBaseSection *>(pTOX);

        pTOXBase->Update();
        pTOXBase->UpdatePageNum();
    }
}

void SwUndoTOXChange::Undo(SwUndoIter & rIter)
{
    *pTOX = aOld;

    UpdateTOXBaseSection();
}

void SwUndoTOXChange::Redo(SwUndoIter & rIter)
{
    *pTOX = aNew;

    UpdateTOXBaseSection();
}

void SwUndoTOXChange::Repeat(SwUndoIter & rIter)
{
    Redo(rIter);
}
