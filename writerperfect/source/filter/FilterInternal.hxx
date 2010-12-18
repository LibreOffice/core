/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* FilterInternal: Debugging information
 *
 * Copyright (C) 2002-2003 William Lachance (william.lachance@sympatico.ca)
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

#ifndef _FILTERINTERNAL_HXX
#define _FILTERINTERNAL_HXX

#include <stdio.h>
#ifdef DEBUG
#define WRITER_DEBUG_MSG(M) printf M
#else
#define WRITER_DEBUG_MSG(M)
#endif

const float fDefaultSideMargin = 1.0f; // inches
const float fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const float fDefaultPageHeight = 11.0f; // inches

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
