/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prtqry.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 15:26:42 $
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

#ifndef _SVX_DIALOGS_HRC
#include <dialogs.hrc>
#endif
#ifndef _SVX_PRTQRY_HXX
#include <prtqry.hxx>
#endif
#ifndef _SVX_DIALMGR_HXX
#include <dialmgr.hxx>
#endif
#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

/* -----------------------------01.02.00 13:57--------------------------------

 ---------------------------------------------------------------------------*/
SvxPrtQryBox::SvxPrtQryBox(Window* pParent) :
    MessBox(pParent, 0,
            String(SVX_RES(RID_SVXSTR_QRY_PRINT_TITLE)),
            String(SVX_RES(RID_SVXSTR_QRY_PRINT_MSG)))
{
    SetImage( QueryBox::GetStandardImage() );

    AddButton(String(SVX_RES(RID_SVXSTR_QRY_PRINT_SELECTION)), RET_OK,
            BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON);

    AddButton(String(SVX_RES(RID_SVXSTR_QRY_PRINT_ALL)), 2, 0);
    AddButton(BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON);
    SetButtonHelpText( RET_OK, String() );
}
/* -----------------------------01.02.00 13:57--------------------------------

 ---------------------------------------------------------------------------*/
SvxPrtQryBox::~SvxPrtQryBox()
{
}


