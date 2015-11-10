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

#include <xfparastyle.hxx>
#include "xfstylemanager.hxx"
#include <xftextstyle.hxx>
#include "ixfstyle.hxx"

XFStyleManager::XFStyleManager() : s_aStdArrowStyles( "arrow" ), s_aTextStyles( "T" ),
    s_aParaStyles( "P" ),s_aListStyles( "L" ),s_aSectionStyles( "Sect" ),
    s_aPageMasters( "PM" ),s_aMasterpages( "MP" ),s_aDateStyles( "N" ),
    s_aGraphicsStyles( "fr" ),s_aTableStyles( "table" ),s_aTableCellStyles( "cell" ),
    s_aTableRowStyles( "row" ),s_aTableColStyles( "col" ),s_pOutlineStyle(nullptr)
{
}

XFStyleManager::~XFStyleManager()
{
    Reset();
}

void    XFStyleManager::Reset()
{
    if( s_pOutlineStyle )
    {
        delete s_pOutlineStyle;
        s_pOutlineStyle = nullptr;
    }

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

void    XFStyleManager::AddFontDecl(XFFontDecl& aFontDecl)
{
    s_aFontDecls.push_back(aFontDecl);
}

IXFStyleRet XFStyleManager::AddStyle(IXFStyle *pStyle)
{
    IXFStyleRet aRet;

    assert(pStyle);
    OUString   name;

    name = pStyle->GetStyleName();

    if( pStyle->GetStyleFamily() == enumXFStyleText )
    {
        if( !name.isEmpty() )
        {
            aRet = s_aStdTextStyles.AddStyle(pStyle);
        }
        else
        {
            aRet = s_aTextStyles.AddStyle(pStyle);
        }
    }
    else if( pStyle->GetStyleFamily() == enumXFStylePara )
    {
        if( !name.isEmpty() )
        {
            aRet = s_aStdParaStyles.AddStyle(pStyle);
        }
        else
        {
            aRet = s_aParaStyles.AddStyle(pStyle);
        }
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleDefaultPara )
    {
        aRet = s_aStdParaStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleList )
    {
        aRet = s_aListStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleSection )
    {
        aRet = s_aSectionStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStylePageMaster )
    {
        aRet = s_aPageMasters.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleMasterPage )
    {
        //Master page don't need name.
        aRet = s_aMasterpages.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleDate )
    {
        aRet = s_aDateStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTime )
    {
        aRet = s_aDateStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleNumber )
    {
        aRet = s_aDateStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStylePercent )
    {
        aRet = s_aDateStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleCurrency )
    {
        aRet = s_aDateStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleGraphics )
    {
        aRet = s_aGraphicsStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTable )
    {
        aRet = s_aTableStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTableCell )
    {
        aRet = s_aTableCellStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTableRow )
    {
        aRet = s_aTableRowStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleTableCol )
    {
        aRet = s_aTableColStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleOutline )
    {
        delete s_pOutlineStyle;
        s_pOutlineStyle = pStyle;
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleStrokeDash )
    {
        aRet = s_aStdStrokeDashStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleArea )
    {
        aRet = s_aStdAreaStyles.AddStyle(pStyle);
    }
    else if( pStyle->GetStyleFamily() == enumXFStyleArrow )
    {
        aRet = s_aStdArrowStyles.AddStyle(pStyle);
    }
    else if (pStyle->GetStyleFamily() == enumXFStyleRuby)
    {
        aRet = s_aRubyStyles.AddStyle(pStyle);
    }

    return aRet;
}

IXFStyle*   XFStyleManager::FindStyle(const OUString& name)
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
        return s_pOutlineStyle;
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

XFParaStyle*    XFStyleManager::FindParaStyle(const OUString& name)
{
    IXFStyle *pStyle = s_aParaStyles.FindStyle(name);
    if( pStyle )
        return static_cast<XFParaStyle*>(pStyle);
    else
        return static_cast<XFParaStyle*>(s_aStdParaStyles.FindStyle(name));
}

XFTextStyle*    XFStyleManager::FindTextStyle(const OUString& name)
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
    std::vector<XFFontDecl>::iterator   itDecl;
    IXFAttrList *pAttrList = pStrm->GetAttrList();

    pAttrList->Clear();
    pStrm->StartElement( "office:font-decls" );

    //font declarations:
    for( itDecl = s_aFontDecls.begin(); itDecl != s_aFontDecls.end(); ++itDecl )
    {
        XFFontDecl &f = *itDecl;

        pAttrList->Clear();
        pAttrList->AddAttribute( "style:name", f.GetFontName() );
        pAttrList->AddAttribute( "fo:font-family", f.GetFontFamily() );
        if( f.GetFontPitchFixed() )
            pAttrList->AddAttribute( "style:font-pitch", "fixed" );
        else
            pAttrList->AddAttribute( "style:font-pitch", "variable" );
        pStrm->StartElement( "style:font-decl" );
        pStrm->EndElement( "style:font-decl" );
    }

    pStrm->EndElement( "office:font-decls" );

    //office:styles:
    pAttrList->Clear();
    pStrm->StartElement( "office:styles" );

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

    pStrm->EndElement( "office:styles" );

    //automatic styles:
    pAttrList->Clear();
    pStrm->StartElement( "office:automatic-styles" );

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

    pStrm->EndElement( "office:automatic-styles" );

    //master:styles
    pAttrList->Clear();
    pStrm->StartElement( "office:master-styles" );
    //masters pages:
    s_aMasterpages.ToXml(pStrm);

    pStrm->EndElement( "office:master-styles" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
