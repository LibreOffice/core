/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#include <swmodule.hxx>
#include <usrpref.hxx>

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _ROWHT_HRC
#include <rowht.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif




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

    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( 0 != dynamic_cast< SwWebDocShell* >( rSh.GetView().GetDocShell() ) )->GetMetric();
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



