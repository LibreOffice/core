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

#include "common.hxx"
#include "exp_share.hxx"
#include <xmlscript/xmlns.h>

#include <o3tl/any.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <tools/date.hxx>
#include <tools/time.hxx>

#include <com/sun/star/io/XPersistObject.hpp>

#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>

#include <com/sun/star/style/VerticalAlignment.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <com/sun/star/view/SelectionType.hpp>

#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/GraphicStorageHandler.hpp>
#include <com/sun/star/document/XGraphicStorageHandler.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>

#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <rtl/ref.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

Reference< xml::sax::XAttributeList > Style::createElement()
{
    rtl::Reference<ElementDescriptor> pStyle = new ElementDescriptor( u"" XMLNS_DIALOGS_PREFIX ":style"_ustr );

    // style-id
    pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":style-id"_ustr, _id );

    // background-color
    if (_set & 0x1)
    {
        pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":background-color"_ustr, "0x" + OUString::number(_backgroundColor,16));
    }

    // text-color
    if (_set & 0x2)
    {
        pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":text-color"_ustr, "0x" + OUString::number(_textColor,16));
    }

    // textline-color
    if (_set & 0x20)
    {
        pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":textline-color"_ustr, "0x" + OUString::number(_textLineColor,16));
    }

    // fill-color
    if (_set & 0x10)
    {
        pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":fill-color"_ustr, "0x" + OUString::number(_fillColor,16));
    }

    // border
    if (_set & 0x4)
    {
        switch (_border)
        {
        case BORDER_NONE:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":border"_ustr, u"none"_ustr );
            break;
        case BORDER_3D:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":border"_ustr, u"3d"_ustr );
            break;
        case BORDER_SIMPLE:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":border"_ustr, u"simple"_ustr );
            break;
        case BORDER_SIMPLE_COLOR: {
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":border"_ustr, "0x" + OUString::number(_borderColor,16));
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
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":look"_ustr, u"none"_ustr );
            break;
        case awt::VisualEffect::LOOK3D:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":look"_ustr, u"3d"_ustr );
            break;
        case awt::VisualEffect::FLAT:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":look"_ustr, u"simple"_ustr );
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
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-name"_ustr, _descr.Name );
        }
        // dialog:font-height %numeric; #IMPLIED
        if (def_descr.Height != _descr.Height)
        {
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-height"_ustr, OUString::number( _descr.Height ) );
        }
        // dialog:font-width %numeric; #IMPLIED
        if (def_descr.Width != _descr.Width)
        {
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-width"_ustr, OUString::number( _descr.Width ) );
        }
        // dialog:font-stylename CDATA #IMPLIED
        if (def_descr.StyleName != _descr.StyleName)
        {
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-stylename"_ustr, _descr.StyleName );
        }
        // dialog:font-family "(decorative|modern|roman|script|swiss|system)" #IMPLIED
        if (def_descr.Family != _descr.Family)
        {
            switch (_descr.Family)
            {
            case awt::FontFamily::DECORATIVE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-family"_ustr, u"decorative"_ustr );
                break;
            case awt::FontFamily::MODERN:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-family"_ustr, u"modern"_ustr );
                break;
            case awt::FontFamily::ROMAN:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-family"_ustr, u"roman"_ustr );
                break;
            case awt::FontFamily::SCRIPT:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-family"_ustr, u"script"_ustr );
                break;
            case awt::FontFamily::SWISS:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-family"_ustr, u"swiss"_ustr );
                break;
            case awt::FontFamily::SYSTEM:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-family"_ustr, u"system"_ustr );
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
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"ansi"_ustr );
                break;
            case awt::CharSet::MAC:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"mac"_ustr );
                break;
            case awt::CharSet::IBMPC_437:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"ibmpc_437"_ustr );
                break;
            case awt::CharSet::IBMPC_850:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"ibmpc_850"_ustr );
                break;
            case awt::CharSet::IBMPC_860:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"ibmpc_860"_ustr );
                break;
            case awt::CharSet::IBMPC_861:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"ibmpc_861"_ustr );
                break;
            case awt::CharSet::IBMPC_863:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"ibmpc_863"_ustr );
                break;
            case awt::CharSet::IBMPC_865:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"ibmpc_865"_ustr );
                break;
            case awt::CharSet::SYSTEM:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"system"_ustr );
                break;
            case awt::CharSet::SYMBOL:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charset"_ustr, u"symbol"_ustr );
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
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-pitch"_ustr, u"fixed"_ustr );
                break;
            case awt::FontPitch::VARIABLE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-pitch"_ustr, u"variable"_ustr );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-pitch!" );
                break;
            }
        }
        // dialog:font-charwidth CDATA #IMPLIED
        if (def_descr.CharacterWidth != _descr.CharacterWidth)
        {
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-charwidth"_ustr, OUString::number( _descr.CharacterWidth ) );
        }
        // dialog:font-weight CDATA #IMPLIED
        if (def_descr.Weight != _descr.Weight)
        {
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-weight"_ustr, OUString::number( _descr.Weight ) );
        }
        // dialog:font-slant "(oblique|italic|reverse_oblique|reverse_italic)" #IMPLIED
        if (def_descr.Slant != _descr.Slant)
        {
            switch (_descr.Slant)
            {
            case awt::FontSlant_OBLIQUE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-slant"_ustr, u"oblique"_ustr );
                break;
            case awt::FontSlant_ITALIC:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-slant"_ustr, u"italic"_ustr );
                break;
            case awt::FontSlant_REVERSE_OBLIQUE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-slant"_ustr, u"reverse_oblique"_ustr );
                break;
            case awt::FontSlant_REVERSE_ITALIC:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-slant"_ustr, u"reverse_italic"_ustr );
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
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"single"_ustr );
                break;
            case awt::FontUnderline::DOUBLE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"double"_ustr );
                break;
            case awt::FontUnderline::DOTTED:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"dotted"_ustr );
                break;
            case awt::FontUnderline::DASH:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"dash"_ustr );
                break;
            case awt::FontUnderline::LONGDASH:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"longdash"_ustr );
                break;
            case awt::FontUnderline::DASHDOT:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"dashdot"_ustr );
                break;
            case awt::FontUnderline::DASHDOTDOT:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"dashdotdot"_ustr );
                break;
            case awt::FontUnderline::SMALLWAVE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"smallwave"_ustr );
                break;
            case awt::FontUnderline::WAVE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"wave"_ustr );
                break;
            case awt::FontUnderline::DOUBLEWAVE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"doublewave"_ustr );
                break;
            case awt::FontUnderline::BOLD:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"bold"_ustr );
                break;
            case awt::FontUnderline::BOLDDOTTED:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"bolddotted"_ustr );
                break;
            case awt::FontUnderline::BOLDDASH:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"bolddash"_ustr );
                break;
            case awt::FontUnderline::BOLDLONGDASH:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"boldlongdash"_ustr );
                break;
            case awt::FontUnderline::BOLDDASHDOT:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"bolddashdot"_ustr );
                break;
            case awt::FontUnderline::BOLDDASHDOTDOT:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"bolddashdotdot"_ustr );
                break;
            case awt::FontUnderline::BOLDWAVE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-underline"_ustr, u"boldwave"_ustr );
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
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-strikeout"_ustr, u"single"_ustr );
                break;
            case awt::FontStrikeout::DOUBLE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-strikeout"_ustr, u"double"_ustr );
                break;
            case awt::FontStrikeout::BOLD:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-strikeout"_ustr, u"bold"_ustr );
                break;
            case awt::FontStrikeout::SLASH:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-strikeout"_ustr, u"slash"_ustr );
                break;
            case awt::FontStrikeout::X:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-strikeout"_ustr, u"x"_ustr );
                break;
            default:
                SAL_WARN( "xmlscript.xmldlg", "### unexpected font-strikeout!" );
                break;
            }
        }
        // dialog:font-orientation CDATA #IMPLIED
        if (def_descr.Orientation != _descr.Orientation)
        {
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-orientation"_ustr, OUString::number( _descr.Orientation ) );
        }
        // dialog:font-kerning %boolean; #IMPLIED
        if (bool(def_descr.Kerning) != bool(_descr.Kerning))
        {
            pStyle->addBoolAttr( u"" XMLNS_DIALOGS_PREFIX ":font-kerning"_ustr, _descr.Kerning );
        }
        // dialog:font-wordlinemode %boolean; #IMPLIED
        if (bool(def_descr.WordLineMode) != bool(_descr.WordLineMode))
        {
            pStyle->addBoolAttr( u"" XMLNS_DIALOGS_PREFIX ":font-wordlinemode"_ustr, _descr.WordLineMode );
        }
        // dialog:font-type "(raster|device|scalable)" #IMPLIED
        if (def_descr.Type != _descr.Type)
        {
            switch (_descr.Type)
            {
            case awt::FontType::RASTER:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-type"_ustr, u"raster"_ustr );
                break;
            case awt::FontType::DEVICE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-type"_ustr, u"device"_ustr );
                break;
            case awt::FontType::SCALABLE:
                pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-type"_ustr, u"scalable"_ustr );
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
        case awt::FontRelief::NONE: // don't export default
            break;
        case awt::FontRelief::EMBOSSED:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-relief"_ustr, u"embossed"_ustr );
            break;
        case awt::FontRelief::ENGRAVED:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-relief"_ustr, u"engraved"_ustr );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### unexpected font-relief!" );
            break;
        }
        // dialog:font-emphasismark (none|dot|circle|disc|accent|above|below) #IMPLIED
        switch (_fontEmphasisMark)
        {
        case awt::FontEmphasisMark::NONE: // don't export default
            break;
        case awt::FontEmphasisMark::DOT:
            pStyle->addAttribute(u"" XMLNS_DIALOGS_PREFIX ":font-emphasismark"_ustr, u"dot"_ustr );
            break;
        case awt::FontEmphasisMark::CIRCLE:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-emphasismark"_ustr, u"circle"_ustr );
            break;
        case awt::FontEmphasisMark::DISC:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-emphasismark"_ustr, u"disc"_ustr );
            break;
        case awt::FontEmphasisMark::ACCENT:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-emphasismark"_ustr, u"accent"_ustr );
            break;
        case awt::FontEmphasisMark::ABOVE:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-emphasismark"_ustr, u"above"_ustr );
            break;
        case awt::FontEmphasisMark::BELOW:
            pStyle->addAttribute( u"" XMLNS_DIALOGS_PREFIX ":font-emphasismark"_ustr, u"below"_ustr );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### unexpected font-emphasismark!" );
            break;
        }
    }

    return pStyle;
}

void ElementDescriptor::addNumberFormatAttr(
    Reference< beans::XPropertySet > const & xFormatProperties )
{
    OUString sFormat;
    lang::Locale locale;
    OSL_VERIFY( xFormatProperties->getPropertyValue( u"FormatString"_ustr ) >>= sFormat );
    OSL_VERIFY( xFormatProperties->getPropertyValue( u"Locale"_ustr ) >>= locale );

    addAttribute(u"" XMLNS_DIALOGS_PREFIX ":format-code"_ustr, sFormat );

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
    addAttribute( u"" XMLNS_DIALOGS_PREFIX ":format-locale"_ustr, aStr );
}

Any ElementDescriptor::readProp( OUString const & rPropName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        return _xProps->getPropertyValue( rPropName );
    }
    return Any();
}

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

void ElementDescriptor::readHexLongAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState( rPropName ))
    {
        Any a( _xProps->getPropertyValue( rPropName ) );
        if (auto n = o3tl::tryAccess<sal_uInt32>(a))
        {
            addAttribute( rAttrName, "0x" + OUString::number(*n, 16)  );
        }
    }
}

void ElementDescriptor::readDateFormatAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    if (auto n = o3tl::tryAccess<sal_Int16>(a))
    {
        switch (*n)
        {
        case 0:
            addAttribute( rAttrName, u"system_short"_ustr );
            break;
        case 1:
            addAttribute( rAttrName, u"system_short_YY"_ustr );
            break;
        case 2:
            addAttribute( rAttrName, u"system_short_YYYY"_ustr );
            break;
        case 3:
            addAttribute( rAttrName, u"system_long"_ustr );
            break;
        case 4:
            addAttribute( rAttrName, u"short_DDMMYY"_ustr );
            break;
        case 5:
            addAttribute( rAttrName, u"short_MMDDYY"_ustr );
            break;
        case 6:
            addAttribute( rAttrName, u"short_YYMMDD"_ustr );
            break;
        case 7:
            addAttribute( rAttrName, u"short_DDMMYYYY"_ustr );
            break;
        case 8:
            addAttribute( rAttrName, u"short_MMDDYYYY"_ustr );
            break;
        case 9:
            addAttribute( rAttrName, u"short_YYYYMMDD"_ustr );
            break;
        case 10:
            addAttribute( rAttrName, u"short_YYMMDD_DIN5008"_ustr );
            break;
        case 11:
            addAttribute( rAttrName, u"short_YYYYMMDD_DIN5008"_ustr );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### unexpected date format!" );
            break;
        }
    }
    else
        OSL_FAIL( "### unexpected property type!" );
}

void ElementDescriptor::readDateAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    if (a.getValueTypeClass() == TypeClass_STRUCT && a.getValueType() == cppu::UnoType<util::Date>::get())
    {
        util::Date aUDate;
        if (a >>= aUDate)
        {
            ::Date aTDate(aUDate);
            addAttribute( rAttrName, OUString::number( aTDate.GetDate() ) );
        }
        else
            OSL_FAIL( "### internal error" );
    }
    else
        OSL_FAIL( "### unexpected property type!" );
}

void ElementDescriptor::readTimeAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    if (a.getValueTypeClass() == TypeClass_STRUCT && a.getValueType() == cppu::UnoType<util::Time>::get())
    {
        util::Time aUTime;
        if (a >>= aUTime)
        {
            ::tools::Time aTTime(aUTime);
            addAttribute( rAttrName, OUString::number( aTTime.GetTime() / ::tools::Time::nanoPerCenti ) );
        }
        else
            OSL_FAIL( "### internal error" );
    }
    else
        OSL_FAIL( "### unexpected property type!" );
}

void ElementDescriptor::readTimeFormatAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    if (auto n = o3tl::tryAccess<sal_Int16>(a))
    {
        switch (*n)
        {
        case 0:
            addAttribute( rAttrName, u"24h_short"_ustr );
            break;
        case 1:
            addAttribute( rAttrName, u"24h_long"_ustr );
            break;
        case 2:
            addAttribute( rAttrName, u"12h_short"_ustr );
            break;
        case 3:
            addAttribute( rAttrName, u"12h_long"_ustr );
            break;
        case 4:
            addAttribute( rAttrName, u"Duration_short"_ustr );
            break;
        case 5:
            addAttribute( rAttrName, u"Duration_long"_ustr );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### unexpected time format!" );
            break;
        }
    }
    else
        OSL_FAIL( "### unexpected property type!" );
}

void ElementDescriptor::readAlignAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    if (auto n = o3tl::tryAccess<sal_Int16>(a))
    {
        switch (*n)
        {
        case 0:
            addAttribute( rAttrName, u"left"_ustr );
            break;
        case 1:
            addAttribute( rAttrName, u"center"_ustr );
            break;
        case 2:
            addAttribute( rAttrName, u"right"_ustr );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### illegal alignment value!" );
            break;
        }
    }
    else
        OSL_FAIL( "### unexpected property type!" );
}

void ElementDescriptor::readVerticalAlignAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    if (a.getValueTypeClass() == TypeClass_ENUM && a.getValueType() == cppu::UnoType<style::VerticalAlignment>::get())
    {
        style::VerticalAlignment eAlign;
        a >>= eAlign;
        switch (eAlign)
        {
        case style::VerticalAlignment_TOP:
            addAttribute( rAttrName, u"top"_ustr );
            break;
        case style::VerticalAlignment_MIDDLE:
            addAttribute( rAttrName, u"center"_ustr );
            break;
        case style::VerticalAlignment_BOTTOM:
            addAttribute( rAttrName, u"bottom"_ustr );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### illegal vertical alignment value!" );
            break;
        }
    }
    else
        OSL_FAIL( "### unexpected property type!" );
}

void ElementDescriptor::readImageOrGraphicAttr(OUString const & rAttrName)
{
    OUString sURL;
    if (beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState(u"Graphic"_ustr))
    {
        uno::Reference<graphic::XGraphic> xGraphic;
        _xProps->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
        if (xGraphic.is())
        {
            Reference< document::XStorageBasedDocument > xDocStorage( _xDocument, UNO_QUERY );
            if ( xDocStorage.is() )
            {
                Reference<XComponentContext> xContext = ::comphelper::getProcessComponentContext();
                uno::Reference<document::XGraphicStorageHandler> xGraphicStorageHandler;
                xGraphicStorageHandler.set(document::GraphicStorageHandler::createWithStorage(xContext, xDocStorage->getDocumentStorage()));
                if (xGraphicStorageHandler.is())
                {
                    sURL = xGraphicStorageHandler->saveGraphic(xGraphic);
                }
            }
        }
    }
    // tdf#130793 Above fails if the dialog is not part of a document. Export the ImageURL then.
    if (sURL.isEmpty()
        && beans::PropertyState_DEFAULT_VALUE != _xPropState->getPropertyState(u"ImageURL"_ustr))
    {
        _xProps->getPropertyValue(u"ImageURL"_ustr) >>= sURL;
    }
    if (!sURL.isEmpty())
        addAttribute(rAttrName, sURL);
}

void ElementDescriptor::readImageAlignAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    if (auto n = o3tl::tryAccess<sal_Int16>(a))
    {
        switch (*n)
        {
        case 0:
            addAttribute( rAttrName, u"left"_ustr );
            break;
        case 1:
            addAttribute( rAttrName, u"top"_ustr );
            break;
        case 2:
            addAttribute( rAttrName, u"right"_ustr );
            break;
        case 3:
            addAttribute( rAttrName, u"bottom"_ustr );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### illegal image alignment value!" );
            break;
        }
    }
    else
        OSL_FAIL( "### unexpected property type!" );
}

void ElementDescriptor::readImagePositionAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    auto n = o3tl::tryAccess<sal_Int16>(a);
    if (!n)
        return;

    switch (*n)
    {
    case awt::ImagePosition::LeftTop:
        addAttribute( rAttrName, u"left-top"_ustr );
        break;
    case awt::ImagePosition::LeftCenter:
        addAttribute( rAttrName, u"left-center"_ustr );
        break;
    case awt::ImagePosition::LeftBottom:
        addAttribute( rAttrName, u"left-bottom"_ustr );
        break;
    case awt::ImagePosition::RightTop:
        addAttribute( rAttrName, u"right-top"_ustr );
        break;
    case awt::ImagePosition::RightCenter:
        addAttribute( rAttrName, u"right-center"_ustr );
        break;
    case awt::ImagePosition::RightBottom:
        addAttribute( rAttrName, u"right-bottom"_ustr );
        break;
    case awt::ImagePosition::AboveLeft:
        addAttribute( rAttrName, u"top-left"_ustr );
        break;
    case awt::ImagePosition::AboveCenter:
        addAttribute( rAttrName, u"top-center"_ustr );
        break;
    case awt::ImagePosition::AboveRight:
        addAttribute( rAttrName, u"top-right"_ustr );
        break;
    case awt::ImagePosition::BelowLeft:
        addAttribute( rAttrName, u"bottom-left"_ustr );
        break;
    case awt::ImagePosition::BelowCenter:
        addAttribute( rAttrName, u"bottom-center"_ustr );
        break;
    case awt::ImagePosition::BelowRight:
        addAttribute( rAttrName, u"bottom-right"_ustr );
        break;
    case awt::ImagePosition::Centered:
        addAttribute( rAttrName, u"center"_ustr );
        break;
    default:
        SAL_WARN( "xmlscript.xmldlg", "### illegal image position value!" );
        break;
    }
}

void ElementDescriptor::readButtonTypeAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    auto n = o3tl::tryAccess<sal_Int16>(a);
    if (!n)
        return;

    switch (static_cast<awt::PushButtonType>(*n))
    {
    case awt::PushButtonType_STANDARD:
        addAttribute( rAttrName, u"standard"_ustr );
        break;
    case awt::PushButtonType_OK:
        addAttribute( rAttrName, u"ok"_ustr );
        break;
    case awt::PushButtonType_CANCEL:
        addAttribute( rAttrName, u"cancel"_ustr );
        break;
    case awt::PushButtonType_HELP:
        addAttribute( rAttrName, u"help"_ustr );
        break;
    default:
        SAL_WARN( "xmlscript.xmldlg", "### illegal button-type value!" );
        break;
    }
}

void ElementDescriptor::readOrientationAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    auto n = o3tl::tryAccess<sal_Int32>(a);
    if (!n)
        return;

    switch (*n)
    {
    case 0:
        addAttribute( rAttrName, u"horizontal"_ustr );
        break;
    case 1:
        addAttribute( rAttrName, u"vertical"_ustr );
        break;
    default:
        SAL_WARN( "xmlscript.xmldlg", "### illegal orientation value!" );
        break;
    }
}

void ElementDescriptor::readLineEndFormatAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any a( _xProps->getPropertyValue( rPropName ) );
    auto n = o3tl::tryAccess<sal_Int16>(a);
    if (!n)
        return;

    switch (*n)
    {
    case awt::LineEndFormat::CARRIAGE_RETURN:
        addAttribute( rAttrName, u"carriage-return"_ustr );
        break;
    case awt::LineEndFormat::LINE_FEED:
        addAttribute( rAttrName, u"line-feed"_ustr );
        break;
    case awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED:
        addAttribute( rAttrName, u"carriage-return-line-feed"_ustr );
        break;
    default:
        SAL_WARN( "xmlscript.xmldlg", "### illegal line end format value!" );
        break;
    }
}

void ElementDescriptor::readDataAwareAttr( OUString const & rAttrName )
{
    Reference< lang::XMultiServiceFactory > xFac;
    if ( _xDocument.is() )
        xFac.set( _xDocument, uno::UNO_QUERY );

    Reference< form::binding::XBindableValue > xBinding( _xProps, UNO_QUERY );

    if ( xFac.is() && xBinding.is() && rAttrName == XMLNS_DIALOGS_PREFIX ":linked-cell" )
    {
        try
        {
            Reference< beans::XPropertySet > xConvertor( xFac->createInstance( u"com.sun.star.table.CellAddressConversion"_ustr ), uno::UNO_QUERY );
            Reference< beans::XPropertySet > xBindable( xBinding->getValueBinding(), UNO_QUERY );
            if ( xBindable.is() )
            {
                table::CellAddress aAddress;
                xBindable->getPropertyValue( u"BoundCell"_ustr ) >>= aAddress;
                xConvertor->setPropertyValue( u"Address"_ustr, Any( aAddress ) );
                OUString sAddress;
                xConvertor->getPropertyValue( u"PersistentRepresentation"_ustr ) >>= sAddress;
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
    if ( !(xEntrySink.is() && rAttrName == XMLNS_DIALOGS_PREFIX ":source-cell-range") )
        return;

    Reference< beans::XPropertySet > xListSource( xEntrySink->getListEntrySource(), UNO_QUERY );
    if ( !xListSource.is() )
        return;

    try
    {
        Reference< beans::XPropertySet > xConvertor( xFac->createInstance( u"com.sun.star.table.CellRangeAddressConversion"_ustr ), uno::UNO_QUERY );

        table::CellRangeAddress aAddress;
        xListSource->getPropertyValue( u"CellRange"_ustr ) >>= aAddress;

        OUString sAddress;
        xConvertor->setPropertyValue( u"Address"_ustr, Any( aAddress ) );
        xConvertor->getPropertyValue( u"PersistentRepresentation"_ustr ) >>= sAddress;
        SAL_INFO("xmlscript.xmldlg","**** cell range source list " << sAddress );
        if ( !sAddress.isEmpty() )
            addAttribute( rAttrName, sAddress );
    }
    catch( uno::Exception& )
    {
    }
}

void ElementDescriptor::readSelectionTypeAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any aSelectionType ( _xProps->getPropertyValue( rPropName ) );

    if (aSelectionType.getValueTypeClass() != TypeClass_ENUM ||
        aSelectionType.getValueType() != cppu::UnoType<view::SelectionType>::get())
        return;

    ::view::SelectionType eSelectionType;
    aSelectionType >>= eSelectionType;

    switch (eSelectionType)
    {
        case ::view::SelectionType_NONE:
            addAttribute( rAttrName, u"none"_ustr );
            break;
        case ::view::SelectionType_SINGLE:
            addAttribute( rAttrName, u"single"_ustr );
            break;
        case ::view::SelectionType_MULTI:
            addAttribute( rAttrName, u"multi"_ustr );
            break;
        case ::view::SelectionType_RANGE:
            addAttribute( rAttrName, u"range"_ustr );
            break;
        default:
            SAL_WARN( "xmlscript.xmldlg", "### illegal selection type value!" );
            break;
    }
}

void ElementDescriptor::readScrollableSettings()
{
    readLongAttr( u"ScrollHeight"_ustr,
                  u"" XMLNS_DIALOGS_PREFIX ":scrollheight"_ustr );
    readLongAttr( u"ScrollWidth"_ustr,
                  u"" XMLNS_DIALOGS_PREFIX ":scrollwidth"_ustr );
    readLongAttr( u"ScrollTop"_ustr,
                  u"" XMLNS_DIALOGS_PREFIX ":scrolltop"_ustr );
    readLongAttr( u"ScrollLeft"_ustr,
                  u"" XMLNS_DIALOGS_PREFIX ":scrollleft"_ustr );
    readBoolAttr( u"HScroll"_ustr,
                  u"" XMLNS_DIALOGS_PREFIX ":hscroll"_ustr );
    readBoolAttr( u"VScroll"_ustr,
                  u"" XMLNS_DIALOGS_PREFIX ":vscroll"_ustr );
}

void ElementDescriptor::readImageScaleModeAttr( OUString const & rPropName, OUString const & rAttrName )
{
    if (beans::PropertyState_DEFAULT_VALUE == _xPropState->getPropertyState( rPropName ))
        return;

    Any aImageScaleMode( _xProps->getPropertyValue( rPropName ) );

    if (aImageScaleMode.getValueTypeClass() != TypeClass_SHORT)
        return;

    sal_Int16 nImageScaleMode = 0;
    aImageScaleMode >>= nImageScaleMode;

    switch(nImageScaleMode)
    {
        case ::awt::ImageScaleMode::NONE:
            addAttribute( rAttrName, u"none"_ustr );
            break;
        case ::awt::ImageScaleMode::ISOTROPIC:
            addAttribute( rAttrName, u"isotropic"_ustr );
            break;
        case ::awt::ImageScaleMode::ANISOTROPIC:
            addAttribute( rAttrName, u"anisotropic"_ustr );
            break;
        default:
            OSL_ENSURE( false, "### illegal image scale mode value.");
            break;
    }
}

void ElementDescriptor::readDefaults( bool supportPrintable, bool supportVisible )
{
    Any a( _xProps->getPropertyValue( u"Name"_ustr ) );

    // The following is a hack to allow 'form' controls to override the default
    // control supported by dialogs. This should work well for both VBA support and
    // normal LibreOffice (when normal 'Dialogs' decide to support form control models)
    // In the future VBA support might require custom models ( and not the just the form
    // variant of a control that we currently use ) In this case the door is still open,
    // we just need to define a new way for the 'ServiceName' to be extracted from the
    // incoming model. E.g. the use of supporting service
    // "com.sun.star.form.FormComponent", 'ServiceName' and XPersistObject
    // is only an implementation detail here, in the future some other
    // method (perhaps a custom prop) could be used instead.
    Reference< lang::XServiceInfo > xSrvInfo( _xProps, UNO_QUERY );
    if ( xSrvInfo.is() && xSrvInfo->supportsService( u"com.sun.star.form.FormComponent"_ustr ) )
    {
        Reference< io::XPersistObject > xPersist( _xProps, UNO_QUERY );
        if ( xPersist.is() )
        {
            OUString sCtrlName = xPersist->getServiceName();
            if ( !sCtrlName.isEmpty() )
                    addAttribute( u"" XMLNS_DIALOGS_PREFIX ":control-implementation"_ustr, sCtrlName );
        }
    }
    addAttribute( u"" XMLNS_DIALOGS_PREFIX ":id"_ustr, *o3tl::doAccess<OUString>(a) );
    readShortAttr( u"TabIndex"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tab-index"_ustr );

    bool bEnabled = false;
    if (_xProps->getPropertyValue( u"Enabled"_ustr ) >>= bEnabled)
    {
        if (! bEnabled)
        {
            addAttribute( u"" XMLNS_DIALOGS_PREFIX ":disabled"_ustr, u"true"_ustr );
        }
    }
    else
    {
        SAL_WARN( "xmlscript.xmldlg", "unexpected property type for \"Enabled\": not bool!" );
    }

    if (supportVisible) try
    {
        bool bVisible = true;
        if (_xProps->getPropertyValue(u"EnableVisible"_ustr ) >>= bVisible)
        {

            // only write out the non default case
            if (! bVisible)
            {
                addAttribute( u"" XMLNS_DIALOGS_PREFIX ":visible"_ustr, u"false"_ustr );
            }
        }
    }
    catch( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("xmlscript.xmldlg");
    }
    // force writing of pos/size
    a = _xProps->getPropertyValue( u"PositionX"_ustr );
    if (auto n = o3tl::tryAccess<sal_Int32>(a))
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":left"_ustr, OUString::number(*n) );
    }
    a = _xProps->getPropertyValue( u"PositionY"_ustr );
    if (auto n = o3tl::tryAccess<sal_Int32>(a))
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":top"_ustr, OUString::number(*n) );
    }
    a = _xProps->getPropertyValue( u"Width"_ustr );
    if (auto n = o3tl::tryAccess<sal_Int32>(a))
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":width"_ustr, OUString::number(*n) );
    }
    a = _xProps->getPropertyValue( u"Height"_ustr );
    if (auto n = o3tl::tryAccess<sal_Int32>(a))
    {
        addAttribute( u"" XMLNS_DIALOGS_PREFIX ":height"_ustr, OUString::number(*n) );
    }

    if (supportPrintable)
    {
        readBoolAttr( u"Printable"_ustr, u"" XMLNS_DIALOGS_PREFIX ":printable"_ustr );
    }
    readLongAttr( u"Step"_ustr, u"" XMLNS_DIALOGS_PREFIX ":page"_ustr );
    readStringAttr( u"Tag"_ustr, u"" XMLNS_DIALOGS_PREFIX ":tag"_ustr );
    readStringAttr( u"HelpText"_ustr, u"" XMLNS_DIALOGS_PREFIX ":help-text"_ustr );
    readStringAttr( u"HelpURL"_ustr, u"" XMLNS_DIALOGS_PREFIX ":help-url"_ustr );
}

void ElementDescriptor::readEvents()
{
    Reference< script::XScriptEventsSupplier > xSupplier( _xProps, UNO_QUERY );
    if (!xSupplier.is())
        return;

    Reference< container::XNameContainer > xEvents( xSupplier->getEvents() );
    if (!xEvents.is())
        return;

    const Sequence< OUString > aNames( xEvents->getElementNames() );
    for ( const auto& rName : aNames )
    {
        script::ScriptEventDescriptor descr;
        if (xEvents->getByName( rName ) >>= descr)
        {
            SAL_WARN_IF( descr.ListenerType.isEmpty() ||
                        descr.EventMethod.isEmpty() ||
                        descr.ScriptCode.isEmpty() ||
                        descr.ScriptType.isEmpty() , "xmlscript.xmldlg", "### invalid event descr!" );

            OUString aEventName;

            if (descr.AddListenerParam.isEmpty())
            {
                // detection of event-name
                StringTriple const * p = g_pEventTranslations;
                while (p->first)
                {
                    if (descr.EventMethod.equalsAscii(p->second) &&
                        descr.ListenerType.equalsAscii(p->first))
                    {
                        aEventName = OStringToOUString( p->third, RTL_TEXTENCODING_ASCII_US );
                        break;
                    }
                    ++p;
                }
            }

            rtl::Reference<ElementDescriptor> pElem;

            if (!aEventName.isEmpty()) // script:event
            {
                pElem = new ElementDescriptor( u"" XMLNS_SCRIPT_PREFIX ":event"_ustr );
                pElem->addAttribute( u"" XMLNS_SCRIPT_PREFIX ":event-name"_ustr, aEventName );
            }
            else // script:listener-event
            {
                pElem = new ElementDescriptor( u"" XMLNS_SCRIPT_PREFIX ":listener-event"_ustr );
                pElem->addAttribute( u"" XMLNS_SCRIPT_PREFIX ":listener-type"_ustr, descr.ListenerType );
                pElem->addAttribute( u"" XMLNS_SCRIPT_PREFIX ":listener-method"_ustr, descr.EventMethod );

                if (!descr.AddListenerParam.isEmpty())
                {
                    pElem->addAttribute( u"" XMLNS_SCRIPT_PREFIX ":listener-param"_ustr, descr.AddListenerParam );
                }
            }
            if ( descr.ScriptType == "StarBasic" )
            {
                // separate optional location
                sal_Int32 nIndex = descr.ScriptCode.indexOf( ':' );
                if (nIndex >= 0)
                {
                    pElem->addAttribute( u"" XMLNS_SCRIPT_PREFIX ":location"_ustr, descr.ScriptCode.copy( 0, nIndex ) );
                    pElem->addAttribute( u"" XMLNS_SCRIPT_PREFIX ":macro-name"_ustr, descr.ScriptCode.copy( nIndex +1 ) );
                }
                else
                {
                    pElem->addAttribute( u"" XMLNS_SCRIPT_PREFIX ":macro-name"_ustr, descr.ScriptCode );
                }
            }
            else
            {
                pElem->addAttribute(u"" XMLNS_SCRIPT_PREFIX ":macro-name"_ustr, descr.ScriptCode );
            }

            // language
            pElem->addAttribute( u"" XMLNS_SCRIPT_PREFIX ":language"_ustr, descr.ScriptType );

            addSubElement( pElem );
        }
        else
        {
            SAL_WARN( "xmlscript.xmldlg", "### unexpected event type in container!" );
        }
    }
}

static bool equalFont( Style const & style1, Style const & style2 )
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
        f1.Pitch == f2.Pitch &&
        f1.CharacterWidth == f2.CharacterWidth &&
        f1.Weight == f2.Weight &&
        f1.Slant == f2.Slant &&
        f1.Underline == f2.Underline &&
        f1.Strikeout == f2.Strikeout &&
        f1.Orientation == f2.Orientation &&
        bool(f1.Kerning) == bool(f2.Kerning) &&
        bool(f1.WordLineMode) == bool(f2.WordLineMode) &&
        f1.Type == f2.Type &&
        style1._fontRelief == style2._fontRelief &&
        style1._fontEmphasisMark == style2._fontEmphasisMark
        );
}

OUString StyleBag::getStyleId( Style const & rStyle )
{
    if (! rStyle._set) // nothing set
    {
        return OUString(); // everything default: no need to export a specific style
    }

    // lookup existing style
    for (auto & rExistingStyle : _styles)
    {
        short demanded_defaults = ~rStyle._set & rStyle._all;
        // test, if defaults are not set
        if ((~rExistingStyle._set & demanded_defaults) == demanded_defaults &&
            (rStyle._set & (rExistingStyle._all & ~rExistingStyle._set)) == 0)
        {
            short bset = rStyle._set & rExistingStyle._set;
            if ((bset & 0x1) &&
                rStyle._backgroundColor != rExistingStyle._backgroundColor)
                continue;
            if ((bset & 0x2) &&
                rStyle._textColor != rExistingStyle._textColor)
                continue;
            if ((bset & 0x20) &&
                rStyle._textLineColor != rExistingStyle._textLineColor)
                continue;
            if ((bset & 0x10) &&
                rStyle._fillColor != rExistingStyle._fillColor)
                continue;
            if ((bset & 0x4) &&
                (rStyle._border != rExistingStyle._border ||
                 (rStyle._border == BORDER_SIMPLE_COLOR &&
                  rStyle._borderColor != rExistingStyle._borderColor)))
                continue;
            if ((bset & 0x8) &&
                !equalFont( rStyle, rExistingStyle ))
                continue;
            if ((bset & 0x40) &&
                rStyle._visualEffect != rExistingStyle._visualEffect)
                continue;

            // merge in
            short bnset = rStyle._set & ~rExistingStyle._set;
            if (bnset & 0x1)
                rExistingStyle._backgroundColor = rStyle._backgroundColor;
            if (bnset & 0x2)
                rExistingStyle._textColor = rStyle._textColor;
            if (bnset & 0x20)
                rExistingStyle._textLineColor = rStyle._textLineColor;
            if (bnset & 0x10)
                rExistingStyle._fillColor = rStyle._fillColor;
            if (bnset & 0x4) {
                rExistingStyle._border = rStyle._border;
                rExistingStyle._borderColor = rStyle._borderColor;
            }
            if (bnset & 0x8) {
                rExistingStyle._descr = rStyle._descr;
                rExistingStyle._fontRelief = rStyle._fontRelief;
                rExistingStyle._fontEmphasisMark = rStyle._fontEmphasisMark;
            }
            if (bnset & 0x40)
                rExistingStyle._visualEffect = rStyle._visualEffect;

            rExistingStyle._all |= rStyle._all;
            rExistingStyle._set |= rStyle._set;

            return rExistingStyle._id;
        }
    }

    // no appr style found, append new
    Style aNewStyle( rStyle );
    aNewStyle._id = OUString::number( _styles.size() );
    _styles.push_back( aNewStyle );
    return _styles.back()._id;
}

StyleBag::~StyleBag()
{
}

void StyleBag::dump( Reference< xml::sax::XExtendedDocumentHandler > const & xOut )
{
    if ( _styles.empty())
        return;

    OUString aStylesName( u"" XMLNS_DIALOGS_PREFIX ":styles"_ustr );
    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aStylesName, Reference< xml::sax::XAttributeList >() );
    // export styles
    for (auto & _style : _styles)
    {
        Reference< xml::sax::XAttributeList > xAttr( _style.createElement() );
        static_cast< ElementDescriptor * >( xAttr.get() )->dump( xOut );
    }
    xOut->ignorableWhitespace( OUString() );
    xOut->endElement( aStylesName );
}

void exportDialogModel(
    Reference< xml::sax::XExtendedDocumentHandler > const & xOut,
    Reference< container::XNameContainer > const & xDialogModel,
    Reference< frame::XModel > const & xDocument )
{
    StyleBag all_styles;
    // window
    Reference< beans::XPropertySet > xProps( xDialogModel, UNO_QUERY );
    OSL_ASSERT( xProps.is() );
    Reference< beans::XPropertyState > xPropState( xProps, UNO_QUERY );
    OSL_ASSERT( xPropState.is() );

    rtl::Reference<ElementDescriptor> pElem = new ElementDescriptor( xProps, xPropState, u"" XMLNS_DIALOGS_PREFIX ":bulletinboard"_ustr, xDocument );
    pElem->readBullitinBoard( &all_styles );

    xOut->startDocument();

    xOut->unknown(
        u"<!DOCTYPE dlg:window PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"dialog.dtd\">"_ustr );
    xOut->ignorableWhitespace( OUString() );

    OUString aWindowName( u"" XMLNS_DIALOGS_PREFIX ":window"_ustr );
    rtl::Reference<ElementDescriptor> pWindow = new ElementDescriptor( xProps, xPropState, aWindowName, xDocument );
    pWindow->readDialogModel( &all_styles );
    xOut->ignorableWhitespace( OUString() );
    xOut->startElement( aWindowName, pWindow );
     // dump out events
    pWindow->dumpSubElements( xOut );
    // dump out stylebag
    all_styles.dump( xOut );

    if ( xDialogModel->getElementNames().hasElements() )
    {
        // open up bulletinboard
        OUString aBBoardName( u"" XMLNS_DIALOGS_PREFIX ":bulletinboard"_ustr );
        xOut->ignorableWhitespace( OUString() );
        xOut->startElement( aBBoardName, pElem );

        pElem->dumpSubElements( xOut );
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
