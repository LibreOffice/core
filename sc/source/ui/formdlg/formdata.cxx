/*************************************************************************
 *
 *  $RCSfile: formdata.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:57 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include "segmentc.hxx"
#endif
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



