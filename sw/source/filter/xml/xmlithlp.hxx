/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SW_XMLITHLP_HXX
#define _SW_XMLITHLP_HXX

#include <sal/types.h>
#include <xmloff/xmlement.hxx>
#include "hintids.hxx"  // for following include
#include <editeng/brshitem.hxx> // for SvxGraphicsPosition

class SvxBorderLine;
struct SvXMLEnumMapEntry;
class SvXMLUnitConverter;
class Color;
namespace rtl { class OUString; }



/** Define various helper variables and functions for xmlimpit.cxx and
 * xmlexpit.cxx.  */


#define SVX_XML_BORDER_STYLE_NONE 0
#define SVX_XML_BORDER_STYLE_SOLID 1
#define SVX_XML_BORDER_STYLE_DOUBLE 2

#define SVX_XML_BORDER_WIDTH_THIN 0
#define SVX_XML_BORDER_WIDTH_MIDDLE 1
#define SVX_XML_BORDER_WIDTH_THICK 2


sal_Bool lcl_frmitems_parseXMLBorder( const ::rtl::OUString& rValue,
                                      const SvXMLUnitConverter& rUnitConverter,
                                      sal_Bool& rHasStyle, sal_uInt16& rStyle,
                                      sal_Bool& rHasWidth, sal_uInt16& rWidth,
                                      sal_uInt16& rNamedWidth,
                                      sal_Bool& rHasColor, Color& rColor );

void lcl_frmitems_setXMLBorderWidth( SvxBorderLine& rLine,
                                     sal_uInt16 nOutWidth, sal_uInt16 nInWidth,
                                     sal_uInt16 nDistance );

void lcl_frmitems_setXMLBorderWidth( SvxBorderLine& rLine,
                                     sal_uInt16 nWidth, sal_Bool bDouble );

sal_Bool lcl_frmitems_setXMLBorder( SvxBorderLine*& rpLine,
                                    sal_Bool bHasStyle, sal_uInt16 nStyle,
                                    sal_Bool bHasWidth, sal_uInt16 nWidth,
                                    sal_uInt16 nNamedWidth,
                                    sal_Bool bHasColor, const Color& rColor );

void lcl_frmitems_setXMLBorder( SvxBorderLine*& rpLine,
                                sal_uInt16 nWidth, sal_uInt16 nOutWidth,
                                sal_uInt16 nInWidth, sal_uInt16 nDistance );

void lcl_frmitems_MergeXMLHoriPos( SvxGraphicPosition& ePos,
                                   SvxGraphicPosition eHori );

void lcl_frmitems_MergeXMLVertPos( SvxGraphicPosition& ePos,
                                     SvxGraphicPosition eVert );

extern const sal_uInt16 aSBorderWidths[];
extern const sal_uInt16 aDBorderWidths[5*11];

extern const struct SvXMLEnumMapEntry psXML_BorderStyles[];
extern const struct SvXMLEnumMapEntry psXML_NamedBorderWidths[];
extern const struct SvXMLEnumMapEntry psXML_BrushRepeat[];
extern const struct SvXMLEnumMapEntry psXML_BrushHoriPos[];
extern const struct SvXMLEnumMapEntry psXML_BrushVertPos[];
extern const struct SvXMLEnumMapEntry psXML_BreakType[];
extern const struct SvXMLEnumMapEntry aXMLTableAlignMap[];
extern const struct SvXMLEnumMapEntry aXMLTableVAlignMap[];
extern const struct SvXMLEnumMapEntry aXML_KeepTogetherType[];


#endif
