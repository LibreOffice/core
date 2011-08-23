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
 Jan 2005			Created
 Feb 2005			Process gemotry, margins
 ************************************************************************/

#include "lwplayout.hxx"
#include "lwpusewhen.hxx"
#include "lwptools.hxx"
#include "xfilter/xfcolumns.hxx"
#include "lwpstory.hxx"
#include "lwpparastyle.hxx"
#include "lwpholder.hxx"
#include "lwpdoc.hxx"
#include "lwppagehint.hxx"
#include "lwpdivinfo.hxx"
#include "lwpgrfobj.hxx"
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

LwpVirtualLayout::LwpVirtualLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpDLNFPVList(objHdr, pStrm)
{}

void LwpVirtualLayout::Read()
{
    LwpDLNFPVList::Read();

    LwpObjectStream* pStrm = m_pObjStrm;
    pStrm->QuickRead(&m_nAttributes, sizeof(m_nAttributes));
    pStrm->QuickRead(&m_nAttributes2, sizeof(m_nAttributes2));
    pStrm->QuickRead(&m_nAttributes3, sizeof(m_nAttributes3));
    pStrm->QuickRead(&m_nOverrideFlag, sizeof(m_nOverrideFlag));
    pStrm->QuickRead(&m_nDirection, sizeof(m_nDirection));

    //Note that two bytes is read into m_nEditorID instead of one byte.
    pStrm->QuickRead(&m_nEditorID, sizeof(m_nEditorID));

    m_NextEnumerated.ReadIndexed(pStrm);
    m_PreviousEnumerated.ReadIndexed(pStrm);

    pStrm->SkipExtra();
}

sal_Bool LwpVirtualLayout::MarginsSameAsParent()
{
    return m_nAttributes2 & STYLE2_MARGINSSAMEASPARENT ? sal_True : sal_False;
}

/**
* @descr: 	Get column width
*
*/
double LwpVirtualLayout::GetColWidth(sal_uInt16 /*nIndex*/)
{
    //return GetContentWidth(); //not support now
    //return LwpTools::ConvertToMetric(5); //test
    return 0; //test
}

/**
* @descr: 	Get the gap between columns
*
*/
double LwpVirtualLayout::GetColGap(sal_uInt16 /*nIndex*/)
{
    //return DEFAULTGAPSIZE;
    //return LwpTools::ConvertToMetric(0.17);//DEFAULTGAPSIZE=0.17
    return LwpTools::ConvertToMetric(0.17);
}

/**
* @descr: 	Whether it is honoring protection
*
*/
sal_Bool LwpVirtualLayout::HonorProtection()
{
    if(!(m_nAttributes2 & STYLE2_HONORPROTECTION))
        return sal_False;

    LwpVirtualLayout* pParent = static_cast<LwpVirtualLayout*> (GetParent()->obj());
    if(pParent && !pParent->IsHeader())
    {
        return pParent->HonorProtection();
    }

    if(m_pFoundry)//is null now
    {
        LwpDocument* pDoc = m_pFoundry->GetDocument();
        /*if(pDoc)
        {
            return pDoc->HonorProtection();
        }*/
        if(pDoc && pDoc->GetRootDocument())
            return pDoc->GetRootDocument()->HonorProtection();
    }

    return sal_True;
}

/**
* @descr: 	Whether it is protected
*
*/
sal_Bool LwpVirtualLayout::IsProtected()
{
    sal_Bool bProtected = (m_nAttributes & STYLE_PROTECTED)!=0;

    LwpVirtualLayout* pParent = static_cast<LwpVirtualLayout*> (GetParent()->obj());
    if(pParent && !pParent->IsHeader())
    {
        if(pParent->HonorProtection()&&(pParent->HasProtection()||bProtected))
        {
            return sal_True;
        }
    }
    else if(m_pFoundry)//is null now
    {
        LwpDocument* pDoc = m_pFoundry->GetDocument();
        if(pDoc)
        {
            if (pDoc->HonorProtection() && bProtected)
            {
                return sal_True;
            }
        }
    }

    return sal_False;
}

/**
* @descr: 	Whether it has protection
*
*/
sal_Bool LwpVirtualLayout::HasProtection()
{
    if(m_nAttributes & STYLE_PROTECTED)
        return sal_True;

    LwpVirtualLayout* pParent = static_cast<LwpVirtualLayout*> (GetParent()->obj());
    if(pParent && !pParent->IsHeader())
    {
        return pParent->HasProtection();
    }

    return sal_False;
}

/**
* @descr: 	Whether it is a mirror layout
*
*/
sal_Bool LwpVirtualLayout::IsComplex()
{
    return m_nAttributes & STYLE_COMPLEX ? sal_True : sal_False;
}

/**
* @descr: 	Get usewhen pointer
*
*/
LwpUseWhen* LwpVirtualLayout::GetUseWhen()
{
    /*
        If we have a parent, and I'm not a page layout,
        use my parents information.
    */
    if(GetLayoutType()!=LWP_PAGE_LAYOUT)
    {
        //get parent
        LwpVirtualLayout* pParent = static_cast<LwpVirtualLayout*> (GetParent()->obj());
        if(pParent && !pParent->IsHeader()&& (pParent->GetLayoutType()!=LWP_PAGE_LAYOUT))
            return pParent->GetUseWhen();

    }

    return VirtualGetUseWhen();
}
/**
 * @descr:  Whether this layout is page layout or not
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsPage()
{
    return (GetLayoutType() == LWP_PAGE_LAYOUT);
}
/**
 * @descr:  Whether this layout is heaer layout or not
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsHeader()
{
    return (GetLayoutType() == LWP_HEADER_LAYOUT);
}
/**
 * @descr:  Whether this layout is footer layout or not
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsFooter()
{
    return (GetLayoutType() == LWP_FOOTER_LAYOUT);
}
/**
 * @descr:  Whether this layout is frame layout or not
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsFrame()
{
    return (GetLayoutType() == LWP_FRAME_LAYOUT);
}

/**
 * @descr:  Whether this layout is cell layout or not
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsCell()
{
    return (GetLayoutType() == LWP_CELL_LAYOUT
        || GetLayoutType() == LWP_CONNECTED_CELL_LAYOUT
        || GetLayoutType() == LWP_HIDDEN_CELL_LAYOUT);
}
/**
 * @descr:  Whether this layout is supertable layout or not
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsSuperTable()
{
    return (GetLayoutType() == LWP_SUPERTABLE_LAYOUT);
}
/**
 * @descr:  Whether this layout is group layout or not
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsGroupHead()
{
    return (GetLayoutType() == LWP_GROUP_LAYOUT);
}
/**
 * @descr:  get the relative type
 * @param:
 * @return:
*/
sal_uInt8 LwpVirtualLayout::GetRelativeType()
{
    return LwpLayoutRelativityGuts::LAY_PARENT_RELATIVE;
}
/**
 * @descr:  whether it is relative anchored layout
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsRelativeAnchored()
{
    sal_uInt8 nType;

    nType = GetRelativeType();
    return (nType == LwpLayoutRelativityGuts::LAY_PARA_RELATIVE) || (nType == LwpLayoutRelativityGuts::LAY_INLINE)
    || (nType == LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE) || (nType == LwpLayoutRelativityGuts::LAY_INLINE_VERTICAL);
}
/**
 * @descr:  whether it is MinimumHeight layout
 * @param:
 * @return:
*/
sal_Bool LwpVirtualLayout::IsMinimumHeight()
{
    return ((m_nAttributes3& STYLE3_MINHEIGHTVALID) != 0);
}

/**
* @descr: 	Get parent layout
*
*/
LwpVirtualLayout* LwpVirtualLayout::GetParentLayout()
{
    return static_cast<LwpVirtualLayout*> (GetParent()->obj());
}

/**
* @descr:   Register child layout style
*
*/
void LwpVirtualLayout::RegisterChildStyle()
{
    //Register all children styles
    LwpVirtualLayout* pLayout = static_cast<LwpVirtualLayout*>(GetChildHead()->obj());
    while(pLayout)
    {
        pLayout->SetFoundry(m_pFoundry);
        pLayout->RegisterStyle();
        pLayout = static_cast<LwpVirtualLayout*>(pLayout->GetNext()->obj());
    }
}

sal_Bool LwpVirtualLayout::NoContentReference()
{
    return (m_nAttributes2 & STYLE2_NOCONTENTREFERENCE) != 0;
}

sal_Bool LwpVirtualLayout::IsStyleLayout()
{
    if (m_nAttributes3 & STYLE3_STYLELAYOUT)
        return sal_True;

    LwpVirtualLayout* pParent =static_cast<LwpVirtualLayout*>(GetParent()->obj());
    if (pParent)
        return pParent->IsStyleLayout();
    return sal_False;
}

/**
* @descr: 	Find child layout by layout type
*
*/
LwpVirtualLayout* LwpVirtualLayout::FindChildByType(LWP_LAYOUT_TYPE eType)
{
    LwpObjectID *pID = GetChildHead();

    while(pID && !pID->IsNull())
    {
        LwpVirtualLayout * pLayout = static_cast<LwpVirtualLayout *>(pID->obj());
        if(!pLayout)
        {
            break;
        }

        if (pLayout && pLayout->GetLayoutType() == eType)
        {
            return pLayout;
        }
        pID = pLayout->GetNext();
    }

    return NULL;
}

/**
* @descr: 	Whether the size of layout is fit the graphic
*
*/
sal_Bool LwpVirtualLayout::IsFitGraphic()
{
    return IsAutoGrowRight() && !IsAutoGrowLeft() && IsAutoGrowDown();
}

/**
* @descr: 	Whether the width of layout is auto grow
*
*/
sal_Bool LwpVirtualLayout::IsAutoGrowWidth()
{
    return IsAutoGrowLeft() || IsAutoGrowRight();
}

/**
* @descr: 	Determine whether the layout width is to margin
*
*/
sal_Bool LwpVirtualLayout::IsInlineToMargin()
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
* @descr: 	Looking for the layout which follows the pStartLayout
* @param:  	pStartLayout - the layout which is used for looking for its following layout
*/
LwpVirtualLayout* LwpAssociatedLayouts::GetLayout(LwpVirtualLayout *pStartLayout)
{
    LwpVirtualLayout* pLayout = NULL;

    if (!pStartLayout && !m_OnlyLayout.IsNull())
        /* Looking for the first layout and there's only one layout in  the list.*/
        return static_cast<LwpVirtualLayout*>(m_OnlyLayout.obj());

    LwpObjectHolder* pObjHolder =static_cast<LwpObjectHolder*>(m_Layouts.GetHead()->obj());
    if(pObjHolder)
    {
        pLayout = static_cast<LwpVirtualLayout*>(pObjHolder->GetObject()->obj());
        if(!pStartLayout )
            return pLayout;

        while(pObjHolder && pStartLayout != pLayout)
        {
            pObjHolder = static_cast<LwpObjectHolder*>(pObjHolder->GetNext()->obj());
            if(pObjHolder)
            {
                pLayout = static_cast<LwpVirtualLayout*>(pObjHolder->GetObject()->obj());
            }
        }

        if(pObjHolder)
        {
            pObjHolder = static_cast<LwpObjectHolder*>(pObjHolder->GetNext()->obj());
            if(pObjHolder)
            {
                pLayout = static_cast<LwpVirtualLayout*>(pObjHolder->GetObject()->obj());
                return pLayout;
            }
        }

        //return pLayout;
    }

    return NULL;
}

LwpHeadLayout::LwpHeadLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
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
    LwpVirtualLayout* pLayout = static_cast<LwpVirtualLayout*>(GetChildHead()->obj());
    while(pLayout)
    {
        pLayout->SetFoundry(m_pFoundry);
        //if the layout is relative to para, the layout will be registered in para
        if(!pLayout->IsRelativeAnchored())
        {
            pLayout->RegisterStyle();
        }
        pLayout = static_cast<LwpVirtualLayout*>(pLayout->GetNext()->obj());
    }
}

/**
 * @descr   find endnote supertable layout from the child layout list. Suppose that there is only one endnote supertablelayout in one division
 * @return pointer to endnote supertable layout
 */
LwpVirtualLayout* LwpHeadLayout::FindEnSuperTableLayout()
{
    LwpVirtualLayout* pLayout = static_cast<LwpVirtualLayout*>(GetChildHead()->obj());
    while(pLayout)
    {
        if(pLayout->GetLayoutType() == LWP_ENDNOTE_SUPERTABLE_LAYOUT)
        {
            return pLayout;
        }
        pLayout = static_cast<LwpVirtualLayout*>(pLayout->GetNext()->obj());
    }
    return NULL;
}

LwpLayoutStyle::LwpLayoutStyle()
    : m_nStyleDefinition(0)
    , m_pDescription(new LwpAtomHolder)
    , m_nKey(0)
{
}

LwpLayoutStyle::~LwpLayoutStyle()
{
    delete m_pDescription;
}

void LwpLayoutStyle::Read(LwpObjectStream* pStrm)
{
    pStrm->QuickRead(&m_nStyleDefinition, sizeof(m_nStyleDefinition));
    m_pDescription->Read(pStrm);
    if (pStrm->CheckExtra())
    {
        pStrm->QuickRead(&m_nKey, 2);
        pStrm->SkipExtra();
    }
}

LwpLayoutMisc::LwpLayoutMisc() :
m_nGridDistance(0), m_nGridType(0),
m_pContentStyle(new LwpAtomHolder)
{
}

LwpLayoutMisc::~LwpLayoutMisc()
{
    if (m_pContentStyle)
    {
        delete m_pContentStyle;
    }
}

void LwpLayoutMisc::Read(LwpObjectStream* pStrm)
{
    pStrm->QuickRead(&m_nGridType, 2);
    pStrm->QuickRead(&m_nGridDistance, 4);
    m_pContentStyle->Read(pStrm);
    pStrm->SkipExtra();
}

LwpMiddleLayout::LwpMiddleLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm )
    : LwpVirtualLayout(objHdr, pStrm),
      m_pStyleStuff(new LwpLayoutStyle), m_pMiscStuff(new LwpLayoutMisc)
{}

LwpMiddleLayout::~LwpMiddleLayout()
{
    if (m_pStyleStuff)
    {
        delete m_pStyleStuff;
    }
    if (m_pMiscStuff)
    {
        delete m_pMiscStuff;
    }
}
void LwpMiddleLayout::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm;

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

    sal_uInt8 nWhatsItGot = 0;
    pStrm->QuickRead(&nWhatsItGot, 1);

    if (nWhatsItGot & DISK_GOT_STYLE_STUFF)
    {
        m_pStyleStuff->Read(pStrm);
    }
    if (nWhatsItGot & DISK_GOT_MISC_STUFF)
    {
        m_pMiscStuff->Read(pStrm);
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
    //end
}

#include "lwplaypiece.hxx"

/**
* @descr: 	Get the geometry of current layout
*
*/
LwpLayoutGeometry* LwpMiddleLayout::GetGeometry()
{
    if( !m_LayGeometry.IsNull() )
    {
        return ( static_cast<LwpLayoutGeometry*> (m_LayGeometry.obj()) );
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetGeometry();
    }
    return NULL;
}

/**
* @descr: 	Get layout height, measured by "cm"
*
*/
double LwpMiddleLayout::GetGeometryHeight()
{
    LwpLayoutGeometry* pGeo = GetGeometry();
    if(pGeo)
    {
        return ( LwpTools::ConvertFromUnitsToMetric( pGeo->GetHeight() ) );
    }
    else
        return -1;
}

/**
* @descr: 	Get layout width, measured by "cm"
*
*/
double LwpMiddleLayout::GetGeometryWidth()
{
    LwpLayoutGeometry* pGeo = GetGeometry();
    if(pGeo)
    {
        return ( LwpTools::ConvertFromUnitsToMetric( pGeo->GetWidth() ) );
    }
    else
        return -1;
}

/**
* @descr: 	Whether the margins is same as parent layout
*
*/
BOOL LwpMiddleLayout::MarginsSameAsParent()
{
    if(m_nOverrideFlag & OVER_MARGINS)
    {
        return LwpVirtualLayout::MarginsSameAsParent();
    }
    if(!m_BasedOnStyle.IsNull())
    {
        LwpVirtualLayout* play = static_cast<LwpVirtualLayout*> (m_BasedOnStyle.obj());
        play->MarginsSameAsParent();
    }
    return LwpVirtualLayout::MarginsSameAsParent();

}

/**
* @descr: 	Get margin
* @param:  	nWhichSide - 0: left, 1: right, 2:top, 3: bottom
*/
double LwpMiddleLayout::GetMarginsValue(const sal_uInt8 &nWhichSide)
{
    double fValue = 0;
    if((nWhichSide==MARGIN_LEFT)||(nWhichSide==MARGIN_RIGHT))
    {
        if ( MarginsSameAsParent() )
        {
            LwpVirtualLayout* pParent = static_cast<LwpVirtualLayout*> (GetParent()->obj());
            if(pParent && !pParent->IsHeader())
            {
                fValue = pParent->GetMarginsValue(nWhichSide);
                return fValue;
            }
        }
    }

    if(m_nOverrideFlag & OVER_MARGINS)
    {
        LwpLayoutMargins* pMar1 = static_cast<LwpLayoutMargins*> (m_LayMargins.obj());
        if(pMar1)
        {
            fValue = pMar1->GetMargins()->GetMarginsValue(nWhichSide);
            return fValue;
        }
    }
    LwpVirtualLayout* pStyle = static_cast<LwpVirtualLayout*> (m_BasedOnStyle.obj());
    if(pStyle)
    {
        fValue = pStyle->GetMarginsValue(nWhichSide);
        return fValue;
    }
    return LwpVirtualLayout::GetMarginsValue(nWhichSide);
}
/**
 * @descr:	Get extmargin value
 * @param:	nWhichSide - 0: left, 1: right, 2:top, 3: bottom
 * @param:
 * @return:
*/
double LwpMiddleLayout::GetExtMarginsValue(const sal_uInt8 &nWhichSide)
{
    double fValue = 0;
    if(m_nOverrideFlag & OVER_MARGINS)
    {
        LwpLayoutMargins* pMar1 = static_cast<LwpLayoutMargins*> (m_LayMargins.obj());
        if(pMar1)
        {
            fValue = pMar1->GetExtMargins()->GetMarginsValue(nWhichSide);
            return fValue;
        }
    }
    LwpVirtualLayout* pStyle = static_cast<LwpVirtualLayout*> (m_BasedOnStyle.obj());
    if(pStyle)
    {
        fValue = pStyle->GetExtMarginsValue(nWhichSide);
        return fValue;
    }
    return LwpVirtualLayout::GetExtMarginsValue(nWhichSide);
}
/**
 * @descr:  Get the LwpBorderStuff object according to m_LayBorderStuff id.
 * @param:
 * @param:
 * @return:
*/
LwpBorderStuff* LwpMiddleLayout::GetBorderStuff()
{
    if(m_nOverrideFlag & OVER_BORDERS)
    {
        LwpLayoutBorder* pLayoutBorder = static_cast<LwpLayoutBorder*>(m_LayBorderStuff.obj());
        return pLayoutBorder->GetBorderStuff();
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetBorderStuff();
    }
    return NULL;
}

/**
 * @descr:  Get LwpBackgroundStuff object according to m_LayBackgroundStuff id;
 * @param:
 * @param:
 * @return:
*/
LwpBackgroundStuff* LwpMiddleLayout::GetBackgroundStuff()
{
    if(m_nOverrideFlag & OVER_BACKGROUND)
    {
        LwpLayoutBackground* pLayoutBackground = static_cast<LwpLayoutBackground*>(m_LayBackgroundStuff.obj());
        return pLayoutBackground->GetBackgoudStuff();
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetBackgroundStuff();
    }
    return NULL;
}
/**
 * @descr:  create xfborder.
 * @param:
 * @param:
 * @return:
*/
XFBorders* LwpMiddleLayout::GetXFBorders()
{
    LwpBorderStuff* pBorderStuff = GetBorderStuff();
    if(pBorderStuff&&pBorderStuff->GetSide() != 0)
    {
        //copy from lwpparastyle.
        XFBorders	*pXFBorders = new XFBorders();
        // apply 4 borders respectively
        LwpBorderStuff::BorderType pType[] = { LwpBorderStuff::LEFT, LwpBorderStuff::RIGHT,
            LwpBorderStuff::TOP, LwpBorderStuff::BOTTOM };

        for (sal_uInt8 nC = 0; nC < 4; nC++)
        {
            if (pBorderStuff->HasSide(pType[nC]))
            {
                LwpParaStyle::ApplySubBorder(pBorderStuff, pType[nC], pXFBorders);
            }
        }
        return pXFBorders;
    }
    return NULL;
}

/**
 * @descr:  Get text direction settings.
 * @param:
 * @param:
 * @return:
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
 * @param:
 * @param:
 * @return:
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
    return NULL;
}

/**
 * @descr:  Add back color settings into xfpagemaster.
 * @param:
 * @param:
 * @return:
*/
LwpTabOverride* LwpMiddleLayout::GetTabOverride()
{
    if(m_nAttributes & OVER_TABS)
    {
        if(!m_TabPiece.IsNull())
            return (LwpTabOverride*)static_cast<LwpTabPiece*>(m_TabPiece.obj())->GetOverride();
        return NULL;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetTabOverride();
    }
    return NULL;
}

/**
 * @descr:   Layscale for graphic & watermark
 * @param:
 * @param:
 * @return:
*/
sal_uInt16 LwpMiddleLayout::GetScaleMode(void)
{
    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj())
        return GetLayoutScale()->GetScaleMode();
    else if (m_BasedOnStyle.obj())
        return static_cast<LwpMiddleLayout*>(m_BasedOnStyle.obj())->GetScaleMode();
    else
        return (LwpLayoutScale::FIT_IN_FRAME | LwpLayoutScale::MAINTAIN_ASPECT_RATIO);
}

void LwpMiddleLayout::SetScaleMode(sal_uInt16 nVal)
{
    m_nOverrideFlag |= OVER_SCALING;
//	nVal |= LwpLayoutScale::MAINTAIN_ASPECT_RATIO;
    GetLayoutScale()->SetScaleMode(nVal);
}

sal_uInt16 LwpMiddleLayout::GetScaleTile(void)
{
    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj())
        return (GetLayoutScale()->GetPlacement() & LwpLayoutScale::TILED)
            ? 1 : 0;
    else if (m_BasedOnStyle.obj())
        return static_cast<LwpMiddleLayout*>(m_BasedOnStyle.obj())->GetScaleTile();
    else
        return 0;
}

void LwpMiddleLayout::SetScaleTile(sal_uInt16 nVal)
{
    m_nOverrideFlag |= OVER_SCALING;
    if (nVal)
        GetLayoutScale()->SetPlacement(GetLayoutScale()->GetPlacement() | LwpLayoutScale::TILED);
    else
        GetLayoutScale()->SetPlacement(GetLayoutScale()->GetPlacement() & ~LwpLayoutScale::TILED);
}

sal_uInt16 LwpMiddleLayout::GetScaleCenter(void)
{
    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj())
        return (GetLayoutScale()->GetPlacement() & LwpLayoutScale::CENTERED)
            ? 1 : 0;
    else if (m_BasedOnStyle.obj())
        return static_cast<LwpMiddleLayout*>(m_BasedOnStyle.obj())->GetScaleCenter();
    else
        return 0;
}

void LwpMiddleLayout::SetScaleCenter(sal_uInt16 nVal)
{
    m_nOverrideFlag |= OVER_SCALING;
    if (nVal)
        GetLayoutScale()->SetPlacement(GetLayoutScale()->GetPlacement() | LwpLayoutScale::CENTERED);
    else
        GetLayoutScale()->SetPlacement(GetLayoutScale()->GetPlacement() & ~LwpLayoutScale::CENTERED);
}

sal_uInt32 LwpMiddleLayout::GetScalePercentage(void)
{
    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj())
        return GetLayoutScale()->GetScalePercentage()/10;//m_nScalePercentage 1000 = 100%
    else if (m_BasedOnStyle.obj())
        return static_cast<LwpMiddleLayout*>(m_BasedOnStyle.obj())->GetScalePercentage();
    else
        return 100;
}

void LwpMiddleLayout::SetScalePercentage(sal_uInt32 nVal)
{
    m_nOverrideFlag |= OVER_SCALING;
    GetLayoutScale()->SetScalePercentage(nVal*10);
}

double LwpMiddleLayout::GetScaleWidth(void)
{
    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj())
        return LwpTools::ConvertFromUnits(GetLayoutScale()->GetScaleWidth());
    else if (m_BasedOnStyle.obj())
        return static_cast<LwpMiddleLayout*>(m_BasedOnStyle.obj())->GetScaleWidth();
    else
        return 0;
}

void LwpMiddleLayout::SetScaleWidth(double fVal)
{
    m_nOverrideFlag |= OVER_SCALING;
    GetLayoutScale()->SetScaleWidth(LwpTools::ConvertToUnits(fVal));
}

double LwpMiddleLayout::GetScaleHeight(void)
{
    if ((m_nOverrideFlag & OVER_SCALING) && m_LayScale.obj())
        return LwpTools::ConvertFromUnits(GetLayoutScale()->GetScaleHeight());
    else if (m_BasedOnStyle.obj())
        return static_cast<LwpMiddleLayout*>(m_BasedOnStyle.obj())->GetScaleHeight();
    else
        return 0;
}
sal_Bool LwpMiddleLayout::CanSizeRight(void)
{
    sal_uInt8 RelType = GetRelativeType();

    if (RelType == LwpLayoutRelativityGuts::LAY_INLINE || RelType == LwpLayoutRelativityGuts::LAY_PARA_RELATIVE
    || RelType == LwpLayoutRelativityGuts::LAY_INLINE_VERTICAL)
    {
        return sal_False;
    }
    return sal_True;
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
        assert(sal_False);
    }
    else if (IsSizeRightToContainer())
    {
        assert(sal_False);
    }
    else
    {
        m_nAttributes3 |= STYLE3_WIDTHVALID;
        return GetGeometry()->GetWidth();
    }
    return 0;
}
sal_Bool LwpMiddleLayout::IsSizeRightToContainer(void)
{
    if (!CanSizeRight())
        return sal_False;

    if (m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection & ((LAY_USEDIRECTION|LAY_AUTOSIZE|LAY_TOCONTAINER)
                        << SHIFT_RIGHT))
        == ((LAY_USEDIRECTION | LAY_TOCONTAINER | LAY_AUTOSIZE) << SHIFT_RIGHT);
    }
    else if (m_BasedOnStyle.obj())
    {
        LwpMiddleLayout * pLayout = static_cast<LwpMiddleLayout *>(m_BasedOnStyle.obj());
        return pLayout->IsSizeRightToContainer();
    }
    else
        return sal_False;
}
sal_Bool LwpMiddleLayout::IsSizeRightToContent(void)
{
    if (!CanSizeRight())
        return sal_False;

    if (m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nDirection & ((LAY_USEDIRECTION|LAY_AUTOSIZE|LAY_TOCONTAINER)
                            << SHIFT_RIGHT))
                == ((LAY_USEDIRECTION | LAY_AUTOSIZE) << SHIFT_RIGHT);
    }
    else if (m_BasedOnStyle.obj())
    {
        LwpMiddleLayout * pLayout = static_cast<LwpMiddleLayout *>(m_BasedOnStyle.obj());
        return pLayout->IsSizeRightToContent();
    }
    else
        return sal_False;
}

/**
* @descr: 	Get layout height
*
*/
double LwpMiddleLayout::GetHeight()
{
    return GetGeometryHeight();
}

/**
* @descr: 	Get layout height
*
*/
double LwpMiddleLayout::GetWidth()
{
    return GetGeometryWidth();
}
/**
* @descr: 	Get layout orgin point
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

void LwpMiddleLayout::SetScaleHeight(double fVal)
{
    m_nOverrideFlag |= OVER_SCALING;
    GetLayoutScale()->SetScaleHeight(LwpTools::ConvertToUnits(fVal));
}

/**
* @descr: 	Whether the fill is pattern fill or not
* @return:	True if yes, false if not.
*/
sal_Bool LwpMiddleLayout::IsPatternFill()
{
    LwpBackgroundStuff* pBackgroundStuff = GetBackgroundStuff();
    if (pBackgroundStuff)
    {
        return pBackgroundStuff->IsPatternFill();
    }

    return sal_False;
}

/**
* @descr: 	Get the fill pattern style. Data are saved in a XFBGImage object
* @return:	the fill pattern style.
*/
XFBGImage* LwpMiddleLayout::GetFillPattern()
{
    LwpBackgroundStuff* pBackgroundStuff = GetBackgroundStuff();
    if (pBackgroundStuff)
    {
        return pBackgroundStuff->GetFillPattern();
    }

    return NULL;

}

/**
* @descr: 	Whether the height and width of layout is auto grow
*
*/
sal_Bool LwpMiddleLayout::IsAutoGrow()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return m_nDirection &
            ((LAY_AUTOGROW << SHIFT_UP) | (LAY_AUTOGROW << SHIFT_DOWN) |
            (LAY_AUTOGROW << SHIFT_RIGHT) | (LAY_AUTOGROW << SHIFT_LEFT))
            ? sal_True : sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsAutoGrow();
    }
    return LwpVirtualLayout::IsAutoGrow();
}

/**
* @descr: 	Whether the height of layout is auto grow down
*
*/
sal_Bool LwpMiddleLayout::IsAutoGrowDown()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return m_nDirection & (LAY_AUTOGROW << SHIFT_DOWN) ? sal_True : sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsAutoGrowDown();
    }
    return LwpVirtualLayout::IsAutoGrowDown();
}

/**
* @descr: 	Whether the height of layout is auto grow up
*
*/
sal_Bool LwpMiddleLayout::IsAutoGrowUp()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return m_nDirection & (LAY_AUTOGROW << SHIFT_UP) ? sal_True : sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsAutoGrowUp();
    }
    return LwpVirtualLayout::IsAutoGrowUp();
}

/**
* @descr: 	Whether the height of layout is auto grow down
*
*/
sal_Bool LwpMiddleLayout::IsAutoGrowLeft()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return m_nDirection & (LAY_AUTOGROW << SHIFT_LEFT) ? sal_True : sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsAutoGrowLeft();
    }
    return LwpVirtualLayout::IsAutoGrowLeft();
}

/**
* @descr: 	Whether the height of layout is auto grow down
*
*/
sal_Bool LwpMiddleLayout::IsAutoGrowRight()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return m_nDirection & (LAY_AUTOGROW << SHIFT_RIGHT) ? sal_True : sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsAutoGrowRight();
    }
    return LwpVirtualLayout::IsAutoGrowRight();
}


/**
* @descr: 	Get contents orientation
*
*/
sal_uInt8 LwpMiddleLayout::GetContentOrientation()
{
    //content orientation in Graphic objects and OLE objects not supported now
    if((m_nOverrideFlag & OVER_ROTATION)&& !m_LayGeometry.IsNull())
    {
        LwpLayoutGeometry* pLayGeometry = static_cast<LwpLayoutGeometry*> (m_LayGeometry.obj());
        return pLayGeometry->GetContentOrientation();
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetContentOrientation();
    }
    return LwpVirtualLayout::GetContentOrientation();
}

/**
* @descr: 	Whether it is honoring protection
*
*/
sal_Bool LwpMiddleLayout::HonorProtection()
{
    if(m_nOverrideFlag & OVER_MISC)
    {
        if(!(m_nAttributes2 & STYLE2_HONORPROTECTION))
            return sal_False;

        LwpVirtualLayout* pParent = static_cast<LwpVirtualLayout*> (GetParent()->obj());
        if(pParent && !pParent->IsHeader())
        {
            return pParent->HonorProtection();
        }

        if(m_pFoundry)//is null now
        {
            LwpDocument* pDoc = m_pFoundry->GetDocument();
            if(pDoc)
            {
                return pDoc->HonorProtection();
            }
        }
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->HonorProtection();
    }

    return LwpVirtualLayout::HonorProtection();
}

/**
* @descr: 	Whether it is pretected
*
*/
sal_Bool LwpMiddleLayout::IsProtected()
{
    sal_Bool bProtected = sal_False;
    if(m_nOverrideFlag & OVER_MISC)
    {
        bProtected = (m_nAttributes & STYLE_PROTECTED)!=0;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        bProtected = pLay->IsProtected();
    }
    else
        bProtected = LwpVirtualLayout::IsProtected();

    LwpVirtualLayout* pParent = static_cast<LwpVirtualLayout*> (GetParent()->obj());
    if(pParent && !pParent->IsHeader())
    {
        /* If a parent's protected then none of its children can be accessed. */
        if(pParent->IsProtected())
            return sal_True;

        if(pParent->HonorProtection())
            return bProtected;

        /* If our parent isn't honoring protection then we aren't protected. */
        return sal_False;

    }
    if(m_pFoundry)//is null now
    {
        LwpDocument* pDoc = m_pFoundry->GetDocument();
        if(pDoc)
        {
            if (pDoc->HonorProtection())
                return bProtected;

            /* If the document isn't honoring protection then we aren't protected.*/
            return sal_False;
        }
    }

    return bProtected;
}

/**
* @descr: 	Get watermark layout
*
*/
LwpVirtualLayout* LwpMiddleLayout::GetWaterMarkLayout()
{
    LwpVirtualLayout* pLay = static_cast<LwpVirtualLayout*>(GetChildHead()->obj());
    while(pLay)
    {
        if( pLay->IsForWaterMark())
        {
            return pLay;
        }
        pLay = static_cast<LwpVirtualLayout*> (pLay->GetNext()->obj());
    }
    return NULL;
}

/**
* @descr: 	Create and reture xfbgimage object for watermark
*
*/
XFBGImage* LwpMiddleLayout::GetXFBGImage()
{
    LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*>(GetWaterMarkLayout());
    if(pLay)
    {
        //test BGImage
        LwpGraphicObject* pGrfObj = static_cast<LwpGraphicObject*>(pLay->GetContent()->obj());
        if(pGrfObj)
        {
            XFBGImage* pXFBGImage = new XFBGImage();

            if(pGrfObj->IsLinked())
            {
                //set file link
                OUString linkedfilepath = pGrfObj->GetLinkedFilePath();
                OUString fileURL = LwpTools::convertToFileUrl(OUStringToOString(linkedfilepath, osl_getThreadTextEncoding()));
                pXFBGImage->SetFileLink(fileURL);
            }
            else
            {
                sal_uInt8* pGrafData = NULL;
                sal_uInt32 nDataLen = pGrfObj->GetRawGrafData(pGrafData);
                pXFBGImage->SetImageData(pGrafData, nDataLen);
                if(pGrafData)
                {
                    delete pGrafData;
                    pGrafData = NULL;
                }
            }


            //automatic, top left
            pXFBGImage->SetPosition(enumXFAlignStart,enumXFAlignTop);
            if(pLay->GetScaleCenter())
            {
                //center
                pXFBGImage->SetPosition(enumXFAlignCenter,enumXFAlignCenter);
            }
            else if(pLay->GetScaleTile())
            {
                //tile
                pXFBGImage->SetRepeate();
            }
            //fit type, area type
            if((pLay->GetScaleMode()& LwpLayoutScale::FIT_IN_FRAME)!=0)
            {
                if((pLay->GetScaleMode()& LwpLayoutScale::MAINTAIN_ASPECT_RATIO)==0)
                {
                    pXFBGImage->SetStretch();
                }
            }
            return pXFBGImage;
        }
    }
    return NULL;
}

/**
* @descr: 	Whether the page uses the printer setting
*
*/
sal_Bool LwpMiddleLayout::GetUsePrinterSettings()
{
    if(m_nOverrideFlag & OVER_SIZE)
    {
        return (m_nAttributes3 & STYLE3_USEPRINTERSETTINGS) != 0;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpMiddleLayout* pLay = static_cast<LwpMiddleLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetUsePrinterSettings();
    }
    return sal_False;
}


//Check whether there are contents in the layout
sal_Bool LwpMiddleLayout::HasContent()
{
    LwpObject* content = m_Content.obj();
    if(content)
        return sal_True;
    return sal_False;
}
//End by

LwpLayout::LwpLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm ) :
    LwpMiddleLayout(objHdr, pStrm), m_pUseWhen(new LwpUseWhen)
{}

LwpLayout::~LwpLayout()
{
    if (m_pUseWhen)
    {
        delete m_pUseWhen;
    }
}

void LwpLayout::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm;

    LwpMiddleLayout::Read();
    if (LwpFileHeader::m_nFileRevision < 0x000B)
    {
        // read PreRevBLayout...
    }
    else
    {
        sal_uInt16 nSimple;
        pStrm->QuickRead(&nSimple, 2);

        if (!nSimple)
        {
            m_pUseWhen->Read(pStrm);

            sal_uInt8 nFlag;
            pStrm->QuickRead(&nFlag, 1);
            if (nFlag)
            {
                m_Positon.ReadIndexed(pStrm);
            }
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
* @descr: 	Get columns number
*
*/
sal_uInt16 LwpLayout::GetNumCols()
{
    if(m_nOverrideFlag & OVER_COLUMNS)
    {
        LwpLayoutColumns* pLayColumns = static_cast<LwpLayoutColumns*>(m_LayColumns.obj());
        if(pLayColumns)
        {
            return pLayColumns->GetNumCols();
        }
    }

    LwpVirtualLayout* pStyle = static_cast<LwpVirtualLayout*> (m_BasedOnStyle.obj());
    if(pStyle)
    {
        return pStyle->GetNumCols();
    }

    return LwpVirtualLayout::GetNumCols();

}

/**
* @descr: 	Get column width
* @param:	the order of column
*/
double LwpLayout::GetColWidth(sal_uInt16 nIndex)
{
    if((m_nOverrideFlag & OVER_COLUMNS)||(m_nAttributes2 & STYLE2_LOCALCOLUMNINFO))
    {
        LwpLayoutColumns* pLayColumns = static_cast<LwpLayoutColumns*>(m_LayColumns.obj());
        if(pLayColumns)
        {
            return pLayColumns->GetColWidth(nIndex);
        }
    }

    LwpVirtualLayout* pStyle = static_cast<LwpVirtualLayout*> (m_BasedOnStyle.obj());
    if(pStyle)
    {
        return pStyle->GetColWidth(nIndex);
    }

    return LwpVirtualLayout::GetColWidth(nIndex);

}

/**
* @descr: 	Get gap between columns
* @param:	the order of column
*/
double LwpLayout::GetColGap(sal_uInt16 nIndex)
{
    if((m_nOverrideFlag & OVER_COLUMNS)||(m_nAttributes2 & STYLE2_LOCALCOLUMNINFO))
    {
        LwpLayoutColumns* pLayColumns = static_cast<LwpLayoutColumns*>(m_LayColumns.obj());
        if(pLayColumns)
        {
            return pLayColumns->GetColGap(nIndex);
        }
    }

    LwpVirtualLayout* pStyle = static_cast<LwpVirtualLayout*> (m_BasedOnStyle.obj());
    if(pStyle)
    {
        return pStyle->GetColGap(nIndex);
    }

    return LwpVirtualLayout::GetColGap(nIndex);
}

/**
* @descr: 	Create and return XFColumns object
*
*/
XFColumns* LwpLayout::GetXFColumns()
{
    //if there is only one column, do not need insert columns
    sal_uInt16 nCols = GetNumCols();
    if(nCols==1)
    {
        return NULL;
    }

    XFColumns* pColumns = new XFColumns();
    //set XFColumnSep
    XFColumnSep* pColumnSep = GetColumnSep();
    if(pColumnSep)
    {
        pColumns->SetSeperator(*pColumnSep);
    }

    //set column count and column gap
    pColumns->SetCount(nCols);
    double fGap = GetColGap(0);
    pColumns->SetGap(fGap);

    //set xfcolumn
    for(sal_uInt16 nIndex = 0; nIndex<nCols; nIndex++)
    {
        XFColumn* pColumn = new XFColumn();
        sal_Int32 nWidth = static_cast<sal_Int32>(GetColWidth(nIndex));
        nWidth=8305/nCols;	//relative width
        pColumn->SetRelWidth(nWidth);

        //the left and right margins is 0;
        double nGap = GetColGap(nIndex)/2;
        //nGap=0;
        pColumn->SetMargins(nGap,nGap);
        if(nIndex==0)
        {
            pColumn->SetMargins(0,nGap);
        }
        if(nIndex==(nCols-1))
        {
            pColumn->SetMargins(nGap,0);
        }
        pColumns->AddColumn(*pColumn);
    }

    return pColumns;
}

/**
* @descr: 	Create and return XFColumnSep object
*
*/
XFColumnSep* LwpLayout::GetColumnSep()
{

    //Get LwpLayoutGutters
    LwpLayoutGutters* pLayoutGutters = static_cast<LwpLayoutGutters*>(m_LayGutterStuff.obj());
    if(!pLayoutGutters)
    {
        return NULL;
    }

    LwpBorderStuff* pBorderStuff = pLayoutGutters->GetBorderStuff();

    if(pBorderStuff)
    {
        LwpBorderStuff::BorderType eType = LwpBorderStuff::LEFT;
        LwpColor	aColor = pBorderStuff->GetSideColor(eType);
        double	fWidth = pBorderStuff->GetSideWidth(eType);
        //sal_uInt16	nType = pBorderStuff->GetSideType(eType);

        XFColumnSep* pColumnSep = new XFColumnSep();
        XFColor aXFColor(aColor.To24Color());
        pColumnSep->SetColor(aXFColor);
        pColumnSep->SetWidth(fWidth);
        pColumnSep->SetRelHeight(100);
        pColumnSep->SetVerticalAlign(enumXFAlignTop);

        return pColumnSep;
    }
    return NULL;

}

/**
* @descr: 	Get use when type
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
* @descr: 	Get use page
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
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpLayout* pLay = static_cast<LwpLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetUsePage();
    }
    return 0;
}

/**
* @descr: 	Get usewhen pointer
*
*/
LwpUseWhen* LwpLayout::VirtualGetUseWhen()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        return m_pUseWhen;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpLayout* pLay = static_cast<LwpLayout*> ( m_BasedOnStyle.obj() );
        return pLay->VirtualGetUseWhen();
    }
    return LwpVirtualLayout::VirtualGetUseWhen();
}

/**
* @descr: 	Whether it is use on all pages
*
*/
sal_Bool LwpLayout::IsUseOnAllPages()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->IsUseOnAllPages();
        else
            return sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpLayout* pLay = static_cast<LwpLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsUseOnAllPages();
    }
    return LwpVirtualLayout::IsUseOnAllPages();
}

/**
* @descr: 	Whether it is use on all even pages
*
*/
sal_Bool LwpLayout::IsUseOnAllEvenPages()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->IsUseOnAllEvenPages();
        else
            return sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpLayout* pLay = static_cast<LwpLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsUseOnAllEvenPages();
    }
    return LwpVirtualLayout::IsUseOnAllEvenPages();
}

/**
* @descr: 	Whether it is use on all odd pages
*
*/
sal_Bool LwpLayout::IsUseOnAllOddPages()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->IsUseOnAllOddPages();
        else
            return sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpLayout* pLay = static_cast<LwpLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsUseOnAllOddPages();
    }
    return LwpVirtualLayout::IsUseOnAllOddPages();
}

/**
* @descr: 	Whether it is use on current page
*
*/
sal_Bool LwpLayout::IsUseOnPage()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        LwpUseWhen* pUseWhen = GetUseWhen();
        if(pUseWhen)
            return pUseWhen->IsUseOnPage();
        else
            return sal_False;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpLayout* pLay = static_cast<LwpLayout*> ( m_BasedOnStyle.obj() );
        return pLay->IsUseOnPage();
    }
    return LwpVirtualLayout::IsUseOnPage();
}

/**
 * @descr:  Get the LwpShadow object according to m_LayShadow id.
 * @param:
 * @param:
 * @return:
*/
LwpShadow* LwpLayout::GetShadow()
{
    if(m_nOverrideFlag & OVER_SHADOW)
    {
        LwpLayoutShadow* pLayoutShadow = static_cast<LwpLayoutShadow*>(m_LayShadow.obj());
        return pLayoutShadow->GetShadow();
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpLayout* pLay = static_cast<LwpLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetShadow();
    }
    return NULL;
}

/**
 * @descr:  create xfshadow
 * @param:
 * @param:
 * @return:
*/
XFShadow* LwpLayout::GetXFShadow()
{
    LwpShadow* pShadow = GetShadow();
    if( pShadow )
    {
        LwpColor color = pShadow->GetColor();
        double	offsetX = pShadow->GetOffsetX();
        double	offsetY = pShadow->GetOffsetY();

        if( offsetX && offsetY && color.IsValidColor() )
        {
            XFShadow* pXFShadow = new XFShadow();
            enumXFShadowPos eXFShadowPos = enumXFShadowLeftTop;
            double fOffset = 0;

            sal_Bool left = sal_False;
            sal_Bool top = sal_False;
            if( offsetX < 0 )
                left = sal_True;
            if( offsetY < 0 )
                top = sal_True;
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
    return NULL;
}

/**
 * @descr get the layout that containers the current frame layout
 *
 */
LwpVirtualLayout* LwpLayout::GetContainerLayout()
{
    if(IsRelativeAnchored())
    {
        //get position
        LwpPara* pPara = static_cast<LwpPara*>(GetPosition()->obj());
        if(pPara)
        {
            LwpStory* pStory = pPara->GetStory();
            return pStory->GetTabLayout();
        }
    }
    return GetParentLayout();
}

LwpPlacableLayout::LwpPlacableLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm )
    : LwpLayout(objHdr, pStrm),m_pFont(NULL)
{}

LwpPlacableLayout::~LwpPlacableLayout()
{}

void LwpPlacableLayout::Read()
{
    LwpObjectStream* pStrm = m_pObjStrm;
    LwpLayout::Read();
    if(LwpFileHeader::m_nFileRevision < 0x000B)
    {
        assert(false);
    }
    else
    {
        sal_uInt16 simple;
        pStrm->QuickRead(&simple, sizeof(simple));
        if(!simple)
        {
            pStrm->QuickRead(&m_nWrapType, sizeof(m_nWrapType));
            pStrm->QuickRead(&m_nBuoyancy, sizeof(m_nBuoyancy));
            pStrm->QuickRead(&m_nBaseLineOffset, sizeof(m_nBaseLineOffset));
            m_Script.Read( pStrm);
        }
        else
        {
            m_nWrapType = LAY_WRAP_AROUND;
            m_nBuoyancy = LAY_BUOYNEUTRAL;
            m_nBaseLineOffset	 = 0;
        }
        m_LayRelativity.ReadIndexed( pStrm);
        if(pStrm->CheckExtra())
        {
            sal_uInt16 count;
            pStrm->QuickRead(&count, sizeof(count));
            if(count)
            {
                // temporily added by  to avoid assertion
                while (count)
                {
                    LwpPoint aPoint;
                    aPoint.Read(pStrm);
                    count--;
                }
                // end added by
            }
            pStrm->SkipExtra();
        }
    }
}
/**
 * @descr:  get wrap type
 * @param:
 * @param:
 * @return:
*/
sal_uInt8 LwpPlacableLayout::GetWrapType()
{
    if(m_nOverrideFlag & OVER_PLACEMENT)
    {
        return m_nWrapType;
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpPlacableLayout* pLay = static_cast<LwpPlacableLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetWrapType();
    }
    return LAY_WRAP_AROUND;
}
/**
 * @descr:  get LayoutRelativity
 * @param:
 * @param:
 * @return:
*/
LwpLayoutRelativity* LwpPlacableLayout::GetRelativityPiece()
{
    if(!m_LayRelativity.IsNull())
    {
        if(m_nOverrideFlag & OVER_PLACEMENT)
        {
            return static_cast<LwpLayoutRelativity*>(m_LayRelativity.obj());
        }
    }
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpPlacableLayout* pLay = static_cast<LwpPlacableLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetRelativityPiece();
    }
    return NULL;
}
/**
* @descr: 	Get relative type
*
*/
sal_uInt8 LwpPlacableLayout::GetRelativeType()
{
    LwpLayoutRelativity* pLayRel = GetRelativityPiece();
    if(pLayRel)
    {
        return pLayRel->GetRelGuts()->GetRelativeType();
    }
    return LwpVirtualLayout::GetRelativeType();
}
/**
* @descr: 	Get relative from where type
*
*/
sal_uInt8 LwpPlacableLayout::GetRelativeFromWhere()
{
    LwpLayoutRelativity* pLayRel = GetRelativityPiece();
    if(pLayRel)
    {
        return pLayRel->GetRelGuts()->GetRelativeFromWhere();
    }
    return 0;
}
/**
* @descr: 	Get relative distance
*
*/
LwpPoint LwpPlacableLayout::GetRelativeDistance()
{
    LwpPoint aPoint;
    LwpLayoutRelativity* pLayRel = GetRelativityPiece();
    if(pLayRel)
    {
        aPoint = pLayRel->GetRelGuts()->GetRelativeDistance();
    }
    return aPoint;
}
/**
* @descr: 	Get tether type
*
*/
sal_uInt8 LwpPlacableLayout::GetTetherType()
{
    LwpLayoutRelativity* pLayRel = GetRelativityPiece();
    if(pLayRel)
    {
        return pLayRel->GetRelGuts()->GetTetherType();
    }
    return 0;
}
/**
* @descr: 	Get tether where type
*
*/
sal_uInt8 LwpPlacableLayout::GetTetherWhere()
{
    LwpLayoutRelativity* pLayRel = GetRelativityPiece();
    if(pLayRel)
    {
        return pLayRel->GetRelGuts()->GetTetherWhere();
    }
    return 0;
}
/**
* @descr: 	Get offset from the baseline
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
    else if( !m_BasedOnStyle.IsNull() )
    {
        LwpPlacableLayout* pLay = static_cast<LwpPlacableLayout*> ( m_BasedOnStyle.obj() );
        return pLay->GetBaseLineOffset();
    }
    return 0;


}
/**
* @descr: 	whether the parent layout is page layout
*
*/
sal_Bool LwpPlacableLayout::IsAnchorPage()
{
    if(IsRelativeAnchored())
        return sal_False;

    LwpVirtualLayout* pLayout = GetParentLayout();
    if(pLayout && (pLayout->IsPage() || pLayout->IsHeader() || pLayout->IsFooter()))
    //if(pLayout && pLayout->IsPage())
    {
        return sal_True;
    }
    return sal_False;
}
/**
* @descr: 	whether the parent layout is frame layout
*
*/
sal_Bool LwpPlacableLayout::IsAnchorFrame()
{
    if(IsRelativeAnchored())
        return sal_False;

    LwpVirtualLayout* pLayout = GetParentLayout();
    if(pLayout && (pLayout->IsFrame()||pLayout->IsGroupHead()))
    {
        return sal_True;
    }
    return sal_False;
}
/**
* @descr: 	whether the parent layout is cell layout
*
*/
sal_Bool LwpPlacableLayout::IsAnchorCell()
{
    if(IsRelativeAnchored())
        return sal_False;

    LwpVirtualLayout* pLayout = GetParentLayout();
    if(pLayout && pLayout->IsCell())
    {
        return sal_True;
    }
    return sal_False;
}

/**
* @descr: 	Get font style for setting position of frame
*
*/
XFFont* LwpPlacableLayout::GetFont()
{
    return m_pFont;
}
/**
* @descr: 	Set font style for setting position of frame
*
*/
void LwpPlacableLayout::SetFont(XFFont * pFont)
{
    m_pFont = pFont;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
