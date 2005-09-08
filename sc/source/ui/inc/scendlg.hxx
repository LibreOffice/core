/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scendlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:47:33 $
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

#ifndef SC_SCENDLG_HXX
#define SC_SCENDLG_HXX


#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif

//===================================================================

class ScNewScenarioDlg : public ModalDialog
{
public:
        ScNewScenarioDlg( Window* pParent, const String& rName, BOOL bEdit = FALSE, BOOL bSheetProtected = FALSE );
        ~ScNewScenarioDlg();

    void SetScenarioData( const String& rName, const String& rComment,
                            const Color& rColor, USHORT nFlags );

    void GetScenarioData( String& rName, String& rComment,
                            Color& rColor, USHORT& rFlags ) const;

private:
    FixedLine           aFlName;
    Edit                aEdName;
    FixedLine           aFlComment;
    MultiLineEdit       aEdComment;
    FixedLine           aFlOptions;
    CheckBox            aCbShowFrame;
    ColorListBox        aLbColor;
    //CheckBox          aCbPrintFrame;
    CheckBox            aCbTwoWay;
    //CheckBox          aCbAttrib;
    //CheckBox          aCbValue;
    CheckBox            aCbCopyAll;
    CheckBox            aCbProtect;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    const String        aDefScenarioName;
    BOOL                bIsEdit;

    DECL_LINK( OkHdl, OKButton * );
    DECL_LINK( EnableHdl, CheckBox * );
};


#endif // SC_SCENDLG_HXX


