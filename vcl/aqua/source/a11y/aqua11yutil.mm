/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



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
