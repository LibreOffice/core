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
 *  For LWP filter architecture prototype - footnote layouts
 */
/*************************************************************************
 * Change History
 Mar 2005		 	Created
 ************************************************************************/

#include "lwpfnlayout.hxx"

LwpFootnoteLayout::LwpFootnoteLayout(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpTableLayout(objHdr, pStrm)
{
}

LwpFootnoteLayout::~LwpFootnoteLayout()
{}

/**
 * @descr  Read object info
 */
void LwpFootnoteLayout::Read()
{
    LwpTableLayout::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Do nothing
 */
void LwpFootnoteLayout::RegisterStyle()
{
}

/**
 * @descr  Do nothing
 */
void LwpFootnoteLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

LwpFnRowLayout::LwpFnRowLayout(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpRowLayout(objHdr, pStrm)
{
}

LwpFnRowLayout::~LwpFnRowLayout()
{}

/**
 * @descr  Read object info
 */
void LwpFnRowLayout::Read()
{
    LwpRowLayout::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Register footnote/endnote contents style
 */
void LwpFnRowLayout::RegisterStyle()
{
    // register cells' style
    LwpObjectID *pCellID= GetChildHead();
    LwpCellLayout * pCellLayout = static_cast<LwpCellLayout *>(pCellID->obj());

    while(pCellLayout)
    {
        pCellLayout->SetFoundry(m_pFoundry);
        pCellLayout->RegisterStyle();
        pCellID = pCellLayout->GetNext();
        pCellLayout = static_cast<LwpCellLayout *>(pCellID->obj());
    }
}

/**
 * @descr  Do nothing
 */
void LwpFnRowLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

LwpFnCellLayout::LwpFnCellLayout(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpCellLayout(objHdr, pStrm)
{
}

LwpFnCellLayout::~LwpFnCellLayout()
{}

/**
 * @descr  Read object info
 */
void LwpFnCellLayout::Read()
{
    LwpCellLayout::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Register footnote/endnote contents style
 */
void LwpFnCellLayout::RegisterStyle()
{
    // content object register styles
    LwpObject * pObj = m_Content.obj();
    if (pObj)
    {
        pObj->SetFoundry(m_pFoundry);
        pObj->RegisterStyle();
    }
}

/**
 * @descr  Do nothing
 */
void LwpFnCellLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

LwpEndnoteLayout::LwpEndnoteLayout(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpTableLayout(objHdr, pStrm)
{
}

LwpEndnoteLayout::~LwpEndnoteLayout()
{}

/**
 * @descr  Read object info
 */
void LwpEndnoteLayout::Read()
{
    LwpTableLayout::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Register footnote/endnote contents style
 */
void LwpEndnoteLayout::RegisterStyle()
{
    // register style of rows
    LwpObjectID *pRowID = GetChildHead();
    LwpRowLayout * pRowLayout = static_cast<LwpRowLayout *>(pRowID->obj());
    while (pRowLayout)
    {
        pRowLayout->SetFoundry(m_pFoundry);
        pRowLayout->RegisterStyle();

        pRowID = pRowLayout->GetNext();
        pRowLayout = static_cast<LwpRowLayout *>(pRowID->obj());
    }
}

/**
 * @descr  Do nothing
 */
void LwpEndnoteLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

LwpEnSuperTableLayout::LwpEnSuperTableLayout(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpSuperTableLayout(objHdr, pStrm)
{
}

LwpEnSuperTableLayout::~LwpEnSuperTableLayout()
{}

/**
 * @descr  Read object info
 */
void LwpEnSuperTableLayout::Read()
{
    LwpSuperTableLayout::Read();
    m_pObjStrm->SkipExtra();
}

void LwpEnSuperTableLayout::RegisterStyle()
{
    // if this layout is style of real table entry
    LwpVirtualLayout* pTableLayout = GetMainTableLayout();
    if (pTableLayout != NULL)
    {
        pTableLayout->SetFoundry(m_pFoundry);
        pTableLayout->RegisterStyle();;
    }
}

/**
 * @descr  Do nothing
 */
void LwpEnSuperTableLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

/**
 * @short   Get child endnote layout
 * @return pointer to endnote layout
 */
 LwpVirtualLayout* LwpEnSuperTableLayout::GetMainTableLayout()
{
    LwpObjectID *pID = GetChildTail();

    while(pID && !pID->IsNull())
    {
        LwpVirtualLayout * pLayout = static_cast<LwpVirtualLayout *>(pID->obj());
        if(!pLayout)
        {
            break;
        }
        if (pLayout && pLayout->GetLayoutType() == LWP_ENDNOTE_LAYOUT)
        {
            return pLayout;
        }
        pID = pLayout->GetPrevious();
    }

    return NULL;
}

LwpFnSuperTableLayout::LwpFnSuperTableLayout(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpEnSuperTableLayout(objHdr, pStrm)
{
}

LwpFnSuperTableLayout::~LwpFnSuperTableLayout()
{}

/**
 * @descr  Read object info
 */
void LwpFnSuperTableLayout::Read()
{
    LwpEnSuperTableLayout::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Do nothing
 */
void LwpFnSuperTableLayout::RegisterStyle()
{
}

/**
 * @descr  Do nothing
 */
void LwpFnSuperTableLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

/**
 * @short   Get child footnote layout
 * @return pointer to footnote layout
 */
LwpVirtualLayout* LwpFnSuperTableLayout::GetMainTableLayout()
{
    LwpObjectID *pID = GetChildTail();

    while(pID && !pID->IsNull())
    {
        LwpVirtualLayout * pLayout = static_cast<LwpVirtualLayout *>(pID->obj());
        if(!pLayout)
        {
            break;
        }
        if (pLayout && pLayout->GetLayoutType() == LWP_FOOTNOTE_LAYOUT)
        {
            return pLayout;
        }
        pID = pLayout->GetPrevious();
    }

    return NULL;
}

LwpContFromLayout::LwpContFromLayout(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpPlacableLayout(objHdr, pStrm)
{
}

LwpContFromLayout::~LwpContFromLayout()
{}

/**
 * @descr  Read object info
 */
void LwpContFromLayout::Read()
{
    LwpPlacableLayout::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Do nothing
 */
void LwpContFromLayout::RegisterStyle()
{
}

/**
 * @descr  Do nothing
 */
void LwpContFromLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

LwpContOnLayout::LwpContOnLayout(LwpObjectHeader &objHdr, LwpSvStream *pStrm)
    :LwpPlacableLayout(objHdr, pStrm)
{
}

LwpContOnLayout::~LwpContOnLayout()
{}

/**
 * @descr  Read object info
 */
void LwpContOnLayout::Read()
{
    LwpPlacableLayout::Read();
    m_pObjStrm->SkipExtra();
}

/**
 * @descr  Do nothing
 */
void LwpContOnLayout::RegisterStyle()
{
}

/**
 * @descr  Do nothing
 */
void LwpContOnLayout::XFConvert(XFContentContainer * /*pCont*/)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
