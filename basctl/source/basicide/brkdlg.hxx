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

#ifndef INCLUDED_BASCTL_SOURCE_BASICIDE_BRKDLG_HXX
#define INCLUDED_BASCTL_SOURCE_BASICIDE_BRKDLG_HXX

#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/field.hxx>

namespace basctl
{

class BreakPointDialog : public ModalDialog
{
private:
    VclPtr<ComboBox>       m_pComboBox;
    VclPtr<OKButton>       m_pOKButton;
    VclPtr<PushButton>     m_pNewButton;
    VclPtr<PushButton>     m_pDelButton;
    VclPtr<CheckBox>       m_pCheckBox;
    VclPtr<NumericField>   m_pNumericField;

    BreakPointList & m_rOriginalBreakPointList;
    BreakPointList m_aModifiedBreakPointList;

protected:
    void            CheckButtons();
    DECL_LINK_TYPED( CheckBoxHdl, Button*, void );
    DECL_LINK_TYPED( ComboBoxHighlightHdl, ComboBox&, void );
    DECL_LINK_TYPED( EditModifyHdl, Edit&, void );
    DECL_LINK_TYPED( ButtonHdl, Button*, void );
    void            UpdateFields( BreakPoint* pBrk );
    BreakPoint*     GetSelectedBreakPoint();


public:
            BreakPointDialog( vcl::Window* pParent, BreakPointList& rBrkList );
    virtual ~BreakPointDialog();
    virtual void dispose() override;

    void    SetCurrentBreakPoint( BreakPoint* pBrk );
};

} // namespace basctl

#endif // INCLUDED_BASCTL_SOURCE_BASICIDE_BRKDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
