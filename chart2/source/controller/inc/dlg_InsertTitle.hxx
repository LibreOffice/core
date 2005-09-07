/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertTitle.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:27:49 $
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
#ifndef _CHART2_DLG_INSERT_TITLE_GRID_HXX
#define _CHART2_DLG_INSERT_TITLE_GRID_HXX

// header for class ModalDialog
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
// header for class Edit
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
// header for class CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

struct InsertTitleDialogData
{
    ::com::sun::star::uno::Sequence< sal_Bool > aPossibilityList;
    ::com::sun::star::uno::Sequence< sal_Bool > aExistenceList;
    ::com::sun::star::uno::Sequence< rtl::OUString > aTextList;

    InsertTitleDialogData();
};

/*************************************************************************
|*
|* Titel-Dialog
|*
\************************************************************************/

class SchTitleDlg : public ModalDialog
{
private:
    CheckBox            aCbxMain;
    Edit                aEdtMain;
    CheckBox            aCbxSub;
    Edit                aEdtSub;
    CheckBox            aCbxXAxis;
    Edit                aEdtXAxis;
    CheckBox            aCbxYAxis;
    Edit                aEdtYAxis;
    CheckBox            aCbxZAxis;
    Edit                aEdtZAxis;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    DECL_LINK(EnableTitleHdl, CheckBox*);

public:
    SchTitleDlg( Window* pParent, const InsertTitleDialogData& rInput );
    virtual ~SchTitleDlg();

    void getResult( InsertTitleDialogData& rOutput );
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

