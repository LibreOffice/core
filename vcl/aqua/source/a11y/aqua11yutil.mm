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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "aqua/aquavcltypes.h"

#include "aqua11yutil.h"

using namespace ::com::sun::star::awt;

@implementation AquaA11yUtil : NSObject

// TODO: should be merged with AquaSalFrame::VCLToCocoa... to a general helper method
+(NSValue *)vclPointToNSPoint:(Point)vclPoint {
    // VCL coordinates are in upper-left-notation, Cocoa likes it the Cartesian way (lower-left)
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
    NSPoint nsPoint = NSMakePoint ( (float) vclPoint.X, (float) ( screenRect.size.height - vclPoint.Y ) );
    return [ NSValue valueWithPoint: nsPoint ];
}

// TODO: should be merged with AquaSalFrame::VCLToCocoa... to a general helper method
+(Point)nsPointToVclPoint:(NSValue *)nsPoint {
    // VCL coordinates are in upper-left-notation, Cocoa likes it the Cartesian way (lower-left)
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
    return Point ( static_cast<long>([ nsPoint pointValue ].x), static_cast<long>(screenRect.size.height - [ nsPoint pointValue ].y) );
}

@end
