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

#ifndef _SVDITEXT_HXX
#define _SVDITEXT_HXX

// SvxItem-Mapping. Wird benoetigt um die SvxItem-Header erfolgreich zu includen

namespace binfilter {

//      textitem.hxx        editdata.hxx
#define ITEMID_COLOR        EE_CHAR_COLOR
#define ITEMID_FONT         EE_CHAR_FONTINFO
#define ITEMID_FONTHEIGHT   EE_CHAR_FONTHEIGHT
#define ITEMID_CHARSCALE_W	EE_CHAR_FONTWIDTH
#define ITEMID_WEIGHT       EE_CHAR_WEIGHT
#define ITEMID_UNDERLINE    EE_CHAR_UNDERLINE
#define ITEMID_CROSSEDOUT   EE_CHAR_STRIKEOUT
#define ITEMID_POSTURE      EE_CHAR_ITALIC
#define ITEMID_CONTOUR      EE_CHAR_OUTLINE
#define ITEMID_SHADOWED     EE_CHAR_SHADOW
#define ITEMID_ESCAPEMENT   EE_CHAR_ESCAPEMENT
#define ITEMID_AUTOKERN     EE_CHAR_PAIRKERNING
#define ITEMID_WORDLINEMODE EE_CHAR_WLM

//      paraitem.hxx       editdata.hxx
#define ITEMID_ADJUST      EE_PARA_JUST
#define ITEMID_FIELD       EE_FEATURE_FIELD

}//end of namespace binfilter
#endif  // _SVDITEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
