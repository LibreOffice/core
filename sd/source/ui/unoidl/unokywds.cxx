/*************************************************************************
 *
 *  $RCSfile: unokywds.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:42 $
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

#include <unokywds.hxx>

#define UNO_PREFIX "com.sun.star."

// SdUnoStyleFamilies
char __FAR_DATA sUNO_SdUnoStyleFamilies[] = "SdUnoStyleFamilies";
char __FAR_DATA sUNO_Graphic_Style_Family_Name[] = "graphics";

// SdUnoPseudoStyleFamily
char __FAR_DATA sUNO_SdUnoPseudoStyleFamily[] = "SdUnoPseudoStyleFamily";
char __FAR_DATA sUNO_PseudoSheet_Title[] = "title";
char __FAR_DATA sUNO_PseudoSheet_SubTitle[] = "subtitle";
char __FAR_DATA sUNO_PseudoSheet_Background[] = "background";
char __FAR_DATA sUNO_PseudoSheet_Background_Objects[] = "backgroundobjects";
char __FAR_DATA sUNO_PseudoSheet_Notes[] = "notes";
char __FAR_DATA sUNO_PseudoSheet_Outline1[] = "outline1";
char __FAR_DATA sUNO_PseudoSheet_Outline2[] = "outline2";
char __FAR_DATA sUNO_PseudoSheet_Outline3[] = "outline3";
char __FAR_DATA sUNO_PseudoSheet_Outline4[] = "outline4";
char __FAR_DATA sUNO_PseudoSheet_Outline5[] = "outline5";
char __FAR_DATA sUNO_PseudoSheet_Outline6[] = "outline6";
char __FAR_DATA sUNO_PseudoSheet_Outline7[] = "outline7";
char __FAR_DATA sUNO_PseudoSheet_Outline8[] = "outline8";
char __FAR_DATA sUNO_PseudoSheet_Outline9[] = "outline9";

// SdUnoPseudoStyle
char __FAR_DATA sUNO_SdUnoPseudoStyle[] = "SdUnoPseudoStyle";

// SdUnoGraphicStyleFamily
char __FAR_DATA sUNO_SdUnoGraphicStyleFamily[] = "SdUnoGraphicStyleFamily";
char __FAR_DATA sUNO_StyleName_standard[] = "standard";
char __FAR_DATA sUNO_StyleName_objwitharrow[] = "objectwitharrow";
char __FAR_DATA sUNO_StyleName_objwithshadow[] = "objectwithshadow";
char __FAR_DATA sUNO_StyleName_objwithoutfill[] = "objectwithoutfill";
char __FAR_DATA sUNO_StyleName_text[] = "text";
char __FAR_DATA sUNO_StyleName_textbody[] = "textbody";
char __FAR_DATA sUNO_StyleName_textbodyjust[] = "textbodyjustfied";
char __FAR_DATA sUNO_StyleName_textbodyindent[] = "textbodyindent";
char __FAR_DATA sUNO_StyleName_title[] = "title";
char __FAR_DATA sUNO_StyleName_title1[] = "title1";
char __FAR_DATA sUNO_StyleName_title2[] = "title2";
char __FAR_DATA sUNO_StyleName_headline[] = "headline";
char __FAR_DATA sUNO_StyleName_headline1[] = "headline1";
char __FAR_DATA sUNO_StyleName_headline2[] = "headline2";
char __FAR_DATA sUNO_StyleName_measure[] = "measure";

// SdUnoGraphicStyle
char __FAR_DATA sUNO_SdUnoGraphicStyle[] = "SdUnoGraphicStyle";

// SdUnoPageBackground
char __FAR_DATA sUNO_SdUnoPageBackground[] = "SdUnoPageBackground";

// SdLayerManager
char __FAR_DATA sUNO_SdLayerManager[] = "SdUnoLayerManager";

// SdLayer
char __FAR_DATA sUNO_SdLayer[] = "SdUnoLayer";

// SdXShape
char __FAR_DATA sUNO_shape_style[] = "Style";

// services
char __FAR_DATA sUNO_Service_StyleFamily[] = UNO_PREFIX "style.StyleFamily";
char __FAR_DATA sUNO_Service_StyleFamilies[] = UNO_PREFIX "style.StyleFamilies";
char __FAR_DATA sUNO_Service_Style[] = UNO_PREFIX "style.Style";
char __FAR_DATA sUNO_Service_AreaShapeDescriptor[] = UNO_PREFIX "drawing.AreaShapeDescriptor";
char __FAR_DATA sUNO_Service_LineShapeDescriptor[] = UNO_PREFIX "drawing.LineShapeDescriptor";
char __FAR_DATA sUNO_Service_ParagraphProperties[] = UNO_PREFIX "style.ParagraphProperties";
char __FAR_DATA sUNO_Service_CharacterProperties[] = UNO_PREFIX "style.CharacterProperties";
char __FAR_DATA sUNO_Service_Text[] = UNO_PREFIX "drawing.Text";
char __FAR_DATA sUNO_Service_PageBackground[] = UNO_PREFIX "drawing.PageBackground";
char __FAR_DATA sUNO_Service_DrawingLayer[] = UNO_PREFIX "drawing.Layer";
char __FAR_DATA sUNO_Service_DrawingLayerManager[] = UNO_PREFIX "drawing.LayerManager";

// properties
char __FAR_DATA sUNO_Prop_Background[] = "Background";

