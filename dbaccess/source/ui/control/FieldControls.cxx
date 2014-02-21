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

#include "FieldControls.hxx"
#include "SqlNameEdit.hxx"

#include <vcl/settings.hxx>

namespace dbaui {

namespace {

void lcl_setSpecialReadOnly( sal_Bool _bReadOnly, Window* _pWin )
{
    StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
    const Color& rNewColor = _bReadOnly ? aSystemStyle.GetDialogColor() : aSystemStyle.GetFieldColor();
    _pWin->SetBackground(Wallpaper(rNewColor));
    _pWin->SetControlBackground(rNewColor);
}

}

OPropColumnEditCtrl::OPropColumnEditCtrl(Window* pParent,
                                                rtl::OUString& _rAllowedChars,
                                                sal_uInt16 nHelpId,
                                                short nPosition,
                                                WinBits nWinStyle)
    :OSQLNameEdit(pParent, nWinStyle, _rAllowedChars)
    ,m_nPos(nPosition)
{
    m_strHelpText = ModuleRes(nHelpId);
}

void
OPropColumnEditCtrl::SetSpecialReadOnly(sal_Bool _bReadOnly)
{
    SetReadOnly(_bReadOnly);
    lcl_setSpecialReadOnly(_bReadOnly,this);
}



OPropEditCtrl::OPropEditCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition, WinBits nWinStyle)
    :Edit(pParent, nWinStyle)
    ,m_nPos(nPosition)
{
    m_strHelpText = ModuleRes(nHelpId);
}

void
OPropEditCtrl::SetSpecialReadOnly(sal_Bool _bReadOnly)
{
    SetReadOnly(_bReadOnly);
    lcl_setSpecialReadOnly(_bReadOnly,this);
}

void
OPropNumericEditCtrl::SetSpecialReadOnly(sal_Bool _bReadOnly)
{
    SetReadOnly(_bReadOnly);
    lcl_setSpecialReadOnly(_bReadOnly,this);
}


OPropNumericEditCtrl::OPropNumericEditCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition, WinBits nWinStyle)
    :NumericField(pParent, nWinStyle)
    ,m_nPos(nPosition)
{
    m_strHelpText = ModuleRes(nHelpId);
}

OPropListBoxCtrl::OPropListBoxCtrl(Window* pParent, sal_uInt16 nHelpId, short nPosition, WinBits nWinStyle)
    :ListBox(pParent, nWinStyle)
    ,m_nPos(nPosition)
{
    m_strHelpText = ModuleRes(nHelpId);
}

void
OPropListBoxCtrl::SetSpecialReadOnly(sal_Bool _bReadOnly)
{
    SetReadOnly(_bReadOnly);
    lcl_setSpecialReadOnly(_bReadOnly,this);
}

} // end namespace dbaui
