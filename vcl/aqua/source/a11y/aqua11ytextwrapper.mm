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

#include "salinst.h"
#include "aqua11ytextwrapper.h"
#include "aqua11ytextattributeswrapper.h"
#include "aqua11yutil.h"
#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/awt/Rectangle.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

// Wrapper for XAccessibleText, XAccessibleEditableText and XAccessibleMultiLineText

@implementation AquaA11yTextWrapper : NSObject

+(id)valueAttributeForElement:(AquaA11yWrapper *)wrapper {
    return CreateNSString ( [ wrapper accessibleText ] -> getText() );
}

+(void)setValueAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value {
    // TODO
}

+(id)numberOfCharactersAttributeForElement:(AquaA11yWrapper *)wrapper {
    return [ NSNumber numberWithLong: [ wrapper accessibleText ] -> getCharacterCount() ];
}

+(id)selectedTextAttributeForElement:(AquaA11yWrapper *)wrapper {
    return CreateNSString ( [ wrapper accessibleText ] -> getSelectedText() );
}

+(void)setSelectedTextAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value {
    if ( [ wrapper accessibleEditableText ] != nil ) {
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

+(void)setVisibleCharacterRangeAttributeForElement:(AquaA11yWrapper *)wrapper to:(id)value {
    // do nothing
}

+(id)sharedTextUIElementsAttributeForElement:(AquaA11yWrapper *)wrapper {
    return [ [ NSArray alloc ] init ]; // unsupported
}

+(id)sharedCharacterRangeAttributeForElement:(AquaA11yWrapper *)wrapper {
    return [ NSValue valueWithRange: NSMakeRange ( 0, 0 ) ]; // unsupported
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
    sal_Int32 index = [ wrapper accessibleText ] -> getIndexAtPoint ( [ AquaA11yUtil nsPointToVclPoint: point ] );
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
        if ( [ wrapper accessibleComponent ] != nil ) {
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
            rtfData = [ attrString RTFFromRange: [ range rangeValue ] documentAttributes: nil ];
        } @catch ( NSException * e) {
            // emtpy
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
