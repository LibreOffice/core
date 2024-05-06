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

#include <dlg_InsertAxis_Grid.hxx>

namespace chart
{

InsertAxisOrGridDialogData::InsertAxisOrGridDialogData()
        : aPossibilityList{ true, true, true, true, true, true }
        , aExistenceList{ false, false, false, false, false, false }
{
}

// SchAxisDlg

SchAxisDlg::SchAxisDlg(weld::Window* pWindow,
    const InsertAxisOrGridDialogData& rInput, bool bAxisDlg)
    : GenericDialogController(pWindow,
          bAxisDlg ?
              u"modules/schart/ui/insertaxisdlg.ui"_ustr :
              u"modules/schart/ui/insertgriddlg.ui"_ustr,
          bAxisDlg ?
              u"InsertAxisDialog"_ustr :
              u"InsertGridDialog"_ustr)
    , m_xCbPrimaryX(m_xBuilder->weld_check_button(u"primaryX"_ustr))
    , m_xCbPrimaryY(m_xBuilder->weld_check_button(u"primaryY"_ustr))
    , m_xCbPrimaryZ(m_xBuilder->weld_check_button(u"primaryZ"_ustr))
    , m_xCbSecondaryX(m_xBuilder->weld_check_button(u"secondaryX"_ustr))
    , m_xCbSecondaryY(m_xBuilder->weld_check_button(u"secondaryY"_ustr))
    , m_xCbSecondaryZ(m_xBuilder->weld_check_button(u"secondaryZ"_ustr))
{
    if (bAxisDlg)
    {
        //todo: remove if secondary z axis are possible somewhere
        m_xCbSecondaryZ->hide();
    }

    m_xCbPrimaryX->set_active( rInput.aExistenceList[0] );
    m_xCbPrimaryY->set_active( rInput.aExistenceList[1] );
    m_xCbPrimaryZ->set_active( rInput.aExistenceList[2] );
    m_xCbSecondaryX->set_active( rInput.aExistenceList[3] );
    m_xCbSecondaryY->set_active( rInput.aExistenceList[4] );
    m_xCbSecondaryZ->set_active( rInput.aExistenceList[5] );

    m_xCbPrimaryX->set_sensitive( rInput.aPossibilityList[0] );
    m_xCbPrimaryY->set_sensitive( rInput.aPossibilityList[1] );
    m_xCbPrimaryZ->set_sensitive( rInput.aPossibilityList[2] );
    m_xCbSecondaryX->set_sensitive( rInput.aPossibilityList[3] );
    m_xCbSecondaryY->set_sensitive( rInput.aPossibilityList[4] );
    m_xCbSecondaryZ->set_sensitive( rInput.aPossibilityList[5] );
}

void SchAxisDlg::getResult( InsertAxisOrGridDialogData& rOutput )
{
    sal_Bool* pExistenceList = rOutput.aExistenceList.getArray();
    pExistenceList[0]=m_xCbPrimaryX->get_active();
    pExistenceList[1]=m_xCbPrimaryY->get_active();
    pExistenceList[2]=m_xCbPrimaryZ->get_active();
    pExistenceList[3]=m_xCbSecondaryX->get_active();
    pExistenceList[4]=m_xCbSecondaryY->get_active();
    pExistenceList[5]=m_xCbSecondaryZ->get_active();
}

SchGridDlg::SchGridDlg(weld::Window* pParent, const InsertAxisOrGridDialogData& rInput)
    : SchAxisDlg(pParent, rInput, false) //rInAttrs, b3D, bNet, bSecondaryX, bSecondaryY, false )
{
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
