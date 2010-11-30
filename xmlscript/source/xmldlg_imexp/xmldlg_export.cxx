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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmlscript.hxx"
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
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>

#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

namespace xmlscript
{

//__________________________________________________________________________________________________
Reference< xml::sax::XAttributeList > Style::createElement()
{
    ElementDescriptor * pStyle = new ElementDescriptor(
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style") ) );

    // style-id
    pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":style-id") ), _id );

    // background-color
    if (_set & 0x1)
    {
        OUStringBuffer buf( 16 );
        buf.append( (sal_Unicode)'0' );
        buf.append( (sal_Unicode)'x' );
        buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)_backgroundColor, 16 ) );
        pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":background-color") ),
                              buf.makeStringAndClear() );
    }

    // text-color
    if (_set & 0x2)
    {
        OUStringBuffer buf( 16 );
        buf.append( (sal_Unicode)'0' );
        buf.append( (sal_Unicode)'x' );
        buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)_textColor, 16 ) );
        pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":text-color") ),
                              buf.makeStringAndClear() );
    }

    // textline-color
    if (_set & 0x20)
    {
        OUStringBuffer buf( 16 );
        buf.append( (sal_Unicode)'0' );
        buf.append( (sal_Unicode)'x' );
        buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)_textLineColor, 16 ) );
        pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":textline-color") ),
                              buf.makeStringAndClear() );
    }

    // fill-color
    if (_set & 0x10)
    {
        OUStringBuffer buf( 16 );
        buf.append( (sal_Unicode)'0' );
        buf.append( (sal_Unicode)'x' );
        buf.append( OUString::valueOf( (sal_Int64)(sal_uInt64)_fillColor, 16 ) );
        pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":fill-color") ),
                              buf.makeStringAndClear() );
    }

    // border
    if (_set & 0x4)
    {
        switch (_border)
        {
        case BORDER_NONE:
            pStyle->addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":border"),
                                  OUSTR("none") );
            break;
        case BORDER_3D:
            pStyle->addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":border"),
                                  OUSTR("3d") );
            break;
        case BORDER_SIMPLE:
            pStyle->addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":border"),
                                  OUSTR("simple") );
            break;
        case BORDER_SIMPLE_COLOR: {
            OUStringBuffer buf;
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("0x") );
            buf.append( OUString::valueOf(
                            (sal_Int64)(sal_uInt64)_borderColor, 16 ) );
            pStyle->addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":border"),
                                  buf.makeStringAndClear() );
            break;
        }
        default:
            OSL_ENSURE( 0, "### unexpected border value!" );
            break;
        }
    }

    // visual effect (look)
    if (_set & 0x40)
    {
        switch (_visualEffect)
        {
        case awt::VisualEffect::NONE:
            pStyle->addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":look"),
                                  OUSTR("none") );
            break;
        case awt::VisualEffect::LOOK3D:
            pStyle->addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":look"),
                                  OUSTR("3d") );
            break;
        case awt::VisualEffect::FLAT:
            pStyle->addAttribute( OUSTR(XMLNS_DIALOGS_PREFIX ":look"),
                                  OUSTR("simple") );
            break;
        default:
            OSL_ENSURE( 0, "### unexpected visual effect value!" );
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
            pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-name") ),
                                  _descr.Name );
        }
        // dialog:font-height %numeric; #IMPLIED
        if (def_descr.Height != _descr.Height)
        {
            pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-height") ),
                                  OUString::valueOf( (sal_Int32)_descr.Height ) );
        }
        // dialog:font-width %numeric; #IMPLIED
        if (def_descr.Width != _descr.Width)
        {
            pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-width") ),
                                  OUString::valueOf( (sal_Int32)_descr.Width ) );
        }
        // dialog:font-stylename CDATA #IMPLIED
        if (def_descr.StyleName != _descr.StyleName)
        {
            pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-stylename") ),
                                  _descr.StyleName );
        }
        // dialog:font-family "(decorative|modern|roman|script|swiss|system)" #IMPLIED
        if (def_descr.Family != _descr.Family)
        {
            switch (_descr.Family)
            {
            case awt::FontFamily::DECORATIVE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-family") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("decorative") ) );
                break;
            case awt::FontFamily::MODERN:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-family") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("modern") ) );
                break;
            case awt::FontFamily::ROMAN:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-family") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("roman") ) );
                break;
            case awt::FontFamily::SCRIPT:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-family") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("script") ) );
                break;
            case awt::FontFamily::SWISS:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-family") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("swiss") ) );
                break;
            case awt::FontFamily::SYSTEM:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-family") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("system") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected font-family!" );
                break;
            }
        }
        // dialog:font-charset "(ansi|mac|ibmpc_437|ibmpc_850|ibmpc_860|ibmpc_861|ibmpc_863|ibmpc_865|system|symbol)" #IMPLIED
        if (def_descr.CharSet != _descr.CharSet)
        {
            switch (_descr.CharSet)
            {
            case awt::CharSet::ANSI:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("ansi") ) );
                break;
            case awt::CharSet::MAC:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("mac") ) );
                break;
            case awt::CharSet::IBMPC_437:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("ibmpc_437") ) );
                break;
            case awt::CharSet::IBMPC_850:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("ibmpc_850") ) );
                break;
            case awt::CharSet::IBMPC_860:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("ibmpc_860") ) );
                break;
            case awt::CharSet::IBMPC_861:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("ibmpc_861") ) );
                break;
            case awt::CharSet::IBMPC_863:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("ibmpc_863") ) );
                break;
            case awt::CharSet::IBMPC_865:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("ibmpc_865") ) );
                break;
            case awt::CharSet::SYSTEM:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("system") ) );
                break;
            case awt::CharSet::SYMBOL:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charset") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("symbol") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected font-charset!" );
                break;
            }
        }
        // dialog:font-pitch "(fixed|variable)" #IMPLIED
        if (def_descr.Pitch != _descr.Pitch)
        {
            switch (_descr.Pitch)
            {
            case awt::FontPitch::FIXED:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-pitch") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("fixed") ) );
                break;
            case awt::FontPitch::VARIABLE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-pitch") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("variable") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected font-pitch!" );
                break;
            }
        }
        // dialog:font-charwidth CDATA #IMPLIED
        if (def_descr.CharacterWidth != _descr.CharacterWidth)
        {
            pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-charwidth") ),
                                  OUString::valueOf( (float)_descr.CharacterWidth ) );
        }
        // dialog:font-weight CDATA #IMPLIED
        if (def_descr.Weight != _descr.Weight)
        {
            pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-weight") ),
                                  OUString::valueOf( (float)_descr.Weight ) );
        }
        // dialog:font-slant "(oblique|italic|reverse_oblique|reverse_italic)" #IMPLIED
        if (def_descr.Slant != _descr.Slant)
        {
            switch (_descr.Slant)
            {
            case awt::FontSlant_OBLIQUE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-slant") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("oblique") ) );
                break;
            case awt::FontSlant_ITALIC:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-slant") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("italic") ) );
                break;
            case awt::FontSlant_REVERSE_OBLIQUE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-slant") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("reverse_oblique") ) );
                break;
            case awt::FontSlant_REVERSE_ITALIC:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-slant") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("reverse_italic") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected font-slant!" );
                break;
            }
        }
        // dialog:font-underline "(single|double|dotted|dash|longdash|dashdot|dashdotdot|smallwave|wave|doublewave|bold|bolddotted|bolddash|boldlongdash|bolddashdot|bolddashdotdot|boldwave)" #IMPLIED
        if (def_descr.Underline != _descr.Underline)
        {
            switch (_descr.Underline)
            {
            case awt::FontUnderline::SINGLE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("single") ) );
                break;
            case awt::FontUnderline::DOUBLE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("double") ) );
                break;
            case awt::FontUnderline::DOTTED:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("dotted") ) );
                break;
            case awt::FontUnderline::DASH:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("dash") ) );
                break;
            case awt::FontUnderline::LONGDASH:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("longdash") ) );
                break;
            case awt::FontUnderline::DASHDOT:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("dashdot") ) );
                break;
            case awt::FontUnderline::DASHDOTDOT:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("dashdotdot") ) );
                break;
            case awt::FontUnderline::SMALLWAVE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("smallwave") ) );
                break;
            case awt::FontUnderline::WAVE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("wave") ) );
                break;
            case awt::FontUnderline::DOUBLEWAVE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("doublewave") ) );
                break;
            case awt::FontUnderline::BOLD:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("bold") ) );
                break;
            case awt::FontUnderline::BOLDDOTTED:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("bolddotted") ) );
                break;
            case awt::FontUnderline::BOLDDASH:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("bolddash") ) );
                break;
            case awt::FontUnderline::BOLDLONGDASH:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("boldlongdash") ) );
                break;
            case awt::FontUnderline::BOLDDASHDOT:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("bolddashdot") ) );
                break;
            case awt::FontUnderline::BOLDDASHDOTDOT:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("bolddashdotdot") ) );
                break;
            case awt::FontUnderline::BOLDWAVE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-underline") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("boldwave") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected font-underline!" );
                break;
            }
        }
        // dialog:font-strikeout "(single|double|bold|slash|x)" #IMPLIED
        if (def_descr.Strikeout != _descr.Strikeout)
        {
            switch (_descr.Strikeout)
            {
            case awt::FontStrikeout::SINGLE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-strikeout") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("single") ) );
                break;
            case awt::FontStrikeout::DOUBLE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-strikeout") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("double") ) );
                break;
            case awt::FontStrikeout::BOLD:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-strikeout") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("bold") ) );
                break;
            case awt::FontStrikeout::SLASH:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-strikeout") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("slash") ) );
                break;
            case awt::FontStrikeout::X:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-strikeout") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("x") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected font-strikeout!" );
                break;
            }
        }
        // dialog:font-orientation CDATA #IMPLIED
        if (def_descr.Orientation != _descr.Orientation)
        {
            pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-orientation") ),
                                  OUString::valueOf( (float)_descr.Orientation ) );
        }
        // dialog:font-kerning %boolean; #IMPLIED
        if ((def_descr.Kerning != sal_False) != (_descr.Kerning != sal_False))
        {
            pStyle->addBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-kerning") ),
                                    _descr.Kerning );
        }
        // dialog:font-wordlinemode %boolean; #IMPLIED
        if ((def_descr.WordLineMode != sal_False) != (_descr.WordLineMode != sal_False))
        {
            pStyle->addBoolAttr( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-wordlinemode") ),
                                 _descr.WordLineMode );
        }
        // dialog:font-type "(raster|device|scalable)" #IMPLIED
        if (def_descr.Type != _descr.Type)
        {
            switch (_descr.Type)
            {
            case awt::FontType::RASTER:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-type") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("raster") ) );
                break;
            case awt::FontType::DEVICE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-type") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("device") ) );
                break;
            case awt::FontType::SCALABLE:
                pStyle->addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-type") ),
                                      OUString( RTL_CONSTASCII_USTRINGPARAM("scalable") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected font-type!" );
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
            pStyle->addAttribute(
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-relief") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("embossed") ) );
            break;
        case awt::FontRelief::ENGRAVED:
            pStyle->addAttribute(
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-relief") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("engraved") ) );
            break;
        default:
            OSL_ENSURE( 0, "### unexpected font-relief!" );
            break;
        }
        // dialog:font-emphasismark (none|dot|circle|disc|accent|above|below) #IMPLIED
        switch (_fontEmphasisMark)
        {
        case awt::FontEmphasisMark::NONE: // dont export default
            break;
        case awt::FontEmphasisMark::DOT:
            pStyle->addAttribute(
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-emphasismark") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("dot") ) );
            break;
        case awt::FontEmphasisMark::CIRCLE:
            pStyle->addAttribute(
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-emphasismark") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("circle") ) );
            break;
        case awt::FontEmphasisMark::DISC:
            pStyle->addAttribute(
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-emphasismark") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("disc") ) );
            break;
        case awt::FontEmphasisMark::ACCENT:
            pStyle->addAttribute(
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-emphasismark") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("accent") ) );
            break;
        case awt::FontEmphasisMark::ABOVE:
            pStyle->addAttribute(
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-emphasismark") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("above") ) );
            break;
        case awt::FontEmphasisMark::BELOW:
            pStyle->addAttribute(
                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":font-emphasismark") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("below") ) );
            break;
        default:
            OSL_ENSURE( 0, "### unexpected font-emphasismark!" );
            break;
        }
    }

    return pStyle;
}

//##################################################################################################

//__________________________________________________________________________________________________
void ElementDescriptor::addNumberFormatAttr(
    Reference< beans::XPropertySet > const & xFormatProperties,
    OUString const & /*rAttrName*/ )
{
    Reference< beans::XPropertyState > xState( xFormatProperties, UNO_QUERY );
    OUString sFormat;
    lang::Locale locale;
    OSL_VERIFY( xFormatProperties->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("FormatString") ) ) >>= sFormat );
    OSL_VERIFY( xFormatProperties->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Locale") ) ) >>= locale );

    addAttribute(
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":format-code") ),
        sFormat );

    // format-locale
    OUStringBuffer buf( 48 );
    buf.append( locale.Language );
    if (locale.Country.getLength())
    {
        buf.append( (sal_Unicode)';' );
        buf.append( locale.Country );
        if (locale.Variant.getLength())
        {
            buf.append( (sal_Unicode)';' );
            buf.append( locale.Variant );
        }
    }
    addAttribute(
        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":format-locale") ),
        buf.makeStringAndClear() );
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
            OSL_ENSURE( 0, "### unexpected property type!" );
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("system_short") ) );
                break;
            case 1:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("system_short_YY") ) );
                break;
            case 2:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("system_short_YYYY") ) );
                break;
            case 3:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("system_long") ) );
                break;
            case 4:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("short_DDMMYY") ) );
                break;
            case 5:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("short_MMDDYY") ) );
                break;
            case 6:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("short_YYMMDD") ) );
                break;
            case 7:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("short_DDMMYYYY") ) );
                break;
            case 8:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("short_MMDDYYYY") ) );
                break;
            case 9:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("short_YYYYMMDD") ) );
                break;
            case 10:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("short_YYMMDD_DIN5008") ) );
                break;
            case 11:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("short_YYYYMMDD_DIN5008") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected date format!" );
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("24h_short") ) );
                break;
            case 1:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("24h_long") ) );
                break;
            case 2:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("12h_short") ) );
                break;
            case 3:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("12h_long") ) );
                break;
            case 4:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("Duration_short") ) );
                break;
            case 5:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("Duration_long") ) );
                break;
            default:
                OSL_ENSURE( 0, "### unexpected time format!" );
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("left") ) );
                break;
            case 1:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("center") ) );
                break;
            case 2:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("right") ) );
                break;
            default:
                OSL_ENSURE( 0, "### illegal alignment value!" );
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("top") ) );
                break;
            case style::VerticalAlignment_MIDDLE:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("center") ) );
                break;
            case style::VerticalAlignment_BOTTOM:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("bottom") ) );
                break;
            default:
                OSL_ENSURE( 0, "### illegal vertical alignment value!" );
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
        rtl::OUString sURL;
        _xProps->getPropertyValue( rPropName ) >>= sURL;

        if ( sURL.getLength() && sURL.compareToAscii( XMLSCRIPT_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( XMLSCRIPT_GRAPHOBJ_URLPREFIX ) ) == 0 )
        {
            Reference< document::XStorageBasedDocument > xDocStorage( _xDocument, UNO_QUERY );
            if ( xDocStorage.is() )
            {
                uno::Sequence< Any > aArgs( 1 );
                aArgs[ 0 ] <<= xDocStorage->getDocumentStorage();

                ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
                uno::Reference< document::XGraphicObjectResolver > xGraphicResolver;
                aContext.createComponentWithArguments( OUSTR( "com.sun.star.comp.Svx.GraphicExportHelper" ), aArgs, xGraphicResolver );
                if ( xGraphicResolver.is() )
                    sURL = xGraphicResolver->resolveGraphicObjectURL( sURL );
            }
        }
        if ( sURL.getLength() )
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("left") ) );
                break;
            case 1:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("top") ) );
                break;
            case 2:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("right") ) );
                break;
            case 3:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("bottom") ) );
                break;
            default:
                OSL_ENSURE( 0, "### illegal image alignment value!" );
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("left-top") ) );
                break;
            case awt::ImagePosition::LeftCenter:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("left-center") ) );
                break;
            case awt::ImagePosition::LeftBottom:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("left-bottom") ) );
                break;
            case awt::ImagePosition::RightTop:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("right-top") ) );
                break;
            case awt::ImagePosition::RightCenter:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("right-center") ) );
                break;
            case awt::ImagePosition::RightBottom:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("right-bottom") ) );
                break;
            case awt::ImagePosition::AboveLeft:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("top-left") ) );
                break;
            case awt::ImagePosition::AboveCenter:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("top-center") ) );
                break;
            case awt::ImagePosition::AboveRight:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("top-right") ) );
                break;
            case awt::ImagePosition::BelowLeft:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("bottom-left") ) );
                break;
            case awt::ImagePosition::BelowCenter:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("bottom-center") ) );
                break;
            case awt::ImagePosition::BelowRight:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("bottom-right") ) );
                break;
            case awt::ImagePosition::Centered:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("center") ) );
                break;
            default:
                OSL_ENSURE( 0, "### illegal image position value!" );
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("standard") ) );
                break;
            case awt::PushButtonType_OK:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("ok") ) );
                break;
            case awt::PushButtonType_CANCEL:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("cancel") ) );
                break;
            case awt::PushButtonType_HELP:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("help") ) );
                break;
            default:
                OSL_ENSURE( 0, "### illegal button-type value!" );
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("horizontal") ) );
                break;
            case 1:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("vertical") ) );
                break;
            default:
                OSL_ENSURE( 0, "### illegal orientation value!" );
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
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("carriage-return") ) );
                break;
            case awt::LineEndFormat::LINE_FEED:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("line-feed") ) );
                break;
            case awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED:
                addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("carriage-return-line-feed") ) );
                break;
            default:
                OSL_ENSURE( 0, "### illegal line end format value!" );
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

    if ( xFac.is() && xBinding.is() && rAttrName.equals( OUSTR(XMLNS_DIALOGS_PREFIX ":linked-cell") ) )
    {
        try
        {
            Reference< beans::XPropertySet > xConvertor( xFac->createInstance( OUSTR( "com.sun.star.table.CellAddressConversion" )), uno::UNO_QUERY );
        Reference< beans::XPropertySet > xBindable( xBinding->getValueBinding(), UNO_QUERY );
            if ( xBindable.is() )
            {
                table::CellAddress aAddress;
                xBindable->getPropertyValue( OUSTR("BoundCell") ) >>= aAddress;
                xConvertor->setPropertyValue( OUSTR("Address"), makeAny( aAddress ) );
                rtl::OUString sAddress;
                xConvertor->getPropertyValue( OUSTR("PersistentRepresentation") ) >>= sAddress;
                if ( sAddress.getLength() > 0 )
                    addAttribute( rAttrName, sAddress );

                OSL_TRACE( "*** Bindable value %s", rtl::OUStringToOString( sAddress, RTL_TEXTENCODING_UTF8 ).getStr() );

            }
        }
        catch( uno::Exception& )
        {
        }
    }
    Reference< form::binding::XListEntrySink > xEntrySink( _xProps, UNO_QUERY );
    if ( xEntrySink.is() && rAttrName.equals( OUSTR( XMLNS_DIALOGS_PREFIX ":source-cell-range") ) )
    {
        Reference< beans::XPropertySet > xListSource( xEntrySink->getListEntrySource(), UNO_QUERY );
        if ( xListSource.is() )
        {
            try
            {
                Reference< beans::XPropertySet > xConvertor( xFac->createInstance( OUSTR( "com.sun.star.table.CellRangeAddressConversion" )), uno::UNO_QUERY );

                table::CellRangeAddress aAddress;
                xListSource->getPropertyValue( OUSTR( "CellRange" ) ) >>= aAddress;

                rtl::OUString sAddress;
                xConvertor->setPropertyValue( OUSTR("Address"), makeAny( aAddress ) );
                xConvertor->getPropertyValue( OUSTR("PersistentRepresentation") ) >>= sAddress;
                OSL_TRACE("**** cell range source list %s",
                    rtl::OUStringToOString( sAddress, RTL_TEXTENCODING_UTF8 ).getStr() );
                if ( sAddress.getLength() > 0 )
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
                    addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("none") ) );
                    break;
                case ::view::SelectionType_SINGLE:
                    addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("single") ) );
                    break;
                case ::view::SelectionType_MULTI:
                    addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("multi") ) );
                    break;
                case ::view::SelectionType_RANGE:
                    addAttribute( rAttrName, OUString( RTL_CONSTASCII_USTRINGPARAM("range") ) );
                    break;
                default:
                    OSL_ENSURE( 0, "### illegal selection type value!" );
                    break;
            }
        }
    }
}
//__________________________________________________________________________________________________
void ElementDescriptor::readDefaults( bool supportPrintable, bool supportVisible )
{
    Any a( _xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Name") ) ) );

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
    if ( xSrvInfo.is() && xSrvInfo->supportsService( OUSTR("com.sun.star.form.FormComponent" ) ) )
    {
        Reference< io::XPersistObject > xPersist( _xProps, UNO_QUERY );
        if ( xPersist.is() )
        {
            OUString sCtrlName = xPersist->getServiceName();
            if ( sCtrlName.getLength() )
                    addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":control-implementation") ), sCtrlName );
        }
    }
    addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":id") ),
             * reinterpret_cast< const OUString * >( a.getValue() ) );
    readShortAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("TabIndex") ),
                   OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tab-index") ) );

    sal_Bool bEnabled = sal_False;
    if (_xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Enabled") ) ) >>= bEnabled)
    {
        if (! bEnabled)
        {
            addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":disabled") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("true") ) );
        }
    }
    else
    {
        OSL_ENSURE( 0, "unexpected property type for \"Enabled\": not bool!" );
    }

    sal_Bool bVisible = sal_True;
    if (supportVisible) try
    {
        if (_xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("EnableVisible") ) ) >>= bVisible)
        {

            // only write out the non default case
            if (! bVisible)
            {
                addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":visible") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("false") ) );
            }
        }
    }
    catch( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    // force writing of pos/size
    a = _xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("PositionX") ) );
    if (a.getValueTypeClass() == TypeClass_LONG)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":left") ),
                      OUString::valueOf( *(sal_Int32 const *)a.getValue() ) );
    }
    a = _xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("PositionY") ) );
    if (a.getValueTypeClass() == TypeClass_LONG)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":top") ),
                      OUString::valueOf( *(sal_Int32 const *)a.getValue() ) );
    }
    a = _xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Width") ) );
    if (a.getValueTypeClass() == TypeClass_LONG)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":width") ),
                      OUString::valueOf( *(sal_Int32 const *)a.getValue() ) );
    }
    a = _xProps->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ) );
    if (a.getValueTypeClass() == TypeClass_LONG)
    {
        addAttribute( OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":height") ),
                      OUString::valueOf( *(sal_Int32 const *)a.getValue() ) );
    }

    if (supportPrintable)
    {
        readBoolAttr(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Printable") ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":printable") ) );
    }
    readLongAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Step") ),
                  OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":page") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("Tag") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":tag") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("HelpText") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":help-text") ) );
    readStringAttr( OUString( RTL_CONSTASCII_USTRINGPARAM("HelpURL") ),
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":help-url") ) );
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
                    OSL_ENSURE( descr.ListenerType.getLength() > 0 &&
                                descr.EventMethod.getLength() > 0 &&
                                descr.ScriptCode.getLength() > 0 &&
                                descr.ScriptType.getLength() > 0,
                                "### invalid event descr!" );

                    OUString aEventName;

                    if (! descr.AddListenerParam.getLength())
                    {
                        // detection of event-name
                        ::rtl::OString listenerType(
                            ::rtl::OUStringToOString(
                                descr.ListenerType,
                                RTL_TEXTENCODING_ASCII_US ) );
                        ::rtl::OString eventMethod(
                            ::rtl::OUStringToOString(
                                descr.EventMethod,
                                RTL_TEXTENCODING_ASCII_US ) );
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

                    if (aEventName.getLength()) // script:event
                    {
                        pElem = new ElementDescriptor(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":event") ) );
                        xElem = pElem;

                        pElem->addAttribute(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":event-name") ),
                            aEventName );
                    }
                    else // script:listener-event
                    {
                        pElem = new ElementDescriptor(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":listener-event") ) );
                        xElem = pElem;

                        pElem->addAttribute(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":listener-type") ),
                            descr.ListenerType );
                        pElem->addAttribute(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":listener-method") ),
                            descr.EventMethod );

                        if (descr.AddListenerParam.getLength())
                        {
                            pElem->addAttribute(
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":listener-param") ),
                                descr.AddListenerParam );
                        }
                    }
                    if ( descr.ScriptType.equals( OUString(  RTL_CONSTASCII_USTRINGPARAM( "StarBasic" ) ) ) )
                    {
                        // separate optional location
                        sal_Int32 nIndex = descr.ScriptCode.indexOf( (sal_Unicode)':' );
                        if (nIndex >= 0)
                        {
                            pElem->addAttribute(
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":location") ),
                                descr.ScriptCode.copy( 0, nIndex ) );
                            pElem->addAttribute(
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":macro-name") ),
                                descr.ScriptCode.copy( nIndex +1 ) );
                        }
                        else
                        {
                            pElem->addAttribute(
                                OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":macro-name") ),
                                descr.ScriptCode );
                        }
                    }
                    else
                    {
                        pElem->addAttribute(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":macro-name") ),
                            descr.ScriptCode );
                    }

                    // language
                    pElem->addAttribute(
                        OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_SCRIPT_PREFIX ":language") ),
                        descr.ScriptType );

                    addSubElement( xElem );
                }
                else
                {
                    OSL_ENSURE( 0, "### unexpected event type in container!" );
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
    SAL_THROW( () )
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
StyleBag::~StyleBag() SAL_THROW( () )
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
        OUString aStylesName( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":styles") );
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

    ElementDescriptor * pElem = new ElementDescriptor(
                    xProps, xPropState,
                    OUString( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":bulletinboard") ), xDocument );
    Reference< xml::sax::XAttributeList > xElem( pElem );
    pElem->readBullitinBoard( &all_styles );

    xOut->startDocument();

    OUString aDocTypeStr( RTL_CONSTASCII_USTRINGPARAM(
        "<!DOCTYPE dlg:window PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\""
        " \"dialog.dtd\">" ) );
    xOut->unknown( aDocTypeStr );
    xOut->ignorableWhitespace( OUString() );


    OUString aWindowName( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":window") );
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
        OUString aBBoardName( RTL_CONSTASCII_USTRINGPARAM(XMLNS_DIALOGS_PREFIX ":bulletinboard") );
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
