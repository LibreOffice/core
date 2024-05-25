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
#include <osx/a11yfactory.h>

#include "a11yselectionwrapper.h"
#include "a11ytablewrapper.h"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

@implementation AquaA11ySelectionWrapper : NSObject

+(id)selectedChildrenAttributeForElement:(AquaA11yWrapper *)wrapper
{
    Reference< XAccessibleSelection > xAccessibleSelection = [ wrapper accessibleSelection ];
    if( xAccessibleSelection.is() )
    {
        try {
            sal_Int64 n = xAccessibleSelection -> getSelectedAccessibleChildCount();

            // Related tdf#158914: implicitly call autorelease selector
            // Callers expect this selector to return an autoreleased object.
            NSMutableArray * children = [ NSMutableArray arrayWithCapacity: n ];

            // Fix hanging when selecting a column or row in Calc
            // When a Calc column is selected, the child count will be
            // at least a million. Constructing that many C++ Calc objects
            // takes several minutes even on a fast Silicon Mac so apply
            // the maximum table cell limit here.
            if ( n < 0 )
                n = 0;
            else if ( n > MAXIMUM_ACCESSIBLE_TABLE_CELLS )
                n = MAXIMUM_ACCESSIBLE_TABLE_CELLS;

            for ( sal_Int64 i=0 ; i < n ; ++i ) {
                // Related tdf#158914: explicitly call release selector
                // [ AquaA11yFactory wrapperForAccessible: ] is not a getter.
                // It expects the caller to release the returned object.
                id child_wrapper = [ AquaA11yFactory wrapperForAccessible: xAccessibleSelection -> getSelectedAccessibleChild( i ) ];
                [ children addObject: child_wrapper ];
                [ child_wrapper release ];
            }

            return children;

        } catch ( Exception&)
        {
        }
    }

    return nil;
}


+(void)addAttributeNamesTo:(NSMutableArray *)attributeNames
{
    [ attributeNames addObject: NSAccessibilitySelectedChildrenAttribute ];
}

+(BOOL)isAttributeSettable:(NSString *)attribute forElement:(AquaA11yWrapper *)wrapper
{
    (void)wrapper;
    if ( [ attribute isEqualToString: NSAccessibilitySelectedChildrenAttribute ] )
    {
        return YES;
    }
    else
    {
        return NO;
    }
}

+(void)setSelectedChildrenAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value
{
    Reference< XAccessibleSelection > xAccessibleSelection = [ wrapper accessibleSelection ];
    try {
        xAccessibleSelection -> clearAccessibleSelection();

        unsigned c = [ value count ];
        for ( unsigned i = 0 ; i < c ; ++i ) {
            xAccessibleSelection -> selectAccessibleChild( [ [ value objectAtIndex: i ] accessibleContext ] -> getAccessibleIndexInParent() );
        }
    } catch ( Exception&) {
    }
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
