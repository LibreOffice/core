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

#include "lwppagelayout.hxx"
#include "lwplaypiece.hxx"
#include "lwpfootnote.hxx"
#include "lwpdoc.hxx"
#include "lwpholder.hxx"
#include "lwppagehint.hxx"
#include "lwpdivinfo.hxx"
#include "lwpstory.hxx"
#include "xfilter/xfstylemanager.hxx"
#include "xfilter/xfmasterpage.hxx"
#include "xfilter/xfcontentcontainer.hxx"
#include "xfilter/xfheader.hxx"
#include "xfilter/xfheaderstyle.hxx"
#include "xfilter/xffooterstyle.hxx"
#include "xfilter/xffooter.hxx"
#include <sfx2/printer.hxx>
#include "lwpchangemgr.hxx"
#include "lwpglobalmgr.hxx"

LwpPageLayout::LwpPageLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm)
    : LwpLayout(objHdr, pStrm)
    , m_pPrinterBinName(new LwpAtomHolder)
    , m_nPrinterBin(0)
    , m_nBdroffset(0)
    , m_pPaperName(new LwpAtomHolder)
    , m_pXFPageMaster(nullptr)
{
}

LwpPageLayout::~LwpPageLayout()
{
    if (m_pPrinterBinName)
    {
        delete m_pPrinterBinName;
    }
    if (m_pPaperName)
    {
        delete m_pPaperName;
    }
}
void LwpPageLayout::Read()
{
    LwpLayout::Read();

    if (LwpFileHeader::m_nFileRevision < 0x000B)
    {
        // read PreRevBLayout...
    }

    m_nPrinterBin = m_pObjStrm->QuickReaduInt16();
    m_pPrinterBinName->Read(m_pObjStrm);

    if (LwpFileHeader::m_nFileRevision >= 0x000B)
        m_nBdroffset = m_pObjStrm->QuickReadInt32();

    if (m_pObjStrm->CheckExtra())
    {
        m_pPaperName->Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }

}

void LwpPageLayout::Parse(IXFStream* pOutputStream)
{
    //Only parse this layout
    rtl::Reference<LwpObject> pStory = m_Content.obj();
    if(pStory.is())
    {
        pStory->SetFoundry(m_pFoundry);
        pStory->DoParse(pOutputStream);   //Do not parse the next story
    }
}

/**
* @descr:   set page margins
*
*/
void LwpPageLayout::ParseMargins(XFPageMaster* pm1)
{
    double fLeft    = GetMarginsValue(MARGIN_LEFT);
    double fRight   = GetMarginsValue(MARGIN_RIGHT);
    double fTop = GetMarginsValue(MARGIN_TOP);
    double fBottom  = GetMarginsValue(MARGIN_BOTTOM);

    pm1->SetMargins( fLeft, fRight, fTop, fBottom );

}

/**
* @descr:   set page height and width
*
*/
void LwpPageLayout::ParseGeometry(XFPageMaster* pm1)
{
    /*
    LwpLayoutGeometry* pLayoutGeo = GetGeometry();
    if(pLayoutGeo)
    {
        pm1->SetPageHeight( GetGeometryHeight() );
        pm1->SetPageWidth( GetGeometryWidth() );
    }
    */
    double fWidth =0;
    double fHeight = 0;
    GetWidthAndHeight(fWidth, fHeight);
    pm1->SetPageWidth( fWidth );
    pm1->SetPageHeight( fHeight );
}

/**
* @descr:   set page watermark
*
*/
void LwpPageLayout::ParseWaterMark(XFPageMaster *pm1)
{
    XFBGImage* pXFBGImage = GetXFBGImage();
    if(pXFBGImage)
    {
        pm1->SetBackImage(pXFBGImage);
    }
}

/**
* @descr:   set page columns
*
*/
void LwpPageLayout::ParseColumns(XFPageMaster * pm1)
{
    XFColumns* pColumns = GetXFColumns();
    if(pColumns)
    {
        pm1->SetColumns(pColumns);
    }
}

/**
* @descr:   set page borders
*
*/
void LwpPageLayout::ParseBorders(XFPageMaster *pm1)
{
    XFBorders* pBordres = GetXFBorders();
    if(pBordres)
    {
        pm1->SetBorders(pBordres);
    }
}

/**
* @descr:   set page shadow
*
*/
void LwpPageLayout::ParseShadow(XFPageMaster *pm1)
{
    XFShadow* pXFShadow = GetXFShadow();
    if(pXFShadow)
    {
        pm1->SetShadow(pXFShadow);
    }
}

/**
* @descr:   set page back pattern
*
*/
void LwpPageLayout::ParsePatternFill(XFPageMaster* pm1)
{
    XFBGImage* pXFBGImage = this->GetFillPattern();
    if (pXFBGImage)
    {
        pm1->SetBackImage(pXFBGImage);
    }
}
/**
* @descr:   set page background
*
*/
void LwpPageLayout::ParseBackGround(XFPageMaster* pm1)
{
    if (this->IsPatternFill())
    {
        ParsePatternFill(pm1);
    }
    else
    {
        ParseBackColor(pm1);
    }
}

/**
* @descr:   set page back color
*
*/
void LwpPageLayout::ParseBackColor(XFPageMaster* pm1)
{
    LwpColor* pColor = GetBackColor();
    if(pColor)
    {
        pm1->SetBackColor(XFColor(pColor->To24Color()));
    }
}

/**
* @descr:   set page footnote separator information
*
*/
void LwpPageLayout::ParseFootNoteSeparator(XFPageMaster * pm1)
{
    //Get the footnoteoptions for the root document
    LwpDocument* pDocument = m_pFoundry ? m_pFoundry->GetDocument() : nullptr;
    if (pDocument)
    {
        LwpObjectID* pFontnodeId = pDocument->GetValidFootnoteOpts();

        LwpFootnoteOptions* pFootnoteOpts = pFontnodeId ? dynamic_cast<LwpFootnoteOptions*>(pFontnodeId->obj().get()) : nullptr;
        if(pFootnoteOpts)
        {
            LwpFootnoteSeparatorOptions& rFootnoteSep = pFootnoteOpts->GetFootnoteSeparator();
            //set length
            sal_uInt32 nLengthPercent = 100;
            double fWidth = 0;
            if(rFootnoteSep.HasSeparator())
            {
                fWidth = rFootnoteSep.GetTopBorderWidth();
            }
            if(rFootnoteSep.HasCustomLength())
            {
                nLengthPercent =  static_cast<sal_uInt32>(100*LwpTools::ConvertFromUnitsToMetric(rFootnoteSep.GetLength())/GetMarginWidth());
                if(nLengthPercent > 100)
                    nLengthPercent = 100;
            }
            double fAbove = LwpTools::ConvertFromUnitsToMetric(rFootnoteSep.GetAbove());
            double fBelow = LwpTools::ConvertFromUnitsToMetric(rFootnoteSep.GetBelow());
            LwpColor aColor = rFootnoteSep.GetTopBorderColor();
            enumXFAlignType eAlignType = enumXFAlignStart;
            if(rFootnoteSep.GetIndent() > 0)
            {
                //SODC don't support indent
                sal_uInt32 nIndentPercent =  static_cast<sal_uInt32>(100*LwpTools::ConvertFromUnitsToMetric(rFootnoteSep.GetIndent())/GetMarginWidth());
                if(nIndentPercent + nLengthPercent >= 100)
                    eAlignType = enumXFAlignEnd;
            }
            if(aColor.IsValidColor())
            {
                XFColor aXFColor(aColor.To24Color());
                pm1->SetFootNoteSeparator(eAlignType,fWidth, nLengthPercent, fAbove, fBelow, aXFColor);
            }
        }
    }
}

/**
* @descr:   Register master page and page master style
*
*/
void LwpPageLayout::RegisterStyle()
{
    XFPageMaster* pm1 = new XFPageMaster();
    m_pXFPageMaster = pm1;

    ParseGeometry( pm1 );
    //Does not process LayoutScale, for watermark is not supported in SODC.
    ParseWaterMark( pm1);
    ParseMargins( pm1);
    ParseColumns(pm1);
    ParseBorders(pm1);
    ParseShadow(pm1);
//  ParseBackColor(pm1);
    ParseBackGround(pm1);
    ParseFootNoteSeparator(pm1);
    pm1->SetTextDir(GetTextDirection());

    LwpUseWhen* pUseWhen = GetUseWhen();
    if(IsComplex() ||( pUseWhen && pUseWhen->IsUseOnAllOddPages()))
    {
        pm1->SetPageUsage(enumXFPageUsageMirror);
    }

    //Add the page master to stylemanager
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_pXFPageMaster = pm1 = static_cast<XFPageMaster*>(pXFStyleManager->AddStyle(pm1).m_pStyle);
    OUString pmname = pm1->GetStyleName();

    //Add master page
    XFMasterPage* mp1 = new XFMasterPage();
    mp1->SetStyleName(GetName().str());
    mp1->SetPageMaster(pmname);
    mp1 = static_cast<XFMasterPage*>(pXFStyleManager->AddStyle(mp1).m_pStyle);
    m_StyleName = mp1->GetStyleName();

    //Set footer style
    LwpFooterLayout* pLayoutFooter = GetFooterLayout();
    if(pLayoutFooter)
    {
        pLayoutFooter->SetFoundry(m_pFoundry);
        pLayoutFooter->RegisterStyle(pm1);
        pLayoutFooter->RegisterStyle(mp1);
    }

    //Set header style
    LwpHeaderLayout* pLayoutHeader = GetHeaderLayout();
    if(pLayoutHeader)
    {
        pLayoutHeader->SetFoundry(m_pFoundry);
        pLayoutHeader->RegisterStyle(pm1);
        pLayoutHeader->RegisterStyle(mp1);
    }
    //register child layout style for mirror page and frame
    RegisterChildStyle();
}

/**
* @descr:   Register master page for endnote which name is "endnote"
* @return:  Return the master page name.
*/
OUString LwpPageLayout::RegisterEndnoteStyle()
{
    XFPageMaster* pm1 = new XFPageMaster();
    m_pXFPageMaster = pm1;

    ParseGeometry( pm1 );
    ParseWaterMark( pm1);
    ParseMargins( pm1);
    ParseColumns(pm1);
    ParseBorders(pm1);
    ParseShadow(pm1);
//  ParseBackColor(pm1);
    ParseBackGround(pm1);
    //ParseFootNoteSeparator(pm1);
    pm1->SetTextDir(GetTextDirection());

    LwpUseWhen* pUseWhen = GetUseWhen();
    if(IsComplex() ||( pUseWhen && pUseWhen->IsUseOnAllOddPages()))
    {
        pm1->SetPageUsage(enumXFPageUsageMirror);
    }

    //Add the page master to stylemanager
    XFStyleManager* pXFStyleManager = LwpGlobalMgr::GetInstance()->GetXFStyleManager();
    m_pXFPageMaster = pm1 = static_cast<XFPageMaster*>(pXFStyleManager->AddStyle(pm1).m_pStyle);
    OUString pmname = pm1->GetStyleName();

    //Add master page
    XFMasterPage* mp1 = new XFMasterPage();
    mp1->SetStyleName("Endnote");
    mp1->SetPageMaster(pmname);

    //Set footer style
    LwpFooterLayout* pLayoutFooter = GetFooterLayout();
    if(pLayoutFooter)
    {
        pLayoutFooter->SetFoundry(m_pFoundry);
        pLayoutFooter->RegisterStyle(pm1);
        pLayoutFooter->RegisterStyle(mp1);
    }

    //Set header style
    LwpHeaderLayout* pLayoutHeader = GetHeaderLayout();
    if(pLayoutHeader)
    {
        pLayoutHeader->SetFoundry(m_pFoundry);
        pLayoutHeader->RegisterStyle(pm1);
        pLayoutHeader->RegisterStyle(mp1);
    }

    return pXFStyleManager->AddStyle(mp1).m_pStyle->GetStyleName();
}
/**
* @descr:   Whether current page layout has columns
*
*/
bool LwpPageLayout::HasColumns()
{
    return GetNumCols() > 1;
}

/**
* @descr:   Whether has filler page text in current page layout
*
*/
bool LwpPageLayout::HasFillerPageText(LwpFoundry* pFoundry)
{
    if(!pFoundry) return false;

    bool bFillerPage = false;
    LwpLayout::UseWhenType eWhenType = GetUseWhenType();
    if(eWhenType==LwpLayout::StartOnOddPage||eWhenType==LwpLayout::StartOnEvenPage)
    {
        //get pagenumber
        sal_uInt16 nPageNumber = 0;

        //get the page number that current page layout inserted
        nPageNumber = GetPageNumber(FIRST_LAYOUTPAGENO)-1;

        if(nPageNumber>0)
        {
            if((eWhenType==LwpLayout::StartOnOddPage)&&(LwpTools::IsOddNumber(nPageNumber)))
            {
                bFillerPage = true;
            }
            else if((eWhenType==LwpLayout::StartOnEvenPage)&&(LwpTools::IsEvenNumber(nPageNumber)))
            {
                bFillerPage = true;
            }
            else
            {
                bFillerPage = false;
            }
        }
    }

    return bFillerPage;
}

/**
* @descr:   Parse filler page text
*
*/
void LwpPageLayout::ConvertFillerPageText(XFContentContainer* pCont)
{
    if(HasFillerPageText(m_pFoundry))
    {
        //get fillerpage story from division info
        LwpDocument* pDoc = m_pFoundry->GetDocument();
        LwpDivInfo* pDivInfo = dynamic_cast<LwpDivInfo*>(pDoc->GetDivInfoID().obj().get());
        LwpStory* pStory = dynamic_cast<LwpStory*>(pDivInfo->GetFillerPageTextID().obj().get());

        //parse fillerpage story
        if(pStory)
        {
            pStory->XFConvert(pCont);
        }
    }
}
/**
* @descr:   Clear columns style in page layout
*
*/
void LwpPageLayout::ResetXFColumns()
{
    if(m_pXFPageMaster)
    {
        m_pXFPageMaster->SetColumns(nullptr);
    }
}

LwpHeaderLayout* LwpPageLayout::GetHeaderLayout()
{
    rtl::Reference<LwpVirtualLayout> xLay(dynamic_cast<LwpVirtualLayout*>(GetChildHead().obj().get()));
    while (xLay.is())
    {
        if (xLay->GetLayoutType() == LWP_HEADER_LAYOUT)
            return dynamic_cast<LwpHeaderLayout*>(xLay.get());
        xLay.set(dynamic_cast<LwpVirtualLayout*>(xLay->GetNext().obj().get()));
    }
    return nullptr;
}

LwpFooterLayout* LwpPageLayout::GetFooterLayout()
{
    rtl::Reference<LwpVirtualLayout> xLay(dynamic_cast<LwpVirtualLayout*>(GetChildHead().obj().get()));
    while (xLay.is())
    {
        if (xLay->GetLayoutType() == LWP_FOOTER_LAYOUT)
            return dynamic_cast<LwpFooterLayout*>(xLay.get());
        xLay.set(dynamic_cast<LwpVirtualLayout*>(xLay->GetNext().obj().get()));
    }
    return nullptr;
}

/**
* @descr:   Get the odd layout if current page layout is mirror page
*
*/
LwpPageLayout* LwpPageLayout::GetOddChildLayout()
{
    if(IsComplex())
    {
        rtl::Reference<LwpVirtualLayout> xLay(dynamic_cast<LwpVirtualLayout*>(GetChildHead().obj().get()));
        while (xLay.is())
        {
            if (xLay->GetLayoutType() == LWP_PAGE_LAYOUT)
            {
                LwpPageLayout* pPageLayout = static_cast<LwpPageLayout*>(xLay.get());
                LwpUseWhen* pUseWhen = pPageLayout->GetUseWhen();
                if(pUseWhen && pUseWhen->IsUseOnAllOddPages())
                {
                    return pPageLayout;
                }
            }
            xLay.set(dynamic_cast<LwpVirtualLayout*>(xLay->GetNext().obj().get()));
        }
    }
    return nullptr;
}

/**
* @descr:   Get margin width of page
*
*/
double LwpPageLayout::GetMarginWidth()
{
    double fPagewidth = GetGeometryWidth();
    double fLeftMargin = GetMarginsValue(MARGIN_LEFT);
    double fRightMargin = GetMarginsValue(MARGIN_RIGHT);

    return fPagewidth - (fLeftMargin + fRightMargin);
}

/**
 * @descr:  Get the pagenumber
 * @param:  if  nLayoutNumber =FIRST_LAYOUTPAGENO,  return the first page number  that current page layout covers
                              =LAST_LAYOUTPAGENO, return the last page number that current page layout covers
                               else, return the specified page number that current page layout covers
 * @param:
 * @return:  if reture value >=0, success to find the page number, or fail.
*/
sal_Int32 LwpPageLayout::GetPageNumber(sal_uInt16 nLayoutNumber)
{
    sal_Int16 nPageNumber = -1;
    LwpFoundry* pFoundry = this->GetFoundry();
    if (!pFoundry)
        return nPageNumber;
    LwpDocument* pDoc = pFoundry->GetDocument();
    LwpDLVListHeadTailHolder* pHeadTail = dynamic_cast<LwpDLVListHeadTailHolder*>(pDoc->GetPageHintsID().obj().get());
    if(!pHeadTail) return nPageNumber;

    //get first pagehint
    LwpPageHint* pPageHint = dynamic_cast<LwpPageHint*>(pHeadTail->GetHead().obj().get());
    while(pPageHint)
    {
        if(this->GetObjectID() == pPageHint->GetPageLayoutID())
        {
            sal_uInt16 nNumber = pPageHint->GetPageNumber();
            if(nLayoutNumber==FIRST_LAYOUTPAGENO && pPageHint->GetLayoutPageNumber()==1)
            {
                //get the first page number
                nPageNumber = nNumber;
                break;
            }
            else if( nLayoutNumber ==LAST_LAYOUTPAGENO && nNumber >nPageNumber )
            {
                //get the last page number
                nPageNumber = nNumber;
                if(pPageHint->GetNext().IsNull())
                {
                    //if is last page number of entire document, reture directly
                    return nPageNumber + pDoc->GetNumberOfPagesBefore();
                }
            }
            else if(nLayoutNumber > 0 && pPageHint->GetLayoutPageNumber() == nLayoutNumber)
            {
                //get specified page number
                nPageNumber = nNumber;
                break;
            }

        }
        pPageHint = dynamic_cast<LwpPageHint*>(pPageHint->GetNext().obj().get());
    }
    if(nPageNumber>=0)
    {
        return nPageNumber + 1 + pDoc->GetNumberOfPagesBefore();
    }
    return -1;
}

/**
* @descr:   Get page width and height
*
*/
void LwpPageLayout::GetWidthAndHeight(double& fWidth, double& fHeight)
{
    //use customized size
    LwpLayoutGeometry* pLayoutGeo = GetGeometry();
    if(pLayoutGeo)
    {
        fWidth = GetGeometryWidth();
        fHeight = GetGeometryHeight();
    }

    if(GetUsePrinterSettings())
    {
        //replaced by printer paper size
        ScopedVclPtrInstance< Printer > pPrinter;
        bool bScreen = pPrinter->IsDisplayPrinter();
        if (!bScreen)//Printer available
        {
            Size aPaperSize = pPrinter->GetPaperSize();
            aPaperSize = pPrinter->PixelToLogic( aPaperSize, MapMode( MAP_10TH_MM ) );
            fWidth = static_cast<double>(aPaperSize.Width())/100;   //cm unit
            fHeight = static_cast<double>(aPaperSize.Height())/100;
        }
    }

    //Follow the former design of Lotus WordPro filter, some default will be given:
    //Page Width: 8.5 Inch -> 21.59 cm
    //Page Height: 11 Inch -> 27.94 cm
    if (fWidth < 4.39)
        fWidth = 21.59;
    if (fHeight < 4.49)
        fHeight = 27.94;
    //End of modification, by ZF
}

/**
* @descr:   Get page width
*
*/
double LwpPageLayout::GetWidth()
{
    double fWidth =0, fHeight = 0;
    GetWidthAndHeight(fWidth, fHeight);
    return fWidth;
}

/**
* @descr:   Get page height
*
*/
double LwpPageLayout::GetHeight()
{
    double fWidth =0, fHeight = 0;
    GetWidthAndHeight(fWidth, fHeight);
    return fHeight;
}
/**
* @descr:  Compare the position of layout. If the position of this layout is earlier than other layout,return true, or return false
*
*/
bool LwpPageLayout::operator<(LwpPageLayout& Other)
{
    LwpPara* pThisPara = GetPagePosition();
    LwpPara* pOtherPara = Other.GetPagePosition();
    if(pThisPara && pOtherPara)
    {
        if(pThisPara == pOtherPara)
        {
            //If the two layouts in the same para, compare which layout is earlied according to frib order
            return pThisPara->ComparePagePosition(this, &Other);
        }
        else
        {
            return *pThisPara < *pOtherPara;
        }
    }

    if(!pThisPara)
        return true;

    return false;
}

/**
* @descr:  Get the position of pagelayout
*
*/
LwpPara* LwpPageLayout::GetPagePosition()
{
    LwpPara* pPara = dynamic_cast<LwpPara*>(GetPosition().obj().get());
    if(pPara)
        return pPara;
    //Get the position from its related section
    LwpFoundry* pFoundry = GetFoundry();
    if(pFoundry)
    {
        LwpSection* pSection = nullptr;
        while( (pSection = pFoundry->EnumSections(pSection)) )
        {
            if(pSection->GetPageLayout() == this)
                return dynamic_cast<LwpPara*>(pSection->GetPosition().obj().get());
        }
    }

    return nullptr;
}
LwpHeaderLayout::LwpHeaderLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm )
    : LwpPlacableLayout(objHdr, pStrm)
    , m_nBorderOffset(0)
{
}

LwpHeaderLayout::~LwpHeaderLayout()
{
}

void LwpHeaderLayout::Read()
{
    LwpPlacableLayout::Read();

    if(LwpFileHeader::m_nFileRevision >= 0x000E)
        m_nBorderOffset = m_pObjStrm->QuickReadInt32();
    else
        m_nBorderOffset = 0;
    m_pObjStrm->SkipExtra();
}

void LwpHeaderLayout::RegisterStyle(XFPageMaster* pm1)
{
    XFHeaderStyle* pHeaderStyle = new XFHeaderStyle();

    //Modify page top margin
    //page top marging: from top of header to the top edge
    double top = GetMarginsValue(MARGIN_TOP);
    pm1->SetMargins(-1, -1, top, -1);

    ParseMargins(pHeaderStyle);
    ParseBorder(pHeaderStyle);
    ParseShadow(pHeaderStyle);
//  ParseBackColor(pHeaderStyle);
    ParseBackGround(pHeaderStyle);

    ParseWaterMark(pHeaderStyle);
    //End by

    pm1->SetHeaderStyle(pHeaderStyle);
}

void LwpHeaderLayout::ParseMargins(XFHeaderStyle* ph1)
{
    //Set height: from top of header to top of body, including the spacing between header and body
    double height = GetGeometryHeight()- GetMarginsValue(MARGIN_TOP);
    if( IsAutoGrowDown() )
    {
        ph1->SetMinHeight(height);
    }
    else
    {
        ph1->SetHeight(height);
    }

    //Set left,right,bottom margins
    LwpMiddleLayout* parent = dynamic_cast<LwpMiddleLayout*> (GetParent().obj().get());
    //left margin in SODC: the space from the left edge of body to the left edge of header
    double left = GetMarginsValue(MARGIN_LEFT) - (parent ? parent->GetMarginsValue(MARGIN_LEFT) : 0);
    if(left<=0) //The left margin in SODC can not be minus value
    {
        left = -1;
    }
    //left margin in SODC: the space from the right edge of header to the right edge of body
    double right = GetMarginsValue(MARGIN_RIGHT) - (parent ? parent->GetMarginsValue(MARGIN_RIGHT) : 0);
    if(right<=0)//The right margin in SODC can not be minus value
    {
        right = -1;
    }
    ph1->SetMargins( left, right, GetMarginsValue(MARGIN_BOTTOM));

    //Word Pro has no dynamic spacing, should be set to false
    ph1->SetDynamicSpace(false);
}

void LwpHeaderLayout::ParseBorder(XFHeaderStyle* pHeaderStyle)
{
    XFBorders* pBordres = GetXFBorders();
    if(pBordres)
    {
        pHeaderStyle->SetBorders(pBordres);
    }
}

void LwpHeaderLayout::ParseShadow(XFHeaderStyle* pHeaderStyle)
{
    XFShadow* pXFShadow = GetXFShadow();
    if(pXFShadow)
    {
        pHeaderStyle->SetShadow(pXFShadow);
    }
}

/**
* @descr:   set header back pattern
*
*/
void LwpHeaderLayout::ParsePatternFill(XFHeaderStyle* pHeaderStyle)
{
    XFBGImage* pXFBGImage = this->GetFillPattern();
    if (pXFBGImage)
    {
        pHeaderStyle->SetBackImage(pXFBGImage);
    }
}
/**
* @descr:   set header background
*
*/
void LwpHeaderLayout::ParseBackGround(XFHeaderStyle* pHeaderStyle)
{
    if (this->IsPatternFill())
    {
        ParsePatternFill(pHeaderStyle);
    }
    else
    {
        ParseBackColor(pHeaderStyle);
    }
}

void LwpHeaderLayout::ParseBackColor(XFHeaderStyle* pHeaderStyle)
{
    LwpColor* pColor = GetBackColor();
    if(pColor)
    {
        pHeaderStyle->SetBackColor(XFColor(pColor->To24Color()));
    }
}

void LwpHeaderLayout::ParseWaterMark(XFHeaderStyle * pHeaderStyle)
{
    XFBGImage* pXFBGImage = GetXFBGImage();
    if(pXFBGImage)
    {
        pHeaderStyle->SetBackImage(pXFBGImage);
    }
}
//End by

void LwpHeaderLayout::RegisterStyle(XFMasterPage* mp1)
{
    XFHeader* pHeader = new XFHeader();
    rtl::Reference<LwpObject> pStory = m_Content.obj();
    if(pStory.is())
    {
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
        pChangeMgr->SetHeadFootFribMap(true);

        //Call the RegisterStyle first to register the styles in header paras, and then XFConvert()
        pStory->SetFoundry(m_pFoundry);
        pStory->DoRegisterStyle();
        //, 06/27/2005
        //register child layout style for framelayout,
        RegisterChildStyle();
        //End
        pChangeMgr->SetHeadFootChange(pHeader);
        pStory->XFConvert(pHeader);

        pChangeMgr->SetHeadFootFribMap(false);
    }
    mp1->SetHeader(pHeader);
}

LwpFooterLayout::LwpFooterLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm )
    : LwpPlacableLayout( objHdr, pStrm )
    , m_nBorderOffset(0)
{
}

LwpFooterLayout::~LwpFooterLayout()
{
}

void LwpFooterLayout::Read()
{
    LwpPlacableLayout::Read();

    if(LwpFileHeader::m_nFileRevision >= 0x000E)
        m_nBorderOffset = m_pObjStrm->QuickReadInt32();
    else
        m_nBorderOffset = 0;
    m_pObjStrm->SkipExtra();
}

void LwpFooterLayout::RegisterStyle(XFPageMaster* pm1)
{
    XFFooterStyle* pFooterStyle = new XFFooterStyle();

    //Modify page bottom margin
    //page bottom margin: from bottom of footer to the bottom edge
    double bottom = GetMarginsValue(MARGIN_BOTTOM);
    pm1->SetMargins(-1, -1, -1, bottom);

    ParseMargins(pFooterStyle);
    ParseBorder(pFooterStyle);
    ParseShadow(pFooterStyle);
    ParseBackGround(pFooterStyle);
//  ParseBackColor(pFooterStyle);

    ParseWaterMark(pFooterStyle);
    //End by

    pm1->SetFooterStyle(pFooterStyle);
}

void LwpFooterLayout::ParseMargins(XFFooterStyle* pFooterStyle)
{

    //Set height: from top of header to top of body, including the spacing between header and body
    double height = GetGeometryHeight() - GetMarginsValue(MARGIN_BOTTOM);
    if( IsAutoGrowUp() )
    {
        pFooterStyle->SetMinHeight(height);
    }
    else
    {
        pFooterStyle->SetHeight(height);
    }

    //Set left,right,top margins
    LwpMiddleLayout* parent = dynamic_cast<LwpMiddleLayout*> (GetParent().obj().get());
    double left = GetMarginsValue(MARGIN_LEFT) - (parent ? parent->GetMarginsValue(MARGIN_LEFT) : 0);
    if(left<=0) //The left margin in SODC can not be minus value
    {
        left = -1;
    }
    double right = GetMarginsValue(MARGIN_RIGHT) - (parent ? parent->GetMarginsValue(MARGIN_RIGHT) : 0);
    if(right<=0)//The left margin in SODC can not be minus value
    {
        right = -1;
    }
    pFooterStyle->SetMargins( left, right, GetMarginsValue(MARGIN_TOP));

    //Word Pro has no dynamic spacing, should be set to false
    pFooterStyle->SetDynamicSpace(false);
}

void LwpFooterLayout::ParseBorder(XFFooterStyle* pFooterStyle)
{
    XFBorders* pBordres = GetXFBorders();
    if(pBordres)
    {
        pFooterStyle->SetBorders(pBordres);
    }
}

void LwpFooterLayout::ParseShadow(XFFooterStyle* pFooterStyle)
{
    XFShadow* pXFShadow = GetXFShadow();
    if(pXFShadow)
    {
        pFooterStyle->SetShadow(pXFShadow);
    }
}
/**
* @descr:   set footer back pattern
*
*/
void LwpFooterLayout::ParsePatternFill(XFFooterStyle* pFooterStyle)
{
    XFBGImage* pXFBGImage = this->GetFillPattern();
    if (pXFBGImage)
    {
        pFooterStyle->SetBackImage(pXFBGImage);
    }
}
/**
* @descr:   set footer background
*
*/
void LwpFooterLayout::ParseBackGround(XFFooterStyle* pFooterStyle)
{
    if (this->IsPatternFill())
    {
        ParsePatternFill(pFooterStyle);
    }
    else
    {
        ParseBackColor(pFooterStyle);
    }
}

void LwpFooterLayout::ParseBackColor(XFFooterStyle* pFooterStyle)
{
    LwpColor* pColor = GetBackColor();
    if(pColor)
    {
        pFooterStyle->SetBackColor(XFColor(pColor->To24Color()));
    }
}

void LwpFooterLayout::RegisterStyle(XFMasterPage* mp1)
{
    XFFooter* pFooter = new XFFooter();
    rtl::Reference<LwpObject> pStory = m_Content.obj(VO_STORY);
    //Call the RegisterStyle first to register the styles in footer paras, and then XFConvert()
    if(pStory.is())
    {
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
        pChangeMgr->SetHeadFootFribMap(true);

        pStory->SetFoundry(m_pFoundry);
        pStory->DoRegisterStyle();
        //register child layout style for framelayout,
        RegisterChildStyle();

        pChangeMgr->SetHeadFootChange(pFooter);

        pStory->XFConvert(pFooter);

        pChangeMgr->SetHeadFootFribMap(false);
    }
    mp1->SetFooter(pFooter);
}

void LwpFooterLayout::ParseWaterMark(XFFooterStyle * pFooterStyle)
{
    XFBGImage* pXFBGImage = GetXFBGImage();
    if(pXFBGImage)
    {
        pFooterStyle->SetBackImage(pXFBGImage);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
