/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/urlcontrol.hxx>
#include "svl/filenotation.hxx"

//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= OFileURLControl
    //=====================================================================
    //---------------------------------------------------------------------
    OFileURLControl::OFileURLControl(Window* _pParent)
        :SvtURLBox(_pParent, INET_PROT_FILE)
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    OFileURLControl::OFileURLControl(Window* _pParent, const ResId& _rId)
        :SvtURLBox(_pParent, _rId, INET_PROT_FILE)
    {
        DisableHistory();
    }

    //---------------------------------------------------------------------
    long OFileURLControl::PreNotify( NotifyEvent& _rNEvt )
    {
        if (GetSubEdit() == _rNEvt.GetWindow())
            if (EVENT_KEYINPUT == _rNEvt.GetType())
                if (KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode())
                    if (IsInDropDown())
                        m_sPreservedText = GetURL();

        return SvtURLBox::PreNotify(_rNEvt);
    }

    //---------------------------------------------------------------------
    long OFileURLControl::Notify( NotifyEvent& _rNEvt )
    {
        if (GetSubEdit() == _rNEvt.GetWindow())
            if (EVENT_KEYINPUT == _rNEvt.GetType())
                if (KEY_RETURN == _rNEvt.GetKeyEvent()->GetKeyCode().GetCode())
                    if (IsInDropDown())
                    {
                        long nReturn = SvtURLBox::Notify(_rNEvt);

                        // build a system dependent (thus more user readable) file name
                        OFileNotation aTransformer(m_sPreservedText, OFileNotation::N_URL);
                        SetText(aTransformer.get(OFileNotation::N_SYSTEM));
                        Modify();

                        // Update the pick list
                        UpdatePickList();

                        return nReturn;
                    }

        return SvtURLBox::Notify(_rNEvt);
    }

//.........................................................................
}   // namespace svt
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
