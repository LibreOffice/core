/*************************************************************************
 *
 *  $RCSfile: PageMasterStyleMap.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: dvo $ $Date: 2001-06-29 21:07:17 $
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
#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

using namespace ::xmloff::token;

//______________________________________________________________________________

const XMLPropertyMapEntry aXMLPageMasterStyleMap[] =
{
// page master
    { "PageStyleLayout",        XML_NAMESPACE_STYLE,    XML_PAGE_USAGE,                 XML_PM_TYPE_PAGESTYLELAYOUT | MID_FLAG_SPECIAL_ITEM,    CTF_PM_PAGEUSAGE },
    { "Width",                  XML_NAMESPACE_FO,       XML_PAGE_WIDTH,                 XML_TYPE_MEASURE,                                       0 },
    { "Height",                 XML_NAMESPACE_FO,       XML_PAGE_HEIGHT,                XML_TYPE_MEASURE,                                       0 },
    { "NumberingType",          XML_NAMESPACE_STYLE,    XML_NUM_FORMAT,                 XML_PM_TYPE_NUMFORMAT | MID_FLAG_MERGE_PROPERTY,        0 },
    { "NumberingType",          XML_NAMESPACE_STYLE,    XML_NUM_LETTER_SYNC,            XML_PM_TYPE_NUMLETTERSYNC,                              0 },
//  { "PrinterPaperTray",       XML_NAMESPACE_STYLE,    XML_PAPER_TRAY_NUMBER,          XML_PM_TYPE_PAPERTRAYNUMBER,                            0 },
    { "IsLandscape",            XML_NAMESPACE_STYLE,    XML_PRINT_ORIENTATION,          XML_PM_TYPE_PRINTORIENTATION,                           0 },
    { "TopMargin",              XML_NAMESPACE_FO,       XML_MARGIN_TOP,                 XML_TYPE_MEASURE,                                       0 },
    { "BottomMargin",           XML_NAMESPACE_FO,       XML_MARGIN_BOTTOM,              XML_TYPE_MEASURE,                                       0 },
    { "LeftMargin",             XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,                                       0 },
    { "RightMargin",            XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,                                       0 },
    { "TopBorder",              XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,                                        CTF_PM_BORDERALL },
    { "TopBorder",              XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,                                        CTF_PM_BORDERTOP },
    { "BottomBorder",           XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,                                        CTF_PM_BORDERBOTTOM },
    { "LeftBorder",             XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,                                        CTF_PM_BORDERLEFT },
    { "RightBorder",            XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,                                        CTF_PM_BORDERRIGHT },
    { "TopBorder",              XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHALL },
    { "TopBorder",              XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHTOP },
    { "BottomBorder",           XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHBOTTOM },
    { "LeftBorder",             XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHLEFT },
    { "RightBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHRIGHT },
    { "TopBorderDistance",      XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,                                       CTF_PM_PADDINGALL },
    { "TopBorderDistance",      XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,                                       CTF_PM_PADDINGTOP },
    { "BottomBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,                                       CTF_PM_PADDINGBOTTOM },
    { "LeftBorderDistance",     XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,                                       CTF_PM_PADDINGLEFT },
    { "RightBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,                                       CTF_PM_PADDINGRIGHT },
    { "ShadowFormat",           XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,                                   0 },
    { "BackColor",              XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    0 },
    { "BackTransparent",        XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      0 },
    { "BackGraphicLocation",    XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_GRAPHICPOSITION },
    { "BackGraphicFilter",      XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_GRAPHICFILTER },
    { "BackGraphicURL",         XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_GRAPHICURL },
    { "PrintAnnotations",       XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTANNOTATIONS | MID_FLAG_MULTI_PROPERTY,                         0 },
    { "PrintCharts",            XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTCHARTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,   0 },
    { "PrintDrawing",           XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTDRAWING | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  0 },
    { "PrintFormulas",          XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTFORMULAS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 },
    { "PrintGrid",              XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTGRID | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,     0 },
    { "PrintHeaders",           XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTHEADERS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  0 },
    { "PrintObjects",           XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTOBJECTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  0 },
    { "PrintZeroValues",        XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTZEROVALUES | MID_FLAG_MERGE_ATTRIBUTE,                         0 },
    { "PrintDownFirst",         XML_NAMESPACE_STYLE,    XML_PRINT_PAGE_ORDER,           XML_PM_TYPE_PRINTPAGEORDER,                             0 },
    { "FirstPageNumber",        XML_NAMESPACE_STYLE,    XML_FIRST_PAGE_NUMBER,          XML_PM_TYPE_FIRSTPAGENUMBER,                            0 },
    { "PageScale",              XML_NAMESPACE_STYLE,    XML_SCALE_TO,                   XML_TYPE_NUMBER16,                                      CTF_PM_SCALETO },
    { "ScaleToPages",           XML_NAMESPACE_STYLE,    XML_SCALE_TO_PAGES,             XML_TYPE_NUMBER16,                                      CTF_PM_SCALETOPAGES },
    { "CenterHorizontally",     XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_HORIZONTAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 },
    { "CenterVertically",       XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_VERTICAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 },
    { "TextColumns",            XML_NAMESPACE_STYLE,    XML_COLUMNS,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_PM_TEXTCOLUMNS },
    { "RegisterModeActive",     XML_NAMESPACE_STYLE,    XML_REGISTER_TRUE,  XML_TYPE_BOOL, 0 },
    { "RegisterParagraphStyle", XML_NAMESPACE_STYLE,    XML_REGISTER_TRUTH_REF_STYLE_NAME,  XML_TYPE_STRING| MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_PM_REGISTER_STYLE },

// footnote
    { "FootnoteHeight",         XML_NAMESPACE_STYLE,    XML_FOOTNOTE_MAX_HEIGHT, XML_TYPE_MEASURE, CTF_PM_FTN_HEIGTH },
    { "FootnoteLineAdjust",     XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_TEXT_HORIZONTAL_ADJUST|MID_FLAG_SPECIAL_ITEM,  CTF_PM_FTN_LINE_ADJUST },
    { "FootnoteLineColor",      XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM,       CTF_PM_FTN_LINE_COLOR },
    { "FootnoteLineDistance",   XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM,CTF_PM_FTN_DISTANCE },
    { "FootnoteLineRelativeWidth", XML_NAMESPACE_STYLE, XML__EMPTY,     XML_TYPE_PERCENT8|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_WIDTH },
    { "FootnoteLineTextDistance", XML_NAMESPACE_STYLE,  XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM, CTF_PM_FTN_LINE_DISTANCE },
    { "FootnoteLineWeight",     XML_NAMESPACE_STYLE,    XML_FOOTNOTE_SEP,   XML_TYPE_MEASURE16|MID_FLAG_ELEMENT_ITEM,   CTF_PM_FTN_LINE_WEIGTH },

// header
    { "HeaderHeight",               XML_NAMESPACE_SVG,      XML_HEIGHT,                     XML_TYPE_MEASURE,       CTF_PM_HEADERHEIGHT },
    { "HeaderHeight",               XML_NAMESPACE_FO,       XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,       CTF_PM_HEADERMINHEIGHT },
    { "HeaderIsDynamicHeight",      XML_NAMESPACE_STYLE,    XML__EMPTY,                     XML_TYPE_BOOL,          CTF_PM_HEADERDYNAMIC },
    { "HeaderLeftMargin",           XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG },
    { "HeaderRightMargin",          XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG },
    { "HeaderBodyDistance",         XML_NAMESPACE_FO,       XML_MARGIN_BOTTOM,              XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG },
    { "HeaderTopBorder",            XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,        CTF_PM_HEADERBORDERALL },
    { "HeaderTopBorder",            XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_HEADERBORDERTOP },
    { "HeaderBottomBorder",         XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,        CTF_PM_HEADERBORDERBOTTOM },
    { "HeaderLeftBorder",           XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERLEFT },
    { "HeaderRightBorder",          XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,        CTF_PM_HEADERBORDERRIGHT },
    { "HeaderTopBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHALL },
    { "HeaderTopBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHTOP },
    { "HeaderBottomBorder",         XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHBOTTOM },
    { "HeaderLeftBorder",           XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHLEFT },
    { "HeaderRightBorder",          XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHRIGHT },
    { "HeaderTopBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGALL },
    { "HeaderTopBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGTOP },
    { "HeaderBottomBorderDistance", XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGBOTTOM },
    { "HeaderLeftBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGLEFT },
    { "HeaderRightBorderDistance",  XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGRIGHT },
    { "HeaderShadowFormat",         XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,   CTF_PM_HEADERFLAG },
    { "HeaderBackColor",            XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_HEADERFLAG },
    { "HeaderBackTransparent",      XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      CTF_PM_HEADERFLAG },
    { "HeaderBackGraphicLocation",  XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_HEADERGRAPHICPOSITION },
    { "HeaderBackGraphicFilter",    XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_HEADERGRAPHICFILTER },
    { "HeaderBackGraphicURL",       XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_HEADERGRAPHICURL },
// footer
    { "FooterHeight",               XML_NAMESPACE_SVG,      XML_HEIGHT,                     XML_TYPE_MEASURE,       CTF_PM_FOOTERHEIGHT },
    { "FooterHeight",               XML_NAMESPACE_FO,       XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,       CTF_PM_FOOTERMINHEIGHT },
    { "FooterIsDynamicHeight",      XML_NAMESPACE_STYLE,    XML__EMPTY,                     XML_TYPE_BOOL,          CTF_PM_FOOTERDYNAMIC },
    { "FooterLeftMargin",           XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG },
    { "FooterRightMargin",          XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG },
    { "FooterBodyDistance",         XML_NAMESPACE_FO,       XML_MARGIN_TOP,                 XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG },
    { "FooterTopBorder",            XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERALL },
    { "FooterTopBorder",            XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERTOP },
    { "FooterBottomBorder",         XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERBOTTOM },
    { "FooterLeftBorder",           XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERLEFT },
    { "FooterRightBorder",          XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERRIGHT },
    { "FooterTopBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHALL },
    { "FooterTopBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHTOP },
    { "FooterBottomBorder",         XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHBOTTOM },
    { "FooterLeftBorder",           XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHLEFT },
    { "FooterRightBorder",          XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHRIGHT },
    { "FooterTopBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGALL },
    { "FooterTopBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGTOP },
    { "FooterBottomBorderDistance", XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGBOTTOM },
    { "FooterLeftBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGLEFT },
    { "FooterRightBorderDistance",  XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGRIGHT },
    { "FooterShadowFormat",         XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,   CTF_PM_FOOTERFLAG },
    { "FooterBackColor",            XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_FOOTERFLAG },
    { "FooterBackTransparent",      XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      CTF_PM_FOOTERFLAG },
    { "FooterBackGraphicLocation",  XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_FOOTERGRAPHICPOSITION },
    { "FooterBackGraphicFilter",    XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_FOOTERGRAPHICFILTER },
    { "FooterBackGraphicURL",       XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_FOOTERGRAPHICURL },
    { 0L }
};

