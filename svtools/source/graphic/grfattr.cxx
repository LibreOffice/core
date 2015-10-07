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


#include <tools/vcompat.hxx>
#include <svtools/grfmgr.hxx>


// - GraphicAttr -


GraphicAttr::GraphicAttr() :
    mfGamma         ( 1.0 ),
    mnMirrFlags     ( BmpMirrorFlags::NONE ),
    mnLeftCrop      ( 0 ),
    mnTopCrop       ( 0 ),
    mnRightCrop     ( 0 ),
    mnBottomCrop    ( 0 ),
    mnRotate10      ( 0 ),
    mnContPercent   ( 0 ),
    mnLumPercent    ( 0 ),
    mnRPercent      ( 0 ),
    mnGPercent      ( 0 ),
    mnBPercent      ( 0 ),
    mbInvert        ( false ),
    mcTransparency  ( 0 ),
    meDrawMode      ( GRAPHICDRAWMODE_STANDARD )
{
}



GraphicAttr::~GraphicAttr()
{
}



bool GraphicAttr::operator==( const GraphicAttr& rAttr ) const
{
    return( ( mfGamma == rAttr.mfGamma ) &&
            ( mnMirrFlags == rAttr.mnMirrFlags ) &&
            ( mnLeftCrop == rAttr.mnLeftCrop ) &&
            ( mnTopCrop == rAttr.mnTopCrop ) &&
            ( mnRightCrop == rAttr.mnRightCrop ) &&
            ( mnBottomCrop == rAttr.mnBottomCrop ) &&
            ( mnRotate10 == rAttr.mnRotate10 ) &&
            ( mnContPercent == rAttr.mnContPercent ) &&
            ( mnLumPercent == rAttr.mnLumPercent ) &&
            ( mnRPercent == rAttr.mnRPercent ) &&
            ( mnGPercent == rAttr.mnGPercent ) &&
            ( mnBPercent == rAttr.mnBPercent ) &&
            ( mbInvert == rAttr.mbInvert ) &&
            ( mcTransparency == rAttr.mcTransparency ) &&
            ( meDrawMode == rAttr.meDrawMode ) );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
