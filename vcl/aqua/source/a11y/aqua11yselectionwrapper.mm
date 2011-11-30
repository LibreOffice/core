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
#include "aqua/aqua11yfactory.h"

#include "aqua11yselectionwrapper.h"

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

@implementation AquaA11ySelectionWrapper : NSObject

+(id)selectedChildrenAttributeForElement:(AquaA11yWrapper *)wrapper
{
    Reference< XAccessibleSelection > xAccessibleSelection = [ wrapper accessibleSelection ];
    if( xAccessibleSelection.is() )
    {
        NSMutableArray * children = [ [ NSMutableArray alloc ] init ];
        try {
            sal_Int32 n = xAccessibleSelection -> getSelectedAccessibleChildCount();
            for ( sal_Int32 i=0 ; i < n ; ++i ) {
                [ children addObject: [ AquaA11yFactory wrapperForAccessible: xAccessibleSelection -> getSelectedAccessibleChild( i ) ] ];
            }
            
            return children;
            
        } catch ( Exception& e)
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
    } catch ( Exception& e) {
    }            
}

@end
