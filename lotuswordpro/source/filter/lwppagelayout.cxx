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
 * Change History
Mar 2005			Created
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
    : LwpLayout(objHdr, pStrm), m_pPrinterBinName(new LwpAtomHolder),
    m_pPaperName(new LwpAtomHolder),m_pXFPageMaster(NULL)
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

    m_pObjStrm->QuickRead(&m_nPrinterBin, sizeof(m_nPrinterBin));
    m_pPrinterBinName->Read(m_pObjStrm);

    if (LwpFileHeader::m_nFileRevision >= 0x000B)
    {
        m_pObjStrm->QuickRead(&m_nBdroffset, sizeof(m_nBdroffset));
    }

    if (m_pObjStrm->CheckExtra())
    {
        m_pPaperName->Read(m_pObjStrm);
        m_pObjStrm->SkipExtra();
    }

}

void LwpPageLayout::Parse(IXFStream* pOutputStream)
{
    //Only parse this layout
    LwpObject* pStory = m_Content.obj();
    if(pStory)
    {
        pStory->SetFoundry(m_pFoundry);
        pStory->Parse(pOutputStream);	//Do not parse the next story
    }
}

/**
* @descr:   set page margins
*
*/
void LwpPageLayout::ParseMargins(XFPageMaster* pm1)
{
    double fLeft	= GetMarginsValue(MARGIN_LEFT);
    double fRight	= GetMarginsValue(MARGIN_RIGHT);
    double fTop	= GetMarginsValue(MARGIN_TOP);
    double fBottom	= GetMarginsValue(MARGIN_BOTTOM);

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
    LwpDocument* pDocument = m_pFoundry->GetDocument();
    if(pDocument)
    {
        LwpFootnoteOptions* pFootnoteOpts = static_cast<LwpFootnoteOptions*>(pDocument->GetValidFootnoteOpts()->obj());
        if(pFootnoteOpts)
        {
            LwpFootnoteSeparatorOptions* pFootnoteSep = pFootnoteOpts->GetFootnoteSeparator();
            //set length
            sal_uInt32 nLengthPercent = 100;
            double fWidth = 0;
            if(pFootnoteSep->HasSeparator())
            {
                fWidth = pFootnoteSep->GetTopBorderWidth();
            }
            if(pFootnoteSep->HasCustomLength())
            {
                nLengthPercent =  static_cast<sal_uInt32>(100*LwpTools::ConvertFromUnitsToMetric(pFootnoteSep->GetLength())/GetMarginWidth());
                if(nLengthPercent > 100)
                    nLengthPercent = 100;
            }
            double fAbove = LwpTools::ConvertFromUnitsToMetric(pFootnoteSep->GetAbove());
            double fBelow = LwpTools::ConvertFromUnitsToMetric(pFootnoteSep->GetBelow());
            LwpColor aColor = pFootnoteSep->GetTopBorderColor();
            enumXFAlignType eAlignType = enumXFAlignStart;
            if(pFootnoteSep->GetIndent() > 0)
            {
                //SODC don't support indent
                sal_uInt32 nIndentPercent =  static_cast<sal_uInt32>(100*LwpTools::ConvertFromUnitsToMetric(pFootnoteSep->GetIndent())/GetMarginWidth());
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
//	ParseBackColor(pm1);
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
    OUString pmname = pXFStyleManager->AddStyle(pm1)->GetStyleName();

    //Add master page
    XFMasterPage* mp1 = new XFMasterPage();
    mp1->SetStyleName(GetName()->str());
    mp1->SetPageMaster(pmname);
    m_StyleName = pXFStyleManager->AddStyle(mp1)->GetStyleName();

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
//	ParseBackColor(pm1);
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
    OUString pmname = pXFStyleManager->AddStyle(pm1)->GetStyleName();

    //Add master page
    XFMasterPage* mp1 = new XFMasterPage();
    mp1->SetStyleName(A2OUSTR("Endnote"));
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

    return pXFStyleManager->AddStyle(mp1)->GetStyleName();
}
/**
* @descr:   Whether current page layout has columns
*
*/
sal_Bool LwpPageLayout::HasColumns()
{
    return GetNumCols() > 1 ? sal_True : sal_False;
}

/**
* @descr:   Whether has filler page text in current page layout
*
*/
sal_Bool LwpPageLayout::HasFillerPageText(LwpFoundry* pFoundry)
{
    if(!pFoundry) return sal_False;

    sal_Bool bFillerPage = sal_False;
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
                bFillerPage = sal_True;
            }
            else if((eWhenType==LwpLayout::StartOnEvenPage)&&(LwpTools::IsEvenNumber(nPageNumber)))
            {
                bFillerPage = sal_True;
            }
            else
            {
                bFillerPage = sal_False;
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
        //get filerpage story from division info
        LwpDocument* pDoc = m_pFoundry->GetDocument();
        LwpDivInfo* pDivInfo = static_cast<LwpDivInfo*>(pDoc->GetDivInfoID()->obj());
        LwpStory* pStory = static_cast<LwpStory*>(pDivInfo->GetFillerPageTextID()->obj());

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
        m_pXFPageMaster->SetColumns(NULL);
    }
}

LwpHeaderLayout* LwpPageLayout::GetHeaderLayout()
{
    LwpVirtualLayout* pLay = static_cast<LwpVirtualLayout*>(GetChildHead()->obj());
    while(pLay)
    {
        if( pLay->GetLayoutType() == LWP_HEADER_LAYOUT )
            return ( static_cast<LwpHeaderLayout*> (pLay) );
        pLay = static_cast<LwpVirtualLayout*> (pLay->GetNext()->obj());
    }
    return NULL;
}

LwpFooterLayout* LwpPageLayout::GetFooterLayout()
{
    LwpVirtualLayout* pLay = static_cast<LwpVirtualLayout*>(GetChildHead()->obj());
    while(pLay)
    {
        if( pLay->GetLayoutType() == LWP_FOOTER_LAYOUT )
            return ( static_cast<LwpFooterLayout*> (pLay) );
        pLay = static_cast<LwpVirtualLayout*> (pLay->GetNext()->obj());
    }
    return NULL;
}

/**
* @descr:   Get the odd layout if current page layout is mirror page
*
*/
LwpPageLayout* LwpPageLayout::GetOddChildLayout()
{
    if(IsComplex())
    {
        LwpVirtualLayout* pLay = static_cast<LwpVirtualLayout*>(GetChildHead()->obj());
        while(pLay)
        {
            if( pLay->GetLayoutType() == LWP_PAGE_LAYOUT )
            {
                LwpPageLayout* pPageLayout = static_cast<LwpPageLayout*> (pLay);
                LwpUseWhen* pUseWhen = pPageLayout->GetUseWhen();
                if(pUseWhen && pUseWhen->IsUseOnAllOddPages())
                {
                    return pPageLayout;
                }
            }
            pLay = static_cast<LwpVirtualLayout*> (pLay->GetNext()->obj());
        }
    }
    return NULL;
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
    LwpDocument* pDoc = pFoundry->GetDocument();
    LwpDLVListHeadTailHolder* pHeadTail = static_cast<LwpDLVListHeadTailHolder*>(pDoc->GetPageHintsID()->obj());
    if(!pHeadTail) return nPageNumber;

    //get first pagehint
    LwpPageHint* pPageHint =static_cast<LwpPageHint*>(pHeadTail->GetHead()->obj());
    while(pPageHint)
    {
        if(*(this->GetObjectID()) == *(pPageHint->GetPageLayoutID()))
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
                if(pPageHint->GetNext()->IsNull())
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
        pPageHint = static_cast<LwpPageHint*>(pPageHint->GetNext()->obj());
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
        Printer aPrinter;
        sal_Bool bScreen = aPrinter.IsDisplayPrinter();
        if (!bScreen)//Printer available
        {
            Size aPaperSize = aPrinter.GetPaperSize();
            aPaperSize = aPrinter.PixelToLogic( aPaperSize, MapMode( MAP_10TH_MM ) );
            fWidth = static_cast<double>(aPaperSize.Width())/100;	//cm unit
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
* @descr:  Compare the position of layout. If the poistion of this layout is earlier than other layout,return ture, or return false
*
*/
sal_Bool LwpPageLayout::operator<(LwpPageLayout& Other)
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
        return sal_True;

    return sal_False;
}

/**
* @descr:  Get the position of pagelayout
*
*/
LwpPara* LwpPageLayout::GetPagePosition()
{
    LwpPara* pPara = static_cast<LwpPara*>(GetPosition()->obj());
    if(pPara)
        return pPara;
    //Get the position from its related section
    LwpSection* pSection = NULL;
    LwpFoundry* pFoundry = GetFoundry();
    if(pFoundry)
    {
        while( (pSection = pFoundry->EnumSections(pSection)) )
        {
            if(pSection->GetPageLayout() == this)
                return static_cast<LwpPara*>(pSection->GetPosition()->obj());
        }
    }

    return NULL;
}
LwpHeaderLayout::LwpHeaderLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm )
    : LwpPlacableLayout(objHdr, pStrm)
{
}

LwpHeaderLayout::~LwpHeaderLayout()
{
}

void LwpHeaderLayout::Read()
{
    LwpPlacableLayout::Read();

    if(LwpFileHeader::m_nFileRevision >= 0x000E)
    {
        m_pObjStrm->QuickRead(&m_nBorderOffset, sizeof(m_nBorderOffset));
    }
    else
        m_nBorderOffset = 0;
    m_pObjStrm->SkipExtra();
}


void LwpHeaderLayout::RegisterStyle(XFPageMaster* pm1)
{
    XFHeaderStyle* pHeaderStyle = new XFHeaderStyle();

    //Modify page top margin
    //page top maring: from top of header to the top edge
    double top = GetMarginsValue(MARGIN_TOP);
    pm1->SetMargins(-1, -1, top, -1);

    ParseMargins(pHeaderStyle);
    ParseBorder(pHeaderStyle);
    ParseShadow(pHeaderStyle);
//	ParseBackColor(pHeaderStyle);
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
    LwpMiddleLayout* parent = static_cast<LwpMiddleLayout*> (GetParent()->obj());
    //left margin in SODC: the space from the left edge of body to the left edge of header
    double left = GetMarginsValue(MARGIN_LEFT) - parent->GetMarginsValue(MARGIN_LEFT);
    if(left<=0)	//The left margin in SODC can not be minus value
    {
        left = -1;
    }
    //left margin in SODC: the space from the right edge of header to the right edge of body
    double right = GetMarginsValue(MARGIN_RIGHT) - parent->GetMarginsValue(MARGIN_RIGHT);
    if(right<=0)//The right margin in SODC can not be minus value
    {
        right = -1;
    }
    ph1->SetMargins( left, right, GetMarginsValue(MARGIN_BOTTOM));

    //Word Pro has no dynamic spacing, should be set to false
    ph1->SetDynamicSpace(sal_False);
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
    LwpObject* pStory = m_Content.obj();
    if(pStory)
    {
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
        pChangeMgr->SetHeadFootFribMap(sal_True);

        //Call the RegisterStyle first to register the styles in header paras, and then XFConvert()
        pStory->SetFoundry(m_pFoundry);
        pStory->RegisterStyle();
        //, 06/27/2005
        //register child layout style for framelayout,
        RegisterChildStyle();
        //End
        pChangeMgr->SetHeadFootChange(pHeader);
        pStory->XFConvert(pHeader);

        pChangeMgr->SetHeadFootFribMap(sal_False);
    }
    mp1->SetHeader(pHeader);
}

LwpFooterLayout::LwpFooterLayout( LwpObjectHeader &objHdr, LwpSvStream* pStrm )
    : LwpPlacableLayout( objHdr, pStrm )
{
}

LwpFooterLayout::~LwpFooterLayout()
{
}

void LwpFooterLayout::Read()
{
    LwpPlacableLayout::Read();

    if(LwpFileHeader::m_nFileRevision >= 0x000E)
    {
        m_pObjStrm->QuickRead(&m_nBorderOffset, sizeof(m_nBorderOffset));
    }
    else
        m_nBorderOffset = 0;
    m_pObjStrm->SkipExtra();
}

void LwpFooterLayout::RegisterStyle(XFPageMaster* pm1)
{
    XFFooterStyle* pFooterStyle = new XFFooterStyle();

    //Modify page bottom margin
    //page bottom maring: from bottom of footer to the bottom edge
    double bottom = GetMarginsValue(MARGIN_BOTTOM);
    pm1->SetMargins(-1, -1, -1, bottom);

    ParseMargins(pFooterStyle);
    ParseBorder(pFooterStyle);
    ParseShadow(pFooterStyle);
    ParseBackGround(pFooterStyle);
//	ParseBackColor(pFooterStyle);

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
    LwpMiddleLayout* parent = static_cast<LwpMiddleLayout*> (GetParent()->obj());
    double left = GetMarginsValue(MARGIN_LEFT) - parent->GetMarginsValue(MARGIN_LEFT);
    if(left<=0)	//The left margin in SODC can not be minus value
    {
        left = -1;
    }
    double right = GetMarginsValue(MARGIN_RIGHT) - parent->GetMarginsValue(MARGIN_RIGHT);
    if(right<=0)//The left margin in SODC can not be minus value
    {
        right = -1;
    }
    pFooterStyle->SetMargins( left, right, GetMarginsValue(MARGIN_TOP));

    //Word Pro has no dynamic spacing, should be set to false
    pFooterStyle->SetDynamicSpace(sal_False);
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
    LwpObject* pStory = m_Content.obj(VO_STORY);
    //Call the RegisterStyle first to register the styles in footer paras, and then XFConvert()
    if(pStory)
    {
        LwpGlobalMgr* pGlobal = LwpGlobalMgr::GetInstance();
        LwpChangeMgr* pChangeMgr = pGlobal->GetLwpChangeMgr();
        pChangeMgr->SetHeadFootFribMap(sal_True);

        pStory->SetFoundry(m_pFoundry);
        pStory->RegisterStyle();
        //, 06/27/2005
        //register child layout style for framelayout,
        RegisterChildStyle();
        //End

        pChangeMgr->SetHeadFootChange(pFooter);//add by ,7/6

        pStory->XFConvert(pFooter);

        pChangeMgr->SetHeadFootFribMap(sal_False);
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
//End by

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
