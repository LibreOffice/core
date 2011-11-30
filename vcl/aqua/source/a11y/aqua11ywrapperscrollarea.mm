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

#include "aqua/salinst.h"

#include "aqua11ywrapperscrollarea.h"
#include "aqua11ywrapperscrollbar.h"
#include "aqua11yrolehelper.h"

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
            AquaA11yWrapper * element = ( AquaA11yWrapper * ) child;
            if ( [ element isKindOfClass: [ AquaA11yWrapperScrollBar class ] ] ) { 
                AquaA11yWrapperScrollBar * scrollBar = (AquaA11yWrapperScrollBar *) element;
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
