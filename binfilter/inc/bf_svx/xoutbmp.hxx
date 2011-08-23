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

#ifndef _XOUTBMP_HXX
#define _XOUTBMP_HXX

#ifndef _SV_GRAPH_HXX
#include <vcl/graph.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
class VirtualDevice;
class INetURLObject;
class Polygon;
namespace binfilter {
class GraphicFilter;

// -----------
// - Defines -
// -----------

#define XOUTBMP_MIRROR_HORZ				0x00000001L
#define XOUTBMP_MIRROR_VERT				0x00000010L

#define XOUTBMP_CONTOUR_HORZ			0x00000001L
#define XOUTBMP_CONTOUR_VERT			0x00000002L
#define XOUTBMP_CONTOUR_EDGEDETECT		0x00000004L

#define XOUTBMP_DONT_EXPAND_FILENAME	0x10000000L
#define XOUTBMP_USE_GIF_IF_POSSIBLE		0x20000000L
#define XOUTBMP_USE_GIF_IF_SENSIBLE		0x40000000L
#define XOUTBMP_USE_NATIVE_IF_POSSIBLE	0x80000000L

// --------------
// - XOutBitmap -
// --------------


class XOutBitmap
{
public:

    static GraphicFilter* pGrfFilter;
};

// ----------------
// - DitherBitmap -
// ----------------

BOOL DitherBitmap( Bitmap& rBitmap );

}//end of namespace binfilter
#endif // _XOUTBMP_HXX
