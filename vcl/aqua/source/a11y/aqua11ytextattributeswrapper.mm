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


#include "aqua/salinst.h"

#include "aqua11ytextattributeswrapper.h"

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

@implementation AquaA11yTextAttributesWrapper : NSObject

+(int)convertUnderlineStyle:(PropertyValue)property {
    int underlineStyle = NSNoUnderlineStyle;
    sal_Int16 value = 0;
    property.Value >>= value;
    if ( value != FontUnderline::NONE 
      && value != FontUnderline::DONTKNOW) {
        underlineStyle = NSSingleUnderlineStyle;
    }
    return underlineStyle;
}

+(int)convertBoldStyle:(PropertyValue)property {
    int boldStyle = 0;
    float value = 0;
    property.Value >>= value;
    if ( value == FontWeight::SEMIBOLD
      || value == FontWeight::BOLD
      || value == FontWeight::ULTRABOLD
      || value == FontWeight::BLACK ) {
        boldStyle = NSBoldFontMask;
    }
    return boldStyle;
}

+(int)convertItalicStyle:(PropertyValue)property {
    int italicStyle = 0;
    sal_Int16 value = property.Value.get<FontSlant>();
    if ( value == FontSlant_ITALIC ) {
        italicStyle = NSItalicFontMask;
    }
    return italicStyle;
}

+(BOOL)isStrikethrough:(PropertyValue)property {
    BOOL strikethrough = NO;
    sal_Int16 value = 0;
    property.Value >>= value;
    if ( value != FontStrikeout::NONE
      && value != FontStrikeout::DONTKNOW ) {
        strikethrough = YES;
    }
    return strikethrough;
}

+(BOOL)convertBoolean:(PropertyValue)property {
    BOOL myBoolean = NO;
    bool value = sal_False;
    property.Value >>= value;
    if ( value ) {
        myBoolean = YES;
    }
    return myBoolean;
}

+(NSNumber *)convertShort:(PropertyValue)property {
    sal_Int16 value = 0;
    property.Value >>= value;
    return [ NSNumber numberWithShort: value ];
}

+(void)addColor:(sal_Int32)salColor forAttribute:(NSString *)attribute andRange:(NSRange)range toString:(NSMutableAttributedString *)string {
    if ( salColor != -1 ) {
        CGFloat elements[] = { salColor & 0x00ff0000, salColor & 0x0000ff00, salColor & 0x000000ff };
        CGColorRef color = CGColorCreate ( CGColorSpaceCreateWithName ( kCGColorSpaceGenericRGB ), elements );
        [ string addAttribute: attribute value: (id) color range: range ];
        CGColorRelease ( color );
    }
}

+(void)addFont:(NSFont *)font toString:(NSMutableAttributedString *)string forRange:(NSRange)range {
    if ( font != nil ) {
        NSDictionary * fontDictionary = [ NSDictionary dictionaryWithObjectsAndKeys:
            [ font fontName ], NSAccessibilityFontNameKey,
            [ font familyName ], NSAccessibilityFontFamilyKey,
            [ font displayName ], NSAccessibilityVisibleNameKey,
            [ NSNumber numberWithFloat: [ font pointSize ] ], NSAccessibilityFontSizeKey, 
            nil
        ];
        [ string addAttribute: NSAccessibilityFontTextAttribute 
                value: fontDictionary 
                range: range
        ];
    }
}

+(void)applyAttributesFrom:(Sequence < PropertyValue >)attributes toString:(NSMutableAttributedString *)string forRange:(NSRange)range storeDefaultsTo:(AquaA11yWrapper *)wrapperStore getDefaultsFrom:(AquaA11yWrapper *)wrapper {
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    // constants
    static const OUString attrUnderline(RTL_CONSTASCII_USTRINGPARAM("CharUnderline"));
    static const OUString attrBold(RTL_CONSTASCII_USTRINGPARAM("CharWeight"));
    static const OUString attrFontname(RTL_CONSTASCII_USTRINGPARAM("CharFontName"));
    static const OUString attrItalic(RTL_CONSTASCII_USTRINGPARAM("CharPosture"));
    static const OUString attrHeight(RTL_CONSTASCII_USTRINGPARAM("CharHeight"));
    static const OUString attrStrikethrough(RTL_CONSTASCII_USTRINGPARAM("CharStrikeout"));
    static const OUString attrShadow(RTL_CONSTASCII_USTRINGPARAM("CharShadowed"));
    static const OUString attrUnderlineColor(RTL_CONSTASCII_USTRINGPARAM("CharUnderlineColor"));
    static const OUString attrUnderlineHasColor(RTL_CONSTASCII_USTRINGPARAM("CharUnderlineHasColor"));
    static const OUString attrForegroundColor(RTL_CONSTASCII_USTRINGPARAM("CharColor"));
    static const OUString attrBackgroundColor(RTL_CONSTASCII_USTRINGPARAM("CharBackColor"));
    static const OUString attrSuperscript(RTL_CONSTASCII_USTRINGPARAM("CharEscapement"));
    // vars
    OUString fontname;
    int fonttraits = 0;
    float fontsize = 0.0;
    sal_Int32 underlineColor = 0;
    BOOL underlineHasColor = NO;
    // add attributes to string
    for ( int attrIndex = 0; attrIndex < attributes.getLength(); attrIndex++ ) {
        PropertyValue property = attributes [ attrIndex ];
        // TODO: NSAccessibilityMisspelledTextAttribute, NSAccessibilityAttachmentTextAttribute, NSAccessibilityLinkTextAttribute
        // NSAccessibilityStrikethroughColorTextAttribute is unsupported by UNP-API
        if ( property.Value.hasValue() ) {
            if ( property.Name.equals ( attrUnderline ) ) {
                int style = [ AquaA11yTextAttributesWrapper convertUnderlineStyle: property ];
                if ( style != NSNoUnderlineStyle ) {
                    [ string addAttribute: NSAccessibilityUnderlineTextAttribute value: [ NSNumber numberWithInt: style ] range: range ];
                }
            } else if ( property.Name.equals ( attrFontname ) ) {
                property.Value >>= fontname;
            } else if ( property.Name.equals ( attrBold ) ) {
                fonttraits |= [ AquaA11yTextAttributesWrapper convertBoldStyle: property ];
            } else if ( property.Name.equals ( attrItalic ) ) {
                fonttraits |= [ AquaA11yTextAttributesWrapper convertItalicStyle: property ];
            } else if ( property.Name.equals ( attrHeight ) ) {
                property.Value >>= fontsize;
            } else if ( property.Name.equals ( attrStrikethrough ) ) {
                if ( [ AquaA11yTextAttributesWrapper isStrikethrough: property ] ) {
                    [ string addAttribute: NSAccessibilityStrikethroughTextAttribute value: [ NSNumber numberWithBool: YES ] range: range ];
                }
            } else if ( property.Name.equals ( attrShadow ) ) {
                if ( [ AquaA11yTextAttributesWrapper convertBoolean: property ] ) {
                    [ string addAttribute: NSAccessibilityShadowTextAttribute value: [ NSNumber numberWithBool: YES ] range: range ];
                }
            } else if ( property.Name.equals ( attrUnderlineColor ) ) {
                property.Value >>= underlineColor;
            } else if ( property.Name.equals ( attrUnderlineHasColor ) ) {
                underlineHasColor = [ AquaA11yTextAttributesWrapper convertBoolean: property ];
            } else if ( property.Name.equals ( attrForegroundColor ) ) {
                [ AquaA11yTextAttributesWrapper addColor: property.Value.get<sal_Int32>() forAttribute: NSAccessibilityForegroundColorTextAttribute andRange: range toString: string ];
            } else if ( property.Name.equals ( attrBackgroundColor ) ) {
                [ AquaA11yTextAttributesWrapper addColor: property.Value.get<sal_Int32>() forAttribute: NSAccessibilityBackgroundColorTextAttribute andRange: range toString: string ];
            } else if ( property.Name.equals ( attrSuperscript ) ) {
                // values < zero mean subscript
                // values > zero mean superscript
                // this is true for both NSAccessibility-API and UNO-API
                NSNumber * number = [ AquaA11yTextAttributesWrapper convertShort: property ];
                if ( [ number shortValue ] != 0 ) {
                    [ string addAttribute: NSAccessibilitySuperscriptTextAttribute value: number range: range ];
                }
            }
        }
    }
    // add underline information
    if ( underlineHasColor ) {
        [ AquaA11yTextAttributesWrapper addColor: underlineColor forAttribute: NSAccessibilityUnderlineColorTextAttribute andRange: range toString: string ];
    }
    // add font information
    if ( wrapperStore != nil ) { // default
        [ wrapperStore setDefaultFontname: CreateNSString ( fontname ) ];
        [ wrapperStore setDefaultFontsize: fontsize ];
        NSFont * font = [ [ NSFontManager sharedFontManager ] fontWithFamily: CreateNSString ( fontname ) traits: fonttraits weight: 0 size: fontsize ];
        [ AquaA11yTextAttributesWrapper addFont: font toString: string forRange: range ];
    } else if ( wrapper != nil && fonttraits != 0 ) { // attribute run and bold and/or italic was found
        NSFont * font = [ [ NSFontManager sharedFontManager ] fontWithFamily: [ wrapper defaultFontname ] traits: fonttraits weight: 0 size: [ wrapper defaultFontsize ] ];
        [ AquaA11yTextAttributesWrapper addFont: font toString: string forRange: range ];
    }
    [ pool release ];
}

+(NSMutableAttributedString *)createAttributedStringForElement:(AquaA11yWrapper *)wrapper inOrigRange:(id)origRange {
    static const Sequence < OUString > emptySequence;
    // vars
    NSMutableAttributedString * string = nil;
    int loc = [ origRange rangeValue ].location;
    int len = [ origRange rangeValue ].length;
    int endIndex = loc + len;
    int currentIndex = loc;
    try {
        NSString * myString = CreateNSString ( [ wrapper accessibleText ] -> getText() ); // TODO: dirty fix for i87817
        string = [ [ NSMutableAttributedString alloc ] initWithString: CreateNSString ( [ wrapper accessibleText ] -> getTextRange ( loc, loc + len ) ) ];
        if ( [ wrapper accessibleTextAttributes ] != nil && [myString characterAtIndex:0] != 57361) { // TODO: dirty fix for i87817
            [ string beginEditing ];
            // add default attributes for whole string
            Sequence < PropertyValue > defaultAttributes = [ wrapper accessibleTextAttributes ] -> getDefaultAttributes ( emptySequence );
            [ AquaA11yTextAttributesWrapper applyAttributesFrom: defaultAttributes toString: string forRange: [ origRange rangeValue ] storeDefaultsTo: wrapper getDefaultsFrom: nil ];
            // add attributes for attribute run(s)
            while ( currentIndex < endIndex ) {
                TextSegment textSegment = [ wrapper accessibleText ] -> getTextAtIndex ( currentIndex, AccessibleTextType::ATTRIBUTE_RUN );
                int endOfRange = endIndex > textSegment.SegmentEnd ? textSegment.SegmentEnd : endIndex;
                NSRange rangeForAttributeRun = NSMakeRange ( currentIndex, endOfRange - currentIndex );
                // add run attributes
                Sequence < PropertyValue > attributes = [ wrapper accessibleTextAttributes ] -> getRunAttributes ( currentIndex, emptySequence );
                [ AquaA11yTextAttributesWrapper applyAttributesFrom: attributes toString: string forRange: rangeForAttributeRun storeDefaultsTo: nil getDefaultsFrom: wrapper ];
                currentIndex = textSegment.SegmentEnd;
            }
            [ string endEditing ];
        }
    } catch ( IllegalArgumentException & e ) {
        // empty
    } catch ( IndexOutOfBoundsException & e ) {
        // empty
    } catch ( RuntimeException& ) {
        // at least don't crash
    }
    return string;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
