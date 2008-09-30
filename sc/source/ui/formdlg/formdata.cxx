/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: formdata.cxx,v $
 * $Revision: 1.6.32.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#include <tools/debug.hxx>

#include "formdata.hxx"

//============================================================================

ScFormEditData::ScFormEditData()
{
    Reset();
}

ScFormEditData::~ScFormEditData()
{
    delete pParent;
}

ScFormEditData::ScFormEditData( const ScFormEditData& r )
{
    *this = r;
}

void ScFormEditData::Reset()
{
    pParent = NULL;
    nMode = 0;
    nFStart = 0;
    nCatSel = 1;        //! oder 0 (zuletzt benutzte)
    nFuncSel = 0;
    nOffset = 0;
    nEdFocus = 0;
    bMatrix =FALSE;
    nUniqueId=0;
    aSelection.Min()=0;
    aSelection.Max()=0;
    aUndoStr.Erase();
}

const ScFormEditData& ScFormEditData::operator=( const ScFormEditData& r )
{
    pParent         = r.pParent;
    nMode           = r.nMode;
    nFStart         = r.nFStart;
    nCatSel         = r.nCatSel;
    nFuncSel        = r.nFuncSel;
    nOffset         = r.nOffset;
    nEdFocus        = r.nEdFocus;
    aUndoStr        = r.aUndoStr;
    bMatrix         = r.bMatrix ;
    nUniqueId       = r.nUniqueId;
    aSelection      = r.aSelection;
    return *this;
}

void ScFormEditData::SaveValues()
{
    ScFormEditData* pTemp = new ScFormEditData(*this);

    Reset();
    pParent = pTemp;
}



