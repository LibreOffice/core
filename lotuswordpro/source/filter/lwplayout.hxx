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

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPLAYOUT_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPLAYOUT_HXX

#include <memory>
#include <sal/config.h>

#include <rtl/ref.hxx>

#include <lwpheader.hxx>
#include <lwpobj.hxx>
#include <lwpobjhdr.hxx>
#include <lwpobjstrm.hxx>
#include <lwpobjid.hxx>
#include "lwpdlvlist.hxx"
#include <lwpfilehdr.hxx>

#include "lwplayoutdef.hxx"
#include <xfilter/xfdefs.hxx>
#include <xfilter/xffont.hxx>
#include <xfilter/xfpagemaster.hxx>
#include <xfilter/xfcolumns.hxx>
#include <xfilter/xfborders.hxx>
#include <xfilter/xfframestyle.hxx>
#include <xfilter/xfframe.hxx>
#include <xfilter/xfbgimage.hxx>
#include "lwpusewhen.hxx"
#include "lwplaypiece.hxx"


#define FIRST_LAYOUTPAGENO  0x0001
#define LAST_LAYOUTPAGENO       0xffff

#define LAY_BUOYNEUTRAL     0x02
#define LAY_BUOYLAYER       0x80

class LwpPara;
class LwpVirtualLayout : public LwpDLNFPVList
{
public:
    LwpVirtualLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    virtual sal_uInt16 GetNumCols(){return 1;}
    virtual double GetColGap(sal_uInt16 nIndex);
    virtual bool IsAutoGrow(){ return false;}
    virtual bool IsAutoGrowUp(){ return false;}
    virtual bool IsAutoGrowLeft(){ return false;}
    virtual bool IsAutoGrowRight(){ return false;}
    bool IsFitGraphic();
    bool IsAutoGrowWidth();
    bool IsInlineToMargin();
    virtual sal_uInt8 GetContentOrientation(){ return TEXT_ORIENT_LRTB;}
    bool GetHonorProtection()
    {
        if (m_bGettingHonorProtection)
            throw std::runtime_error("recursion in layout");
        m_bGettingHonorProtection = true;
        bool bRet = HonorProtection();
        m_bGettingHonorProtection = false;
        return bRet;
    }
    bool GetMarginsSameAsParent()
    {
        if (m_bGettingMarginsSameAsParent)
            throw std::runtime_error("recursion in layout");
        m_bGettingMarginsSameAsParent = true;
        bool bRet = MarginsSameAsParent();
        m_bGettingMarginsSameAsParent = false;
        return bRet;
    }
    bool GetIsProtected()
    {
        if (m_bGettingIsProtected)
            throw std::runtime_error("recursion in layout");
        m_bGettingIsProtected = true;
        bool bRet = IsProtected();
        m_bGettingIsProtected = false;
        return bRet;
    }
    bool GetIsAutoGrowDown()
    {
        if (m_bGettingIsAutoGrowDown)
            throw std::runtime_error("recursion in layout");
        m_bGettingIsAutoGrowDown = true;
        bool bRet = IsAutoGrowDown();
        m_bGettingIsAutoGrowDown = false;
        return bRet;
    }
    bool GetHasProtection()
    {
        if (m_bGettingHasProtection)
            throw std::runtime_error("recursion in layout");
        m_bGettingHasProtection = true;
        bool bRet = HasProtection();
        m_bGettingHasProtection = false;
        return bRet;
    }
    double GetMarginsValue(sal_uInt8 nWhichSide)
    {
        if (m_bGettingMarginsValue)
            throw std::runtime_error("recursion in layout");
        m_bGettingMarginsValue = true;
        auto fRet = MarginsValue(nWhichSide);
        m_bGettingMarginsValue = false;
        return fRet;
    }
    double GetExtMarginsValue(sal_uInt8 nWhichSide)
    {
        if (m_bGettingExtMarginsValue)
            throw std::runtime_error("recursion in layout");
        m_bGettingExtMarginsValue = true;
        auto fRet = ExtMarginsValue(nWhichSide);
        m_bGettingExtMarginsValue = false;
        return fRet;
    }
    const OUString& GetStyleName(){ return m_StyleName;}
    bool IsComplex();
    virtual bool IsAnchorPage(){ return false;}
    virtual bool IsAnchorFrame(){ return false;}
    virtual bool IsAnchorCell(){ return false;}
    bool IsPage();
    bool IsHeader();
    bool IsFooter();
    bool IsFrame();
    bool IsCell();
    bool IsSuperTable();
    bool IsGroupHead();
    virtual sal_uInt8 GetRelativeType();
    bool IsRelativeAnchored();
    LwpUseWhen* GetUseWhen();
    virtual LwpUseWhen* VirtualGetUseWhen(){ return nullptr;}
    virtual bool IsUseOnAllPages(){ return false;}
    virtual bool IsUseOnAllEvenPages(){ return false;}
    virtual bool IsUseOnAllOddPages(){ return false;}
    virtual bool IsUseOnPage(){ return false;}
    virtual sal_Int32 GetPageNumber(sal_uInt16 /*nLayoutNumber*/) { return -1;}
    bool IsMinimumHeight();
    virtual bool IsForWaterMark(){ return false;}
    virtual LwpPara* GetLastParaOfPreviousStory() { return nullptr; }
    rtl::Reference<LwpVirtualLayout> GetParentLayout();
    virtual rtl::Reference<LwpVirtualLayout> GetContainerLayout() { return rtl::Reference<LwpVirtualLayout>(); }
    void RegisterChildStyle();
    bool NoContentReference();
    bool IsStyleLayout();
    enumXFAlignType GetVerticalAlignmentType()
    {
        if (m_nAttributes & STYLE_CENTEREDVERTICALLY)
        {
            return enumXFAlignMiddle;
        }
        else if (m_nAttributes & STYLE_BOTTOMALIGNED)
        {
            return enumXFAlignBottom;
        }
        return enumXFAlignTop;
    };
    void SetStyleName(const OUString & str){ m_StyleName = str;}
    virtual double GetWidth(){return -1;}

    //Check whether there are contents in the layout
    virtual bool HasContent(){return false;}
    //End by
protected:
    void Read() override;
    bool HasProtection();
    virtual bool HonorProtection();
    virtual bool IsProtected();
    virtual bool IsAutoGrowDown(){ return false;}
    virtual double MarginsValue(sal_uInt8 /*nWhichSide*/){return 0;}
    virtual double ExtMarginsValue(sal_uInt8 /*nWhichSide*/){return 0;}
    virtual bool MarginsSameAsParent();
protected:
    bool m_bGettingHonorProtection;
    bool m_bGettingMarginsSameAsParent;
    bool m_bGettingHasProtection;
    bool m_bGettingIsProtected;
    bool m_bGettingIsAutoGrowDown;
    bool m_bGettingMarginsValue;
    bool m_bGettingExtMarginsValue;
    bool m_bGettingUsePrinterSettings;
    bool m_bGettingScaleCenter;
    bool m_bGettingBorderStuff;
    bool m_bGettingUseWhen;
    bool m_bGettingStyleLayout;
    sal_uInt32 m_nAttributes;
    sal_uInt32 m_nAttributes2;
    sal_uInt32 m_nAttributes3;
    sal_uInt32 m_nOverrideFlag;
    sal_uInt16 m_nDirection;
    sal_uInt16 m_nEditorID;
    LwpObjectID m_NextEnumerated;
    LwpObjectID m_PreviousEnumerated;
    OUString m_StyleName;
    enum LWP_LAYOUT_TYPE
    {
        LWP_VIRTUAL_LAYOUT,
        LWP_HEAD_LAYOUT,
        LWP_PAGE_LAYOUT,
        LWP_HEADER_LAYOUT,
        LWP_FOOTER_LAYOUT,
        LWP_FRAME_LAYOUT,
        LWP_SUPERTABLE_LAYOUT,
        LWP_TABLE_LAYOUT,
        LWP_ROW_LAYOUT,
        LWP_COLUMN_LAYOUT,
        LWP_CELL_LAYOUT,
        LWP_CONNECTED_CELL_LAYOUT,
        LWP_HIDDEN_CELL_LAYOUT,
        LWP_TABLE_HEADING_LAYOUT,
        LWP_ROW_HEADING_LAYOUT,
        LWP_DROPCAP_LAYOUT,
        LWP_GROUP_LAYOUT,
        LWP_TOC_SUPERTABLE_LAYOUT,
        LWP_FOOTNOTE_LAYOUT,
        LWP_FOOTNOTE_ROW_LAYOUT,
        LWP_FOOTNOTE_CELL_LAYOUT,
        LWP_ENDNOTE_LAYOUT,
        LWP_ENDNOTE_SUPERTABLE_LAYOUT,
        LWP_FOOTNOTE_SUPERTABLE_LAYOUT,
        LWP_NOTE_LAYOUT,
        LWP_NOTEHEADER_LAYOUT,
        LWP_NOTETEXT_LAYOUT,
        LWP_VIEWPORT_LAYOUT
    };
    enum LayoutDirection
    {
        LAY_USEDIRECTION    = 0x01,
        LAY_AUTOGROW        = 0x02,
        LAY_AUTOSIZE        = 0x04,
        LAY_TOCONTAINER     = 0x08,
        LAY_DIRALLBITS      = 0x0f
    };
    enum    {SHIFT_UP = 0, SHIFT_DOWN = 4, SHIFT_LEFT = 8, SHIFT_RIGHT =12};
public:
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_VIRTUAL_LAYOUT;}
    LwpVirtualLayout* FindChildByType(LWP_LAYOUT_TYPE eType);
};

class LwpAssociatedLayouts final
{
public:
    LwpAssociatedLayouts(){}
    void Read(LwpObjectStream* pStrm);
    LwpObjectID& GetOnlyLayout() { return m_OnlyLayout;}
    rtl::Reference<LwpVirtualLayout> GetLayout(LwpVirtualLayout const * pStartLayout);
private:
    LwpObjectID m_OnlyLayout; //LwpVirtualLayout
    LwpDLVListHeadTail m_Layouts;
};

class LwpHeadLayout : public LwpVirtualLayout
{
public:
    LwpHeadLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm);
    void RegisterStyle() override;
    rtl::Reference<LwpVirtualLayout> FindEnSuperTableLayout();
protected:
    void Read() override;
    virtual LWP_LAYOUT_TYPE GetLayoutType () override { return LWP_HEAD_LAYOUT;}
};

class LwpLayoutStyle final
{
public:
    LwpLayoutStyle();
    ~LwpLayoutStyle();
    void Read(LwpObjectStream* pStrm);
private:
    sal_uInt32      m_nStyleDefinition;
    std::unique_ptr<LwpAtomHolder>  m_pDescription;
    sal_uInt16      m_nKey;
};

class LwpLayoutMisc final
{
public:
    LwpLayoutMisc();
    ~LwpLayoutMisc();
    void Read(LwpObjectStream* pStrm);
private:
    sal_Int32     m_nGridDistance;
    sal_uInt16    m_nGridType;
    LwpAtomHolder m_aContentStyle;
};

class LwpMiddleLayout : public LwpVirtualLayout
{
public:
    LwpMiddleLayout( LwpObjectHeader const &objHdr, LwpSvStream* pStrm );
    virtual ~LwpMiddleLayout() override;
    double GetGeometryHeight();
    double GetGeometryWidth();
    LwpBorderStuff* GetBorderStuff();
    LwpBackgroundStuff* GetBackgroundStuff();
    LwpLayoutGeometry* GetGeometry();
    enumXFTextDir GetTextDirection();
    std::unique_ptr<XFBorders> GetXFBorders();
    LwpColor* GetBackColor();
    virtual bool IsAutoGrow() override;
    virtual bool IsAutoGrowUp() override;
    virtual bool IsAutoGrowLeft() override;
    virtual bool IsAutoGrowRight() override;
    virtual sal_uInt8 GetContentOrientation() override;
    virtual bool HonorProtection() override;
    virtual bool IsProtected() override;
    rtl::Reference<LwpVirtualLayout> GetWaterMarkLayout();
    std::unique_ptr<XFBGImage> GetXFBGImage();
    bool GetUsePrinterSettings();

    LwpLayoutScale* GetLayoutScale(){return dynamic_cast<LwpLayoutScale*>(m_LayScale.obj().get());}
        sal_uInt16 GetScaleMode();
    sal_uInt16 GetScaleTile();
    sal_uInt16 GetScaleCenter();

    bool CanSizeRight();
    virtual double GetWidth() override;
    sal_Int32 GetMinimumWidth();
    bool IsSizeRightToContent();
    bool IsSizeRightToContainer();
    sal_Int32 DetermineWidth();
    virtual double GetHeight();
    LwpPoint GetOrigin();

    bool IsPatternFill();
    std::unique_ptr<XFBGImage> GetFillPattern();

    //Check whether there are contents in the layout
    virtual bool HasContent() override;

protected:
    void Read() override;
    virtual bool MarginsSameAsParent() override;
    virtual double MarginsValue(sal_uInt8 nWhichSide) override;
    virtual double ExtMarginsValue(sal_uInt8 nWhichSide) override;
    virtual bool IsAutoGrowDown() override;
private:
    LwpObjectID m_BasedOnStyle;
protected:
    enum
    {
        DISK_GOT_STYLE_STUFF        = 0x01,
        DISK_GOT_MISC_STUFF         = 0x02
    };

    LwpObjectID m_Content;
    rtl::Reference<LwpObject> GetBasedOnStyle();
    LwpObjectID     m_TabPiece;
    LwpLayoutStyle  m_aStyleStuff;
    LwpLayoutMisc   m_aMiscStuff;
    LwpObjectID     m_LayGeometry;
    LwpObjectID     m_LayScale;
    LwpObjectID     m_LayMargins;
    LwpObjectID     m_LayBorderStuff;
    LwpObjectID     m_LayBackgroundStuff;
    LwpObjectID     m_LayExtBorderStuff;
    bool            m_bGettingGeometry;
    bool            m_bGettingBackgroundStuff;
public:
    LwpObjectID& GetContent() { return m_Content; }
    LwpTabOverride* GetTabOverride();
};

class LwpLayout : public LwpMiddleLayout
{
private:
    bool m_bGettingShadow;
    bool m_bGettingNumCols;
public:
    LwpLayout( LwpObjectHeader const &objHdr, LwpSvStream* pStrm );
    virtual ~LwpLayout() override;
    XFColumns* GetXFColumns();
    XFColumnSep* GetColumnSep();
    LwpShadow* GetShadow();
    XFShadow* GetXFShadow();
protected:
    void Read() override;
protected:
    LwpUseWhen  m_aUseWhen;
    LwpObjectID m_Position;
    LwpObjectID m_LayColumns;
    LwpObjectID m_LayGutterStuff;
    LwpObjectID m_LayJoinStuff;
    LwpObjectID m_LayShadow;
    LwpObjectID m_LayExtJoinStuff;

public:
    LwpUseWhen* VirtualGetUseWhen() override;
    virtual sal_uInt16 GetNumCols() override;
    virtual double GetColGap(sal_uInt16 nIndex) override;
    sal_uInt16 GetUsePage();
public:
    enum UseWhenType
    {
        StartWithinColume,
        StartWithinPage,
        StartOnNextPage,
        StartOnOddPage,
        StartOnEvenPage
    };
public:
    UseWhenType GetUseWhenType();
    virtual bool IsUseOnAllPages() override;
    virtual bool IsUseOnAllEvenPages() override;
    virtual bool IsUseOnAllOddPages() override;
    virtual bool IsUseOnPage() override;
    LwpObjectID& GetPosition(){ return m_Position;}
    virtual rtl::Reference<LwpVirtualLayout> GetContainerLayout() override;
};

class LwpPlacableLayout : public LwpLayout
{
public:
    LwpPlacableLayout( LwpObjectHeader const &objHdr, LwpSvStream* pStrm );
    virtual ~LwpPlacableLayout() override;
    sal_uInt8 GetWrapType();
    LwpLayoutRelativity* GetRelativityPiece();
    virtual sal_uInt8 GetRelativeType() override;
    sal_Int32 GetBaseLineOffset();
    virtual bool IsAnchorPage() override;
    virtual bool IsAnchorFrame() override;
    virtual bool IsAnchorCell() override;
    virtual void XFConvertFrame(XFContentContainer* /*pCont*/, sal_Int32 /*nStart*/ = 0, sal_Int32 /*nEnd*/ = 0, bool /*bAll*/ = false) {}
    /**
     * @descr:   Get font style for setting position of frame
     *
     */
    const rtl::Reference<XFFont>& GetFont() { return m_pFont; }
    void SetFont(rtl::Reference<XFFont> const & pFont);
    enum WrapType
    {
        LAY_WRAP_AROUND = 1,
        LAY_NO_WRAP_BESIDE,
        LAY_NO_WRAP_AROUND,
        LAY_WRAP_LEFT,
        LAY_WRAP_RIGHT,
        LAY_WRAP_BOTH,
        LAY_WRAP_IRREG_BOTH,
        LAY_WRAP_IRREG_LEFT,
        LAY_WRAP_IRREG_RIGHT,
        LAY_WRAP_IRREG_BIGGEST
    };
protected:
    void Read() override;
protected:
    bool m_bGettingWrapType;
    bool m_bGettingLayoutRelativity;
    sal_uInt8 m_nWrapType;
    sal_uInt8 m_nBuoyancy;
    sal_Int32 m_nBaseLineOffset;
    LwpAtomHolder m_Script;
    LwpObjectID m_LayRelativity;
    rtl::Reference<XFFont> m_pFont;//for frame position
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
