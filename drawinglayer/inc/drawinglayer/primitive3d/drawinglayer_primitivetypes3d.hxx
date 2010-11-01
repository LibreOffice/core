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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX

//////////////////////////////////////////////////////////////////////////////
// define ranges for other libraries

#define PRIMITIVE3D_ID_RANGE_DRAWINGLAYER           (0 << 16)
// #define  PRIMITIVE3D_ID_RANGE_SVX                    (1 << 16)

//////////////////////////////////////////////////////////////////////////////
// local primitives

#define PRIMITIVE3D_ID_GROUPPRIMITIVE3D                         (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 0)
#define PRIMITIVE3D_ID_HATCHTEXTUREPRIMITIVE3D                  (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 1)
#define PRIMITIVE3D_ID_MODIFIEDCOLORPRIMITIVE3D                 (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 2)
#define PRIMITIVE3D_ID_POLYGONHAIRLINEPRIMITIVE3D               (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 3)
#define PRIMITIVE3D_ID_POLYGONSTROKEPRIMITIVE3D                 (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 4)
#define PRIMITIVE3D_ID_POLYGONTUBEPRIMITIVE3D                   (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 5)
#define PRIMITIVE3D_ID_POLYPOLYGONMATERIALPRIMITIVE3D           (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 6)
#define PRIMITIVE3D_ID_SDRCUBEPRIMITIVE3D                       (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 7)
#define PRIMITIVE3D_ID_SDREXTRUDEPRIMITIVE3D                    (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 8)
#define PRIMITIVE3D_ID_SDRLATHEPRIMITIVE3D                      (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 9)
#define PRIMITIVE3D_ID_SDRPOLYPOLYGONPRIMITIVE3D                (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 10)
#define PRIMITIVE3D_ID_SDRSPHEREPRIMITIVE3D                     (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 11)
#define PRIMITIVE3D_ID_SHADOWPRIMITIVE3D                        (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 12)
#define PRIMITIVE3D_ID_UNIFIEDTRANSPARENCETEXTUREPRIMITIVE3D    (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 13)
#define PRIMITIVE3D_ID_GRADIENTTEXTUREPRIMITIVE3D               (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 14)
#define PRIMITIVE3D_ID_BITMAPTEXTUREPRIMITIVE3D                 (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 15)
#define PRIMITIVE3D_ID_TRANSPARENCETEXTUREPRIMITIVE3D           (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 16)
#define PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D                     (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 17)
#define PRIMITIVE3D_ID_HIDDENGEOMETRYPRIMITIVE3D                (PRIMITIVE3D_ID_RANGE_DRAWINGLAYER| 18)

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
