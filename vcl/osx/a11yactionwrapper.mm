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


#include <osx/salinst.h>
#include <quartz/utils.h>

#include "a11yactionwrapper.h"

// Wrapper for XAccessibleAction

@implementation AquaA11yActionWrapper : NSObject

+(NSString *)nativeActionNameFor:(NSString *)actionName {
    // TODO: Optimize ?
    //       Use NSAccessibilityActionDescription
    if ( [ actionName isEqualToString: @"press" ] ) {
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
    if ( [ wrapper accessibleAction ] ) {
        for ( int cnt = 0; cnt < [ wrapper accessibleAction ] -> getAccessibleActionCount(); cnt++ ) {
            // Related tdf#158914: explicitly call autorelease selector
            // CreateNSString() is not a getter. It expects the caller to
            // release the returned string.
            [ actionNames addObject: [ AquaA11yActionWrapper nativeActionNameFor: [ CreateNSString ( [ wrapper accessibleAction ] -> getAccessibleActionDescription ( cnt ) ) autorelease ] ] ];
        }
    }
    return [actionNames autorelease];
}

+(void)doAction:(NSString *)action ofElement:(AquaA11yWrapper *)wrapper {
    if ( [ wrapper accessibleAction ] ) {
        for ( int cnt = 0; cnt < [ wrapper accessibleAction ] -> getAccessibleActionCount(); cnt++ ) {
            // Related tdf#158914: explicitly call autorelease selector
            // CreateNSString() is not a getter. It expects the caller to
            // release the returned string.
            if ( [ action isEqualToString: [ AquaA11yActionWrapper nativeActionNameFor: [ CreateNSString ( [ wrapper accessibleAction ] -> getAccessibleActionDescription ( cnt ) ) autorelease ] ] ] ) {
                [ wrapper accessibleAction ] -> doAccessibleAction ( cnt );
                break;
            }
        }
    }
}

+(NSAccessibilityActionName)actionNameForSelector:(SEL)aSelector
{
    NSAccessibilityActionName pRet = nil;

    if ( aSelector == @selector(accessibilityPerformDecrement) ) {
        pRet = NSAccessibilityDecrementAction;
    } else if ( aSelector == @selector(accessibilityPerformIncrement) ) {
        pRet = NSAccessibilityIncrementAction;
    } else if ( aSelector == @selector(accessibilityPerformPick) ) {
        pRet = NSAccessibilityPickAction;
    } else if ( aSelector == @selector(accessibilityPerformPress) ) {
        pRet = NSAccessibilityPressAction;
    } else if ( aSelector == @selector(accessibilityPerformShowMenu) ) {
        pRet = NSAccessibilityShowMenuAction;
    }

    return pRet;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
