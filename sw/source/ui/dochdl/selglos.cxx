/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: selglos.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:39:34 $
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



#include "swtypes.hxx"

#include "selglos.hxx"

#include "selglos.hrc"
#include "dochdl.hrc"

#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

// STATIC DATA -----------------------------------------------------------


// CTOR / DTOR -----------------------------------------------------------

SwSelGlossaryDlg::SwSelGlossaryDlg(Window * pParent, const String &rShortName)
    : ModalDialog(pParent, SW_RES(DLG_SEL_GLOS)),
    aGlosBox(this, SW_RES( LB_GLOS)),
    aGlosFL(this, SW_RES( FL_GLOS)),
    aOKBtn(this, SW_RES( BT_OK)),
    aCancelBtn(this, SW_RES( BT_CANCEL)),
    aHelpBtn(this, SW_RES(BT_HELP))
{
    String sText(aGlosFL.GetText());
    sText += rShortName;
    aGlosFL.SetText(sText);
    FreeResource();

    aGlosBox.SetDoubleClickHdl(LINK(this, SwSelGlossaryDlg, DoubleClickHdl));
}

/*-----------------25.02.94 20:50-------------------
 dtor ueberladen
--------------------------------------------------*/
SwSelGlossaryDlg::~SwSelGlossaryDlg() {}
/* -----------------25.10.99 08:33-------------------

 --------------------------------------------------*/
IMPL_LINK(SwSelGlossaryDlg, DoubleClickHdl, ListBox*, /*pBox*/)
{
    EndDialog(RET_OK);
    return 0;
}
