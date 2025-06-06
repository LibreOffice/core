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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPPARA_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPPARA_HXX

#include <memory>
#include <config_lgpl.h>

#include <lwpobj.hxx>
#include <lwpobjstrm.hxx>

#include <string_view>
#include <vector>
#include "lwpdlvlist.hxx"
#include <lwpbasetype.hxx>
#include <lwpoverride.hxx>
#include <lwpfoundry.hxx>
#include "lwplayout.hxx"
#include "lwpfribptr.hxx"
#include "lwpfribtext.hxx"
#include <xfilter/xfparastyle.hxx>
#include <xfilter/xfsection.hxx>

class LwpParaProperty;
class LwpBreaksOverride;
class LwpBulletStyleMgr;

class LwpNotifyListPersistent final
{
public:
    LwpNotifyListPersistent(){}
    void Read(LwpObjectStream* pObjStrm);
private:
    LwpObjectID m_Head;
};

class LwpForked3NotifyList final
{
public:
    LwpForked3NotifyList(){}
    LwpNotifyListPersistent& GetExtraList() { return m_ExtraList; }
    void Read(LwpObjectStream* pObjStrm);
private:
    LwpNotifyListPersistent m_ExtraList;
    LwpNotifyListPersistent m_PersistentList;
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

    sal_uInt16 nNumLevel;//hidelevels of paranumber frib

    ParaNumbering()
    {
        clear();
    }

    void clear()
    {
        pPrefix = nullptr;
        pParaNumber = nullptr;
        pSuffix = nullptr;
        nNumLevel = 0;
    }
};

class LwpDropcapLayout;
class LwpPara : public LwpDLVList
{
public:
    LwpPara(LwpObjectHeader const & objHdr, LwpSvStream* pStrm);

    void Read() override;
    void RegisterStyle() override;
    void Parse(IXFStream* pOutputStream) override;
    void XFConvert(XFContentContainer* pCont) override;

    LwpPara* GetParent();
    LwpObjectID& GetStoryID();
    LwpStory* GetStory();

    LwpParaStyle* GetParaStyle();
    XFParaStyle* GetXFParaStyle();
    LwpIndentOverride* GetIndent();
    LwpTabOverride* GetLocalTabOverride();
    LwpNumberingOverride* GetParaNumbering();

    inline LwpSilverBullet* GetSilverBullet();
    const OUString& GetBulletChar() const;
    sal_uInt32 GetBulletFontID() const;
    sal_uInt16 GetLevel() const;
    bool GetBulletFlag() const;

    void GetParaNumber(sal_uInt16 nPosition, ParaNumbering* pParaNumbering);
    LwpFribPtr& GetFribs();
    double GetBelowSpacing() const;
    LwpParaProperty* GetProperty(sal_uInt32 nPropType);
    void GatherDropcapInfo();
    const OUString& GetBulletStyleName() const;
    void SetBelowSpacing(double value);
    void SetBulletFlag(bool bFlag);
    void SetIndent(LwpIndentOverride* pIndentOverride);
    void SetFirstFrib(const OUString& Content,sal_uInt32 FontID);
    OUString const & GetContentText(bool bAllText = false);

    void SetParaDropcap(bool bFlag);
    void SetDropcapLayout(LwpDropcapLayout* pLayout);

    XFContentContainer* GetXFContainer();
    void AddXFContent(XFContent* pCont);
    void SetXFContainer(XFContentContainer* pCont);
    void RegisterTabStyle(XFParaStyle* pXFParaStyle);

    LwpBulletStyleMgr* GetBulletStyleMgr();
    bool operator <(LwpPara const & Other);
    bool ComparePagePosition(LwpVirtualLayout const * pPreLayout, LwpVirtualLayout const * pNextLayout);

    bool IsInCell();

    void SetAllText(std::u16string_view sText);

private:
    sal_uInt32  m_nOrdinal; // Ordinal number of this paragraph
    LwpObjectID m_ParaStyle;    // handle of paragraph style
    LwpPoint        m_Hint;     // Paragraph hint - width & height
    LwpObjectID m_Story;        // Story the paragraph is part of

    sal_uInt16  m_nFlags;
    sal_uInt16  m_nLevel;
    LwpFribPtr  m_Fribs;
    std::vector< std::unique_ptr<LwpParaProperty> >  m_vProps;
    //LwpForked3NotifyList* m_NotifyList;   //not saved

    OUString m_StyleName;
    OUString m_ParentStyleName;//Add to support toc
    std::unique_ptr<LwpBreaksOverride> m_pBreaks;
    OUString m_AftPageBreakName;
    OUString m_BefPageBreakName;
    OUString m_AftColumnBreakName;

    OUString m_BefColumnBreakName;
    std::unique_ptr<LwpIndentOverride> m_pIndentOverride;
    OUString m_Content;//for silver bullet,get text of first frib
    sal_uInt32 m_FontID;//for silver bullet
    OUString m_AllText;//get all text in this paragraph

    bool m_bHasBullet;
    LwpObjectID m_aSilverBulletID;
    LwpSilverBullet* m_pSilverBullet;
    std::unique_ptr<LwpBulletOverride> m_xBullOver;
    std::unique_ptr<LwpNumberingOverride> m_xParaNumbering;
    OUString m_aBulletStyleName;
    bool m_bBullContinue;
    //end add

    OUString m_SectionStyleName;
    bool m_bHasDropcap;
    sal_uInt16 m_nLines;
    sal_uInt32 m_nChars;
    LwpDropcapLayout* m_pDropcapLayout;
    double m_BelowSpacing;

    rtl::Reference<XFContentContainer> m_xXFContainer; //Current container for VO_PARA

    enum
    {
        /* bit definitions for the paragraph object flags */
        DEMAND_LOAD     = 0x0001,   // need to demand load this para
        DATA_DIRTY      = 0x0002,   // paragraph data is dirty
        SPELLSTARTOVER  = 0x0004,   // need to recheck paragraph
        SPELLDIRTY      = 0x0008,   // has misspelled word in para
        SPELLCHECKING   = 0x0010,   // started checking paragraph
        READING         = 0x0020,   // We're loading this para from disk
        DISKCHANGED     = 0x0040,   // Read size different from write size
        USEFLOWBREAKS   = 0x0080,   // Use line breaks provided by filter
        VALID_LEVEL     = 0x0100,   // cLevel is valid
        NOUSECOUNT      = 0x0200,   // Don't change the style's use count
        CHANGED         = 0x0400,   // This para has been edited
        SPREADBULLET    = 0x0800,   // Para's bullet is in edit-on-page mode
        NEWBULLET       = 0x1000,   // Bullets should have new font behavior

        // Don't write these flags out to disk
        NOWRITEFLAGS    = (READING | DISKCHANGED | CHANGED),

        MAX_INDENT_LEVELS   = 10
    };
private:
    virtual ~LwpPara() override;

    static void OverrideAlignment(LwpAlignmentOverride* base,LwpAlignmentOverride* over,XFParaStyle* pOverStyle);
    void OverrideIndent(LwpIndentOverride* base,LwpIndentOverride* over,XFParaStyle* pOverStyle);
    void OverrideSpacing(LwpSpacingOverride* base,LwpSpacingOverride* over,XFParaStyle* pOverStyle);
    void OverrideParaBorder(LwpParaProperty* pProps, XFParaStyle* pOverStyle);
    void OverrideParaBreaks(LwpParaProperty* pProps, XFParaStyle* pOverStyle);

    void OverrideParaBullet(LwpParaProperty* pProps);
    void OverrideParaNumbering(LwpParaProperty const * pProps);

    void RegisterMasterPage(XFParaStyle const * pBaseStyle);
    void RegisterNewSectionStyle(LwpPageLayout* pLayout);

    void ParseDropcapContent();
    rtl::Reference<XFContentContainer> AddBulletList(XFContentContainer* pCont);
    void AddBreakAfter(XFContentContainer* pCont);
    void AddBreakBefore(XFContentContainer* pCont);
    rtl::Reference<XFSection> CreateXFSection();

    void ReadPropertyList(LwpObjectStream* pFile);
};

inline LwpSilverBullet* LwpPara::GetSilverBullet()
{
    return m_pSilverBullet;
}
inline const OUString& LwpPara::GetBulletChar() const
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
inline void LwpPara::SetBulletFlag(bool bFlag)
{
    m_bHasBullet = bFlag;
}
inline bool LwpPara::GetBulletFlag() const
{
    return m_bHasBullet;
}
inline LwpFribPtr& LwpPara::GetFribs()
{
    return m_Fribs;
}
inline XFContentContainer* LwpPara::GetXFContainer()
{
    return m_xXFContainer.get();
}
inline const OUString& LwpPara::GetBulletStyleName() const
{
    return m_aBulletStyleName;
}
inline void LwpPara::AddXFContent(XFContent* pCont)
{
    if (!m_xXFContainer)
        throw std::runtime_error("paragraph lacks container");
    m_xXFContainer->Add(pCont);
}
inline void LwpPara::SetXFContainer(XFContentContainer* pCont)
{
    m_xXFContainer.set(pCont);
}
inline LwpIndentOverride* LwpPara::GetIndent()
{
    return m_pIndentOverride.get();
}
inline void LwpPara::SetIndent(LwpIndentOverride* pIndentOverride)
{
    m_pIndentOverride.reset( pIndentOverride );
}
inline LwpObjectID& LwpPara::GetStoryID()
{
    return m_Story;
}
inline LwpStory* LwpPara::GetStory()
{
    if (m_Story.obj().is())
        return dynamic_cast<LwpStory*>(m_Story.obj().get());
    return nullptr;
}
inline void LwpPara::SetParaDropcap(bool bFlag)
{
    m_bHasDropcap = bFlag;
}
inline void LwpPara::SetDropcapLayout(LwpDropcapLayout* pLayout)
{
    m_pDropcapLayout = pLayout;
}
inline double LwpPara::GetBelowSpacing() const
{
    return m_BelowSpacing;
}
inline void LwpPara::SetBelowSpacing(double value)
{
    m_BelowSpacing = value;
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
