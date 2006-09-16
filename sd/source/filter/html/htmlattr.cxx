/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: htmlattr.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 18:22:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "htmlattr.hxx"

#ifndef _SD_HTMLEX_HXX
#include "htmlex.hxx"
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX //autogen
#include <vcl/decoview.hxx>
#endif


// =====================================================================
// =====================================================================
SdHtmlAttrPreview::SdHtmlAttrPreview( Window* pParent, const ResId& rResId )
:Control( pParent, rResId )
{
//    SetMapMode( MAP_100TH_MM );
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

// =====================================================================
// =====================================================================
void SdHtmlAttrPreview::GetColors( Color& aBack, Color& aText, Color& aLink,
                                   Color& aVLink, Color& aALink ) const
{
    aBack = m_aBackColor;
    aText = m_aTextColor;
    aLink = m_aLinkColor;
    aVLink = m_aVLinkColor;
    aALink = m_aALinkColor;
}
