/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SwUndoTOXChange.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _SW_UNDO_TOX_CHANGE_HXX
#define _SW_UNDO_TOX_CHANGE_HXX
#include <undobj.hxx>
#include <tox.hxx>

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
