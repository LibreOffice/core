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
 *  LwpPara: Word Pro object for paragraph
 ************************************************************************/
/*************************************************************************
 * Change History
 Jan 2005			Created
 ************************************************************************/


#ifndef _LWPPARAGRAPH_HXX_
#define _LWPPARAGRAPH_HXX_
#include "lwpfribheader.hxx"
#include "lwpobj.hxx"
#include "lwpobjstrm.hxx"

#include <vector>
#include "lwpheader.hxx"
#include "lwpdlvlist.hxx"
#include "lwpbasetype.hxx"
#include "lwpoverride.hxx"
#include "lwpfoundry.hxx"
#include "lwplayout.hxx"
#include "lwpfrib.hxx"
#include "lwpfribptr.hxx"
#include "lwpfribtext.hxx"
#include "xfilter/xfparagraph.hxx"
#include "xfilter/xfdefs.hxx"
#include "xfilter/xfparastyle.hxx"
#include "xfilter/xfsection.hxx"

class LwpParaProperty;
class LwpPara;
class LwpBreaksOverride;
class LwpBulletStyleMgr;
class LwpNotifyListPersistent
{
public:
    LwpNotifyListPersistent(){};
    void Read(LwpObjectStream* pObjStrm);
protected:
    LwpObjectID m_Head;
};

class LwpForked3NotifyList
{
public:
    LwpForked3NotifyList(){};
protected:
    LwpNotifyListPersistent m_ExtraList;
    LwpNotifyListPersistent m_PersistentList;
public:
    LwpNotifyListPersistent* GetExtraList(){return &m_ExtraList;}
    void Read(LwpObjectStream* pObjStrm);
};

class LwpParaStyle;
class LwpTabOverride;
class LwpNumberingOverride;
class LwpSilverBullet;
class LwpPageLayout;

struct ParaNumbering
{
    LwpFribText* pPrefix;
    LwpFribParaNumber* pParaNumber;
    LwpFribText* pSuffix;

    sal_uInt16 nPrefixLevel;//hidelevels of prefix text frib
    sal_uInt16 nNumLevel;//hidelevels of paranumber frib
    sal_uInt16 nSuffixLevel;//hidelevels of suffix text frib

    ParaNumbering()
    {
        clear();
    }

    void clear()
    {
        pPrefix = NULL;
        pParaNumber = NULL;
        pSuffix = NULL;
        nPrefixLevel = 0;
        nNumLevel = 0;
        nSuffixLevel = 0;
    }
};

class LwpDropcapLayout;
class LwpCellLayout;
class LwpPara : public LwpDLVList
{
public:
    LwpPara(LwpObjectHeader& objHdr, LwpSvStream* pStrm);
    ~LwpPara();
public:
    void Read();
    void RegisterStyle();
    void Parse(IXFStream* pOutputStream);
    void XFConvert(XFContentContainer* pCont);
    void Release();

    LwpPara* GetParent();
    LwpObjectID* GetStoryID();
    LwpStory* GetStory();
    LwpBreaksOverride* GetBreaks();

    LwpParaStyle* GetParaStyle();
    XFParaStyle* GetXFParaStyle();
    LwpIndentOverride* GetIndent();
    LwpTabOverride* GetLocalTabOverride();
    LwpNumberingOverride* GetParaNumbering();

    inline LwpSilverBullet* GetSilverBullet();
    inline LwpObjectID GetSilverBulletID();
    rtl::OUString GetBulletChar() const;
    sal_uInt32 GetBulletFontID() const;
    sal_uInt16 GetLevel() const;
    sal_Bool GetBulletFlag() const;

    void GetParaNumber(sal_uInt16 nPosition, ParaNumbering* pParaNumbering);
    LwpFribPtr* GetFribs();
    double GetBelowSpacing();
    LwpParaProperty* GetProperty(sal_uInt32 nPropType);
    void GatherDropcapInfo();
    rtl::OUString GetBulletStyleName() const;
    void SetBelowSpacing(double value);
    void SetBulletStyleName(const rtl::OUString& rNewName);
    void SetBulletFlag(sal_Bool bFlag);
    void SetIndent(LwpIndentOverride* pIndentOverride);
    void SetFirstFrib(rtl::OUString Content,sal_uInt32 FontID);
    OUString GetContentText(sal_Bool bAllText = sal_False);

    void SetParaDropcap(sal_Bool bFlag);
    void SetDropcapLines(sal_uInt16 number);
    void SetDropcapChars(sal_uInt32 chars);
    void SetDropcapLayout(LwpDropcapLayout* pLayout);
    sal_Bool IsHasDropcap();

    XFContentContainer* GetXFContainer();
    void AddXFContent(XFContent* pCont);
    void SetXFContainer(XFContentContainer* pCont);
    void FindLayouts();// for register pagelayout, add by , 02/20/2005
    void RegisterTabStyle(XFParaStyle* pXFParaStyle);

    // add by  04/13/2005
    LwpVirtualLayout* GetLayoutWithMyStory();
    LwpBulletStyleMgr* GetBulletStyleMgr();
    sal_uInt32 GetOrdinal(){ return m_nOrdinal;};
    sal_Bool operator <(LwpPara& Other);
    sal_Bool ComparePagePosition(LwpVirtualLayout* pPreLayout, LwpVirtualLayout* pNextLayout);
//	rtl::OUString RegisterBulletStyle();
    // end add

    sal_Bool IsNumberRight();
    sal_Bool IsInCell();

    void SetAllText(OUString sText);
    OUString GetStyleName(){return m_StyleName;}

protected:
    sal_uInt32	m_nOrdinal;	// Ordinal number of this paragraph
    LwpObjectID	m_ParaStyle;	// handle of paragraph style
    LwpPoint		m_Hint;		// Paragraph hint - width & height
    LwpObjectID	m_Story;		// Story the paragraph is part of

    sal_uInt16	m_nFlags;
    sal_uInt16	m_nLevel;
    LwpFribPtr	m_Fribs;
    LwpParaProperty*  m_pProps;
    //LwpForked3NotifyList* m_NotifyList;	//not saved

    rtl::OUString m_StyleName;
    rtl::OUString m_ParentStyleName;//Add by ,to support toc,2005/12/13
    LwpBreaksOverride* m_pBreaks;
    rtl::OUString m_AftPageBreakName;
    rtl::OUString m_BefPageBreakName;
    rtl::OUString m_AftColumnBreakName;

    rtl::OUString m_BefColumnBreakName;
    LwpIndentOverride* m_pIndentOverride;
    rtl::OUString m_Content;//for silver bullet,get text of first frib, add by  2/1
    sal_uInt32 m_FontID;//for silver bullet, add by  2/1
    rtl::OUString m_AllText;//get all text in this paragraph

    //add by
    sal_Bool m_bHasBullet;
    LwpObjectID m_aSilverBulletID;
    LwpSilverBullet* m_pSilverBullet;
    LwpBulletOverride* m_pBullOver;
//	LwpNumberingOverride* m_pParaNumbering;
    LwpNumberingOverride m_aParaNumbering;
    rtl::OUString m_aBulletStyleName;
    sal_Bool m_bBullContinue;
    //end add

    OUString m_SectionStyleName;
    sal_Bool m_bHasDropcap;
    sal_uInt16 m_nLines;
    sal_uInt32 m_nChars;
    LwpDropcapLayout* m_pDropcapLayout;
    double m_BelowSpacing;

    XFContentContainer* m_pXFContainer;	//Current container for VO_PARA

    rtl::OUString m_TabStyleName;
    enum
    {
        /* bit definitions for the paragraph object flags */
        DEMAND_LOAD		= 0x0001,	// need to demand load this para
        DATA_DIRTY		= 0x0002,	// paragraph data is dirty
        SPELLSTARTOVER	= 0x0004,	// need to recheck paragraph
        SPELLDIRTY		= 0x0008,	// has misspelled word in para
        SPELLCHECKING	= 0x0010,	// started checking paragraph
        READING			= 0x0020,	// We're loading this para from disk
        DISKCHANGED		= 0x0040,	// Read size different from write size
        USEFLOWBREAKS	= 0x0080,	// Use line breaks provided by filter
        VALID_LEVEL		= 0x0100,	// cLevel is valid
        NOUSECOUNT		= 0x0200,	// Don't change the style's use count
        CHANGED			= 0x0400,	// This para has been edited
        SPREADBULLET	= 0x0800,	// Para's bullet is in edit-on-page mode
        NEWBULLET		= 0x1000,	// Bullets should have new font behavior

        // Don't write these flags out to disk
        NOWRITEFLAGS	= (READING | DISKCHANGED | CHANGED),

        MAX_INDENT_LEVELS	= 10
    };
private:
    void OverrideAlignment(LwpAlignmentOverride* base,LwpAlignmentOverride* over,XFParaStyle* pOverStyle);//add by  1-24
    void OverrideIndent(LwpIndentOverride* base,LwpIndentOverride* over,XFParaStyle* pOverStyle);
    void OverrideSpacing(LwpSpacingOverride* base,LwpSpacingOverride* over,XFParaStyle* pOverStyle);
    void OverrideParaBorder(LwpParaProperty* pProps, XFParaStyle* pOverStyle);
    void OverrideParaBreaks(LwpParaProperty* pProps, XFParaStyle* pOverStyle);

    void OverrideParaBullet(LwpParaProperty* pProps);
    void OverrideParaNumbering(LwpParaProperty* pProps);
    void OverrideTab(LwpTabOverride* base,LwpTabOverride* over,XFParaStyle* pOverStyle);
    BOOL IsBadHintsInFile();

    //void OutPutSectionTail(IXFStream* pOutputStream);
    //void OutPutBulletTail(LwpBulletStyleMgr* pBulletStyleMgr,IXFStream* pOutputStream);

    sal_Bool RegisterMasterPage(XFParaStyle* pBaseStyle);
    void RegisterNewSectionStyle(LwpPageLayout* pLayout);

    void ParseDropcapContent();
    XFContentContainer* AddBulletList(XFContentContainer* pCont);
    void AddBreakAfter(XFContentContainer* pCont);
    void AddBreakBefore(XFContentContainer* pCont);
    XFSection* CreateXFSection();

    /*
    sal_Bool IsNeedTabForTOC();
    void AddTabStyleForTOC();
    void AddTabStopForTOC();
    maybe useful for futer version */
};

inline LwpSilverBullet* LwpPara::GetSilverBullet()
{
    return m_pSilverBullet;
}
inline LwpObjectID LwpPara::GetSilverBulletID()
{
    return m_aSilverBulletID;
}
inline rtl::OUString LwpPara::GetBulletChar() const
{
    return m_Content;
}
inline sal_uInt32 LwpPara::GetBulletFontID() const
{
    return m_FontID;
}
inline sal_uInt16 LwpPara::GetLevel() const
{
    return m_nLevel;
}
inline void LwpPara::SetBulletStyleName(const rtl::OUString& rNewName)
{
    m_aBulletStyleName = rNewName;
}
inline void LwpPara::SetBulletFlag(sal_Bool bFlag)
{
    m_bHasBullet = bFlag;
}
inline sal_Bool LwpPara::GetBulletFlag() const
{
    return m_bHasBullet;
}
inline LwpFribPtr* LwpPara::GetFribs()
{
    return &m_Fribs;
}
inline LwpBreaksOverride* LwpPara::GetBreaks()
{
    return m_pBreaks;
}
inline XFContentContainer* LwpPara::GetXFContainer()
{
    return m_pXFContainer;
}
inline rtl::OUString LwpPara::GetBulletStyleName() const
{
    return m_aBulletStyleName;
}
inline void LwpPara::AddXFContent(XFContent* pCont)
{
    m_pXFContainer->Add(pCont);
}
inline void LwpPara::SetXFContainer(XFContentContainer* pCont)
{
    m_pXFContainer = pCont;
}
inline LwpIndentOverride* LwpPara::GetIndent()
{
    return m_pIndentOverride;
}
inline void LwpPara::SetIndent(LwpIndentOverride* pIndentOverride)
{
    if (m_pIndentOverride)
        delete m_pIndentOverride;
    m_pIndentOverride = pIndentOverride;
}
inline LwpObjectID* LwpPara::GetStoryID()
{
    return &m_Story;
}
inline LwpStory* LwpPara::GetStory()
{
    if (m_Story.obj())
        return static_cast<LwpStory*>(m_Story.obj());
    return NULL;
}

inline sal_Bool LwpPara::IsHasDropcap()
{
    return m_bHasDropcap;
}
inline void LwpPara::SetParaDropcap(sal_Bool bFlag)
{
    m_bHasDropcap = bFlag;
}
inline void LwpPara::SetDropcapLines(sal_uInt16 number)
{
    m_nLines = number;
}
inline void LwpPara::SetDropcapChars(sal_uInt32 chars)
{
    m_nChars = chars;
}
inline void LwpPara::SetDropcapLayout(LwpDropcapLayout* pLayout)
{
    m_pDropcapLayout = pLayout;
}
inline BOOL LwpPara::IsBadHintsInFile()
{
    return (BOOL) ((m_nFlags & DOC_BADFILEPARAHINTS) != 0);
}
inline double LwpPara::GetBelowSpacing()
{
    return m_BelowSpacing;
}
inline void LwpPara::SetBelowSpacing(double value)
{
    m_BelowSpacing = value;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
