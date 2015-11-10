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
/*************************************************************************
 * @file
 *  For LWP filter architecture prototype
 ************************************************************************/

#include "lwpfribbreaks.hxx"
#include "lwpstory.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfsectionstyle.hxx"
#include "lwpsection.hxx"
#include "xfilter/xfsection.hxx"
#include "lwpglobalmgr.hxx"

void LwpFribColumnBreak::RegisterBreakStyle(LwpPara * pPara)
{
//    XFParaStyle* pBaseStyle = static_cast<XFParaStyle*>(pFoundry->GetStyleManager()->GetStyle(styleID));
    XFParaStyle* pBaseStyle =  pPara->GetXFParaStyle();
    if (pBaseStyle == nullptr) return;
//    m_StyleName = pBaseStyle->GetStyleName();

    XFParaStyle* pOverStyle = new XFParaStyle;
    *pOverStyle = *pBaseStyle;
    pOverStyle->SetStyleName("");

    //Old code
    //if (static_cast<LwpStory*>(pPara->GetStoryID()->obj())
    //  ->GetCurrentLayout()->GetNumCols() == 1)
    //New code
    LwpStory* pStory = static_cast<LwpStory*>(pPara->GetStoryID().obj().get());
    LwpPageLayout* pCurLayout = pStory ? pStory->GetCurrentLayout() : nullptr;
    if( pCurLayout && (pCurLayout->GetNumCols() == 1) )

    {
//      if (!GetNext() || GetNext()->GetType()==FRIB_TAG_EOP)
            pOverStyle->SetBreaks(enumXFBreakBefPage);
//      else
//          pOverStyle->SetBreaks(enumXFBreakBefPage);
    }
    else
    {
        if (!GetNext() || GetNext()->GetType()==FRIB_TAG_EOP)
            pOverStyle->SetBreaks(enumXFBreakAftColumn);
        else
            pOverStyle->SetBreaks(enumXFBreakBefColumn);
    }
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(pOverStyle).m_pStyle->GetStyleName();
}

LwpFribPageBreak::LwpFribPageBreak( LwpPara* pPara )
    : LwpFrib(pPara), m_bLastFrib(false), m_pMasterPage(nullptr)
{
}

LwpFribPageBreak::~LwpFribPageBreak()
{
    if(m_pMasterPage)
    {
        delete m_pMasterPage;
        m_pMasterPage = nullptr;
    }
}

void LwpFribPageBreak::Read(LwpObjectStream * pObjStrm, sal_uInt16 /*len*/)
{
    m_Layout.ReadIndexed(pObjStrm);
}

void LwpFribPageBreak::RegisterBreakStyle(LwpPara* pPara)
{
    XFParaStyle* pBaseStyle =  pPara->GetXFParaStyle();
    if (pBaseStyle == nullptr) return;

    LwpPageLayout* pLayout = static_cast<LwpPageLayout*>(m_Layout.obj().get());
    if(pLayout)
    {
        m_pMasterPage = new LwpMasterPage(pPara, pLayout);
        m_pMasterPage->RegisterMasterPage(this);
        return;
    }

    XFParaStyle* pOverStyle = new XFParaStyle;
    *pOverStyle = *pBaseStyle;
    pOverStyle->SetStyleName("");
    pOverStyle->SetMasterPage(pBaseStyle->GetMasterPage());

    if (!GetNext() || GetNext()->GetType()==FRIB_TAG_EOP)
        m_bLastFrib = true;
    else
        m_bLastFrib = false;

    if (m_bLastFrib)
        pOverStyle->SetBreaks(enumXFBreakAftPage);
    else
        pOverStyle->SetBreaks(enumXFBreakBefPage);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(pOverStyle).m_pStyle->GetStyleName();
}

void LwpFribPageBreak::ParseLayout()
{
    if(m_pMasterPage)
    {
        m_pMasterPage->ParseSection(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
