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

#include "curledit.hxx"
#include <vcl/svapp.hxx>
#include <osl/diagnose.h>

namespace dbaui
{
    DBG_NAME(OConnectionURLEdit)
// OConnectionURLEdit
OConnectionURLEdit::OConnectionURLEdit(Window* _pParent, const ResId& _rResId,sal_Bool _bShowPrefix)
    :Edit(_pParent, _rResId)
    ,m_pTypeCollection(NULL)
    ,m_pForcedPrefix(NULL)
    ,m_bShowPrefix(_bShowPrefix)
{
    DBG_CTOR(OConnectionURLEdit ,NULL);
}

OConnectionURLEdit::~OConnectionURLEdit()
{
    DBG_DTOR(OConnectionURLEdit ,NULL);
    // delete my sub controls
    Edit* pSubEdit = GetSubEdit();
    SetSubEdit(NULL);
    delete pSubEdit;
    delete m_pForcedPrefix;
}

void OConnectionURLEdit::SetTextNoPrefix(const String& _rText)
{
    OSL_ENSURE(GetSubEdit(), "OConnectionURLEdit::SetTextNoPrefix: have no current type, not changing the text!");
    if (GetSubEdit())
        GetSubEdit()->SetText(_rText);
}

String OConnectionURLEdit::GetTextNoPrefix() const
{
    if (GetSubEdit())
        return GetSubEdit()->GetText();
    return GetText();
}

void OConnectionURLEdit::SetText(const OUString& _rStr)
{
    Selection aNoSelection(0,0);
    SetText(_rStr, aNoSelection);
}

void OConnectionURLEdit::SetText(const OUString& _rStr, const Selection& /*_rNewSelection*/)
{
    // create new sub controls, if necessary
    if (!GetSubEdit())
        SetSubEdit(new Edit(this, 0));
    if ( !m_pForcedPrefix )
    {
        m_pForcedPrefix = new FixedText(this, WB_VCENTER);

        // we use a gray background for the fixed text
        StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
        m_pForcedPrefix->SetBackground(Wallpaper(aSystemStyle.GetDialogColor()));
    }

    m_pForcedPrefix->Show(m_bShowPrefix);

    sal_Bool bIsEmpty = _rStr.isEmpty();
    // calc the prefix
    String sPrefix;
    if (!bIsEmpty)
    {
        // determine the type of the new URL described by the new text
        sPrefix = m_pTypeCollection->getPrefix(_rStr);
    }

    // the fixed text gets the prefix
    if ( m_pForcedPrefix )
        m_pForcedPrefix->SetText(sPrefix);

    // both subs have to be resized according to the text len of the prefix
    Size aMySize = GetSizePixel();
    sal_Int32 nTextWidth = 0;
    if ( m_pForcedPrefix && m_bShowPrefix)
    {
        nTextWidth = m_pForcedPrefix->GetTextWidth(sPrefix) + 2;
        m_pForcedPrefix->SetPosSizePixel(Point(0, -2), Size(nTextWidth, aMySize.Height()));
    }
    GetSubEdit()->SetPosSizePixel(Point(nTextWidth, -2), Size(aMySize.Width() - nTextWidth - 4, aMySize.Height()));
        // -2 because the edit has a frame which is 2 pixel wide ... should not be necessary, but I don't fully understand this ....

    // show the sub controls (in case they were just created)
    GetSubEdit()->Show();

    // do the real SetTex
//  Edit::SetText(bIsEmpty ? _rStr : m_pTypeCollection->cutPrefix(_rStr), _rNewSelection);
    String sNewText( _rStr );
    if ( !bIsEmpty )
        sNewText  = m_pTypeCollection->cutPrefix( _rStr );
    Edit::SetText( sNewText );
}

OUString OConnectionURLEdit::GetText() const
{
    if ( m_pForcedPrefix )
        return m_pForcedPrefix->GetText() += Edit::GetText();
    return Edit::GetText();
}

void OConnectionURLEdit::ShowPrefix(sal_Bool _bShowPrefix)
{
    m_bShowPrefix = _bShowPrefix;
    if ( m_pForcedPrefix )
        m_pForcedPrefix->Show(m_bShowPrefix);
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
