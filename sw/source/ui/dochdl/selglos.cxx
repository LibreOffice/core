/*************************************************************************
 *
 *  $RCSfile: selglos.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mh $ $Date: 2001-10-25 16:54:08 $
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
IMPL_LINK(SwSelGlossaryDlg, DoubleClickHdl, ListBox*, pBox)
{
    EndDialog(RET_OK);
    return 0;
}
