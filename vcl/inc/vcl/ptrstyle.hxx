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

#ifndef _VCL_PTRSTYLE_HXX
#define _VCL_PTRSTYLE_HXX

#include <vcl/sv.h>

// -----------------
// - Pointer-Types -
// -----------------

typedef sal_uInt16 PointerStyle;

#define POINTER_ARROW                   ((PointerStyle)0)
#define POINTER_NULL                    ((PointerStyle)1)
#define POINTER_WAIT                    ((PointerStyle)2)
#define POINTER_TEXT                    ((PointerStyle)3)
#define POINTER_HELP                    ((PointerStyle)4)
#define POINTER_CROSS                   ((PointerStyle)5)
#define POINTER_MOVE                    ((PointerStyle)6)
#define POINTER_NSIZE                   ((PointerStyle)7)
#define POINTER_SSIZE                   ((PointerStyle)8)
#define POINTER_WSIZE                   ((PointerStyle)9)
#define POINTER_ESIZE                   ((PointerStyle)10)
#define POINTER_NWSIZE                  ((PointerStyle)11)
#define POINTER_NESIZE                  ((PointerStyle)12)
#define POINTER_SWSIZE                  ((PointerStyle)13)
#define POINTER_SESIZE                  ((PointerStyle)14)
#define POINTER_WINDOW_NSIZE            ((PointerStyle)15)
#define POINTER_WINDOW_SSIZE            ((PointerStyle)16)
#define POINTER_WINDOW_WSIZE            ((PointerStyle)17)
#define POINTER_WINDOW_ESIZE            ((PointerStyle)18)
#define POINTER_WINDOW_NWSIZE           ((PointerStyle)19)
#define POINTER_WINDOW_NESIZE           ((PointerStyle)20)
#define POINTER_WINDOW_SWSIZE           ((PointerStyle)21)
#define POINTER_WINDOW_SESIZE           ((PointerStyle)22)
#define POINTER_HSPLIT                  ((PointerStyle)23)
#define POINTER_VSPLIT                  ((PointerStyle)24)
#define POINTER_HSIZEBAR                ((PointerStyle)25)
#define POINTER_VSIZEBAR                ((PointerStyle)26)
#define POINTER_HAND                    ((PointerStyle)27)
#define POINTER_REFHAND                 ((PointerStyle)28)
#define POINTER_PEN                     ((PointerStyle)29)
#define POINTER_MAGNIFY                 ((PointerStyle)30)
#define POINTER_FILL                    ((PointerStyle)31)
#define POINTER_ROTATE                  ((PointerStyle)32)
#define POINTER_HSHEAR                  ((PointerStyle)33)
#define POINTER_VSHEAR                  ((PointerStyle)34)
#define POINTER_MIRROR                  ((PointerStyle)35)
#define POINTER_CROOK                   ((PointerStyle)36)
#define POINTER_CROP                    ((PointerStyle)37)
#define POINTER_MOVEPOINT               ((PointerStyle)38)
#define POINTER_MOVEBEZIERWEIGHT        ((PointerStyle)39)
#define POINTER_MOVEDATA                ((PointerStyle)40)
#define POINTER_COPYDATA                ((PointerStyle)41)
#define POINTER_LINKDATA                ((PointerStyle)42)
#define POINTER_MOVEDATALINK            ((PointerStyle)43)
#define POINTER_COPYDATALINK            ((PointerStyle)44)
#define POINTER_MOVEFILE                ((PointerStyle)45)
#define POINTER_COPYFILE                ((PointerStyle)46)
#define POINTER_LINKFILE                ((PointerStyle)47)
#define POINTER_MOVEFILELINK            ((PointerStyle)48)
#define POINTER_COPYFILELINK            ((PointerStyle)49)
#define POINTER_MOVEFILES               ((PointerStyle)50)
#define POINTER_COPYFILES               ((PointerStyle)51)
#define POINTER_NOTALLOWED              ((PointerStyle)52)
#define POINTER_DRAW_LINE               ((PointerStyle)53)
#define POINTER_DRAW_RECT               ((PointerStyle)54)
#define POINTER_DRAW_POLYGON            ((PointerStyle)55)
#define POINTER_DRAW_BEZIER             ((PointerStyle)56)
#define POINTER_DRAW_ARC                ((PointerStyle)57)
#define POINTER_DRAW_PIE                ((PointerStyle)58)
#define POINTER_DRAW_CIRCLECUT          ((PointerStyle)59)
#define POINTER_DRAW_ELLIPSE            ((PointerStyle)60)
#define POINTER_DRAW_FREEHAND           ((PointerStyle)61)
#define POINTER_DRAW_CONNECT            ((PointerStyle)62)
#define POINTER_DRAW_TEXT               ((PointerStyle)63)
#define POINTER_DRAW_CAPTION            ((PointerStyle)64)
#define POINTER_CHART                   ((PointerStyle)65)
#define POINTER_DETECTIVE               ((PointerStyle)66)
#define POINTER_PIVOT_COL               ((PointerStyle)67)
#define POINTER_PIVOT_ROW               ((PointerStyle)68)
#define POINTER_PIVOT_FIELD             ((PointerStyle)69)
#define POINTER_CHAIN                   ((PointerStyle)70)
#define POINTER_CHAIN_NOTALLOWED        ((PointerStyle)71)
#define POINTER_TIMEEVENT_MOVE          ((PointerStyle)72)
#define POINTER_TIMEEVENT_SIZE          ((PointerStyle)73)
#define POINTER_AUTOSCROLL_N            ((PointerStyle)74)
#define POINTER_AUTOSCROLL_S            ((PointerStyle)75)
#define POINTER_AUTOSCROLL_W            ((PointerStyle)76)
#define POINTER_AUTOSCROLL_E            ((PointerStyle)77)
#define POINTER_AUTOSCROLL_NW           ((PointerStyle)78)
#define POINTER_AUTOSCROLL_NE           ((PointerStyle)79)
#define POINTER_AUTOSCROLL_SW           ((PointerStyle)80)
#define POINTER_AUTOSCROLL_SE           ((PointerStyle)81)
#define POINTER_AUTOSCROLL_NS           ((PointerStyle)82)
#define POINTER_AUTOSCROLL_WE           ((PointerStyle)83)
#define POINTER_AUTOSCROLL_NSWE         ((PointerStyle)84)
#define POINTER_AIRBRUSH                ((PointerStyle)85)
#define POINTER_TEXT_VERTICAL           ((PointerStyle)86)
#define POINTER_PIVOT_DELETE            ((PointerStyle)87)

// --> FME 2004-07-30 #i32329# Enhanced table selection
#define POINTER_TAB_SELECT_S            ((PointerStyle)88)
#define POINTER_TAB_SELECT_E            ((PointerStyle)89)
#define POINTER_TAB_SELECT_SE           ((PointerStyle)90)
#define POINTER_TAB_SELECT_W            ((PointerStyle)91)
#define POINTER_TAB_SELECT_SW           ((PointerStyle)92)
// <--

// --> FME 2004-08-16 #i20119# Paintbrush tool
#define POINTER_PAINTBRUSH              ((PointerStyle)93)
// <--

#define POINTER_COUNT                   94

#endif // _VCL_PTRSTYLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
