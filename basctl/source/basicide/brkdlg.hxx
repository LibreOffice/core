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

#ifndef BASCTL_BRKDLG_HXX
#define BASCTL_BRKDLG_HXX

#include <svheader.hxx>
#include <bastypes.hxx>
#include <vcl/dialog.hxx>

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

namespace basctl
{

class BreakPointDialog : public ModalDialog
{
private:
    ComboBox        aComboBox;
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    PushButton      aNewButton;
    PushButton      aDelButton;
//  PushButton      aShowButton;
    ::CheckBox      aCheckBox;
    FixedText       aBrkText;
    FixedText       aPassText;
    NumericField    aNumericField;

    BreakPointList & m_rOriginalBreakPointList;
    BreakPointList m_aModifiedBreakPointList;

protected:
    void            CheckButtons();
    DECL_LINK( CheckBoxHdl, ::CheckBox * );
    DECL_LINK( ComboBoxHighlightHdl, ComboBox * );
    DECL_LINK( EditModifyHdl, Edit * );
    DECL_LINK( ButtonHdl, Button * );
    void            UpdateFields( BreakPoint* pBrk );
    BreakPoint*     GetSelectedBreakPoint();


public:
            BreakPointDialog( Window* pParent, BreakPointList& rBrkList );

    void    SetCurrentBreakPoint( BreakPoint* pBrk );
};

} // namespace basctl

#endif // BASCTL_BRKDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
