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
#ifndef _SETMAPVIRTDEV_HXX
#define _SETMAPVIRTDEV_HXX
class OutputDevice; 
class VirtualDevice; 
class MapMode; 
class Point; 
namespace binfilter {



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

    @author OD

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


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
