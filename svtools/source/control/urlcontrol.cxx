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

#include <svl/filenotation.hxx>
#include <svtools/urlcontrol.hxx>
#include <vcl/layout.hxx>
#include <vcl/builderfactory.hxx>

namespace svt
{
    //= OFileURLControl
    OFileURLControl::OFileURLControl(vcl::Window* _pParent, WinBits nStyle)
        : SvtURLBox(_pParent, nStyle, INetProtocol::File)
    {
        DisableHistory();
    }

    VCL_BUILDER_DECL_FACTORY(OFileURLControl)
    {
        (void)rMap;
        WinBits nWinBits = WB_LEFT|WB_VCENTER|WB_3DLOOK|WB_TABSTOP|
                           WB_DROPDOWN|WB_AUTOSIZE|WB_AUTOHSCROLL;
        VclPtrInstance<OFileURLControl> pListBox(pParent, nWinBits);
        pListBox->EnableAutoSize(true);
        rRet = pListBox;
    }

    bool OFileURLControl::PreNotify( NotifyEvent& _rNEvt )
    {
        if (GetSubEdit() == _rNEvt.GetWindow())
            if (MouseNotifyEvent::KEYINPUT == _rNEvt.GetType())
                if (KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode())
                    if (IsInDropDown())
                        m_sPreservedText = GetURL();

        return SvtURLBox::PreNotify(_rNEvt);
    }

    bool OFileURLControl::Notify( NotifyEvent& _rNEvt )
    {
        if (GetSubEdit() == _rNEvt.GetWindow())
            if (MouseNotifyEvent::KEYINPUT == _rNEvt.GetType())
                if (KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode())
                    if (IsInDropDown())
                    {
                        bool bReturn = SvtURLBox::Notify(_rNEvt);

                        // build a system dependent (thus more user readable) file name
                        OFileNotation aTransformer(m_sPreservedText, OFileNotation::N_URL);
                        SetText(aTransformer.get(OFileNotation::N_SYSTEM));
                        Modify();

                        // Update the pick list
                        UpdatePickList();

                        return bReturn;
                    }

        return SvtURLBox::Notify(_rNEvt);
    }
}   // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
