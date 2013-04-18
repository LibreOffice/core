/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_PRIMITIVETYPES3D_HXX

//////////////////////////////////////////////////////////////////////////////
// define ranges for other libraries

#define PRIMITIVE3D_ID_RANGE_DRAWINGLAYER           (0 << 16)

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
