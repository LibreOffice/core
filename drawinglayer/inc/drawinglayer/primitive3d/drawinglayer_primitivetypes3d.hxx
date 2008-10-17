/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drawinglayer_primitivetypes3d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2008-06-24 15:30:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX

//////////////////////////////////////////////////////////////////////////////
// define ranges for other libraries

#define PRIMITIVE3D_ID_RANGE_DRAWINGLAYER           (0 << 16)
// #define  PRIMITIVE3D_ID_RANGE_SVX                    (1 << 16)

//////////////////////////////////////////////////////////////////////////////
// local primitives

#define PRIMITIVE3D_ID_GROUPPRIMITIVE3D                 (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 0)
#define PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D          (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 1)
#define PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D         (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 2)
#define PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D       (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 3)
#define PRIMITIVE3D_ID_POLYGONSTROKEPRIMITIVE3D         (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 4)
#define PRIMITIVE3D_ID_POLYGONTUBEPRIMITIVE3D           (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 5)
#define PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D   (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 6)
#define PRIMITIVE3D_ID_SDRCUBEPRIMITIVE3D               (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 7)
#define PRIMITIVE3D_ID_SDREXTRUDEPRIMITIVE3D            (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 8)
#define PRIMITIVE3D_ID_SDRLATHEPRIMITIVE3D              (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 9)
#define PRIMITIVE3D_ID_SDRPOLYPOLYGONPRIMITIVE3D        (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 10)
#define PRIMITIVE3D_ID_SDRSPHEREPRIMITIVE3D             (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 11)
#define PRIMITIVE3D_ID_SHADOWPRIMITIVE3D                (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 12)
#define PRIMITIVE3D_ID_UNIFIEDALPHATEXTUREPRIMITIVE3D   (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 13)
#define PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D       (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 14)
#define PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D         (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 15)
#define PRIMITIVE3D_ID_ALPHATEXTUREPRIMITIVE3D          (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 16)
#define PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D             (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 17)
#define PRIMITIVE3D_ID_HITTESTPRIMITIVE3D               (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 18)

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
