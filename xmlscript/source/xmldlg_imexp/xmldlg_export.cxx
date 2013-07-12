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

#include "exp_share.hxx"

#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>

#include <com/sun/star/io/XPersistObject.hpp>

#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>

#include <com/sun/star/style/VerticalAlignment.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/util/NumberFormat.hpp>

#include <com/sun/star/view/SelectionType.hpp>

#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/GraphicObjectResolver.hpp>

#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

//__________________________________________________________________________________________________
Reference< xml::sax::XAttributeList > Style::createElement()
{
    ElementDescriptor * pStyle = new ElementDescriptor( XMLNS_DIALOGS_PREFIX ":style" );

    // style-id
    pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":style-id", _id );

    // background-color
    if (_set & 0x1)
    {
        OUStringBuffer buf( 16 );
        buf.append( (sal_Unicode)'0' );
        buf.append( (sal_Unicode)'x' );
        buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)_backgroundColor, 16 ) );
        pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":background-color", buf.makeStringAndClear() );
    }

    // text-color
    if (_set & 0x2)
    {
        OUStringBuffer buf( 16 );
        buf.append( (sal_Unicode)'0' );
        buf.append( (sal_Unicode)'x' );
        buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)_textColor, 16 ) );
        pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":text-color", buf.makeStringAndClear() );
    }

    // textline-color
    if (_set & 0x20)
    {
        OUStringBuffer buf( 16 );
        buf.append( (sal_Unicode)'0' );
        buf.append( (sal_Unicode)'x' );
        buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)_textLineColor, 16 ) );
        pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":textline-color", buf.makeStringAndClear() );
    }

    // fill-color
    if (_set & 0x10)
    {
        OUStringBuffer buf( 16 );
        buf.append( (sal_Unicode)'0' );
        buf.append( (sal_Unicode)'x' );
        buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)_fillColor, 16 ) );
        pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":fill-color", buf.makeStringAndClear() );
    }

    // border
    if (_set & 0x4)
    {
        switch (_border)
        {
        case BORDER_NONE:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":border", "none" );
            break;
        case BORDER_3D:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":border", "3d" );
            break;
        case BORDER_SIMPLE:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":border", "simple" );
            break;
        case BORDER_SIMPLE_COLOR: {
            OUStringBuffer buf;
            buf.appendAscii( "0x" );
            buf.append( OUString::valueOf((sal_Int64)(sal_uInt64)_borderColor, 16 ) );
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":border", buf.makeStringAndClear() );
            break;
        }
        default:
            SAL_WARN( "xmlscript.xmldlg", "### unexpected border value!" );
            break;
        }
    }

    // visual effect (look)
    if (_set & 0x40)
    {
        switch (_visualEffect)
        {
        case awt::VisualEffect::NONE:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":look", "none" );
            break;
        case awt::VisualEffect::LOOK3D:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":look", "3d" );
            break;
        case awt::VisualEffect::FLAT:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":look", "simple" );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### unexpected visual effect value!" );
            break;
        }
    }

    // font-
    if (_set & 0x8)
    {
        awt::FontDescriptor def_descr;

        // dialog:font-name CDATA #IMPLIED
        if (def_descr.Name != _descr.Name)
        {
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-name", _descr.Name );
        }
        // dialog:font-height %numeric; #IMPLIED
        if (def_descr.Height != _descr.Height)
        {
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-height", OUString::valueOf( (sal_Int32)_descr.Height ) );
        }
        // dialog:font-width %numeric; #IMPLIED
        if (def_descr.Width != _descr.Width)
        {
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-width", OUString::valueOf( (sal_Int32)_descr.Width ) );
        }
        // dialog:font-stylename CDATA #IMPLIED
        if (def_descr.StyleName != _descr.StyleName)
        {
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-stylename", _descr.StyleName );
        }
        // dialog:font-family "(decorative|modern|roman|script|swiss|system)" #IMPLIED
        if (def_descr.Family != _descr.Family)
        {
            switch (_descr.Family)
            {
            case awt::FontFamily::DECORATIVE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-family", "decorative" );
                break;
            case awt::FontFamily::MODERN:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-family", "modern" );
                break;
            case awt::FontFamily::ROMAN:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-family", "roman" );
                break;
            case awt::FontFamily::SCRIPT:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-family", "script" );
                break;
            case awt::FontFamily::SWISS:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-family", "swiss" );
                break;
            case awt::FontFamily::SYSTEM:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-family", "system" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-family!" );
                break;
            }
        }
        // dialog:font-charset "(ansi|mac|ibmpc_437|ibmpc_850|ibmpc_860|ibmpc_861|ibmpc_863|ibmpc_865|system|symbol)" #IMPLIED
        if (def_descr.CharSet != _descr.CharSet)
        {
            switch (_descr.CharSet)
            {
            case awt::CharSet::ANSI:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "ansi" );
                break;
            case awt::CharSet::MAC:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "mac" );
                break;
            case awt::CharSet::IBMPC_437:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "ibmpc_437" );
                break;
            case awt::CharSet::IBMPC_850:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "ibmpc_850" );
                break;
            case awt::CharSet::IBMPC_860:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "ibmpc_860" );
                break;
            case awt::CharSet::IBMPC_861:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "ibmpc_861" );
                break;
            case awt::CharSet::IBMPC_863:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "ibmpc_863" );
                break;
            case awt::CharSet::IBMPC_865:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "ibmpc_865" );
                break;
            case awt::CharSet::SYSTEM:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "system" );
                break;
            case awt::CharSet::SYMBOL:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charset", "symbol" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-charset!" );
                break;
            }
        }
        // dialog:font-pitch "(fixed|variable)" #IMPLIED
        if (def_descr.Pitch != _descr.Pitch)
        {
            switch (_descr.Pitch)
            {
            case awt::FontPitch::FIXED:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-pitch", "fixed" );
                break;
            case awt::FontPitch::VARIABLE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-pitch", "variable" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-pitch!" );
                break;
            }
        }
        // dialog:font-charwidth CDATA #IMPLIED
        if (def_descr.CharacterWidth != _descr.CharacterWidth)
        {
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-charwidth", OUString::valueOf( (float)_descr.CharacterWidth ) );
        }
        // dialog:font-weight CDATA #IMPLIED
        if (def_descr.Weight != _descr.Weight)
        {
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-weight", OUString::valueOf( (float)_descr.Weight ) );
        }
        // dialog:font-slant "(oblique|italic|reverse_oblique|reverse_italic)" #IMPLIED
        if (def_descr.Slant != _descr.Slant)
        {
            switch (_descr.Slant)
            {
            case awt::FontSlant_OBLIQUE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-slant", "oblique" );
                break;
            case awt::FontSlant_ITALIC:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-slant", "italic" );
                break;
            case awt::FontSlant_REVERSE_OBLIQUE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-slant", "reverse_oblique" );
                break;
            case awt::FontSlant_REVERSE_ITALIC:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-slant", "reverse_italic" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-slant!" );
                break;
            }
        }
        // dialog:font-underline "(single|double|dotted|dash|longdash|dashdot|dashdotdot|smallwave|wave|doublewave|bold|bolddotted|bolddash|boldlongdash|bolddashdot|bolddashdotdot|boldwave)" #IMPLIED
        if (def_descr.Underline != _descr.Underline)
        {
            switch (_descr.Underline)
            {
            case awt::FontUnderline::SINGLE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "single" );
                break;
            case awt::FontUnderline::DOUBLE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "double" );
                break;
            case awt::FontUnderline::DOTTED:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "dotted" );
                break;
            case awt::FontUnderline::DASH:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "dash" );
                break;
            case awt::FontUnderline::LONGDASH:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "longdash" );
                break;
            case awt::FontUnderline::DASHDOT:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "dashdot" );
                break;
            case awt::FontUnderline::DASHDOTDOT:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "dashdotdot" );
                break;
            case awt::FontUnderline::SMALLWAVE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "smallwave" );
                break;
            case awt::FontUnderline::WAVE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "wave" );
                break;
            case awt::FontUnderline::DOUBLEWAVE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "doublewave" );
                break;
            case awt::FontUnderline::BOLD:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "bold" );
                break;
            case awt::FontUnderline::BOLDDOTTED:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "bolddotted" );
                break;
            case awt::FontUnderline::BOLDDASH:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "bolddash" );
                break;
            case awt::FontUnderline::BOLDLONGDASH:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "boldlongdash" );
                break;
            case awt::FontUnderline::BOLDDASHDOT:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "bolddashdot" );
                break;
            case awt::FontUnderline::BOLDDASHDOTDOT:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "bolddashdotdot" );
                break;
            case awt::FontUnderline::BOLDWAVE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-underline", "boldwave" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-underline!" );
                break;
            }
        }
        // dialog:font-strikeout "(single|double|bold|slash|x)" #IMPLIED
        if (def_descr.Strikeout != _descr.Strikeout)
        {
            switch (_descr.Strikeout)
            {
            case awt::FontStrikeout::SINGLE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-strikeout", "single" );
                break;
            case awt::FontStrikeout::DOUBLE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-strikeout", "double" );
                break;
            case awt::FontStrikeout::BOLD:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-strikeout", "bold" );
                break;
            case awt::FontStrikeout::SLASH:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-strikeout", "slash" );
                break;
            case awt::FontStrikeout::X:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-strikeout", "x" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-strikeout!" );
                break;
            }
        }
        // dialog:font-orientation CDATA #IMPLIED
        if (def_descr.Orientation != _descr.Orientation)
        {
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-orientation", OUString::valueOf( (float)_descr.Orientation ) );
        }
        // dialog:font-kerning %boolean; #IMPLIED
        if ((def_descr.Kerning != sal_False) != (_descr.Kerning != sal_False))
        {
            pStyle->addBoolAttr( XMLNS_DIALOGS_PREFIX ":font-kerning", _descr.Kerning );
        }
        // dialog:font-wordlinemode %boolean; #IMPLIED
        if ((def_descr.WordLineMode != sal_False) != (_descr.WordLineMode != sal_False))
        {
            pStyle->addBoolAttr( XMLNS_DIALOGS_PREFIX ":font-wordlinemode", _descr.WordLineMode );
        }
        // dialog:font-type "(raster|device|scalable)" #IMPLIED
        if (def_descr.Type != _descr.Type)
        {
            switch (_descr.Type)
            {
            case awt::FontType::RASTER:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-type", "raster" );
                break;
            case awt::FontType::DEVICE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-type", "device" );
                break;
            case awt::FontType::SCALABLE:
                pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-type", "scalable" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-type!" );
                break;
            }
        }

        // additional attributes not in FontDescriptor struct
        // dialog:font-relief (none|embossed|engraved) #IMPLIED
        switch (_fontRelief)
        {
        case awt::FontRelief::NONE: // dont export default
            break;
        case awt::FontRelief::EMBOSSED:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-relief", "embossed" );
            break;
        case awt::FontRelief::ENGRAVED:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-relief", "engraved" );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### unexpected font-relief!" );
            break;
        }
        // dialog:font-emphasismark (none|dot|circle|disc|accent|above|below) #IMPLIED
        switch (_fontEmphasisMark)
        {
        case awt::FontEmphasisMark::NONE: // dont export default
            break;
        case awt::FontEmphasisMark::DOT:
            pStyle->addAttribute(XMLNS_DIALOGS_PREFIX ":font-emphasismark", "dot" );
            break;
        case awt::FontEmphasisMark::CIRCLE:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-emphasismark", "circle" );
            break;
        case awt::FontEmphasisMark::DISC:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-emphasismark", "disc" );
            break;
        case awt::FontEmphasisMark::ACCENT:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-emphasismark", "accent" );
            break;
        case awt::FontEmphasisMark::ABOVE:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-emphasismark", "above" );
            break;
        case awt::FontEmphasisMark::BELOW:
            pStyle->addAttribute( XMLNS_DIALOGS_PREFIX ":font-emphasismark", "below" );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### unexpected font-emphasismark!" );
            break;
        }
    }

    return pStyle;
}

//##################################################################################################

//__________________________________________________________________________________________________
void ElementDescriptor::addNumberFormatAttr(
    Reference< beans::XPropertySet > const & xFormatProperties )
{
    Reference< beans::XPropertyState > xState( xFormatProperties, UNO_QUERY );
    OUString sFormat;
    lang::Locale locale;
    OSL_VERIFY( xFormatProperties->getPropertyValue( "FormatString" ) >>= sFormat );
    OSL_VERIFY( xFormatProperties->getPropertyValue( "Locale" ) >>= locale );

    addAttribute(XMLNS_DIALOGS_PREFIX ":format-code", sFormat );

    // format-locale
    LanguageTag aLanguageTag( locale);
    OUString aStr;
    if (aLanguageTag.isIsoLocale())
    {
        // Old style "lll;CC" for compatibility, I really don't know what may
        // consume this.
        if (aLanguageTag.getCountry().isEmpty())
            aStr = aLanguageTag.getLanguage();
        else
            aStr = aLanguageTag.getLanguage() + ";" + aLanguageTag.getCountry();
    }
    else
    {
        aStr = aLanguageTag.getBcp47( false);
    }
    addAttribute( XMLNS_DIALOGS_PREFIX ":format-locale", aStr );
}
//__________________________________________________________________________________________________
Any ElementDescriptor::readProp( OUString const & rPropName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        return _xProps->getPropertyValue( rPropName );
    }
    return Any();
}

//______________________________________________________________________________
void ElementDescriptor::readStringAttr(
    OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE !=
        _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        OUString v;
        if (a >>= v)
            addAttribute( rAttrName, v );
        else
            SAL_WARN( "xmlscript.xmldlg", "### unexpected property type!" );
    }
}

//__________________________________________________________________________________________________
void ElementDescriptor::readHexLongAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_LONG)
        {
            OUStringBuffer buf( 16 );
            buf.append( (sal_Unicode)'0' );
            buf.append( (sal_Unicode)'x' );
            buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)*(sal_uInt32 *)a.getValue(), 16 ) );
            addAttribute( rAttrName, buf.makeStringAndClear() );
        }
    }
}

//__________________________________________________________________________________________________
void ElementDescriptor::readDateFormatAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_SHORT)
        {
            switch (*(sal_Int16 const *)a.getValue())
            {
            case 0:
                addAttribute( rAttrName, "system_short" );
                break;
            case 1:
                addAttribute( rAttrName, "system_short_YY" );
                break;
            case 2:
                addAttribute( rAttrName, "system_short_YYYY" );
                break;
            case 3:
                addAttribute( rAttrName, "system_long" );
                break;
            case 4:
                addAttribute( rAttrName, "short_DDMMYY" );
                break;
            case 5:
                addAttribute( rAttrName, "short_MMDDYY" );
                break;
            case 6:
                addAttribute( rAttrName, "short_YYMMDD" );
                break;
            case 7:
                addAttribute( rAttrName, "short_DDMMYYYY" );
                break;
            case 8:
                addAttribute( rAttrName, "short_MMDDYYYY" );
                break;
            case 9:
                addAttribute( rAttrName, "short_YYYYMMDD" );
                break;
            case 10:
                addAttribute( rAttrName, "short_YYMMDD_DIN5008" );
                break;
            case 11:
                addAttribute( rAttrName, "short_YYYYMMDD_DIN5008" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected date format!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readTimeFormatAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_SHORT)
        {
            switch (*(sal_Int16 const *)a.getValue())
            {
            case 0:
                addAttribute( rAttrName, "24h_short" );
                break;
            case 1:
                addAttribute( rAttrName, "24h_long" );
                break;
            case 2:
                addAttribute( rAttrName, "12h_short" );
                break;
            case 3:
                addAttribute( rAttrName, "12h_long" );
                break;
            case 4:
                addAttribute( rAttrName, "Duration_short" );
                break;
            case 5:
                addAttribute( rAttrName, "Duration_long" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected time format!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readAlignAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_SHORT)
        {
            switch (*(sal_Int16 const *)a.getValue())
            {
            case 0:
                addAttribute( rAttrName, "left" );
                break;
            case 1:
                addAttribute( rAttrName, "center" );
                break;
            case 2:
                addAttribute( rAttrName, "right" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### illegal alignment value!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readVerticalAlignAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_ENUM && a.getValueType() == ::getCppuType( (style::VerticalAlignment*)0 ))
        {
            style::VerticalAlignment eAlign;
            a >>= eAlign;
            switch (eAlign)
            {
            case style::VerticalAlignment_TOP:
                addAttribute( rAttrName, "top" );
                break;
            case style::VerticalAlignment_MIDDLE:
                addAttribute( rAttrName, "center" );
                break;
            case style::VerticalAlignment_BOTTOM:
                addAttribute( rAttrName, "bottom" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### illegal vertical alignment value!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readImageURLAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        OUString sURL;
        _xProps->getPropertyValue( rPropName ) >>= sURL;

        if ( sURL.startsWith( XMLSCRIPT_GRAPHOBJ_URLPREFIX ) )
        {
            Reference< document::XStorageBasedDocument > xDocStorage( _xDocument, UNO_QUERY );
            if ( xDocStorage.is() )
            {
                Reference<XComponentContext> xContext = ::comphelper::getProcessComponentContext();
                uno::Reference< document::XGraphicObjectResolver > xGraphicResolver =
                    document::GraphicObjectResolver::createWithStorage( xContext, xDocStorage->getDocumentStorage() );
                sURL = xGraphicResolver->resolveGraphicObjectURL( sURL );
            }
        }
        if ( !sURL.isEmpty() )
                addAttribute( rAttrName, sURL );
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readImageAlignAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_SHORT)
        {
            switch (*(sal_Int16 const *)a.getValue())
            {
            case 0:
                addAttribute( rAttrName, "left" );
                break;
            case 1:
                addAttribute( rAttrName, "top" );
                break;
            case 2:
                addAttribute( rAttrName, "right" );
                break;
            case 3:
                addAttribute( rAttrName, "bottom" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### illegal image alignment value!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readImagePositionAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_SHORT)
        {
            switch (*(sal_Int16 const *)a.getValue())
            {
            case awt::ImagePosition::LeftTop:
                addAttribute( rAttrName, "left-top" );
                break;
            case awt::ImagePosition::LeftCenter:
                addAttribute( rAttrName, "left-center" );
                break;
            case awt::ImagePosition::LeftBottom:
                addAttribute( rAttrName, "left-bottom" );
                break;
            case awt::ImagePosition::RightTop:
                addAttribute( rAttrName, "right-top" );
                break;
            case awt::ImagePosition::RightCenter:
                addAttribute( rAttrName, "right-center" );
                break;
            case awt::ImagePosition::RightBottom:
                addAttribute( rAttrName, "right-bottom" );
                break;
            case awt::ImagePosition::AboveLeft:
                addAttribute( rAttrName, "top-left" );
                break;
            case awt::ImagePosition::AboveCenter:
                addAttribute( rAttrName, "top-center" );
                break;
            case awt::ImagePosition::AboveRight:
                addAttribute( rAttrName, "top-right" );
                break;
            case awt::ImagePosition::BelowLeft:
                addAttribute( rAttrName, "bottom-left" );
                break;
            case awt::ImagePosition::BelowCenter:
                addAttribute( rAttrName, "bottom-center" );
                break;
            case awt::ImagePosition::BelowRight:
                addAttribute( rAttrName, "bottom-right" );
                break;
            case awt::ImagePosition::Centered:
                addAttribute( rAttrName, "center" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### illegal image position value!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readButtonTypeAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_SHORT)
        {
            switch (*(sal_Int16 const *)a.getValue())
            {
            case awt::PushButtonType_STANDARD:
                addAttribute( rAttrName, "standard" );
                break;
            case awt::PushButtonType_OK:
                addAttribute( rAttrName, "ok" );
                break;
            case awt::PushButtonType_CANCEL:
                addAttribute( rAttrName, "cancel" );
                break;
            case awt::PushButtonType_HELP:
                addAttribute( rAttrName, "help" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### illegal button-type value!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readOrientationAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_LONG)
        {
            switch (*(sal_Int32 const *)a.getValue())
            {
            case 0:
                addAttribute( rAttrName, "horizontal" );
                break;
            case 1:
                addAttribute( rAttrName, "vertical" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### illegal orientation value!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readLineEndFormatAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (a.getValueTypeClass() == TypeClass_SHORT)
        {
            switch (*(sal_Int16 const *)a.getValue())
            {
            case awt::LineEndFormat::CARRIAGE_RETURN:
                addAttribute( rAttrName, "carriage-return" );
                break;
            case awt::LineEndFormat::LINE_FEED:
                addAttribute( rAttrName, "line-feed" );
                break;
            case awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED:
                addAttribute( rAttrName, "carriage-return-line-feed" );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### illegal line end format value!" );
                break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readDataAwareAttr( OUString const & rAttrName )
{
    Reference< lang::XMultiServiceFactory > xFac;
    if ( _xDocument.is() )
        xFac.set( _xDocument, uno::UNO_QUERY );

    Reference< form::binding::XBindableValue > xBinding( _xProps, UNO_QUERY );

    if ( xFac.is() && xBinding.is() && rAttrName.equals( XMLNS_DIALOGS_PREFIX ":linked-cell" ) )
    {
        try
        {
            Reference< beans::XPropertySet > xConvertor( xFac->createInstance( "com.sun.star.table.CellAddressConversion" ), uno::UNO_QUERY );
        Reference< beans::XPropertySet > xBindable( xBinding->getValueBinding(), UNO_QUERY );
            if ( xBindable.is() )
            {
                table::CellAddress aAddress;
                xBindable->getPropertyValue( "BoundCell" ) >>= aAddress;
                xConvertor->setPropertyValue( "Address", makeAny( aAddress ) );
                OUString sAddress;
                xConvertor->getPropertyValue( "PersistentRepresentation" ) >>= sAddress;
                if ( !sAddress.isEmpty() )
                    addAttribute( rAttrName, sAddress );

                SAL_INFO("xmlscript.xmldlg", "*** Bindable value " << sAddress );

            }
        }
        catch( uno::Exception& )
        {
        }
    }
    Reference< form::binding::XListEntrySink > xEntrySink( _xProps, UNO_QUERY );
    if ( xEntrySink.is() && rAttrName.equals( XMLNS_DIALOGS_PREFIX ":source-cell-range" ) )
    {
        Reference< beans::XPropertySet > xListSource( xEntrySink->getListEntrySource(), UNO_QUERY );
        if ( xListSource.is() )
        {
            try
            {
                Reference< beans::XPropertySet > xConvertor( xFac->createInstance( "com.sun.star.table.CellRangeAddressConversion" ), uno::UNO_QUERY );

                table::CellRangeAddress aAddress;
                xListSource->getPropertyValue( "CellRange" ) >>= aAddress;

                OUString sAddress;
                xConvertor->setPropertyValue( "Address", makeAny( aAddress ) );
                xConvertor->getPropertyValue( "PersistentRepresentation" ) >>= sAddress;
                SAL_INFO("xmlscript.xmldlg","**** cell range source list " << sAddress );
                if ( !sAddress.isEmpty() )
                    addAttribute( rAttrName, sAddress );
            }
            catch( uno::Exception& )
            {
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readSelectionTypeAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any aSelectionType ( _xProps->getPropertyValue( rPropName ) );

        if (aSelectionType.getValueTypeClass() == TypeClass_ENUM && aSelectionType.getValueType() == ::getCppuType( (::view::SelectionType*)0 ))
        {
            ::view::SelectionType eSelectionType;
            aSelectionType >>= eSelectionType;

            switch (eSelectionType)
            {
                case ::view::SelectionType_NONE:
                    addAttribute( rAttrName, "none" );
                    break;
                case ::view::SelectionType_SINGLE:
                    addAttribute( rAttrName, "single" );
                    break;
                case ::view::SelectionType_MULTI:
                    addAttribute( rAttrName, "multi" );
                    break;
                case ::view::SelectionType_RANGE:
                    addAttribute( rAttrName, "range" );
                    break;
                default:
                    SAL_WARN( "xmlscript.xmldlg", "### illegal selection type value!" );
                    break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readScrollableSettings()
{
    readLongAttr( "ScrollHeight",
                  XMLNS_DIALOGS_PREFIX ":scrollheight" );
    readLongAttr( "ScrollWidth",
                  XMLNS_DIALOGS_PREFIX ":scrollwidth" );
    readLongAttr( "ScrollTop",
                  XMLNS_DIALOGS_PREFIX ":scrolltop" );
    readLongAttr( "ScrollLeft",
                  XMLNS_DIALOGS_PREFIX ":scrollleft" );
    readBoolAttr( "HScroll",
                  XMLNS_DIALOGS_PREFIX ":hscroll" );
    readBoolAttr( "VScroll",
                  XMLNS_DIALOGS_PREFIX ":vscroll" );
}

void ElementDescriptor::readDefaults( bool supportPrintable, bool supportVisible )
{
    Any a( _xProps->getPropertyValue( "Name" ) );

    // The following is a hack to allow 'form' controls to override the default
    // control supported by dialogs. This should work well for both vba support and
    // normal openoffice ( when normal 'Dialogs' decide to support form control models )
    // In the future VBA support might require custom models ( and not the just the form
    // variant of a control that we currently use ) In this case the door is still open,
    // we just need to define a new way for the 'ServiceName' to be extracted from the
    // incomming model. E.g. the use of supporting service
    // "com.sun.star.form.FormComponent", 'ServiceName' and XPersistObject
    // is only an implementation detail here, in the future some other
    // method ( perhaps a custom prop ) could be used instead.
    Reference< lang::XServiceInfo > xSrvInfo( _xProps, UNO_QUERY );
    if ( xSrvInfo.is() && xSrvInfo->supportsService( "com.sun.star.form.FormComponent" ) )
    {
        Reference< io::XPersistObject > xPersist( _xProps, UNO_QUERY );
        if ( xPersist.is() )
        {
            OUString sCtrlName = xPersist->getServiceName();
            if ( !sCtrlName.isEmpty() )
                    addAttribute( XMLNS_DIALOGS_PREFIX ":control-implementation", sCtrlName );
        }
    }
    addAttribute( XMLNS_DIALOGS_PREFIX ":id", * reinterpret_cast< const OUString * >( a.getValue() ) );
    readShortAttr( "TabIndex", XMLNS_DIALOGS_PREFIX ":tab-index" );

    sal_Bool bEnabled = sal_False;
    if (_xProps->getPropertyValue( "Enabled" ) >>= bEnabled)
    {
        if (! bEnabled)
        {
            addAttribute( XMLNS_DIALOGS_PREFIX ":disabled", "true" );
        }
    }
    else
    {
        SAL_WARN( "xmlscript.xmldlg", "unexpected property type for \"Enabled\": not bool!" );
    }

    sal_Bool bVisible = sal_True;
    if (supportVisible) try
    {
        if (_xProps->getPropertyValue("EnableVisible" ) >>= bVisible)
        {

            // only write out the non default case
            if (! bVisible)
            {
                addAttribute( XMLNS_DIALOGS_PREFIX ":visible", "false" );
            }
        }
    }
    catch( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    // force writing of pos/size
    a = _xProps->getPropertyValue( "PositionX" );
    if (a.getValueTypeClass() == TypeClass_LONG)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":left", OUString::valueOf( *(sal_Int32 const *)a.getValue() ) );
    }
    a = _xProps->getPropertyValue( "PositionY" );
    if (a.getValueTypeClass() == TypeClass_LONG)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":top", OUString::valueOf( *(sal_Int32 const *)a.getValue() ) );
    }
    a = _xProps->getPropertyValue( "Width" );
    if (a.getValueTypeClass() == TypeClass_LONG)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":width", OUString::valueOf( *(sal_Int32 const *)a.getValue() ) );
    }
    a = _xProps->getPropertyValue( "Height" );
    if (a.getValueTypeClass() == TypeClass_LONG)
    {
        addAttribute( XMLNS_DIALOGS_PREFIX ":height", OUString::valueOf( *(sal_Int32 const *)a.getValue() ) );
    }

    if (supportPrintable)
    {
        readBoolAttr( "Printable", XMLNS_DIALOGS_PREFIX ":printable" );
    }
    readLongAttr( "Step", XMLNS_DIALOGS_PREFIX ":page" );
    readStringAttr( "Tag", XMLNS_DIALOGS_PREFIX ":tag" );
    readStringAttr( "HelpText", XMLNS_DIALOGS_PREFIX ":help-text" );
    readStringAttr( "HelpURL", XMLNS_DIALOGS_PREFIX ":help-url" );
}

struct StringTriple
{
    char const * first;
    char const * second;
    char const * third;
};
extern StringTriple const * const g_pEventTranslations;

//__________________________________________________________________________________________________
void ElementDescriptor::readEvents()
    SAL_THROW( (Exception) )
{
    Reference< script::XScriptEventsSupplier > xSupplier( _xProps, UNO_QUERY );
    if (xSupplier.is())
    {
        Reference< container::XNameContainer > xEvents( xSupplier->getEvents() );
        if (xEvents.is())
        {
            Sequence< OUString > aNames( xEvents->getElementNames() );
            OUString const * pNames = aNames.getConstArray();
            for ( sal_Int32 nPos = 0; nPos < aNames.getLength(); ++nPos )
            {
                script::ScriptEventDescriptor descr;
                if (xEvents->getByName( pNames[ nPos ] ) >>= descr)
                {
                    SAL_WARN_IF( descr.ListenerType.isEmpty() ||
                                descr.EventMethod.isEmpty() ||
                                descr.ScriptCode.isEmpty() ||
                                descr.ScriptType.isEmpty() , "xmlscript.xmldlg", "### invalid event descr!" );

                    OUString aEventName;

                    if (descr.AddListenerParam.isEmpty())
                    {
                        // detection of event-name
                        OString listenerType( OUStringToOString( descr.ListenerType, RTL_TEXTENCODING_ASCII_US ) );
                        OString eventMethod( OUStringToOString( descr.EventMethod, RTL_TEXTENCODING_ASCII_US ) );
                        StringTriple const * p = g_pEventTranslations;
                        while (p->first)
                        {
                            if (0 == ::rtl_str_compare( p->second, eventMethod.getStr() ) &&
                                0 == ::rtl_str_compare( p->first, listenerType.getStr() ))
                            {
                                aEventName = OUString( p->third, ::rtl_str_getLength( p->third ), RTL_TEXTENCODING_ASCII_US );
                                break;
                            }
                            ++p;
                        }
                    }

                    ElementDescriptor * pElem;
                    Reference< xml::sax::XAttributeList > xElem;

                    if (!aEventName.isEmpty()) // script:event
                    {
                        pElem = new ElementDescriptor( XMLNS_SCRIPT_PREFIX ":event" );
                        xElem = pElem;

                        pElem->addAttribute( XMLNS_SCRIPT_PREFIX ":event-name", aEventName );
                    }
                    else // script:listener-event
                    {
                        pElem = new ElementDescriptor( XMLNS_SCRIPT_PREFIX ":listener-event" );
                        xElem = pElem;

                        pElem->addAttribute( XMLNS_SCRIPT_PREFIX ":listener-type", descr.ListenerType );
                        pElem->addAttribute( XMLNS_SCRIPT_PREFIX ":listener-method", descr.EventMethod );

                        if (!descr.AddListenerParam.isEmpty())
                        {
                            pElem->addAttribute( XMLNS_SCRIPT_PREFIX ":listener-param", descr.AddListenerParam );
                        }
                    }
                    if ( descr.ScriptType == "StarBasic" )
                    {
                        // separate optional location
                        sal_Int32 nIndex = descr.ScriptCode.indexOf( (sal_Unicode)':' );
                        if (nIndex >= 0)
                        {
                            pElem->addAttribute( XMLNS_SCRIPT_PREFIX ":location", descr.ScriptCode.copy( 0, nIndex ) );
                            pElem->addAttribute( XMLNS_SCRIPT_PREFIX ":macro-name", descr.ScriptCode.copy( nIndex +1 ) );
                        }
                        else
                        {
                            pElem->addAttribute( XMLNS_SCRIPT_PREFIX ":macro-name", descr.ScriptCode );
                        }
                    }
                    else
                    {
                        pElem->addAttribute(XMLNS_SCRIPT_PREFIX ":macro-name", descr.ScriptCode );
                    }

                    // language
                    pElem->addAttribute( XMLNS_SCRIPT_PREFIX ":language", descr.ScriptType );

                    addSubElement( xElem );
                }
                else
                {
                    SAL_WARN( "xmlscript.xmldlg", "### unexpected event type in container!" );
                }
            }
        }
    }
}

//##################################################################################################

inline bool equalFont( Style const & style1, Style const & style2 )
{
    awt::FontDescriptor const & f1 = style1._descr;
    awt::FontDescriptor const & f2 = style2._descr;
    return (
        f1.Name == f2.Name &&
        f1.Height == f2.Height &&
        f1.Width == f2.Width &&
        f1.StyleName == f2.StyleName &&
        f1.Family == f2.Family &&
        f1.CharSet == f2.CharSet &&
        f1.Pitch == f2.CharSet &&
        f1.CharacterWidth == f2.CharacterWidth &&
        f1.Weight == f2.Weight &&
        f1.Slant == f2.Slant &&
        f1.Underline == f2.Underline &&
        f1.Strikeout == f2.Strikeout &&
        f1.Orientation == f2.Orientation &&
        (f1.Kerning != sal_False) == (f2.Kerning != sal_False) &&
        (f1.WordLineMode != sal_False) == (f2.WordLineMode != sal_False) &&
        f1.Type == f2.Type &&
        style1._fontRelief == style2._fontRelief &&
        style1._fontEmphasisMark == style2._fontEmphasisMark
        );
}
//__________________________________________________________________________________________________
OUString StyleBag::getStyleId( Style const & rStyle )
    SAL_THROW(())
{
    if (! rStyle._set) // nothin set
    {
        return OUString(); // everything default: no need to export a specific style
    }

    // lookup existing style
    for ( size_t nStylesPos = 0; nStylesPos < _styles.size(); ++nStylesPos )
    {
        Style * pStyle = _styles[ nStylesPos ];

        short demanded_defaults = ~rStyle._set & rStyle._all;
        // test, if defaults are not set
        if ((~pStyle->_set & demanded_defaults) == demanded_defaults &&
            (rStyle._set & (pStyle->_all & ~pStyle->_set)) == 0)
        {
            short bset = rStyle._set & pStyle->_set;
            if ((bset & 0x1) &&
                rStyle._backgroundColor != pStyle->_backgroundColor)
                continue;
            if ((bset & 0x2) &&
                rStyle._textColor != pStyle->_textColor)
                continue;
            if ((bset & 0x20) &&
                rStyle._textLineColor != pStyle->_textLineColor)
                continue;
            if ((bset & 0x10) &&
                rStyle._fillColor != pStyle->_fillColor)
                continue;
            if ((bset & 0x4) &&
                (rStyle._border != pStyle->_border ||
                 (rStyle._border == BORDER_SIMPLE_COLOR &&
                  rStyle._borderColor != pStyle->_borderColor)))
                continue;
            if ((bset & 0x8) &&
                !equalFont( rStyle, *pStyle ))
                continue;
            if ((bset & 0x40) &&
                rStyle._visualEffect != pStyle->_visualEffect)
                continue;

            // merge in
            short bnset = rStyle._set & ~pStyle->_set;
            if (bnset & 0x1)
                pStyle->_backgroundColor = rStyle._backgroundColor;
            if (bnset & 0x2)
                pStyle->_textColor = rStyle._textColor;
            if (bnset & 0x20)
                pStyle->_textLineColor = rStyle._textLineColor;
            if (bnset & 0x10)
                pStyle->_fillColor = rStyle._fillColor;
            if (bnset & 0x4) {
                pStyle->_border = rStyle._border;
                pStyle->_borderColor = rStyle._borderColor;
            }
            if (bnset & 0x8) {
                pStyle->_descr = rStyle._descr;
                pStyle->_fontRelief = rStyle._fontRelief;
                pStyle->_fontEmphasisMark = rStyle._fontEmphasisMark;
            }
            if (bnset & 0x40)
                pStyle->_visualEffect = rStyle._visualEffect;

            pStyle->_all |= rStyle._all;
            pStyle->_set |= rStyle._set;

            return pStyle->_id;
        }
    }

    // no appr style found, append new
    Style * pStyle = new Style( rStyle );
    pStyle->_id = OUString::valueOf( (sal_Int32)_styles.size() );
    _styles.push_back( pStyle );
    return pStyle->_id;
}
//__________________________________________________________________________________________________
StyleBag::~StyleBag() SAL_THROW(())
{
    for ( size_t nPos = 0; nPos < _styles.size(); ++nPos )
    {
        delete _styles[ nPos ];
    }
}
//__________________________________________________________________________________________________
void StyleBag::dump( Reference< xml::sax::XExtendedDocumentHandler > const & xOut )
{
    if (! _styles.empty())
    {
        OUString aStylesName( XMLNS_DIALOGS_PREFIX ":styles" );
        xOut->ignorableWhitespace( OUString() );
        xOut->startElement( aStylesName, Reference< xml::sax::XAttributeList >() );
        // export styles
        for ( size_t nPos = 0; nPos < _styles.size(); ++nPos )
        {
            Reference< xml::sax::XAttributeList > xAttr( _styles[ nPos ]->createElement() );
            static_cast< ElementDescriptor * >( xAttr.get() )->dump( xOut.get() );
        }
        xOut->ignorableWhitespace( OUString() );
        xOut->endElement( aStylesName );
    }
}

//##################################################################################################

//==================================================================================================
void SAL_CALL exportDialogModel(
    Reference< xml::sax::XExtendedDocumentHandler > const & xOut,
    Reference< container::XNameContainer > const & xDialogModel,
    Reference< frame::XModel > const & xDocument )
    SAL_THROW( (Exception) )
{
    StyleBag all_styles;
    // window
    Reference< beans::XPropertySet > xProps( xDialogModel, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    Reference< beans::XPropertyState > xPropState( xProps, UNO_QUERY );
    OSL_ASSERT( xPropState.is() );

    ElementDescriptor * pElem = new ElementDescriptor( xProps, xPropState, XMLNS_DIALOGS_PREFIX ":bulletinboard", xDocument );
    Reference< xml::sax::XAttributeList > xElem( pElem );
    pElem->readBullitinBoard( &all_styles );

    xOut->startDocument();

    OUString aDocTypeStr(
        "<!DOCTYPE dlg:window PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"dialog.dtd\">" );
    xOut->unknown( aDocTypeStr );
    xOut->ignorableWhitespace( OUString() );


    OUString aWindowName( XMLNS_DIALOGS_PREFIX ":window" );
    ElementDescriptor * pWindow = new ElementDescriptor( xProps, xPropState, aWindowName, xDocument );
    Reference< xml::sax::XAttributeList > xWindow( pWindow );
    pWindow->readDialogModel( &all_styles );
    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aWindowName, xWindow );
     // dump out events
    pWindow->dumpSubElements( xOut.get() );
    // dump out stylebag
    all_styles.dump( xOut );

    if ( xDialogModel->getElementNames().getLength() )
    {
        // open up bulletinboard
        OUString aBBoardName( XMLNS_DIALOGS_PREFIX ":bulletinboard" );
        xOut->ignorableWhitespace( OUString() );
        xOut->startElement( aBBoardName, xElem );

        pElem->dumpSubElements( xOut.get() );
        // end bulletinboard
        xOut->ignorableWhitespace( OUString() );
        xOut->endElement( aBBoardName );
    }

    // end window
    xOut->ignorableWhitespace( OUString() );
    xOut->endElement( aWindowName );

    xOut->endDocument();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
