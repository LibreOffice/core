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


#include <vcl/svapp.hxx>
#include <osx/salinst.h>

#include "a11ywrapperscrollarea.h"
#include "a11ywrapperscrollbar.h"
#include "a11yrolehelper.h"

// Wrapper for AXScrollArea role

@implementation AquaA11yWrapperScrollArea : AquaA11yWrapper

-(id)scrollBarWithOrientation:(NSString *)orientation {
    AquaA11yWrapper * theScrollBar = nil;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    NSArray * elementChildren = [ self accessibilityAttributeValue: NSAccessibilityChildrenAttribute ];
    if ( [ elementChildren count ] > 0 ) {
        NSEnumerator * enumerator = [ elementChildren objectEnumerator ];
        id child;
        while ( ( child = [ enumerator nextObject ] ) ) {
            AquaA11yWrapper * element = static_cast<AquaA11yWrapper *>(child);
            if ( [ element isKindOfClass: [ AquaA11yWrapperScrollBar class ] ] ) { 
                AquaA11yWrapperScrollBar * scrollBar = static_cast<AquaA11yWrapperScrollBar *>(element);
                if ( [ [ scrollBar orientationAttribute ] isEqualToString: orientation ] ) {
                    theScrollBar = scrollBar;
                    break;
                }
            }
        }
    }
    [ pool release ];
    return theScrollBar;
}

-(id)verticalScrollBarAttribute {
    return [ self scrollBarWithOrientation: NSAccessibilityVerticalOrientationValue ];
}

-(id)horizontalScrollBarAttribute {
    return [ self scrollBarWithOrientation: NSAccessibilityHorizontalOrientationValue ];
}

-(NSArray *)accessibilityAttributeNames {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return [ NSArray array ];

    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
    // Special Attributes and removing unwanted attributes depending on role
    [ attributeNames removeObject: NSAccessibilityEnabledAttribute ];
    [ attributeNames addObjectsFromArray: [ NSArray arrayWithObjects:
            NSAccessibilityContentsAttribute, 
            NSAccessibilityVerticalScrollBarAttribute, 
            NSAccessibilityHorizontalScrollBarAttribute, 
            nil ]
    ];
    return attributeNames;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
