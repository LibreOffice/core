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

#include "lwpfribtable.hxx"
#include "lwppara.hxx"
#include "lwptablelayout.hxx"
#include "lwpchangemgr.hxx"
#include "lwpglobalmgr.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfchange.hxx"

/**
 * @short:   Read table frib
 */
void LwpFribTable::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objTable.ReadIndexed(pObjStrm);
}

LwpSuperTableLayout* LwpFribTable::GetSuperTable()
{
    return dynamic_cast<LwpSuperTableLayout*>(m_objTable.obj().get());
}

void LwpFribTable::RegisterNewStyle()
{
    LwpSuperTableLayout* pSuper = GetSuperTable();
    if (pSuper)
        pSuper->RegisterNewStyle();
    XFParaStyle* pOldStyle = m_pPara->GetXFParaStyle();
    if(HasNextFrib())
    {
        if (pOldStyle->GetMasterPage().isEmpty())
            m_StyleName = pOldStyle->GetStyleName();
        else
        {
            XFParaStyle* pParaStyle = new XFParaStyle;
            *pParaStyle = *(pOldStyle);
            XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
            m_StyleName = pXFStyleManager->AddStyle(pParaStyle).m_pStyle->GetStyleName();
        }
    }
}

void LwpFribTable::XFConvert(XFContentContainer* pCont)
{
    XFContentContainer* pXFContentContainer = pCont;
    LwpSuperTableLayout* pSuper = GetSuperTable();
    if (!pSuper)
        return;
    sal_uInt8 nType = pSuper->GetRelativeType();
    rtl::Reference<LwpVirtualLayout> xContainer(pSuper->GetContainerLayout());
    if (!xContainer.is())
        return;
    if ( LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE == nType
        && !xContainer->IsCell())
    {
        pXFContentContainer = m_pPara->GetXFContainer();
        //delete the additional blank para
        XFParagraph* pCurrPara = m_pPara->GetFribs().GetXFPara();
        if (pXFContentContainer && !pCurrPara->HasContents())
        {
            if(pXFContentContainer->GetLastContent() == pCurrPara)
            {
                pXFContentContainer->RemoveLastContent();
            }
        }
    }
    else if( LwpLayoutRelativityGuts::LAY_PARA_RELATIVE == nType)
    {
        //same page as text and in frame
        if (xContainer->IsFrame())
        {
            pXFContentContainer = m_pPara->GetXFContainer();
        }
        else if (xContainer->IsCell())
        {
            //same page as text and in cell, get the first xfpara
            rtl::Reference<XFContent> first(
                pCont->FindFirstContent(enumXFContentPara));
            XFContentContainer* pXFFirtPara = static_cast<XFContentContainer*>(first.get());
            if(pXFFirtPara)
                pXFContentContainer = pXFFirtPara;
        }
    }
    OUString sChangeID;
    if(m_bRevisionFlag)
    {
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
        sChangeID = pChangeMgr->GetChangeID(this);
        if (!sChangeID.isEmpty() && pXFContentContainer)
        {
            XFChangeStart* pChangeStart = new XFChangeStart;
            pChangeStart->SetChangeID(sChangeID);
            pXFContentContainer->Add(pChangeStart);
        }
    }

    if (pXFContentContainer)
        pSuper->XFConvert(pXFContentContainer);

    if(m_bRevisionFlag)
    {
        if (!sChangeID.isEmpty() && pXFContentContainer)
        {
            XFChangeEnd* pChangeEnd = new XFChangeEnd;
            pChangeEnd->SetChangeID(sChangeID);
            pXFContentContainer->Add(pChangeEnd);
        }
    }

    if(LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE == nType
        && HasNextFrib())
    {
        XFParagraph* pXFPara = new XFParagraph();
        pXFPara->SetStyleName(m_StyleName);
        m_pPara->AddXFContent(pXFPara);
        m_pPara->GetFribs().SetXFPara(pXFPara);
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
