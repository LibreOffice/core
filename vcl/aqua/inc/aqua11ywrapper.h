/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
