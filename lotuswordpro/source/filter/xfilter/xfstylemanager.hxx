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
/*************************************************************************
 * Change History
 * 2004-12-23 create this file.
 ************************************************************************/
#ifndef		_XFSTYLEMANAGER_HXX
#define		_XFSTYLEMANAGER_HXX

#include	"xfglobal.hxx"
#include	"xffontdecl.hxx"
#include	"xfstylecont.hxx"
#include	"xfconfigmanager.hxx"
#include	<map>
#include	<vector>

class IXFStream;
class IXFStyle;
class XFOutlineStyle;
class XFParaStyle;
class XFTextStyle;
class XFLineNumberConfig;
class XFFootnoteConfig;
class XFEndnoteConfig;

/**
 * @brief
 * Style manager for the filter.
 * This is a global obejct, all styles will be placed here before output.
 */
class XFStyleManager// : public IXFObject
{
public:
    XFStyleManager();
    ~XFStyleManager();
    XFStyleManager(XFStyleManager&){}

public:
    /**
     * @descr	clear all styles, called before load file.
     */
    void		Reset();

    void		AddFontDecl(XFFontDecl& aFontDecl);

    void		AddFontDecl(rtl::OUString name, rtl::OUString family, sal_Bool fixed = false);

    IXFStyle*	AddStyle(IXFStyle *pStyle);

    IXFStyle*	FindStyle(rtl::OUString name);

    XFParaStyle*	FindParaStyle(rtl::OUString name);

    XFTextStyle*	FindTextStyle(rtl::OUString name);

    void		SetLineNumberConfig(XFLineNumberConfig *pLNConfig);

    void		SetFootnoteConfig(XFFootnoteConfig *pFNConfig);

    void		SetEndnoteConfig(XFEndnoteConfig *pFNConfig);

    void		ToXml(IXFStream *strm);

private:
    std::vector<XFFontDecl>	s_aFontDecls;
    //standard styles that can be inherited,<office:styles>
    XFStyleContainer	s_aStdTextStyles;
    XFStyleContainer	s_aStdParaStyles;
    XFStyleContainer	s_aStdStrokeDashStyles;
    XFStyleContainer	s_aStdAreaStyles;
    XFStyleContainer	s_aStdArrowStyles;
    XFConfigManager	s_aConfigManager;
    //automatic styles,<office:automatic-styles>
    XFStyleContainer	s_aTextStyles;
    XFStyleContainer	s_aParaStyles;
    XFStyleContainer s_aListStyles;
    XFStyleContainer	s_aSectionStyles;
    XFStyleContainer	s_aPageMasters;
    XFStyleContainer	s_aMasterpages;
    XFStyleContainer	s_aDateStyles;
    XFStyleContainer	s_aGraphicsStyles;
    XFStyleContainer	s_aTableStyles;
    XFStyleContainer	s_aTableCellStyles;
    XFStyleContainer	s_aTableRowStyles;
    XFStyleContainer	s_aTableColStyles;
    IXFStyle			*s_pOutlineStyle;
    XFStyleContainer s_aRubyStyles;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
