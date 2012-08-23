/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
