/*************************************************************************
 *
 *  $RCSfile: dlg_InsertAxis_Grid.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:25 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "dlg_InsertAxis_Grid.hxx"
#include "dlg_InsertAxis_Grid.hrc"

#include "ResId.hxx"
#include "SchSfxItemIds.hxx"
#include "Strings.hrc"
#include "SchSlotIds.hxx"

// header for class SfxBoolItem
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

InsertMeterDialogData::InsertMeterDialogData()
        : aPossibilityList(6)
        , aExistenceList(6)
{
    for(sal_Int32 nN=6;nN--;)
        aPossibilityList[nN]=true;
    for(nN=6;nN--;)
        aExistenceList[nN]=false;
}

//==============================
//
// SchAxisDlg
//
//==============================

SchAxisDlg::SchAxisDlg( Window* pWindow
                       , const InsertMeterDialogData& rInput, BOOL bAxisDlg )
                       :
        ModalDialog( pWindow, SchResId( DLG_AXIS_OR_GRID )),

        aFlPrimary( this, ResId( FL_PRIMARY_AXIS )),
        aFlPrimaryGrid( this, ResId( FL_PRIMARY_GRID )),
        aCbPrimaryX( this, ResId( CB_X_PRIMARY )),
        aCbPrimaryY( this, ResId( CB_Y_PRIMARY )),
        aCbPrimaryZ( this, ResId( CB_Z_PRIMARY )),

        aFlSecondary( this, ResId( FL_SECONDARY_AXIS )),
        aFlSecondaryGrid( this, ResId( FL_SECONDARY_GRID )),
        aCbSecondaryX( this, ResId( CB_X_SECONDARY )),
        aCbSecondaryY( this, ResId( CB_Y_SECONDARY )),
        aCbSecondaryZ( this, ResId( CB_Z_SECONDARY )),

        aPbOK( this, ResId( BTN_OK )),
        aPbCancel( this, ResId( BTN_CANCEL )),
        aPbHelp( this, ResId( BTN_HELP ))

        //rOutAttrs( rInAttrs )
{
    if(!bAxisDlg)
    {
        SetHelpId( SID_INSERT_GRIDS );
        SetText( String( SchResId( STR_TITLE_GRID ) ) );

        aFlPrimary.Hide();
        aFlSecondary.Hide();
        aFlPrimaryGrid.Show();
        aFlSecondaryGrid.Show();
    }
    FreeResource();

    aCbPrimaryX.Check( rInput.aExistenceList[0] );
    aCbPrimaryY.Check( rInput.aExistenceList[1] );
    aCbPrimaryZ.Check( rInput.aExistenceList[2] );
    aCbSecondaryX.Check( rInput.aExistenceList[3] );
    aCbSecondaryY.Check( rInput.aExistenceList[4] );
    aCbSecondaryZ.Check( rInput.aExistenceList[5] );

    aCbPrimaryX.Enable( rInput.aPossibilityList[0] );
    aCbPrimaryY.Enable( rInput.aPossibilityList[1] );
    aCbPrimaryZ.Enable( rInput.aPossibilityList[2] );
    aCbSecondaryX.Enable( rInput.aPossibilityList[3] );
    aCbSecondaryY.Enable( rInput.aPossibilityList[4] );
    aCbSecondaryZ.Enable( rInput.aPossibilityList[5] );
}

SchAxisDlg::~SchAxisDlg()
{
}

void SchAxisDlg::getResult( InsertMeterDialogData& rOutput )
{
    rOutput.aExistenceList[0]=aCbPrimaryX.IsChecked();
    rOutput.aExistenceList[1]=aCbPrimaryY.IsChecked();
    rOutput.aExistenceList[2]=aCbPrimaryZ.IsChecked();
    rOutput.aExistenceList[3]=aCbSecondaryX.IsChecked();
    rOutput.aExistenceList[4]=aCbSecondaryY.IsChecked();
    rOutput.aExistenceList[5]=aCbSecondaryZ.IsChecked();
}

SchGridDlg::SchGridDlg( Window* pParent, const InsertMeterDialogData& rInput )
                : SchAxisDlg( pParent, rInput, false )//rInAttrs, b3D, bNet, bSecondaryX, bSecondaryY, false )
{
}

SchGridDlg::~SchGridDlg()
{
}

//.............................................................................
} //namespace chart
//.............................................................................



