/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: aqua11ywrapper.mm,v $
 *
 * $Revision: 1.2 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "salinst.h"
#include "aqua11ywrapper.h"
#include "aqua11yactionwrapper.h"
#include "aqua11ycomponentwrapper.h"
#include "aqua11ylistener.hxx"
#include "aqua11yselectionwrapper.h"
#include "aqua11ytablewrapper.h"
#include "aqua11ytextwrapper.h"
#include "aqua11yvaluewrapper.h"
#include "aqua11yfactory.h"
#include "aqua11yfocuslistener.hxx"
#include "aqua11yfocustracker.hxx"
#include "aqua11yrolehelper.h"
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

@interface SalFrameWindow : NSWindow
{
}
-(Reference<XAccessibleContext>)accessibleContext;
@end

@implementation AquaA11yWrapper : NSView

#pragma mark -
#pragma mark Init and dealloc

-(id)initWithAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    self = [ super init ];
    if ( self != nil ) {
        [ self setDefaults: rxAccessibleContext ];
    }
    return self;
}

-(void) setDefaults: (Reference < XAccessibleContext >) rxAccessibleContext {
    mDefaultFontsize = 0.0;
    mpDefaultFontname = nil;
    mpReferenceWrapper = new ReferenceWrapper;
    mpReferenceWrapper -> rAccessibleContext = rxAccessibleContext;
    // Querying all supported interfaces
    try {
        // XAccessibleComponent
        mpReferenceWrapper -> rAccessibleComponent = Reference < XAccessibleComponent > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleExtendedComponent
        mpReferenceWrapper -> rAccessibleExtendedComponent = Reference < XAccessibleExtendedComponent > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleSelection
        mpReferenceWrapper -> rAccessibleSelection = Reference< XAccessibleSelection > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleTable
        mpReferenceWrapper -> rAccessibleTable = Reference < XAccessibleTable > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleText
        mpReferenceWrapper -> rAccessibleText = Reference < XAccessibleText > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleEditableText
        mpReferenceWrapper -> rAccessibleEditableText = Reference < XAccessibleEditableText > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleValue
        mpReferenceWrapper -> rAccessibleValue = Reference < XAccessibleValue > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleAction
        mpReferenceWrapper -> rAccessibleAction = Reference < XAccessibleAction > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleTextAttributes
        mpReferenceWrapper -> rAccessibleTextAttributes = Reference < XAccessibleTextAttributes > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleEventBroadcaster
        if ( ! rxAccessibleContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::TRANSIENT ) ) {
            Reference< XAccessibleEventBroadcaster > xBroadcaster(rxAccessibleContext, UNO_QUERY);
            if( xBroadcaster.is() ) {
                /*
                 * We intentionally do not hold a reference to the event listener in the wrapper object,
                 * but let the listener control the life cycle of the wrapper instead ..
                 */
                xBroadcaster->addEventListener( new AquaA11yEventListener( self, rxAccessibleContext -> getAccessibleRole() ) );
            }
        }
    } catch ( const Exception ) {
    }
}

-(void)dealloc {
    if ( mpReferenceWrapper != nil ) {
        delete mpReferenceWrapper;
    }
    if ( mpDefaultFontname != nil ) {
        [ mpDefaultFontname release ];
    }
    [ super dealloc ];
}

#pragma mark -
#pragma mark Utility Section

// generates selectors for attribute name AXAttributeNameHere
// (getter without parameter) attributeNameHereAttribute
// (getter with parameter)    attributeNameHereAttributeForParameter:
// (setter)                   setAttributeNameHereAttributeForElement:to:
-(SEL)selectorForAttribute:(NSString *)attribute asGetter:(MacOSBOOL)asGetter withGetterParameter:(MacOSBOOL)withGetterParameter {
    SEL selector = nil;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    @try {
        // step 1: create method name from attribute name
        NSMutableString * methodName = [ NSMutableString string ];
        if ( ! asGetter ) {
            [ methodName appendString: @"set" ];
        }
        NSString * firstChar = [ attribute substringWithRange: NSMakeRange ( 2, 1 ) ]; // drop leading "AX" and get first char
        if ( asGetter ) {
            [ methodName appendString: [ firstChar lowercaseString ] ]; 
        } else {
            [ methodName appendString: firstChar ]; 
        }
        [ methodName appendString: [ attribute substringFromIndex: 3 ] ]; // append rest of attribute name
        // append rest of method name
        [ methodName appendString: @"Attribute" ];
        if ( ! asGetter ) {
            [ methodName appendString: @"ForElement:to:" ];
        } else if ( asGetter && withGetterParameter ) {
            [ methodName appendString: @"ForParameter:" ];
        }
        // step 2: create selector
        selector = NSSelectorFromString ( methodName );
    } @catch ( id exception ) {
        selector = nil;
    }
    [ pool release ];
    return selector;
}

#pragma mark -
#pragma mark Attribute Value Getters
// ( called via Reflection by accessibilityAttributeValue )

-(id)roleAttribute {
    return [ AquaA11yRoleHelper getNativeRoleFrom: [ self accessibleContext ] ];
}

-(id)subroleAttribute {
    NSString * subRole = [ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ];
    if ( ! [ subRole isEqualToString: @"" ] ) {
        return subRole;
    } else {
        [ subRole release ];
        return [ super accessibilityAttributeValue: NSAccessibilitySubroleAttribute ];
    }
}

-(id)titleAttribute {
    return CreateNSString ( [ self accessibleContext ] -> getAccessibleName() );
}

-(id)descriptionAttribute {
    if ( [ self accessibleExtendedComponent ] != nil ) {
        return [ AquaA11yComponentWrapper descriptionAttributeForElement: self ];
    } else {
        return CreateNSString ( [ self accessibleContext ] -> getAccessibleDescription() );
    }
}

-(id)enabledAttribute {
    if ( [ self accessibleContext ] -> getAccessibleStateSet().is() ) {
        return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::ENABLED ) ];
    } else {
        return nil;
    }
}

-(id)focusedAttribute {
    if ( [ self accessibleContext ] -> getAccessibleStateSet().is() ) {
        return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::FOCUSED ) ];
    } else {
        return nil;
    }
}

-(id)parentAttribute {
    try {
        Reference< XAccessible > xParent( [ self accessibleContext ] -> getAccessibleParent() );
        if ( xParent.is() ) {
            Reference< XAccessibleContext > xContext( xParent -> getAccessibleContext() );
            if ( xContext.is() ) {
                id parent_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: xContext ];
                [ parent_wrapper autorelease ];
                return NSAccessibilityUnignoredAncestor( parent_wrapper );
            }
        }
    } catch (const Exception&) {
    }
    
    OSL_ASSERT( 0 );
    return nil;
}

-(id)childrenAttribute {
    if ( [ self accessibleTable ] != nil ) {
        return [ AquaA11yTableWrapper childrenAttributeForElement: self ];
    } else {
        try {
            NSMutableArray * children = [ [ NSMutableArray alloc ] init ];
            Reference< XAccessibleContext > xContext( [ self accessibleContext ] );
            
            sal_Int32 cnt = xContext -> getAccessibleChildCount();
            for ( sal_Int32 i = 0; i < cnt; i++ ) {
                Reference< XAccessible > xChild( xContext -> getAccessibleChild( i ) );
                if( xChild.is() ) {
                    Reference< XAccessibleContext > xChildContext( xChild -> getAccessibleContext() );
                    // the menubar is already accessible (including Apple- and Application-Menu) through NSApplication => omit it here
                    if ( xChildContext.is() && AccessibleRole::MENU_BAR != xChildContext -> getAccessibleRole() ) {
                        id wrapper = [ AquaA11yFactory wrapperForAccessibleContext: xChildContext ];
                        [ children addObject: wrapper ];
                        [ wrapper release ];
                    }
                }
            }
            
            [ children autorelease ];
            return NSAccessibilityUnignoredChildren( children );
        } catch (const Exception &e) {
            // TODO: Log
            return nil;
        }
    }
}

-(id)windowAttribute {
    // go upstairs until reaching the broken connection
    AquaA11yWrapper * aWrapper = self;
    while ( [ aWrapper accessibleContext ] -> getAccessibleParent().is() ) {
        aWrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ aWrapper accessibleContext ] -> getAccessibleParent() -> getAccessibleContext() ];
        [ aWrapper autorelease ];
    }
    // get associated NSWindow
    NSView * theView = [ aWrapper viewElementForParent ];
    return theView;
}

-(id)topLevelUIElementAttribute {
    return [ self windowAttribute ];
}

-(id)sizeAttribute {
    if ( [ self accessibleComponent ] != nil ) {
        return [ AquaA11yComponentWrapper sizeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)positionAttribute {
    if ( [ self accessibleComponent ] != nil ) {
        return [ AquaA11yComponentWrapper positionAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)helpAttribute {
    return CreateNSString ( [ self accessibleContext ] -> getAccessibleDescription() );
}

-(id)roleDescriptionAttribute {
    return [ AquaA11yRoleHelper getRoleDescriptionFrom: 
            [ AquaA11yRoleHelper getNativeRoleFrom: [ self accessibleContext ] ] 
            with: [ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ] ];
}

-(id)valueAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper valueAttributeForElement: self ];
    } else if ( [ self accessibleValue ] != nil ) {
        return [ AquaA11yValueWrapper valueAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)minValueAttribute {
    if ( [ self accessibleValue ] != nil ) {
        return [ AquaA11yValueWrapper minValueAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)maxValueAttribute {
    if ( [ self accessibleValue ] != nil ) {
        return [ AquaA11yValueWrapper maxValueAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)contentsAttribute {
    return [ self childrenAttribute ];
}

-(id)selectedChildrenAttribute {
    return [ AquaA11ySelectionWrapper selectedChildrenAttributeForElement: self ];
}

-(id)numberOfCharactersAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper numberOfCharactersAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)selectedTextAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper selectedTextAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)selectedTextRangeAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper selectedTextRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)visibleCharacterRangeAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper visibleCharacterRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)tabsAttribute {
    return self; // TODO ???
}

-(id)sharedTextUIElementsAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper sharedTextUIElementsAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)sharedCharacterRangeAttribute {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper sharedCharacterRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)expandedAttribute {
    return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::EXPANDED ) ];
}

-(id)selectedAttribute {
    return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::SELECTED ) ];
}

-(id)stringForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper stringForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)attributedStringForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper attributedStringForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)rangeForIndexAttributeForParameter:(id)index {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper rangeForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rangeForPositionAttributeForParameter:(id)point {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper rangeForPositionAttributeForElement: self forParameter: point ];
    } else {
        return nil;
    }
}

-(id)boundsForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper boundsForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)styleRangeForIndexAttributeForParameter:(id)index {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper styleRangeForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rTFForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] != nil ) {
        return [ AquaA11yTextWrapper rTFForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)orientationAttribute {
    NSString * orientation = nil;
    Reference < XAccessibleStateSet > stateSet = [ self accessibleContext ] -> getAccessibleStateSet();
    if ( stateSet -> contains ( AccessibleStateType::HORIZONTAL ) ) {
        orientation = NSAccessibilityHorizontalOrientationValue;
    } else if ( stateSet -> contains ( AccessibleStateType::VERTICAL ) ) {
        orientation = NSAccessibilityVerticalOrientationValue;
    }
    return orientation;
}

-(id)titleUIElementAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRelationSet().is() ) {
        NSString * title = [ self titleAttribute ];
        id titleElement = nil;
        if ( [ title length ] == 0 ) {
            AccessibleRelation relationLabeledBy = [ self accessibleContext ] -> getAccessibleRelationSet() -> getRelationByType ( AccessibleRelationType::LABELED_BY );
            if ( relationLabeledBy.RelationType == AccessibleRelationType::LABELED_BY && relationLabeledBy.TargetSet.hasElements()  ) {
                Reference < XAccessible > rxAccessible ( relationLabeledBy.TargetSet[0], UNO_QUERY );
                titleElement = [ AquaA11yFactory wrapperForAccessibleContext: rxAccessible -> getAccessibleContext() ];
            }
        }
        if ( title != nil ) {
            [ title release ];
        }
        return titleElement;
    } else {
        return nil;
    }
}

-(id)servesAsTitleForUIElementsAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRelationSet().is() ) {
        id titleForElement = nil;
        AccessibleRelation relationLabelFor = [ self accessibleContext ] -> getAccessibleRelationSet() -> getRelationByType ( AccessibleRelationType::LABEL_FOR );
        if ( relationLabelFor.RelationType == AccessibleRelationType::LABEL_FOR && relationLabelFor.TargetSet.hasElements() ) {
            Reference < XAccessible > rxAccessible ( relationLabelFor.TargetSet[0], UNO_QUERY );
            titleForElement = [ AquaA11yFactory wrapperForAccessibleContext: rxAccessible -> getAccessibleContext() ];
        }
        return titleForElement;
    } else {
        return nil;
    }
}

#pragma mark -
#pragma mark Accessibility Protocol

-(id)accessibilityAttributeValue:(NSString *)attribute {
    id value = nil;
    try {
        SEL methodSelector = [ self selectorForAttribute: attribute asGetter: YES withGetterParameter: NO ];
        if ( [ self respondsToSelector: methodSelector ] ) {
            value = [ self performSelector: methodSelector ];
        }
    } catch ( const Exception & e ) {
        // empty
    }
    return value;
}

-(MacOSBOOL)accessibilityIsIgnored {
    MacOSBOOL ignored = NO;
    sal_Int16 nRole = [ self accessibleContext ] -> getAccessibleRole();
    switch ( nRole ) {
        case AccessibleRole::PANEL:
        case AccessibleRole::FRAME:
        case AccessibleRole::ROOT_PANE:
        case AccessibleRole::SEPARATOR:
        case AccessibleRole::FILLER:
        case AccessibleRole::DIALOG:
            ignored = YES;
            break;
        default: 
            ignored = ! ( [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::VISIBLE ) );
            break;
    }
    return ignored; // TODO: to be completed
}

-(NSArray *)accessibilityAttributeNames {
    NSString * nativeSubrole = nil;
    NSString * title = nil;
    NSMutableArray * attributeNames = nil;
    try {
        // Default Attributes
        attributeNames = [ NSMutableArray arrayWithObjects: 
            NSAccessibilityRoleAttribute, 
            NSAccessibilityDescriptionAttribute, 
            NSAccessibilityParentAttribute, 
            NSAccessibilityWindowAttribute, 
            NSAccessibilityHelpAttribute, 
            NSAccessibilityTopLevelUIElementAttribute, 
            NSAccessibilityRoleDescriptionAttribute, 
            nil ];
        nativeSubrole = (NSString *) [ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ];
        title = (NSString *) [ self titleAttribute ];
        Reference < XAccessibleRelationSet > rxRelationSet = [ self accessibleContext ] -> getAccessibleRelationSet();
        // Special Attributes depending on attribute values
        if ( nativeSubrole != nil && ! [ nativeSubrole isEqualToString: @"" ] ) {
            [ attributeNames addObject: NSAccessibilitySubroleAttribute ];
        }
        if ( [ self accessibleContext ] -> getAccessibleChildCount() > 0 ) {
            [ attributeNames addObject: NSAccessibilityChildrenAttribute ];
        }
        if ( title != nil && ! [ title isEqualToString: @"" ] ) {
            [ attributeNames addObject: NSAccessibilityTitleAttribute ];
        }
        if ( [ title length ] == 0 && rxRelationSet.is() && rxRelationSet -> containsRelation ( AccessibleRelationType::LABELED_BY ) ) {
            [ attributeNames addObject: NSAccessibilityTitleUIElementAttribute ];
        }
        if ( rxRelationSet.is() && rxRelationSet -> containsRelation ( AccessibleRelationType::LABEL_FOR ) ) {
            [ attributeNames addObject: NSAccessibilityServesAsTitleForUIElementsAttribute ];
        }
        // Special Attributes depending on interface
        if ( [ self accessibleText ] != nil ) {
            [ AquaA11yTextWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleComponent ] != nil ) {
            [ AquaA11yComponentWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleSelection ] != nil ) {
            [ AquaA11ySelectionWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleValue ] != nil ) {
            [ AquaA11yValueWrapper addAttributeNamesTo: attributeNames ];
        }
        [ nativeSubrole release ];
        [ title release ];
        return attributeNames;
    } catch ( DisposedException & e ) { // Object is no longer available
        if ( nativeSubrole != nil ) {
            [ nativeSubrole release ];
        }
        if ( title != nil ) {
            [ title release ];
        }
        if ( attributeNames != nil ) {
            [ attributeNames release ];
        }
        return nil;
    }
}

-(MacOSBOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
    MacOSBOOL isSettable = NO;
    if ( [ self accessibleText ] != nil ) {
        isSettable = [ AquaA11yTextWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleComponent ] != nil ) {
        isSettable = [ AquaA11yComponentWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleSelection ] != nil ) {
        isSettable = [ AquaA11ySelectionWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleValue ] != nil ) {
        isSettable = [ AquaA11yValueWrapper isAttributeSettable: attribute forElement: self ];
    }
    return isSettable; // TODO: to be completed
}

-(NSArray *)accessibilityParameterizedAttributeNames {
    NSMutableArray * attributeNames = [ [ NSMutableArray alloc ] init ];
    // Special Attributes depending on interface
    if ( [ self accessibleText ] != nil ) {
        [ AquaA11yTextWrapper addParameterizedAttributeNamesTo: attributeNames ];
    }
    return attributeNames; // TODO: to be completed
}

-(id)accessibilityAttributeValue:(NSString *)attribute forParameter:(id)parameter {
    SEL methodSelector = [ self selectorForAttribute: attribute asGetter: YES withGetterParameter: YES ];
    if ( [ self respondsToSelector: methodSelector ] ) {
        return [ self performSelector: methodSelector withObject: parameter ];
    }
    return nil; // TODO: to be completed
}

-(MacOSBOOL)accessibilitySetOverrideValue:(id)value forAttribute:(NSString *)attribute {
    return NO; // TODO
}

-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute {
    SEL methodSelector = [ self selectorForAttribute: attribute asGetter: NO withGetterParameter: NO ];
    if ( [ AquaA11yComponentWrapper respondsToSelector: methodSelector ] ) {
        [ AquaA11yComponentWrapper performSelector: methodSelector withObject: self withObject: value ];
    }
    if ( [ AquaA11yTextWrapper respondsToSelector: methodSelector ] ) {
        [ AquaA11yTextWrapper performSelector: methodSelector withObject: self withObject: value ];
    }
    if ( [ AquaA11ySelectionWrapper respondsToSelector: methodSelector ] ) {
        [ AquaA11ySelectionWrapper performSelector: methodSelector withObject: self withObject: value ];
    }
    if ( [ AquaA11yValueWrapper respondsToSelector: methodSelector ] ) {
        [ AquaA11yValueWrapper performSelector: methodSelector withObject: self withObject: value ];
    }
}

-(id)accessibilityFocusedUIElement {

    // as this seems to be the first API call on a newly created SalFrameView object,
    // make sure self gets registered in the repository ..
    [ self accessibleContext ]; 

    AquaA11yWrapper * focusedUIElement = AquaA11yFocusListener::get()->getFocusedUIElement();
//    AquaA11yWrapper * ancestor = focusedUIElement;
            
      // Make sure the focused object is a descendant of self
//    do  {
//       if( self == ancestor )
             return focusedUIElement;
        
//       ancestor = [ ancestor accessibilityAttributeValue: NSAccessibilityParentAttribute ];
//    }  while( nil != ancestor );
        
    return self;
}

// TODO: hard-coded like the role descriptions. is there a better way?
-(NSString *)accessibilityActionDescription:(NSString *)action {
    if ( [ action isEqualToString: NSAccessibilityConfirmAction ] ) {
        return @"confirm";
    } else if ( [ action isEqualToString: NSAccessibilityDecrementAction ] ) {
        return @"decrement";
    } else if ( [ action isEqualToString: NSAccessibilityDeleteAction ] ) {
        return @"delete";
    } else if ( [ action isEqualToString: NSAccessibilityIncrementAction ] ) {
        return @"increment";
    } else if ( [ action isEqualToString: NSAccessibilityPickAction ] ) {
        return @"pick";
    } else if ( [ action isEqualToString: NSAccessibilityPressAction ] ) {
        return @"press";
    } else if ( [ action isEqualToString: NSAccessibilityCancelAction ] ) {
        return @"cancel";
    } else if ( [ action isEqualToString: NSAccessibilityRaiseAction ] ) {
        return @"raise";
    } else if ( [ action isEqualToString: NSAccessibilityShowMenuAction ] ) {
        return @"show menu";
    } else {
        return [ NSString string ];
    }
}

-(AquaA11yWrapper *)actionResponder {
    AquaA11yWrapper * wrapper = nil;
    // get some information
    NSString * role = (NSString *) [ self accessibilityAttributeValue: NSAccessibilityRoleAttribute ];
    id enabledAttr = [ self enabledAttribute ];
    MacOSBOOL enabled = [ enabledAttr boolValue ];
    NSView * parent = (NSView *) [ self accessibilityAttributeValue: NSAccessibilityParentAttribute ];
    AquaA11yWrapper * parentAsWrapper = nil;
    if ( [ parent isKindOfClass: [ AquaA11yWrapper class ] ] ) {
        parentAsWrapper = (AquaA11yWrapper *) parent;
    }
    NSString * parentRole = (NSString *) [ parent accessibilityAttributeValue: NSAccessibilityRoleAttribute ];
    // if we are a textarea inside a combobox, then the combobox is the action responder
    if ( enabled 
      && [ role isEqualToString: NSAccessibilityTextAreaRole ] 
      && [ parentRole isEqualToString: NSAccessibilityComboBoxRole ] 
      && parentAsWrapper != nil ) {
        wrapper = parentAsWrapper;
    } else if ( enabled && [ self accessibleAction ] != nil ) {
        wrapper = self ;
    }
    [ parentRole release ];
    [ enabledAttr release ];
    [ role release ];
    return wrapper;
}

-(void)accessibilityPerformAction:(NSString *)action {
    AquaA11yWrapper * actionResponder = [ self actionResponder ];
    if ( actionResponder != nil ) {
        [ AquaA11yActionWrapper doAction: action ofElement: actionResponder ];
    }
}

-(NSArray *)accessibilityActionNames {
    NSArray * actionNames = nil;
    AquaA11yWrapper * actionResponder = [ self actionResponder ];
    if ( actionResponder != nil ) {
        actionNames = [ AquaA11yActionWrapper actionNamesForElement: actionResponder ];
    } else {
        actionNames = [ [ NSArray alloc ] init ];
    }
    return actionNames;
}

#pragma mark -
#pragma mark Hit Test

-(MacOSBOOL)isViewElement:(NSObject *)viewElement hitByPoint:(NSPoint)point {
    MacOSBOOL hit = NO;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    NSValue * position = [ viewElement accessibilityAttributeValue: NSAccessibilityPositionAttribute ];
    NSValue * size = [ viewElement accessibilityAttributeValue: NSAccessibilitySizeAttribute ];
    if ( position != nil && size != nil ) {
        float minX = [ position pointValue ].x;
        float minY = [ position pointValue ].y;
        float maxX = minX + [ size sizeValue ].width;
        float maxY = minY + [ size sizeValue ].height;
        if ( minX < point.x && maxX > point.x && minY < point.y && maxY > point.y ) {
            hit = YES;
        }
    }
    [ pool release ];
    return hit;
}

Reference < XAccessibleContext > hitTestRunner ( Point point, Reference < XAccessibleContext > rxAccessibleContext ) {
    Reference < XAccessibleContext > hitChild;
    Reference < XAccessibleContext > emptyReference;
    try {
        Reference < XAccessibleComponent > rxAccessibleComponent ( rxAccessibleContext, UNO_QUERY );
        if ( rxAccessibleComponent.is() ) {
            Point location = rxAccessibleComponent -> getLocationOnScreen();
            Point hitPoint ( point.X - location.X , point.Y - location.Y); 
            Reference < XAccessible > rxAccessible = rxAccessibleComponent -> getAccessibleAtPoint ( hitPoint );
            if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() ) {
                if ( rxAccessible -> getAccessibleContext() -> getAccessibleChildCount() > 0 ) {
                    hitChild = hitTestRunner ( point, rxAccessible -> getAccessibleContext() );
                    if ( ! hitChild.is() ) {
                        hitChild = rxAccessible -> getAccessibleContext();
                    }
                } else {
                    hitChild = rxAccessible -> getAccessibleContext();
                }
            }
        } 
        if ( !hitChild.is() && rxAccessibleContext -> getAccessibleChildCount() > 0 ) { // special treatment for e.g. comboboxes
            for ( int i = 0; i < rxAccessibleContext -> getAccessibleChildCount(); i++ ) {
                Reference < XAccessible > rxAccessibleChild = rxAccessibleContext -> getAccessibleChild ( i );
                if ( rxAccessibleChild.is() && rxAccessibleChild -> getAccessibleContext().is() && rxAccessibleChild -> getAccessibleContext() -> getAccessibleRole() != AccessibleRole::LIST ) {
                    Reference < XAccessibleContext > myHitChild = hitTestRunner ( point, rxAccessibleChild -> getAccessibleContext() );
                    if ( myHitChild.is() ) {
                        hitChild = myHitChild;
                        break;
                    }
                }
            }
        }
    } catch ( RuntimeException ) {
        return emptyReference;
    }
    return hitChild;
}

-(id)accessibilityHitTest:(NSPoint)point {
    static id wrapper = nil;
    if ( nil != wrapper ) {
        [ wrapper release ];
        wrapper = nil;
    }
    Reference < XAccessibleContext > hitChild;
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
    Point hitPoint ( point.x , screenRect.size.height - point.y ); 
    // check child windows first
    NSWindow * window = (NSWindow *) [ self accessibilityAttributeValue: NSAccessibilityWindowAttribute ];
    NSArray * childWindows = [ window childWindows ];
    if ( [ childWindows count ] > 0 ) {
        NSWindow * element = nil;
        NSEnumerator * enumerator = [ childWindows objectEnumerator ];
        while ( ( element = [ enumerator nextObject ] ) && hitChild == nil ) {
            if ( [ element isKindOfClass: [ SalFrameWindow class ] ] && [ self isViewElement: element hitByPoint: point ] ) {
                // we have a child window that is hit
                Reference < XAccessibleRelationSet > relationSet = [ ( ( SalFrameWindow * ) element ) accessibleContext ] -> getAccessibleRelationSet();
                if ( relationSet.is() && relationSet -> containsRelation ( AccessibleRelationType::SUB_WINDOW_OF )) {
                    // we have a valid relation to the parent element
                    AccessibleRelation relation = relationSet -> getRelationByType ( AccessibleRelationType::SUB_WINDOW_OF );
                    for ( int i = 0; i < relation.TargetSet.getLength() && !hitChild.is(); i++ ) {
                        Reference < XAccessible > rxAccessible ( relation.TargetSet [ i ], UNO_QUERY );
                        if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() ) {
                            // hit test for children of parent
                            hitChild = hitTestRunner ( hitPoint, rxAccessible -> getAccessibleContext() );
                        }
                    }
                }
            }
        }
    }
    // nothing hit yet, so check ourself
    if ( ! hitChild.is() ) {
        if ( mpReferenceWrapper == nil ) {
            [ self setDefaults: [ self accessibleContext ] ];
        }
        hitChild = hitTestRunner ( hitPoint, mpReferenceWrapper -> rAccessibleContext );
    }
    if ( hitChild.is() ) {
        wrapper = [ AquaA11yFactory wrapperForAccessibleContext: hitChild ];
    }
    if ( wrapper != nil ) {
        [ wrapper retain ]; // TODO: retain only when transient ?
    }
    return wrapper;
}

#pragma mark -
#pragma mark Access Methods

-(XAccessibleAction *)accessibleAction {
    return mpReferenceWrapper -> rAccessibleAction.get();
}

-(XAccessibleContext *)accessibleContext {
    return mpReferenceWrapper -> rAccessibleContext.get();
}

-(XAccessibleComponent *)accessibleComponent {
    return mpReferenceWrapper -> rAccessibleComponent.get();
}

-(XAccessibleExtendedComponent *)accessibleExtendedComponent {
    return mpReferenceWrapper -> rAccessibleExtendedComponent.get();
}

-(XAccessibleSelection *)accessibleSelection {
    return mpReferenceWrapper -> rAccessibleSelection.get();
}

-(XAccessibleTable *)accessibleTable {
    return mpReferenceWrapper -> rAccessibleTable.get();
}

-(XAccessibleText *)accessibleText {
    return mpReferenceWrapper -> rAccessibleText.get();
}

-(XAccessibleEditableText *)accessibleEditableText {
    return mpReferenceWrapper -> rAccessibleEditableText.get();
}

-(XAccessibleValue *)accessibleValue {
    return mpReferenceWrapper -> rAccessibleValue.get();
}

-(XAccessibleTextAttributes *)accessibleTextAttributes {
    return mpReferenceWrapper -> rAccessibleTextAttributes.get();
}

-(NSView *)viewElementForParent {
    return self;
}

// These four are for AXTextAreas only. They are needed, because bold and italic
// attributes have to be bound to a font on the Mac. Our UNO-API instead handles
// and reports them independently. When they occur we bundle them to a font with
// this information here to create a according NSFont.
-(void)setDefaultFontname:(NSString *)fontname {
    if ( mpDefaultFontname != nil ) {
        [ mpDefaultFontname release ];
    }
    mpDefaultFontname = fontname;
}

-(NSString *)defaultFontname {
    return mpDefaultFontname;
}

-(void)setDefaultFontsize:(float)fontsize {
    mDefaultFontsize = fontsize;
}

-(float)defaultFontsize {
    return mDefaultFontsize;
}

@end
