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

#ifndef CGM_MAIN_HXX
#define CGM_MAIN_HXX

#include "cgm.hxx"

// ---------------------------------------------------------------

//      CGM_LEVEL1                      0x0001  // VERSION 1 METAFILE
//      CGM_LEVEL2                      0x0002
//      CGM_LEVEL3                      0x0003
//      CGM_GDSF_ONLY                   0x00F0
//      CGM_GDSF_ONLY                   0x00F0
#define CGM_UNKNOWN_LEVEL               0x00F1
#define CGM_UNKNOWN_COMMAND             0x00F2
#define CGM_DESCRIPTION                 0x00F3

#define CGM_EXTENDED_PRIMITIVES_SET     0x0100  // INDICATES PRIMITIVES WHICH ARE AVAILABLE IN
                                                // LEVEL 1 BUT ARE NOT DEFINED IN ISO 7942(GKS)
#define CGM_DRAWING_PLUS_CONTROL_SET    0x0200  // INDICATES THAT THIS IS AN ADDITIONAL LEVEL 1
                                                // ELEMENT

#define BMCOL( _col ) BitmapColor( (sal_Int8)(_col >> 16 ), (sal_Int8)( _col >> 8 ), (sal_Int8)_col )

#include <vcl/salbtype.hxx>
#include <tools/stream.hxx>
#include "bundles.hxx"
#include "bitmap.hxx"
#include "elements.hxx"


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
