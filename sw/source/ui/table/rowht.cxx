/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/dlgutil.hxx>


#include <fmtfsize.hxx>
#include <swtypes.hxx>
#include <rowht.hxx>
#include <wrtsh.hxx>
#include <frmatr.hxx>
#include <wdocsh.hxx>
#include <view.hxx>
#include <swmodule.hxx>
#include <usrpref.hxx>

#include <cmdid.h>
#include <rowht.hrc>
#include <table.hrc>




void SwTableHeightDlg::Apply()
{
    SwTwips nHeight = static_cast< SwTwips >(aHeightEdit.Denormalize(aHeightEdit.GetValue(FUNIT_TWIP)));
    SwFmtFrmSize aSz(ATT_FIX_SIZE, 0, nHeight);

    SwFrmSize eFrmSize = (SwFrmSize) aAutoHeightCB.IsChecked() ?
        ATT_MIN_SIZE : ATT_FIX_SIZE;
    if(eFrmSize != aSz.GetHeightSizeType())
    {
        aSz.SetHeightSizeType(eFrmSize);
    }
    rSh.SetRowHeight( aSz );
}

// CTOR / DTOR -----------------------------------------------------------


SwTableHeightDlg::SwTableHeightDlg( Window *pParent, SwWrtShell &rS ) :

    SvxStandardDialog(pParent, SW_RES(DLG_ROW_HEIGHT)),
    aHeightFL(this, SW_RES(FL_HEIGHT)),

    aHeightEdit(this, SW_RES(ED_HEIGHT)),
    aAutoHeightCB(this, SW_RES(CB_AUTOHEIGHT)),
    aOKBtn(this, SW_RES(BT_OK)),
    aCancelBtn(this, SW_RES(BT_CANCEL)),
    aHelpBtn( this, SW_RES( BT_HELP ) ),
    rSh( rS )
{
    FreeResource();

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( 0 != PTR_CAST( SwWebDocShell,
                                rSh.GetView().GetDocShell() ) )->GetMetric();
    ::SetFieldUnit( aHeightEdit, eFieldUnit );

    aHeightEdit.SetMin(MINLAY, FUNIT_TWIP);
    if(!aHeightEdit.GetMin())
        aHeightEdit.SetMin(1);
    SwFmtFrmSize *pSz;
    rSh.GetRowHeight( pSz );
    if ( pSz )
    {
        long nHeight = pSz->GetHeight();
        aAutoHeightCB.Check(pSz->GetHeightSizeType() != ATT_FIX_SIZE);
        aHeightEdit.SetValue(aHeightEdit.Normalize(nHeight), FUNIT_TWIP);

        delete pSz;
    }
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
