/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: formdata.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:01:56 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

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

void ScFormEditData::RestoreValues()
{
    ScFormEditData* pTemp = pParent;
    DBG_ASSERT(pTemp,"RestoreValues ohne Parent");
    if (pTemp)
    {
        *this = *pTemp;
        pTemp->pParent = NULL;      // sonst wird der auch geloescht!
        delete pTemp;
    }
}



