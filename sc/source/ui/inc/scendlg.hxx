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

#ifndef SC_SCENDLG_HXX
#define SC_SCENDLG_HXX


#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/ctrlbox.hxx>

//===================================================================

class ScNewScenarioDlg : public ModalDialog
{
public:
    ScNewScenarioDlg(Window* pParent, const OUString& rName, bool bEdit = false, bool bSheetProtected = false);

    void SetScenarioData( const OUString& rName, const OUString& rComment,
                          const Color& rColor, sal_uInt16 nFlags );

    void GetScenarioData( OUString& rName, OUString& rComment,
                          Color& rColor, sal_uInt16& rFlags ) const;

private:
    Edit*               m_pEdName;
    VclMultiLineEdit*   m_pEdComment;
    CheckBox*           m_pCbShowFrame;
    ColorListBox*       m_pLbColor;
    CheckBox*           m_pCbTwoWay;
    CheckBox*           m_pCbCopyAll;
    CheckBox*           m_pCbProtect;
    OKButton*           m_pBtnOk;
    const OUString      aDefScenarioName;
    bool                bIsEdit;

    DECL_LINK(OkHdl, void *);
    DECL_LINK( EnableHdl, CheckBox * );
};


#endif // SC_SCENDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
