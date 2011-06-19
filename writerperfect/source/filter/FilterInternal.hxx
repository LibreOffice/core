/* FilterInternal: Debugging information
 *
 * Copyright (C) 2002-2003 William Lachance (wrlach@gmail.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpd.sourceforge.net
 *
 */
#ifndef __FILTERINTERNAL_HXX__
#define __FILTERINTERNAL_HXX__

#ifdef DEBUG
#include <stdio.h>
#define WRITER_DEBUG_MSG(M) printf M
#else
#define WRITER_DEBUG_MSG(M)
#endif

const double fDefaultSideMargin = 1.0; // inches
const double fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const double fDefaultPageHeight = 11.0; // inches

#endif
