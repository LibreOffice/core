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
#include <quartz/utils.h>
#include <quartz/salgdi.h>

#include "a11ytextattributeswrapper.h"

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>

namespace css_awt = ::com::sun::star::awt;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

// cannot use NSFontDescriptor as it has no notion of explicit NSUn{bold,italic}FontMask
@interface AquaA11yFontDescriptor : NSObject
{
    NSString *_name;
    NSFontTraitMask _traits;
    CGFloat _size;
}
-(void)setName:(NSString*)name;
-(void)setBold:(NSFontTraitMask)bold;
-(void)setItalic:(NSFontTraitMask)italic;
-(void)setSize:(CGFloat)size;
-(NSFont*)font;
@end

@implementation AquaA11yFontDescriptor
- (id)init
{
    if((self = [super init]))
    {
        _name = nil;
        _traits = 0;
        _size = 0.0;
    }
    return self;
}

- (id)initWithDescriptor:(AquaA11yFontDescriptor*)descriptor {
    if((self = [super init]))
    {
        _name = [descriptor->_name retain];
        _traits = descriptor->_traits;
        _size = descriptor->_size;
    }
    return self;
}

- (void)dealloc {
    [_name release];
    [super dealloc];
}

-(void)setName:(NSString*)name {
    if (_name != name) {
        [name retain];
        [_name release];
        _name = name;
    }
}

-(void)setBold:(NSFontTraitMask)bold {
    _traits &= ~(NSBoldFontMask | NSUnboldFontMask);
    _traits |= bold & (NSBoldFontMask | NSUnboldFontMask);
};

-(void)setItalic:(NSFontTraitMask)italic {
    _traits &= ~(NSItalicFontMask | NSUnitalicFontMask);
    _traits |= italic & (NSItalicFontMask | NSUnitalicFontMask);
};

-(void)setSize:(CGFloat)size { _size = size; }

-(NSFont*)font {
    return [[NSFontManager sharedFontManager] fontWithFamily:_name traits:_traits weight:0 size:_size];
}
@end

@implementation AquaA11yTextAttributesWrapper : NSObject

+(int)convertUnderlineStyle:(PropertyValue)property {
    int underlineStyle = NSUnderlineStyleNone;
    sal_Int16 value = 0;
    property.Value >>= value;
    if ( value != ::css_awt::FontUnderline::NONE
      && value != ::css_awt::FontUnderline::DONTKNOW) {
        underlineStyle = NSUnderlineStyleSingle;
    }
    return underlineStyle;
}

+(int)convertBoldStyle:(PropertyValue)property {
    int boldStyle = NSUnboldFontMask;
    float value = 0;
    property.Value >>= value;
    if ( value == ::css_awt::FontWeight::SEMIBOLD
      || value == ::css_awt::FontWeight::BOLD
      || value == ::css_awt::FontWeight::ULTRABOLD
      || value == ::css_awt::FontWeight::BLACK ) {
        boldStyle = NSBoldFontMask;
    }
    return boldStyle;
}

+(int)convertItalicStyle:(PropertyValue)property {
    int italicStyle = NSUnitalicFontMask;
    ::css_awt::FontSlant value = property.Value.get< ::css_awt::FontSlant>();
    if ( value == ::css_awt::FontSlant_ITALIC ) {
        italicStyle = NSItalicFontMask;
    }
    return italicStyle;
}

+(BOOL)isStrikethrough:(PropertyValue)property {
    bool strikethrough = false;
    sal_Int16 value = 0;
    property.Value >>= value;
    if ( value != ::css_awt::FontStrikeout::NONE
      && value != ::css_awt::FontStrikeout::DONTKNOW ) {
        strikethrough = true;
    }
    return strikethrough;
}

+(BOOL)convertBoolean:(PropertyValue)property {
    bool myBoolean = false;
    bool value = false;
    property.Value >>= value;
    if ( value ) {
        myBoolean = true;
    }
    return myBoolean;
}

+(NSNumber *)convertShort:(PropertyValue)property {
    sal_Int16 value = 0;
    property.Value >>= value;
    return [ NSNumber numberWithShort: value ];
}

+(void)addColor:(Color)nColor forAttribute:(NSString *)attribute andRange:(NSRange)range toString:(NSMutableAttributedString *)string {
    if( nColor == COL_TRANSPARENT )
        return;
    const RGBAColor aRGBAColor( nColor);
    CGColorRef aColorRef = CGColorCreate ( CGColorSpaceCreateWithName ( kCGColorSpaceGenericRGB ), aRGBAColor.AsArray() );
    [ string addAttribute: attribute value: reinterpret_cast<id>(aColorRef) range: range ];
    CGColorRelease( aColorRef );
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

+(void)applyAttributesFrom:(Sequence < PropertyValue > const &)attributes toString:(NSMutableAttributedString *)string forRange:(NSRange)range fontDescriptor:(AquaA11yFontDescriptor*)fontDescriptor {
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    // vars
    sal_Int32 underlineColor = 0;
    bool underlineHasColor = false;
    // add attributes to string
    for ( const PropertyValue& property : attributes ) {
        // TODO: NSAccessibilityMisspelledTextAttribute, NSAccessibilityAttachmentTextAttribute, NSAccessibilityLinkTextAttribute
        // NSAccessibilityStrikethroughColorTextAttribute is unsupported by UNP-API
        if ( property.Value.hasValue() ) {
            if ( property.Name == "CharUnderline" ) {
                int style = [ AquaA11yTextAttributesWrapper convertUnderlineStyle: property ];
                if ( style != NSUnderlineStyleNone ) {
                    [ string addAttribute: NSAccessibilityUnderlineTextAttribute value: [ NSNumber numberWithInt: style ] range: range ];
                }
            } else if ( property.Name == "CharFontName" ) {
                OUString fontname;
                property.Value >>= fontname;
                [fontDescriptor setName:CreateNSString(fontname)];
            } else if ( property.Name == "CharWeight" ) {
                [fontDescriptor setBold:[AquaA11yTextAttributesWrapper convertBoldStyle:property]];
            } else if ( property.Name == "CharPosture" ) {
                [fontDescriptor setItalic:[AquaA11yTextAttributesWrapper convertItalicStyle:property]];
            } else if ( property.Name == "CharHeight" ) {
                float size;
                property.Value >>= size;
                [fontDescriptor setSize:size];
            } else if ( property.Name == "CharStrikeout" ) {
                if ( [ AquaA11yTextAttributesWrapper isStrikethrough: property ] ) {
                    [ string addAttribute: NSAccessibilityStrikethroughTextAttribute value: [ NSNumber numberWithBool: YES ] range: range ];
                }
            } else if ( property.Name == "CharShadowed" ) {
                if ( [ AquaA11yTextAttributesWrapper convertBoolean: property ] ) {
                    [ string addAttribute: NSAccessibilityShadowTextAttribute value: [ NSNumber numberWithBool: YES ] range: range ];
                }
            } else if ( property.Name == "CharUnderlineColor" ) {
                property.Value >>= underlineColor;
            } else if ( property.Name == "CharUnderlineHasColor" ) {
                underlineHasColor = [ AquaA11yTextAttributesWrapper convertBoolean: property ];
            } else if ( property.Name == "CharColor" ) {
                [ AquaA11yTextAttributesWrapper addColor: Color(ColorTransparency, property.Value.get<sal_Int32>()) forAttribute: NSAccessibilityForegroundColorTextAttribute andRange: range toString: string ];
            } else if ( property.Name == "CharBackColor" ) {
                [ AquaA11yTextAttributesWrapper addColor: Color(ColorTransparency, property.Value.get<sal_Int32>()) forAttribute: NSAccessibilityBackgroundColorTextAttribute andRange: range toString: string ];
            } else if ( property.Name == "CharEscapement" ) {
                // values < zero mean subscript
                // values > zero mean superscript
                // this is true for both NSAccessibility-API and UNO-API
                NSNumber * number = [ AquaA11yTextAttributesWrapper convertShort: property ];
                if ( [ number shortValue ] != 0 ) {
                    [ string addAttribute: NSAccessibilitySuperscriptTextAttribute value: number range: range ];
                }
            } else if ( property.Name == "ParaAdjust" ) {
                sal_Int32 alignment;
                property.Value >>= alignment;
                NSNumber *textAlignment = nil;
SAL_WNODEPRECATED_DECLARATIONS_PUSH
    // 'NSCenterTextAlignment' is deprecated: first deprecated in macOS 10.12
    // 'NSJustifiedTextAlignment' is deprecated: first deprecated in macOS 10.12
    // 'NSLeftTextAlignment' is deprecated: first deprecated in macOS 10.12
    // 'NSRightTextAlignment' is deprecated: first deprecated in macOS 10.12
                switch(static_cast<css::style::ParagraphAdjust>(alignment)) {
                    case css::style::ParagraphAdjust_RIGHT : textAlignment = [NSNumber numberWithInteger:NSRightTextAlignment]    ; break;
                    case css::style::ParagraphAdjust_CENTER: textAlignment = [NSNumber numberWithInteger:NSCenterTextAlignment]   ; break;
                    case css::style::ParagraphAdjust_BLOCK : textAlignment = [NSNumber numberWithInteger:NSJustifiedTextAlignment]; break;
                    case css::style::ParagraphAdjust_LEFT  :
                    default                                             : textAlignment = [NSNumber numberWithInteger:NSLeftTextAlignment]     ; break;
                }
SAL_WNODEPRECATED_DECLARATIONS_POP
                NSDictionary *paragraphStyle = [NSDictionary dictionaryWithObjectsAndKeys:textAlignment, @"AXTextAlignment", textAlignment, @"AXVisualTextAlignment", nil];
                [string addAttribute:@"AXParagraphStyle" value:paragraphStyle range:range];
            }
        }
    }
    // add underline information
    if ( underlineHasColor ) {
        [ AquaA11yTextAttributesWrapper addColor: Color(ColorTransparency, underlineColor) forAttribute: NSAccessibilityUnderlineColorTextAttribute andRange: range toString: string ];
    }
    // add font information
    NSFont * font = [fontDescriptor font];
    [AquaA11yTextAttributesWrapper addFont:font toString:string forRange:range];
    [ pool release ];
}

+(void)addMarkup:(XAccessibleTextMarkup*)markup withType:(sal_Int32)type toString:(NSMutableAttributedString*)string inRange:(NSRange)range {
    const sal_Int32 markupCount = markup->getTextMarkupCount(type);
    for (sal_Int32 markupIndex = 0; markupIndex < markupCount; ++markupIndex) {
        TextSegment markupSegment = markup->getTextMarkup(markupIndex, type);
        NSRange markupRange = NSMakeRange(markupSegment.SegmentStart, markupSegment.SegmentEnd - markupSegment.SegmentStart);
        markupRange = NSIntersectionRange(range, markupRange);
        if (markupRange.length > 0) {
            markupRange.location -= range.location;
            switch(type) {
                case css::text::TextMarkupType::SPELLCHECK: {
                    [string addAttribute:NSAccessibilityMisspelledTextAttribute value:[NSNumber numberWithBool:YES] range:markupRange];
                    [string addAttribute:@"AXMarkedMisspelled" value:[NSNumber numberWithBool:YES] range:markupRange];
                    break;
                }
            }
        }
    }
}

+(void)addMarkup:(XAccessibleTextMarkup*)markup toString:(NSMutableAttributedString*)string inRange:(NSRange)range {
    [AquaA11yTextAttributesWrapper addMarkup:markup withType:css::text::TextMarkupType::SPELLCHECK toString:string inRange:range];
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
        if ( [ wrapper accessibleTextAttributes ] && [myString characterAtIndex:0] != 57361) { // TODO: dirty fix for i87817
            [ string beginEditing ];
            // add default attributes for whole string
            Sequence < PropertyValue > defaultAttributes = [ wrapper accessibleTextAttributes ] -> getDefaultAttributes ( emptySequence );
            AquaA11yFontDescriptor *defaultFontDescriptor = [[AquaA11yFontDescriptor alloc] init];
            [ AquaA11yTextAttributesWrapper applyAttributesFrom: defaultAttributes toString: string forRange: NSMakeRange ( 0, len ) fontDescriptor: defaultFontDescriptor ];
            // add attributes for attribute run(s)
            while ( currentIndex < endIndex ) {
                TextSegment textSegment = [ wrapper accessibleText ] -> getTextAtIndex ( currentIndex, AccessibleTextType::ATTRIBUTE_RUN );
                int endOfRange = endIndex > textSegment.SegmentEnd ? textSegment.SegmentEnd : endIndex;
                NSRange rangeForAttributeRun = NSMakeRange ( currentIndex - loc , endOfRange - currentIndex );
                // add run attributes
                Sequence < PropertyValue > attributes = [ wrapper accessibleTextAttributes ] -> getRunAttributes ( currentIndex, emptySequence );
                AquaA11yFontDescriptor *fontDescriptor = [[AquaA11yFontDescriptor alloc] initWithDescriptor:defaultFontDescriptor];
                [ AquaA11yTextAttributesWrapper applyAttributesFrom: attributes toString: string forRange: rangeForAttributeRun fontDescriptor: fontDescriptor ];
                [fontDescriptor release];
                currentIndex = textSegment.SegmentEnd;
            }
            [defaultFontDescriptor release];
            if ([wrapper accessibleTextMarkup])
                [AquaA11yTextAttributesWrapper addMarkup:[wrapper accessibleTextMarkup] toString:string inRange:[origRange rangeValue]];
            [ string endEditing ];
        }
    } catch ( IllegalArgumentException & ) {
        // empty
    } catch ( IndexOutOfBoundsException & ) {
        // empty
    } catch ( RuntimeException& ) {
        // at least don't crash
    }
    return string;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
