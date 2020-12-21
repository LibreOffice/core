/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLITHLP_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLITHLP_HXX

#include <sal/types.h>
#include <xmloff/xmlement.hxx>
#include <editeng/brushitem.hxx>

namespace editeng { class SvxBorderLine; }

template<typename EnumT> struct SvXMLEnumMapEntry;
class SvXMLUnitConverter;
class Color;

/** Define various helper variables and functions for xmlimpit.cxx and
 * xmlexpit.cxx.  */
bool sw_frmitems_parseXMLBorder( std::u16string_view rValue,
                                      const SvXMLUnitConverter& rUnitConverter,
                                      bool& rHasStyle, sal_uInt16& rStyle,
                                      bool& rHasWidth, sal_uInt16& rWidth,
                                      sal_uInt16& rNamedWidth,
                                      bool& rHasColor, Color& rColor );

bool sw_frmitems_setXMLBorder( std::unique_ptr<editeng::SvxBorderLine>& rpLine,
                                    bool bHasStyle, sal_uInt16 nStyle,
                                    bool bHasWidth, sal_uInt16 nWidth,
                                    sal_uInt16 nNamedWidth,
                                    bool bHasColor, const Color& rColor );

void sw_frmitems_setXMLBorder( std::unique_ptr<editeng::SvxBorderLine>& rpLine,
                                sal_uInt16 nWidth, sal_uInt16 nOutWidth,
                                sal_uInt16 nInWidth, sal_uInt16 nDistance );

void sw_frmitems_MergeXMLHoriPos( SvxGraphicPosition& ePos,
                                   SvxGraphicPosition eHori );

void sw_frmitems_MergeXMLVertPos( SvxGraphicPosition& ePos,
                                     SvxGraphicPosition eVert );

extern const struct SvXMLEnumMapEntry<sal_uInt16> psXML_BorderStyles[];
extern const struct SvXMLEnumMapEntry<sal_uInt16> psXML_NamedBorderWidths[];
extern const struct SvXMLEnumMapEntry<SvxGraphicPosition> psXML_BrushRepeat[];
extern const struct SvXMLEnumMapEntry<SvxGraphicPosition> psXML_BrushHoriPos[];
extern const struct SvXMLEnumMapEntry<SvxGraphicPosition> psXML_BrushVertPos[];
extern const struct SvXMLEnumMapEntry<sal_uInt16> psXML_BreakType[];
extern const struct SvXMLEnumMapEntry<sal_Int16> aXMLTableAlignMap[];
extern const struct SvXMLEnumMapEntry<sal_Int16> aXMLTableVAlignMap[];
extern const struct SvXMLEnumMapEntry<sal_uInt16> aXML_KeepTogetherType[];

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
