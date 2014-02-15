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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_SETMAPVIRTDEV_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SETMAPVIRTDEV_HXX

class OutputDevice;
class VirtualDevice;
class MapMode;
class Point;

/** method to set mapping/pixel offset for virtual output device

    OD 12.11.2002 #96272# - method implements two solutions for the mapping of
    the virtual output device:
    The old solution set the origin of the mapping mode, which will be used in
    the virtual output device. This causes several paint errors, because of the
    different roundings in the virtual output device and the original output device.
    The new solution avoids the rounding differences between virtual and original
    output device by setting a pixel offset at the virtual output device.
    A local boolean controls, which solution is used, in order to switch in
    escalation back to old solution.

    @param _pOrgOutDev
    input parameter - constant instance of the original output device, for which
    the virtual output device is created.

    @param _pVirDev
    input/output parameter - instance of the virtual output device.

    @param _pMapMode
    input/output parameter - instance of the mapping mode, which will be set
    at the virtual output device.

    @param _rNewOrigin
    input parameter - constant instance of the origin, which will be used in
    the virtual output device
*/
void SetMappingForVirtDev(  const Point&    _rNewOrigin,
                            MapMode*        _pMapMode,
                            const OutputDevice* _pOrgOutDev,
                            VirtualDevice*  _pVirDev );


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
