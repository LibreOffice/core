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
#ifndef _SVX_IMPGRF_HXX
#define _SVX_IMPGRF_HXX

// include ---------------------------------------------------------------

#include <bf_svtools/filter.hxx>

namespace binfilter {

// Funktionen ------------------------------------------------------------

// returnt einen static Graphic-Filter, wird einmalig angelegt,
// steht immer zur Verfuegung, DARF NIE geloescht werden!!!!
GraphicFilter* GetGrfFilter();
USHORT	FillFilter( GraphicFilter& rFilter );

// class SvxImportGraphicDialog ------------------------------------------
#ifndef SV_NODIALOG

#define ENABLE_STANDARD			((USHORT)0x0001)	// Standard-Button
#define ENABLE_LINK				((USHORT)0x0002)	// Verkn"upfungs-Box
#define ENABLE_STD_AND_LINK		(ENABLE_STANDARD | ENABLE_LINK)
#define ENABLE_PROPERTY			((USHORT)0x0004)	// Eigenschaften-Button
#define ENABLE_ALL				((USHORT)0x0007)	// alle
#define ENABLE_PROP_WITHOUTLINK ((USHORT)0x0008)	// Eigenschaften ohne Link
#define ENABLE_EMPTY_FILENAMES	((USHORT)0x0010)	// Leere Dateinamen zulassen

#endif	// SV_NODIALOG

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
