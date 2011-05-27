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

#include "aqua/salinst.h"

#include "aqua11yactionwrapper.h"

// Wrapper for XAccessibleAction

@implementation AquaA11yActionWrapper : NSObject

+(NSString *)nativeActionNameFor:(NSString *)actionName {
    // TODO: Optimize ?
    //       Use NSAccessibilityActionDescription
    if ( [ actionName isEqualToString: @"click" ] ) {
        return NSAccessibilityPressAction;
    } else if ( [ actionName isEqualToString: @"togglePopup" ] ) {
        return NSAccessibilityShowMenuAction;
    } else if ( [ actionName isEqualToString: @"select" ] ) {
        return NSAccessibilityPickAction;
    } else if ( [ actionName isEqualToString: @"incrementLine" ] ) {
        return NSAccessibilityIncrementAction;
    } else if ( [ actionName isEqualToString: @"decrementLine" ] ) {
        return NSAccessibilityDecrementAction;
    } else if ( [ actionName isEqualToString: @"incrementBlock" ] ) {
        return NSAccessibilityIncrementAction; // TODO ?
    } else if ( [ actionName isEqualToString: @"decrementBlock" ] ) {
        return NSAccessibilityDecrementAction; // TODO ?
    } else if ( [ actionName isEqualToString: @"Browse" ] ) {
        return NSAccessibilityPressAction; // TODO ?
    } else {
        return [ NSString string ];
    }
}

+(NSArray *)actionNamesForElement:(AquaA11yWrapper *)wrapper {
    NSMutableArray * actionNames = [ [ NSMutableArray alloc ] init ];
    if ( [ wrapper accessibleAction ] != nil ) {
        for ( int cnt = 0; cnt < [ wrapper accessibleAction ] -> getAccessibleActionCount(); cnt++ ) {
            [ actionNames addObject: [ AquaA11yActionWrapper nativeActionNameFor: CreateNSString ( [ wrapper accessibleAction ] -> getAccessibleActionDescription ( cnt ) ) ] ];
        }
    }
    return actionNames;
}

+(void)doAction:(NSString *)action ofElement:(AquaA11yWrapper *)wrapper {
    if ( [ wrapper accessibleAction ] != nil ) {
        for ( int cnt = 0; cnt < [ wrapper accessibleAction ] -> getAccessibleActionCount(); cnt++ ) {
            if ( [ action isEqualToString: [ AquaA11yActionWrapper nativeActionNameFor: CreateNSString ( [ wrapper accessibleAction ] -> getAccessibleActionDescription ( cnt ) ) ] ] ) {
                [ wrapper accessibleAction ] -> doAccessibleAction ( cnt );
                break;
            }
        }
    }
}

@end
