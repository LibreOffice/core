/*************************************************************************
 *
 *  $RCSfile: xmlithlp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dvo $ $Date: 2001-07-12 14:42:19 $
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

#ifndef _SW_XMLITHLP_HXX
#define _SW_XMLITHLP_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _XMLOFF_XMLEMENT_HXX
#include <xmloff/xmlement.hxx>
#endif

#ifndef _HINTIDS_HXX
#include "hintids.hxx"  // for following include
#endif

#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx> // for SvxGraphicsPosition
#endif

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


#endif
