/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "curledit.hxx"
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/diagnose.h>

namespace dbaui
{

OConnectionURLEdit::OConnectionURLEdit(Window* _pParent, const ResId& _rResId,sal_Bool _bShowPrefix)
    :Edit(_pParent, _rResId)
    ,m_pTypeCollection(NULL)
    ,m_pForcedPrefix(NULL)
    ,m_bShowPrefix(_bShowPrefix)
{
}

OConnectionURLEdit::~OConnectionURLEdit()
{
    
    Edit* pSubEdit = GetSubEdit();
    SetSubEdit(NULL);
    delete pSubEdit;
    delete m_pForcedPrefix;
}

void OConnectionURLEdit::SetTextNoPrefix(const OUString& _rText)
{
    OSL_ENSURE(GetSubEdit(), "OConnectionURLEdit::SetTextNoPrefix: have no current type, not changing the text!");
    if (GetSubEdit())
        GetSubEdit()->SetText(_rText);
}

OUString OConnectionURLEdit::GetTextNoPrefix() const
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
    
    if (!GetSubEdit())
        SetSubEdit(new Edit(this, 0));
    if ( !m_pForcedPrefix )
    {
        m_pForcedPrefix = new FixedText(this, WB_VCENTER);

        
        StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
        m_pForcedPrefix->SetBackground(Wallpaper(aSystemStyle.GetDialogColor()));
    }

    m_pForcedPrefix->Show(m_bShowPrefix);

    sal_Bool bIsEmpty = _rStr.isEmpty();
    
    OUString sPrefix;
    if (!bIsEmpty)
    {
        
        sPrefix = m_pTypeCollection->getPrefix(_rStr);
    }

    
    m_pForcedPrefix->SetText(sPrefix);

    
    Size aMySize = GetSizePixel();
    sal_Int32 nTextWidth = 0;
    if ( m_pForcedPrefix && m_bShowPrefix)
    {
        nTextWidth = m_pForcedPrefix->GetTextWidth(sPrefix) + 2;
        m_pForcedPrefix->SetPosSizePixel(Point(0, -2), Size(nTextWidth, aMySize.Height()));
    }
    GetSubEdit()->SetPosSizePixel(Point(nTextWidth, -2), Size(aMySize.Width() - nTextWidth - 4, aMySize.Height()));
        

    
    GetSubEdit()->Show();

    

    OUString sNewText( _rStr );
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

}   

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
