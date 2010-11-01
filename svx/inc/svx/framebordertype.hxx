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

#ifndef SVX_FRAMEBORDERTYPE_HXX
#define SVX_FRAMEBORDERTYPE_HXX

#include "svx/svxdllapi.h"

namespace svx {

// ============================================================================

/** Enumerates all borders a frame selection control can contain. */
enum FrameBorderType
{
    FRAMEBORDER_NONE,       /// No border (special state).
    FRAMEBORDER_LEFT,       /// Left frame border.
    FRAMEBORDER_RIGHT,      /// Right frame border.
    FRAMEBORDER_TOP,        /// Top frame border.
    FRAMEBORDER_BOTTOM,     /// Bottom frame border.
    FRAMEBORDER_HOR,        /// Inner horizontal frame border.
    FRAMEBORDER_VER,        /// Inner vertical frame border.
    FRAMEBORDER_TLBR,       /// Top-left to bottom-right frame border.
    FRAMEBORDER_BLTR        /// Bottom-left to top-right frame border.
};

/** The number of valid frame border types (excluding FRAMEBORDER_NONE). */
const int FRAMEBORDERTYPE_COUNT = 8;

/** Returns the frame border type from a 0-based integer index. */
SVX_DLLPUBLIC FrameBorderType GetFrameBorderTypeFromIndex( size_t nIndex );

/** Returns the zero-based index of a valid frame border type. */
size_t GetIndexFromFrameBorderType( FrameBorderType eBorder );

// ============================================================================

} // namespace svx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
