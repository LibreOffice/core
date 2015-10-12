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
 *  the class for section frib and process section and page layout
 ************************************************************************/

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPFRIBSECTION_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPFRIBSECTION_HXX

#include "lwpfrib.hxx"
#include "lwppara.hxx"

/**
 * @brief       Process page layout for inserting pagelayout or section
 *
 */
class LwpMasterPage
{
public:
    LwpMasterPage(LwpPara* pPara, LwpPageLayout* pLayout);
    ~LwpMasterPage(){}

public:
    //for page layout register and parse
    bool RegisterMasterPage(LwpFrib* pFrib);
    void RegisterFillerPageStyle();
    void ParseSection(LwpFrib* pFrib);
    XFSection* CreateXFSection();

    OUString GetStyleName(){ return m_StyleName;}
    OUString GetFillerPageStyleName(){ return m_FillerPageStyleName;}
    bool IsNextPageType();

private:
    bool IsNeedSection();

private:
    bool     m_bNewSection;
    OUString m_FillerPageStyleName;
    OUString m_SectionStyleName;
    OUString m_StyleName; //master page style name

    LwpPara* m_pPara;
    LwpPageLayout* m_pLayout;
};

/**
 * @brief       FRIB_TAB_SECTION object
 *
 */
class LwpSection;
class XFIndex;
class LwpFribSection: public LwpFrib
{
public:
    explicit LwpFribSection(LwpPara* pPara );
    virtual ~LwpFribSection();
    void Read(LwpObjectStream* pObjStrm, sal_uInt16 len) override;
    LwpSection * GetSection();
    void RegisterSectionStyle();
    void ParseSection();
    void SetSectionName();

    LwpPageLayout* GetPageLayout();
private:
    void SetDefaultAlphaIndex(XFIndex* pXFIndex);

private:
    LwpObjectID m_Section;
    LwpMasterPage* m_pMasterPage;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
