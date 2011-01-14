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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"


#include "dlgedpage.hxx"
#include "dlged.hxx"
#include "dlgedmod.hxx"
#include "dlgedobj.hxx"


TYPEINIT1( DlgEdPage, SdrPage );

//----------------------------------------------------------------------------

DlgEdPage::DlgEdPage( DlgEdModel& rModel, FASTBOOL bMasterPage )
    :SdrPage( rModel, bMasterPage )
{
}

//----------------------------------------------------------------------------

DlgEdPage::DlgEdPage( const DlgEdPage& rPage )
    :SdrPage( rPage )
{
    pDlgEdForm = rPage.pDlgEdForm;
}

//----------------------------------------------------------------------------

DlgEdPage::~DlgEdPage()
{
    Clear();
}

//----------------------------------------------------------------------------

SdrPage* DlgEdPage::Clone() const
{
    return new DlgEdPage( *this );
}

//----------------------------------------------------------------------------

SdrObject* DlgEdPage::SetObjectOrdNum(sal_uLong nOldObjNum, sal_uLong nNewObjNum)
{
    SdrObject* pObj = SdrPage::SetObjectOrdNum( nOldObjNum, nNewObjNum );

    DlgEdHint aHint( DLGED_HINT_OBJORDERCHANGED );
    if ( pDlgEdForm )
    {
        DlgEditor* pDlgEditor = pDlgEdForm->GetDlgEditor();
        if ( pDlgEditor )
            pDlgEditor->Broadcast( aHint );
    }

    return pObj;
}

//----------------------------------------------------------------------------
