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


#include <osx/osxvcltypes.h>

#include "a11yutil.h"

using namespace ::com::sun::star::awt;

@implementation AquaA11yUtil : NSObject

// TODO: should be merged with AquaSalFrame::VCLToCocoa... to a general helper method
+(NSValue *)vclPointToNSPoint:(Point)vclPoint {
    // VCL coordinates are in upper-left-notation, Cocoa likes it the Cartesian way (lower-left)
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
    NSPoint nsPoint = NSMakePoint ( static_cast<float>(vclPoint.X), static_cast<float>( screenRect.size.height - vclPoint.Y ) );
    return [ NSValue valueWithPoint: nsPoint ];
}

// TODO: should be merged with AquaSalFrame::VCLToCocoa... to a general helper method
+(Point)nsPointToVclPoint:(NSValue *)nsPoint {
    // VCL coordinates are in upper-left-notation, Cocoa likes it the Cartesian way (lower-left)
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
    return Point ( static_cast<long>([ nsPoint pointValue ].x), static_cast<long>(screenRect.size.height - [ nsPoint pointValue ].y) );
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
