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
 * Style manager for the filter.
 ************************************************************************/

#include <sal/config.h>

#include <xfilter/xfparastyle.hxx>
#include <xfilter/xfstylemanager.hxx>
#include <xfilter/xftextstyle.hxx>
#include <xfilter/ixfstyle.hxx>

XFStyleManager::XFStyleManager() : s_aStdArrowStyles( u"arrow"_ustr ), s_aTextStyles( u"T"_ustr ),
    s_aParaStyles( u"P"_ustr ),s_aListStyles( u"L"_ustr ),s_aSectionStyles( u"Sect"_ustr ),
    s_aPageMasters( u"PM"_ustr ),s_aMasterpages( u"MP"_ustr ),s_aDateStyles( u"N"_ustr ),
    s_aGraphicsStyles( u"fr"_ustr ),s_aTableStyles( u"table"_ustr ),s_aTableCellStyles( u"cell"_ustr ),
    s_aTableRowStyles( u"row"_ustr ),s_aTableColStyles( u"col"_ustr )
{
}

XFStyleManager::~XFStyleManager()
{
    Reset();
}

void    XFStyleManager::Reset()
{
    s_pOutlineStyle.reset();

    s_aStdTextStyles.Reset();
    s_aStdParaStyles.Reset();
    s_aStdStrokeDashStyles.Reset();
    s_aStdAreaStyles.Reset();
    s_aStdArrowStyles.Reset();
    s_aTextStyles.Reset();
    s_aParaStyles.Reset();
    s_aListStyles.Reset();
    s_aSectionStyles.Reset();
    s_aPageMasters.Reset();
    s_aMasterpages.Reset();
    s_aDateStyles.Reset();
    s_aGraphicsStyles.Reset();
    s_aConfigManager.Reset();
    //must clear all static containers.
    s_aFontDecls.clear();
}

void    XFStyleManager::AddFontDecl(XFFontDecl const & aFontDecl)
{
    s_aFontDecls.push_back(aFontDecl);
}

IXFStyleRet XFStyleManager::AddStyle(std::unique_ptr<IXFStyle> pStyle)
{
    IXFStyleRet aRet;

    assert(pStyle);
    OUString name = pStyle->GetStyleName();

    if( pStyle->GetStyleFamily() == enumXFStyleText )
    {
        if( !name.isEmpty() )
        {
            aRet = s_aStdTextStyles.AddStyle(std::move(pStyle));
        }
        else
        {
            aRet = s_aTextStyles.AddStyle(std::move(pStyle));
        }
    }
    else if( pStyle->GetStyleFamily() == enumXFStylePara )
    {
        if( !name.isEmpty() )
        {
            aRet = s_aStdParaStyles.AddStyle(std::move(pStyle));
        }
        else
        {
            aRet = s_aParaStyles.AddStyle(std::move(pStyle));
        }
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleDefaultPara )
    {
        aRet = s_aStdParaStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleList )
    {
        aRet = s_aListStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleSection )
    {
        aRet = s_aSectionStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStylePageMaster )
    {
        aRet = s_aPageMasters.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleMasterPage )
    {
        //Master page don't need name.
        aRet = s_aMasterpages.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleDate )
    {
        aRet = s_aDateStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTime )
    {
        aRet = s_aDateStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleNumber )
    {
        aRet = s_aDateStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStylePercent )
    {
        aRet = s_aDateStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleCurrency )
    {
        aRet = s_aDateStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleGraphics )
    {
        aRet = s_aGraphicsStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTable )
    {
        aRet = s_aTableStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTableCell )
    {
        aRet = s_aTableCellStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTableRow )
    {
        aRet = s_aTableRowStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTableCol )
    {
        aRet = s_aTableColStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleOutline )
    {
        s_pOutlineStyle = std::move(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleStrokeDash )
    {
        aRet = s_aStdStrokeDashStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleArea )
    {
        aRet = s_aStdAreaStyles.AddStyle(std::move(pStyle));
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleArrow )
    {
        aRet = s_aStdArrowStyles.AddStyle(std::move(pStyle));
    }
    else if (pStyle->GetStyleFamily() == enumXFStyleRuby)
    {
        aRet = s_aRubyStyles.AddStyle(std::move(pStyle));
    }

    return aRet;
}

IXFStyle*   XFStyleManager::FindStyle(std::u16string_view name)
{
    IXFStyle *pStyle = FindParaStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = FindTextStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aListStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aSectionStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aPageMasters.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aMasterpages.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aDateStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aGraphicsStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aTableStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aTableCellStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aTableRowStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aTableColStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    if(s_pOutlineStyle && s_pOutlineStyle->GetStyleName() == name )
        return s_pOutlineStyle.get();
    pStyle = s_aStdStrokeDashStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aStdAreaStyles.FindStyle(name);
    if( pStyle )
        return pStyle;
    pStyle = s_aStdArrowStyles.FindStyle(name);
    if( pStyle )
        return pStyle;

    return nullptr;
}

XFParaStyle*    XFStyleManager::FindParaStyle(std::u16string_view name)
{
    IXFStyle *pStyle = s_aParaStyles.FindStyle(name);
    if( pStyle )
        return static_cast<XFParaStyle*>(pStyle);
    else
        return static_cast<XFParaStyle*>(s_aStdParaStyles.FindStyle(name));
}

XFTextStyle*    XFStyleManager::FindTextStyle(std::u16string_view name)
{
    IXFStyle *pStyle = s_aTextStyles.FindStyle(name);
    if( pStyle )
        return static_cast<XFTextStyle*>(pStyle);
    else
        return static_cast<XFTextStyle*>(s_aStdTextStyles.FindStyle(name));
}

void    XFStyleManager::SetLineNumberConfig(XFLineNumberConfig *pLNConf)
{
    s_aConfigManager.SetLineNumberConfig(pLNConf);
}

void    XFStyleManager::SetFootnoteConfig(XFFootnoteConfig *pFNConfig)
{
    s_aConfigManager.SetFootnoteConfig(pFNConfig);
}

void    XFStyleManager::SetEndnoteConfig(XFEndnoteConfig *pENConfig)
{
    s_aConfigManager.SetEndnoteConfig(pENConfig);
}

void    XFStyleManager::ToXml(IXFStream *pStrm)
{
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pStrm->StartElement( u"office:font-decls"_ustr );

    //font declarations:
    for (const auto & fontDecl : s_aFontDecls)
    {
        pAttrList->Clear();
        pAttrList->AddAttribute( u"style:name"_ustr, fontDecl.GetFontName() );
        pAttrList->AddAttribute( u"fo:font-family"_ustr, fontDecl.GetFontFamily() );
        pAttrList->AddAttribute( u"style:font-pitch"_ustr, u"variable"_ustr );
        pStrm->StartElement( u"style:font-decl"_ustr );
        pStrm->EndElement( u"style:font-decl"_ustr );
    }

    pStrm->EndElement( u"office:font-decls"_ustr );

    //office:styles:
    pAttrList->Clear();
    pStrm->StartElement( u"office:styles"_ustr );

    s_aStdParaStyles.ToXml(pStrm);
    s_aStdTextStyles.ToXml(pStrm);
    s_aStdStrokeDashStyles.ToXml(pStrm);
    s_aStdAreaStyles.ToXml(pStrm);
    s_aStdArrowStyles.ToXml(pStrm);
    //date,time styles:
    s_aDateStyles.ToXml(pStrm);
    s_aConfigManager.ToXml(pStrm);
    //for optimist.
    s_aListStyles.ToXml(pStrm);

    if( s_pOutlineStyle )
        s_pOutlineStyle->ToXml(pStrm);

    pStrm->EndElement( u"office:styles"_ustr );

    //automatic styles:
    pAttrList->Clear();
    pStrm->StartElement( u"office:automatic-styles"_ustr );

    s_aTableStyles.ToXml(pStrm);
    s_aTableCellStyles.ToXml(pStrm);
    s_aTableRowStyles.ToXml(pStrm);
    s_aTableColStyles.ToXml(pStrm);
    s_aParaStyles.ToXml(pStrm);
    s_aTextStyles.ToXml(pStrm);
    s_aSectionStyles.ToXml(pStrm);
    s_aPageMasters.ToXml(pStrm);
    s_aRubyStyles.ToXml(pStrm);
    //graphics:
    s_aGraphicsStyles.ToXml(pStrm);

    pStrm->EndElement( u"office:automatic-styles"_ustr );

    //master:styles
    pAttrList->Clear();
    pStrm->StartElement( u"office:master-styles"_ustr );
    //masters pages:
    s_aMasterpages.ToXml(pStrm);

    pStrm->EndElement( u"office:master-styles"_ustr );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
