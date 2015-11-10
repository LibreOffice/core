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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPSTORY_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPSTORY_HXX

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
    typedef std::pair<OUString, sal_uInt8> NamePosPair;
    std::vector <NamePosPair> m_vBulletStyleNameList;

    // , 02/16/2005
    bool m_bPMModified;
    LwpPageLayout* m_pCurrentLayout;    //for register page layout style
    std::vector<LwpPageLayout*> m_LayoutList;
    LwpLayout* m_pTabLayout;    //for register tab style

    bool m_bDropcap;
    LwpHyperlinkMgr* m_pHyperlinkMgr;
    OUString m_CurrSectionName;

    XFContentContainer* m_pXFContainer;

protected:
    void Read() override;
    void XFConvertFrameInCell(XFContentContainer* pCont);
    void XFConvertFrameInFrame(XFContentContainer* pCont);
    void XFConvertFrameInHeaderFooter(XFContentContainer* pCont);

public:
    void RegisterStyle() override;
    void Parse(IXFStream* pOutputStream) override;
    virtual void XFConvert(XFContentContainer* pCont) override;
    void XFConvertFrameInPage(XFContentContainer* pCont);
    inline LwpObjectID& GetFirstPara();
    inline LwpObjectID& GetLastPara();
     XFContentContainer* GetXFContent();
    void AddXFContent(XFContent* pContent);

    bool GetDropcapFlag() { return m_bDropcap; }
    LwpPageLayout* GetCurrentLayout() { return m_pCurrentLayout; }
    inline LwpMiddleLayout* GetTabLayout();//for register tab style
    OUString GetSectionName() { return m_CurrSectionName; }
    LwpHyperlinkMgr* GetHyperlinkMgr() { return m_pHyperlinkMgr; }

    inline bool IsPMModified() { return m_bPMModified; }
    inline void SetPMModified(bool bPMModified);
    inline void SetDropcapFlag(bool bFlag);
    inline void SetTabLayout(LwpLayout* pLayout);
    inline void SetSectionName(const OUString& StyleName);

    void SetCurrentLayout(LwpPageLayout* pPageLayout);
    void AddPageLayout(LwpPageLayout* pObject);
    LwpPageLayout* GetNextPageLayout();
    bool IsNeedSection();
    void SortPageLayout();

    LwpPara* GetLastParaOfPreviousStory();

    OUString GetContentText(bool bAllText = false);
    inline void AddBullStyleName2List(const OUString& rStyleName, const sal_uInt8& nPos);
    bool IsBullStyleUsedBefore(const OUString& rStyleName, const sal_uInt8& nPos);

    OUString RegisterFirstFribStyle();
};

LwpObjectID& LwpStory::GetFirstPara()
{
    return m_ParaList.GetHead();
}
LwpObjectID& LwpStory::GetLastPara()
{
    return m_ParaList.GetTail();
}
LwpMiddleLayout* LwpStory::GetTabLayout()
{
    if(m_pTabLayout)
        return m_pTabLayout;
    return static_cast<LwpMiddleLayout*>(GetLayout(nullptr));
}
void LwpStory::SetPMModified(bool bPMModified)
{
    m_bPMModified = bPMModified;
}
void LwpStory::SetDropcapFlag(bool bFlag)
{
    m_bDropcap = bFlag;
}
void LwpStory::SetTabLayout(LwpLayout* pLayout)
{
    m_pTabLayout = pLayout;
}
void LwpStory::SetSectionName(const OUString& StyleName)
{
    m_CurrSectionName = StyleName;
}
inline void LwpStory::AddBullStyleName2List(const OUString& rStyleName, const sal_uInt8& nPos)
{
    m_vBulletStyleNameList.push_back(std::make_pair(rStyleName, nPos));
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
