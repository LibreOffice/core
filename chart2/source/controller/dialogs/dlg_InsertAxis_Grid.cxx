/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "dlg_InsertAxis_Grid.hxx"
#include "dlg_InsertAxis_Grid.hrc"

#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "HelpIds.hrc"
#include "NoWarningThisInCTOR.hxx"
#include "ObjectNameProvider.hxx"

// header for class SfxBoolItem
#include <svl/eitem.hxx>

//.............................................................................
namespace chart
{
//.............................................................................

InsertAxisOrGridDialogData::InsertAxisOrGridDialogData()
        : aPossibilityList(6)
        , aExistenceList(6)
{
    sal_Int32 nN = 0;
    for(nN=6;nN--;)
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
                       , const InsertAxisOrGridDialogData& rInput, sal_Bool bAxisDlg )
                       :
        ModalDialog( pWindow, SchResId( DLG_AXIS_OR_GRID )),

        aFlPrimary( this, SchResId( FL_PRIMARY_AXIS )),
        aFlPrimaryGrid( this, SchResId( FL_PRIMARY_GRID )),
        aCbPrimaryX( this, SchResId( CB_X_PRIMARY )),
        aCbPrimaryY( this, SchResId( CB_Y_PRIMARY )),
        aCbPrimaryZ( this, SchResId( CB_Z_PRIMARY )),

        aFlSecondary( this, SchResId( FL_SECONDARY_AXIS )),
        aFlSecondaryGrid( this, SchResId( FL_SECONDARY_GRID )),
        aCbSecondaryX( this, SchResId( CB_X_SECONDARY )),
        aCbSecondaryY( this, SchResId( CB_Y_SECONDARY )),
        aCbSecondaryZ( this, SchResId( CB_Z_SECONDARY )),

        aPbOK( this, SchResId( BTN_OK )),
        aPbCancel( this, SchResId( BTN_CANCEL )),
        aPbHelp( this, SchResId( BTN_HELP ))

        //rOutAttrs( rInAttrs )
{
    FreeResource();
    if(!bAxisDlg)
    {
        SetHelpId( HID_INSERT_GRIDS );
        SetText( ObjectNameProvider::getName(OBJECTTYPE_GRID,true) );

        aCbPrimaryX.SetHelpId( HID_SCH_CB_XGRID );
        aCbPrimaryY.SetHelpId( HID_SCH_CB_YGRID );
        aCbPrimaryZ.SetHelpId( HID_SCH_CB_ZGRID );
        aCbSecondaryX.SetHelpId( HID_SCH_CB_SECONDARY_XGRID );
        aCbSecondaryY.SetHelpId( HID_SCH_CB_SECONDARY_YGRID );
        aCbSecondaryZ.SetHelpId( HID_SCH_CB_SECONDARY_ZGRID );

        aFlPrimary.Hide();
        aFlSecondary.Hide();
        aFlPrimaryGrid.Show();
        aFlSecondaryGrid.Show();
    }
    else
    {
        SetText( ObjectNameProvider::getName(OBJECTTYPE_AXIS,true) );

        //todo: remove if secondary z axis are possible somewhere
        {
            aCbSecondaryZ.Hide();

            Size aSize( GetSizePixel() );
            aSize.Height() -= ( aCbSecondaryZ.GetPosPixel().Y() - aCbSecondaryY.GetPosPixel().Y() );
            SetSizePixel(aSize);
        }
    }

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

void SchAxisDlg::getResult( InsertAxisOrGridDialogData& rOutput )
{
    rOutput.aExistenceList[0]=aCbPrimaryX.IsChecked();
    rOutput.aExistenceList[1]=aCbPrimaryY.IsChecked();
    rOutput.aExistenceList[2]=aCbPrimaryZ.IsChecked();
    rOutput.aExistenceList[3]=aCbSecondaryX.IsChecked();
    rOutput.aExistenceList[4]=aCbSecondaryY.IsChecked();
    rOutput.aExistenceList[5]=aCbSecondaryZ.IsChecked();
}

SchGridDlg::SchGridDlg( Window* pParent, const InsertAxisOrGridDialogData& rInput )
                : SchAxisDlg( pParent, rInput, false )//rInAttrs, b3D, bNet, bSecondaryX, bSecondaryY, false )
{
}

SchGridDlg::~SchGridDlg()
{
}

//.............................................................................
} //namespace chart
//.............................................................................



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
