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
 *  the class for VO_FrameLayout
 ************************************************************************/

#include <memory>
#include "lwpframelayout.hxx"
#include "lwppara.hxx"
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xfparagraph.hxx>
#include <xfilter/xffloatframe.hxx>
#include <xfilter/xfrubystyle.hxx>
#include "lwppagelayout.hxx"
#include "lwpoleobject.hxx"
#include "lwptablelayout.hxx"
#include "lwpgrfobj.hxx"
#include <lwpglobalmgr.hxx>

LwpFrame::LwpFrame(LwpPlacableLayout* pLayout):m_pLayout(pLayout)
{
}

LwpFrame::~LwpFrame()
{
}
/**
* @descr:  parse frame
* @param:  register frame style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::RegisterStyle(std::unique_ptr<XFFrameStyle>& rFrameStyle)
{
    ApplyWrapType(rFrameStyle.get());
    ApplyMargins(rFrameStyle.get());
    ApplyPadding(rFrameStyle.get());
    ApplyBorders(rFrameStyle.get());
    ApplyColumns(rFrameStyle.get());
    ApplyShadow(rFrameStyle.get());
    ApplyBackGround(rFrameStyle.get());
    ApplyWatermark(rFrameStyle.get());
    ApplyProtect(rFrameStyle.get());
    ApplyTextDir(rFrameStyle.get());
    ApplyPosType(rFrameStyle.get());

    rFrameStyle->SetStyleName(m_pLayout->GetName().str());
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_StyleName = pXFStyleManager->AddStyle(std::move(rFrameStyle)).m_pStyle->GetStyleName();
    m_pLayout->SetStyleName(m_StyleName);
}

/**
* @descr:  parse frame and set frame properties
* @param:   pXFFrame - XFFrame object
* @param:  nPageNo - the page number that the frame anchors
*
*/
 void LwpFrame::Parse(XFFrame* pXFFrame, sal_Int32 nPageNo)
 {
    //set the frame style name
    pXFFrame->SetStyleName(m_StyleName);

    //SetAnchorType and position,if it's page anchor,set the page number.
    ParseAnchorType(pXFFrame);
    if(nPageNo>0)
    {
        pXFFrame->SetAnchorPage(nPageNo);
    }

    //Set frame Name
    OUString aFrameName = m_pLayout->GetName().str();
    if(!aFrameName.isEmpty())
    {
        //cause the bug of SODC, the linkframe name can not be "Frame1", so I change the frame name
        /*if(aFrameName.equals("Frame1"))
        {
            aFrameName = "Frame1_COPY";
        }
        pXFFrame->SetName(aFrameName);*/
        pXFFrame->SetName(m_StyleName);
    }

    LwpLayoutGeometry* pLayoutGeo = m_pLayout->GetGeometry();
    //Set frame Width and height
    if(pLayoutGeo)
    {
        double fWidth = m_pLayout->GetWidth();
        double fHeight = m_pLayout->GetHeight();

        pXFFrame->SetWidth( fWidth );
        pXFFrame->SetHeight( fHeight );

        //Get content obj;
        /*LwpObject* pObj =*/ m_pLayout->GetContent().obj();
        if(m_pLayout->IsGroupHead()&&(m_pLayout->IsMinimumHeight()))
        {
            //process grouplayout height. there is problems now
            pXFFrame->SetHeight( fHeight );
        }
        /*
        else if(m_pLayout->IsFitGraphic() && pObj && pObj->GetTag() == VO_GRAPHIC)
        {
            //If is graphic, get original size and set it;
            LwpGraphicObject* pGrpObj = static_cast<LwpGraphicObject*>(pObj);
            long nHeight =0, nWidth =0;
            pGrpObj->GetGrafOrgSize(nWidth, nHeight);
            //add margins to the width and height;
            fWidth = (double)nWidth/TWIPS_PER_CM + m_pLayout->GetMarginsValue(MARGIN_LEFT) + m_pLayout->GetMarginsValue(MARGIN_RIGHT);
            fHeight = (double)nHeight/TWIPS_PER_CM + m_pLayout->GetMarginsValue(MARGIN_TOP) + m_pLayout->GetMarginsValue(MARGIN_BOTTOM);
            pXFFrame->SetWidth(fWidth);
            pXFFrame->SetHeight(fHeight);
        }
        */
        else if(m_pLayout->IsAutoGrow())
        {
            pXFFrame->SetMinHeight( fHeight );
        }
    }

    if(m_pLayout->IsFrame())
    {
        //Set frame link. Only frame layout has this feature
        LwpFrameLayout* pLayout= static_cast<LwpFrameLayout*>(m_pLayout);
        pXFFrame->SetNextLink(pLayout->GetNextLinkName());
    }

 }
/**
* @descr:  parse frame relative to page, frame or cell
* @param:   pCont - content container which contains the frame
*
*/
 void LwpFrame::XFConvert(XFContentContainer* pCont)
 {
    // parse the frame which anchor to page
    rtl::Reference<LwpVirtualLayout> xParent = m_pLayout->GetParentLayout();
    if (!xParent.is())
        throw std::runtime_error("missing Parent Layout");
    if (xParent->IsPage() && xParent->GetParentLayout().is() && xParent->GetParentLayout()->IsPage())
    {
        //for mirror page, problems exist if the parent layout is header or footer layout,
        xParent = xParent->GetParentLayout();
    }
    if(m_pLayout->IsAnchorPage()&& xParent->IsPage())
    {
        //get parent layout
        if(m_pLayout->IsUseOnPage())
        {
            sal_Int32 nPageNo = xParent->GetPageNumber(m_pLayout->GetUsePage());
            if(nPageNo>0)
                m_pLayout->XFConvertFrame(pCont, nPageNo);
        }
        else if(m_pLayout->IsUseOnAllPages())
        {
            sal_Int32 nFirst = xParent->GetPageNumber(FIRST_LAYOUTPAGENO);
            sal_Int32 nLast = xParent->GetPageNumber(LAST_LAYOUTPAGENO);
            if(nLast > 0)
                m_pLayout->XFConvertFrame(pCont, nFirst, nLast, true);

        }
        else if(m_pLayout->IsUseOnAllOddPages()||m_pLayout->IsUseOnAllEvenPages())
        {
            sal_Int32 nFirst = xParent->GetPageNumber(FIRST_LAYOUTPAGENO);
            sal_Int32 nLast = xParent->GetPageNumber(LAST_LAYOUTPAGENO);
            if(nLast > 0)
            {
                sal_uInt16 first = static_cast<sal_uInt16>(nFirst);
                if((m_pLayout->IsUseOnAllOddPages() && !LwpTools::IsOddNumber(first))
                || (m_pLayout->IsUseOnAllEvenPages() && !LwpTools::IsEvenNumber(first)))
                    nFirst++;
                if(nFirst <= nLast)
                {
                    m_pLayout->XFConvertFrame(pCont, nFirst, nLast);
                }
            }
        }
    }
    else
    {
        m_pLayout->XFConvertFrame(pCont);
    }

 }
/**
* @descr:  set frame wrap type style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyWrapType(XFFrameStyle *pFrameStyle)
{
    enumXFWrap eWrap = enumXFWrapNone;
    switch(m_pLayout->GetWrapType())
    {
        case LwpPlacableLayout::LAY_WRAP_AROUND:    //fall through
        case LwpPlacableLayout::LAY_WRAP_IRREG_BIGGEST:
        {
            //In SODC, if Optimal wrap type is used and the distance between the frame object
            //and page margins is less than 2cm, the text is not wrapped. While there is no this feature in Word Pro
            //So the optimal wrap type is translated to left side or right side wrap type according to the distance
            //between the frame object and page margins

            eWrap = enumXFWrapBest;
            rtl::Reference<LwpVirtualLayout> xContainer(m_pLayout->GetContainerLayout());
            LwpMiddleLayout* pParent = dynamic_cast<LwpMiddleLayout*>(xContainer.get());
            if(pParent)
            {
                if(IsLeftWider())
                    eWrap = enumXFWrapLeft;
                else
                    eWrap = enumXFWrapRight;
            }
            break;
        }
        case LwpPlacableLayout::LAY_NO_WRAP_BESIDE:
        {
            eWrap = enumXFWrapNone;
            break;
        }
        case LwpPlacableLayout::LAY_NO_WRAP_AROUND:
        {
            eWrap = enumXFWrapRunThrough;
            if(!m_pLayout->GetBackColor() && !m_pLayout->GetWaterMarkLayout().is())
            {
                //pFrameStyle->SetBackGround(sal_True);
                XFColor aXFColor(0xffffff); //white color
                pFrameStyle->SetBackColor(aXFColor);
                pFrameStyle->SetTransparency(100);  //transparency
            }
            break;
        }
        case LwpPlacableLayout::LAY_WRAP_LEFT:      //fall through
        case LwpPlacableLayout::LAY_WRAP_IRREG_LEFT:
        {
            eWrap = enumXFWrapLeft;
            break;
        }
        case LwpPlacableLayout::LAY_WRAP_RIGHT: //fall through
        case LwpPlacableLayout::LAY_WRAP_IRREG_RIGHT:
        {
            eWrap = enumXFWrapRight;
            break;
        }
        case LwpPlacableLayout::LAY_WRAP_BOTH:  //fall through
        case LwpPlacableLayout::LAY_WRAP_IRREG_BOTH:
        {
            eWrap = enumXFWrapParallel;
            break;
        }
        default:
            break;
    }

    //If it is the type of with para above, wrap type is enumXFWrapNone
    if(m_pLayout->GetRelativeType()==LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE)
    {
        eWrap = enumXFWrapNone;
    }

    pFrameStyle->SetWrapType(eWrap);
}
/**
* @descr:   set frame margins style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyMargins(XFFrameStyle *pFrameStyle)
{
    double fLeft    = m_pLayout->GetExtMarginsValue(MARGIN_LEFT);
    double fRight   = m_pLayout->GetExtMarginsValue(MARGIN_RIGHT);
    double fTop = m_pLayout->GetExtMarginsValue(MARGIN_TOP);
    double fBottom  = m_pLayout->GetExtMarginsValue(MARGIN_BOTTOM);
    pFrameStyle->SetMargins(fLeft,fRight,fTop,fBottom);
}
/**
* @descr:  set padding border style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyPadding(XFFrameStyle *pFrameStyle)
{
    double fLeft    = m_pLayout->GetMarginsValue(MARGIN_LEFT);
    double fRight   = m_pLayout->GetMarginsValue(MARGIN_RIGHT);
    double fTop = m_pLayout->GetMarginsValue(MARGIN_TOP);
    double fBottom  = m_pLayout->GetMarginsValue(MARGIN_BOTTOM);
    pFrameStyle->SetPadding(fLeft,fRight,fTop,fBottom);
}
/**
* @descr:  set frame border style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyBorders(XFFrameStyle *pFrameStyle)
{
    std::unique_ptr<XFBorders> pBordres = m_pLayout->GetXFBorders();
    if(pBordres)
    {
        pFrameStyle->SetBorders(std::move(pBordres));
    }
}
/**
* @descr:  set frame columns style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyColumns(XFFrameStyle *pFrameStyle)
{
    XFColumns* pColumns = m_pLayout->GetXFColumns();
    if(pColumns)
    {
        pFrameStyle->SetColumns(pColumns);
    }
}
/**
* @descr:  set frame shadow style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyShadow(XFFrameStyle* pFrameStyle)
{
    XFShadow* pXFShadow = m_pLayout->GetXFShadow();
    if(pXFShadow)
    {
        pFrameStyle->SetShadow(pXFShadow);
    }
}
/**
* @descr:  set frame back color style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyBackColor(XFFrameStyle* pFrameStyle)
{
    LwpColor* pColor = m_pLayout->GetBackColor();
    if(pColor)
    {
        XFColor aXFColor(pColor->To24Color());
        pFrameStyle->SetBackColor(aXFColor);
    }
}
/**
* @descr:  set frame protect style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyProtect(XFFrameStyle* pFrameStyle)
{
    if(m_pLayout->GetIsProtected())
    {
        pFrameStyle->SetProtect(true,true,true);
    }
}
/**
* @descr:  set frame text direction style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyTextDir(XFFrameStyle* pFrameStyle)
{
    pFrameStyle->SetTextDir(m_pLayout->GetTextDirection());
}
/**
* @descr:  set frame position type style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyPosType(XFFrameStyle* pFrameStyle)
{
    enumXFFrameXPos eXPos = enumXFFrameXPosCenter;
    enumXFFrameXRel eXRel = enumXFFrameXRelPara;
    enumXFFrameYPos eYPos = enumXFFrameYPosMiddle;
    enumXFFrameYRel eYRel = enumXFFrameYRelPara;
    sal_uInt8 nType = m_pLayout->GetRelativeType();
    switch(nType)
    {
        case LwpLayoutRelativityGuts::LAY_PARENT_RELATIVE://fall through
        case LwpLayoutRelativityGuts::LAY_CONTENT_RELATIVE:
        {
            //anchor to page, frame and cell
            eXPos = enumXFFrameXPosFromLeft;
            eXRel = enumXFFrameXRelPage;
            //set vertical position
            if(m_pLayout->IsAnchorPage())//in page
            {
                rtl::Reference<LwpVirtualLayout> xContainer(m_pLayout->GetContainerLayout());
                if (xContainer.is() && (xContainer->IsHeader() || xContainer->IsFooter()))
                {
                    //Only anchor to para, the frame can display in header and footer of each page
                    eYPos = enumXFFrameYPosFromTop; //from top
                    eYRel = enumXFFrameYRelPara; //from margin
                }
                else
                {
                    eYPos = enumXFFrameYPosFromTop;
                    eYRel = enumXFFrameYRelPage;
                }
            }
            if(m_pLayout->IsAnchorFrame()) //in frame
            {
                eYPos = enumXFFrameYPosFromTop;
                eYRel = enumXFFrameYRelPage;
            }
            if(m_pLayout->IsAnchorCell())
            {
                //SODC has no this type, simulate this feature
                eYPos = enumXFFrameYPosFromTop; //from top
                eYRel = enumXFFrameYRelPara; //from margin
            }
            break;
        }
        case LwpLayoutRelativityGuts::LAY_PARA_RELATIVE:    //same page as text
        {
            eXPos = enumXFFrameXPosFromLeft;
            eXRel = enumXFFrameXRelPage;
            //set vertical position
            rtl::Reference<LwpVirtualLayout> xContainer(m_pLayout->GetContainerLayout());
            if (xContainer.is() && xContainer->IsPage())//in page
            {
                //eYPos = enumXFFrameYPosFromTop;
                //eYRel = enumXFFrameYRelPage;
                eYPos = enumXFFrameYPosBelow;
                eYRel = enumXFFrameYRelChar;
            }
            else if (xContainer.is() && xContainer->IsFrame()) //in frame
            {
                eYPos = enumXFFrameYPosFromTop;
                eYRel = enumXFFrameYRelPage;
            }
            else
            {
                eYPos = enumXFFrameYPosFromTop; //from top
                eYRel = enumXFFrameYRelPara; //from margin
            }
            break;
        }
        case LwpLayoutRelativityGuts::LAY_INLINE:   //in text
        {
            eXPos = enumXFFrameXPosFromLeft;    //need not be set
            eXRel = enumXFFrameXRelParaContent; //need not be set
            eYPos = enumXFFrameYPosTop; //should be from top
            eYRel = enumXFFrameYRelBaseLine;
            sal_Int32 nOffset = m_pLayout->GetBaseLineOffset();
            if(nOffset>0)
            {
                eYPos = enumXFFrameYPosFromTop;
            }
            break;
        }
        case LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE:   //with para above
        {
            eXPos = enumXFFrameXPosFromLeft;
            eXRel = enumXFFrameXRelParaContent;
            //eYPos = enumXFFrameYPosTop;
            eYPos = enumXFFrameYPosBottom;
            eYRel = enumXFFrameYRelParaContent;
            break;
        }
        case LwpLayoutRelativityGuts::LAY_INLINE_VERTICAL:  //in text - vertical
        {
            eXPos = enumXFFrameXPosFromLeft;
            eXRel = enumXFFrameXRelPage;
            eYPos = enumXFFrameYPosFromTop; //should be below position
            eYRel = enumXFFrameYRelChar;
            break;
        }
        default:
            break;
    }

    pFrameStyle->SetXPosType(eXPos,eXRel);
    pFrameStyle->SetYPosType(eYPos,eYRel);
}
/**
* @descr:  set frame watermark style
* @param:  pFrameStyle - Frame Style object
*
*/
void LwpFrame::ApplyWatermark(XFFrameStyle *pFrameStyle)
{
    std::unique_ptr<XFBGImage> xBGImage(m_pLayout->GetXFBGImage());
    if (xBGImage)
    {
        pFrameStyle->SetBackImage(xBGImage);
        //set watermark transparent
        rtl::Reference<LwpVirtualLayout> xWaterMarkLayout(m_pLayout->GetWaterMarkLayout());
        LwpMiddleLayout* pLay = dynamic_cast<LwpMiddleLayout*>(xWaterMarkLayout.get());
        LwpBackgroundStuff* pBackgroundStuff = pLay ? pLay->GetBackgroundStuff() : nullptr;
        if(pBackgroundStuff && !pBackgroundStuff->IsTransparent())
        {
            pFrameStyle->SetTransparency(100);
        }
     }
}

/**
 * @short   Apply pattern fill to frame style
 * @param pFrameStyle - pointer of XFFrameStyle
 * @return
 */
void LwpFrame::ApplyPatternFill(XFFrameStyle* pFrameStyle)
{
    std::unique_ptr<XFBGImage> xXFBGImage(m_pLayout->GetFillPattern());
    if (xXFBGImage)
    {
        pFrameStyle->SetBackImage(xXFBGImage);
    }
}

/**
 * @short   Apply background to frame style
 * @param pFrameStyle - pointer of XFFrameStyle
 * @return
 */
void LwpFrame::ApplyBackGround(XFFrameStyle* pFrameStyle)
{
    if (!m_pLayout)
    {
        return;
    }

    if (m_pLayout->IsPatternFill())
    {
        ApplyPatternFill(pFrameStyle);
    }
    else
    {
        ApplyBackColor(pFrameStyle);
    }
}

/**
* @descr:  set frame size, anchor type, anchored page number
* @param:  pXFFrame - XFFrame  object
*
*/
void LwpFrame::ParseAnchorType(XFFrame *pXFFrame)
{
    //set position
    double fXOffset = 0;
    double fYOffset = 0;
    //set anchor type
    enumXFAnchor eAnchor = enumXFAnchorNone;

    LwpLayoutGeometry* pLayoutGeo = m_pLayout->GetGeometry();
    if(pLayoutGeo)
    {
        LwpPoint aPoint = pLayoutGeo->GetOrigin();
        fXOffset = LwpTools::ConvertFromUnitsToMetric(aPoint.GetX());
        fYOffset = LwpTools::ConvertFromUnitsToMetric(aPoint.GetY());
    }
    //set anchor type
    eAnchor = enumXFAnchorNone;
    sal_uInt8 nType = m_pLayout->GetRelativeType();
    switch(nType)
    {
        case LwpLayoutRelativityGuts::LAY_PARENT_RELATIVE://fall through
        case LwpLayoutRelativityGuts::LAY_CONTENT_RELATIVE:
        {
            //anchor to page, frame and cell
            if(m_pLayout->IsAnchorPage())//in page
            {
                rtl::Reference<LwpVirtualLayout> xContainer(m_pLayout->GetContainerLayout());
                if (xContainer.is() && (xContainer->IsHeader() || xContainer->IsFooter()))
                {
                    eAnchor = enumXFAnchorPara;
                    fYOffset -= xContainer->GetMarginsValue(MARGIN_TOP);
                }
                else
                    eAnchor = enumXFAnchorPage;
            }
            if(m_pLayout->IsAnchorFrame()) //in frame
            {
                eAnchor = enumXFAnchorFrame;
            }
            if(m_pLayout->IsAnchorCell())   //in cell
            {
                //eAnchor = enumXFAnchorChar;
                eAnchor = enumXFAnchorPara;
                rtl::Reference<LwpVirtualLayout> xContainer(m_pLayout->GetContainerLayout());
                LwpMiddleLayout* pContainer = dynamic_cast<LwpMiddleLayout*>(xContainer.get());
                if (pContainer)
                {
                    fYOffset -= pContainer->GetMarginsValue(MARGIN_TOP);
                }
            }
            break;
        }
        case LwpLayoutRelativityGuts::LAY_PARA_RELATIVE:    //same page as text
        {
            eAnchor = enumXFAnchorChar;
            rtl::Reference<LwpVirtualLayout> xContainer(m_pLayout->GetContainerLayout());
            if (xContainer.is() && xContainer->IsPage())//in page
            {
                //eAnchor = enumXFAnchorPage;
                eAnchor = enumXFAnchorChar;// to character
            }
            else if (xContainer.is() && xContainer->IsFrame()) //in frame
            {
                eAnchor = enumXFAnchorFrame;
            }
            else if (xContainer.is() && xContainer->IsCell()) //in cell
            {
                //eAnchor = enumXFAnchorChar;
                eAnchor = enumXFAnchorPara;
                fYOffset -= xContainer->GetMarginsValue(MARGIN_TOP);
            }
            else if (xContainer.is() && (xContainer->IsHeader() || xContainer->IsFooter()))//in header or footer
            {
                eAnchor = enumXFAnchorPara;
                fYOffset -= xContainer->GetMarginsValue(MARGIN_TOP);
            }
            break;
        }
        case LwpLayoutRelativityGuts::LAY_INLINE:   //in text
        {
            eAnchor = enumXFAnchorAsChar;
            sal_Int32 nOffset = m_pLayout->GetBaseLineOffset();
            if(nOffset>0 && pLayoutGeo)
            {
                //experiential value
                fYOffset =-(m_pLayout->GetGeometryHeight()+2*m_pLayout->GetExtMarginsValue(MARGIN_BOTTOM)-LwpTools::ConvertFromUnitsToMetric(nOffset));
            }
            break;
        }
        case LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE:   //with para above
        {
            eAnchor = enumXFAnchorPara;
            break;
        }
        case LwpLayoutRelativityGuts::LAY_INLINE_VERTICAL:  //in text - vertical
        {
            eAnchor = enumXFAnchorChar;
            //set vertical position
            double offset = 0;

            //because of the different feature between Word Pro and SODC, I simulate the vertical base offset
            //between anchor and frame origin using the font height.
            rtl::Reference<XFFont> pFont = m_pLayout->GetFont();
            if(pFont.is())
            {
                offset = static_cast<double>(pFont->GetFontSize())*CM_PER_INCH/POINTS_PER_INCH;
            }
            fYOffset = offset-fYOffset;
            break;
        }
        default:
            break;
    }

    pXFFrame->SetX(fXOffset);
    pXFFrame->SetY(fYOffset);
    pXFFrame->SetAnchorPage(0);
    pXFFrame->SetAnchorType(eAnchor);
}

/**
 * @descr Calculate the distance between the frame object and the page margins.
 *       And determine which side(left or right) is wider
 */
bool LwpFrame::IsLeftWider()
{
    rtl::Reference<LwpVirtualLayout> xLayout(m_pLayout->GetContainerLayout());
    LwpVirtualLayout* pParent = xLayout.get();
    if (pParent)
    {
        LwpPoint aPoint = m_pLayout->GetOrigin();
        double fXOffset = LwpTools::ConvertFromUnitsToMetric(aPoint.GetX());
        double fWidth = m_pLayout->GetWidth();
        double fWrapLeft = m_pLayout->GetExtMarginsValue(MARGIN_LEFT);
        double fWrapRight = m_pLayout->GetExtMarginsValue(MARGIN_RIGHT);

        //LwpPoint aParentPoint = pParent->GetOrigin();
        //double fParentXOffset = LwpTools::ConvertFromUnitsToMetric(aParentPoint.GetX());
        double fParentWidth = pParent->GetWidth();
        if(pParent->IsCell())
        {
            //Get actual width of this cell layout
            fParentWidth = static_cast<LwpCellLayout*>(pParent)->GetActualWidth();
        }
        double fParentMarginLeft = pParent->GetMarginsValue(MARGIN_LEFT);
        double fParentMarginRight = pParent->GetMarginsValue(MARGIN_RIGHT);

        double fLeft = fXOffset - fWrapLeft -fParentMarginLeft;
        double fRight = fParentWidth - fParentMarginRight -(fXOffset + fWidth + fWrapRight);
        if(fLeft > fRight)
            return true;
    }
    return false;
}

LwpFrameLink::LwpFrameLink()
{}

LwpFrameLink::~LwpFrameLink()
{}

/**
 * @descr frame link information
 *
 */
void LwpFrameLink::Read(LwpObjectStream* pStrm)
{
    m_PreviousLayout.ReadIndexed(pStrm);
    m_NextLayout.ReadIndexed(pStrm);
    pStrm->SkipExtra();
}

LwpFrameLayout::LwpFrameLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpPlacableLayout(objHdr, pStrm), m_bGettingMaxWidth(false)
{
}

LwpFrameLayout::~LwpFrameLayout()
{
}

/**
 * @descr read frame layout object
 *
 */
void LwpFrameLayout::Read()
{
    LwpPlacableLayout::Read();
    if(LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        if(m_pObjStrm->QuickReaduInt16())
        {
            m_Link.Read(m_pObjStrm.get());
        }
    }
    m_pObjStrm->SkipExtra();
}

/**
 * @descr create a xfframe and add into content container
 * @param:  pCont - content container that contains the frame.
 *
 */
 void LwpFrameLayout::XFConvert(XFContentContainer* pCont)
 {
    if(m_pFrame)
    {
        //parse the frame which anchor to paragraph
        if(IsRelativeAnchored())
        {
            XFConvertFrame(pCont);
        }
        else
        {
            m_pFrame->XFConvert(pCont);
        }

    }
 }
/**
 * @descr create a xfframe and add into content container, called by XFConvert
 * @param:  pCont - content container that contains the frame.
 * @param:  nPageNo - the page number that the frame anchors
 *
 */
void LwpFrameLayout::XFConvertFrame(XFContentContainer* pCont, sal_Int32 nStart , sal_Int32 nEnd, bool bAll )
{
    if(m_pFrame)
    {
        rtl::Reference<XFFrame> xXFFrame;
        if(nEnd < nStart)
        {
            xXFFrame.set(new XFFrame);
        }
        else
        {
            xXFFrame.set(new XFFloatFrame(nStart, nEnd, bAll));
        }

        m_pFrame->Parse(xXFFrame.get(), nStart);
        //if it is a link frame, parse contents only once
        if(!HasPreviousLinkLayout())
        {
            rtl::Reference<LwpObject> content = m_Content.obj();
            if (content.is())
            {
                content->DoXFConvert(xXFFrame.get());
                //set frame size according to ole size
                ApplyGraphicSize(xXFFrame.get());
            }
        }
        pCont->Add(xXFFrame.get());
    }
}
/**
 * @descr register frame style
 *
 */
void  LwpFrameLayout::RegisterStyle()
{
    //if it is for water mark, don't register style
    if (IsForWaterMark())
        return;

    if (m_pFrame)
        return;

    //register frame style
    std::unique_ptr<XFFrameStyle> xFrameStyle(new XFFrameStyle);
    m_pFrame.reset(new LwpFrame(this));
    m_pFrame->RegisterStyle(xFrameStyle);

    //register content style
    rtl::Reference<LwpObject> content = m_Content.obj();
    if (content.is())
    {
        content->SetFoundry(m_pFoundry);
        content->DoRegisterStyle();
    }

    //register child frame style
    RegisterChildStyle();
}

/**
 * @descr get the name of the frame that current frame links
 *
 */
OUString LwpFrameLayout::GetNextLinkName()
{
    OUString aName;
    LwpObjectID& rObjectID = m_Link.GetNextLayout();
    if(!rObjectID.IsNull())
    {
        LwpLayout* pLayout = dynamic_cast<LwpLayout*>(rObjectID.obj().get());
        if (pLayout)
        {
            LwpAtomHolder& rHolder = pLayout->GetName();
            aName = rHolder.str();
            //for division name conflict
            if(!pLayout->GetStyleName().isEmpty())
                aName = pLayout->GetStyleName();
        }
    }
    return aName;
}
/**
 * @descr whether current frame is linked by other frame
 *
 */
bool LwpFrameLayout::HasPreviousLinkLayout()
{
    LwpObjectID& rObjectID = m_Link.GetPreviousLayout();
    return !rObjectID.IsNull();
}
/**
 * @descr whether current frame is for water mark. Problem maybe exists by this method, must be tracking
 *
 */
bool LwpFrameLayout::IsForWaterMark()
{
    if(m_nBuoyancy >=LAY_BUOYLAYER)
    {
        if (m_Content.IsNull())
            return false;
        rtl::Reference<LwpObject> content = m_Content.obj();
        if (!content.is())
            return false;
        if (content->GetTag() == VO_GRAPHIC)
            return true;
    }
    return false;
}

/**
 * @descr Get frame width
 *
 */
double LwpFrameLayout::GetWidth()
{
    double fWidth = LwpMiddleLayout::GetWidth();
    if(IsInlineToMargin() && IsAutoGrowWidth())
    {
        //for text field entry when choosing maximize field length
        fWidth = GetMaxWidth();
    }
    return fWidth;
}

/**
 * @descr Get frame width when the text field chooses maximize field length
 *
 */
double LwpFrameLayout::GetMaxWidth()
{
    if (m_bGettingMaxWidth)
        throw std::runtime_error("recursive GetMaxWidth");

    m_bGettingMaxWidth = true;
    double fActualWidth = 0;
    rtl::Reference<LwpVirtualLayout> xLayout(GetContainerLayout());
    LwpMiddleLayout* pParent = dynamic_cast<LwpMiddleLayout*>(xLayout.get());
    if (pParent)
    {
        LwpPoint aPoint = GetOrigin();
        double fXOffset = LwpTools::ConvertFromUnitsToMetric(aPoint.GetX());
        double fWrapRight = GetExtMarginsValue(MARGIN_RIGHT);

        //Get parent layout width
        double fParentWidth = pParent->GetWidth();
        if(pParent->IsCell())
        {
            //Get actual width of this cell layout
            fParentWidth = static_cast<LwpCellLayout*>(pParent)->GetActualWidth();
        }

        double fParentMarginRight = 0;
        sal_uInt8 nType = GetRelativeType();
        if(nType == LwpLayoutRelativityGuts::LAY_INLINE
            || nType == LwpLayoutRelativityGuts::LAY_INLINE_NEWLINE )
        {
            fParentMarginRight = pParent->GetMarginsValue(MARGIN_RIGHT);
        }

        fActualWidth = fParentWidth - fXOffset - fParentMarginRight - fWrapRight;
    }

    m_bGettingMaxWidth = false;
    return fActualWidth;
}

/**
 * @descr Set frame size according to graphic size
 *
 */
void LwpFrameLayout::ApplyGraphicSize(XFFrame * pXFFrame)
{
    rtl::Reference<LwpObject> content = m_Content.obj();
    if(content.is() && (content->GetTag() == VO_GRAPHIC
                || content->GetTag() == VO_OLEOBJECT ))
    {
        LwpGraphicOleObject* pGraOle = static_cast<LwpGraphicOleObject*>(content.get());
        //Get frame geometry size
        double fWidth = 0;
        double fHeight = 0;
        pGraOle->GetGrafScaledSize(fWidth, fHeight);
        if( IsFitGraphic())
        {
            //graphic scaled sze
            fWidth += GetMarginsValue(MARGIN_LEFT) + GetMarginsValue(MARGIN_RIGHT);
            fHeight += GetMarginsValue(MARGIN_TOP) + GetMarginsValue(MARGIN_BOTTOM);
        }
        else if(IsAutoGrowDown() || IsAutoGrowUp())
        {
            fWidth = GetWidth();
            fHeight += GetMarginsValue(MARGIN_TOP) + GetMarginsValue(MARGIN_BOTTOM);
        }
        else if( IsAutoGrowLeft() || IsAutoGrowRight())
        {
            fHeight = GetHeight();
            fWidth += GetMarginsValue(MARGIN_LEFT) + GetMarginsValue(MARGIN_RIGHT);
        }
        else
        {
            fWidth = GetWidth();
            fHeight = GetHeight();
        }
        pXFFrame->SetWidth(fWidth);
        pXFFrame->SetHeight(fHeight);
    }
}

LwpGroupLayout::LwpGroupLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpPlacableLayout(objHdr, pStrm)
{

}

LwpGroupLayout::~LwpGroupLayout()
{
}
/**
 * @descr read group layout object
 *
 */
void LwpGroupLayout::Read()
{
    LwpPlacableLayout::Read();
    m_pObjStrm->SkipExtra();
}
/**
 * @descr register group frame style
 *
 */
void LwpGroupLayout::RegisterStyle()
{
    if (m_pFrame)
        return;

    //register frame style
    std::unique_ptr<XFFrameStyle> xFrameStyle(new XFFrameStyle);
    m_pFrame.reset(new LwpFrame(this));
    m_pFrame->RegisterStyle(xFrameStyle);

    //register child frame style
    RegisterChildStyle();
}
/**
 * @descr create a xfframe and add into content container
 * @param:  pCont - content container that contains the frame.
 *
 */
void LwpGroupLayout::XFConvert(XFContentContainer *pCont)
{
    if(m_pFrame)
    {
        //parse the frame which anchor to paragraph
        if(IsRelativeAnchored())
        {
            XFConvertFrame(pCont);
        }
        else
        {
            m_pFrame->XFConvert(pCont);
        }

    }
}
/**
 * @descr create a xfframe and add into content container, called by XFConvert
 * @param:  pCont - content container that contains the frame.
 * @param:  nPageNo - the page number that the frame anchors
 *
 */
void LwpGroupLayout::XFConvertFrame(XFContentContainer* pCont, sal_Int32 nStart , sal_Int32 nEnd, bool bAll)
{
    if(m_pFrame)
    {
        rtl::Reference<XFFrame> xXFFrame;
        if(nEnd < nStart)
        {
            xXFFrame.set(new XFFrame);
        }
        else
        {
            xXFFrame.set(new XFFloatFrame(nStart, nEnd, bAll));
        }

        m_pFrame->Parse(xXFFrame.get(), nStart);

        //add child frame into group
        LwpVirtualLayout* pLayout = dynamic_cast<LwpVirtualLayout*>(GetChildHead().obj().get());

        while (pLayout && pLayout != this)
        {
            pLayout->DoXFConvert(xXFFrame.get());
            pLayout = dynamic_cast<LwpVirtualLayout*>(pLayout->GetNext().obj().get());
        }

        pCont->Add(xXFFrame.get());
    }
}

LwpGroupFrame::LwpGroupFrame(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    :LwpContent(objHdr, pStrm)
{}

LwpGroupFrame::~LwpGroupFrame()
{}

void LwpGroupFrame::Read()
{
    LwpContent::Read();
    m_pObjStrm->SkipExtra();

}

void  LwpGroupFrame::RegisterStyle()
{
}

void LwpGroupFrame::XFConvert(XFContentContainer* /*pCont*/)
{
}

LwpDropcapLayout::LwpDropcapLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpFrameLayout(objHdr, pStrm)
{
    m_nChars = 1;
    m_nLines = 3;
}

void LwpDropcapLayout::Read()
{
    LwpFrameLayout::Read();
    m_nLines = m_pObjStrm->QuickReaduInt16();
    m_pObjStrm->SeekRel(1);
    m_pObjStrm->SkipExtra();
}

void LwpDropcapLayout::Parse(IXFStream* pOutputStream)
{
    LwpStory* pStory = static_cast<LwpStory*>(m_Content.obj(VO_STORY).get());
    if (!pStory)
        return;
    rtl::Reference<LwpObject> pPara = pStory->GetFirstPara().obj(VO_PARA);
    if(pPara.is())
    {
        pPara->SetFoundry(m_pFoundry);
        pPara->DoParse(pOutputStream);
    }
}

void LwpDropcapLayout::XFConvert(XFContentContainer* pCont)
{
    LwpStory* pStory = static_cast<LwpStory*>(m_Content.obj(VO_STORY).get());
    if (pStory)
    {
        pStory->SetFoundry(m_pFoundry);
        pStory->XFConvert(pCont);
    }
}

void LwpDropcapLayout::RegisterStyle(LwpFoundry* pFoundry)
{
    LwpStory* pStory = static_cast<LwpStory*>(m_Content.obj(VO_STORY).get());
    if (pStory)
    {
        pStory->SetDropcapFlag(true);
        pStory->SetFoundry(pFoundry);
        LwpPara* pPara = dynamic_cast<LwpPara*>(pStory->GetFirstPara().obj().get());
        while(pPara)
        {
            pPara->SetFoundry(pFoundry);
            pPara->RegisterStyle();
            pPara = dynamic_cast<LwpPara*>(pPara->GetNext().obj().get());
        }
    }
}

/**
 * @descr  do nothing
 *
 */
void LwpDropcapLayout::RegisterStyle()
{
}

LwpRubyLayout::LwpRubyLayout(LwpObjectHeader const &objHdr, LwpSvStream* pStrm)
    : LwpFrameLayout(objHdr, pStrm)
    , m_nPlacement(0)
    , m_nAlignment(0)
    , m_nStateFlag(0)
    , m_nXOffset(0)
    , m_nYOffset(0)
{
}

void LwpRubyLayout::Read()
{
    LwpFrameLayout::Read();
    m_nPlacement = m_pObjStrm->QuickReaduInt8();
    m_nAlignment = m_pObjStrm->QuickReaduInt8();
    m_nStateFlag = m_pObjStrm->QuickReaduInt16();
    m_nXOffset = m_pObjStrm->QuickReadInt32();
    m_nYOffset = m_pObjStrm->QuickReadInt32();
    m_objRubyMarker.ReadIndexed(m_pObjStrm.get());
    m_pObjStrm->SkipExtra();
}

LwpRubyMarker* LwpRubyLayout::GetMarker()
{
    return static_cast<LwpRubyMarker*>(m_objRubyMarker.obj(VO_RUBYMARKER).get());
}

LwpStory* LwpRubyLayout::GetContentStory()
{
    return static_cast<LwpStory*>(m_Content.obj(VO_STORY).get());
}

void LwpRubyLayout::ConvertContentText()
{
    LwpStory* pStory = GetContentStory();
    LwpRubyMarker* pMarker = GetMarker();
    if (pStory && pMarker)
        pMarker->SetRubyText(pStory->GetContentText(true));
}

void LwpRubyLayout::RegisterStyle()
{
    LwpRubyMarker* pMarker = GetMarker();
    if (!pMarker)
        throw std::runtime_error("missing Ruby Marker");

    std::unique_ptr<XFRubyStyle> xRubyStyle(new XFRubyStyle);

    enumXFRubyPosition eType = enumXFRubyLeft;
    if (m_nAlignment == LEFT)
    {
        eType = enumXFRubyLeft;
    }
    else if(m_nAlignment == RIGHT)
    {
        eType =  enumXFRubyRight;
    }
    else if(m_nAlignment == CENTER)
    {
        eType =  enumXFRubyCenter;
    }
    xRubyStyle->SetAlignment(eType);

    eType = enumXFRubyTop;
    if (m_nPlacement == TOP)
    {
        eType = enumXFRubyTop;
    }
    else if(m_nPlacement == BOTTOM)
    {
        eType =  enumXFRubyBottom;
    }
    xRubyStyle->SetPosition(eType);

    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    OUString rubyStyle = pXFStyleManager->AddStyle(std::move(xRubyStyle)).m_pStyle->GetStyleName();
    pMarker->SetRubyStyleName(rubyStyle);

    LwpStory* pStory = GetContentStory();
    pStory->SetFoundry(m_pFoundry);
    OUString textStyle = pStory->RegisterFirstFribStyle();
    pMarker->SetTextStyleName(textStyle);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
