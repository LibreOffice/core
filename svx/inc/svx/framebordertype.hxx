/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: framebordertype.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:54:11 $
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

#ifndef SVX_FRAMEBORDERTYPE_HXX
#define SVX_FRAMEBORDERTYPE_HXX

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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

