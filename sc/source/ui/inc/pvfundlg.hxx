/*************************************************************************
 *
 *  $RCSfile: pvfundlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:00 $
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

#ifndef SC_PVFUNDLG_HXX
#define SC_PVFUNDLG_HXX

#ifndef _SV_HXX
#endif
//#ifndef _SVTOOL_HXX
//#include <svtool.hxx>
//#endif
//#include <svtfixed.hxx>

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

//===================================================================

class ScPivotFunctionDlg : public ModalDialog
{
public:
        ScPivotFunctionDlg( Window*         pParent,
                            BOOL            bSubTotalFunc,
                            const String&   rName,
                            USHORT          nFunctions,
                            BOOL            bIsShowAll );
        ~ScPivotFunctionDlg();

    USHORT  GetFuncMask() const { return nFuncMask; }
    BOOL    GetShowAll() const;

private:
    RadioButton         aBtnNone;
    RadioButton         aBtnAuto;
    RadioButton         aBtnUser;
    MultiListBox        aLbFunc;
    GroupBox            aGbFunc;
    CheckBox            aCbShowAll;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    FixedText           aFtNameLabel;
    FixedInfo           aFtName;

    USHORT              nFuncMask;

private:
    void InitFuncLb     ( USHORT nFunctions );
    void SetUI      ( BOOL bSubTotals );
    // Handler
    DECL_LINK( ClickHdl, PushButton * );
    DECL_LINK( DblClickHdl, MultiListBox * );
    DECL_LINK( RadioClickHdl, RadioButton * );
};


#endif // SC_PVFUNDLG_HXX


