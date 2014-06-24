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
/*************************************************************************
 * Change History
 Jan 2005           Created
 ************************************************************************/

#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPLAYOUT_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_LWPLAYOUT_HXX

#include <sal/config.h>

#include <rtl/ref.hxx>

#include "lwpheader.hxx"
#include "lwpobj.hxx"
#include "lwpobjhdr.hxx"
#include "lwpobjstrm.hxx"
#include "lwpobjid.hxx"
#include "lwpdlvlist.hxx"
#include "lwpfilehdr.hxx"

#include "lwplayoutdef.hxx"
#include "xfilter/xfdefs.hxx"
#include "xfilter/xffont.hxx"
#include "xfilter/xfpagemaster.hxx"
#include "xfilter/xfcolumns.hxx"
#include "xfilter/xfborders.hxx"
#include "xfilter/xfframestyle.hxx"
#include "xfilter/xfframe.hxx"
#include "xfilter/xfbgimage.hxx"
#include "lwpusewhen.hxx"

#define FIRST_LAYOUTPAGENO  0x0001
#define LAST_LAYOUTPAGENO       0xffff

#define LAY_BUOYNEUTRAL     0x02
#define LAY_BUOYLAYER       0x80

class LwpPara;
class LwpVirtualLayout : public LwpDLNFPVList
{
public:
    LwpVirtualLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpVirtualLayout(){}
    virtual bool MarginsSameAsParent();
    inline virtual sal_uInt16 GetNumCols(){return 1;}
    virtual double GetColWidth(sal_uInt16 nIndex);
    virtual double GetColGap(sal_uInt16 nIndex);
    virtual double GetMarginsValue(const sal_uInt8& /*nWhichSide*/){return 0;}
    virtual double GetExtMarginsValue(const sal_uInt8& /*nWhichSide*/){return 0;}
    virtual bool IsAutoGrow(){ return false;}
    virtual bool IsAutoGrowUp(){ return false;}
    virtual bool IsAutoGrowDown(){ return false;}
    virtual bool IsAutoGrowLeft(){ return false;}
    virtual bool IsAutoGrowRight(){ return false;}
    virtual bool IsFitGraphic();
    virtual bool IsAutoGrowWidth();
    virtual bool IsInlineToMargin();
    virtual sal_uInt8 GetContentOrientation(){ return TEXT_ORIENT_LRTB;}
    virtual bool HonorProtection();
    virtual bool IsProtected();
    virtual bool HasProtection();
    virtual OUString GetStyleName(){ return m_StyleName;}
    virtual bool IsComplex();
    virtual bool IsAnchorPage(){ return false;}
    virtual bool IsAnchorFrame(){ return false;}
    virtual bool IsAnchorCell(){ return false;}
    virtual bool IsPage();
    virtual bool IsHeader();
    virtual bool IsFooter();
    virtual bool IsFrame();
    virtual bool IsCell();
    virtual bool IsSuperTable();
    virtual bool IsGroupHead();
    virtual sal_uInt8 GetRelativeType();
    virtual bool IsRelativeAnchored();
    virtual LwpUseWhen* GetUseWhen();
    virtual LwpUseWhen* VirtualGetUseWhen(){ return NULL;}
    virtual bool IsUseOnAllPages(){ return false;}
    virtual bool IsUseOnAllEvenPages(){ return false;}
    virtual bool IsUseOnAllOddPages(){ return false;}
    virtual bool IsUseOnPage(){ return false;}
    virtual sal_Int32 GetPageNumber(sal_uInt16 /*nLayoutNumber*/ = 0){ return -1;}
    virtual bool IsMinimumHeight();
    virtual bool IsForWaterMark(){ return false;}
    virtual LwpPara* GetLastParaOfPreviousStory() { return NULL; }
    virtual LwpVirtualLayout* GetParentLayout();
    virtual LwpVirtualLayout* GetContainerLayout(){ return NULL;}
    virtual void RegisterChildStyle();
    virtual bool NoContentReference();
    virtual bool IsStyleLayout();
    virtual enumXFAlignType GetVerticalAlignmentType()
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
    virtual void SetStyleName(const OUString & str){ m_StyleName = str;}
    virtual double GetWidth(){return -1;}

    //Check whether there are contents in the layout
    virtual bool HasContent(){return false;}
    //End by
protected:
    void Read() SAL_OVERRIDE;
protected:
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
    virtual LwpVirtualLayout* FindChildByType(LWP_LAYOUT_TYPE eType);
};

class LwpAssociatedLayouts
{
public:
    LwpAssociatedLayouts(){}
public:
    void Read(LwpObjectStream* pStrm);
    LwpObjectID& GetOnlyLayout() { return m_OnlyLayout;}
    LwpDLVListHeadTail& GetLayouts() { return m_Layouts;}
    LwpVirtualLayout* GetLayout(LwpVirtualLayout* pStartLayout);
protected:
    LwpObjectID m_OnlyLayout; //LwpVirtualLayout
    LwpDLVListHeadTail m_Layouts;
};

class LwpHeadLayout : public LwpVirtualLayout
{
public:
    LwpHeadLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpHeadLayout(){}
    void RegisterStyle() SAL_OVERRIDE;
    LwpVirtualLayout* FindEnSuperTableLayout();
protected:
    void Read() SAL_OVERRIDE;
    virtual LWP_LAYOUT_TYPE GetLayoutType () SAL_OVERRIDE { return LWP_HEAD_LAYOUT;}
};

class LwpLayoutStyle
{
public:
    LwpLayoutStyle();
    virtual ~LwpLayoutStyle();
    void Read(LwpObjectStream* pStrm);
private:
    sal_uInt32      m_nStyleDefinition;
    LwpAtomHolder*  m_pDescription;
    sal_uInt16      m_nKey;
};

class LwpLayoutMisc
{
public:
    LwpLayoutMisc();
    virtual ~LwpLayoutMisc();
    void Read(LwpObjectStream* pStrm);
private:
    sal_Int32   m_nGridDistance;
    sal_uInt16 m_nGridType;
    LwpAtomHolder* m_pContentStyle;
};

#include "lwplaypiece.hxx"

class LwpMiddleLayout : public LwpVirtualLayout
{
public:
    LwpMiddleLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm );
    virtual ~LwpMiddleLayout();
    virtual bool MarginsSameAsParent() SAL_OVERRIDE;
    virtual double GetMarginsValue(const sal_uInt8& nWhichSide) SAL_OVERRIDE;
    virtual double GetExtMarginsValue(const sal_uInt8& nWhichSide) SAL_OVERRIDE;
    LwpLayoutGeometry* GetGeometry();
    double GetGeometryHeight();
    double GetGeometryWidth();
    LwpBorderStuff* GetBorderStuff();
    LwpBackgroundStuff* GetBackgroundStuff();
    enumXFTextDir GetTextDirection();
    XFBorders* GetXFBorders();
    LwpColor* GetBackColor();
    virtual bool IsAutoGrow() SAL_OVERRIDE;
    virtual bool IsAutoGrowUp() SAL_OVERRIDE;
    virtual bool IsAutoGrowDown() SAL_OVERRIDE;
    virtual bool IsAutoGrowLeft() SAL_OVERRIDE;
    virtual bool IsAutoGrowRight() SAL_OVERRIDE;
    virtual sal_uInt8 GetContentOrientation() SAL_OVERRIDE;
    virtual bool HonorProtection() SAL_OVERRIDE;
    virtual bool IsProtected() SAL_OVERRIDE;
    virtual LwpVirtualLayout* GetWaterMarkLayout();
    XFBGImage* GetXFBGImage();
    virtual bool GetUsePrinterSettings();

    LwpLayoutScale* GetLayoutScale(){return dynamic_cast<LwpLayoutScale*>(m_LayScale.obj().get());}
        sal_uInt16 GetScaleMode(void);
    sal_uInt16 GetScaleTile(void);
    sal_uInt16 GetScaleCenter(void);
    sal_uInt32 GetScalePercentage(void);
    double GetScaleWidth(void);
    double GetScaleHeight(void);

    bool CanSizeRight(void);
    virtual double GetWidth() SAL_OVERRIDE;
    virtual sal_Int32 GetMinimumWidth();
    bool IsSizeRightToContent(void);
    bool IsSizeRightToContainer(void);
    sal_Int32 DetermineWidth();
    virtual double GetHeight();
    virtual LwpPoint GetOrigin();

    bool IsPatternFill();
    XFBGImage* GetFillPattern();

    //Check whether there are contents in the layout
    virtual bool HasContent() SAL_OVERRIDE;
    //End by

protected:
    void Read() SAL_OVERRIDE;
protected:
    enum
    {
        DISK_GOT_STYLE_STUFF        = 0x01,
        DISK_GOT_MISC_STUFF         = 0x02
    };

    LwpObjectID m_Content;
    LwpObjectID m_BasedOnStyle;

    LwpObjectID     m_TabPiece;
    LwpLayoutStyle* m_pStyleStuff;
    LwpLayoutMisc*  m_pMiscStuff;
    LwpObjectID     m_LayGeometry;
    LwpObjectID     m_LayScale;
    LwpObjectID     m_LayMargins;
    LwpObjectID     m_LayBorderStuff;
    LwpObjectID     m_LayBackgroundStuff;
    LwpObjectID     m_LayExtBorderStuff;
public:
    LwpObjectID& GetContent() { return m_Content; }
    LwpTabOverride* GetTabOverride();
};

class LwpLayout : public LwpMiddleLayout
{
public:
    LwpLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm );
    virtual ~LwpLayout();
    XFColumns* GetXFColumns();
    XFColumnSep* GetColumnSep();
    LwpShadow* GetShadow();
    XFShadow* GetXFShadow();
protected:
    void Read() SAL_OVERRIDE;
protected:
    LwpUseWhen* m_pUseWhen;
    LwpObjectID m_Positon;
    LwpObjectID m_LayColumns;
    LwpObjectID m_LayGutterStuff;
    LwpObjectID m_LayJoinStuff;
    LwpObjectID m_LayShadow;
    LwpObjectID m_LayExtJoinStuff;

public:
    LwpUseWhen* VirtualGetUseWhen() SAL_OVERRIDE;
    virtual sal_uInt16 GetNumCols() SAL_OVERRIDE;
    virtual double GetColWidth(sal_uInt16 nIndex) SAL_OVERRIDE;
    virtual double GetColGap(sal_uInt16 nIndex) SAL_OVERRIDE;
    virtual sal_uInt16 GetUsePage();
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
    virtual UseWhenType GetUseWhenType();
    virtual bool IsUseOnAllPages() SAL_OVERRIDE;
    virtual bool IsUseOnAllEvenPages() SAL_OVERRIDE;
    virtual bool IsUseOnAllOddPages() SAL_OVERRIDE;
    virtual bool IsUseOnPage() SAL_OVERRIDE;
    LwpObjectID& GetPosition(){ return m_Positon;}
    virtual LwpVirtualLayout* GetContainerLayout() SAL_OVERRIDE;
};

class LwpPlacableLayout : public LwpLayout
{
public:
    LwpPlacableLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm );
    virtual ~LwpPlacableLayout();
    virtual sal_uInt8 GetWrapType();
    virtual LwpLayoutRelativity* GetRelativityPiece();
    virtual sal_uInt8 GetRelativeType() SAL_OVERRIDE;
    virtual sal_Int32 GetBaseLineOffset();
    virtual bool IsAnchorPage() SAL_OVERRIDE;
    virtual bool IsAnchorFrame() SAL_OVERRIDE;
    virtual bool IsAnchorCell() SAL_OVERRIDE;
    virtual void XFConvertFrame(XFContentContainer* /*pCont*/, sal_Int32 /*nStart*/ = 0, sal_Int32 /*nEnd*/ = 0, bool /*bAll*/ = false) {}
    /**
     * @descr:   Get font style for setting position of frame
     *
     */
    rtl::Reference<XFFont> GetFont() { return m_pFont; }
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
    void Read() SAL_OVERRIDE;
protected:
    sal_uInt8 m_nWrapType;
    sal_uInt8 m_nBuoyancy;
    sal_Int32 m_nBaseLineOffset;
    LwpAtomHolder m_Script;
    LwpObjectID m_LayRelativity;
    sal_uInt16 m_nPageNumber;//for frame anchored to page
    rtl::Reference<XFFont> m_pFont;//for frame position
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
