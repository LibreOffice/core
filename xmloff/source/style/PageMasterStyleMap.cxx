/*************************************************************************
 *
 *  $RCSfile: PageMasterStyleMap.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dr $ $Date: 2000-10-18 11:30:51 $
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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif


//______________________________________________________________________________

const XMLPropertyMapEntry aXMLPageMasterStyleMap[] =
{
// page master
    { "PageStyleLayout",        XML_NAMESPACE_STYLE,    sXML_page_usage,                XML_PM_TYPE_PAGESTYLELAYOUT,    0 },
    { "PageWidth",              XML_NAMESPACE_FO,       sXML_page_width,                XML_TYPE_MEASURE,               0 },
    { "PageHeight",             XML_NAMESPACE_FO,       sXML_page_height,               XML_TYPE_MEASURE,               0 },
    { "NumberingType",          XML_NAMESPACE_STYLE,    sXML_num_format,                XML_PM_TYPE_NUMFORMAT | MID_FLAG_MERGE_PROPERTY, 0 },
    { "NumberingType",          XML_NAMESPACE_STYLE,    sXML_num_letter_sync,           XML_PM_TYPE_NUMLETTERSYNC | MID_FLAG_MERGE_PROPERTY, 0 },
    { "PrinterPaperTray",       XML_NAMESPACE_STYLE,    sXML_paper_tray_number,         XML_PM_TYPE_PAPERTRAYNUMBER,    0 },
    { "Landscape",              XML_NAMESPACE_STYLE,    sXML_print_orientation,         XML_PM_TYPE_PRINTORIENTATION,   0 },
    { "TopMargin",              XML_NAMESPACE_FO,       sXML_margin_top,                XML_TYPE_MEASURE,               0 },
    { "BottomMargin",           XML_NAMESPACE_FO,       sXML_margin_bottom,             XML_TYPE_MEASURE,               0 },
    { "LeftMargin",             XML_NAMESPACE_FO,       sXML_margin_left,               XML_TYPE_MEASURE,               0 },
    { "RightMargin",            XML_NAMESPACE_FO,       sXML_margin_right,              XML_TYPE_MEASURE,               0 },
    { "TopBorder",              XML_NAMESPACE_FO,       sXML_border,                    XML_TYPE_BORDER,                CTF_PM_BORDERALL },
    { "TopBorder",              XML_NAMESPACE_FO,       sXML_border_top,                XML_TYPE_BORDER,                CTF_PM_BORDERTOP },
    { "BottomBorder",           XML_NAMESPACE_FO,       sXML_border_bottom,             XML_TYPE_BORDER,                CTF_PM_BORDERBOTTOM },
    { "LeftBorder",             XML_NAMESPACE_FO,       sXML_border_left,               XML_TYPE_BORDER,                CTF_PM_BORDERLEFT },
    { "RightBorder",            XML_NAMESPACE_FO,       sXML_border_right,              XML_TYPE_BORDER,                CTF_PM_BORDERRIGHT },
    { "TopBorder",              XML_NAMESPACE_STYLE,    sXML_border_line_width,         XML_TYPE_BORDER_WIDTH,          CTF_PM_BORDERWIDTHALL },
    { "TopBorder",              XML_NAMESPACE_STYLE,    sXML_border_line_width_top,     XML_TYPE_BORDER_WIDTH,          CTF_PM_BORDERWIDTHTOP },
    { "BottomBorder",           XML_NAMESPACE_STYLE,    sXML_border_line_width_bottom,  XML_TYPE_BORDER_WIDTH,          CTF_PM_BORDERWIDTHBOTTOM },
    { "LeftBorder",             XML_NAMESPACE_STYLE,    sXML_border_line_width_left,    XML_TYPE_BORDER_WIDTH,          CTF_PM_BORDERWIDTHLEFT },
    { "RightBorder",            XML_NAMESPACE_STYLE,    sXML_border_line_width_right,   XML_TYPE_BORDER_WIDTH,          CTF_PM_BORDERWIDTHRIGHT },
    { "BorderDistance",         XML_NAMESPACE_FO,       sXML_padding,                   XML_TYPE_MEASURE,               CTF_PM_PADDINGALL },
    { "TopBorderDistance",      XML_NAMESPACE_FO,       sXML_padding_top,               XML_TYPE_MEASURE,               CTF_PM_PADDINGTOP },
    { "BottomBorderDistance",   XML_NAMESPACE_FO,       sXML_padding_bottom,            XML_TYPE_MEASURE,               CTF_PM_PADDINGBOTTOM },
    { "LeftBorderDistance",     XML_NAMESPACE_FO,       sXML_padding_left,              XML_TYPE_MEASURE,               CTF_PM_PADDINGLEFT },
    { "RightBorderDistance",    XML_NAMESPACE_FO,       sXML_padding_right,             XML_TYPE_MEASURE,               CTF_PM_PADDINGRIGHT },
    { "ShadowFormat",           XML_NAMESPACE_STYLE,    sXML_shadow,                    XML_TYPE_TEXT_SHADOW,           0 },
    { "BackgroundColor",        XML_NAMESPACE_FO,       sXML_background_color,          XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY, CTF_PM_BACKGROUNDCOLOR },
    { "IsBackgroundTransparent",XML_NAMESPACE_FO,       sXML_background_color,          XML_TYPE_ISTRANSPARENT | MID_FLAG_MULTI_PROPERTY, CTF_PM_ISBACKGROUNDTRANS },
// header
    { "HeaderHeight",                   XML_NAMESPACE_SVG,      sXML_height,                    XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERHEIGHT },
    { "HeaderHeight",                   XML_NAMESPACE_FO,       sXML_min_height,                XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERMINHEIGHT },
    { "HeaderDynamic",                  XML_NAMESPACE_STYLE,    sXML__empty,                    XML_TYPE_BOOL | MID_FLAG_SPECIAL_ITEM,          CTF_PM_HEADERDYNAMIC },
    { "HeaderLeftMargin",               XML_NAMESPACE_FO,       sXML_margin_left,               XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERFLAG },
    { "HeaderRightMargin",              XML_NAMESPACE_FO,       sXML_margin_right,              XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERFLAG },
    { "HeaderBodyDistance",             XML_NAMESPACE_FO,       sXML_margin_bottom,             XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERFLAG },
    { "HeaderTopBorder",                XML_NAMESPACE_FO,       sXML_border,                    XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_HEADERBORDERALL },
    { "HeaderTopBorder",                XML_NAMESPACE_FO,       sXML_border_top,                XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_HEADERBORDERTOP },
    { "HeaderBottomBorder",             XML_NAMESPACE_FO,       sXML_border_bottom,             XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_HEADERBORDERBOTTOM },
    { "HeaderLeftBorder",               XML_NAMESPACE_FO,       sXML_border_left,               XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_HEADERBORDERLEFT },
    { "HeaderRightBorder",              XML_NAMESPACE_FO,       sXML_border_right,              XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_HEADERBORDERRIGHT },
    { "HeaderTopBorder",                XML_NAMESPACE_STYLE,    sXML_border_line_width,         XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_HEADERBORDERWIDTHALL },
    { "HeaderTopBorder",                XML_NAMESPACE_STYLE,    sXML_border_line_width_top,     XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_HEADERBORDERWIDTHTOP },
    { "HeaderBottomBorder",             XML_NAMESPACE_STYLE,    sXML_border_line_width_bottom,  XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_HEADERBORDERWIDTHBOTTOM },
    { "HeaderLeftBorder",               XML_NAMESPACE_STYLE,    sXML_border_line_width_left,    XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_HEADERBORDERWIDTHLEFT },
    { "HeaderRightBorder",              XML_NAMESPACE_STYLE,    sXML_border_line_width_right,   XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_HEADERBORDERWIDTHRIGHT },
    { "HeaderBorderDistance",           XML_NAMESPACE_FO,       sXML_padding,                   XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERPADDINGALL },
    { "HeaderTopBorderDistance",        XML_NAMESPACE_FO,       sXML_padding_top,               XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERPADDINGTOP },
    { "HeaderBottomBorderDistance",     XML_NAMESPACE_FO,       sXML_padding_bottom,            XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERPADDINGBOTTOM },
    { "HeaderLeftBorderDistance",       XML_NAMESPACE_FO,       sXML_padding_left,              XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERPADDINGLEFT },
    { "HeaderRightBorderDistance",      XML_NAMESPACE_FO,       sXML_padding_right,             XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_HEADERPADDINGRIGHT },
    { "HeaderShadowFormat",             XML_NAMESPACE_STYLE,    sXML_shadow,                    XML_TYPE_TEXT_SHADOW | MID_FLAG_SPECIAL_ITEM,   CTF_PM_HEADERFLAG },
    { "HeaderBackgroundColor",          XML_NAMESPACE_FO,       sXML_background_color,          XML_TYPE_COLORTRANSPARENT | MID_FLAG_SPECIAL_ITEM | MID_FLAG_MULTI_PROPERTY, CTF_PM_HEADERBACKGROUNDCOLOR },
    { "HeaderIsBackgroundTransparent",  XML_NAMESPACE_FO,       sXML_background_color,          XML_TYPE_ISTRANSPARENT | MID_FLAG_SPECIAL_ITEM | MID_FLAG_MULTI_PROPERTY, CTF_PM_HEADERISBACKGROUNDTRANS },
// footer
    { "FooterHeight",                   XML_NAMESPACE_SVG,      sXML_height,                    XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERHEIGHT },
    { "FooterHeight",                   XML_NAMESPACE_FO,       sXML_min_height,                XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERMINHEIGHT },
    { "FooterDynamic",                  XML_NAMESPACE_STYLE,    sXML__empty,                    XML_TYPE_BOOL | MID_FLAG_SPECIAL_ITEM,          CTF_PM_FOOTERDYNAMIC },
    { "FooterLeftMargin",               XML_NAMESPACE_FO,       sXML_margin_left,               XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERFLAG },
    { "FooterRightMargin",              XML_NAMESPACE_FO,       sXML_margin_right,              XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERFLAG },
    { "FooterBodyDistance",             XML_NAMESPACE_FO,       sXML_margin_bottom,             XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERFLAG },
    { "FooterTopBorder",                XML_NAMESPACE_FO,       sXML_border,                    XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_FOOTERBORDERALL },
    { "FooterTopBorder",                XML_NAMESPACE_FO,       sXML_border_top,                XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_FOOTERBORDERTOP },
    { "FooterBottomBorder",             XML_NAMESPACE_FO,       sXML_border_bottom,             XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_FOOTERBORDERBOTTOM },
    { "FooterLeftBorder",               XML_NAMESPACE_FO,       sXML_border_left,               XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_FOOTERBORDERLEFT },
    { "FooterRightBorder",              XML_NAMESPACE_FO,       sXML_border_right,              XML_TYPE_BORDER | MID_FLAG_SPECIAL_ITEM,        CTF_PM_FOOTERBORDERRIGHT },
    { "FooterTopBorder",                XML_NAMESPACE_STYLE,    sXML_border_line_width,         XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_FOOTERBORDERWIDTHALL },
    { "FooterTopBorder",                XML_NAMESPACE_STYLE,    sXML_border_line_width_top,     XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_FOOTERBORDERWIDTHTOP },
    { "FooterBottomBorder",             XML_NAMESPACE_STYLE,    sXML_border_line_width_bottom,  XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_FOOTERBORDERWIDTHBOTTOM },
    { "FooterLeftBorder",               XML_NAMESPACE_STYLE,    sXML_border_line_width_left,    XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_FOOTERBORDERWIDTHLEFT },
    { "FooterRightBorder",              XML_NAMESPACE_STYLE,    sXML_border_line_width_right,   XML_TYPE_BORDER_WIDTH | MID_FLAG_SPECIAL_ITEM,  CTF_PM_FOOTERBORDERWIDTHRIGHT },
    { "FooterBorderDistance",           XML_NAMESPACE_FO,       sXML_padding,                   XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERPADDINGALL },
    { "FooterTopBorderDistance",        XML_NAMESPACE_FO,       sXML_padding_top,               XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERPADDINGTOP },
    { "FooterBottomBorderDistance",     XML_NAMESPACE_FO,       sXML_padding_bottom,            XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERPADDINGBOTTOM },
    { "FooterLeftBorderDistance",       XML_NAMESPACE_FO,       sXML_padding_left,              XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERPADDINGLEFT },
    { "FooterRightBorderDistance",      XML_NAMESPACE_FO,       sXML_padding_right,             XML_TYPE_MEASURE | MID_FLAG_SPECIAL_ITEM,       CTF_PM_FOOTERPADDINGRIGHT },
    { "FooterShadowFormat",             XML_NAMESPACE_STYLE,    sXML_shadow,                    XML_TYPE_TEXT_SHADOW | MID_FLAG_SPECIAL_ITEM,   CTF_PM_FOOTERFLAG },
    { "FooterBackgroundColor",          XML_NAMESPACE_FO,       sXML_background_color,          XML_TYPE_COLORTRANSPARENT | MID_FLAG_SPECIAL_ITEM | MID_FLAG_MULTI_PROPERTY, CTF_PM_FOOTERBACKGROUNDCOLOR },
    { "FooterIsBackgroundTransparent",  XML_NAMESPACE_FO,       sXML_background_color,          XML_TYPE_ISTRANSPARENT | MID_FLAG_SPECIAL_ITEM | MID_FLAG_MULTI_PROPERTY, CTF_PM_FOOTERISBACKGROUNDTRANS },
    { 0L }
};

