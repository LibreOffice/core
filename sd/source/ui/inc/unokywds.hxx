/*************************************************************************
 *
 *  $RCSfile: unokywds.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-08 12:40:59 $
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

#ifndef _SD_UNOKYWDS_HXX_
#define _SD_UNOKYWDS_HXX_

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#ifndef SD_DEFINE_KEYWORDS
#define SD_CONSTASCII_ACTION( n, s ) extern sal_Char __FAR_DATA n[sizeof(s)]
#else
#define SD_CONSTASCII_ACTION( n, s ) sal_Char __FAR_DATA n[sizeof(s)] = s
#endif

#define UNO_PREFIX "com.sun.star."

// SdXImpressDocument
SD_CONSTASCII_ACTION( sUNO_Service_DrawingDocument, UNO_PREFIX "drawing.DrawingDocument" );
SD_CONSTASCII_ACTION( sUNO_Service_PresentationDocument, UNO_PREFIX "drawing.PresentationDocument" );

// SdUnoStyleFamilies
SD_CONSTASCII_ACTION( sUNO_SdUnoStyleFamilies, "SdUnoStyleFamilies" );
SD_CONSTASCII_ACTION( sUNO_Graphic_Style_Family_Name, "graphics" );

// SdUnoPseudoStyleFamily
SD_CONSTASCII_ACTION( sUNO_SdUnoPseudoStyleFamily, "SdUnoPseudoStyleFamily" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Title, "title" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_SubTitle, "subtitle" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Background, "background" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Background_Objects, "backgroundobjects" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Notes, "notes" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline1, "outline1" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline2, "outline2" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline3, "outline3" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline4, "outline4" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline5, "outline5" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline6, "outline6" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline7, "outline7" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline8, "outline8" );
SD_CONSTASCII_ACTION( sUNO_PseudoSheet_Outline9, "outline9" );

// SdUnoPseudoStyle
SD_CONSTASCII_ACTION( sUNO_SdUnoPseudoStyle, "SdUnoPseudoStyle" );

// SdUnoGraphicStyleFamily
SD_CONSTASCII_ACTION( sUNO_SdUnoGraphicStyleFamily, "SdUnoGraphicStyleFamily" );
SD_CONSTASCII_ACTION( sUNO_StyleName_standard, "standard" );
SD_CONSTASCII_ACTION( sUNO_StyleName_objwitharrow, "objectwitharrow" );
SD_CONSTASCII_ACTION( sUNO_StyleName_objwithshadow, "objectwithshadow" );
SD_CONSTASCII_ACTION( sUNO_StyleName_objwithoutfill, "objectwithoutfill" );
SD_CONSTASCII_ACTION( sUNO_StyleName_text, "text" );
SD_CONSTASCII_ACTION( sUNO_StyleName_textbody, "textbody" );
SD_CONSTASCII_ACTION( sUNO_StyleName_textbodyjust, "textbodyjustfied" );
SD_CONSTASCII_ACTION( sUNO_StyleName_textbodyindent, "textbodyindent" );
SD_CONSTASCII_ACTION( sUNO_StyleName_title, "title" );
SD_CONSTASCII_ACTION( sUNO_StyleName_title1, "title1" );
SD_CONSTASCII_ACTION( sUNO_StyleName_title2, "title2" );
SD_CONSTASCII_ACTION( sUNO_StyleName_headline, "headline" );
SD_CONSTASCII_ACTION( sUNO_StyleName_headline1, "headline1" );
SD_CONSTASCII_ACTION( sUNO_StyleName_headline2, "headline2" );
SD_CONSTASCII_ACTION( sUNO_StyleName_measure, "measure" );

// SdUnoGraphicStyle
SD_CONSTASCII_ACTION( sUNO_SdUnoGraphicStyle, "SdUnoGraphicStyle" );

// SdUnoPageBackground
SD_CONSTASCII_ACTION( sUNO_SdUnoPageBackground, "SdUnoPageBackground" );

// SdLayerManager
SD_CONSTASCII_ACTION( sUNO_SdLayerManager, "SdUnoLayerManager" );

// SdLayer
SD_CONSTASCII_ACTION( sUNO_SdLayer, "SdUnoLayer" );
SD_CONSTASCII_ACTION( sUNO_LayerName_background,         "background" );
SD_CONSTASCII_ACTION( sUNO_LayerName_background_objects, "backgroundobjects" );
SD_CONSTASCII_ACTION( sUNO_LayerName_layout,             "layout" );
SD_CONSTASCII_ACTION( sUNO_LayerName_controls,           "controls" );
SD_CONSTASCII_ACTION( sUNO_LayerName_measurelines,       "measurelines" );

// SdXShape
SD_CONSTASCII_ACTION( sUNO_shape_style, "Style" );
SD_CONSTASCII_ACTION( sUNO_shape_layername, "LayerName" );

// services
SD_CONSTASCII_ACTION( sUNO_Service_StyleFamily, UNO_PREFIX "style.StyleFamily" );
SD_CONSTASCII_ACTION( sUNO_Service_StyleFamilies, UNO_PREFIX "style.StyleFamilies" );
SD_CONSTASCII_ACTION( sUNO_Service_Style, UNO_PREFIX "style.Style" );
SD_CONSTASCII_ACTION( sUNO_Service_FillProperties, UNO_PREFIX "drawing.FillProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_LineProperties, UNO_PREFIX "drawing.LineProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_ParagraphProperties, UNO_PREFIX "style.ParagraphProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_CharacterProperties, UNO_PREFIX "style.CharacterProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_Text, UNO_PREFIX "drawing.Text" );
SD_CONSTASCII_ACTION( sUNO_Service_TextProperties, UNO_PREFIX "drawing.TextProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_ShadowProperties, UNO_PREFIX "drawing.ShadowProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_ConnectorProperties, UNO_PREFIX "drawing.ConnectorProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_MeasureProperties, UNO_PREFIX "drawing.MeasureProperties" );
SD_CONSTASCII_ACTION( sUNO_Service_PageBackground, UNO_PREFIX "drawing.PageBackground" );
SD_CONSTASCII_ACTION( sUNO_Service_DrawingLayer, UNO_PREFIX "drawing.Layer" );
SD_CONSTASCII_ACTION( sUNO_Service_DrawingLayerManager, UNO_PREFIX "drawing.LayerManager" );

// properties
SD_CONSTASCII_ACTION( sUNO_Prop_Background, "Background" );

#endif

