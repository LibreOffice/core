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
#include "aqua11ywrapperradiobutton.h"
#include "aqua11ytextwrapper.h"
#include "aqua11yvaluewrapper.h"

// Wrapper for AXRadioButton role

@implementation AquaA11yWrapperRadioButton : AquaA11yWrapper

-(id)valueAttribute {
    if ( [ self accessibleValue ] != nil ) {
        return [ AquaA11yValueWrapper valueAttributeForElement: self ];
    }
    return [ NSNumber numberWithInt: 0 ];
}

-(BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
    if ( [ attribute isEqualToString: NSAccessibilityValueAttribute ] ) {
        return NO;
    }
    return [ super accessibilityIsAttributeSettable: attribute ];
}

-(NSArray *)accessibilityAttributeNames {
    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
    // Special Attributes and removing unwanted attributes depending on role
    [ attributeNames removeObjectsInArray: [ AquaA11yTextWrapper specialAttributeNames ] ];
    [ attributeNames addObject: NSAccessibilityMinValueAttribute ];
    [ attributeNames addObject: NSAccessibilityMaxValueAttribute ];
    [ attributeNames addObject: NSAccessibilityValueAttribute ];
    return attributeNames;
}

@end
