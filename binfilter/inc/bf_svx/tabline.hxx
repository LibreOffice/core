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
#ifndef _SVX_TAB_LINE_HXX
#define _SVX_TAB_LINE_HXX

/*************************************************************************
|*
|* Linien-Tab-Page
|*
\************************************************************************/

//defines fuer die Symbolauswahl auf der Page
//Positive Werte (0,...n) gibt den Indes in pSymbollist an, sonst :
#define SVX_SYMBOLTYPE_NONE	      (-3)  //Symbol unsichtbar
#define SVX_SYMBOLTYPE_AUTO	      (-2)  //Symbol automatisch erzeugen
#define SVX_SYMBOLTYPE_BRUSHITEM  (-1)  //Symbol ist als Graphic im Item enthalten:
#define SVX_SYMBOLTYPE_UNKNOWN  (-100) //unbekannt/nicht initialisiert

#endif // _SVX_TAB_LINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
