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
#include <osx/saldata.hxx>

#include <osx/a11ywrapper.h>
#include <osx/a11ylistener.hxx>
#include <osx/a11yfactory.h>
#include <osx/a11yfocustracker.hxx>

#include <quartz/salgdi.h>
#include <quartz/utils.h>

#include "a11yfocuslistener.hxx"
#include "a11yactionwrapper.h"
#include "a11ycomponentwrapper.h"
#include "a11yselectionwrapper.h"
#include "a11ytablewrapper.h"
#include "a11ytextwrapper.h"
#include "a11yvaluewrapper.h"
#include "a11yrolehelper.h"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessibleEventBroadcaster.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <sal/log.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

@interface SalFrameWindow : NSWindow
{
}
-(Reference<XAccessibleContext>)accessibleContext;
@end

static bool isPopupMenuOpen = false;

static std::ostream &operator<<(std::ostream &s, NSObject *obj) {
    return s << [[obj description] UTF8String];
}

@implementation AquaA11yWrapper

#pragma mark -
#pragma mark Init and dealloc

-(id)init {
    return [ super init ];
}

-(id)initWithAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    self = [ super init ];
    if ( self ) {
        [ self setDefaults: rxAccessibleContext ];
    }
    return self;
}

-(void)setDisposed {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;

    mIsDisposed = YES;

    // Release all strong C++ references
    maReferenceWrapper = ReferenceWrapper();

    // Related tdf@158914 avoid resurrecting object's C++ references
    // Posting an NSAccessibilityUIElementDestroyedNotification
    // notification causes [ AquaA11yWrapper isAccessibilityElement ]
    // to be called on the object so mark the object as disposed
    // before posting the destroyed notification.
    NSAccessibilityPostNotification( self, NSAccessibilityUIElementDestroyedNotification );
}

-(void) setDefaults: (Reference < XAccessibleContext >) rxAccessibleContext {
    mActsAsRadioGroup = NO;
    maReferenceWrapper.rAccessibleContext = rxAccessibleContext;
    mIsTableCell = NO;
    mIsDisposed = NO;
    // Querying all supported interfaces
    try {
        // XAccessibleComponent
        maReferenceWrapper.rAccessibleComponent.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleExtendedComponent
        maReferenceWrapper.rAccessibleExtendedComponent.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleSelection
        maReferenceWrapper.rAccessibleSelection.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleTable
        maReferenceWrapper.rAccessibleTable.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleText
        maReferenceWrapper.rAccessibleText.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleEditableText
        maReferenceWrapper.rAccessibleEditableText.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleValue
        maReferenceWrapper.rAccessibleValue.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleAction
        maReferenceWrapper.rAccessibleAction.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleTextAttributes
        maReferenceWrapper.rAccessibleTextAttributes.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleMultiLineText
        maReferenceWrapper.rAccessibleMultiLineText.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleTextMarkup
        maReferenceWrapper.rAccessibleTextMarkup.set( rxAccessibleContext, UNO_QUERY );
        // XAccessibleEventBroadcaster
        #if 0
        /* #i102033# NSAccessibility does not seemt to know an equivalent for transient children.
           That means we need to cache this, else e.g. tree list boxes are not accessible (moreover
           it crashes by notifying dead objects - which would seemt o be another bug)

           FIXME:
           Unfortunately this can increase memory consumption drastically until the non transient parent
           is destroyed and finally all the transients are released.
        */
        if ( ! ( rxAccessibleContext -> getAccessibleStateSet() & AccessibleStateType::TRANSIENT ) )
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

#pragma mark -
#pragma mark Utility Section

// generates selectors for attribute name AXAttributeNameHere
// (getter without parameter) attributeNameHereAttribute
// (getter with parameter)    attributeNameHereAttributeForParameter:
// (setter)                   setAttributeNameHereAttributeForElement:to:
-(SEL)selectorForAttribute:(NSString *)attribute asGetter:(BOOL)asGetter withGetterParameter:(BOOL)withGetterParameter {
    SEL selector = static_cast<SEL>(nil);
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    @try {
        // step 1: create method name from attribute name
        NSMutableString * methodName = [ NSMutableString string ];
        if ( ! asGetter ) {
            [ methodName appendString: @"set" ];
        }
        NSRange const aRange = { 2, 1 };
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
    } @catch ( id  ) {
        selector = static_cast<SEL>(nil);
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
    RelationSet. In NSAccessibility the relationship is expressed through the hierarchy. An AXRadioGroup contains two or more AXRadioButton
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
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
                //TODO: 10.10 accessibilityAttributeValue:
            return [ super accessibilityAttributeValue: NSAccessibilitySubroleAttribute ];
            SAL_WNODEPRECATED_DECLARATIONS_POP
        }
    }
}

-(id)titleAttribute {
    // Related tdf#158914: explicitly call autorelease selector
    // CreateNSString() is not a getter. It expects the caller to
    // release the returned string.
    return [ CreateNSString ( [ self accessibleContext ] -> getAccessibleName() ) autorelease ];
}

-(id)descriptionAttribute {
    if ( [ self accessibleContext ] -> getAccessibleRole() == AccessibleRole::COMBO_BOX ) {
        return [ self titleAttribute ];
    } else if ( [ self accessibleExtendedComponent ] ) {
        return [ AquaA11yComponentWrapper descriptionAttributeForElement: self ];
    } else {
        // Related tdf#158914: explicitly call autorelease selector
        // CreateNSString() is not a getter. It expects the caller to
        // release the returned string.
        return [ CreateNSString ( [ self accessibleContext ] -> getAccessibleDescription() ) autorelease ];
    }
}

-(id)enabledAttribute {
    sal_Int64 nStateSet = [ self accessibleContext ] -> getAccessibleStateSet();
    if ( nStateSet ) {
        return [ NSNumber numberWithBool: ( (nStateSet & AccessibleStateType::ENABLED) != 0 ) ];
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
            if ( rxContext.is() ) {
                sal_Int64 nStateSet = rxContext -> getAccessibleStateSet();
                if ( nStateSet ) {
                    isFocused = [ NSNumber numberWithBool: ( ( nStateSet & AccessibleStateType::FOCUSED ) != 0 ) ];
                }
            }
        }
        return isFocused;
    } else if ( [ self accessibleContext ] -> getAccessibleStateSet() ) {
        sal_Int64 nStateSet = [ self accessibleContext ] -> getAccessibleStateSet();
        return [ NSNumber numberWithBool: ( ( nStateSet & AccessibleStateType::FOCUSED ) != 0 ) ];
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

    OSL_ASSERT( false );
    return nil;
}

-(id)childrenAttribute {
    if ( mActsAsRadioGroup ) {
        NSMutableArray * children = [ [ NSMutableArray alloc ] init ];
        Reference < XAccessibleRelationSet > rxAccessibleRelationSet = [ self accessibleContext ] -> getAccessibleRelationSet();
        AccessibleRelation const relationMemberOf = rxAccessibleRelationSet -> getRelationByType ( AccessibleRelationType::MEMBER_OF );
        if ( relationMemberOf.RelationType == AccessibleRelationType::MEMBER_OF && relationMemberOf.TargetSet.hasElements() ) {
            for ( const auto& i : relationMemberOf.TargetSet ) {
                Reference < XAccessible > rMateAccessible( i, UNO_QUERY );
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
        return [children autorelease];
    } else if ( [ self accessibleTable ] )
    {
        AquaA11yTableWrapper* pTable = [self isKindOfClass: [AquaA11yTableWrapper class]] ? static_cast<AquaA11yTableWrapper*>(self) : nil;
        return [ AquaA11yTableWrapper childrenAttributeForElement: pTable ];
    } else {
        NSMutableArray * children = [ [ NSMutableArray alloc ] init ];

        try {
            Reference< XAccessibleContext > xContext( [ self accessibleContext ] );

            try {
                sal_Int64 cnt = xContext -> getAccessibleChildCount();
                for ( sal_Int64 i = 0; i < cnt; i++ ) {
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
            }
            catch (const IndexOutOfBoundsException&)
            {
                SAL_WARN("vcl", "Accessible object has invalid index in parent");
            }

            // if not already acting as RadioGroup now is the time to replace RadioButtons with RadioGroups and remove RadioButtons
            if ( ! mActsAsRadioGroup ) {
                NSEnumerator * enumerator = [ children objectEnumerator ];
                AquaA11yWrapper * element;
                while ( ( element = static_cast<AquaA11yWrapper *>([ enumerator nextObject ]) ) ) {
                    if ( [ element accessibleContext ] -> getAccessibleRole() == AccessibleRole::RADIO_BUTTON ) {
                        if ( [ element isFirstRadioButtonInGroup ] ) {
                            id wrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ element accessibleContext ] createIfNotExists: YES asRadioGroup: YES ];
                            [ children replaceObjectAtIndex: [ children indexOfObjectIdenticalTo: element ] withObject: wrapper ];
                        }
                        [ children removeObject: element ];
                    }
                }
            }

            return NSAccessibilityUnignoredChildren( [ children autorelease ] );
        } catch (const Exception &) {
            // TODO: Log
        }

        [ children autorelease ];
        return [NSArray array];
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
    NSWindow* theWindow = [ aWrapper windowForParent ];
    return theWindow;
}

-(id)topLevelUIElementAttribute {
    return [ self windowAttribute ];
}

-(id)sizeAttribute {
    if ( [ self accessibleComponent ] ) {
        return [ AquaA11yComponentWrapper sizeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)positionAttribute {
    if ( [ self accessibleComponent ] ) {
        return [ AquaA11yComponentWrapper positionAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)helpAttribute {
    // Related tdf#158914: explicitly call autorelease selector
    // CreateNSString() is not a getter. It expects the caller to
    // release the returned string.
    return [ CreateNSString ( [ self accessibleContext ] -> getAccessibleDescription() ) autorelease ];
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
        NSMutableString * value = [ NSMutableString string ];
        [ value appendString: @"radio button " ];
        [ value appendString: [ nIndex stringValue ] ];
        [ value appendString: @" of " ];
        [ value appendString: [ nGroupsize stringValue ] ];
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
    } else if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper valueAttributeForElement: self ];
    } else if ( [ self accessibleValue ] ) {
        return [ AquaA11yValueWrapper valueAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)minValueAttribute {
    if ( [ self accessibleValue ] ) {
        return [ AquaA11yValueWrapper minValueAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)maxValueAttribute {
    if ( [ self accessibleValue ] ) {
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
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper numberOfCharactersAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)selectedTextAttribute {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper selectedTextAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)selectedTextRangeAttribute {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper selectedTextRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)visibleCharacterRangeAttribute {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper visibleCharacterRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)tabsAttribute {
    return self; // TODO ???
}

-(id)sharedTextUIElementsAttribute {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper sharedTextUIElementsAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)sharedCharacterRangeAttribute {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper sharedCharacterRangeAttributeForElement: self ];
    } else {
        return nil;
    }
}

-(id)expandedAttribute {
    sal_Int64 nStateSet = [ self accessibleContext ] -> getAccessibleStateSet();
    return [ NSNumber numberWithBool: ( ( nStateSet & AccessibleStateType::EXPANDED ) != 0 ) ];
}

-(id)selectedAttribute {
    sal_Int64 nStateSet = [ self accessibleContext ] -> getAccessibleStateSet();
    return [ NSNumber numberWithBool: ( ( nStateSet & AccessibleStateType::SELECTED ) != 0 ) ];
}

-(id)stringForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper stringForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)attributedStringForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper attributedStringForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)rangeForIndexAttributeForParameter:(id)index {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper rangeForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rangeForPositionAttributeForParameter:(id)point {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper rangeForPositionAttributeForElement: self forParameter: point ];
    } else {
        return nil;
    }
}

-(id)boundsForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper boundsForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)styleRangeForIndexAttributeForParameter:(id)index {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper styleRangeForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rTFForRangeAttributeForParameter:(id)range {
    if ( [ self accessibleText ] ) {
        return [ AquaA11yTextWrapper rTFForRangeAttributeForElement: self forParameter: range ];
    } else {
        return nil;
    }
}

-(id)orientationAttribute {
    NSString * orientation = nil;
    sal_Int64 stateSet = [ self accessibleContext ] -> getAccessibleStateSet();
    if ( stateSet & AccessibleStateType::HORIZONTAL ) {
        orientation = NSAccessibilityHorizontalOrientationValue;
    } else if ( stateSet & AccessibleStateType::VERTICAL ) {
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
    if ( [ self accessibleMultiLineText ] ) {
        return [ AquaA11yTextWrapper lineForIndexAttributeForElement: self forParameter: index ];
    } else {
        return nil;
    }
}

-(id)rangeForLineAttributeForParameter:(id)line {
    if ( [ self accessibleMultiLineText ] ) {
        return [ AquaA11yTextWrapper rangeForLineAttributeForElement: self forParameter: line ];
    } else {
        return nil;
    }
}

#pragma mark -
#pragma mark Accessibility Protocol

-(id)accessibilityAttributeValue:(NSString *)attribute {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return nil;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeValue:" << attribute << "]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return nil;
    }

    id value = nil;
    // if we are no longer in the wrapper repository, we have been disposed
    AquaA11yWrapper * theWrapper = [ AquaA11yFactory wrapperForAccessibleContext: [ self accessibleContext ] createIfNotExists: NO ];
    if ( theWrapper || mIsTableCell ) {
        try {
            SEL methodSelector = [ self selectorForAttribute: attribute asGetter: YES withGetterParameter: NO ];
            if ( [ self respondsToSelector: methodSelector ] ) {
                value = [ self performSelector: methodSelector ];
            }
        } catch ( const DisposedException & ) {
            mIsTableCell = NO; // just to be sure
            [ AquaA11yFactory removeFromWrapperRepositoryFor: [ self accessibleContext ] ];
            return nil;
        } catch ( const Exception & ) {
            // empty
        }
    }
    if ( theWrapper ) {
        [ theWrapper release ]; // the above called method calls retain on the returned Wrapper
    }
    return value;
}

-(BOOL)accessibilityIsIgnored {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return YES;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityIsIgnored]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return NO;
    }
    BOOL ignored = false;
    try {
        sal_Int16 nRole = [ self accessibleContext ] -> getAccessibleRole();
        switch ( nRole ) {
            //case AccessibleRole::PANEL:
            case AccessibleRole::FRAME:
            case AccessibleRole::ROOT_PANE:
            case AccessibleRole::SEPARATOR:
            case AccessibleRole::FILLER:
            case AccessibleRole::DIALOG:
                ignored = true;
                break;
            default:
                ignored = ! ( [ self accessibleContext ] -> getAccessibleStateSet() & AccessibleStateType::VISIBLE );
                break;
        }
    } catch ( DisposedException& ) {
        ignored = true;
    } catch ( RuntimeException& ) {
        ignored = true;
    }

    return ignored; // TODO: to be completed
}

-(NSArray *)accessibilityAttributeNames {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return nil;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeNames]");
    // #i90575# guard NSAccessibility protocol against unwanted access
    if ( isPopupMenuOpen ) {
        return nil;
    }
    NSString * nativeSubrole = nil;
    NSString * title = nil;
    NSMutableArray * attributeNames = nil;
    sal_Int64 nAccessibleChildren = 0;
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
        nativeSubrole = static_cast<NSString *>([ AquaA11yRoleHelper getNativeSubroleFrom: [ self accessibleContext ] -> getAccessibleRole() ]);
        title = static_cast<NSString *>([ self titleAttribute ]);
        Reference < XAccessibleRelationSet > rxRelationSet = [ self accessibleContext ] -> getAccessibleRelationSet();
        // Special Attributes depending on attribute values
        if ( nativeSubrole && ! [ nativeSubrole isEqualToString: @"" ] ) {
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

        if ( title && ! [ title isEqualToString: @"" ] ) {
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

        if ( [ self accessibleText ] ) {
            [ AquaA11yTextWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleComponent ] ) {
            [ AquaA11yComponentWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleSelection ] ) {
            [ AquaA11ySelectionWrapper addAttributeNamesTo: attributeNames ];
        }
        if ( [ self accessibleValue ] ) {
            [ AquaA11yValueWrapper addAttributeNamesTo: attributeNames ];
        }
        // Related: tdf#153374 Don't release autoreleased attributeNames
        return attributeNames;
    } catch ( DisposedException & ) { // Object is no longer available
        // Related: tdf#153374 Don't release autoreleased attributeNames
        // Also, return an autoreleased empty array instead of a retained array.
        [ AquaA11yFactory removeFromWrapperRepositoryFor: [ self accessibleContext ] ];
        return [ NSArray array ];
    }
}

-(BOOL)accessibilityIsAttributeSettable:(NSString *)attribute {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return NO;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeIsSettable:" << attribute << "]");
    bool isSettable = false;
    if ( [ self accessibleText ] ) {
        isSettable = [ AquaA11yTextWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleComponent ] ) {
        isSettable = [ AquaA11yComponentWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleSelection ] ) {
        isSettable = [ AquaA11ySelectionWrapper isAttributeSettable: attribute forElement: self ];
    }
    if ( ! isSettable && [ self accessibleValue ] ) {
        isSettable = [ AquaA11yValueWrapper isAttributeSettable: attribute forElement: self ];
    }
    return isSettable; // TODO: to be completed
}

-(NSArray *)accessibilityParameterizedAttributeNames {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return [ NSArray array ];

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityParameterizedAttributeNames]");
    NSMutableArray * attributeNames = [ NSMutableArray array ];
    // Special Attributes depending on interface
    if ( [ self accessibleText ] ) {
        [ AquaA11yTextWrapper addParameterizedAttributeNamesTo: attributeNames ];
    }
    return attributeNames; // TODO: to be completed
}

-(id)accessibilityAttributeValue:(NSString *)attribute forParameter:(id)parameter {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return nil;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityAttributeValue:" << attribute << " forParameter:" << (static_cast<NSObject*>(parameter)) << "]");
    SEL methodSelector = [ self selectorForAttribute: attribute asGetter: YES withGetterParameter: YES ];
    if ( [ self respondsToSelector: methodSelector ] ) {
        try {
            return [ self performSelector: methodSelector withObject: parameter ];
        } catch ( const DisposedException & ) {
            mIsTableCell = NO; // just to be sure
            [ AquaA11yFactory removeFromWrapperRepositoryFor: [ self accessibleContext ] ];
            return nil;
        } catch ( const Exception & ) {
            // empty
        }
    }
    return nil; // TODO: to be completed
}

-(BOOL)accessibilitySetOverrideValue:(id)value forAttribute:(NSString *)attribute
{
    SAL_INFO("vcl.a11y", "[" << self << " accessibilitySetOverrideValue:" << (static_cast<NSObject*>(value)) << " forAttribute:" << attribute << "]");
    return NO; // TODO
}

-(void)accessibilitySetValue:(id)value forAttribute:(NSString *)attribute {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilitySetValue:" << (static_cast<NSObject*>(value)) << " forAttribute:" << attribute << "]");
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
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return nil;

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
    NSString * role = static_cast<NSString *>([ self accessibilityAttributeValue: NSAccessibilityRoleAttribute ]);
    id enabledAttr = [ self enabledAttribute ];
    bool enabled = [ enabledAttr boolValue ];
    NSView * parent = static_cast<NSView *>([ self accessibilityAttributeValue: NSAccessibilityParentAttribute ]);
    AquaA11yWrapper * parentAsWrapper = nil;
    if ( [ parent isKindOfClass: [ AquaA11yWrapper class ] ] ) {
        parentAsWrapper = static_cast<AquaA11yWrapper *>(parent);
    }
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
        //TODO: 10.10 accessibilityAttributeValue:
    NSString * parentRole = static_cast<NSString *>([ parent accessibilityAttributeValue: NSAccessibilityRoleAttribute ]);
    SAL_WNODEPRECATED_DECLARATIONS_POP
    // if we are a textarea inside a combobox, then the combobox is the action responder
    if ( enabled
      && [ role isEqualToString: NSAccessibilityTextAreaRole ]
      && [ parentRole isEqualToString: NSAccessibilityComboBoxRole ]
      && parentAsWrapper ) {
        wrapper = parentAsWrapper;
    } else if ( enabled && [ self accessibleAction ] ) {
        wrapper = self ;
    }
    return wrapper;
}

-(void)accessibilityPerformAction:(NSString *)action {
    [ self performAction: action ];
}

-(BOOL)performAction:(NSString *)action {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return NO;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityPerformAction:" << action << "]");
    AquaA11yWrapper * actionResponder = [ self actionResponder ];
    if ( actionResponder ) {
        [ AquaA11yActionWrapper doAction: action ofElement: actionResponder ];
        return YES;
    }
    return NO;
}

-(NSArray *)accessibilityActionNames {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return nil;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityActionNames]");
    NSArray * actionNames = nil;
    AquaA11yWrapper * actionResponder = [ self actionResponder ];
    if ( actionResponder ) {
        actionNames = [ AquaA11yActionWrapper actionNamesForElement: actionResponder ];
    } else {
        actionNames = [ NSArray array ];
    }
    return actionNames;
}

#pragma mark -
#pragma mark Hit Test

-(BOOL)isViewElement:(NSObject *)viewElement hitByPoint:(NSPoint)point {
    bool hit = false;
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
        //TODO: 10.10 accessibilityAttributeValue:
    NSValue * position = [ viewElement accessibilityAttributeValue: NSAccessibilityPositionAttribute ];
    NSValue * size = [ viewElement accessibilityAttributeValue: NSAccessibilitySizeAttribute ];
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if ( position && size ) {
        float minX = [ position pointValue ].x;
        float minY = [ position pointValue ].y;
        float maxX = minX + [ size sizeValue ].width;
        float maxY = minY + [ size sizeValue ].height;
        if ( minX < point.x && maxX > point.x && minY < point.y && maxY > point.y ) {
            hit = true;
        }
    }
    [ pool release ];
    return hit;
}

static Reference < XAccessibleContext > hitTestRunner ( css::awt::Point point,
                                                 Reference < XAccessibleContext > const & rxAccessibleContext ) {
    Reference < XAccessibleContext > hitChild;
    Reference < XAccessibleContext > emptyReference;
    try {
        Reference < XAccessibleComponent > rxAccessibleComponent ( rxAccessibleContext, UNO_QUERY );
        if ( rxAccessibleComponent.is() ) {
            css::awt::Point location = rxAccessibleComponent -> getLocationOnScreen();
            css::awt::Point hitPoint ( point.X - location.X , point.Y - location.Y);
            Reference < XAccessible > rxAccessible = rxAccessibleComponent -> getAccessibleAtPoint ( hitPoint );
            if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() &&
                 rxAccessible -> getAccessibleContext() -> getAccessibleChildCount() == 0 ) {
                hitChild = rxAccessible -> getAccessibleContext();
            }
        }

        // iterate the hierarchy looking doing recursive hit testing.
        // apparently necessary as a special treatment for e.g. comboboxes
        if ( !hitChild.is() ) {
            bool bSafeToIterate = true;
            sal_Int64 nCount = rxAccessibleContext -> getAccessibleChildCount();

            if (nCount < 0 || nCount > SAL_MAX_UINT16 /* slow enough for anyone */)
                bSafeToIterate = false;
            else { // manages descendants is an horror from the a11y standards guys.
                sal_Int64 nStateSet = rxAccessibleContext -> getAccessibleStateSet();
                if ( nStateSet & AccessibleStateType::MANAGES_DESCENDANTS )
                    bSafeToIterate = false;
            }

            if( bSafeToIterate ) {
                try {
                    for ( sal_Int64 i = 0; i < rxAccessibleContext -> getAccessibleChildCount(); i++ ) {
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
                catch (const IndexOutOfBoundsException&)
                {
                    SAL_WARN("vcl", "Accessible object has invalid index in parent");
                }
            }
        }
    } catch ( RuntimeException ) {
        return emptyReference;
    }
    return hitChild;
}

-(id)accessibilityHitTest:(NSPoint)point {
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return nil;

    SAL_INFO("vcl.a11y", "[" << self << " accessibilityHitTest:" << point << "]");
    Reference < XAccessibleContext > hitChild;
    NSRect screenRect = [ [ NSScreen mainScreen ] frame ];
    css::awt::Point hitPoint ( static_cast<sal_Int32>(point.x) , static_cast<sal_Int32>(screenRect.size.height - point.y) );
    // check child windows first
    NSWindow * window = static_cast<NSWindow *>([ self accessibilityAttributeValue: NSAccessibilityWindowAttribute ]);
    NSArray * childWindows = [ window childWindows ];
    if ( [ childWindows count ] > 0 ) {
        NSWindow * element = nil;
        NSEnumerator * enumerator = [ childWindows objectEnumerator ];
        while ( ( element = [ enumerator nextObject ] ) && !hitChild.is() ) {
            if ( [ element isKindOfClass: [ SalFrameWindow class ] ] && [ self isViewElement: element hitByPoint: point ] ) {
                // we have a child window that is hit
                Reference < XAccessibleRelationSet > relationSet = [ static_cast<SalFrameWindow *>(element) accessibleContext ] -> getAccessibleRelationSet();
                if ( relationSet.is() && relationSet -> containsRelation ( AccessibleRelationType::SUB_WINDOW_OF )) {
                    // we have a valid relation to the parent element
                    AccessibleRelation const relation = relationSet -> getRelationByType ( AccessibleRelationType::SUB_WINDOW_OF );
                    for ( const auto & i : relation.TargetSet ) {
                        Reference < XAccessible > rxAccessible ( i, UNO_QUERY );
                        if ( rxAccessible.is() && rxAccessible -> getAccessibleContext().is() ) {
                            // hit test for children of parent
                            hitChild = hitTestRunner ( hitPoint, rxAccessible -> getAccessibleContext() );
                            if (hitChild.is())
                                break;
                        }
                    }
                }
            }
        }
    }
    // nothing hit yet, so check ourself
    if ( ! hitChild.is() ) {
        if ( !maReferenceWrapper.rAccessibleContext ) {
            [ self setDefaults: [ self accessibleContext ] ];
        }
        hitChild = hitTestRunner ( hitPoint, maReferenceWrapper.rAccessibleContext );
    }
    if ( hitChild.is() ) {
        // Related tdf#158914: do not retain wrapper
        // [ AquaA11yFactory wrapperForAccessibleContext: ] already retains
        // the returned object so retaining it until the next call to this
        // selector can lead to a memory leak when dragging selected cells
        // in Calc to a new location. So autorelease the object so that
        // transient objects stay alive but not past the next clearing of
        // the autorelease pool.
        return [ [ AquaA11yFactory wrapperForAccessibleContext: hitChild ] autorelease ];
    }
    return nil;
}

#pragma mark -
#pragma mark Access Methods

-(XAccessibleAction *)accessibleAction {
    return maReferenceWrapper.rAccessibleAction.get();
}

-(XAccessibleContext *)accessibleContext {
    return maReferenceWrapper.rAccessibleContext.get();
}

-(XAccessibleComponent *)accessibleComponent {
    return maReferenceWrapper.rAccessibleComponent.get();
}

-(XAccessibleExtendedComponent *)accessibleExtendedComponent {
    return maReferenceWrapper.rAccessibleExtendedComponent.get();
}

-(XAccessibleSelection *)accessibleSelection {
    return maReferenceWrapper.rAccessibleSelection.get();
}

-(XAccessibleTable *)accessibleTable {
    return maReferenceWrapper.rAccessibleTable.get();
}

-(XAccessibleText *)accessibleText {
    return maReferenceWrapper.rAccessibleText.get();
}

-(XAccessibleEditableText *)accessibleEditableText {
    return maReferenceWrapper.rAccessibleEditableText.get();
}

-(XAccessibleValue *)accessibleValue {
    return maReferenceWrapper.rAccessibleValue.get();
}

-(XAccessibleTextAttributes *)accessibleTextAttributes {
    return maReferenceWrapper.rAccessibleTextAttributes.get();
}

-(XAccessibleMultiLineText *)accessibleMultiLineText {
    return maReferenceWrapper.rAccessibleMultiLineText.get();
}

-(XAccessibleTextMarkup *)accessibleTextMarkup {
    return maReferenceWrapper.rAccessibleTextMarkup.get();
}

-(NSWindow*)windowForParent {
    return nil;
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

// NSAccessibility selectors

- (BOOL)isAccessibilityElement
{
    return ! [ self accessibilityIsIgnored ];
}

- (BOOL)isAccessibilityFocused
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityFocusedAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (id)accessibilityTopLevelUIElement
{
    return [ self accessibilityAttributeValue: NSAccessibilityTopLevelUIElementAttribute ];
}

- (id)accessibilityValue
{
    return [ self accessibilityAttributeValue: NSAccessibilityValueAttribute ];
}

- (NSArray *)accessibilityVisibleChildren
{
    return [ self accessibilityChildren ];
}

- (NSAccessibilitySubrole)accessibilitySubrole
{
    return [ self accessibilityAttributeValue: NSAccessibilitySubroleAttribute ];
}

- (NSString *)accessibilityTitle
{
    return [ self accessibilityAttributeValue: NSAccessibilityTitleAttribute ];
}

- (id)accessibilityTitleUIElement
{
    return [ self accessibilityAttributeValue: NSAccessibilityTitleUIElementAttribute ];
}

- (NSAccessibilityOrientation)accessibilityOrientation
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityOrientationAttribute ];
    if ( pNumber )
        return NSAccessibilityOrientation([ pNumber integerValue ]);
    else
        return NSAccessibilityOrientationUnknown;
}

- (id)accessibilityParent
{
    return [ self accessibilityAttributeValue: NSAccessibilityParentAttribute ];
}

- (NSAccessibilityRole)accessibilityRole
{
    return [ self accessibilityAttributeValue: NSAccessibilityRoleAttribute ];
}

- (NSString *)accessibilityRoleDescription
{
    return [ self accessibilityAttributeValue: NSAccessibilityRoleDescriptionAttribute ];
}

- (BOOL)isAccessibilitySelected
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilitySelectedAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (NSArray *)accessibilitySelectedChildren
{
    return [ self accessibilityAttributeValue: NSAccessibilitySelectedChildrenAttribute ];
}

- (NSArray *)accessibilityServesAsTitleForUIElements
{
    return [ self accessibilityAttributeValue: NSAccessibilityServesAsTitleForUIElementsAttribute ];
}

- (id)accessibilityMinValue
{
    return [ self accessibilityAttributeValue: NSAccessibilityMinValueAttribute ];
}

- (id)accessibilityMaxValue
{
    return [ self accessibilityAttributeValue: NSAccessibilityMaxValueAttribute ];
}

- (id)accessibilityWindow
{
    return [ self accessibilityAttributeValue: NSAccessibilityWindowAttribute ];
}

- (NSString *)accessibilityHelp
{
    return [ self accessibilityAttributeValue: NSAccessibilityHelpAttribute ];
}

- (BOOL)isAccessibilityExpanded
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityExpandedAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (BOOL)isAccessibilityEnabled
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityEnabledAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (NSArray *)accessibilityChildren
{
    return [ self accessibilityAttributeValue: NSAccessibilityChildrenAttribute ];
}

- (NSArray <id<NSAccessibilityElement>> *)accessibilityChildrenInNavigationOrder
{
    return [ self accessibilityChildren ];
}

- (NSArray *)accessibilityContents
{
    return [ self accessibilityAttributeValue: NSAccessibilityContentsAttribute ];
}

- (NSString *)accessibilityLabel
{
    return [ self accessibilityAttributeValue: NSAccessibilityDescriptionAttribute ];
}

- (id)accessibilityApplicationFocusedUIElement
{
    return [ self accessibilityFocusedUIElement ];
}

- (BOOL)isAccessibilityDisclosed
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityDisclosingAttribute ];
    if ( pNumber )
        return [ pNumber boolValue ];
    else
        return NO;
}

- (id)accessibilityHorizontalScrollBar
{
    return [ self accessibilityAttributeValue: NSAccessibilityHorizontalScrollBarAttribute ];
}

- (id)accessibilityVerticalScrollBar
{
    return [ self accessibilityAttributeValue: NSAccessibilityVerticalScrollBarAttribute ];
}

- (NSArray *)accessibilityTabs
{
    return [ self accessibilityAttributeValue: NSAccessibilityTabsAttribute ];
}

- (NSArray *)accessibilityColumns
{
    return [ self accessibilityAttributeValue: NSAccessibilityColumnsAttribute ];
}

- (NSArray *)accessibilityRows
{
    return [ self accessibilityAttributeValue: NSAccessibilityRowsAttribute ];
}

- (NSRange)accessibilitySharedCharacterRange
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilitySharedCharacterRangeAttribute ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSArray *)accessibilitySharedTextUIElements
{
    return [ self accessibilityAttributeValue: NSAccessibilitySharedTextUIElementsAttribute ];
}

- (NSRange)accessibilityVisibleCharacterRange
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityVisibleCharacterRangeAttribute ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSInteger)accessibilityNumberOfCharacters
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityNumberOfCharactersAttribute ];
    if ( pNumber )
        return [ pNumber integerValue ];
    else
        return 0;
}

- (NSString *)accessibilitySelectedText
{
    return [ self accessibilityAttributeValue: NSAccessibilitySelectedTextAttribute ];
}

- (NSRange)accessibilitySelectedTextRange
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilitySelectedTextRangeAttribute ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSAttributedString *)accessibilityAttributedStringForRange:(NSRange)aRange
{
    return [ self accessibilityAttributeValue: NSAccessibilityAttributedStringForRangeParameterizedAttribute forParameter: [ NSValue valueWithRange: aRange ] ];
}

- (NSRange)accessibilityRangeForLine:(NSInteger)nLine
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityRangeForLineParameterizedAttribute forParameter: [NSNumber numberWithInteger: nLine ] ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSString *)accessibilityStringForRange:(NSRange)aRange
{
    return [ self accessibilityAttributeValue: NSAccessibilityStringForRangeParameterizedAttribute forParameter: [ NSValue valueWithRange: aRange ] ];
}

- (NSRange)accessibilityRangeForPosition:(NSPoint)aPoint
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityRangeForPositionParameterizedAttribute forParameter: [ NSValue valueWithPoint: aPoint ] ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSRange)accessibilityRangeForIndex:(NSInteger)nIndex
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityRangeForIndexParameterizedAttribute forParameter: [ NSNumber numberWithInteger: nIndex ] ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSRect)accessibilityFrameForRange:(NSRange)aRange
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityBoundsForRangeParameterizedAttribute forParameter: [ NSValue valueWithRange: aRange ] ];
    if ( pValue )
        return [ pValue rectValue ];
    else
        return NSZeroRect;
}

- (NSData *)accessibilityRTFForRange:(NSRange)aRange
{
    return [ self accessibilityAttributeValue: NSAccessibilityRTFForRangeParameterizedAttribute forParameter: [ NSValue valueWithRange: aRange ] ];
}

- (NSRange)accessibilityStyleRangeForIndex:(NSInteger)nIndex
{
    NSValue *pValue = [ self accessibilityAttributeValue: NSAccessibilityStyleRangeForIndexParameterizedAttribute forParameter: [ NSNumber numberWithInteger: nIndex ] ];
    if ( pValue )
        return [ pValue rangeValue ];
    else
        return NSMakeRange( NSNotFound, 0 );
}

- (NSInteger)accessibilityLineForIndex:(NSInteger)nIndex
{
    NSNumber *pNumber = [ self accessibilityAttributeValue: NSAccessibilityLineForIndexParameterizedAttribute forParameter: [ NSNumber numberWithInteger: nIndex ] ];
    if ( pNumber )
        return [ pNumber integerValue ];
    else
        return 0;
}

- (BOOL)accessibilityPerformDecrement
{
    return [ self performAction: NSAccessibilityDecrementAction ];
}

- (BOOL)accessibilityPerformPick
{
    return [ self performAction: NSAccessibilityPickAction ];
}

- (BOOL)accessibilityPerformShowMenu
{
    return [ self performAction: NSAccessibilityShowMenuAction ];
}

- (BOOL)accessibilityPerformPress
{
    return [ self performAction: NSAccessibilityPressAction ];
}

- (BOOL)accessibilityPerformIncrement
{
    return [ self performAction: NSAccessibilityIncrementAction ];
}

// NSAccessibilityElement selectors

- (NSRect)accessibilityFrame
{
    // Related: tdf#148453 Acquire solar mutex during native accessibility calls
    SolarMutexGuard aGuard;
    if ( mIsDisposed )
        return NSZeroRect;

    try {
        XAccessibleComponent *pAccessibleComponent = [ self accessibleComponent ];
        if ( pAccessibleComponent ) {
            com::sun::star::awt::Point location = pAccessibleComponent->getLocationOnScreen();
            com::sun::star::awt::Size size = pAccessibleComponent->getSize();
            NSRect screenRect = sal::aqua::getTotalScreenBounds();
            NSRect frame = NSMakeRect( float(location.X), float( screenRect.size.height - size.Height - location.Y ), float(size.Width), float(size.Height) );
            return frame;
        }
    } catch ( DisposedException& ) {
    } catch ( RuntimeException& ) {
    }

    return NSZeroRect;
}

- (BOOL)accessibilityNotifiesWhenDestroyed
{
    return YES;
}

- (BOOL)isAccessibilitySelectorAllowed:(SEL)aSelector
{
    if ( ! aSelector )
        return NO;

    // don't explicitly report (non-)expanded state when not expandable
    if (aSelector == @selector(isAccessibilityExpanded))
    {
        // Acquire solar mutex during native accessibility calls
        SolarMutexGuard aGuard;
        if ( mIsDisposed )
            return NO;

        const sal_Int64 nStateSet = [ self accessibleContext ] -> getAccessibleStateSet();
        if (!( nStateSet & AccessibleStateType::EXPANDABLE))
            return false;
    }

    if ( [ self respondsToSelector: aSelector ] ) {
        // Ignore actions if action is not supported
        NSAccessibilityActionName pActionName = [ AquaA11yActionWrapper actionNameForSelector: aSelector ];
        if ( pActionName ) {
            NSArray *pActionNames = [ self accessibilityActionNames ];
            if ( ! pActionNames || ! [ pActionNames containsObject: pActionName ] )
                return NO;
        } else {
            // Ignore "setAccessibility" selectors if attribute is not settable
            static NSString *pSetPrefix = @"setAccessibility";
            NSString *pSelName = NSStringFromSelector( aSelector );
            if ( pSelName && [ pSelName hasPrefix: pSetPrefix ] && [ pSelName hasSuffix: @":" ] ) {
                NSAccessibilityAttributeName pAttrName = [ pSelName substringToIndex: [ pSelName length ] - 1 ];
                if ( pAttrName && [ pAttrName length ] > [ pSetPrefix length ] ) {
                    pAttrName = [ pAttrName substringFromIndex: [ pSetPrefix length ] ];
                    if ( pAttrName && [ pAttrName length ] ) {
                        pAttrName = [ @"AX" stringByAppendingString: pAttrName ];
                        if ( pAttrName && [ pAttrName length ] && ! [ self accessibilityIsAttributeSettable: pAttrName ] )
                                return NO;
                    }
                }
            }
        }
    }

    return [ super isAccessibilitySelectorAllowed: aSelector ];
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
