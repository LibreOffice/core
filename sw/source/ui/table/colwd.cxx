/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colwd.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:32:40 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif



#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SVX_DLGUTIL_HXX //autogen
#include <svx/dlgutil.hxx>
#endif

#ifndef _COLWD_HXX
#include <colwd.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _WDOCSH_HXX
#include <wdocsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _MODCFG_HXX
#include <modcfg.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _COLWD_HRC
#include <colwd.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif


IMPL_LINK_INLINE_START( SwTableWidthDlg, LoseFocusHdl, Edit *, EMPTYARG )
{
    USHORT nId = (USHORT)aColEdit.GetValue()-1;
    const SwTwips lWidth = rFnc.GetColWidth(nId);
    aWidthEdit.SetValue(aWidthEdit.Normalize(lWidth), FUNIT_TWIP);
    aWidthEdit.SetMax(aWidthEdit.Normalize(rFnc.GetMaxColWidth(nId)), FUNIT_TWIP);
    return 0;
}
IMPL_LINK_INLINE_END( SwTableWidthDlg, LoseFocusHdl, Edit *, EMPTYARG )



SwTableWidthDlg::SwTableWidthDlg(Window *pParent, SwTableFUNC &rTableFnc ) :

    SvxStandardDialog( pParent, SW_RES(DLG_COL_WIDTH) ),

    aColFT(this,        SW_RES(FT_COL)),
    aColEdit(this,      SW_RES(ED_COL)),
    aWidthFT(this,      SW_RES(FT_WIDTH)),
    aWidthEdit(this,    SW_RES(ED_WIDTH)),
    aWidthFL(this,     SW_RES(FL_WIDTH)),
    aOKBtn(this,        SW_RES(BT_OK)),
    aCancelBtn(this,    SW_RES(BT_CANCEL)),
    aHelpBtn(this,      SW_RES(BT_HELP)),
    rFnc(rTableFnc)
{
    FreeResource();

    BOOL bIsWeb = rTableFnc.GetShell()
                    ? static_cast< BOOL >(0 != PTR_CAST( SwWebDocShell,
                            rTableFnc.GetShell()->GetView().GetDocShell()) )
                    : FALSE;
    FieldUnit eFieldUnit = SW_MOD()->GetUsrPref( bIsWeb )->GetMetric();
    ::SetFieldUnit(aWidthEdit, eFieldUnit );

    aColEdit.SetValue( rFnc.GetCurColNum() +1 );
    aWidthEdit.SetMin(aWidthEdit.Normalize(MINLAY), FUNIT_TWIP);
    if(!aWidthEdit.GetMin())
        aWidthEdit.SetMin(1);

    if(rFnc.GetColCount() == 0)
        aWidthEdit.SetMin(aWidthEdit.Normalize(rFnc.GetColWidth(0)), FUNIT_TWIP);
    aColEdit.SetMax(rFnc.GetColCount() +1 );
    aColEdit.SetModifyHdl(LINK(this,SwTableWidthDlg, LoseFocusHdl));
    LoseFocusHdl();
}



void SwTableWidthDlg::Apply()
{
    rFnc.InitTabCols();
    rFnc.SetColWidth(
            static_cast< USHORT >(aColEdit.GetValue() - 1),
            static_cast< USHORT >(aWidthEdit.Denormalize(aWidthEdit.GetValue(FUNIT_TWIP))));
}




