/*************************************************************************
 *
 *  $RCSfile: css1kywd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
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
#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#include "css1kywd.hxx"

/*  */

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS_mimetype, "text/css" );

/*  */

// ein par allgemeine Strings

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_import, "import" );

// Feature: PrintExt
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_page, "page" );
//sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_media, "media" );
// /Feature: PrintExt

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_important, "important" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_link, "link" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_visited, "visited" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_first_letter, "first-letter" );

// Feature: PrintExt
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_left, "left" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_right, "right" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_first, "first" );
// /Feature: PrintExt

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_url, "url" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_rgb, "rgb" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_UNIT_pt, "pt" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_UNIT_mm, "mm" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_UNIT_cm, "cm" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_UNIT_pc, "pc" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_UNIT_inch, "in" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_UNIT_px, "px" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_UNIT_em, "em" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_UNIT_ex, "ex" );

/*  */

// Strings fuer Font-Properties

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_font_family, "font-family" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_serif, "serif" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_sans_serif, "sans-serif" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_cursive, "cursive" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_fantasy, "fantasy" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_monospace, "monospace" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_font_style, "font-style" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_normal, "normal" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_italic, "italic" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_oblique, "oblique" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_font_variant, "font-variant" );

//sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_normal, "normal" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_small_caps, "small-caps" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_font_weight, "font-weight" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_extra_light, "extra-light" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_light, "light" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_demi_light, "demi-light" );
//sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_medium, "medium" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_demi_bold, "demi-bold" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_bold, "bold" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_extra_bold, "extra-bold" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_lighter, "lighter" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_bolder, "bolder" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_font_size, "font-size" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_xx_small, "xx-small" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_x_small, "x-small" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_small, "small" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_medium, "medium" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_large, "large" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_x_large, "x-large" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_xx_large, "xx-large" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_larger, "larger" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_smaller, "smaller" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_font, "font" );



/*  */

// Strings fuer Farb- und Hintergrund-Properties

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_color, "color" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_background, "background" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_background_color, "background-color" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_transparent, "transparent" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_repeat, "repeat" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_repeat_x, "repeat-x" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_repeat_y, "repeat-y" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_no_repeat, "no-repeat" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_top, "top" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_middle, "middle" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_bottom, "bottom" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_scroll, "scroll" );


/*  */

// Strings fuer Text-Properties

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_letter_spacing, "letter-spacing" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_text_decoration, "text-decoration" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_none, "none" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_underline, "underline" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_line_through, "line-through" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_blink, "blink" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_text_align, "text-align" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_left, "left" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_center, "center" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_right, "right" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_justify, "justify" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_text_indent, "text-indent" );


sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_line_height, "line-height" );


/*  */

// Strings fuer Box-Properties

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_margin_left, "margin-left" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_margin_right, "margin-right" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_margin_top, "margin-top" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_margin_bottom, "margin-bottom" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_margin, "margin" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_padding_top, "padding-top" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_padding_bottom, "padding-bottom" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_padding_left, "padding-left" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_padding_right, "padding-right" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_padding, "padding" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_auto, "auto" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_left_width, "border-left-width" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_right_width, "border-right-width" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_top_width, "border-top-width" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_bottom_width, "border-bottom-width" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_width, "border-width" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_color, "border-color" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_style, "border-style" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_left, "border-left" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_right, "border-right" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_top, "border-top" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border_bottom, "border-bottom" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_border, "border" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_thin, "thin" );
//sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_medium, "medium" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_thick, "thick" );

//sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_none, "none" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_dotted, "dotted" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_dashed, "dashed" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_solid, "solid" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_double, "double" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_groove, "groove" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_ridge, "ridge" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_inset, "inset" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_outset, "outset" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_width, "width" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_height, "height" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_float, "float" );

/*  */

// Strings fuer Positioning

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_position, "position" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_absolute, "absolute" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_relative, "relative" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_static, "static" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_left, "left" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_top, "top" );

/*  */

// Feature: PrintExt

// Strings fuer Printing Extensions

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_page_break_before, "page-break-before" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_page_break_after, "page-break-after" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_page_break_inside, "page-break-inside" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_size, "size" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_widows, "widows" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_orphans, "orphans" );
//sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_P_marks, "marks" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_always, "always" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_avoid, "avoid" );

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_portrait, "portrait" );
sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_landscape, "landscape" );

//sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_crop, "crop" );
//sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_PV_cross, "cross" );

// /Feature: PrintExt

sal_Char __FAR_DATA CSS1_CONSTASCII_DEF( sCSS1_class_abs_pos, "sd-abs-pos" );

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/css1kywd.cxx,v 1.1.1.1 2000-09-18 17:14:55 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.25  2000/09/18 16:04:43  willem.vandorp
      OpenOffice header added.

      Revision 1.24  2000/04/10 12:20:55  mib
      unicode

      Revision 1.23  1999/06/11 14:44:00  MIB
      Individuelle Umrandungs-Abstaende


      Rev 1.22   11 Jun 1999 16:44:00   MIB
   Individuelle Umrandungs-Abstaende

      Rev 1.21   15 Apr 1999 13:44:52   MIB
   background-color

      Rev 1.20   15 Apr 1998 14:40:44   MIB
   Zwei-seitige Printing-Extensions

      Rev 1.19   03 Apr 1998 15:11:32   MIB
   CSS1-Padding

      Rev 1.18   26 Nov 1997 19:09:04   MA
   includes

      Rev 1.17   10 Oct 1997 10:14:22   MIB
   CSS1-Printing-Ext: @page,page-break-before/after (noch kein left/right)

      Rev 1.16   30 Jul 1997 12:27:18   MIB
   absolute positioning

      Rev 1.15   02 Jul 1997 17:11:28   MIB
   neue CSS1-Border-Properties

      Rev 1.14   21 May 1997 11:11:42   MIB
   <LINK REL=STYLESHEET> zum Enbinden von externen Style-Sheets

      Rev 1.13   22 Oct 1996 10:28:38   MIB
   Umstellung von CSS1 auf Klein-Buchstaben (u.a. wegen Amaya)

      Rev 1.12   15 Oct 1996 13:35:40   MIB
   letter-spacing/Laufweite imp./exp.

      Rev 1.11   14 Oct 1996 16:42:24   MIB
   Import CSS1-Umrandung (border)

      Rev 1.10   13 Oct 1996 12:12:40   MIB
   first-letter-Pseudo-Element/DropCaps

      Rev 1.9   11 Oct 1996 16:48:00   MIB
   font-variant

      Rev 1.8   24 Sep 1996 12:47:42   MIB
   bug fix: Einheit fuer Inch ist in nicht inch

      Rev 1.7   19 Aug 1996 18:04:56   MIB
   Anpassungen an Draft vom 26.Juli 96

      Rev 1.6   12 Aug 1996 10:29:56   MIB
   Anpassungen an draft vom 26.7.96, Background-Property

      Rev 1.5   07 Aug 1996 13:21:30   MIB
   Import von color und text-decoration

      Rev 1.4   10 Jul 1996 15:41:36   MIB
   Includes fuer precompiled header gerichtet

      Rev 1.3   03 Jul 1996 16:51:48   MIB
   visited und link

      Rev 1.2   18 Jun 1996 12:19:28   MIB
   margin[-[left|right|top|bottom]]?, text-indent

      Rev 1.1   17 Jun 1996 15:57:26   MIB
   restliche Keywords fuer Font-Properties

      Rev 1.0   16 Jun 1996 14:32:30   MIB
   Initial revision.

*************************************************************************/


