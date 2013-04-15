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


#undef SC_DLLIMPLEMENTATION

#include "namecrea.hxx"
#include "scresid.hxx"

ScNameCreateDlg::ScNameCreateDlg( Window * pParent, sal_uInt16 nFlags )
    : ModalDialog(pParent, "CreateNamesDialog", "modules/scalc/ui/createnamesdialog.ui")
{
    get(m_pTopBox, "top");
    get(m_pLeftBox, "left");
    get(m_pBottomBox, "bottom");
    get(m_pRightBox, "right");
    m_pTopBox->Check   ( (nFlags & NAME_TOP)   ? sal_True : false );
    m_pLeftBox->Check  ( (nFlags & NAME_LEFT)  ? sal_True : false );
    m_pBottomBox->Check( (nFlags & NAME_BOTTOM)? sal_True : false );
    m_pRightBox->Check ( (nFlags & NAME_RIGHT) ? sal_True : false );
}

sal_uInt16 ScNameCreateDlg::GetFlags() const
{
    sal_uInt16  nResult = 0;

    nResult |= m_pTopBox->IsChecked()      ? NAME_TOP:     0 ;
    nResult |= m_pLeftBox->IsChecked()     ? NAME_LEFT:    0 ;
    nResult |= m_pBottomBox->IsChecked()   ? NAME_BOTTOM:  0 ;
    nResult |= m_pRightBox->IsChecked()    ? NAME_RIGHT:   0 ;

    return nResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
