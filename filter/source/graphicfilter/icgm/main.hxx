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

#ifndef CGM_MAIN_HXX
#define CGM_MAIN_HXX

#include "cgm.hxx"

// ---------------------------------------------------------------

#define CGM_LEVEL1						0x0001	// VERSION 1 METAFILE
#define CGM_LEVEL2						0x0002
#define CGM_LEVEL3						0x0003
#define CGM_GDSF_ONLY					0x00F0
#define CGM_UNKNOWN_LEVEL				0x00F1
#define CGM_UNKNOWN_COMMAND				0x00F2
#define CGM_DESCRIPTION					0x00F3

#define CGM_EXTENDED_PRIMITIVES_SET		0x0100	// INDICATES PRIMITIVES WHICH ARE AVAILABLE IN
                                                // LEVEL 1 BUT ARE NOT DEFINED IN ISO 7942(GKS)
#define CGM_DRAWING_PLUS_CONTROL_SET	0x0200	// INDICATES THAT THIS IS AN ADDITIONAL LEVEL 1
                                                // ELEMENT
#define ComOut( Level, Description ) if ( mpCommentOut ) ImplComment( Level, Description );

#define BMCOL( _col ) BitmapColor( (sal_Int8)(_col >> 16 ), (sal_Int8)( _col >> 8 ), (sal_Int8)_col )
#define ALIGN2( _nElementSize ) { _nElementSize = ( _nElementSize + 1 ) & ~1; }
#define ALIGN4( _nElementSize ) { _nElementSize = ( _nElementSize + 3 ) & ~3; }
#define ALIGN8( _nElementSize ) { _nElementSize = ( _nElementSize + 7 ) & ~7; }

#include <vcl/salbtype.hxx>
#include <tools/stream.hxx>
#include <tools/list.hxx>
#include "bundles.hxx"
#include "bitmap.hxx"
#include "elements.hxx"


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
