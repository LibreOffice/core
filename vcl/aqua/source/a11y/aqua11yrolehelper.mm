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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include "aqua/aqua11yfactory.h"

#include "aqua11yrolehelper.h"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;

@implementation AquaA11yRoleHelper

+(id)simpleMapNativeRoleFrom: (XAccessibleContext *) accessibleContext {
    id nativeRole = nil;
    switch( accessibleContext -> getAccessibleRole() ) {
#define MAP(a,b) \
        case a: nativeRole = b; break
            
        MAP( AccessibleRole::UNKNOWN, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::ALERT, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::COLUMN_HEADER, NSAccessibilityColumnRole );
        MAP( AccessibleRole::CANVAS, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::CHECK_BOX, NSAccessibilityCheckBoxRole );
        MAP( AccessibleRole::CHECK_MENU_ITEM, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::COLOR_CHOOSER, NSAccessibilityColorWellRole ); // FIXME
        MAP( AccessibleRole::COMBO_BOX, NSAccessibilityComboBoxRole );
        MAP( AccessibleRole::DATE_EDITOR, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::DESKTOP_ICON, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::DESKTOP_PANE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::DIRECTORY_PANE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::DIALOG, NSAccessibilityGroupRole );
        MAP( AccessibleRole::DOCUMENT, NSAccessibilityGroupRole );
        MAP( AccessibleRole::EMBEDDED_OBJECT, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::END_NOTE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::FILE_CHOOSER, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::FILLER, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::FONT_CHOOSER, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::FOOTER, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::FOOTNOTE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::FRAME, NSAccessibilityWindowRole );
        MAP( AccessibleRole::GLASS_PANE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::GRAPHIC, NSAccessibilityImageRole );
        MAP( AccessibleRole::GROUP_BOX, NSAccessibilityGroupRole );
        MAP( AccessibleRole::HEADER, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::HEADING, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::HYPER_LINK, NSAccessibilityLinkRole );
        MAP( AccessibleRole::ICON, NSAccessibilityImageRole );
        MAP( AccessibleRole::INTERNAL_FRAME, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::LABEL, NSAccessibilityStaticTextRole );
        MAP( AccessibleRole::LAYERED_PANE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::LIST, NSAccessibilityMenuRole );
        MAP( AccessibleRole::LIST_ITEM, NSAccessibilityMenuItemRole );
        MAP( AccessibleRole::MENU, NSAccessibilityMenuRole );
        MAP( AccessibleRole::MENU_BAR, NSAccessibilityMenuBarRole );
        MAP( AccessibleRole::MENU_ITEM, NSAccessibilityMenuItemRole );
        MAP( AccessibleRole::OPTION_PANE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::PAGE_TAB, NSAccessibilityButtonRole );
        MAP( AccessibleRole::PAGE_TAB_LIST, NSAccessibilityTabGroupRole );
        MAP( AccessibleRole::PANEL, NSAccessibilityGroupRole );
        MAP( AccessibleRole::PARAGRAPH, NSAccessibilityTextAreaRole );
        MAP( AccessibleRole::PASSWORD_TEXT, NSAccessibilityTextFieldRole );
        MAP( AccessibleRole::POPUP_MENU, NSAccessibilityMenuRole );
        MAP( AccessibleRole::PUSH_BUTTON, NSAccessibilityButtonRole );
        MAP( AccessibleRole::PROGRESS_BAR, NSAccessibilityProgressIndicatorRole );
        MAP( AccessibleRole::RADIO_BUTTON, NSAccessibilityRadioButtonRole );
        MAP( AccessibleRole::RADIO_MENU_ITEM, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::ROW_HEADER, NSAccessibilityRowRole );
        MAP( AccessibleRole::ROOT_PANE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::SCROLL_BAR, NSAccessibilityScrollBarRole );
        MAP( AccessibleRole::SCROLL_PANE, NSAccessibilityScrollAreaRole );
        MAP( AccessibleRole::SHAPE, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::SEPARATOR, NSAccessibilitySplitterRole ); // FIXME
        MAP( AccessibleRole::SLIDER, NSAccessibilitySliderRole );
        MAP( AccessibleRole::SPIN_BOX, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::SPLIT_PANE, NSAccessibilitySplitterRole );
        MAP( AccessibleRole::STATUS_BAR, NSAccessibilityGroupRole ); // FIXME
        MAP( AccessibleRole::TABLE, NSAccessibilityTableRole );
        MAP( AccessibleRole::TABLE_CELL, NSAccessibilityTextFieldRole );
        MAP( AccessibleRole::TEXT, NSAccessibilityTextAreaRole );
        MAP( AccessibleRole::TEXT_FRAME, NSAccessibilityGroupRole );
        MAP( AccessibleRole::TOGGLE_BUTTON, NSAccessibilityCheckBoxRole );
        MAP( AccessibleRole::TOOL_BAR, NSAccessibilityToolbarRole );
        MAP( AccessibleRole::TOOL_TIP, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::TREE, NSAccessibilityGroupRole );
        MAP( AccessibleRole::VIEW_PORT, NSAccessibilityUnknownRole ); // FIXME
        MAP( AccessibleRole::WINDOW, NSAccessibilityWindowRole );

        MAP( AccessibleRole::BUTTON_DROPDOWN, NSAccessibilityMenuButtonRole );
        MAP( AccessibleRole::BUTTON_MENU, NSAccessibilityMenuButtonRole );
        MAP( AccessibleRole::CAPTION, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::CHART, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::FORM, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::IMAGE_MAP, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::NOTE, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::PAGE, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::RULER, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::SECTION, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::TREE_ITEM, NSAccessibilityUnknownRole );
        MAP( AccessibleRole::TREE_TABLE, NSAccessibilityUnknownRole );

#undef MAP
        default:
            break;
    }
    return nativeRole;
}

+(id)getNativeRoleFrom: (XAccessibleContext *) accessibleContext {
    id nativeRole = [ AquaA11yRoleHelper simpleMapNativeRoleFrom: accessibleContext ];
    if ( accessibleContext -> getAccessibleRole() == AccessibleRole::LABEL ) {
        if ( accessibleContext -> getAccessibleChildCount() > 0 ) {
            [ nativeRole release ];
            nativeRole = NSAccessibilityOutlineRole;
        } else if ( accessibleContext -> getAccessibleParent().is() ) {
            Reference < XAccessibleContext > rxParentContext = accessibleContext -> getAccessibleParent() -> getAccessibleContext();
            if ( rxParentContext.is() ) {
                NSString * roleParent = (NSString *) [ AquaA11yRoleHelper simpleMapNativeRoleFrom: rxParentContext.get() ];
                if ( [ roleParent isEqualToString: NSAccessibilityOutlineRole ] ) {
                    [ nativeRole release ];
                    nativeRole = NSAccessibilityRowRole;
                }
                [ roleParent release ];
            }
        }
    } else if ( accessibleContext -> getAccessibleRole() == AccessibleRole::COMBO_BOX ) {
        Reference < XAccessible > rxAccessible = accessibleContext -> getAccessibleChild(0);
        if ( rxAccessible.is() ) {
            Reference < XAccessibleContext > rxAccessibleContext = rxAccessible -> getAccessibleContext();
            if ( rxAccessibleContext.is() && rxAccessibleContext -> getAccessibleRole() == AccessibleRole::TEXT ) {
                if ( ! rxAccessibleContext -> getAccessibleStateSet() -> contains ( AccessibleStateType::EDITABLE ) ) {
                    [ nativeRole release ];
                    nativeRole = NSAccessibilityPopUpButtonRole;
                }
            }
        }
    }
    return nativeRole;
}

+(id)getNativeSubroleFrom: (sal_Int16) nRole {
    id nativeSubrole = nil;
    switch( nRole ) {
#define MAP(a,b) \
        case a: nativeSubrole = b; break

        MAP( AccessibleRole::UNKNOWN, NSAccessibilityUnknownSubrole );
        MAP( AccessibleRole::ALERT, NSAccessibilitySystemDialogSubrole );
        MAP( AccessibleRole::COLUMN_HEADER, @"" );
        MAP( AccessibleRole::CANVAS, @"" );
        MAP( AccessibleRole::CHECK_BOX, @"" );
        MAP( AccessibleRole::CHECK_MENU_ITEM, @"" );
        MAP( AccessibleRole::COLOR_CHOOSER, @"" );
        MAP( AccessibleRole::COMBO_BOX, @"" );
        MAP( AccessibleRole::DATE_EDITOR, @"" );
        MAP( AccessibleRole::DESKTOP_ICON, @"" );
        MAP( AccessibleRole::DESKTOP_PANE, @"" );
        MAP( AccessibleRole::DIRECTORY_PANE, @"" );
        MAP( AccessibleRole::DIALOG, NSAccessibilityDialogSubrole );
        MAP( AccessibleRole::DOCUMENT, @"" );
        MAP( AccessibleRole::EMBEDDED_OBJECT, @"" );
        MAP( AccessibleRole::END_NOTE, @"" );
        MAP( AccessibleRole::FILE_CHOOSER, @"" );
        MAP( AccessibleRole::FILLER, @"" );
        MAP( AccessibleRole::FONT_CHOOSER, @"" );
        MAP( AccessibleRole::FOOTER, @"" );
        MAP( AccessibleRole::FOOTNOTE, @"" );
        MAP( AccessibleRole::FRAME, @"" );
        MAP( AccessibleRole::GLASS_PANE, @"" );
        MAP( AccessibleRole::GRAPHIC, @"" );
        MAP( AccessibleRole::GROUP_BOX, @"" );
        MAP( AccessibleRole::HEADER, @"" );
        MAP( AccessibleRole::HEADING, @"" );
        MAP( AccessibleRole::HYPER_LINK, NSAccessibilityTextLinkSubrole );
        MAP( AccessibleRole::ICON, @"" );
        MAP( AccessibleRole::INTERNAL_FRAME, @"" );
        MAP( AccessibleRole::LABEL, @"" );
        MAP( AccessibleRole::LAYERED_PANE, @"" );
        MAP( AccessibleRole::LIST, @"" );
        MAP( AccessibleRole::LIST_ITEM, NSAccessibilityOutlineRowSubrole );
        MAP( AccessibleRole::MENU, @"" );
        MAP( AccessibleRole::MENU_BAR, @"" );
        MAP( AccessibleRole::MENU_ITEM, @"" );
        MAP( AccessibleRole::OPTION_PANE, @"" );
        MAP( AccessibleRole::PAGE_TAB, @"" );
        MAP( AccessibleRole::PAGE_TAB_LIST, @"" );
        MAP( AccessibleRole::PANEL, @"" );
        MAP( AccessibleRole::PARAGRAPH, @"" );
        MAP( AccessibleRole::PASSWORD_TEXT, NSAccessibilitySecureTextFieldSubrole );
        MAP( AccessibleRole::POPUP_MENU, @"" );
        MAP( AccessibleRole::PUSH_BUTTON, @"" );
        MAP( AccessibleRole::PROGRESS_BAR, @"" );
        MAP( AccessibleRole::RADIO_BUTTON, @"" );
        MAP( AccessibleRole::RADIO_MENU_ITEM, @"" );
        MAP( AccessibleRole::ROW_HEADER, @"" );
        MAP( AccessibleRole::ROOT_PANE, @"" );
        MAP( AccessibleRole::SCROLL_BAR, @"" );
        MAP( AccessibleRole::SCROLL_PANE, @"" );
        MAP( AccessibleRole::SHAPE, @"" );
        MAP( AccessibleRole::SEPARATOR, @"" );
        MAP( AccessibleRole::SLIDER, @"" );
        MAP( AccessibleRole::SPIN_BOX, @"" );
        MAP( AccessibleRole::SPLIT_PANE, @"" );
        MAP( AccessibleRole::STATUS_BAR, @"" );
        MAP( AccessibleRole::TABLE, @"" );
        MAP( AccessibleRole::TABLE_CELL, @"" );
        MAP( AccessibleRole::TEXT, @"" );
        MAP( AccessibleRole::TEXT_FRAME, @"" );
        MAP( AccessibleRole::TOGGLE_BUTTON, @"" );
        MAP( AccessibleRole::TOOL_BAR, @"" );
        MAP( AccessibleRole::TOOL_TIP, @"" );
        MAP( AccessibleRole::TREE, @"" );
        MAP( AccessibleRole::VIEW_PORT, @"" );
        MAP( AccessibleRole::WINDOW, NSAccessibilityStandardWindowSubrole );

        MAP( AccessibleRole::BUTTON_DROPDOWN, @"" );
        MAP( AccessibleRole::BUTTON_MENU, @"" );
        MAP( AccessibleRole::CAPTION, @"" );
        MAP( AccessibleRole::CHART, @"" );
        MAP( AccessibleRole::FORM, @"" );
        MAP( AccessibleRole::IMAGE_MAP, @"" );
        MAP( AccessibleRole::NOTE, @"" );
        MAP( AccessibleRole::PAGE, @"" );
        MAP( AccessibleRole::RULER, @"" );
        MAP( AccessibleRole::SECTION, @"" );
        MAP( AccessibleRole::TREE_ITEM, @"" );
        MAP( AccessibleRole::TREE_TABLE, @"" );

#undef MAP
        default:
            break;
    }
    return nativeSubrole;
}

+(id)getRoleDescriptionFrom: (NSString *) role with: (NSString *) subRole {
	id roleDescription;
	if ( [ subRole length ] == 0 )
		roleDescription = NSAccessibilityRoleDescription( role, nil );
	else
		roleDescription = NSAccessibilityRoleDescription( role, subRole );
    return roleDescription;
}

@end
