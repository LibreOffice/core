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


#include "osx/salinst.h"
#include "quartz/utils.h"
#include "a11ytextwrapper.h"
#include "a11ytextattributeswrapper.h"
#include "a11yutil.h"

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

// Wrapper for XAccessibleText, XAccessibleEditableText and XAccessibleMultiLineText

@implementation AquaA11yTextWrapper : NSObject

+(id)valueAttributeForElement:(AquaA11yWrapper *)wrapper {
    return CreateNSString ( [ wrapper accessibleText ] -> getText() );
}

+(void)setValueAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value
{
    // TODO
    (void)wrapper;
    (void)value;
}

+(id)numberOfCharactersAttributeForElement:(AquaA11yWrapper *)wrapper {
    return [ NSNumber numberWithLong: [ wrapper accessibleText ] -> getCharacterCount() ];
}

+(id)selectedTextAttributeForElement:(AquaA11yWrapper *)wrapper {
    return CreateNSString ( [ wrapper accessibleText ] -> getSelectedText() );
}

+(void)setSelectedTextAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value {
    if ( [ wrapper accessibleEditableText ] ) {
        NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
        OUString newText = GetOUString ( (NSString *) value );
        NSRange selectedTextRange = [ [ AquaA11yTextWrapper selectedTextRangeAttributeForElement: wrapper ] rangeValue ];
        try {
            [ wrapper accessibleEditableText ] -> replaceText ( selectedTextRange.location, selectedTextRange.location + selectedTextRange.length, newText );
        } catch ( const Exception & e ) {
            // empty
        }
        [ pool release ];
    }
}

+(id)selectedTextRangeAttributeForElement:(AquaA11yWrapper *)wrapper {
    sal_Int32 start = [ wrapper accessibleText ] -> getSelectionStart();
    sal_Int32 end = [ wrapper accessibleText ] -> getSelectionEnd();
    if ( start != end ) {
        return [ NSValue valueWithRange: NSMakeRange ( start, end - start ) ]; // true selection
    } else {
        long caretPos = [ wrapper accessibleText ] -> getCaretPosition();
        if ( caretPos < 0 || caretPos > [ wrapper accessibleText ] -> getCharacterCount() ) {
            return nil;
        }
        return [ NSValue valueWithRange: NSMakeRange ( caretPos, 0 ) ]; // insertion point
    }
}

+(void)setSelectedTextRangeAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value {
    NSRange range = [ value rangeValue ];
    try {
        [ wrapper accessibleText ] -> setSelection ( range.location, range.location + range.length );
    } catch ( const Exception & e ) {
        // empty
    }
}

+(id)visibleCharacterRangeAttributeForElement:(AquaA11yWrapper *)wrapper {
    // the OOo a11y API returns only the visible portion...
    return [ NSValue valueWithRange: NSMakeRange ( 0, [ wrapper accessibleText ] -> getCharacterCount() ) ];
}

+(void)setVisibleCharacterRangeAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value
{
    // do nothing
    (void)wrapper;
    (void)value;
}

+(id)sharedTextUIElementsAttributeForElement:(AquaA11yWrapper *)wrapper
{
    (void)wrapper;
    return [NSArray arrayWithObject:wrapper];
}

+(id)sharedCharacterRangeAttributeForElement:(AquaA11yWrapper *)wrapper
{
    (void)wrapper;
    return [ NSValue valueWithRange: NSMakeRange ( 0, [wrapper accessibleText]->getCharacterCount() ) ];
}

+(void)addAttributeNamesTo:(NSMutableArray *)attributeNames {
    [ attributeNames addObjectsFromArray: [ AquaA11yTextWrapper specialAttributeNames ] ];
}

+(NSArray *)specialAttributeNames {
    return [ NSArray arrayWithObjects: 
            NSAccessibilityValueAttribute, 
            NSAccessibilityNumberOfCharactersAttribute, 
            NSAccessibilitySelectedTextAttribute, 
            NSAccessibilitySelectedTextRangeAttribute, 
            NSAccessibilityVisibleCharacterRangeAttribute, 
            NSAccessibilitySharedTextUIElementsAttribute, 
            NSAccessibilitySharedCharacterRangeAttribute, 
            nil ];
}

+(void)addParameterizedAttributeNamesTo:(NSMutableArray *)attributeNames {
    [ attributeNames addObjectsFromArray: [ AquaA11yTextWrapper specialParameterizedAttributeNames ] ];
}

+(NSArray *)specialParameterizedAttributeNames {
    return [ NSArray arrayWithObjects: 
            NSAccessibilityStringForRangeParameterizedAttribute, 
            NSAccessibilityAttributedStringForRangeParameterizedAttribute, 
            NSAccessibilityRangeForIndexParameterizedAttribute, 
            NSAccessibilityRangeForPositionParameterizedAttribute, 
            NSAccessibilityBoundsForRangeParameterizedAttribute, 
            NSAccessibilityStyleRangeForIndexParameterizedAttribute, 
            NSAccessibilityRTFForRangeParameterizedAttribute, 
            NSAccessibilityLineForIndexParameterizedAttribute, 
            NSAccessibilityRangeForLineParameterizedAttribute, 
            nil ];
}

+(id)lineForIndexAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)index {
    NSNumber * lineNumber = nil;
    try {
        sal_Int32 line = [ wrapper accessibleMultiLineText ] -> getLineNumberAtIndex ( (sal_Int32) [ index intValue ] );
        lineNumber = [ NSNumber numberWithInt: line ];
    } catch ( IndexOutOfBoundsException & e ) {
        // empty
    }
    return lineNumber;
}

+(id)rangeForLineAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)line {
    NSValue * range = nil;
    try {
        TextSegment textSegment = [ wrapper accessibleMultiLineText ] -> getTextAtLineNumber ( [ line intValue ] );
        range = [ NSValue valueWithRange: NSMakeRange ( textSegment.SegmentStart, textSegment.SegmentEnd - textSegment.SegmentStart ) ];
    } catch ( IndexOutOfBoundsException & e ) {
        // empty
    }
    return range;
}

+(id)stringForRangeAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)range {
    int loc = [ range rangeValue ].location;
    int len = [ range rangeValue ].length;
    NSMutableString * textRange = [ [ NSMutableString alloc ] init ];
    try {
        [ textRange appendString: CreateNSString ( [ wrapper accessibleText ] -> getTextRange ( loc, loc + len ) ) ];
    } catch ( IndexOutOfBoundsException & e ) {
        // empty
    }
    return textRange;
}

+(id)attributedStringForRangeAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)range {
    return [ AquaA11yTextAttributesWrapper createAttributedStringForElement: wrapper inOrigRange: range ];
}

+(id)rangeForIndexAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)index {
    NSValue * range = nil;
    try {
        TextSegment textSegment = [ wrapper accessibleText ] -> getTextBeforeIndex ( [ index intValue ], AccessibleTextType::GLYPH );
        range = [ NSValue valueWithRange: NSMakeRange ( textSegment.SegmentStart, textSegment.SegmentEnd - textSegment.SegmentStart ) ];
    } catch ( IndexOutOfBoundsException & e ) {
        // empty
    } catch ( IllegalArgumentException & e ) {
        // empty
    }
    return range;
}

+(id)rangeForPositionAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)point {
    NSValue * value = nil;
    Point aPoint( [ AquaA11yUtil nsPointToVclPoint: point ]);
    const Point screenPos = [ wrapper accessibleComponent ] -> getLocationOnScreen();
    aPoint.X -= screenPos.X;
    aPoint.Y -= screenPos.Y;
    sal_Int32 index = [ wrapper accessibleText ] -> getIndexAtPoint( aPoint );
    if ( index > -1 ) {
        value = [ AquaA11yTextWrapper rangeForIndexAttributeForElement: wrapper forParameter: [ NSNumber numberWithLong: index ] ];
    }
    return value;
}

+(id)boundsForRangeAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)range {
    NSValue * rect = nil;
    try {
        // TODO: this is ugly!!!
        // the UNP-API can only return the bounds for a single character, not for a range
        int loc = [ range rangeValue ].location;
        int len = [ range rangeValue ].length;
        int minx = 0x7fffffff, miny = 0x7fffffff, maxx = 0, maxy = 0;
        for ( int i = 0; i < len; i++ ) {
            Rectangle vclRect = [ wrapper accessibleText ] -> getCharacterBounds ( loc + i );
            if ( vclRect.X < minx ) {
                minx = vclRect.X;
            }
            if ( vclRect.Y < miny ) {
                miny = vclRect.Y;
            }
            if ( vclRect.Width + vclRect.X > maxx ) {
                maxx = vclRect.Width + vclRect.X;
            }
            if ( vclRect.Height + vclRect.Y > maxy ) {
                maxy = vclRect.Height + vclRect.Y;
            }
        }
        if ( [ wrapper accessibleComponent ] ) {
            // get location on screen (must be added since get CharacterBounds returns values relative to parent)
            Point screenPos = [ wrapper accessibleComponent ] -> getLocationOnScreen();
            Point pos ( minx + screenPos.X, miny + screenPos.Y );
            Point size ( maxx - minx, maxy - miny );
            NSValue * nsPos = [ AquaA11yUtil vclPointToNSPoint: pos ];
            rect = [ NSValue valueWithRect: NSMakeRect ( [ nsPos pointValue ].x, [ nsPos pointValue ].y - size.Y, size.X, size.Y ) ];
            //printf("Range: %s --- Rect: %s\n", [ NSStringFromRange ( [ range rangeValue ] ) UTF8String ], [ NSStringFromRect ( [ rect rectValue ] ) UTF8String ]);
        }
    } catch ( IndexOutOfBoundsException & e ) {
        // empty
    }
    return rect;
}

+(id)styleRangeForIndexAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)index {
    NSValue * range = nil;
    try {
        TextSegment textSegment = [ wrapper accessibleText ] -> getTextAtIndex ( [ index intValue ], AccessibleTextType::ATTRIBUTE_RUN );
        range = [ NSValue valueWithRange: NSMakeRange ( textSegment.SegmentStart, textSegment.SegmentEnd - textSegment.SegmentStart ) ];
    } catch ( IndexOutOfBoundsException & e ) {
        // empty
    } catch ( IllegalArgumentException & e ) {
        // empty
    }
    return range;
}

+(id)rTFForRangeAttributeForElement:(AquaA11yWrapper *)wrapper forParameter:(id)range {
    NSData * rtfData = nil;
    NSAttributedString * attrString = (NSAttributedString *) [ AquaA11yTextWrapper attributedStringForRangeAttributeForElement: wrapper forParameter: range ];
    if ( attrString != nil ) {
        @try {
            rtfData = [ attrString RTFFromRange: [ range rangeValue ] documentAttributes: @{NSDocumentTypeDocumentAttribute : NSRTFTextDocumentType} ];
        } @catch ( NSException * e) {
            // empty
        }
    }
    return rtfData;
}

+(BOOL)isAttributeSettable:(NSString *)attribute forElement:(AquaA11yWrapper *)wrapper {
    BOOL isSettable = NO;
    if ( [ attribute isEqualToString: NSAccessibilityValueAttribute ]
      || [ attribute isEqualToString: NSAccessibilitySelectedTextAttribute ]
      || [ attribute isEqualToString: NSAccessibilitySelectedTextRangeAttribute ]
      || [ attribute isEqualToString: NSAccessibilityVisibleCharacterRangeAttribute ] ) {
        if ( ! [ [ wrapper accessibilityAttributeValue: NSAccessibilityRoleAttribute ] isEqualToString: NSAccessibilityStaticTextRole ] ) {
            isSettable = YES;
        }
    }
    return isSettable;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
