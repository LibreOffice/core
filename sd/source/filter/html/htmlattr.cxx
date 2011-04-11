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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "htmlattr.hxx"
#include "htmlex.hxx"
#include <tools/link.hxx>
#include <vcl/decoview.hxx>


// =====================================================================
// =====================================================================
SdHtmlAttrPreview::SdHtmlAttrPreview( Window* pParent, const ResId& rResId )
:Control( pParent, rResId )
{
}

// =====================================================================
// =====================================================================
SdHtmlAttrPreview::~SdHtmlAttrPreview()
{
}

// =====================================================================
// =====================================================================
void SdHtmlAttrPreview::Paint( const Rectangle& rRect )
{
    DecorationView aDecoView( this );

    Rectangle aTextRect;
    aTextRect.SetSize(GetOutputSize());

    SetLineColor(m_aBackColor);
    SetFillColor(m_aBackColor);
    DrawRect(rRect);
    SetFillColor();

    int nHeight = (aTextRect.nBottom - aTextRect.nTop) >> 2;
    aTextRect.nBottom =  nHeight + aTextRect.nTop;

    SetTextColor(m_aTextColor);
    DrawText( aTextRect, String(SdResId(STR_HTMLATTR_TEXT)),
                         TEXT_DRAW_CENTER|TEXT_DRAW_VCENTER );

    aTextRect.Move(0,nHeight);
    SetTextColor(m_aLinkColor);
    DrawText( aTextRect, String(SdResId(STR_HTMLATTR_LINK)),
                         TEXT_DRAW_CENTER|TEXT_DRAW_VCENTER );

    aTextRect.Move(0,nHeight);
    SetTextColor(m_aALinkColor);
    DrawText( aTextRect, String(SdResId(STR_HTMLATTR_ALINK)),
                         TEXT_DRAW_CENTER|TEXT_DRAW_VCENTER );

    aTextRect.Move(0,nHeight);
    SetTextColor(m_aVLinkColor);
    DrawText( aTextRect, String(SdResId(STR_HTMLATTR_VLINK)),
                         TEXT_DRAW_CENTER|TEXT_DRAW_VCENTER );
}

// =====================================================================
// =====================================================================
void SdHtmlAttrPreview::SetColors( Color& aBack, Color& aText, Color& aLink,
                                   Color& aVLink, Color& aALink )
{
    m_aBackColor = aBack;
    m_aTextColor = aText;
    m_aLinkColor = aLink;
    m_aVLinkColor = aVLink;
    m_aALinkColor = aALink;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
