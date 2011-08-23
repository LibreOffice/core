/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SDUIKS_HXX
#define _SDUIKS_HXX

namespace binfilter {

#define UIK_SHAPEFACTORY					UIK_DRAWING(00)
#define UIK_POLYGONFACTORY					UIK_DRAWING(01)
#define UIK_TEXTSHAPEFACTORY				UIK_DRAWING(02)
#define UIK_SNAPFACTORY						UIK_DRAWING(03)

#define UIK_DRAWPAGES						UIK_DRAWING(10)
#define UIK_DRAWPAGESUMMARIZER				UIK_DRAWING(11)
#define UIK_DRAWPAGEXPANDER					UIK_DRAWING(12)

#define UIK_XSHAPEDESCRIPTOR				UIK_DRAWING(20)
#define UIK_XSHAPE							UIK_DRAWING(21)
#define UIK_XSHAPECONNECTOR					UIK_DRAWING(22)
#define UIK_XCONNECTABLESHAPE				UIK_DRAWING(23)
#define UIK_XSHAPE3D						UIK_DRAWING(24)
#define UIK_XSHAPEGROUP						UIK_DRAWING(25)
#define UIK_XSHAPEMIRROR					UIK_DRAWING(26)
#define UIK_XSHAPEALIGNER					UIK_DRAWING(27)
#define UIK_XSHAPEARRANGER					UIK_DRAWING(28)
#define UIK_XCONVERTABLESHAPE				UIK_DRAWING(29)
#define UIK_XANIMATEDSHAPEFACTORY			UIK_DRAWING(2a)

#define UIK_DRAWPAGE	  					UIK_DRAWING(30)
#define UIK_SHAPES		  					UIK_DRAWING(31)
#define UIK_XSHAPEGROUPER                   UIK_DRAWING(35)
#define UIK_XSHAPECOMBINER                  UIK_DRAWING(36)
#define UIK_SHAPEBINDER                      UIK_DRAWING(37)
#define UIK_DRAWLAYER                       UIK_DRAWING(38)

#define SD_UIK_PRESENTATION                 UIK_DRAWING(40)
#define SD_UIK_PRESENTATION_NAVIGATION      UIK_DRAWING(41)
#define SD_UIK_CUSTOM_PRESENTATION          UIK_DRAWING(42)
#define SD_UIK_CUSTOM_PRESENTATIONS			UIK_DRAWING(43)
//#define SD_UIK_CUSTOM_PRESENTATIONS			UIK_DRAWING(44)

#define SD_UIK_ZOOM							UIK_DRAWING(50)
#define SD_UIK_DRAW_VIEW					UIK_DRAWING(51)
#define SD_UIK_OUTLINE_VIEW					UIK_DRAWING(52)
#define SD_UIK_SLIDE_VIEW                   UIK_DRAWING(53)

#define SD_UIK_PRESENTATION_STYLES          UIK_DRAWING(60)

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
