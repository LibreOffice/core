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

#include "ResId.hxx"
#include "chartview/ChartSfxItemIds.hxx"
#include "HelpIds.hrc"
#include "NoWarningThisInCTOR.hxx"
#include "ObjectNameProvider.hxx"

// header for class SfxBoolItem
#include <svl/eitem.hxx>

namespace chart
{

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

//
// SchAxisDlg
//

SchAxisDlg::SchAxisDlg(Window* pWindow,
    const InsertAxisOrGridDialogData& rInput, bool bAxisDlg)
    : ModalDialog(pWindow,
          bAxisDlg ?
              OString("InsertAxisDialog") :
              OString("InsertGridDialog"),
          bAxisDlg ?
              OUString("modules/schart/ui/insertaxisdlg.ui") :
              OUString("modules/schart/ui/insertgriddlg.ui"))
{
    get(m_pCbPrimaryX, "primaryX");
    get(m_pCbPrimaryY, "primaryY");
    get(m_pCbPrimaryZ, "primaryZ");
    get(m_pCbSecondaryX, "secondaryX");
    get(m_pCbSecondaryY, "secondaryY");
    get(m_pCbSecondaryZ, "secondaryZ");

    if (bAxisDlg)
    {

        //todo: remove if secondary z axis are possible somewhere
        m_pCbSecondaryZ->Hide();
    }

    m_pCbPrimaryX->Check( rInput.aExistenceList[0] );
    m_pCbPrimaryY->Check( rInput.aExistenceList[1] );
    m_pCbPrimaryZ->Check( rInput.aExistenceList[2] );
    m_pCbSecondaryX->Check( rInput.aExistenceList[3] );
    m_pCbSecondaryY->Check( rInput.aExistenceList[4] );
    m_pCbSecondaryZ->Check( rInput.aExistenceList[5] );

    m_pCbPrimaryX->Enable( rInput.aPossibilityList[0] );
    m_pCbPrimaryY->Enable( rInput.aPossibilityList[1] );
    m_pCbPrimaryZ->Enable( rInput.aPossibilityList[2] );
    m_pCbSecondaryX->Enable( rInput.aPossibilityList[3] );
    m_pCbSecondaryY->Enable( rInput.aPossibilityList[4] );
    m_pCbSecondaryZ->Enable( rInput.aPossibilityList[5] );
}

SchAxisDlg::~SchAxisDlg()
{
}

void SchAxisDlg::getResult( InsertAxisOrGridDialogData& rOutput )
{
    rOutput.aExistenceList[0]=m_pCbPrimaryX->IsChecked();
    rOutput.aExistenceList[1]=m_pCbPrimaryY->IsChecked();
    rOutput.aExistenceList[2]=m_pCbPrimaryZ->IsChecked();
    rOutput.aExistenceList[3]=m_pCbSecondaryX->IsChecked();
    rOutput.aExistenceList[4]=m_pCbSecondaryY->IsChecked();
    rOutput.aExistenceList[5]=m_pCbSecondaryZ->IsChecked();
}

SchGridDlg::SchGridDlg( Window* pParent, const InsertAxisOrGridDialogData& rInput )
                : SchAxisDlg( pParent, rInput, false )//rInAttrs, b3D, bNet, bSecondaryX, bSecondaryY, false )
{
}

SchGridDlg::~SchGridDlg()
{
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
