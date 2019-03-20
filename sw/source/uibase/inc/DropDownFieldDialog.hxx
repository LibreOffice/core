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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_DROPDOWNFIELDDIALOG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_DROPDOWNFIELDDIALOG_HXX

#include <vcl/weld.hxx>

class SwDropDownField;
class SwField;
class SwWrtShell;

// Dialog to edit drop down field selection
namespace sw
{
class DropDownFieldDialog : public weld::GenericDialogController
{
    SwWrtShell                 &m_rSh;
    SwDropDownField*           m_pDropField;

    weld::Button* m_pPressedButton;
    std::unique_ptr<weld::TreeView> m_xListItemsLB;
    std::unique_ptr<weld::Button>   m_xOKPB;
    std::unique_ptr<weld::Button>   m_xPrevPB;
    std::unique_ptr<weld::Button>   m_xNextPB;
    std::unique_ptr<weld::Button>   m_xEditPB;

    DECL_LINK(EditHdl, weld::Button&, void);
    DECL_LINK(PrevHdl, weld::Button&, void);
    DECL_LINK(NextHdl, weld::Button&, void);
    void    Apply();
    DECL_LINK(DoubleClickHdl, weld::TreeView&, void);
public:
    DropDownFieldDialog(weld::Widget *pParent, SwWrtShell &rSh,
                        SwField* pField, bool bPrevButton, bool bNextButton);
    virtual ~DropDownFieldDialog() override;
    bool PrevButtonPressed() const;
    bool NextButtonPressed() const;
    virtual short run() override
    {
        short nRet = GenericDialogController::run();
        if (nRet == RET_OK)
            Apply();
        return nRet;
    }
};
} //namespace sw

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
