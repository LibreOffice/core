/*************************************************************************
 *
 *  $RCSfile: drawuiks.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:00:55 $
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
