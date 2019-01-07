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

#include <memory>
#include "lwplayout.hxx"
#include "lwpusewhen.hxx"
#include <lwptools.hxx>
#include "lwplaypiece.hxx"
#include <xfilter/xfcolumns.hxx>
#include "lwpstory.hxx"
#include "lwpparastyle.hxx"
#include "lwpholder.hxx"
#include "lwpdoc.hxx"
#include "lwppagehint.hxx"
#include "lwpdivinfo.hxx"
#include "lwpgrfobj.hxx"
#include <osl/thread.h>
#include <sal/log.hxx>

LwpVirtualLayout::LwpVirtualLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpDLNFPVList(objHdr, pStrm)
    , m_bGettingHonorProtection(false)
    , m_bGettingMarginsSameAsParent(false)
    , m_bGettingHasProtection(false)
    , m_bGettingIsProtected(false)
    , m_bGettingIsAutoGrowDown(false)
    , m_bGettingMarginsValue(false)
    , m_bGettingExtMarginsValue(false)
    , m_bGettingUsePrinterSettings(false)
    , m_bGettingScaleCenter(false)
    , m_bGettingBorderStuff(false)
    , m_bGettingUseWhen(false)
    , m_bGettingStyleLayout(false)
    , m_nAttributes(0)
    , m_nAttributes2(0)
    , m_nAttributes3(0)
    , m_nOverrideFlag(0)
    , m_nDirection(0)
    , m_nEditorID(0)
{
}

void LwpVirtualLayout::Read()
{
    LwpDLNFPVList::Read();

    LwpObjectStream* pStrm = m_pObjStrm.get();
    m_nAttributes = pStrm->QuickReaduInt32();
    m_nAttributes2 = pStrm->QuickReaduInt32();
    m_nAttributes3 = pStrm->QuickReaduInt32();
    m_nOverrideFlag = pStrm->QuickReaduInt32();
    m_nDirection = pStrm->QuickReaduInt16();

    //Note that two bytes is read into m_nEditorID instead of one byte.
    m_nEditorID = pStrm->QuickReaduInt16();

    m_NextEnumerated.ReadIndexed(pStrm);
    m_PreviousEnumerated.ReadIndexed(pStrm);

    pStrm->SkipExtra();
}

bool LwpVirtualLayout::MarginsSameAsParent()
{
    return (m_nAttributes2 & STYLE2_MARGINSSAMEASPARENT) != 0;
}

/**
* @descr:   Get the gap between columns
*
*/
double LwpVirtualLayout::GetColGap(sal_uInt16 /*nIndex*/)
{
    //return DEFAULTGAPSIZE;
    //return LwpTools::ConvertToMetric(0.17);//DEFAULTGAPSIZE=0.17
    return LwpTools::ConvertToMetric(0.17);
}

/**
* @descr:   Whether it is honoring protection
*
*/
bool LwpVirtualLayout::HonorProtection()
{
    if(!(m_nAttributes2 & STYLE2_HONORPROTECTION))
        return false;

    rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
    if (xParent.is() && !xParent->IsHeader())
    {
        return xParent->GetHonorProtection();
    }

    if(m_pFoundry)//is null now
    {
        LwpDocument* pDoc = m_pFoundry->GetDocument();
        if(pDoc && pDoc->GetRootDocument())
            return pDoc->GetRootDocument()->GetHonorProtection();
    }

    return true;
}

/**
* @descr:   Whether it is protected
*
*/
bool LwpVirtualLayout::IsProtected()
{
    bool bProtected = (m_nAttributes & STYLE_PROTECTED)!=0;

    rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
    if (xParent.is() && !xParent->IsHeader())
    {
        if (xParent->GetHonorProtection() && (xParent->GetHasProtection()||bProtected))
        {
            return true;
        }
    }
    else if(m_pFoundry)//is null now
    {
        LwpDocument* pDoc = m_pFoundry->GetDocument();
        if(pDoc)
        {
            if (pDoc->GetHonorProtection() && bProtected)
            {
                return true;
            }
        }
    }

    return false;
}

/**
* @descr:   Whether it has protection
*
*/
bool LwpVirtualLayout::HasProtection()
{
    if(m_nAttributes & STYLE_PROTECTED)
        return true;

    rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
    if (xParent.is() && !xParent->IsHeader())
    {
        return xParent->GetHasProtection();
    }

    return false;
}

/**
* @descr:   Whether it is a mirror layout
*
*/
bool LwpVirtualLayout::IsComplex()
{
    return (m_nAttributes & STYLE_COMPLEX) != 0;
}

/**
* @descr:   Get usewhen pointer
*
*/
LwpUseWhen* LwpVirtualLayout::GetUseWhen()
{
    if (m_bGettingUseWhen)
        throw std::runtime_error("recursion in layout");
    m_bGettingUseWhen= true;

    LwpUseWhen* pRet = nullptr;

    /*
        If we have a parent, and I'm not a page layout,
        use my parents information.
    */
    if(GetLayoutType()!=LWP_PAGE_LAYOUT)
    {
        //get parent
        rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
        if (xParent.is() && !xParent->IsHeader() && (xParent->GetLayoutType() != LWP_PAGE_LAYOUT))
            pRet = xParent->GetUseWhen();

    }

    if (!pRet)
        pRet = VirtualGetUseWhen();

    m_bGettingUseWhen = false;

    return pRet;
}
/**
 * @descr:  Whether this layout is page layout or not
*/
bool LwpVirtualLayout::IsPage()
{
    return (GetLayoutType() == LWP_PAGE_LAYOUT);
}
/**
 * @descr:  Whether this layout is header layout or not
*/
bool LwpVirtualLayout::IsHeader()
{
    return (GetLayoutType() == LWP_HEADER_LAYOUT);
}
/**
 * @descr:  Whether this layout is footer layout or not
*/
bool LwpVirtualLayout::IsFooter()
{
    return (GetLayoutType() == LWP_FOOTER_LAYOUT);
}
/**
 * @descr:  Whether this layout is frame layout or not
*/
bool LwpVirtualLayout::IsFrame()
{
    return (GetLayoutType() == LWP_FRAME_LAYOUT);
}

/**
 * @descr:  Whether this layout is cell layout or not
*/
bool LwpVirtualLayout::IsCell()
{
    return (GetLayoutType() == LWP_CELL_LAYOUT
        || GetLayoutType() == LWP_CONNECTED_CELL_LAYOUT
        || GetLayoutType() == LWP_HIDDEN_CELL_LAYOUT);
}
/**
 * @descr:  Whether this layout is supertable layout or not
*/
bool LwpVirtualLayout::IsSuperTable()
{
    return (GetLayoutType() == LWP_SUPERTABLE_LAYOUT);
}
/**
 * @descr:  Whether this layout is group layout or not
*/
bool LwpVirtualLayout::IsGroupHead()
{
    return (GetLayoutType() == LWP_GROUP_LAYOUT);
}
/**
 * @descr:  get the relative type
*/
sal_uInt8 LwpVirtualLayout::GetRelativeType()
{
    return LwpLayoutRelativityGuts::LAY_PARENT_RELATIVE;
}
/**
 * @descr:  whether it is relative anchored layout
*/
bool LwpVirtualLayout::IsRelativeAnchored()
{
    sal_uInt8 nType;

    nType = GetRelativeType();
    return (nType == LwpLayoutRelativityGuts::LAY_PARA_RELATIVE) || (nType == LwpLayoutRelativityGuts::LAY_INLINE)
    || (nType == LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE) || (nType == LwpLayoutRelativityGuts::LAY_INLINE_VERTICAL);
}
/**
 * @descr:  whether it is MinimumHeight layout
*/
bool LwpVirtualLayout::IsMinimumHeight()
{
    return ((m_nAttributes3& STYLE3_MINHEIGHTVALID) != 0);
}

/**
* @descr:   Get parent layout
*
*/
rtl::Reference<LwpVirtualLayout> LwpVirtualLayout::GetParentLayout()
{
    return rtl::Reference<LwpVirtualLayout>(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
}

/**
* @descr:   Register child layout style
*
*/
void LwpVirtualLayout::RegisterChildStyle()
{
    //Register all children styles
    rtl::Reference<LwpVirtualLayout> xLayout(dynamic_cast<LwpVirtualLayout*>(GetChildHead().obj().get()));
    while (xLayout.is())
    {
        xLayout->SetFoundry(m_pFoundry);
        xLayout->DoRegisterStyle();
        xLayout.set(dynamic_cast<LwpVirtualLayout*>(xLayout->GetNext().obj().get()));
    }
}

bool LwpVirtualLayout::NoContentReference()
{
    return (m_nAttributes2 & STYLE2_NOCONTENTREFERENCE) != 0;
}

bool LwpVirtualLayout::IsStyleLayout()
{
    if (m_bGettingStyleLayout)
        throw std::runtime_error("recursion in layout");
    m_bGettingStyleLayout = true;

    bool bRet = false;
    if (m_nAttributes3 & STYLE3_STYLELAYOUT)
        bRet = true;
    else
    {
        rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
        if (xParent.is())
            bRet = xParent->IsStyleLayout();
    }

    m_bGettingStyleLayout = false;
    return bRet;
}

/**
* @descr:   Find child layout by layout type
*
*/
LwpVirtualLayout* LwpVirtualLayout::FindChildByType(LWP_LAYOUT_TYPE eType)
{
    LwpObjectID& rID = GetChildHead();
    LwpVirtualLayout* pPrevLayout = nullptr;

    while(!rID.IsNull())
    {
        LwpVirtualLayout * pLayout = dynamic_cast<LwpVirtualLayout *>(rID.obj().get());
        if (!pLayout)
            break;

        if (pPrevLayout && pLayout == pPrevLayout)
        {
            SAL_WARN("lwp", "loop in layout");
            break;
        }

        pPrevLayout = pLayout;

        if (pLayout->GetLayoutType() == eType)
            return pLayout;

        rID = pLayout->GetNext();
    }

    return nullptr;
}

/**
* @descr:   Whether the size of layout is fit the graphic
*
*/
bool LwpVirtualLayout::IsFitGraphic()
{
    return IsAutoGrowRight() && !IsAutoGrowLeft() && GetIsAutoGrowDown();
}

/**
* @descr:   Whether the width of layout is auto grow
*
*/
bool LwpVirtualLayout::IsAutoGrowWidth()
{
    return IsAutoGrowLeft() || IsAutoGrowRight();
}

/**
* @descr:   Determine whether the layout width is to margin
*
*/
bool LwpVirtualLayout::IsInlineToMargin()
{
    return (m_nAttributes3 & STYLE3_INLINETOMARGIN) != 0;
}

void LwpAssociatedLayouts::Read(LwpObjectStream* pStrm)
{
    m_OnlyLayout.ReadIndexed(pStrm);
    m_Layouts.Read(pStrm);
    pStrm->SkipExtra();
}

/**
* @descr:   Looking for the layout which follows the pStartLayout
* @param:   pStartLayout - the layout which is used for looking for its following layout
*/
rtl::Reference<LwpVirtualLayout> LwpAssociatedLayouts::GetLayout(LwpVirtualLayout const *pStartLayout)
{
    if (!pStartLayout && !m_OnlyLayout.IsNull())
        /* Looking for the first layout and there's only one layout in  the list.*/
        return rtl::Reference<LwpVirtualLayout>(dynamic_cast<LwpVirtualLayout*>(m_OnlyLayout.obj().get()));

    rtl::Reference<LwpObjectHolder> xObjHolder(dynamic_cast<LwpObjectHolder*>(m_Layouts.GetHead().obj().get()));
    if (xObjHolder.is())
    {
        rtl::Reference<LwpVirtualLayout> xLayout(dynamic_cast<LwpVirtualLayout*>(xObjHolder->GetObject().obj().get()));
        if (!pStartLayout)
            return xLayout;

        while (xObjHolder.is() && pStartLayout != xLayout.get())
        {
            xObjHolder.set(dynamic_cast<LwpObjectHolder*>(xObjHolder->GetNext().obj().get()));
            if (xObjHolder.is())
            {
                xLayout.set(dynamic_cast<LwpVirtualLayout*>(xObjHolder->GetObject().obj().get()));
            }
        }

        if (xObjHolder.is())
        {
            xObjHolder.set(dynamic_cast<LwpObjectHolder*>(xObjHolder->GetNext().obj().get()));
            if (xObjHolder.is())
            {
                xLayout.set(dynamic_cast<LwpVirtualLayout*>(xObjHolder->GetObject().obj().get()));
                return xLayout;
            }
        }
    }

    return rtl::Reference<LwpVirtualLayout>();
}

LwpHeadLayout::LwpHeadLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpVirtualLayout(objHdr, pStrm)
{}

void LwpHeadLayout::Read()
{
    LwpVirtualLayout::Read();
    //For PermissiveLayout
    m_pObjStrm->SkipExtra();
    //For me
    m_pObjStrm->SkipExtra();
}

void LwpHeadLayout::RegisterStyle()
{
    //Register all children styles
    rtl::Reference<LwpVirtualLayout> xLayout(dynamic_cast<LwpVirtualLayout*>(GetChildHead().obj().get()));
    std::set<LwpVirtualLayout*> aSeen;
    while (xLayout.is())
    {
        aSeen.insert(xLayout.get());
        xLayout->SetFoundry(m_pFoundry);
        //if the layout is relative to para, the layout will be registered in para
        if (!xLayout->IsRelativeAnchored())
            xLayout->DoRegisterStyle();
        rtl::Reference<LwpVirtualLayout> xNext(dynamic_cast<LwpVirtualLayout*>(xLayout->GetNext().obj().get()));
        if (aSeen.find(xNext.get()) != aSeen.end())
            throw std::runtime_error("loop in conversion");
        xLayout = xNext;
    }
}

/**
 * @descr   find endnote supertable layout from the child layout list. Suppose that there is only one endnote supertablelayout in one division
 * @return pointer to endnote supertable layout
 */
rtl::Reference<LwpVirtualLayout> LwpHeadLayout::FindEnSuperTableLayout()
{
    rtl::Reference<LwpVirtualLayout> xLayout(dynamic_cast<LwpVirtualLayout*>(GetChildHead().obj().get()));
    std::set<LwpVirtualLayout*> aSeen;
    while (xLayout.get())
    {
        aSeen.insert(xLayout.get());
        if (xLayout->GetLayoutType() == LWP_ENDNOTE_SUPERTABLE_LAYOUT)
        {
            return xLayout;
        }
        xLayout.set(dynamic_cast<LwpVirtualLayout*>(xLayout->GetNext().obj().get()));
        if (aSeen.find(xLayout.get()) != aSeen.end())
            throw std::runtime_error("loop in conversion");
    }
    return rtl::Reference<LwpVirtualLayout>();
}

LwpLayoutStyle::LwpLayoutStyle()
    : m_nStyleDefinition(0)
    , m_pDescription(new LwpAtomHolder)
    , m_nKey(0)
{
}

LwpLayoutStyle::~LwpLayoutStyle()
{
}

void LwpLayoutStyle::Read(LwpObjectStream* pStrm)
{
    m_nStyleDefinition = pStrm->QuickReaduInt32();
    m_pDescription->Read(pStrm);
    if (pStrm->CheckExtra())
    {
        m_nKey = pStrm->QuickReaduInt16();
        pStrm->SkipExtra();
    }
}

LwpLayoutMisc::LwpLayoutMisc() :
m_nGridDistance(0), m_nGridType(0)
{
}

LwpLayoutMisc::~LwpLayoutMisc()
{
}

void LwpLayoutMisc::Read(LwpObjectStream* pStrm)
{
    m_nGridType = pStrm->QuickReaduInt16();
    m_nGridDistance = pStrm->QuickReadInt32();
    m_aContentStyle.Read(pStrm);
    pStrm->SkipExtra();
}

LwpMiddleLayout::LwpMiddleLayout( LwpObjectHeader const &objHdr, LwpSvStream* pStrm )
    : LwpVirtualLayout(objHdr, pStrm)
    , m_bGettingGeometry(false)
    , m_bGettingBackgroundStuff(false)
{
}

LwpMiddleLayout::~LwpMiddleLayout()
{
}

void LwpMiddleLayout::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm.get();

    LwpVirtualLayout::Read();

    //skip CLiteLayout data;
    LwpAtomHolder ContentClass;
    ContentClass.Read(pStrm);
    pStrm->SkipExtra();

    // before layout hierarchy rework
    if(LwpFileHeader::m_nFileRevision < 0x000B)
        return;

    m_Content.ReadIndexed(pStrm);

    // 01/20/2005
    m_BasedOnStyle.ReadIndexed(pStrm);
    m_TabPiece.ReadIndexed(pStrm);

    sal_uInt8 nWhatsItGot = pStrm->QuickReaduInt8();

    if (nWhatsItGot & DISK_GOT_STYLE_STUFF)
    {
        m_aStyleStuff.Read(pStrm);
    }
    if (nWhatsItGot & DISK_GOT_MISC_STUFF)
    {
        m_aMiscStuff.Read(pStrm);
    }

    m_LayGeometry.ReadIndexed(pStrm);
    m_LayScale.ReadIndexed(pStrm);
    m_LayMargins.ReadIndexed(pStrm);
    m_LayBorderStuff.ReadIndexed(pStrm);
    m_LayBackgroundStuff.ReadIndexed(pStrm);

    if (pStrm->CheckExtra())
    {
        m_LayExtBorderStuff.ReadIndexed(pStrm);
        pStrm->SkipExtra();
    }
}

rtl::Reference<LwpObject> LwpMiddleLayout::GetBasedOnStyle()
{
    rtl::Reference<LwpObject> xRet(m_BasedOnStyle.obj());
    if (xRet.get() == this)
    {
        SAL_WARN("lwp", "style based on itself");
        return rtl::Reference<LwpObject>();
    }
    return xRet;
}

/**
* @descr:   Get the geometry of current layout
*
*/
LwpLayoutGeometry* LwpMiddleLayout::GetGeometry()
{
    if (m_bGettingGeometry)
        throw std::runtime_error("recursion in layout");
    m_bGettingGeometry = true;

    LwpLayoutGeometry* pRet = nullptr;
    if( !m_LayGeometry.IsNull() )
    {
        pRet = dynamic_cast<LwpLayoutGeometry*> (m_LayGeometry.obj().get());
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            pRet = pLay->GetGeometry();
        }
    }

    m_bGettingGeometry = false;
    return pRet;
}

/**
* @descr:   Get layout height, measured by "cm"
*
*/
double LwpMiddleLayout::GetGeometryHeight()
{
    LwpLayoutGeometry* pGeo = GetGeometry();
    if(pGeo)
    {
        return LwpTools::ConvertFromUnitsToMetric( pGeo->GetHeight() );
    }
    else
        return -1;
}

/**
* @descr:   Get layout width, measured by "cm"
*
*/
double LwpMiddleLayout::GetGeometryWidth()
{
    LwpLayoutGeometry* pGeo = GetGeometry();
    if(pGeo)
    {
        return LwpTools::ConvertFromUnitsToMetric( pGeo->GetWidth() );
    }
    else
        return -1;
}

/**
* @descr:   Whether the margins is same as parent layout
*
*/
bool LwpMiddleLayout::MarginsSameAsParent()
{
    if(m_nOverrideFlag & OVER_MARGINS)
    {
        return LwpVirtualLayout::MarginsSameAsParent();
    }
    rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
    if (LwpVirtualLayout* pLay = dynamic_cast<LwpVirtualLayout*>(xBase.get()))
    {
        pLay->GetMarginsSameAsParent();
    }
    return LwpVirtualLayout::MarginsSameAsParent();
}

/**
* @descr:   Get margin
* @param:   nWhichSide - 0: left, 1: right, 2:top, 3: bottom
*/
double LwpMiddleLayout::MarginsValue(sal_uInt8 nWhichSide)
{
    double fValue = 0;
    if((nWhichSide==MARGIN_LEFT)||(nWhichSide==MARGIN_RIGHT))
    {
        if ( GetMarginsSameAsParent() )
        {
            rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
            if (xParent.is() && !xParent->IsHeader())
            {
                fValue = xParent->GetMarginsValue(nWhichSide);
                return fValue;
            }
        }
    }

    if(m_nOverrideFlag & OVER_MARGINS)
    {
        LwpLayoutMargins* pMar1 = dynamic_cast<LwpLayoutMargins*> (m_LayMargins.obj().get());
        if(pMar1)
        {
            fValue = pMar1->GetMargins().GetMarginsValue(nWhichSide);
            return fValue;
        }
    }
    rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
    LwpVirtualLayout* pStyle = dynamic_cast<LwpVirtualLayout*>(xBase.get());
    if (pStyle)
    {
        fValue = pStyle->GetMarginsValue(nWhichSide);
        return fValue;
    }
    return LwpVirtualLayout::MarginsValue(nWhichSide);
}
/**
 * @descr:  Get extmargin value
 * @param:  nWhichSide - 0: left, 1: right, 2:top, 3: bottom
 * @param:
 * @return:
*/
double LwpMiddleLayout::ExtMarginsValue(sal_uInt8 nWhichSide)
{
    double fValue = 0;
    if(m_nOverrideFlag & OVER_MARGINS)
    {
        LwpLayoutMargins* pMar1 = dynamic_cast<LwpLayoutMargins*> (m_LayMargins.obj().get());
        if(pMar1)
        {
            fValue = pMar1->GetExtMargins().GetMarginsValue(nWhichSide);
            return fValue;
        }
    }
    LwpVirtualLayout* pStyle = dynamic_cast<LwpVirtualLayout*>(GetBasedOnStyle().get());
    if(pStyle)
    {
        fValue = pStyle->GetExtMarginsValue(nWhichSide);
        return fValue;
    }
    return LwpVirtualLayout::ExtMarginsValue(nWhichSide);
}
/**
 * @descr:  Get the LwpBorderStuff object according to m_LayBorderStuff id.
*/
LwpBorderStuff* LwpMiddleLayout::GetBorderStuff()
{
    if (m_bGettingBorderStuff)
        throw std::runtime_error("recursion in layout");
    m_bGettingBorderStuff = true;

    LwpBorderStuff* pRet = nullptr;

    if(m_nOverrideFlag & OVER_BORDERS)
    {
        LwpLayoutBorder* pLayoutBorder = dynamic_cast<LwpLayoutBorder*>(m_LayBorderStuff.obj().get());
        pRet = pLayoutBorder ? &pLayoutBorder->GetBorderStuff() : nullptr;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            pRet = pLay->GetBorderStuff();
        }
    }

    m_bGettingBorderStuff= false;
    return pRet;
}

/**
 * @descr:  Get LwpBackgroundStuff object according to m_LayBackgroundStuff id;
*/
LwpBackgroundStuff* LwpMiddleLayout::GetBackgroundStuff()
{
    if (m_bGettingBackgroundStuff)
        throw std::runtime_error("recursion in layout");
    m_bGettingBackgroundStuff = true;

    LwpBackgroundStuff* pRet = nullptr;

    if(m_nOverrideFlag & OVER_BACKGROUND)
    {
        LwpLayoutBackground* pLayoutBackground = dynamic_cast<LwpLayoutBackground*>(m_LayBackgroundStuff.obj().get());
        pRet = pLayoutBackground ? &pLayoutBackground->GetBackgoudStuff() : nullptr;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            pRet = pLay->GetBackgroundStuff();
        }
    }

    m_bGettingBackgroundStuff = false;
    return pRet;
}

/**
 * @descr:  create xfborder.
*/
std::unique_ptr<XFBorders> LwpMiddleLayout::GetXFBorders()
{
    LwpBorderStuff* pBorderStuff = GetBorderStuff();
    if(pBorderStuff&&pBorderStuff->GetSide() != 0)
    {
        //copy from lwpparastyle.
        std::unique_ptr<XFBorders> xXFBorders(new XFBorders);
        // apply 4 borders respectively
        LwpBorderStuff::BorderType const pType[] = { LwpBorderStuff::LEFT, LwpBorderStuff::RIGHT,
            LwpBorderStuff::TOP, LwpBorderStuff::BOTTOM };

        for (LwpBorderStuff::BorderType nC : pType)
        {
            if (pBorderStuff->HasSide(nC))
            {
                LwpParaStyle::ApplySubBorder(pBorderStuff, nC, xXFBorders.get());
            }
        }
        return xXFBorders;
    }
    return nullptr;
}

/**
 * @descr:  Get text direction settings.
*/
enumXFTextDir LwpMiddleLayout::GetTextDirection()
{
    enumXFTextDir eTextDir = enumXFTextDirNone;
    sal_uInt8 nDirection = GetContentOrientation();
    switch(nDirection)
    {
        case TEXT_ORIENT_LRTB:
        {
            eTextDir = enumXFTextDirLR_TB;
            break;
        }
        case TEXT_ORIENT_TBRL:
        {
            eTextDir = enumXFTextDirTB_RL;
            break;
        }
        case TEXT_ORIENT_RLBT: // not supported now
        {
            eTextDir = enumXFTextDirNone;
            break;
        }
        case TEXT_ORIENT_BTLR: // not supported now
        {
            eTextDir = enumXFTextDirNone;
            break;
        }
        default:
            break;
    }
    return eTextDir;
}
/**
 * @descr: Get back ground color.
*/
LwpColor* LwpMiddleLayout::GetBackColor()
{
    LwpBackgroundStuff* pBackgroundStuff = GetBackgroundStuff();
    if(pBackgroundStuff && !pBackgroundStuff->IsTransparent())
    {
        LwpColor* pColor = pBackgroundStuff->GetFillColor();
        if(pColor->IsValidColor())
        {
            return pColor;
        }
    }
    return nullptr;
}

/**
 * @descr:  Add back color settings into xfpagemaster.
*/
LwpTabOverride* LwpMiddleLayout::GetTabOverride()
{
    if(m_nAttributes & OVER_TABS)
    {
        if(!m_TabPiece.IsNull())
        {
            LwpTabPiece *pPiece = dynamic_cast<LwpTabPiece*>(m_TabPiece.obj().get());
            return static_cast<LwpTabOverride*>(pPiece ? pPiece->GetOverride() : nullptr);
        }
        return nullptr;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            return pLay->GetTabOverride();
        }
    }
    return nullptr;
}

/**
 * @descr:   Layscale for graphic & watermark
*/
sal_uInt16 LwpMiddleLayout::GetScaleMode()
{
    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj().is() && GetLayoutScale())
        return GetLayoutScale()->GetScaleMode();
    rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
    if (xBase.is())
        return dynamic_cast<LwpMiddleLayout&>(*xBase).GetScaleMode();
    else
        return (LwpLayoutScale::FIT_IN_FRAME | LwpLayoutScale::MAINTAIN_ASPECT_RATIO);
}

sal_uInt16 LwpMiddleLayout::GetScaleTile()
{
    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj().is() && GetLayoutScale())
        return (GetLayoutScale()->GetPlacement() & LwpLayoutScale::TILED)
            ? 1 : 0;
    rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
    if (xBase.is())
        return dynamic_cast<LwpMiddleLayout&>(*xBase).GetScaleTile();
    else
        return 0;
}

sal_uInt16 LwpMiddleLayout::GetScaleCenter()
{
    if (m_bGettingScaleCenter)
        throw std::runtime_error("recursion in layout");
    m_bGettingScaleCenter = true;

    sal_uInt16 nRet = 0;

    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj().is() && GetLayoutScale())
    {
        nRet = (GetLayoutScale()->GetPlacement() & LwpLayoutScale::CENTERED)
            ? 1 : 0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (xBase.is())
            nRet = dynamic_cast<LwpMiddleLayout&>(*xBase).GetScaleCenter();
    }

    m_bGettingScaleCenter = false;
    return nRet;
}

bool LwpMiddleLayout::CanSizeRight()
{
    sal_uInt8 RelType = GetRelativeType();

    return !(RelType == LwpLayoutRelativityGuts::LAY_INLINE || RelType == LwpLayoutRelativityGuts::LAY_PARA_RELATIVE
             || RelType == LwpLayoutRelativityGuts::LAY_INLINE_VERTICAL);
}
sal_Int32 LwpMiddleLayout::GetMinimumWidth()
{
    if (((m_nAttributes3 & STYLE3_WIDTHVALID) != 0) && GetGeometry())
    {
        return GetGeometry()->GetWidth();
    }
    else if (m_nOverrideFlag & OVER_SIZE)
    {
        return DetermineWidth();
    }
    return 0;
}
sal_Int32 LwpMiddleLayout::DetermineWidth()
{
    if (IsSizeRightToContent())
    {
        assert(false);
    }
    else if (IsSizeRightToContainer())
    {
        assert(false);
    }
    else if (LwpLayoutGeometry* pGeo = GetGeometry())
    {
        m_nAttributes3 |= STYLE3_WIDTHVALID;
        return pGeo->GetWidth();
    }
    return 0;
}
bool LwpMiddleLayout::IsSizeRightToContainer()
{
    if (!CanSizeRight())
        return false;

    if (m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection & ((LAY_USEDIRECTION|LAY_AUTOSIZE|LAY_TOCONTAINER)
                        << SHIFT_RIGHT))
        == ((LAY_USEDIRECTION | LAY_TOCONTAINER | LAY_AUTOSIZE) << SHIFT_RIGHT);
    }
    rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
    if (xBase.is())
    {
        LwpMiddleLayout * pLayout = dynamic_cast<LwpMiddleLayout *>(xBase.get());
        return pLayout && pLayout->IsSizeRightToContainer();
    }
    else
        return false;
}
bool LwpMiddleLayout::IsSizeRightToContent()
{
    if (!CanSizeRight())
        return false;

    if (m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection & ((LAY_USEDIRECTION|LAY_AUTOSIZE|LAY_TOCONTAINER)
                            << SHIFT_RIGHT))
                == ((LAY_USEDIRECTION | LAY_AUTOSIZE) << SHIFT_RIGHT);
    }
    rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
    if (xBase.is())
    {
        LwpMiddleLayout * pLayout = dynamic_cast<LwpMiddleLayout *>(xBase.get());
        return pLayout && pLayout->IsSizeRightToContent();
    }
    else
        return false;
}

/**
* @descr:   Get layout height
*
*/
double LwpMiddleLayout::GetHeight()
{
    return GetGeometryHeight();
}

/**
* @descr:   Get layout height
*
*/
double LwpMiddleLayout::GetWidth()
{
    return GetGeometryWidth();
}
/**
* @descr:   Get layout origin point
*
*/
LwpPoint LwpMiddleLayout::GetOrigin()
{
    LwpLayoutGeometry* pGeo = GetGeometry();
    if(pGeo)
    {
        sal_uInt8 nType = GetRelativeType();
        if(nType == LwpLayoutRelativityGuts::LAY_INLINE
            || nType == LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE )
        {
            return pGeo->GetAbsoluteOrigin();
        }
        else
            return pGeo->GetOrigin();

    }

    return LwpPoint();
}

/**
* @descr:   Whether the fill is pattern fill or not
* @return:  True if yes, false if not.
*/
bool LwpMiddleLayout::IsPatternFill()
{
    LwpBackgroundStuff* pBackgroundStuff = GetBackgroundStuff();
    if (pBackgroundStuff)
    {
        return pBackgroundStuff->IsPatternFill();
    }

    return false;
}

/**
* @descr:   Get the fill pattern style. Data are saved in a XFBGImage object
* @return:  the fill pattern style.
*/
std::unique_ptr<XFBGImage> LwpMiddleLayout::GetFillPattern()
{
    LwpBackgroundStuff* pBackgroundStuff = GetBackgroundStuff();
    if (pBackgroundStuff)
    {
        return pBackgroundStuff->GetFillPattern();
    }

    return std::unique_ptr<XFBGImage>();
}

/**
* @descr:   Whether the height and width of layout is auto grow
*
*/
bool LwpMiddleLayout::IsAutoGrow()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection &
            ((LAY_AUTOGROW << SHIFT_UP) | (LAY_AUTOGROW << SHIFT_DOWN) |
            (LAY_AUTOGROW << SHIFT_RIGHT) | (LAY_AUTOGROW << SHIFT_LEFT))) != 0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            return pLay->IsAutoGrow();
        }
    }
    return LwpVirtualLayout::IsAutoGrow();
}

/**
* @descr:   Whether the height of layout is auto grow down
*
*/
bool LwpMiddleLayout::IsAutoGrowDown()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection & (LAY_AUTOGROW << SHIFT_DOWN)) != 0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            return pLay->GetIsAutoGrowDown();
        }
    }
    return LwpVirtualLayout::IsAutoGrowDown();
}

/**
* @descr:   Whether the height of layout is auto grow up
*
*/
bool LwpMiddleLayout::IsAutoGrowUp()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection & (LAY_AUTOGROW << SHIFT_UP)) != 0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            return pLay->IsAutoGrowUp();
        }
    }
    return LwpVirtualLayout::IsAutoGrowUp();
}

/**
* @descr:   Whether the height of layout is auto grow down
*
*/
bool LwpMiddleLayout::IsAutoGrowLeft()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection & (LAY_AUTOGROW << SHIFT_LEFT)) != 0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            return pLay->IsAutoGrowLeft();
        }
    }
    return LwpVirtualLayout::IsAutoGrowLeft();
}

/**
* @descr:   Whether the height of layout is auto grow down
*
*/
bool LwpMiddleLayout::IsAutoGrowRight()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection & (LAY_AUTOGROW << SHIFT_RIGHT)) != 0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            return pLay->IsAutoGrowRight();
        }
    }
    return LwpVirtualLayout::IsAutoGrowRight();
}

/**
* @descr:   Get contents orientation
*
*/
sal_uInt8 LwpMiddleLayout::GetContentOrientation()
{
    //content orientation in Graphic objects and OLE objects not supported now
    if((m_nOverrideFlag & OVER_ROTATION)&& !m_LayGeometry.IsNull())
    {
        LwpLayoutGeometry* pLayGeometry = dynamic_cast<LwpLayoutGeometry*> (m_LayGeometry.obj().get());
        if (pLayGeometry)
            return pLayGeometry->GetContentOrientation();
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            return pLay->GetContentOrientation();
        }
    }
    return LwpVirtualLayout::GetContentOrientation();
}

/**
* @descr:   Whether it is honoring protection
*
*/
bool LwpMiddleLayout::HonorProtection()
{
    if(m_nOverrideFlag & OVER_MISC)
    {
        if(!(m_nAttributes2 & STYLE2_HONORPROTECTION))
            return false;

        rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
        if (xParent.is() && !xParent->IsHeader())
        {
            return xParent->GetHonorProtection();
        }

        if(m_pFoundry)//is null now
        {
            LwpDocument* pDoc = m_pFoundry->GetDocument();
            if (pDoc)
            {
                return pDoc->GetHonorProtection();
            }
        }
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            return pLay->GetHonorProtection();
        }
    }

    return LwpVirtualLayout::HonorProtection();
}

/**
* @descr:   Whether it is protected
*
*/
bool LwpMiddleLayout::IsProtected()
{
    bool bProtected = false;
    if(m_nOverrideFlag & OVER_MISC)
    {
        bProtected = (m_nAttributes & STYLE_PROTECTED)!=0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            bProtected = pLay->GetIsProtected();
        }
        else
            bProtected = LwpVirtualLayout::IsProtected();
    }

    rtl::Reference<LwpVirtualLayout> xParent(dynamic_cast<LwpVirtualLayout*>(GetParent().obj().get()));
    if (xParent.is() && !xParent->IsHeader())
    {
        /* If a parent's protected then none of its children can be accessed. */
        if (xParent->GetIsProtected())
            return true;

        if (xParent->GetHonorProtection())
            return bProtected;

        /* If our parent isn't honoring protection then we aren't protected. */
        return false;

    }
    if(m_pFoundry)//is null now
    {
        LwpDocument* pDoc = m_pFoundry->GetDocument();
        if(pDoc)
        {
            if (pDoc->GetHonorProtection())
                return bProtected;

            /* If the document isn't honoring protection then we aren't protected.*/
            return false;
        }
    }

    return bProtected;
}

/**
* @descr:   Get watermark layout
*
*/
rtl::Reference<LwpVirtualLayout> LwpMiddleLayout::GetWaterMarkLayout()
{
    rtl::Reference<LwpVirtualLayout> xLay(dynamic_cast<LwpVirtualLayout*>(GetChildHead().obj().get()));
    std::set<LwpVirtualLayout*> aSeen;
    while (xLay.is())
    {
        aSeen.insert(xLay.get());
        if (xLay->IsForWaterMark())
        {
            return xLay;
        }
        rtl::Reference<LwpVirtualLayout> xNext(dynamic_cast<LwpVirtualLayout*>(xLay->GetNext().obj().get()));
        if (aSeen.find(xNext.get()) != aSeen.end())
            throw std::runtime_error("loop in conversion");
        xLay = xNext;
    }
    return rtl::Reference<LwpVirtualLayout>();
}

/**
* @descr:   Create and return xfbgimage object for watermark
*
*/
std::unique_ptr<XFBGImage> LwpMiddleLayout::GetXFBGImage()
{
    std::unique_ptr<XFBGImage> xXFBGImage;

    rtl::Reference<LwpVirtualLayout> xWaterMarkLayout(GetWaterMarkLayout());
    LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xWaterMarkLayout.get());
    if(pLay)
    {
        //test BGImage
        LwpGraphicObject* pGrfObj = dynamic_cast<LwpGraphicObject*>(pLay->GetContent().obj().get());
        if(pGrfObj)
        {
            xXFBGImage.reset(new XFBGImage);
            if(pGrfObj->IsLinked())
            {
                //set file link
                OUString linkedfilepath = pGrfObj->GetLinkedFilePath();
                OUString fileURL = LwpTools::convertToFileUrl(OUStringToOString(linkedfilepath, osl_getThreadTextEncoding()));
                xXFBGImage->SetFileLink(fileURL);
            }
            else
            {
                std::vector<sal_uInt8> aGrafData = pGrfObj->GetRawGrafData();
                xXFBGImage->SetImageData(aGrafData.data(), aGrafData.size());
            }

            //automatic, top left
            xXFBGImage->SetPosition(enumXFAlignStart,enumXFAlignTop);
            if(pLay->GetScaleCenter())
            {
                //center
                xXFBGImage->SetPosition();
            }
            else if(pLay->GetScaleTile())
            {
                //tile
                xXFBGImage->SetRepeate();
            }
            //fit type, area type
            if((pLay->GetScaleMode()& LwpLayoutScale::FIT_IN_FRAME)!=0)
            {
                if((pLay->GetScaleMode()& LwpLayoutScale::MAINTAIN_ASPECT_RATIO)==0)
                {
                    xXFBGImage->SetStretch();
                }
            }
        }
    }
    return xXFBGImage;
}

/**
* @descr:   Whether the page uses the printer setting
*
*/
bool LwpMiddleLayout::GetUsePrinterSettings()
{
    if (m_bGettingUsePrinterSettings)
        throw std::runtime_error("recursion in layout");
    m_bGettingUsePrinterSettings = true;

    bool bRet = false;

    if(m_nOverrideFlag & OVER_SIZE)
    {
        bRet = (m_nAttributes3 & STYLE3_USEPRINTERSETTINGS) != 0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xBase.get()))
        {
            bRet = pLay->GetUsePrinterSettings();
        }
    }

    m_bGettingUsePrinterSettings = false;
    return bRet;
}

//Check whether there are contents in the layout
bool LwpMiddleLayout::HasContent()
{
    rtl::Reference<LwpObject> content = m_Content.obj();
    return content.is();
}

LwpLayout::LwpLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpMiddleLayout(objHdr, pStrm)
    , m_bGettingShadow(false)
    , m_bGettingNumCols(false)
{
}

LwpLayout::~LwpLayout()
{
}

void LwpLayout::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm.get();

    LwpMiddleLayout::Read();
    if (LwpFileHeader::m_nFileRevision < 0x000B)
    {
        // read PreRevBLayout...
    }
    else
    {
        sal_uInt16 nSimple = pStrm->QuickReaduInt16();

        if (!nSimple)
        {
            m_aUseWhen.Read(pStrm);

            sal_uInt8 nFlag = pStrm->QuickReaduInt8();
            if (nFlag)
                m_Position.ReadIndexed(pStrm);
        }

        m_LayColumns.ReadIndexed(pStrm);
        m_LayGutterStuff.ReadIndexed(pStrm);
        m_LayJoinStuff.ReadIndexed(pStrm);
        m_LayShadow.ReadIndexed(pStrm);

        if (pStrm->CheckExtra())
        {
            m_LayExtJoinStuff.ReadIndexed(pStrm);
            pStrm->SkipExtra();
        }
    }
}

/**
* @descr:   Get columns number
*
*/
sal_uInt16 LwpLayout::GetNumCols()
{
    if (m_bGettingNumCols)
        throw std::runtime_error("recursion in layout");
    m_bGettingNumCols = true;

    sal_uInt16 nRet = 0;
    LwpLayoutColumns* pLayColumns = (m_nOverrideFlag & OVER_COLUMNS) ? dynamic_cast<LwpLayoutColumns*>(m_LayColumns.obj().get()) : nullptr;
    if (pLayColumns)
    {
        nRet = pLayColumns->GetNumCols();
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        LwpVirtualLayout* pStyle = dynamic_cast<LwpVirtualLayout*>(xBase.get());
        nRet = pStyle ? pStyle->GetNumCols() : LwpVirtualLayout::GetNumCols();
    }
    m_bGettingNumCols = false;
    return nRet;
}


/**
* @descr:   Get gap between columns
* @param:   the order of column
*/
double LwpLayout::GetColGap(sal_uInt16 nIndex)
{
    if((m_nOverrideFlag & OVER_COLUMNS)||(m_nAttributes2 & STYLE2_LOCALCOLUMNINFO))
    {
        LwpLayoutColumns* pLayColumns = dynamic_cast<LwpLayoutColumns*>(m_LayColumns.obj().get());
        if(pLayColumns)
        {
            return pLayColumns->GetColGap(nIndex);
        }
    }

    rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
    LwpVirtualLayout* pStyle = dynamic_cast<LwpVirtualLayout*>(xBase.get());
    if (pStyle)
    {
        return pStyle->GetColGap(nIndex);
    }

    return LwpVirtualLayout::GetColGap(nIndex);
}

/**
* @descr:   Create and return XFColumns object
*
*/
XFColumns* LwpLayout::GetXFColumns()
{
    //if there is only one column, do not need insert columns
    sal_uInt16 nCols = GetNumCols();
    if(nCols==1)
    {
        return nullptr;
    }

    std::unique_ptr<XFColumns> xColumns(new XFColumns);
    //set XFColumnSep
    std::unique_ptr<XFColumnSep> xColumnSep(GetColumnSep());
    if (xColumnSep)
    {
        xColumns->SetSeparator(*xColumnSep);
    }

    //set column count and column gap
    xColumns->SetCount(nCols);
    double fGap = GetColGap(0);
    xColumns->SetGap(fGap);

    //set xfcolumn
    for(sal_uInt16 nIndex = 0; nIndex<nCols; nIndex++)
    {
        XFColumn aColumn;
        sal_Int32 nWidth = 8305/nCols;  //relative width
        aColumn.SetRelWidth(nWidth);

        //the left and right margins is 0;
        double nGap = GetColGap(nIndex)/2;
        aColumn.SetMargins(nGap,nGap);
        if(nIndex==0)
        {
            aColumn.SetMargins(0,nGap);
        }
        if(nIndex==(nCols-1))
        {
            aColumn.SetMargins(nGap,0);
        }
        xColumns->AddColumn(aColumn);
    }

    return xColumns.release();
}

/**
* @descr:   Create and return XFColumnSep object
*
*/
XFColumnSep* LwpLayout::GetColumnSep()
{

    //Get LwpLayoutGutters
    LwpLayoutGutters* pLayoutGutters = dynamic_cast<LwpLayoutGutters*>(m_LayGutterStuff.obj().get());
    if(!pLayoutGutters)
    {
        return nullptr;
    }

    LwpBorderStuff& rBorderStuff = pLayoutGutters->GetBorderStuff();

    LwpBorderStuff::BorderType eType = LwpBorderStuff::LEFT;
    LwpColor    aColor = rBorderStuff.GetSideColor(eType);
    double  fWidth = rBorderStuff.GetSideWidth(eType);
    //sal_uInt16    nType = rBorderStuff->GetSideType(eType);

    XFColumnSep* pColumnSep = new XFColumnSep();
    XFColor aXFColor(aColor.To24Color());
    pColumnSep->SetColor(aXFColor);
    pColumnSep->SetWidth(fWidth);
    pColumnSep->SetRelHeight(100);
    pColumnSep->SetVerticalAlign(enumXFAlignTop);

    return pColumnSep;
}

/**
* @descr:   Get use when type
*
*/
LwpLayout::UseWhenType LwpLayout::GetUseWhenType()
{
    UseWhenType eType = StartWithinPage;
    LwpUseWhen* pUseWhen = GetUseWhen();
    if(pUseWhen)
    {
        if(pUseWhen->IsStartOnThisHF())
        {
            eType =  StartWithinColume;
        }
        else if(pUseWhen->IsStartOnThisPage())
        {
            eType =  StartWithinPage;
        }
        else if(pUseWhen->IsStartOnNextPage())
        {
            eType = StartOnNextPage;
        }
        else if(pUseWhen->IsStartOnNextOddPage())
        {
            eType =  StartOnOddPage;
        }
        else if(pUseWhen->IsStartOnNextEvenPage())
        {
            eType =   StartOnEvenPage;
        }

    }
    else
    {
        eType =   StartOnNextPage;
    }
    return eType;
}

/**
* @descr:   Get use page
*
*/
sal_uInt16 LwpLayout::GetUsePage()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->GetUsePage();
        else
            return 0;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpLayout* pLay = dynamic_cast<LwpLayout*>(xBase.get()))
        {
            return pLay->GetUsePage();
        }
    }
    return 0;
}

/**
* @descr:   Get usewhen pointer
*
*/
LwpUseWhen* LwpLayout::VirtualGetUseWhen()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        return &m_aUseWhen;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpLayout* pLay = dynamic_cast<LwpLayout*>(xBase.get()))
        {
            return pLay->VirtualGetUseWhen();
        }
    }
    return LwpVirtualLayout::VirtualGetUseWhen();
}

/**
* @descr:   Whether it is use on all pages
*
*/
bool LwpLayout::IsUseOnAllPages()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->IsUseOnAllPages();
        else
            return false;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpLayout* pLay = dynamic_cast<LwpLayout*>(xBase.get()))
        {
            return pLay->IsUseOnAllPages();
        }
    }
    return LwpVirtualLayout::IsUseOnAllPages();
}

/**
* @descr:   Whether it is use on all even pages
*
*/
bool LwpLayout::IsUseOnAllEvenPages()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->IsUseOnAllEvenPages();
        else
            return false;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpLayout* pLay = dynamic_cast<LwpLayout*>(xBase.get()))
        {
            return pLay->IsUseOnAllEvenPages();
        }
    }
    return LwpVirtualLayout::IsUseOnAllEvenPages();
}

/**
* @descr:   Whether it is use on all odd pages
*
*/
bool LwpLayout::IsUseOnAllOddPages()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->IsUseOnAllOddPages();
        else
            return false;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpLayout* pLay = dynamic_cast<LwpLayout*>(xBase.get()))
        {
            return pLay->IsUseOnAllOddPages();
        }
    }
    return LwpVirtualLayout::IsUseOnAllOddPages();
}

/**
* @descr:   Whether it is use on current page
*
*/
bool LwpLayout::IsUseOnPage()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->IsUseOnPage();
        else
            return false;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpLayout* pLay = dynamic_cast<LwpLayout*>(xBase.get()))
        {
            return pLay->IsUseOnPage();
        }
    }
    return LwpVirtualLayout::IsUseOnPage();
}

/**
 * @descr:  Get the LwpShadow object according to m_LayShadow id.
*/
LwpShadow* LwpLayout::GetShadow()
{
    if (m_bGettingShadow)
        throw std::runtime_error("recursion in layout");
    m_bGettingShadow = true;
    LwpShadow* pRet = nullptr;
    if(m_nOverrideFlag & OVER_SHADOW)
    {
        LwpLayoutShadow* pLayoutShadow = dynamic_cast<LwpLayoutShadow*>(m_LayShadow.obj().get());
        pRet = pLayoutShadow ? &pLayoutShadow->GetShadow() : nullptr;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpLayout* pLay = dynamic_cast<LwpLayout*>(xBase.get()))
        {
            pRet = pLay->GetShadow();
        }
    }
    m_bGettingShadow = false;
    return pRet;
}

/**
 * @descr:  create xfshadow
*/
XFShadow* LwpLayout::GetXFShadow()
{
    LwpShadow* pShadow = GetShadow();
    if( pShadow )
    {
        LwpColor color = pShadow->GetColor();
        double  offsetX = pShadow->GetOffsetX();
        double  offsetY = pShadow->GetOffsetY();

        if( offsetX && offsetY && color.IsValidColor() )
        {
            XFShadow* pXFShadow = new XFShadow();
            enumXFShadowPos eXFShadowPos = enumXFShadowLeftTop;
            double fOffset = 0;

            bool left = false;
            bool top = false;
            if( offsetX < 0 )
                left = true;
            if( offsetY < 0 )
                top = true;
            if( left )
            {
                fOffset = -offsetX;
                if( top )
                    eXFShadowPos = enumXFShadowLeftTop;
                else
                    eXFShadowPos = enumXFShadowLeftBottom;
            }
            else
            {
                fOffset = offsetX;
                if( top )
                    eXFShadowPos = enumXFShadowRightTop;
                else
                    eXFShadowPos = enumXFShadowRightBottom;
            }

            pXFShadow->SetPosition(eXFShadowPos);
            pXFShadow->SetOffset(fOffset);
            pXFShadow->SetColor(XFColor(color.To24Color()));

            return pXFShadow;
        }
    }
    return nullptr;
}

/**
 * @descr get the layout that containers the current frame layout
 *
 */
rtl::Reference<LwpVirtualLayout> LwpLayout::GetContainerLayout()
{
    if(IsRelativeAnchored())
    {
        //get position
        LwpPara* pPara = dynamic_cast<LwpPara*>(GetPosition().obj().get());
        if(pPara)
        {
            LwpStory* pStory = pPara->GetStory();
            return pStory ? pStory->GetTabLayout() : nullptr;
        }
    }
    return GetParentLayout();
}

LwpPlacableLayout::LwpPlacableLayout( LwpObjectHeader const &objHdr, LwpSvStream* pStrm )
    : LwpLayout(objHdr, pStrm)
    , m_bGettingWrapType(false)
    , m_bGettingLayoutRelativity(false)
    , m_nWrapType(0)
    , m_nBuoyancy(0)
    , m_nBaseLineOffset(0)
{}

LwpPlacableLayout::~LwpPlacableLayout()
{}

void LwpPlacableLayout::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm.get();
    LwpLayout::Read();
    if(LwpFileHeader::m_nFileRevision < 0x000B)
    {
        assert(false);
    }
    else
    {
        sal_uInt16 simple = pStrm->QuickReaduInt16();
        if(!simple)
        {
            m_nWrapType = pStrm->QuickReaduInt8();
            m_nBuoyancy = pStrm->QuickReaduInt8();
            m_nBaseLineOffset = pStrm->QuickReadInt32();
            m_Script.Read( pStrm);
        }
        else
        {
            m_nWrapType = LAY_WRAP_AROUND;
            m_nBuoyancy = LAY_BUOYNEUTRAL;
            m_nBaseLineOffset    = 0;
        }
        m_LayRelativity.ReadIndexed( pStrm);
        if(pStrm->CheckExtra())
        {
            sal_uInt16 count = pStrm->QuickReaduInt16();
            if(count)
            {
                // temporily added by  to avoid assertion
                while (count)
                {
                    LwpPoint aPoint;
                    aPoint.Read(pStrm);
                    count--;
                }
            }
            pStrm->SkipExtra();
        }
    }
}
/**
 * @descr:  get wrap type
*/
sal_uInt8 LwpPlacableLayout::GetWrapType()
{
    if (m_bGettingWrapType)
        throw std::runtime_error("recursion in layout");
    m_bGettingWrapType = true;
    sal_uInt8 nWrapType = LAY_WRAP_AROUND;
    if (m_nOverrideFlag & OVER_PLACEMENT)
    {
        nWrapType = m_nWrapType;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpPlacableLayout* pLay = dynamic_cast<LwpPlacableLayout*>(xBase.get()))
        {
            nWrapType = pLay->GetWrapType();
        }
    }
    m_bGettingWrapType = false;
    return nWrapType;
}
/**
 * @descr:  get LayoutRelativity
*/
LwpLayoutRelativity* LwpPlacableLayout::GetRelativityPiece()
{
    if (m_bGettingLayoutRelativity)
        throw std::runtime_error("recursion in layout");
    m_bGettingLayoutRelativity = true;
    LwpLayoutRelativity* pRet = nullptr;
    if (!m_LayRelativity.IsNull())
    {
        if (m_nOverrideFlag & OVER_PLACEMENT)
        {
            pRet = dynamic_cast<LwpLayoutRelativity*>(m_LayRelativity.obj().get());
        }
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpPlacableLayout* pLay = dynamic_cast<LwpPlacableLayout*>(xBase.get()))
        {
            pRet = pLay->GetRelativityPiece();
        }
    }
    m_bGettingLayoutRelativity = false;
    return pRet;
}
/**
* @descr:   Get relative type
*
*/
sal_uInt8 LwpPlacableLayout::GetRelativeType()
{
    LwpLayoutRelativity* pLayRel = GetRelativityPiece();
    if(pLayRel)
    {
        return pLayRel->GetRelGuts().GetRelativeType();
    }
    return LwpVirtualLayout::GetRelativeType();
}
/**
* @descr:   Get offset from the baseline
*
*/
sal_Int32 LwpPlacableLayout::GetBaseLineOffset()
{
    /* The baseline is only valid if this is flow-with-text */
    if(GetRelativeType()!=LwpLayoutRelativityGuts::LAY_INLINE)
    {
        return 0;
    }

    // First, ask our content if it has a baseline, ignore now
    /*
    if (Content && Content->GetBaseLineOffset(&Baseline))
        return Baseline;
    */

    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        return m_nBaseLineOffset;
    }
    else
    {
        rtl::Reference<LwpObject> xBase(GetBasedOnStyle());
        if (LwpPlacableLayout* pLay = dynamic_cast<LwpPlacableLayout*>(xBase.get()))
        {
            return pLay->GetBaseLineOffset();
        }
    }
    return 0;

}
/**
* @descr:   whether the parent layout is page layout
*
*/
bool LwpPlacableLayout::IsAnchorPage()
{
    if(IsRelativeAnchored())
        return false;

    rtl::Reference<LwpVirtualLayout> xLayout = GetParentLayout();
    return xLayout.is() && (xLayout->IsPage() || xLayout->IsHeader() || xLayout->IsFooter());
}
/**
* @descr:   whether the parent layout is frame layout
*
*/
bool LwpPlacableLayout::IsAnchorFrame()
{
    if(IsRelativeAnchored())
        return false;

    rtl::Reference<LwpVirtualLayout> xLayout = GetParentLayout();
    return xLayout.is() && (xLayout->IsFrame() || xLayout->IsGroupHead());
}
/**
* @descr:   whether the parent layout is cell layout
*
*/
bool LwpPlacableLayout::IsAnchorCell()
{
    if(IsRelativeAnchored())
        return false;

    rtl::Reference<LwpVirtualLayout> xLayout = GetParentLayout();
    return xLayout.is() && xLayout->IsCell();
}

/**
* @descr:   Set font style for setting position of frame
*
*/
void LwpPlacableLayout::SetFont(rtl::Reference<XFFont> const & pFont)
{
    m_pFont = pFont;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
