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

#include "aqua11ywrapperbutton.h"
#include "aqua11ytextwrapper.h"

// Wrapper for AXButton role

@implementation AquaA11yWrapperButton : AquaA11yWrapper

-(id)valueAttribute {
    return [ NSString string ]; // we propagate AXTitle, that's enough
}

-(id)descriptionAttribute {
    return [ NSString string ]; // we propagate AXTitle, that's enough
}

-(NSArray *)accessibilityAttributeNames {
    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
    // Special Attributes and removing unwanted attributes depending on role
    if ( [ attributeNames containsObject: NSAccessibilityTitleAttribute ] ) {
        [ attributeNames removeObject: NSAccessibilityDescriptionAttribute ];
    } else {
        [ attributeNames addObject: NSAccessibilityTitleAttribute ];
    }
    // Remove text-specific attributes
    [ attributeNames removeObjectsInArray: [ AquaA11yTextWrapper specialAttributeNames ] ];
    return attributeNames;
}

@end
