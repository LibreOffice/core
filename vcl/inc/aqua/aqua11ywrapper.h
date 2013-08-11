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

#ifndef _SV_AQUA11WRAPPER_H
#define _SV_AQUA11WRAPPER_H

#include "aquavcltypes.h"
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleExtendedComponent.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleText.hpp>
#include <com/sun/star/accessibility/XAccessibleTextAttributes.hpp>
#include <com/sun/star/accessibility/XAccessibleEditableText.hpp>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include <com/sun/star/accessibility/XAccessibleMultiLineText.hpp>

// rAccessibleXYZ as a field in an Objective-C-Class would not call Con-/Destructor, so use a struct instead
struct ReferenceWrapper
{
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleAction > rAccessibleAction;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext > rAccessibleContext;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleComponent > rAccessibleComponent;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleExtendedComponent > rAccessibleExtendedComponent;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleSelection > rAccessibleSelection;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleTable > rAccessibleTable;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleText > rAccessibleText;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleEditableText > rAccessibleEditableText;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleValue > rAccessibleValue;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleTextAttributes > rAccessibleTextAttributes;
    ::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleMultiLineText > rAccessibleMultiLineText;
};

@interface AquaA11yWrapper : NSView
{
    ReferenceWrapper * mpReferenceWrapper;
    NSString * mpDefaultFontname;
    float mDefaultFontsize;
    int mDefaultFonttraits;
    BOOL mActsAsRadioGroup;
    BOOL mIsTableCell;
}
// NSAccessibility Protocol
-(id)accessibilityAttributeValue:(NSString *)attribute;
-(BOOL)accessibilityIsIgnored;
-(NSArray *)accessibilityAttributeNames;
-(BOOL)accessibilityIsAttributeSettable:(NSString *)attribute;
-(NSArray *)accessibilityParameterizedAttributeNames;
-(BOOL)accessibilitySetOverrideValue:(id)value forAttribute:(NSString *)attribute;
-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute;
-(id)accessibilityAttributeValue:(NSString *)attribute forParameter:(id)parameter;
-(id)accessibilityFocusedUIElement;
-(NSString *)accessibilityActionDescription:(NSString *)action;
-(void)accessibilityPerformAction:(NSString *)action;
-(NSArray *)accessibilityActionNames;
-(id)accessibilityHitTest:(NSPoint)point;
// Attribute values
-(id)parentAttribute;
-(id)valueAttribute;
-(id)titleAttribute;
-(id)helpAttribute;
-(id)numberOfCharactersAttribute;
-(id)selectedTextAttribute;
-(id)selectedTextRangeAttribute;
-(id)visibleCharacterRangeAttribute;
-(id)childrenAttribute;
-(id)orientationAttribute;
-(id)windowAttribute;
// Wrapper-specific
-(void)setActsAsRadioGroup:(BOOL)actsAsRadioGroup;
-(BOOL)actsAsRadioGroup;
-(NSView *)viewElementForParent;
-(id)initWithAccessibleContext: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) anAccessibleContext;
-(void) setDefaults: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext;
-(void) dealloc;
-(void)setDefaultFontname:(NSString *)fontname;
-(NSString *)defaultFontname;
-(void)setDefaultFontsize:(float)fontsize;
-(float)defaultFontsize;
-(void)setDefaultFonttraits:(int)fonttraits;
-(int)defaultFonttraits;
+(void)setPopupMenuOpen:(BOOL)popupMenuOpen;
-(::com::sun::star::accessibility::XAccessibleAction *)accessibleAction;
-(::com::sun::star::accessibility::XAccessibleContext *)accessibleContext;
-(::com::sun::star::accessibility::XAccessibleComponent *)accessibleComponent;
-(::com::sun::star::accessibility::XAccessibleExtendedComponent *)accessibleExtendedComponent;
-(::com::sun::star::accessibility::XAccessibleSelection *)accessibleSelection;
-(::com::sun::star::accessibility::XAccessibleTable *)accessibleTable;
-(::com::sun::star::accessibility::XAccessibleText *)accessibleText;
-(::com::sun::star::accessibility::XAccessibleEditableText *)accessibleEditableText;
-(::com::sun::star::accessibility::XAccessibleValue *)accessibleValue;
-(::com::sun::star::accessibility::XAccessibleTextAttributes *)accessibleTextAttributes;
-(::com::sun::star::accessibility::XAccessibleMultiLineText *)accessibleMultiLineText;
@end

#endif // _SV_AQUA11WRAPPER_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
