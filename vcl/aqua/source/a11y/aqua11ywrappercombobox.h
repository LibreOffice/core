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



#ifndef _SV_AQUA11WRAPPERCOMBOBOX_H
#define _SV_AQUA11WRAPPERCOMBOBOX_H

#include "aqua/aqua11ywrapper.h"
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

@interface AquaA11yWrapperComboBox : AquaA11yWrapper
{
    AquaA11yWrapper * textArea;
}
-(id)initWithAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) anAccessibleContext;
-(id)valueAttribute;
-(id)numberOfCharactersAttribute;
-(id)selectedTextAttribute;
-(id)selectedTextRangeAttribute;
-(id)visibleCharacterRangeAttribute;
// Accessibility Protocol
-(BOOL)accessibilityIsAttributeSettable:(NSString *)attribute;
-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute;
-(NSArray *)accessibilityAttributeNames;
@end

#endif // _SV_AQUA11WRAPPERCOMBOBOX_H
