/*************************************************************************
 *
 *  $RCSfile: insrc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2002-12-05 12:47:59 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXINTITEM_HXX //autogen
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif


#include "cmdid.h"
#include "swtypes.hxx"
#include "insrc.hxx"
#include "view.hxx"
#include "table.hrc"
#include "insrc.hrc"




void SwInsRowColDlg::Apply()
{
    USHORT nSlot = bColumn ? FN_TABLE_INSERT_COL : FN_TABLE_INSERT_ROW;
    SfxUInt16Item aCount( nSlot, aCountEdit.GetValue() );
    SfxBoolItem  aAfter( FN_PARAM_INSERT_AFTER, aAfterBtn.IsChecked() );
    rView.GetViewFrame()->GetDispatcher()->Execute( nSlot,
            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD,
            &aCount, &aAfter, 0L);
}



SwInsRowColDlg::SwInsRowColDlg(SwView& rVw, BOOL bCol )
    : SvxStandardDialog( rVw.GetWindow(), SW_RES(DLG_INS_ROW_COL) ),
    aCount( this, SW_RES( FT_COUNT ) ),
    aCountEdit( this, SW_RES( ED_COUNT ) ),
    aInsFL( this, SW_RES( FL_INS ) ),
    aBeforeBtn( this, SW_RES( CB_POS_BEFORE ) ),
    aAfterBtn( this, SW_RES( CB_POS_AFTER ) ),
    aPosFL( this, SW_RES( FL_POS ) ),
    aRow(SW_RES(STR_ROW)),
    aCol(SW_RES(STR_COL)),
    aOKBtn( this, SW_RES( BT_OK ) ),
    aCancelBtn( this, SW_RES( BT_CANCEL ) ),
    aHelpBtn( this, SW_RES( BT_HELP ) ),
    rView(rVw),
    bColumn( bCol )
{
    FreeResource();
    String aTmp( GetText() );
    if( bColumn )
    {
        aTmp += aCol;
        SetHelpId( FN_TABLE_INSERT_COL_DLG );
    }
    else
    {
        aTmp += aRow;
        SetHelpId( FN_TABLE_INSERT_ROW_DLG );
    }
    SetText( aTmp );
}




