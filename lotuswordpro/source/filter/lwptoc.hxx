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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTOC_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPTOC_HXX

#include "lwptablelayout.hxx"

class XFIndex;
class LwpFoundry;
class LwpTocLevelData;
class XFIndex;
/**
 * @brief
 * VO_TOCSUPERTABLELAYOUT object
 */
class LwpTocSuperLayout : public LwpSuperTableLayout
{
public:
    LwpTocSuperLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpTocSuperLayout();
    void RegisterStyle() override;
    virtual void XFConvert(XFContentContainer* pCont) override;
    virtual LWP_LAYOUT_TYPE GetLayoutType () override { return LWP_TOC_SUPERTABLE_LAYOUT;}
    virtual void XFConvertFrame(XFContentContainer* pCont, sal_Int32 nStart = 0, sal_Int32 nEnd = 0, bool bAll = false) override;
protected:
    void Read() override;
    bool GetRightAlignPageNumber(sal_uInt16 index);
    bool GetUsePageNumber(sal_uInt16 index);
    sal_uInt16 GetSeparatorType(sal_uInt16 index);
    LwpTocLevelData * GetSearchLevelPtr(sal_uInt16 index);
    static LwpTocLevelData * GetNextSearchLevelPtr(sal_uInt16 index, LwpTocLevelData * pCurData);
    void AddSourceStyle(XFIndex* pToc, LwpTocLevelData * pLevel,  LwpFoundry * pFoundry);
private:
    enum {MAX_LEVELS = 9};
    enum
    {
        TS_NOLEADERS = 0x01,
        TS_LEADERDOTS = 0x02,
        TS_LEADERDASHES = 0x04,
        TS_LEADERUNDERLINE = 0x08,
        TS_SEPARATORCOMMA = 0x10,
        TS_SEPARATORDOTS = 0x20,
        TS_PAGENUMBER = 0x40,
        TS_RIGHTALIGN = 0x80
    };

    enum
    {
        NONE = 0,
        LEADERDOTS = 1,
        LEADERDASHES = 2,
        LEADERUNDERLINE = 3,
        SEPARATORCOMMA = 4,
        SEPARATORDOTS = 5
    };

    //m_nFrom
    enum
    {
        SELECTEDTEXT = 1,
        ENTIREDOCUMENT = 2,
        CURRENTLEVELDIVISION = 3,
        CURRENTDIVISION = 4,
        CURRENTSECTION = 5
    };

    LwpAtomHolder m_TextMarker;         /* for selected text */
    LwpAtomHolder m_ParentName;     /* for currentleveldivision */
    LwpAtomHolder m_DivisionName;       /* for currentdivision&currentsection */
    LwpAtomHolder m_SectionName;        /* for currentsection */
    sal_uInt16 m_nFrom;
    LwpDLVListHeadTail m_SearchItems;
    LwpAtomHolder m_DestName[MAX_LEVELS];
    LwpAtomHolder m_DestPGName[MAX_LEVELS];
    sal_uInt32 m_nFlags[MAX_LEVELS];

    OUString m_TabStyleName;

    XFContentContainer* m_pCont;
};
/**
 * @brief
 * VO_TOCLEVELDATA object
 */
class LwpTocLevelData : public LwpDLVList
{
public:
    enum
    {
        USETEXT = 0x01,
        USENUMBER = 0x02
    };
    LwpTocLevelData(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    void RegisterStyle() override;
    virtual void XFConvert(XFContentContainer* pCont) override;
    inline sal_uInt16 GetLevel(){return m_nLevel;}
    inline bool GetUseText(){   return (m_nFlags & USETEXT) != 0;}
    inline OUString GetSearchStyle(){return m_SearchName.str();}
    inline bool GetUseLeadingText(){    return (m_nFlags & USENUMBER) != 0;}
private:
    virtual ~LwpTocLevelData();

    sal_uInt16 m_nFlags;
    sal_uInt16 m_nLevel;
    LwpAtomHolder m_SearchName;

protected:
    void Read() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
