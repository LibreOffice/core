/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inpdlg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-11-06 14:53:00 $
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
#ifndef _INPDLG_HXX
#define _INPDLG_HXX

#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif

#ifndef _SV_SVMEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

class SwInputField;
class SwSetExpField;
class SwUserFieldType;
class SwField;
class SwWrtShell;

/*--------------------------------------------------------------------
     Beschreibung: Einfuegen Felder
 --------------------------------------------------------------------*/

class SwFldInputDlg: public SvxStandardDialog
{
    virtual void    Apply();
    virtual void    StateChanged( StateChangedType );

    SwWrtShell         &rSh;
    SwInputField*       pInpFld;
    SwSetExpField*      pSetFld;
    SwUserFieldType*    pUsrType;

    Edit                aLabelED;

    MultiLineEdit       aEditED;
    FixedLine           aEditFL;

    OKButton            aOKBT;
    CancelButton        aCancelBT;
    PushButton          aNextBT;
    HelpButton          aHelpBT;

    DECL_LINK(NextHdl, PushButton*);
public:
    SwFldInputDlg(  Window *pParent, SwWrtShell &rSh,
                    SwField* pField, BOOL bNextButton = FALSE );
    ~SwFldInputDlg();
};


#endif
