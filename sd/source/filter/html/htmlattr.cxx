/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
