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


#include "osx/salinst.h"

#include "a11ywrapperbutton.h"
#include "a11ytextwrapper.h"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
