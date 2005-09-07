/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlg_InsertAxis_Grid.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:26:48 $
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
#ifndef _CHART2_DLG_INSERT_AXIS_GRID_HXX
#define _CHART2_DLG_INSERT_AXIS_GRID_HXX

// header for class ModalDialog
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
// header for class FixedLine
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
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

struct InsertMeterDialogData
{
    ::com::sun::star::uno::Sequence< sal_Bool > aPossibilityList;
    ::com::sun::star::uno::Sequence< sal_Bool > aExistenceList;

    InsertMeterDialogData();
};

/*************************************************************************
|*
|* insert Axis dialog (also base for grid dialog)
|*
\************************************************************************/
class SchAxisDlg : public ModalDialog
{
protected:
    FixedLine           aFlPrimary;
    FixedLine           aFlPrimaryGrid;
    CheckBox            aCbPrimaryX;
    CheckBox            aCbPrimaryY;
    CheckBox            aCbPrimaryZ;

    FixedLine           aFlSecondary;
    FixedLine           aFlSecondaryGrid;
    CheckBox            aCbSecondaryX;
    CheckBox            aCbSecondaryY;
    CheckBox            aCbSecondaryZ;

    OKButton            aPbOK;
    CancelButton        aPbCancel;
    HelpButton          aPbHelp;

public:
    SchAxisDlg( Window* pParent, const InsertMeterDialogData& rInput, BOOL bAxisDlg=true );
    virtual ~SchAxisDlg();

    void getResult( InsertMeterDialogData& rOutput );
};


/*************************************************************************
|*
|* Grid dialog
|*
\************************************************************************/
class SchGridDlg : public SchAxisDlg
{
public:
    SchGridDlg( Window* pParent, const InsertMeterDialogData& rInput );
    virtual ~SchGridDlg();
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
