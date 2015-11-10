/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/**
 * @file
 *  For LWP filter architecture prototype - notes
 */

#include "lwpnotes.hxx"
#include "lwppara.hxx"
#include "xfilter/xfannotation.hxx"
#include "xfilter/xftextspan.hxx"
#include "localtime.hxx"
#include "lwptools.hxx"

 LwpFribNote::LwpFribNote(LwpPara* pPara ):LwpFrib(pPara)
{
}

/**
 * @descr  read  frib information
 */
void LwpFribNote::Read(LwpObjectStream *pObjStrm, sal_uInt16 /*len*/)
{
    m_Layout.ReadIndexed(pObjStrm);
}

/**
 * @descr  Register style
 */
void LwpFribNote::RegisterNewStyle()
{
    rtl::Reference<LwpObject> pLayout = m_Layout.obj();
    if(pLayout.is())
    {
        //register font style
        LwpFrib::RegisterStyle(m_pPara->GetFoundry());
        //register foonote style
        pLayout->SetFoundry(m_pPara->GetFoundry());
        pLayout->RegisterStyle();
    }
}

/**
 * @descr  convert note
 */
void LwpFribNote::XFConvert(XFContentContainer* pCont)
{
    LwpNoteLayout* pLayout =static_cast<LwpNoteLayout*>(m_Layout.obj().get());
    if(pLayout)
    {
        XFAnnotation* pXFNote = new XFAnnotation;
        pXFNote->SetAuthor(pLayout->GetAuthor());
        LtTm aTm;
        long nTime = pLayout->GetTime();
        if(LtgLocalTime(nTime, aTm))
        {
            pXFNote->SetDate(LwpTools::DateTimeToOUString(aTm));
        }

        pLayout->XFConvert(pXFNote);
        if(m_pModifiers)
        {
            XFTextSpan *pSpan = new XFTextSpan();
            pSpan->SetStyleName(GetStyleName());
            pSpan->Add(pXFNote);
            pCont->Add(pSpan);
        }
        else
        {
            pCont->Add(pXFNote);
        }
    }

}

LwpNoteLayout::LwpNoteLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpFrameLayout(objHdr, pStrm)
    , m_nTime(0)
{
}

LwpNoteLayout::~LwpNoteLayout()
{

}

/**
 * @descr read note layout object
 *
 */
void LwpNoteLayout::Read()
{
    LwpFrameLayout::Read();

    m_nTime = m_pObjStrm->QuickReaduInt32();
    m_UserName.Read(m_pObjStrm);

    LwpAtomHolder aUserInitials;
    aUserInitials.Read(m_pObjStrm);

    LwpColor aColor;
    aColor.Read(m_pObjStrm);

    // vacant note sequence
    m_pObjStrm->QuickReadInt32();

    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Register style
 */
void LwpNoteLayout::RegisterStyle()
{
    LwpVirtualLayout* pTextLayout = GetTextLayout();
    if(pTextLayout)
    {
        pTextLayout->SetFoundry(GetFoundry());
        pTextLayout->RegisterStyle();
    }
}

/**
 * @descr  convert note
 */
void LwpNoteLayout::XFConvert(XFContentContainer * pCont)
{
    LwpVirtualLayout* pTextLayout = GetTextLayout();
    if(pTextLayout)
    {
        pTextLayout->XFConvert(pCont);
    }
}

/**
 * @descr  Get layout that contains note text.
 */
LwpVirtualLayout* LwpNoteLayout::GetTextLayout()
{
    LwpVirtualLayout* pLayout = FindChildByType(LWP_VIEWPORT_LAYOUT);
    if(pLayout)
    {
        return pLayout->FindChildByType(LWP_NOTETEXT_LAYOUT);
    }

    return nullptr;
}
/**
 * @descr  Get author.
 */
OUString LwpNoteLayout::GetAuthor()
{
    if(m_UserName.HasValue())
    {
        if(m_UserName.str() != " ")
        {
            return m_UserName.str();
        }
    }
    //if username is null or writerspace, get username from noteheaderlayout
    LwpNoteHeaderLayout* pTextLayout = static_cast<LwpNoteHeaderLayout*>(FindChildByType(LWP_NOTEHEADER_LAYOUT));
    if(pTextLayout)
    {
        LwpStory* pStory = static_cast<LwpStory*>(pTextLayout->GetContent().obj().get());
        if(pStory)
        {
            LwpPara* pFirst = static_cast<LwpPara*>(pStory->GetFirstPara().obj().get());
            if(pFirst)
                return pFirst->GetContentText(true);
        }
    }

    return m_UserName.str();
}

LwpNoteHeaderLayout::LwpNoteHeaderLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpFrameLayout(objHdr, pStrm)
{
}

LwpNoteHeaderLayout::~LwpNoteHeaderLayout()
{
}

/**
 * @descr read note layout object
 *
 */
void LwpNoteHeaderLayout::Read()
{
    LwpFrameLayout::Read();
    m_pObjStrm->SkipExtra();
}

void LwpNoteHeaderLayout::RegisterStyle()
{
}

void LwpNoteHeaderLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

LwpNoteTextLayout::LwpNoteTextLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpFrameLayout(objHdr, pStrm)
{
}

LwpNoteTextLayout::~LwpNoteTextLayout()
{
}

/**
 * @descr read note layout object
 *
 */
void LwpNoteTextLayout::Read()
{
    LwpFrameLayout::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Register style, SODC doesn't support text style in note
 */
void LwpNoteTextLayout::RegisterStyle()
{
    rtl::Reference<LwpObject> pContent = m_Content.obj();
    if(pContent.is())
    {
        pContent->SetFoundry(GetFoundry());
        pContent->RegisterStyle();
    }
}

void LwpNoteTextLayout::XFConvert(XFContentContainer * pCont)
{
    rtl::Reference<LwpObject> pContent = m_Content.obj();
    if(pContent.is())
    {
        pContent->XFConvert(pCont);
    }
}

LwpViewportLayout::LwpViewportLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpPlacableLayout(objHdr, pStrm)
{
}

LwpViewportLayout::~LwpViewportLayout()
{
}

/**
 * @descr read note layout object
 *
 */
void LwpViewportLayout::Read()
{
    LwpPlacableLayout::Read();
    m_pObjStrm->SkipExtra();
}

void LwpViewportLayout::RegisterStyle()
{
}

void LwpViewportLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
