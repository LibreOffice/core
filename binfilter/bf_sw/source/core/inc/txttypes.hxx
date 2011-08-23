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
#ifndef _TXTTYPES_HXX
#define _TXTTYPES_HXX

namespace binfilter {

// Ueberpruefte USHORTs, z.B. Indizes auf Arrays oder garantiert kleine
// Integerwerte, auch von aussen vorgegebene
#define MSHORT USHORT
#define MSHRT_MAX USHRT_MAX
// Koordinaten: Breite, Hoehe und Offsets in X-/Y-Richtung sowie Ascent etc.
#define KSHORT USHORT
#define KSHRT_MAX USHRT_MAX

// Portiontypen
#define POR_LIN			0x0000
#define POR_FLYCNT		0x0001

#define	POR_HOLE		0x0080
#define POR_TMPEND 		0x0081
#define POR_BRK 		0x0082
#define POR_KERN 		0x0083
#define POR_ARROW 		0x0084
#define POR_MULTI 		0x0085

#define POR_TXT 		0x8000
#define POR_LAY 		0x8001
#define POR_PARA		0x8002
#define POR_URL			0x8003
#define POR_HNG 		0x8004

#define	POR_DROP		0x8080
#define POR_TOX 		0x8089
#define POR_ISOTOX		0x808a
#define POR_REF 		0x808b
#define	POR_ISOREF		0x808c

#define	POR_EXP 		0xc080
#define POR_BLANK		0xc081
#define POR_POSTITS     0xc082

#define POR_HYPH		0xd080
#define POR_HYPHSTR 	0xd081
#define POR_SOFTHYPH	0xd082
#define	POR_SOFTHYPHSTR 0xd083
#define POR_SOFTHYPH_COMP 0xd084

#define POR_FLD 		0xe080
#define POR_HIDDEN		0xe081
#define	POR_QUOVADIS	0xe082
#define	POR_ERGOSUM 	0xe083
#define POR_COMBINED 	0xe084
#define POR_FTN         0xe085

#define	POR_FTNNUM		0xe880
#define POR_NUMBER		0xe881
#define POR_BULLET		0xe882
#define POR_GRFNUM		0xe883

#define POR_GLUE		0x0480

#define POR_MARGIN		0x04c0

#define POR_FIX 		0x06c0
#define POR_FLY 		0x06c1

#define	POR_TAB 		0x0750

#define	POR_TABRIGHT	0x07d0
#define	POR_TABCENTER	0x07d1
#define	POR_TABDECIMAL	0x07d2

#define POR_TABLEFT 	0x0740

} //namespace binfilter
#endif	//_TXTTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
