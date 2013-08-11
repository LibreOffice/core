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


#include "aqua/salinst.h"
#include "aqua/saldata.hxx"

#include "aqua/aqua11ywrapper.h"
#include "aqua/aqua11ylistener.hxx"
#include "aqua/aqua11yfactory.h"
#include "aqua/aqua11yfocustracker.hxx"

#include "quartz/utils.h"

#include "aqua11yfocuslistener.hxx"
#include "aqua11yactionwrapper.h"
#include "aqua11ycomponentwrapper.h"
#include "aqua11yselectionwrapper.h"
#include "aqua11ytablewrapper.h"
#include "aqua11ytextwrapper.h"
#include "aqua11yvaluewrapper.h"
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

static BOOL isPopupMenuOpen = NO;

static std::ostream &operator<<(std::ostream &s, NSObject *obj) {
    return s << [[obj description] UTF8String];
}

static std::ostream &operator<<(std::ostream &s, NSPoint point) {
    return s << NSStringFromPoint(point);
}


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
    mDefaultFonttraits = 0;
    mpDefaultFontname = nil;
    mpReferenceWrapper = new ReferenceWrapper;
    mActsAsRadioGroup = NO;
    mpReferenceWrapper -> rAccessibleContext = rxAccessibleContext;
    mIsTableCell = NO;
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
        // XAccessibleMultiLineText
        mpReferenceWrapper -> rAccessibleMultiLineText = Reference < XAccessibleMultiLineText > ( rxAccessibleContext, UNO_QUERY );
        // XAccessibleEventBroadcaster
        #if 0
        /* #i102033# NSAccessibility does not seemt to know an equivalent for transient children.
           That means we need to cache this, else e.g. tree list boxes are not accessible (moreover
           it crashes by notifying dead objects - which would seemt o be another bug)

           FIXME:
           Unfortunately this can increase memory consumption drastically until the non transient parent
           is destroyed an finally all the transients are released.
        */
        if ( ! rxAccessibleContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::TRANSIENT ) )
        #endif
        {
            Reference< XAccessibleEventBroadcaster > xBroadcaster(rxAccessibleContext, UNO_QUERY);
            if( xBroadcaster.is() ) {
                /*
                 * We intentionally do not hold a reference to the event listener in the wrapper object,
                 * but let the listener control the life cycle of the wrapper instead ..
                 */
                xBroadcaster->addAccessibleEventListener( new AquaA11yEventListener( self, rxAccessibleContext -> getAccessibleRole() ) );
            }
        }
        // TABLE_CELL
        if ( rxAccessibleContext -> getAccessibleRole() == AccessibleRole::TABLE_CELL ) {
            mIsTableCell = YES;
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
-(SEL)selectorForAttribute:(NSString *)attribute asGetter:(BOOL)asGetter withGetterParameter:(BOOL)withGetterParameter {
    SEL selector = nil;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    @try {
        // step 1: create method name from attribute name
        NSMutableString * methodName = [ NSMutableString string ];
        if ( ! asGetter ) {
            [ methodName appendString: @"set" ];
        }
        NSRange aRange = { 2, 1 };
        NSString * firstChar = [ attribute substringWithRange: aRange ]; // drop leading "AX" and get first char
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

-(Reference < XAccessible >)getFirstRadioButtonInGroup {
    Reference < XAccessibleRelationSet > rxAccessibleRelationSet = [ self accessibleContext ] -> getAccessibleRelationSet();
    if( rxAccessibleRelationSet.is() )
    {
        AccessibleRelation relationMemberOf = rxAccessibleRelationSet -> getRelationByType ( AccessibleRelationType::MEMBER_OF );
        if ( relationMemberOf.RelationType == AccessibleRelationType::MEMBER_OF && relationMemberOf.TargetSet.hasElements() )
            return Reference < XAccessible > ( relationMemberOf.TargetSet[0], UNO_QUERY );
    }
    return Reference < XAccessible > ();
}

-(BOOL)isFirstRadioButtonInGroup {
    Reference < XAccessible > rFirstMateAccessible = [ self getFirstRadioButtonInGroup ];
    if ( rFirstMateAccessible.is() && rFirstMateAccessible -> getAccessibleContext().get() == [ self accessibleContext ] ) {
        return YES;
    }
    return NO;
}

#pragma mark -
#pragma mark Attribute Value Getters
// ( called via Reflection by accessibilityAttributeValue )

/*
    Radiobutton grouping is done differently in NSAccessibility and the UNO-API. In UNO related radio buttons share an entry in their
    RelationSet. In NSAccessibility the relationship is axpressed through the hierarchy. A AXRadioGroup contains two or more AXRadioButton
    objects. Since this group is not available in the UNO hierarchy, an extra wrapper is used for it. This wrapper shares almost all
    attributes with the first radio button of the group, except for the role, subrole, role description, parent and children attributes.
    So in this five methods there is a special treatment for radio buttons and groups.
*/

-(id)roleAttribute {
    if ( mActsAsRadioGroup ) {
        return NSAccessibilityRadioGroupRole;
    }
    else {
        return [ AquaA11yRoleHelper getNativeRoleFrom: [ self accessibleContext ] ];
    }
}

-(id)subroleAttribute {
    if ( mActsAsRadioGroup ) {
        return @"";
    } else {
        NSString * subRole = [ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ];
        if ( ! [ subRole isEqualToString: @"" ] ) {
            return subRole;
        } else {
            [ subRole release ];
            return [ super accessibilityAttributeValue: NSAccessibilitySubroleAttribute ];
        }
    }
}

-(id)titleAttribute {
    return CreateNSString ( [ self accessibleContext ] -> getAccessibleName() );
}

-(id)descriptionAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::COMBO_BOX ) {
        return [ self titleAttribute ];
    } else if ( [ self accessibleExtendedComponent ] != nil ) {
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
    if ( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::COMBO_BOX ) {
        id isFocused = nil;
        Reference < XAccessible > rxParent = [ self accessibleContext ] -> getAccessibleParent();
        if ( rxParent.is() ) {
            Reference < XAccessibleContext > rxContext = rxParent -> getAccessibleContext();
            if ( rxContext.is() && rxContext -> getAccessibleStateSet().is() ) {
                isFocused = [ NSNumber numberWithBool: rxContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::FOCUSED ) ];
            }
        }
        return isFocused;
    } else if ( [ self accessibleContext ] -> getAccessibleStateSet().is() ) {
        return [ NSNumber numberWithBool: [ self accessibleContext ] -> getAccessibleStateSet() -> contains ( AccessibleStateType::FOCUSED ) ];
    } else {
        return nil;
    }
}

-(id)parentAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::RADIO_BUTTON && ! mActsAsRadioGroup ) {
        Reference < XAccessible > rxAccessible = [ self getFirstRadioButtonInGroup ];
        if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() ) {
            Reference < XAccessibleContext > rxAccessibleContext = rxAccessible -> getAccessibleContext();
            id parent_wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rxAccessibleContext createIfNotExists: YES asRadioGroup: YES ];
            [ parent_wrapper autorelease ];
            return NSAccessibilityUnignoredAncestor( parent_wrapper );
        }
        return nil;
    }
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
    if ( mActsAsRadioGroup ) {
        NSMutableArray * children = [ [ NSMutableArray alloc ] init ];
        Reference < XAccessibleRelationSet > rxAccessibleRelationSet = [ self accessibleContext ] -> getAccessibleRelationSet();
        AccessibleRelation relationMemberOf = rxAccessibleRelationSet -> getRelationByType ( AccessibleRelationType::MEMBER_OF );
        if ( relationMemberOf.RelationType == AccessibleRelationType::MEMBER_OF && relationMemberOf.TargetSet.hasElements() ) {
            for ( int index = 0; index < relationMemberOf.TargetSet.getLength(); index++ ) {
                Reference < XAccessible > rMateAccessible = Reference < XAccessible > ( relationMemberOf.TargetSet[index], UNO_QUERY );
                if ( rMateAccessible.is() ) {
                    Reference< XAccessibleContext > rMateAccessibleContext( rMateAccessible -> getAccessibleContext() );
                    if ( rMateAccessibleContext.is() ) {
                        id wrapper = [ AquaA11yFactory wrapperForAccessibleContext: rMateAccessibleContext ];
                        [ children addObject: wrapper ];
                        [ wrapper release ];
                    }
                }
            }
        }
        return children;
    } else if ( [ self accessibleTable ] != nil )
    {
        AquaA11yTableWrapper* pTable = [self isKindOfClass: [AquaA11yTableWrapper class]] ? (AquaA11yTableWrapper*)self : nil;
        return [ AquaA11yTableWrapper childrenAttributeForElement: pTable ];
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

            // if not already acting as RadioGroup now is the time to replace RadioButtons with RadioGroups and remove RadioButtons
            if ( ! mActsAsRadioGroup ) {
                NSEnumerator * enumerator = [ children objectEnumerator ];
                AquaA11yWrapper * element;
                while ( ( element = ( (AquaA11yWrapper *) [ enumerator nextObject ] ) ) ) {
                    if ( [ element accessibleContext ] -> getAccessibleRole() == AccessibleRole::RADIO_BUTTON ) {
                        if ( [ element isFirstRadioButtonInGroup ] ) {
                            id wrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ element accessibleContext ] createIfNotExists: YES asRadioGroup: YES ];
                            [ children replaceObjectAtIndex: [ children indexOfObjectIdenticalTo: element ] withObject: wrapper ];
                        }
                        [ children removeObject: element ];
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
    int loops = 0;
    while ( [ aWrapper accessibleContext ] -> getAccessibleParent().is() ) {
        AquaA11yWrapper *aTentativeParentWrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ aWrapper accessibleContext ] -> getAccessibleParent() -> getAccessibleContext() ];
        // Quick-and-dirty fix for infinite loop after fixing crash in
        // fdo#47275
        if ( aTentativeParentWrapper == aWrapper )
            break;
        // Even dirtier fix for infinite loop in fdo#55156
        if ( loops++ == 100 )
            break;
        aWrapper = aTentativeParentWrapper;
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
    if ( mActsAsRadioGroup ) {
        return [ AquaA11yRoleHelper getRoleDescriptionFrom: NSAccessibilityRadioGroupRole with: @"" ];
	} else if( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::RADIO_BUTTON ) {
		// FIXME: VO should read this because of hierarchy, this is just a workaround
		// get parent and its children
		AquaA11yWrapper * parent = [ self parentAttribute ];
		NSArray * children = [ parent childrenAttribute ];
		// find index of self
		int index = 1;
		NSEnumerator * enumerator = [ children objectEnumerator ];
		AquaA11yWrapper * child = nil;
		while ( ( child = [ enumerator nextObject ] ) ) {
			if ( self == child ) {
				break;
			}
			index++;
		}
		// build string
		NSNumber * nIndex = [ NSNumber numberWithInt: index ];
		NSNumber * nGroupsize = [ NSNumber numberWithInt: [ children count ] ];
		NSMutableString * value = [ [ NSMutableString alloc ] init ];
		[ value appendString: @"radio button " ];
		[ value appendString: [ nIndex stringValue ] ];
		[ value appendString: @" of " ];
		[ value appendString: [ nGroupsize stringValue ] ];
		// clean up and return string
		[ nIndex release ];
		[ nGroupsize release ];
		[ children release ];
		return value;
    } else {
        return [ AquaA11yRoleHelper getRoleDescriptionFrom:
                [ AquaA11yRoleHelper getNativeRoleFrom: [ self accessibleContext ] ]
                with: [ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ] ];
    }
}

-(id)valueAttribute {
    if ( [ [ self roleAttribute ] isEqualToString: NSAccessibilityMenuItemRole ] ) {
        return nil;
    } else if ( [ self accessibleText ] != nil ) {
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

-(id)lineForIndexAttributeForParameter:(id)index {
    if ( [ self accessibleMultiLineText ] != nil ) {
        return [ AquaA11yTextWrapper lineForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rangeForLineAttributeForParameter:(id)line {
    if ( [ self accessibleMultiLineText ] != nil ) {
        return [ AquaA11yTextWrapper rangeForLineAttributeForElement: self forParameter: line ];
    } else {
        return nil;
    }
}

#pragma mark -
#pragma mark Accessibility Protocol

-(id)accessibilityAttributeValue:(NSString *)attribute {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeValue:" << attribute << "]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return nil;
    }

    id value = nil;
    // if we are no longer in the wrapper repository, we have been disposed
    AquaA11yWrapper * theWrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ self accessibleContext ] createIfNotExists: NO ];
    if ( theWrapper != nil || mIsTableCell ) {
        try {
            SEL methodSelector = [ self selectorForAttribute: attribute asGetter: YES withGetterParameter: NO ];
            if ( [ self respondsToSelector: methodSelector ] ) {
                value = [ self performSelector: methodSelector ];
            }
        } catch ( const DisposedException & e ) {
            mIsTableCell = NO; // just to be sure
            [ AquaA11yFactory removeFromWrapperRepositoryFor: [ self accessibleContext ] ];
            return nil;
        } catch ( const Exception & e ) {
            // empty
        }
    }
    if ( theWrapper != nil ) {
        [ theWrapper release ]; // the above called method calls retain on the returned Wrapper
    }
    return value;
}

-(BOOL)accessibilityIsIgnored {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityIsIgnored]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return NO;
    }
    BOOL ignored = NO;
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
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeNames]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return nil;
    }
    NSString * nativeSubrole = nil;
    NSString * title = nil;
    NSMutableArray * attributeNames = nil;
    sal_Int32 nAccessibleChildren = 0;
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
        try
        {
            nAccessibleChildren = [ self accessibleContext ] -> getAccessibleChildCount();
            if (  nAccessibleChildren > 0 ) {
                [ attributeNames addObject: NSAccessibilityChildrenAttribute ];
        }
        }
        catch( DisposedException& ) {}
        catch( RuntimeException& ) {}

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
        if( [self accessibleContext ] -> getAccessibleRole() == AccessibleRole::TABLE )
            [AquaA11yTableWrapper addAttributeNamesTo: attributeNames object: self];

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
        [ AquaA11yFactory removeFromWrapperRepositoryFor: [ self accessibleContext ] ];
        return [ [ NSArray alloc ] init ];
    }
}

-(BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeIsSettable:" << attribute << "]");
    BOOL isSettable = NO;
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
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityParameterizedAttributeNames]");
    NSMutableArray * attributeNames = [ [ NSMutableArray alloc ] init ];
    // Special Attributes depending on interface
    if ( [ self accessibleText ] != nil ) {
        [ AquaA11yTextWrapper addParameterizedAttributeNamesTo: attributeNames ];
    }
    return attributeNames; // TODO: to be completed
}

-(id)accessibilityAttributeValue:(NSString *)attribute forParameter:(id)parameter {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeValue:" << attribute << " forParameter:" << parameter << "]");
    SEL methodSelector = [ self selectorForAttribute: attribute asGetter: YES withGetterParameter: YES ];
    if ( [ self respondsToSelector: methodSelector ] ) {
        return [ self performSelector: methodSelector withObject: parameter ];
    }
    return nil; // TODO: to be completed
}

-(BOOL)accessibilitySetOverrideValue:(id)value forAttribute:(NSString *)attribute
{
    SAL_INFO("vcl.a11y", "[" << self << " accessibilitySetOverrideValue:" << value << " forAttribute:" << attribute << "]");
    (void)value;
    (void)attribute;
    return NO; // TODO
}

-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilitySetValue:" << value << " forAttribute:" << attribute << "]");
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
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityFocusedUIElement]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return nil;
    }

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

-(NSString *)accessibilityActionDescription:(NSString *)action {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityActionDescription:" << action << "]");
    return NSAccessibilityActionDescription(action);
}

-(AquaA11yWrapper *)actionResponder {
    AquaA11yWrapper * wrapper = nil;
    // get some information
    NSString * role = (NSString *) [ self accessibilityAttributeValue: NSAccessibilityRoleAttribute ];
    id enabledAttr = [ self enabledAttribute ];
    BOOL enabled = [ enabledAttr boolValue ];
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
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityPerformAction:" << action << "]");
    AquaA11yWrapper * actionResponder = [ self actionResponder ];
    if ( actionResponder != nil ) {
        [ AquaA11yActionWrapper doAction: action ofElement: actionResponder ];
    }
}

-(NSArray *)accessibilityActionNames {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityActionNames]");
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

-(BOOL)isViewElement:(NSObject *)viewElement hitByPoint:(NSPoint)point {
    BOOL hit = NO;
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

Reference < XAccessibleContext > hitTestRunner ( com::sun::star::awt::Point point,
                                                 Reference < XAccessibleContext > rxAccessibleContext ) {
    Reference < XAccessibleContext > hitChild;
    Reference < XAccessibleContext > emptyReference;
    try {
        Reference < XAccessibleComponent > rxAccessibleComponent ( rxAccessibleContext, UNO_QUERY );
        if ( rxAccessibleComponent.is() ) {
            com::sun::star::awt::Point location = rxAccessibleComponent -> getLocationOnScreen();
            com::sun::star::awt::Point hitPoint ( point.X - location.X , point.Y - location.Y);
            Reference < XAccessible > rxAccessible = rxAccessibleComponent -> getAccessibleAtPoint ( hitPoint );
            if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() &&
                 rxAccessible -> getAccessibleContext() -> getAccessibleChildCount() == 0 ) {
                hitChild = rxAccessible -> getAccessibleContext();
            }
        }

        // iterate the hirerachy looking doing recursive hit testing.
        // apparently necessary as a special treatment for e.g. comboboxes
        if ( !hitChild.is() ) {
            bool bSafeToIterate = true;
            sal_Int32 nCount = rxAccessibleContext -> getAccessibleChildCount();

            if ( nCount < 0 || nCount > SAL_MAX_UINT16 /* slow enough for anyone */ )
                bSafeToIterate = false;
            else { // manages descendants is an horror from the a11y standards guys.
                Reference< XAccessibleStateSet > xStateSet;
                xStateSet = rxAccessibleContext -> getAccessibleStateSet();
                if (xStateSet.is() && xStateSet -> contains(AccessibleStateType::MANAGES_DESCENDANTS ) )
                    bSafeToIterate = false;
            }

            if( bSafeToIterate ) {
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
        }
    } catch ( RuntimeException ) {
        return emptyReference;
    }
    return hitChild;
}

-(id)accessibilityHitTest:(NSPoint)point {
    SAL_INFO("vcl.a11y", "[" << self << " accessibilityHitTest:" << point << "]");
    static id wrapper = nil;
    if ( nil != wrapper ) {
        [ wrapper release ];
        wrapper = nil;
    }
    Reference < XAccessibleContext > hitChild;
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
    com::sun::star::awt::Point hitPoint ( static_cast<long>(point.x) , static_cast<long>(screenRect.size.height - point.y) );
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

-(XAccessibleMultiLineText *)accessibleMultiLineText {
    return mpReferenceWrapper -> rAccessibleMultiLineText.get();
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

-(void)setDefaultFonttraits:(int)fonttraits {
    mDefaultFonttraits = fonttraits;
}

-(int)defaultFonttraits {
    return mDefaultFonttraits;
}

-(void)setActsAsRadioGroup:(BOOL)actsAsRadioGroup {
    mActsAsRadioGroup = actsAsRadioGroup;
}

-(BOOL)actsAsRadioGroup {
    return mActsAsRadioGroup;
}

+(void)setPopupMenuOpen:(BOOL)popupMenuOpen {
    isPopupMenuOpen = popupMenuOpen;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
