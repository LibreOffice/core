/*************************************************************************
 *
 *  $RCSfile: PageMasterStyleMap.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:20:42 $
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

#define _MAP(name,prefix,token,type,context)  { name, sizeof(name)-1, prefix, token, type, context }
#define PLMAP(name,prefix,token,type,context) \
        _MAP(name,prefix,token,type|XML_TYPE_PROP_PAGE_LAYOUT,context)
#define HFMAP(name,prefix,token,type,context) \
        _MAP(name,prefix,token,type|XML_TYPE_PROP_HEADER_FOOTER,context)

//______________________________________________________________________________

const XMLPropertyMapEntry aXMLPageMasterStyleMap[] =
{
// page master
    PLMAP( "PageStyleLayout",           XML_NAMESPACE_STYLE,    XML_PAGE_USAGE,                 XML_PM_TYPE_PAGESTYLELAYOUT | MID_FLAG_SPECIAL_ITEM,    CTF_PM_PAGEUSAGE ),
    PLMAP( "Width",                 XML_NAMESPACE_FO,       XML_PAGE_WIDTH,                 XML_TYPE_MEASURE,                                       0 ),
    PLMAP( "Height",                    XML_NAMESPACE_FO,       XML_PAGE_HEIGHT,                XML_TYPE_MEASURE,                                       0 ),
    PLMAP( "NumberingType",         XML_NAMESPACE_STYLE,    XML_NUM_FORMAT,                 XML_PM_TYPE_NUMFORMAT | MID_FLAG_MERGE_PROPERTY,        0 ),
    PLMAP( "NumberingType",         XML_NAMESPACE_STYLE,    XML_NUM_LETTER_SYNC,            XML_PM_TYPE_NUMLETTERSYNC,                              0 ),
    PLMAP( "PrinterPaperTray",      XML_NAMESPACE_STYLE,    XML_PAPER_TRAY_NAME,            XML_TYPE_STRING | MID_FLAG_PROPERTY_MAY_EXCEPT,         0 ),
    PLMAP( "IsLandscape",               XML_NAMESPACE_STYLE,    XML_PRINT_ORIENTATION,          XML_PM_TYPE_PRINTORIENTATION,                           0 ),
    PLMAP( "TopMargin",             XML_NAMESPACE_FO,       XML_MARGIN_TOP,                 XML_TYPE_MEASURE,                                       0 ),
    PLMAP( "BottomMargin",          XML_NAMESPACE_FO,       XML_MARGIN_BOTTOM,              XML_TYPE_MEASURE,                                       0 ),
    PLMAP( "LeftMargin",                XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,                                       0 ),
    PLMAP( "RightMargin",               XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,                                       0 ),
    PLMAP( "TopBorder",             XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,                                        CTF_PM_BORDERALL ),
    PLMAP( "TopBorder",             XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,                                        CTF_PM_BORDERTOP ),
    PLMAP( "BottomBorder",          XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,                                        CTF_PM_BORDERBOTTOM ),
    PLMAP( "LeftBorder",                XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,                                        CTF_PM_BORDERLEFT ),
    PLMAP( "RightBorder",               XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,                                        CTF_PM_BORDERRIGHT ),
    PLMAP( "TopBorder",             XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHALL ),
    PLMAP( "TopBorder",             XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHTOP ),
    PLMAP( "BottomBorder",          XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHBOTTOM ),
    PLMAP( "LeftBorder",                XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHLEFT ),
    PLMAP( "RightBorder",               XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,                                  CTF_PM_BORDERWIDTHRIGHT ),
    PLMAP( "TopBorderDistance",     XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,                                       CTF_PM_PADDINGALL ),
    PLMAP( "TopBorderDistance",     XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,                                       CTF_PM_PADDINGTOP ),
    PLMAP( "BottomBorderDistance",  XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,                                       CTF_PM_PADDINGBOTTOM ),
    PLMAP( "LeftBorderDistance",        XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,                                       CTF_PM_PADDINGLEFT ),
    PLMAP( "RightBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,                                       CTF_PM_PADDINGRIGHT ),
    PLMAP( "ShadowFormat",          XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,                                   0 ),
    PLMAP( "BackColor",             XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    0 ),
    PLMAP( "BackTransparent",           XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      0 ),
    PLMAP( "BackGraphicLocation",       XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_GRAPHICPOSITION ),
    PLMAP( "BackGraphicFilter",     XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_GRAPHICFILTER ),
    PLMAP( "BackGraphicURL",            XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_GRAPHICURL ),
    PLMAP( "PrintAnnotations",      XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTANNOTATIONS | MID_FLAG_MULTI_PROPERTY,                         CTF_PM_PRINT_ANNOTATIONS ),
    PLMAP( "PrintCharts",               XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTCHARTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,   CTF_PM_PRINT_CHARTS ),
    PLMAP( "PrintDrawing",          XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTDRAWING | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  CTF_PM_PRINT_DRAWING ),
    PLMAP( "PrintFormulas",         XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTFORMULAS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, CTF_PM_PRINT_FORMULAS ),
    PLMAP( "PrintGrid",             XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTGRID | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,     CTF_PM_PRINT_GRID ),
    PLMAP( "PrintHeaders",          XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTHEADERS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  CTF_PM_PRINT_HEADERS ),
    PLMAP( "PrintObjects",          XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTOBJECTS | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE,  CTF_PM_PRINT_OBJECTS ),
    PLMAP( "PrintZeroValues",           XML_NAMESPACE_STYLE,    XML_PRINT,                      XML_PM_TYPE_PRINTZEROVALUES | MID_FLAG_MERGE_ATTRIBUTE,                         CTF_PM_PRINT_ZEROVALUES ),
    PLMAP( "PrintDownFirst",            XML_NAMESPACE_STYLE,    XML_PRINT_PAGE_ORDER,           XML_PM_TYPE_PRINTPAGEORDER,                             0 ),
    PLMAP( "FirstPageNumber",           XML_NAMESPACE_STYLE,    XML_FIRST_PAGE_NUMBER,          XML_PM_TYPE_FIRSTPAGENUMBER,                            0 ),
    PLMAP( "PageScale",             XML_NAMESPACE_STYLE,    XML_SCALE_TO,                   XML_TYPE_NUMBER16,                                      CTF_PM_SCALETO ),
    PLMAP( "ScaleToPages",          XML_NAMESPACE_STYLE,    XML_SCALE_TO_PAGES,             XML_TYPE_NUMBER16,                                      CTF_PM_SCALETOPAGES ),
    PLMAP( "ScaleToPagesX",         XML_NAMESPACE_STYLE,    XML_SCALE_TO_X,                 XML_TYPE_NUMBER16,                                      CTF_PM_SCALETOX ),
    PLMAP( "ScaleToPagesY",         XML_NAMESPACE_STYLE,    XML_SCALE_TO_Y,                 XML_TYPE_NUMBER16,                                      CTF_PM_SCALETOY ),
    PLMAP( "CenterHorizontally",        XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_HORIZONTAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    PLMAP( "CenterVertically",      XML_NAMESPACE_STYLE,    XML_TABLE_CENTERING,            XML_PM_TYPE_CENTER_VERTICAL | MID_FLAG_MULTI_PROPERTY | MID_FLAG_MERGE_ATTRIBUTE, 0 ),
    PLMAP( "TextColumns",               XML_NAMESPACE_STYLE,    XML_COLUMNS,    MID_FLAG_ELEMENT_ITEM|XML_TYPE_TEXT_COLUMNS, CTF_PM_TEXTCOLUMNS ),
    PLMAP( "RegisterModeActive",        XML_NAMESPACE_STYLE,    XML_REGISTER_TRUE,  XML_TYPE_BOOL, 0 ),
    PLMAP( "RegisterParagraphStyle",    XML_NAMESPACE_STYLE,    XML_REGISTER_TRUTH_REF_STYLE_NAME,  XML_TYPE_STYLENAME| MID_FLAG_SPECIAL_ITEM_IMPORT, CTF_PM_REGISTER_STYLE ),
    PLMAP( "WritingMode",                 XML_NAMESPACE_STYLE,    XML_WRITING_MODE,               XML_TYPE_TEXT_WRITING_MODE, 0 ),

    PLMAP( "GridColor", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_COLOR, XML_TYPE_COLOR, 0 ),
    PLMAP( "GridLines", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_LINES, XML_TYPE_NUMBER16, 0 ),
    PLMAP( "GridBaseHeight", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_BASE_HEIGHT, XML_TYPE_MEASURE, 0 ),
    PLMAP( "GridRubyHeight", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_HEIGHT, XML_TYPE_MEASURE, 0 ),
    PLMAP( "GridMode", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_MODE, XML_TYPE_LAYOUT_GRID_MODE, 0 ),
    PLMAP( "RubyBelow", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_RUBY_BELOW, XML_TYPE_BOOL, 0 ),
    PLMAP( "GridPrint", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_PRINT, XML_TYPE_BOOL, 0 ),
    PLMAP( "GridDisplay", XML_NAMESPACE_STYLE, XML_LAYOUT_GRID_DISPLAY, XML_TYPE_BOOL, 0 ),

    PLMAP( "UserDefinedAttributes", XML_NAMESPACE_TEXT,     XML_XMLNS,                      XML_TYPE_ATTRIBUTE_CONTAINER | MID_FLAG_SPECIAL_ITEM, 0 ),

// footnote
    PLMAP( "FootnoteHeight",            XML_NAMESPACE_STYLE,    XML_FOOTNOTE_MAX_HEIGHT, XML_TYPE_MEASURE, CTF_PM_FTN_HEIGTH ),
    PLMAP( "FootnoteLineAdjust",        XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_TEXT_HORIZONTAL_ADJUST|MID_FLAG_SPECIAL_ITEM,  CTF_PM_FTN_LINE_ADJUST ),
    PLMAP( "FootnoteLineColor",     XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_COLOR|MID_FLAG_SPECIAL_ITEM,       CTF_PM_FTN_LINE_COLOR ),
    PLMAP( "FootnoteLineDistance",  XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM,CTF_PM_FTN_DISTANCE ),
    PLMAP( "FootnoteLineRelativeWidth", XML_NAMESPACE_STYLE,    XML__EMPTY,     XML_TYPE_PERCENT8|MID_FLAG_SPECIAL_ITEM,    CTF_PM_FTN_LINE_WIDTH ),
    PLMAP( "FootnoteLineTextDistance", XML_NAMESPACE_STYLE, XML__EMPTY,     XML_TYPE_MEASURE|MID_FLAG_SPECIAL_ITEM, CTF_PM_FTN_LINE_DISTANCE ),
    PLMAP( "FootnoteLineWeight",        XML_NAMESPACE_STYLE,    XML_FOOTNOTE_SEP,   XML_TYPE_MEASURE16|MID_FLAG_ELEMENT_ITEM,   CTF_PM_FTN_LINE_WEIGTH ),

// header
    HFMAP( "HeaderHeight",              XML_NAMESPACE_SVG,      XML_HEIGHT,                     XML_TYPE_MEASURE,       CTF_PM_HEADERHEIGHT ),
    HFMAP( "HeaderHeight",              XML_NAMESPACE_FO,       XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,       CTF_PM_HEADERMINHEIGHT ),
    HFMAP( "HeaderIsDynamicHeight",     XML_NAMESPACE_STYLE,    XML__EMPTY,                     XML_TYPE_BOOL,          CTF_PM_HEADERDYNAMIC ),
    HFMAP( "HeaderLeftMargin",          XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG ),
    HFMAP( "HeaderRightMargin",         XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG ),
    HFMAP( "HeaderBodyDistance",            XML_NAMESPACE_FO,       XML_MARGIN_BOTTOM,              XML_TYPE_MEASURE,       CTF_PM_HEADERFLAG ),
    HFMAP( "HeaderTopBorder",               XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,        CTF_PM_HEADERBORDERALL ),
    HFMAP( "HeaderTopBorder",               XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_HEADERBORDERTOP ),
    HFMAP( "HeaderBottomBorder",            XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,        CTF_PM_HEADERBORDERBOTTOM ),
    HFMAP( "HeaderLeftBorder",          XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_HEADERBORDERLEFT ),
    HFMAP( "HeaderRightBorder",         XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,        CTF_PM_HEADERBORDERRIGHT ),
    HFMAP( "HeaderTopBorder",               XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHALL ),
    HFMAP( "HeaderTopBorder",               XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHTOP ),
    HFMAP( "HeaderBottomBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHBOTTOM ),
    HFMAP( "HeaderLeftBorder",          XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHLEFT ),
    HFMAP( "HeaderRightBorder",         XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,  CTF_PM_HEADERBORDERWIDTHRIGHT ),
    HFMAP( "HeaderTopBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGALL ),
    HFMAP( "HeaderTopBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGTOP ),
    HFMAP( "HeaderBottomBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGBOTTOM ),
    HFMAP( "HeaderLeftBorderDistance",  XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGLEFT ),
    HFMAP( "HeaderRightBorderDistance", XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,       CTF_PM_HEADERPADDINGRIGHT ),
    HFMAP( "HeaderShadowFormat",            XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,   CTF_PM_HEADERFLAG ),
    HFMAP( "HeaderBackColor",               XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_HEADERFLAG ),
    HFMAP( "HeaderBackTransparent",     XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      CTF_PM_HEADERFLAG ),
    HFMAP( "HeaderBackGraphicLocation", XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_HEADERGRAPHICPOSITION ),
    HFMAP( "HeaderBackGraphicFilter",       XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_HEADERGRAPHICFILTER ),
    HFMAP( "HeaderBackGraphicURL",      XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_HEADERGRAPHICURL ),
    HFMAP( "HeaderDynamicSpacing",        XML_NAMESPACE_STYLE,    XML_DYNAMIC_SPACING,            XML_TYPE_BOOL,          CTF_PM_HEADERFLAG ),

// footer
    HFMAP( "FooterHeight",              XML_NAMESPACE_SVG,      XML_HEIGHT,                     XML_TYPE_MEASURE,       CTF_PM_FOOTERHEIGHT ),
    HFMAP( "FooterHeight",              XML_NAMESPACE_FO,       XML_MIN_HEIGHT,                 XML_TYPE_MEASURE,       CTF_PM_FOOTERMINHEIGHT ),
    HFMAP( "FooterIsDynamicHeight",     XML_NAMESPACE_STYLE,    XML__EMPTY,                     XML_TYPE_BOOL,          CTF_PM_FOOTERDYNAMIC ),
    HFMAP( "FooterLeftMargin",          XML_NAMESPACE_FO,       XML_MARGIN_LEFT,                XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG ),
    HFMAP( "FooterRightMargin",         XML_NAMESPACE_FO,       XML_MARGIN_RIGHT,               XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG ),
    HFMAP( "FooterBodyDistance",            XML_NAMESPACE_FO,       XML_MARGIN_TOP,                 XML_TYPE_MEASURE,       CTF_PM_FOOTERFLAG ),
    HFMAP( "FooterTopBorder",               XML_NAMESPACE_FO,       XML_BORDER,                     XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERALL ),
    HFMAP( "FooterTopBorder",               XML_NAMESPACE_FO,       XML_BORDER_TOP,                 XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERTOP ),
    HFMAP( "FooterBottomBorder",            XML_NAMESPACE_FO,       XML_BORDER_BOTTOM,              XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERBOTTOM ),
    HFMAP( "FooterLeftBorder",          XML_NAMESPACE_FO,       XML_BORDER_LEFT,                XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERLEFT ),
    HFMAP( "FooterRightBorder",         XML_NAMESPACE_FO,       XML_BORDER_RIGHT,               XML_TYPE_BORDER,        CTF_PM_FOOTERBORDERRIGHT ),
    HFMAP( "FooterTopBorder",               XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH,          XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHALL ),
    HFMAP( "FooterTopBorder",               XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_TOP,      XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHTOP ),
    HFMAP( "FooterBottomBorder",            XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_BOTTOM,   XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHBOTTOM ),
    HFMAP( "FooterLeftBorder",          XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_LEFT,     XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHLEFT ),
    HFMAP( "FooterRightBorder",         XML_NAMESPACE_STYLE,    XML_BORDER_LINE_WIDTH_RIGHT,    XML_TYPE_BORDER_WIDTH,  CTF_PM_FOOTERBORDERWIDTHRIGHT ),
    HFMAP( "FooterTopBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING,                    XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGALL ),
    HFMAP( "FooterTopBorderDistance",   XML_NAMESPACE_FO,       XML_PADDING_TOP,                XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGTOP ),
    HFMAP( "FooterBottomBorderDistance",    XML_NAMESPACE_FO,       XML_PADDING_BOTTOM,             XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGBOTTOM ),
    HFMAP( "FooterLeftBorderDistance",  XML_NAMESPACE_FO,       XML_PADDING_LEFT,               XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGLEFT ),
    HFMAP( "FooterRightBorderDistance", XML_NAMESPACE_FO,       XML_PADDING_RIGHT,              XML_TYPE_MEASURE,       CTF_PM_FOOTERPADDINGRIGHT ),
    HFMAP( "FooterShadowFormat",            XML_NAMESPACE_STYLE,    XML_SHADOW,                     XML_TYPE_TEXT_SHADOW,   CTF_PM_FOOTERFLAG ),
    HFMAP( "FooterBackColor",               XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_COLORTRANSPARENT | MID_FLAG_MULTI_PROPERTY,    CTF_PM_FOOTERFLAG ),
    HFMAP( "FooterBackTransparent",     XML_NAMESPACE_FO,       XML_BACKGROUND_COLOR,           XML_TYPE_ISTRANSPARENT | MID_FLAG_MERGE_ATTRIBUTE,      CTF_PM_FOOTERFLAG ),
    HFMAP( "FooterBackGraphicLocation", XML_NAMESPACE_STYLE,    XML_POSITION,                   XML_TYPE_BUILDIN_CMP_ONLY | MID_FLAG_SPECIAL_ITEM,      CTF_PM_FOOTERGRAPHICPOSITION ),
    HFMAP( "FooterBackGraphicFilter",   XML_NAMESPACE_STYLE,    XML_FILTER_NAME,                XML_TYPE_STRING | MID_FLAG_SPECIAL_ITEM,                CTF_PM_FOOTERGRAPHICFILTER ),
    HFMAP( "FooterBackGraphicURL",      XML_NAMESPACE_STYLE,    XML_BACKGROUND_IMAGE,           XML_TYPE_STRING | MID_FLAG_ELEMENT_ITEM,                CTF_PM_FOOTERGRAPHICURL ),
    HFMAP( "FooterDynamicSpacing",        XML_NAMESPACE_STYLE,    XML_DYNAMIC_SPACING,           XML_TYPE_BOOL,          CTF_PM_FOOTERFLAG ),

    { 0L }
};

