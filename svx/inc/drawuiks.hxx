/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawuiks.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:30:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SDUIKS_HXX
#define _SDUIKS_HXX

#include <usr/uiks.hxx>

#define UIK_SHAPEFACTORY                    UIK_DRAWING(00)
#define UIK_POLYGONFACTORY                  UIK_DRAWING(01)
#define UIK_TEXTSHAPEFACTORY                UIK_DRAWING(02)
#define UIK_SNAPFACTORY                     UIK_DRAWING(03)

#define UIK_DRAWPAGES                       UIK_DRAWING(10)
#define UIK_DRAWPAGESUMMARIZER              UIK_DRAWING(11)
#define UIK_DRAWPAGEXPANDER                 UIK_DRAWING(12)

#define UIK_XSHAPEDESCRIPTOR                UIK_DRAWING(20)
#define UIK_XSHAPE                          UIK_DRAWING(21)
#define UIK_XSHAPECONNECTOR                 UIK_DRAWING(22)
#define UIK_XCONNECTABLESHAPE               UIK_DRAWING(23)
#define UIK_XSHAPE3D                        UIK_DRAWING(24)
#define UIK_XSHAPEGROUP                     UIK_DRAWING(25)
#define UIK_XSHAPEMIRROR                    UIK_DRAWING(26)
#define UIK_XSHAPEALIGNER                   UIK_DRAWING(27)
#define UIK_XSHAPEARRANGER                  UIK_DRAWING(28)
#define UIK_XCONVERTABLESHAPE               UIK_DRAWING(29)
#define UIK_XANIMATEDSHAPEFACTORY           UIK_DRAWING(2a)

#define UIK_DRAWPAGE                        UIK_DRAWING(30)
#define UIK_SHAPES                          UIK_DRAWING(31)
#define UIK_XSHAPEGROUPER                   UIK_DRAWING(35)
#define UIK_XSHAPECOMBINER                  UIK_DRAWING(36)
#define UIK_SHAPEBINDER                      UIK_DRAWING(37)
#define UIK_DRAWLAYER                       UIK_DRAWING(38)

#define SD_UIK_PRESENTATION                 UIK_DRAWING(40)
#define SD_UIK_PRESENTATION_NAVIGATION      UIK_DRAWING(41)
#define SD_UIK_CUSTOM_PRESENTATION          UIK_DRAWING(42)
#define SD_UIK_CUSTOM_PRESENTATIONS         UIK_DRAWING(43)
//#define SD_UIK_CUSTOM_PRESENTATIONS           UIK_DRAWING(44)

#define SD_UIK_ZOOM                         UIK_DRAWING(50)
#define SD_UIK_DRAW_VIEW                    UIK_DRAWING(51)
#define SD_UIK_OUTLINE_VIEW                 UIK_DRAWING(52)
#define SD_UIK_SLIDE_VIEW                   UIK_DRAWING(53)

#define SD_UIK_PRESENTATION_STYLES          UIK_DRAWING(60)

#endif
