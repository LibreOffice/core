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

#ifndef _LWPPAGELAYOUT_HXX
#define _LWPPAGELAYOUT_HXX

#include "lwplayout.hxx"

class LwpHeaderLayout;
class LwpFooterLayout;

class LwpPageLayout: public LwpLayout
{
public:
    LwpPageLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    virtual ~LwpPageLayout();
    virtual void RegisterStyle();
    OUString RegisterEndnoteStyle();
    virtual void Parse(IXFStream* pOutputStream);
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_PAGE_LAYOUT;}
protected:
    void Read();
    LwpHeaderLayout* GetHeaderLayout();
    LwpFooterLayout* GetFooterLayout();
    //void RegisterChildStyle();
    void ParseGeometry(XFPageMaster* pm1);
    void ParseMargins(XFPageMaster* pm1);
    void ParseBorders(XFPageMaster* pm1);
    // add by , 06/03/2005
    void ParsePatternFill(XFPageMaster* pm1);
    void ParseBackGround(XFPageMaster* pm1);
    // end add
    void ParseBackColor(XFPageMaster* pm1);
    void ParseWaterMark(XFPageMaster* pm1);
    void ParseColumns(XFPageMaster* pm1);
    void ParseShadow(XFPageMaster* pm1);
    void ParseFootNoteSeparator(XFPageMaster* pm1);
    double GetMarginWidth();
    void GetWidthAndHeight(double& fWidth, double& fHeight);
    double GetWidth();
    double GetHeight();
protected:
    LwpAtomHolder*	m_pPrinterBinName;
    sal_uInt16		m_nPrinterBin;
    sal_Int32		m_nBdroffset;
    LwpAtomHolder*	m_pPaperName;
    XFPageMaster* m_pXFPageMaster;
public:
    sal_Bool HasColumns();
    sal_Bool HasFillerPageText(LwpFoundry* pFoundry);
    void ConvertFillerPageText(XFContentContainer* pCont);
    void ResetXFColumns();
    LwpPageLayout* GetOddChildLayout();
    virtual sal_Int32 GetPageNumber(sal_uInt16 nLayoutNumber = 0);
    sal_Bool operator <(LwpPageLayout& Other);
    LwpPara* GetPagePosition();
};

#include "xfilter/xfmasterpage.hxx"

class LwpHeaderLayout: public LwpPlacableLayout
{
public:
    LwpHeaderLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpHeaderLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_HEADER_LAYOUT;}
    void RegisterStyle( XFPageMaster* pm1 );
    void RegisterStyle( XFMasterPage* mp1 );
protected:
    void Read();
    void ParseMargins( XFHeaderStyle* ph1 );
    void ParseBorder( XFHeaderStyle* ph1 );
    void ParseShadow( XFHeaderStyle* ph1 );
    // add by , 06/03/2005
    void ParsePatternFill(XFHeaderStyle* ph1);
    void ParseBackGround(XFHeaderStyle* ph1);
    // end add
    void ParseBackColor(XFHeaderStyle* ph1);

    void ParseWaterMark(XFHeaderStyle* ph1);
    //End by
private:
    sal_Int32 m_nBorderOffset;
};

class LwpFooterLayout: public LwpPlacableLayout
{
public:
    LwpFooterLayout(LwpObjectHeader &objHdr, LwpSvStream* pStrm);
    ~LwpFooterLayout();
    virtual LWP_LAYOUT_TYPE GetLayoutType () { return LWP_FOOTER_LAYOUT;}
    void RegisterStyle(XFPageMaster* pm1);
    void RegisterStyle(XFMasterPage* mp1);
protected:
    void Read();
    void ParseMargins( XFFooterStyle* pFooterStyle );
    void ParseBorder( XFFooterStyle* pFooterStyle );
    void ParseShadow( XFFooterStyle* pFooterStyle );
    // add by , 06/03/2005
    void ParsePatternFill(XFFooterStyle* pFooterStyle);
    void ParseBackGround(XFFooterStyle* pFooterStyle);
    // end add

    void ParseBackColor(XFFooterStyle* pFooterStyle);

    void ParseWaterMark(XFFooterStyle* pFooterStyle);
    //End by

private:
    sal_Int32 m_nBorderOffset;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
