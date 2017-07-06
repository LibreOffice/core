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
 *  For LWP filter architecture prototype - footnote
 */

#include "lwpfribframe.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "lwpglobalmgr.hxx"
#include "xfilter/xfchange.hxx"
/**
 * @short:   Read frame frib
 */
void LwpFribFrame::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objLayout.ReadIndexed(pObjStrm);
}

/**
*  @descr:  Get the layout object which the frib points to
*
*/
rtl::Reference<LwpObject> LwpFribFrame::GetLayout()
{
    return m_objLayout.obj();
}

/**
*  @descr:  register frame style
*  @param:   pFoundry - current foundry pointer
*
*/
void LwpFribFrame::RegisterStyle(LwpFoundry* pFoundry)
{
    rtl::Reference<LwpObject> pObject = m_objLayout.obj();

    if (pObject.is() && pObject->GetTag() == VO_DROPCAPLAYOUT)
    {
        LwpDropcapLayout *pLayout = dynamic_cast<LwpDropcapLayout*>(pObject.get());
        if (!pLayout)
            return;
        pLayout->RegisterStyle(pFoundry);
    }
    else
    {
        //register frame style
        LwpPlacableLayout* pLayout = dynamic_cast<LwpPlacableLayout*>(pObject.get());
        if (!pLayout)
            return;
        pLayout->SetFoundry(pFoundry);
        pLayout->DoRegisterStyle();

        //register next frib text style
        sal_uInt8 nType = pLayout->GetRelativeType();
        if(LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE == nType
            && HasNextFrib())
        {
            XFParaStyle* pOldStyle = m_pPara->GetXFParaStyle();
            if (pOldStyle->GetMasterPage().isEmpty())
                m_StyleName = pOldStyle->GetStyleName();
            else
            {
                XFParaStyle* pParaStyle = new XFParaStyle;
                *pParaStyle = *pOldStyle;
                XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
                m_StyleName = pXFStyleManager->AddStyle(pParaStyle).m_pStyle->GetStyleName();
            }
        }
        //remember the current paragraph font size which will be used in parsing frame
        pLayout->SetFont(GetFont());
    }
}
void LwpFribFrame::SetParaDropcap(LwpPara* pPara)
{
    rtl::Reference<LwpObject> pObject = m_objLayout.obj();

    if (pObject.is() && pObject->GetTag() == VO_DROPCAPLAYOUT)
    {
        pPara->SetParaDropcap(true);
        pPara->SetDropcapLayout(dynamic_cast<LwpDropcapLayout*>(pObject.get()));
    }
    else
        pPara->SetParaDropcap(false);
}

/**
*  @descr:  convert frame
*
*/
void LwpFribFrame::XFConvert(XFContentContainer* pCont)
{
    XFContentContainer* pXFContentContainer = pCont;
    LwpVirtualLayout* pLayout = dynamic_cast<LwpVirtualLayout*>(GetLayout().get());
    if (!pLayout)
        return;
    sal_uInt8 nType = pLayout->GetRelativeType();
    if( LwpLayoutRelativityGuts::LAY_PARA_RELATIVE == nType)
    {
        rtl::Reference<LwpVirtualLayout> xContainerLayout(pLayout->GetContainerLayout());
        if (xContainerLayout.is() && xContainerLayout->IsFrame())
        {
            //same page as text and in frame
            pXFContentContainer = m_pPara->GetXFContainer();
        }
        else if (xContainerLayout.is() && xContainerLayout->IsCell())
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
        if (!sChangeID.isEmpty())
        {
            XFChangeStart* pChangeStart = new XFChangeStart;
            pChangeStart->SetChangeID(sChangeID);
            pXFContentContainer->Add(pChangeStart);
        }
    }

    pLayout->DoXFConvert(pXFContentContainer);

    if(m_bRevisionFlag)
    {
        if (!sChangeID.isEmpty())
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

/**
 *  @descr:  Read Ruby frame
 *
 */
void LwpFribRubyFrame::Read(LwpObjectStream* pObjStrm, sal_uInt16 /*len*/)
{
    m_objLayout.ReadIndexed(pObjStrm);
}
/**
 *  @descr:  Register Ruby frame style
 *
 */
void LwpFribRubyFrame::RegisterStyle(LwpFoundry* pFoundry)
{
    LwpRubyLayout* pLayout = GetLayout();
    if (pLayout)
    {
        pLayout->SetFoundry(pFoundry);
        pLayout->RegisterStyle();
    }
}
/**
 *  @descr:  convert Ruby frame
 *
 */
void LwpFribRubyFrame::XFConvert()
{
    LwpRubyLayout* pLayout = GetLayout();
    if (pLayout)
    {
        pLayout->ConvertContentText();
    }
}

LwpRubyLayout* LwpFribRubyFrame::GetLayout()
{
    return dynamic_cast<LwpRubyLayout*>(m_objLayout.obj().get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
