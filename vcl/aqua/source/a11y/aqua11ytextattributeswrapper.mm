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
#include "quartz/utils.h"
#include "aqua/salgdi.h"

#include "aqua11ytextattributeswrapper.h"

#include <com/sun/star/accessibility/AccessibleTextType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/text/TextMarkupType.hpp>

namespace css_awt = ::com::sun::star::awt;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::rtl;

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
    int underlineStyle = NSNoUnderlineStyle;
    sal_Int16 value = 0;
    property.Value >>= value;
    if ( value != ::css_awt::FontUnderline::NONE
      && value != ::css_awt::FontUnderline::DONTKNOW) {
        underlineStyle = NSSingleUnderlineStyle;
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
    sal_Int16 value = property.Value.get< ::css_awt::FontSlant>();
    if ( value == ::css_awt::FontSlant_ITALIC ) {
        italicStyle = NSItalicFontMask;
    }
    return italicStyle;
}

+(BOOL)isStrikethrough:(PropertyValue)property {
    BOOL strikethrough = NO;
    sal_Int16 value = 0;
    property.Value >>= value;
    if ( value != ::css_awt::FontStrikeout::NONE
      && value != ::css_awt::FontStrikeout::DONTKNOW ) {
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

+(void)addColor:(SalColor)nSalColor forAttribute:(NSString *)attribute andRange:(NSRange)range toString:(NSMutableAttributedString *)string {
    if( nSalColor == COL_TRANSPARENT )
        return;
    const RGBAColor aRGBAColor( nSalColor);
    CGColorRef aColorRef = CGColorCreate ( CGColorSpaceCreateWithName ( kCGColorSpaceGenericRGB ), aRGBAColor.AsArray() );
    [ string addAttribute: attribute value: (id) aColorRef range: range ];
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

+(void)applyAttributesFrom:(Sequence < PropertyValue >)attributes toString:(NSMutableAttributedString *)string forRange:(NSRange)range fontDescriptor:(AquaA11yFontDescriptor*)fontDescriptor {
    NSAutoreleasePool * pool = [ [ NSAutoreleasePool alloc ] init ];
    // constants
    static const OUString attrUnderline("CharUnderline");
    static const OUString attrBold("CharWeight");
    static const OUString attrFontname("CharFontName");
    static const OUString attrItalic("CharPosture");
    static const OUString attrHeight("CharHeight");
    static const OUString attrStrikethrough("CharStrikeout");
    static const OUString attrShadow("CharShadowed");
    static const OUString attrUnderlineColor("CharUnderlineColor");
    static const OUString attrUnderlineHasColor("CharUnderlineHasColor");
    static const OUString attrForegroundColor("CharColor");
    static const OUString attrBackgroundColor("CharBackColor");
    static const OUString attrSuperscript("CharEscapement");
    // vars
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
                OUString fontname;
                property.Value >>= fontname;
                [fontDescriptor setName:CreateNSString(fontname)];
            } else if ( property.Name.equals ( attrBold ) ) {
                [fontDescriptor setBold:[AquaA11yTextAttributesWrapper convertBoldStyle:property]];
            } else if ( property.Name.equals ( attrItalic ) ) {
                [fontDescriptor setItalic:[AquaA11yTextAttributesWrapper convertItalicStyle:property]];
            } else if ( property.Name.equals ( attrHeight ) ) {
                float size;
                property.Value >>= size;
                [fontDescriptor setSize:size];
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
    NSFont * font = [fontDescriptor font];
    [AquaA11yTextAttributesWrapper addFont:font toString:string forRange:range];
    [ pool release ];
}

+(void)addMarkup:(XAccessibleTextMarkup*)markup toString:(NSMutableAttributedString*)string inRange:(NSRange)range {
    [AquaA11yTextAttributesWrapper addMarkup:markup withType:(::com::sun::star::text::TextMarkupType::SPELLCHECK) toString:string inRange:range];
}

+(void)addMarkup:(XAccessibleTextMarkup*)markup withType:(long)type toString:(NSMutableAttributedString*)string inRange:(NSRange)range {
    const long markupCount = markup->getTextMarkupCount(type);
    for (long markupIndex = 0; markupIndex < markupCount; ++markupIndex) {
        TextSegment markupSegment = markup->getTextMarkup(markupIndex, type);
        NSRange markupRange = NSMakeRange(markupSegment.SegmentStart, markupSegment.SegmentEnd - markupSegment.SegmentStart);
        markupRange = NSIntersectionRange(range, markupRange);
        if (markupRange.length > 0) {
            markupRange.location -= range.location;
            switch(type) {
                case ::com::sun::star::text::TextMarkupType::SPELLCHECK: {
                    [string addAttribute:NSAccessibilityMisspelledTextAttribute value:[NSNumber numberWithBool:YES] range:markupRange];
                    break;
                }
            }
        }
    }
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
