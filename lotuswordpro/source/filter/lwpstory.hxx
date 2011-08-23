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
 *  LwpStory:
        1. Word Pro object for paragraph list;
        2. the content of layout object
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/

#ifndef _LWPSTORY_HXX
#define _LWPSTORY_HXX
#include "lwpcontent.hxx"

class LwpPageLayout;
class LwpStory: public LwpContent
{
public:
    LwpStory(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpStory();
private:
    //File format members:
    LwpDLVListHeadTail m_ParaList;
    LwpObjectID m_FirstParaStyle;

    // for bullet , 05/23/2005
    typedef std::pair<rtl::OUString, sal_uInt8> NamePosPair;
    std::vector <NamePosPair> m_vBulletStyleNameList;

    // , 02/16/2005
    sal_Bool m_bPMModified;
    LwpPageLayout* m_pCurrentLayout;	//for register page layout style
    std::vector<LwpPageLayout*> m_LayoutList;
    LwpLayout* m_pTabLayout;	//for register tab style

    sal_Bool m_bDropcap;
    LwpHyperlinkMgr* m_pHyperlinkMgr;
    OUString m_CurrSectionName;

    XFContentContainer* m_pXFContainer;

protected:
    void Read();
    void XFConvertFrame(XFContentContainer* pCont);
    void XFConvertFrameInCell(XFContentContainer* pCont);
    void XFConvertFrameInFrame(XFContentContainer* pCont);
    void XFConvertFrameInHeaderFooter(XFContentContainer* pCont);

public:
    void RegisterStyle();
    void Parse(IXFStream* pOutputStream);
    virtual void XFConvert(XFContentContainer* pCont);
    void XFConvertFrameInPage(XFContentContainer* pCont);
    inline LwpDLVListHeadTail* GetParaList();
    inline LwpObjectID* GetFirstPara();
    inline LwpObjectID* GetLastPara();
     XFContentContainer* GetXFContent();
    void AddXFContent(XFContent* pContent);

    inline sal_Bool GetDropcapFlag();
    inline LwpPageLayout* GetCurrentLayout();
    inline LwpMiddleLayout* GetTabLayout();//for register tab style
    inline OUString GetSectionName();
    inline LwpHyperlinkMgr* GetHyperlinkMgr();

    inline sal_Bool IsPMModified();
    inline void SetPMModified(sal_Bool bPMModified);
    inline void SetDropcapFlag(sal_Bool bFlag);
    inline void SetTabLayout(LwpLayout* pLayout);
    inline void SetSectionName(OUString StyleName);

    void SetCurrentLayout(LwpPageLayout* pPageLayout);
    void AddPageLayout(LwpPageLayout* pObject);
    LwpPageLayout* GetNextPageLayout();
    sal_Bool IsNeedSection();
    void SortPageLayout();

    LwpPara* GetLastParaOfPreviousStory();

    OUString GetContentText(sal_Bool bAllText = sal_False);//add by ,for CHB,05/5/25
    inline void AddBullStyleName2List(const rtl::OUString& rStyleName, const sal_uInt8& nPos);
    sal_Bool IsBullStyleUsedBefore(const rtl::OUString& rStyleName, const sal_uInt8& nPos);

    OUString RegisterFirstFribStyle();
};

LwpDLVListHeadTail* LwpStory::GetParaList()
{
    return &m_ParaList;
}
LwpObjectID* LwpStory::GetFirstPara()
{
    return m_ParaList.GetHead();
}
LwpObjectID* LwpStory::GetLastPara()
{
    return m_ParaList.GetTail();
}
sal_Bool LwpStory::GetDropcapFlag()
{
    return m_bDropcap;
}
LwpMiddleLayout* LwpStory::GetTabLayout()
{
    if(m_pTabLayout)
        return m_pTabLayout;
    return static_cast<LwpMiddleLayout*>(GetLayout(NULL));
}
void LwpStory::SetPMModified(sal_Bool bPMModified)
{
    m_bPMModified = bPMModified;
}
void LwpStory::SetDropcapFlag(sal_Bool bFlag)
{
    m_bDropcap = bFlag;
}
void LwpStory::SetTabLayout(LwpLayout* pLayout)
{
    m_pTabLayout = pLayout;
}
void LwpStory::SetSectionName(OUString StyleName)
{
    m_CurrSectionName = StyleName;
}
OUString LwpStory::GetSectionName()
{
    return m_CurrSectionName;
}
sal_Bool LwpStory::IsPMModified()
{
    return m_bPMModified;
}
LwpPageLayout* LwpStory::GetCurrentLayout()
{
    return m_pCurrentLayout;
}
LwpHyperlinkMgr* LwpStory::GetHyperlinkMgr()
{
    return m_pHyperlinkMgr;
}
inline void LwpStory::AddBullStyleName2List(const rtl::OUString& rStyleName, const sal_uInt8& nPos)
{
    m_vBulletStyleNameList.push_back(std::make_pair(rStyleName, nPos));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
