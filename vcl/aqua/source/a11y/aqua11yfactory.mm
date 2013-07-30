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
#include "aqua/aqua11yfactory.h"
#include "aqua/aqua11yfocustracker.hxx"

#include "aqua11yfocuslistener.hxx"
#include "aqua11yrolehelper.h"
#include "aqua11ywrapperbutton.h"
#include "aqua11ywrapperstatictext.h"
#include "aqua11ywrappertextarea.h"
#include "aqua11ywrappercheckbox.h"
#include "aqua11ywrappercombobox.h"
#include "aqua11ywrappergroup.h"
#include "aqua11ywrapperlist.h"
#include "aqua11ywrapperradiobutton.h"
#include "aqua11ywrapperradiogroup.h"
#include "aqua11ywrapperrow.h"
#include "aqua11ywrapperscrollarea.h"
#include "aqua11ywrapperscrollbar.h"
#include "aqua11ywrappersplitter.h"
#include "aqua11ywrappertabgroup.h"
#include "aqua11ywrappertoolbar.h"
#include "aqua11ytablewrapper.h"

#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

static bool enabled = false; 

@implementation AquaA11yFactory : NSObject

#pragma mark -
#pragma mark Wrapper Repository

+(NSMutableDictionary *)allWrapper {
    static NSMutableDictionary * mdAllWrapper = nil;
    if ( mdAllWrapper == nil ) {
        mdAllWrapper = [ [ [ NSMutableDictionary alloc ] init ] retain ];
        // initialize keyboard focus tracker
        rtl::Reference< AquaA11yFocusListener > listener( AquaA11yFocusListener::get() );
        AquaA11yFocusTracker::get().setFocusListener(listener.get());
        enabled = true;      
    }
    return mdAllWrapper;
}

+(NSValue *)keyForAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    return [ NSValue valueWithPointer: rxAccessibleContext.get() ];
}

+(NSValue *)keyForAccessibleContextAsRadioGroup: (Reference < XAccessibleContext >) rxAccessibleContext {
    return [ NSValue valueWithPointer: ( rxAccessibleContext.get() + 2 ) ];
}

+(AquaA11yWrapper *)wrapperForAccessible: (Reference < XAccessible >) rxAccessible {
    if ( rxAccessible.is() ) {
        Reference< XAccessibleContext > xAccessibleContext = rxAccessible->getAccessibleContext();
        if( xAccessibleContext.is() ) {
            return [ AquaA11yFactory wrapperForAccessibleContext: xAccessibleContext ];
        }
    }
    return nil;
}

+(AquaA11yWrapper *)wrapperForAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    return [ AquaA11yFactory wrapperForAccessibleContext: rxAccessibleContext createIfNotExists: YES asRadioGroup: NO ];
}

+(AquaA11yWrapper *)wrapperForAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext createIfNotExists:(BOOL) bCreate {
    return [ AquaA11yFactory wrapperForAccessibleContext: rxAccessibleContext createIfNotExists: bCreate asRadioGroup: NO ];
}

+(AquaA11yWrapper *)wrapperForAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext createIfNotExists:(BOOL) bCreate asRadioGroup:(BOOL) asRadioGroup{
    NSMutableDictionary * dAllWrapper = [ AquaA11yFactory allWrapper ];
    NSValue * nKey = nil;
    if ( asRadioGroup ) {
        nKey = [ AquaA11yFactory keyForAccessibleContextAsRadioGroup: rxAccessibleContext ];
    } else {
        nKey = [ AquaA11yFactory keyForAccessibleContext: rxAccessibleContext ];
    }
    AquaA11yWrapper * aWrapper = (AquaA11yWrapper *) [ dAllWrapper objectForKey: nKey ];
    if ( aWrapper != nil ) {
        [ aWrapper retain ];
    } else if ( bCreate ) {
        NSString * nativeRole = [ AquaA11yRoleHelper getNativeRoleFrom: rxAccessibleContext.get() ];
        // TODO: reflection
        if ( [ nativeRole isEqualToString: NSAccessibilityButtonRole ] ) {
            aWrapper = [ [ AquaA11yWrapperButton alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityTextAreaRole ] ) {
            aWrapper = [ [ AquaA11yWrapperTextArea alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityStaticTextRole ] ) {
            aWrapper = [ [ AquaA11yWrapperStaticText alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityComboBoxRole ] ) {
            aWrapper = [ [ AquaA11yWrapperComboBox alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityGroupRole ] ) {
            aWrapper = [ [ AquaA11yWrapperGroup alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityToolbarRole ] ) {
            aWrapper = [ [ AquaA11yWrapperToolbar alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityScrollAreaRole ] ) {
            aWrapper = [ [ AquaA11yWrapperScrollArea alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityTabGroupRole ] ) {
            aWrapper = [ [ AquaA11yWrapperTabGroup alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityScrollBarRole ] ) {
            aWrapper = [ [ AquaA11yWrapperScrollBar alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityCheckBoxRole ] ) {
            aWrapper = [ [ AquaA11yWrapperCheckBox alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityRadioGroupRole ] ) {
            aWrapper = [ [ AquaA11yWrapperRadioGroup alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityRadioButtonRole ] ) {
            aWrapper = [ [ AquaA11yWrapperRadioButton alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityRowRole ] ) {
            aWrapper = [ [ AquaA11yWrapperRow alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityListRole ] ) {
            aWrapper = [ [ AquaA11yWrapperList alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilitySplitterRole ] ) {
            aWrapper = [ [ AquaA11yWrapperSplitter alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else if ( [ nativeRole isEqualToString: NSAccessibilityTableRole ] ) {
            aWrapper = [ [ AquaA11yTableWrapper alloc ] initWithAccessibleContext: rxAccessibleContext ];
        } else {
            aWrapper = [ [ AquaA11yWrapper alloc ] initWithAccessibleContext: rxAccessibleContext ];
        }
        [ nativeRole release ];
        [ aWrapper setActsAsRadioGroup: asRadioGroup ];
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
            [ dAllWrapper setObject: aWrapper forKey: nKey ];
            /* fdo#67410: Accessibility notifications are not delivered on NSView subclasses that do not
               "reasonably" participate in NSView hierarchy (perhaps the only important point is
               that the view is a transitive subview of the NSWindow's content view, but I
               did not try to verify that).

               So let the superview-subviews relationship mirror the AXParent-AXChildren relationship.
            */
            id parent = [aWrapper accessibilityAttributeValue:NSAccessibilityParentAttribute];
            if (parent) {
                if ([parent isKindOfClass:[NSView class]]) {
                    // SAL_DEBUG("Wrapper INIT: " << [[aWrapper description] UTF8String] << " ==> " << [[parent description] UTF8String]);
                    NSView *parentView = (NSView *)parent;
                    [parentView addSubview:aWrapper positioned:NSWindowBelow relativeTo:nil];
                } else if ([parent isKindOfClass:NSClassFromString(@"SalFrameWindow")]) {
                    NSWindow *window = (NSWindow *)parent;
                    NSView *salView = [window contentView];
                    // SAL_DEBUG("Wrapper INIT SAL: " << [[aWrapper description] UTF8String] << " ==> " << [[salView description] UTF8String]);
                    [salView addSubview:aWrapper positioned:NSWindowBelow relativeTo:nil];
                } else {
                    // SAL_DEBUG("Wrapper INIT: !! " << [[aWrapper description] UTF8String] << " !==>! " << [[parent description] UTF8String] << "!!");
                }
            } else {
                // SAL_DEBUG("Wrapper INIT: " << [[aWrapper description] UTF8String] << " ==> NO PARENT");
            }
        }
    }
    return aWrapper;
}

+(void)insertIntoWrapperRepository: (NSView *) viewElement forAccessibleContext: (Reference < XAccessibleContext >) rxAccessibleContext {
    NSMutableDictionary * dAllWrapper = [ AquaA11yFactory allWrapper ];
    [ dAllWrapper setObject: viewElement forKey: [ AquaA11yFactory keyForAccessibleContext: rxAccessibleContext ] ];
}

+(void)removeFromWrapperRepositoryFor: (::com::sun::star::uno::Reference < ::com::sun::star::accessibility::XAccessibleContext >) rxAccessibleContext {
    // TODO: when RADIO_BUTTON search for associated RadioGroup-wrapper and delete that as well
    AquaA11yWrapper * theWrapper = [ AquaA11yFactory wrapperForAccessibleContext: rxAccessibleContext createIfNotExists: NO ];
    if ( theWrapper != nil ) {
        if (![theWrapper isKindOfClass:NSClassFromString(@"SalFrameView")]) {
            [theWrapper removeFromSuperview];
        }
        [ [ AquaA11yFactory allWrapper ] removeObjectForKey: [ AquaA11yFactory keyForAccessibleContext: rxAccessibleContext ] ];
        [ theWrapper release ];
    }
}

+(void)registerView: (NSView *) theView {
    if ( enabled && [ theView isKindOfClass: [ AquaA11yWrapper class ] ] ) {
        // insertIntoWrapperRepository gets called from SalFrameView itself to bootstrap the bridge initially
        [ (AquaA11yWrapper *) theView accessibleContext ];
    }
}

+(void)revokeView: (NSView *) theView {
    if ( enabled && [ theView isKindOfClass: [ AquaA11yWrapper class ] ] ) {
        [ AquaA11yFactory removeFromWrapperRepositoryFor: [ (AquaA11yWrapper *) theView accessibleContext ] ];
    }
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
