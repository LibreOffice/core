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

#include "imp_share.hxx"
#include "xml_import.hxx"

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
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <tools/date.hxx>
#include <tools/time.hxx>

#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>

#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XValueBinding.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/document/XGraphicObjectResolver.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/script/DocumentScriptLibraryContainer.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;

namespace xmlscript
{

//__________________________________________________________________________________________________
void EventElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    static_cast< ControlElement * >( _pParent )->_events.push_back( this );
}
//__________________________________________________________________________________________________
ControlElement::ControlElement(
    OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ElementBase * pParent, DialogImport * pImport )
    SAL_THROW(())
    : ElementBase(
        pImport->XMLNS_DIALOGS_UID, rLocalName, xAttributes, pParent, pImport )
{
    if (_pParent)
    {
        // inherit position
        _nBasePosX = static_cast< ControlElement * >( _pParent )->_nBasePosX;
        _nBasePosY = static_cast< ControlElement * >( _pParent )->_nBasePosY;
    }
    else
    {
        _nBasePosX = 0;
        _nBasePosY = 0;
    }
}

//__________________________________________________________________________________________________
Reference< xml::input::XElement > ControlElement::getStyle(
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aStyleId( xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID,"style-id" ) );
    if (!aStyleId.isEmpty())
    {
        return _pImport->getStyle( aStyleId );
    }
    return Reference< xml::input::XElement >();
}
//__________________________________________________________________________________________________
OUString ControlElement::getControlId(
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aId( xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, "id" ) );
    if (aId.isEmpty())
    {
        throw xml::sax::SAXException( "missing id attribute!", Reference< XInterface >(), Any() );
    }
    return aId;
}

OUString ControlElement::getControlModelName(
    OUString const& rDefaultModel,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aModel;
    aModel = xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, "control-implementation");
    if (aModel.isEmpty())
        aModel = rDefaultModel;
    return aModel;
}


//##################################################################################################

//__________________________________________________________________________________________________
bool StyleElement::importTextColorStyle(
    Reference< beans::XPropertySet > const & xProps )
{
    if ((_inited & 0x2) != 0)
    {
        if ((_hasValue & 0x2) != 0)
        {
            xProps->setPropertyValue("TextColor", makeAny( _textColor ) );
            return true;
        }
        return false;
    }
    _inited |= 0x2;

    if (getLongAttr( &_textColor, "text-color", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _hasValue |= 0x2;
        xProps->setPropertyValue( "TextColor", makeAny( _textColor ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool StyleElement::importTextLineColorStyle(
    Reference< beans::XPropertySet > const & xProps )
{
    if ((_inited & 0x20) != 0)
    {
        if ((_hasValue & 0x20) != 0)
        {
            xProps->setPropertyValue( "TextLineColor", makeAny( _textLineColor ) );
            return true;
        }
        return false;
    }
    _inited |= 0x20;

    if (getLongAttr( &_textLineColor, "textline-color", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _hasValue |= 0x20;
        xProps->setPropertyValue( "TextLineColor", makeAny( _textLineColor ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool StyleElement::importFillColorStyle(
    Reference< beans::XPropertySet > const & xProps )
{
    if ((_inited & 0x10) != 0)
    {
        if ((_hasValue & 0x10) != 0)
        {
            xProps->setPropertyValue( "FillColor", makeAny( _fillColor ) );
            return true;
        }
        return false;
    }
    _inited |= 0x10;

    if (getLongAttr( &_fillColor, "fill-color", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _hasValue |= 0x10;
        xProps->setPropertyValue( "FillColor", makeAny( _fillColor ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool StyleElement::importBackgroundColorStyle(
    Reference< beans::XPropertySet > const & xProps )
{
    if ((_inited & 0x1) != 0)
    {
        if ((_hasValue & 0x1) != 0)
        {
            xProps->setPropertyValue( "BackgroundColor", makeAny( _backgroundColor ) );
            return true;
        }
        return false;
    }
    _inited |= 0x1;

    if (getLongAttr( &_backgroundColor, "background-color", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _hasValue |= 0x1;
        xProps->setPropertyValue( "BackgroundColor", makeAny( _backgroundColor ) );
        return true;
    }
    return false;
}

//__________________________________________________________________________________________________
bool StyleElement::importBorderStyle(
    Reference< beans::XPropertySet > const & xProps )
{
    if ((_inited & 0x4) != 0)
    {
        if ((_hasValue & 0x4) != 0)
        {
            xProps->setPropertyValue( "Border", makeAny( _border == BORDER_SIMPLE_COLOR ? BORDER_SIMPLE : _border ) );
            if (_border == BORDER_SIMPLE_COLOR)
                xProps->setPropertyValue( "BorderColor", makeAny(_borderColor) );
            return true;
        }
        return false;
    }
    _inited |= 0x4;

    OUString aValue;
    if (getStringAttr(&aValue, "border", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
        {
        if ( aValue == "none" )
            _border = BORDER_NONE;
        else if ( aValue == "3d" )
            _border = BORDER_3D;
        else if ( aValue == "simple" )
            _border = BORDER_SIMPLE;
        else {
            _border = BORDER_SIMPLE_COLOR;
            _borderColor = toInt32(aValue);
        }

        _hasValue |= 0x4;
        importBorderStyle(xProps); // write values
    }
    return false;
}

//______________________________________________________________________________
bool StyleElement::importVisualEffectStyle(
    Reference<beans::XPropertySet> const & xProps )
{
    if ((_inited & 0x40) != 0)
    {
        if ((_hasValue & 0x40) != 0)
        {
            xProps->setPropertyValue( "VisualEffect", makeAny(_visualEffect) );
            return true;
        }
        return false;
    }
    _inited |= 0x40;

    OUString aValue;
    if (getStringAttr( &aValue, "look", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "none" )
        {
            _visualEffect = awt::VisualEffect::NONE;
        }
        else if ( aValue == "3d" )
        {
            _visualEffect = awt::VisualEffect::LOOK3D;
        }
        else if ( aValue == "simple" )
        {
            _visualEffect = awt::VisualEffect::FLAT;
        }
        else
            OSL_ASSERT( 0 );

        _hasValue |= 0x40;
        xProps->setPropertyValue( "VisualEffect", makeAny(_visualEffect) );
    }
    return false;
}

//__________________________________________________________________________________________________
void StyleElement::setFontProperties(
    Reference< beans::XPropertySet > const & xProps )
{
    xProps->setPropertyValue("FontDescriptor", makeAny( _descr ) );
    xProps->setPropertyValue("FontEmphasisMark", makeAny( _fontEmphasisMark ) );
    xProps->setPropertyValue("FontRelief", makeAny( _fontRelief ) );
}
//__________________________________________________________________________________________________
bool StyleElement::importFontStyle(
    Reference< beans::XPropertySet > const & xProps )
{
    if ((_inited & 0x8) != 0)
    {
        if ((_hasValue & 0x8) != 0)
        {
            setFontProperties( xProps );
            return true;
        }
        return false;
    }
    _inited |= 0x8;

    OUString aValue;
    bool bFontImport;

    // dialog:font-name CDATA #IMPLIED
    bFontImport = getStringAttr( &_descr.Name, "font-name", _xAttributes, _pImport->XMLNS_DIALOGS_UID );

    // dialog:font-height %numeric; #IMPLIED
    if (getStringAttr( &aValue, "font-height", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.Height = (sal_Int16)toInt32( aValue );
        bFontImport = true;
    }
    // dialog:font-width %numeric; #IMPLIED
    if (getStringAttr(&aValue, "font-width", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.Width = (sal_Int16)toInt32( aValue );
        bFontImport = true;
    }
    // dialog:font-stylename CDATA #IMPLIED
    bFontImport |= getStringAttr( &_descr.StyleName, "font-stylename", _xAttributes, _pImport->XMLNS_DIALOGS_UID );

    // dialog:font-family "(decorative|modern|roman|script|swiss|system)" #IMPLIED
    if (getStringAttr(&aValue, "font-family", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "decorative" )
        {
            _descr.Family = awt::FontFamily::DECORATIVE;
        }
        else if ( aValue == "modern" )
        {
            _descr.Family = awt::FontFamily::MODERN;
        }
        else if ( aValue == "roman" )
        {
            _descr.Family = awt::FontFamily::ROMAN;
        }
        else if ( aValue == "script" )
        {
            _descr.Family = awt::FontFamily::SCRIPT;
        }
        else if ( aValue == "swiss" )
        {
            _descr.Family = awt::FontFamily::SWISS;
        }
        else if ( aValue == "system" )
        {
            _descr.Family = awt::FontFamily::SYSTEM;
        }
        else
        {
            throw xml::sax::SAXException("invalid font-family style!", Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-charset "(ansi|mac|ibmpc_437|ibmpc_850|ibmpc_860|ibmpc_861|ibmpc_863|ibmpc_865|system|symbol)" #IMPLIED
    if (getStringAttr(&aValue, "font-charset", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "ansi" )
        {
            _descr.CharSet = awt::CharSet::ANSI;
        }
        else if ( aValue == "mac" )
        {
            _descr.CharSet = awt::CharSet::MAC;
        }
        else if ( aValue == "ibmpc_437" )
        {
            _descr.CharSet = awt::CharSet::IBMPC_437;
        }
        else if ( aValue == "ibmpc_850" )
        {
            _descr.CharSet = awt::CharSet::IBMPC_850;
        }
        else if ( aValue == "ibmpc_860" )
        {
            _descr.CharSet = awt::CharSet::IBMPC_860;
        }
        else if ( aValue == "ibmpc_861" )
        {
            _descr.CharSet = awt::CharSet::IBMPC_861;
        }
        else if ( aValue == "ibmpc_863" )
        {
            _descr.CharSet = awt::CharSet::IBMPC_863;
        }
        else if ( aValue == "ibmpc_865" )
        {
            _descr.CharSet = awt::CharSet::IBMPC_865;
        }
        else if ( aValue == "system" )
        {
            _descr.CharSet = awt::CharSet::SYSTEM;
        }
        else if ( aValue == "symbol" )
        {
            _descr.CharSet = awt::CharSet::SYMBOL;
        }
        else
        {
            throw xml::sax::SAXException("invalid font-charset style!", Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-pitch "(fixed|variable)" #IMPLIED
    if (getStringAttr( &aValue, "font-pitch", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "fixed" )
        {
            _descr.Pitch = awt::FontPitch::FIXED;
        }
        else if ( aValue == "variable" )
        {
            _descr.Pitch = awt::FontPitch::VARIABLE;
        }
        else
        {
            throw xml::sax::SAXException("invalid font-pitch style!", Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-charwidth CDATA #IMPLIED
    if (getStringAttr( &aValue, "font-charwidth", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.CharacterWidth = aValue.toFloat();
        bFontImport = true;
    }
    // dialog:font-weight CDATA #IMPLIED
    if (getStringAttr( &aValue, "font-weight", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.Weight = aValue.toFloat();
        bFontImport = true;
    }

    // dialog:font-slant "(oblique|italic|reverse_oblique|reverse_italic)" #IMPLIED
    if (getStringAttr( &aValue, "font-slant", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "oblique" )
        {
            _descr.Slant = awt::FontSlant_OBLIQUE;
        }
        else if ( aValue == "italic" )
        {
            _descr.Slant = awt::FontSlant_ITALIC;
        }
        else if ( aValue == "reverse_oblique" )
        {
            _descr.Slant = awt::FontSlant_REVERSE_OBLIQUE;
        }
        else if ( aValue == "reverse_italic" )
        {
            _descr.Slant = awt::FontSlant_REVERSE_ITALIC;
        }
        else
        {
            throw xml::sax::SAXException("invalid font-slant style!", Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-underline "(single|double|dotted|dash|longdash|dashdot|dashdotdot|smallwave|wave|doublewave|bold|bolddotted|bolddash|boldlongdash|bolddashdot|bolddashdotdot|boldwave)" #IMPLIED
    if (getStringAttr( &aValue, "font-underline", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "single" )
        {
            _descr.Underline = awt::FontUnderline::SINGLE;
        }
        else if ( aValue == "double" )
        {
            _descr.Underline = awt::FontUnderline::DOUBLE;
        }
        else if ( aValue == "dotted" )
        {
            _descr.Underline = awt::FontUnderline::DOTTED;
        }
        else if ( aValue == "dash" )
        {
            _descr.Underline = awt::FontUnderline::DASH;
        }
        else if ( aValue == "longdash" )
        {
            _descr.Underline = awt::FontUnderline::LONGDASH;
        }
        else if ( aValue == "dashdot" )
        {
            _descr.Underline = awt::FontUnderline::DASHDOT;
        }
        else if ( aValue == "dashdotdot" )
        {
            _descr.Underline = awt::FontUnderline::DASHDOTDOT;
        }
        else if ( aValue == "smallwave" )
        {
            _descr.Underline = awt::FontUnderline::SMALLWAVE;
        }
        else if ( aValue == "wave" )
        {
            _descr.Underline = awt::FontUnderline::WAVE;
        }
        else if ( aValue == "doublewave" )
        {
            _descr.Underline = awt::FontUnderline::DOUBLEWAVE;
        }
        else if ( aValue == "bold" )
        {
            _descr.Underline = awt::FontUnderline::BOLD;
        }
        else if ( aValue == "bolddotted" )
        {
            _descr.Underline = awt::FontUnderline::BOLDDOTTED;
        }
        else if ( aValue == "bolddash" )
        {
            _descr.Underline = awt::FontUnderline::BOLDDASH;
        }
        else if ( aValue == "boldlongdash" )
        {
            _descr.Underline = awt::FontUnderline::BOLDLONGDASH;
        }
        else if ( aValue == "bolddashdot" )
        {
            _descr.Underline = awt::FontUnderline::BOLDDASHDOT;
        }
        else if ( aValue == "bolddashdotdot" )
        {
            _descr.Underline = awt::FontUnderline::BOLDDASHDOTDOT;
        }
        else if ( aValue == "boldwave" )
        {
            _descr.Underline = awt::FontUnderline::BOLDWAVE;
        }
        else
        {
            throw xml::sax::SAXException("invalid font-underline style!", Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-strikeout "(single|double|bold|slash|x)" #IMPLIED
    if (getStringAttr( &aValue, "font-strikeout", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "single" )
        {
            _descr.Strikeout = awt::FontStrikeout::SINGLE;
        }
        else if ( aValue == "double" )
        {
            _descr.Strikeout = awt::FontStrikeout::DOUBLE;
        }
        else if ( aValue == "bold" )
        {
            _descr.Strikeout = awt::FontStrikeout::BOLD;
        }
        else if ( aValue == "slash" )
        {
            _descr.Strikeout = awt::FontStrikeout::SLASH;
        }
        else if ( aValue == "x" )
        {
            _descr.Strikeout = awt::FontStrikeout::X;
        }
        else
        {
            throw xml::sax::SAXException( "invalid font-strikeout style!" , Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-orientation CDATA #IMPLIED
    if (getStringAttr( &aValue, "font-orientation", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.Orientation = aValue.toFloat();
        bFontImport = true;
    }
    // dialog:font-kerning %boolean; #IMPLIED
    bFontImport |= getBoolAttr( &_descr.Kerning, "font-kerning", _xAttributes, _pImport->XMLNS_DIALOGS_UID );
    // dialog:font-wordlinemode %boolean; #IMPLIED
    bFontImport |= getBoolAttr( &_descr.WordLineMode,"font-wordlinemode", _xAttributes, _pImport->XMLNS_DIALOGS_UID );

    // dialog:font-type "(raster|device|scalable)" #IMPLIED
    if (getStringAttr( &aValue, "font-type", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "raster" )
        {
            _descr.Type = awt::FontType::RASTER;
        }
        else if ( aValue == "device" )
        {
            _descr.Type = awt::FontType::DEVICE;
        }
        else if ( aValue == "scalable" )
        {
            _descr.Type = awt::FontType::SCALABLE;
        }
        else
        {
            throw xml::sax::SAXException( "invalid font-type style!", Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // additional properties which are not part of the FontDescriptor struct
    // dialog:font-relief (none|embossed|engraved) #IMPLIED
    if (getStringAttr( &aValue, "font-relief", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "none" )
        {
            _fontRelief = awt::FontRelief::NONE;
        }
        else if ( aValue == "embossed" )
        {
            _fontRelief = awt::FontRelief::EMBOSSED;
        }
        else if ( aValue == "engraved" )
        {
            _fontRelief = awt::FontRelief::ENGRAVED;
        }
        else
        {
            throw xml::sax::SAXException("invalid font-relief style!", Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }
    // dialog:font-emphasismark (none|dot|circle|disc|accent|above|below) #IMPLIED
    if (getStringAttr(&aValue, "font-emphasismark", _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if ( aValue == "none" )
        {
            _fontEmphasisMark = awt::FontEmphasisMark::NONE;
        }
        else if ( aValue == "dot" )
        {
            _fontEmphasisMark = awt::FontEmphasisMark::DOT;
        }
        else if ( aValue == "circle" )
        {
            _fontEmphasisMark = awt::FontEmphasisMark::CIRCLE;
        }
        else if ( aValue == "disc" )
        {
            _fontEmphasisMark = awt::FontEmphasisMark::DISC;
        }
        else if ( aValue == "accent" )
        {
            _fontEmphasisMark = awt::FontEmphasisMark::ACCENT;
        }
        else if ( aValue == "above" )
        {
            _fontEmphasisMark = awt::FontEmphasisMark::ABOVE;
        }
        else if ( aValue == "below" )
        {
            _fontEmphasisMark = awt::FontEmphasisMark::BELOW;
        }
        else
        {
            throw xml::sax::SAXException( "invalid font-emphasismark style!", Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // ==================================================
    if (bFontImport)
    {
        _hasValue |= 0x8;
        setFontProperties( xProps );
    }

    return bFontImport;
}

//##################################################################################################

//__________________________________________________________________________________________________
bool ImportContext::importStringProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aValue(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aValue.isEmpty())
    {
        _xControlModel->setPropertyValue( rPropName, makeAny( aValue ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importDoubleProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aValue(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aValue.isEmpty())
    {
        _xControlModel->setPropertyValue( rPropName, makeAny( aValue.toDouble() ) );
        return true;
    }
    return false;
}

//__________________________________________________________________________________________________
bool ImportContext::importBooleanProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    sal_Bool bBool;
    if (getBoolAttr(
            &bBool, rAttrName, xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _xControlModel->setPropertyValue( rPropName, makeAny( bBool ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importLongProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aValue(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aValue.isEmpty())
    {
        _xControlModel->setPropertyValue( rPropName, makeAny( toInt32( aValue ) ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importLongProperty(
    sal_Int32 nOffset,
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aValue(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aValue.isEmpty())
    {
        _xControlModel->setPropertyValue( rPropName, makeAny( toInt32( aValue ) + nOffset ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importHexLongProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aValue(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aValue.isEmpty())
    {
        _xControlModel->setPropertyValue( rPropName, makeAny( toInt32( aValue ) ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importShortProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aValue(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aValue.isEmpty())
    {
        _xControlModel->setPropertyValue( rPropName, makeAny( (sal_Int16)toInt32( aValue ) ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importAlignProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aAlign(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aAlign.isEmpty())
    {
        sal_Int16 nAlign;
        if ( aAlign == "left" )
        {
            nAlign = 0;
        }
        else if ( aAlign == "center" )
        {
            nAlign = 1;
        }
        else if ( aAlign == "right" )
        {
            nAlign = 2;
        }
        else if ( aAlign == "none" )
        {
            nAlign = 0; // default
        }
        else
        {
            throw xml::sax::SAXException("invalid align value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nAlign ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importVerticalAlignProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aAlign(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aAlign.isEmpty())
    {
        style::VerticalAlignment eAlign;

        if ( aAlign == "top" )
        {
            eAlign = style::VerticalAlignment_TOP;
        }
        else if ( aAlign == "center" )
        {
            eAlign = style::VerticalAlignment_MIDDLE;
        }
        else if ( aAlign == "bottom" )
        {
            eAlign = style::VerticalAlignment_BOTTOM;
        }
        else
        {
            throw xml::sax::SAXException( "invalid vertical align value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( eAlign ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importImageURLProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString sURL = xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, rAttrName );
    if ( !sURL.isEmpty() )
    {
        Reference< document::XStorageBasedDocument > xDocStorage( _pImport->getDocOwner(), UNO_QUERY );

        uno::Reference< document::XGraphicObjectResolver > xGraphicResolver;
        if ( xDocStorage.is() )
        {
            uno::Sequence< Any > aArgs( 1 );
            aArgs[ 0 ] <<= xDocStorage->getDocumentStorage();
            xGraphicResolver.set(
                _pImport->getComponentContext()->getServiceManager()->createInstanceWithArgumentsAndContext( "com.sun.star.comp.Svx.GraphicImportHelper" , aArgs, _pImport->getComponentContext() ),
                UNO_QUERY );
            if ( xGraphicResolver.is() )
            {
                OUString aTmp("vnd.sun.star.Package:");
                aTmp += sURL;
                try
                {
                    aTmp = xGraphicResolver->resolveGraphicObjectURL( aTmp );
                    if ( !aTmp.isEmpty() )
                        sURL = aTmp;
                }
                catch( const uno::Exception& )
                {
                    return false;
                }

            }
        }
        if ( !sURL.isEmpty() )
        {
            Reference< beans::XPropertySet > xProps( getControlModel(), UNO_QUERY );
            if ( xProps.is() )
            {
                xProps->setPropertyValue( rPropName, makeAny( sURL ) );
                return true;
            }
        }
    }
    return false;
}
//__________________________________________________________________________________________________
 bool ImportContext::importDataAwareProperty(
        OUString const & rPropName,
        Reference<xml::input::XAttributes> const & xAttributes )
{
    OUString sLinkedCell;
    OUString sCellRange;
    if ( rPropName.equals( "linked-cell" ) )
       sLinkedCell = xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, rPropName );
    if ( rPropName.equals( "source-cell-range" ) )
        sCellRange = xAttributes->getValueByUidName( _pImport->XMLNS_DIALOGS_UID, rPropName );
    bool bRes = false;
    Reference< lang::XMultiServiceFactory > xFac( _pImport->getDocOwner(), UNO_QUERY );
    if ( xFac.is() && ( !sLinkedCell.isEmpty() ||  !sCellRange.isEmpty() ) )
    {
        // Set up Celllink
        if ( !sLinkedCell.isEmpty() )
        {
            Reference< form::binding::XBindableValue > xBindable( getControlModel(), uno::UNO_QUERY );
            Reference< beans::XPropertySet > xConvertor( xFac->createInstance( "com.sun.star.table.CellAddressConversion" ), uno::UNO_QUERY );
            if ( xBindable.is() && xConvertor.is() )
            {
                table::CellAddress aAddress;
                xConvertor->setPropertyValue( "PersistentRepresentation" , uno::makeAny( sLinkedCell ) );
                xConvertor->getPropertyValue( "Address" ) >>= aAddress;
                beans::NamedValue aArg1;
                aArg1.Name = "BoundCell";
                aArg1.Value <<= aAddress;

                uno::Sequence< uno::Any > aArgs(1);
                aArgs[ 0 ]  <<= aArg1;

                uno::Reference< form::binding::XValueBinding > xBinding( xFac->createInstanceWithArguments( "com.sun.star.table.CellValueBinding" , aArgs ), uno::UNO_QUERY );
                xBindable->setValueBinding( xBinding );
                bRes = true;
            }
        }
        // Set up CelllRange
        if ( !sCellRange.isEmpty() )
        {
            Reference< form::binding::XListEntrySink  > xListEntrySink( getControlModel(), uno::UNO_QUERY );
            Reference< beans::XPropertySet > xConvertor( xFac->createInstance( "com.sun.star.table.CellRangeAddressConversion" ), uno::UNO_QUERY );
            if ( xListEntrySink.is() && xConvertor.is() )
            {
                table::CellRangeAddress aAddress;
                xConvertor->setPropertyValue( "PersistentRepresentation" , uno::makeAny( sCellRange ) );
                xConvertor->getPropertyValue( "Address" ) >>= aAddress;
                beans::NamedValue aArg1;
                aArg1.Name = "CellRange";
                aArg1.Value <<= aAddress;

                uno::Sequence< uno::Any > aArgs(1);
                aArgs[ 0 ]  <<= aArg1;

                uno::Reference< form::binding::XListEntrySource > xSource( xFac->createInstanceWithArguments( "com.sun.star.table.CellRangeListSource" , aArgs ), uno::UNO_QUERY );
                xListEntrySink->setListEntrySource( xSource );
                bRes = true;
            }
        }
    }
    return bRes;
}
//__________________________________________________________________________________________________
bool ImportContext::importImageAlignProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aAlign(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aAlign.isEmpty())
    {
        sal_Int16 nAlign;
        if ( aAlign == "left" )
        {
            nAlign = 0;
        }
        else if ( aAlign == "top" )
        {
            nAlign = 1;
        }
        else if ( aAlign == "right" )
        {
            nAlign = 2;
        }
        else if ( aAlign == "bottom" )
        {
            nAlign = 3;
        }
        else
        {
            throw xml::sax::SAXException( "invalid image align value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nAlign ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importImagePositionProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aPosition(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aPosition.isEmpty())
    {
        sal_Int16 nPosition;
        if ( aPosition == "left-top" )
        {
            nPosition = awt::ImagePosition::LeftTop;
        }
        else if ( aPosition == "left-center" )
        {
            nPosition = awt::ImagePosition::LeftCenter;
        }
        else if ( aPosition == "left-bottom" )
        {
            nPosition = awt::ImagePosition::LeftBottom;
        }
        else if ( aPosition == "right-top" )
        {
            nPosition = awt::ImagePosition::RightTop;
        }
        else if ( aPosition == "right-center" )
        {
            nPosition = awt::ImagePosition::RightCenter;
        }
        else if ( aPosition == "right-bottom" )
        {
            nPosition = awt::ImagePosition::RightBottom;
        }
        else if ( aPosition == "top-left" )
        {
            nPosition = awt::ImagePosition::AboveLeft;
        }
        else if ( aPosition == "top-center" )
        {
            nPosition = awt::ImagePosition::AboveCenter;
        }
        else if ( aPosition == "top-right" )
        {
            nPosition = awt::ImagePosition::AboveRight;
        }
        else if ( aPosition == "bottom-left" )
        {
            nPosition = awt::ImagePosition::BelowLeft;
        }
        else if ( aPosition == "bottom-center" )
        {
            nPosition = awt::ImagePosition::BelowCenter;
        }
        else if ( aPosition == "bottom-right" )
        {
            nPosition = awt::ImagePosition::BelowRight;
        }
        else if ( aPosition == "center" )
        {
            nPosition = awt::ImagePosition::Centered;
        }
        else
        {
            throw xml::sax::SAXException( "invalid image position value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nPosition ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importButtonTypeProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString buttonType(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!buttonType.isEmpty())
    {
        sal_Int16 nButtonType;
        if ( buttonType == "standard" )
        {
            nButtonType = awt::PushButtonType_STANDARD;
        }
        else if ( buttonType == "ok" )
        {
            nButtonType = awt::PushButtonType_OK;
        }
        else if ( buttonType == "cancel" )
        {
            nButtonType = awt::PushButtonType_CANCEL;
        }
        else if ( buttonType == "help" )
        {
            nButtonType = awt::PushButtonType_HELP;
        }
        else
        {
            throw xml::sax::SAXException( "invalid button-type value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nButtonType ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importDateFormatProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aFormat(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aFormat.isEmpty())
    {
        sal_Int16 nFormat;
        if ( aFormat == "system_short" )
        {
            nFormat = 0;
        }
        else if ( aFormat == "system_short_YY" )
        {
            nFormat = 1;
        }
        else if ( aFormat == "system_short_YYYY" )
        {
            nFormat = 2;
        }
        else if ( aFormat == "system_long" )
        {
            nFormat = 3;
        }
        else if ( aFormat == "short_DDMMYY" )
        {
            nFormat = 4;
        }
        else if ( aFormat == "short_MMDDYY" )
        {
            nFormat = 5;
        }
        else if ( aFormat == "short_YYMMDD" )
        {
            nFormat = 6;
        }
        else if ( aFormat == "short_DDMMYYYY" )
        {
            nFormat = 7;
        }
        else if ( aFormat == "short_MMDDYYYY" )
        {
            nFormat = 8;
        }
        else if ( aFormat == "short_YYYYMMDD" )
        {
            nFormat = 9;
        }
        else if ( aFormat == "short_YYMMDD_DIN5008" )
        {
            nFormat = 10;
        }
        else if ( aFormat == "short_YYYYMMDD_DIN5008" )
        {
            nFormat = 11;
        }
        else
        {
            throw xml::sax::SAXException( "invalid date-format value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nFormat ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importTimeProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aValue(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aValue.isEmpty())
    {
        ::Time aTTime(toInt32( aValue ) * ::Time::nanoPerCenti);
        util::Time aUTime(aTTime.GetUNOTime());
        _xControlModel->setPropertyValue( rPropName, makeAny( aUTime ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importDateProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aValue(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aValue.isEmpty())
    {
        ::Date aTDate(toInt32( aValue ));
        util::Date aUDate(aTDate.GetUNODate());
        _xControlModel->setPropertyValue( rPropName, makeAny( aUDate ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importTimeFormatProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aFormat(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aFormat.isEmpty())
    {
        sal_Int16 nFormat;
        if ( aFormat == "24h_short" )
        {
            nFormat = 0;
        }
        else if ( aFormat == "24h_long" )
        {
            nFormat = 1;
        }
        else if ( aFormat == "12h_short" )
        {
            nFormat = 2;
        }
        else if ( aFormat == "12h_long" )
        {
            nFormat = 3;
        }
        else if ( aFormat == "Duration_short" )
        {
            nFormat = 4;
        }
        else if ( aFormat == "Duration_long" )
        {
            nFormat = 5;
        }
        else
        {
            throw xml::sax::SAXException( "invalid time-format value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nFormat ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importOrientationProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aOrient(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aOrient.isEmpty())
    {
        sal_Int32 nOrient;
        if ( aOrient == "horizontal" )
        {
            nOrient = 0;
        }
        else if ( aOrient == "vertical" )
        {
            nOrient = 1;
        }
        else
        {
            throw xml::sax::SAXException( "invalid orientation value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nOrient ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importLineEndFormatProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aFormat(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aFormat.isEmpty())
    {
        sal_Int16 nFormat;
        if ( aFormat == "carriage-return" )
        {
            nFormat = awt::LineEndFormat::CARRIAGE_RETURN;
        }
        else if ( aFormat == "line-feed" )
        {
            nFormat = awt::LineEndFormat::LINE_FEED;
        }
        else if ( aFormat == "carriage-return-line-feed" )
        {
            nFormat = awt::LineEndFormat::CARRIAGE_RETURN_LINE_FEED;
        }
        else
        {
            throw xml::sax::SAXException( "invalid line end format value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nFormat ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importSelectionTypeProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aSelectionType(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (!aSelectionType.isEmpty())
    {
        view::SelectionType eSelectionType;

        if ( aSelectionType == "none" )
        {
            eSelectionType = view::SelectionType_NONE;
        }
        else if ( aSelectionType == "single" )
        {
            eSelectionType = view::SelectionType_SINGLE;
        }
        else if ( aSelectionType == "multi" )
        {
            eSelectionType = view::SelectionType_MULTI;
        }
        else  if ( aSelectionType == "range" )
        {
            eSelectionType = view::SelectionType_RANGE;
        }
        else
        {
            throw xml::sax::SAXException( "invalid selection type value!", Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( eSelectionType ) );
        return true;
    }
    return false;
}

//==================================================================================================
struct StringTriple
{
    char const * first;
    char const * second;
    char const * third;
};
static StringTriple const s_aEventTranslations[] =
{
    // from xmloff/source/forms/formevents.cxx
    // 28.09.2001 tbe added on-adjustmentvaluechange
    { "com.sun.star.form.XApproveActionListener", "approveAction", "on-approveaction" },
    { "com.sun.star.awt.XActionListener", "actionPerformed", "on-performaction" },
    { "com.sun.star.form.XChangeListener", "changed", "on-change" },
    { "com.sun.star.awt.XTextListener", "textChanged", "on-textchange" },
    { "com.sun.star.awt.XItemListener", "itemStateChanged", "on-itemstatechange" },
    { "com.sun.star.awt.XFocusListener", "focusGained", "on-focus" },
    { "com.sun.star.awt.XFocusListener", "focusLost", "on-blur" },
    { "com.sun.star.awt.XKeyListener", "keyPressed", "on-keydown" },
    { "com.sun.star.awt.XKeyListener", "keyReleased", "on-keyup" },
    { "com.sun.star.awt.XMouseListener", "mouseEntered", "on-mouseover" },
    { "com.sun.star.awt.XMouseMotionListener", "mouseDragged", "on-mousedrag" },
    { "com.sun.star.awt.XMouseMotionListener", "mouseMoved", "on-mousemove" },
    { "com.sun.star.awt.XMouseListener", "mousePressed", "on-mousedown" },
    { "com.sun.star.awt.XMouseListener", "mouseReleased", "on-mouseup" },
    { "com.sun.star.awt.XMouseListener", "mouseExited", "on-mouseout" },
    { "com.sun.star.form.XResetListener", "approveReset", "on-approvereset" },
    { "com.sun.star.form.XResetListener", "resetted", "on-reset" },
    { "com.sun.star.form.XSubmitListener", "approveSubmit", "on-submit" },
    { "com.sun.star.form.XUpdateListener", "approveUpdate", "on-approveupdate" },
    { "com.sun.star.form.XUpdateListener", "updated", "on-update" },
    { "com.sun.star.form.XLoadListener", "loaded", "on-load" },
    { "com.sun.star.form.XLoadListener", "reloading", "on-startreload" },
    { "com.sun.star.form.XLoadListener", "reloaded", "on-reload" },
    { "com.sun.star.form.XLoadListener", "unloading", "on-startunload" },
    { "com.sun.star.form.XLoadListener", "unloaded", "on-unload" },
    { "com.sun.star.form.XConfirmDeleteListener", "confirmDelete", "on-confirmdelete" },
    { "com.sun.star.sdb.XRowSetApproveListener", "approveRowChange", "on-approverowchange" },
    { "com.sun.star.sdbc.XRowSetListener", "rowChanged", "on-rowchange" },
    { "com.sun.star.sdb.XRowSetApproveListener", "approveCursorMove", "on-approvecursormove" },
    { "com.sun.star.sdbc.XRowSetListener", "cursorMoved", "on-cursormove" },
    { "com.sun.star.form.XDatabaseParameterListener", "approveParameter", "on-supplyparameter" },
    { "com.sun.star.sdb.XSQLErrorListener", "errorOccured", "on-error" },
    { "com.sun.star.awt.XAdjustmentListener", "adjustmentValueChanged", "on-adjustmentvaluechange" },
    { 0, 0, 0 }
};
extern StringTriple const * const g_pEventTranslations;
StringTriple const * const g_pEventTranslations = s_aEventTranslations;

//__________________________________________________________________________________________________
void ImportContext::importEvents(
    ::std::vector< Reference< xml::input::XElement > > const & rEvents )
{
    Reference< script::XScriptEventsSupplier > xSupplier(
        _xControlModel, UNO_QUERY );
    if (xSupplier.is())
    {
        Reference< container::XNameContainer > xEvents( xSupplier->getEvents() );
        if (xEvents.is())
        {
            for ( size_t nPos = 0; nPos < rEvents.size(); ++nPos )
            {
                script::ScriptEventDescriptor descr;

                EventElement * pEventElement = static_cast< EventElement * >( rEvents[ nPos ].get() );
                sal_Int32 nUid = pEventElement->getUid();
                OUString aLocalName( pEventElement->getLocalName() );
                Reference< xml::input::XAttributes > xAttributes( pEventElement->getAttributes() );

                // nowadays script events
                if (_pImport->XMLNS_SCRIPT_UID == nUid)
                {
                    if (!getStringAttr( &descr.ScriptType, "language"  , xAttributes, _pImport->XMLNS_SCRIPT_UID ) ||
                        !getStringAttr( &descr.ScriptCode, "macro-name", xAttributes, _pImport->XMLNS_SCRIPT_UID ))
                    {
                        throw xml::sax::SAXException( "missing language or macro-name attribute(s) of event!", Reference< XInterface >(), Any() );
                    }
                    if ( descr.ScriptType == "StarBasic" )
                    {
                        OUString aLocation;
                        if (getStringAttr( &aLocation, "location", xAttributes, _pImport->XMLNS_SCRIPT_UID ))
                        {
                            // prepend location
                            OUStringBuffer buf;
                            buf.append( aLocation );
                            buf.append( (sal_Unicode)':' );
                            buf.append( descr.ScriptCode );
                            descr.ScriptCode = buf.makeStringAndClear();
                        }
                    }
                    else if ( descr.ScriptType == "Script" )
                    {
                        // Check if there is a protocol, if not assume
                        // this is an early scripting framework url ( without
                        // the protocol ) and fix it up!!
                        if ( descr.ScriptCode.indexOf( ':' ) == -1 )
                        {
                            OUStringBuffer buf;
                            buf.append( "vnd.sun.star.script:" );
                            buf.append( descr.ScriptCode );
                            descr.ScriptCode = buf.makeStringAndClear();
                        }
                    }

                    // script:event element
                    if ( aLocalName == "event" )
                    {
                        OUString aEventName;
                        if (! getStringAttr( &aEventName, "event-name", xAttributes, _pImport->XMLNS_SCRIPT_UID ))
                        {
                            throw xml::sax::SAXException( "missing event-name attribute!", Reference< XInterface >(), Any() );
                        }

                        // lookup in table
                        OString str( OUStringToOString( aEventName, RTL_TEXTENCODING_ASCII_US ) );
                        StringTriple const * p = g_pEventTranslations;
                        while (p->first)
                        {
                            if (0 == ::rtl_str_compare( p->third, str.getStr() ))
                            {
                                descr.ListenerType = OUString(
                                    p->first, ::rtl_str_getLength( p->first ),
                                    RTL_TEXTENCODING_ASCII_US );
                                descr.EventMethod = OUString(
                                    p->second, ::rtl_str_getLength( p->second ),
                                    RTL_TEXTENCODING_ASCII_US );
                                break;
                            }
                            ++p;
                        }

                        if (! p->first)
                        {
                            throw xml::sax::SAXException( "no matching event-name found!", Reference< XInterface >(), Any() );
                        }
                    }
                    else // script:listener-event element
                    {
                        SAL_WARN_IF( aLocalName != "listener-event", "xmlscript.xmldlg", "aLocalName != listener-event" );

                        if (!getStringAttr( &descr.ListenerType, "listener-type"  , xAttributes, _pImport->XMLNS_SCRIPT_UID ) ||
                            !getStringAttr( &descr.EventMethod , "listener-method", xAttributes, _pImport->XMLNS_SCRIPT_UID ))
                        {
                            throw xml::sax::SAXException("missing listener-type or listener-method attribute(s)!", Reference< XInterface >(), Any() );
                        }
                        // optional listener param
                        getStringAttr( &descr.AddListenerParam,  "listener-param", xAttributes, _pImport->XMLNS_SCRIPT_UID );
                    }
                }
                else // deprecated dlg:event element
                {
                    SAL_WARN_IF( _pImport->XMLNS_DIALOGS_UID != nUid || aLocalName != "event", "xmlscript.xmldlg", "_pImport->XMLNS_DIALOGS_UID != nUid || aLocalName != \"event\"" );

                    if (!getStringAttr( &descr.ListenerType, "listener-type", xAttributes, _pImport->XMLNS_DIALOGS_UID ) ||
                        !getStringAttr( &descr.EventMethod,  "event-method",  xAttributes, _pImport->XMLNS_DIALOGS_UID ))
                    {
                        throw xml::sax::SAXException("missing listener-type or event-method attribute(s)!", Reference< XInterface >(), Any() );
                    }

                    getStringAttr( &descr.ScriptType, "script-type", xAttributes, _pImport->XMLNS_DIALOGS_UID );
                    getStringAttr( &descr.ScriptCode, "script-code", xAttributes, _pImport->XMLNS_DIALOGS_UID );
                    getStringAttr( &descr.AddListenerParam, "param", xAttributes, _pImport->XMLNS_DIALOGS_UID );
                }

                OUStringBuffer buf;
                buf.append( descr.ListenerType );
                buf.appendAscii( "::" );
                buf.append( descr.EventMethod );
                xEvents->insertByName( buf.makeStringAndClear(), makeAny( descr ) );
            }
        }
    }
}
//__________________________________________________________________________________________________
void ImportContext::importScollableSettings(
    Reference< xml::input::XAttributes > const & _xAttributes )
{
    importLongProperty( OUString( "ScrollHeight" ),
                        OUString( "scrollheight" ),
                        _xAttributes );
    importLongProperty( OUString( "ScrollWidth" ),
                        OUString( "scrollwidth" ),
                        _xAttributes );
    importLongProperty( OUString( "ScrollTop" ),
                        OUString( "scrolltop" ),
                        _xAttributes );
    importLongProperty( OUString( "ScrollLeft" ),
                        OUString( "scrollleft" ),
                        _xAttributes );
    importBooleanProperty( OUString( "HScroll" ),
                           OUString( "hscroll" ),
                           _xAttributes );
    importBooleanProperty( OUString( "VScroll" ),
                           OUString( "vscroll" ),
                           _xAttributes );
}

void ImportContext::importDefaults(
    sal_Int32 nBaseX, sal_Int32 nBaseY,
    Reference< xml::input::XAttributes > const & xAttributes,
    bool supportPrintable )
{
    _xControlModel->setPropertyValue( "Name", makeAny( _aId ) );

    importShortProperty( "TabIndex", "tab-index", xAttributes );

    sal_Bool bDisable = sal_False;
    if (getBoolAttr( &bDisable,"disabled", xAttributes, _pImport->XMLNS_DIALOGS_UID ) && bDisable)
    {
        _xControlModel->setPropertyValue( "Enabled", makeAny( sal_False ) );
    }

    sal_Bool bVisible = sal_True;
    if (getBoolAttr( &bVisible, "visible", xAttributes, _pImport->XMLNS_DIALOGS_UID ) && !bVisible)
    {
    try
    {
            _xControlModel->setPropertyValue( "EnableVisible", makeAny( sal_False ) );
    }
    catch( Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    }

    if (!importLongProperty( nBaseX, "PositionX", "left", xAttributes ) ||
        !importLongProperty( nBaseY, "PositionY", "top",  xAttributes ) ||
        !importLongProperty( "Width", "width", xAttributes ) ||
        !importLongProperty( "Height", "height", xAttributes ))
    {
        throw xml::sax::SAXException( "missing pos size attribute(s)!", Reference< XInterface >(), Any() );
    }

    if (supportPrintable)
    {
        importBooleanProperty("Printable", "printable", xAttributes );
    }

    sal_Int32 nLong;
    if (! getLongAttr( &nLong, "page", xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        nLong = 0;
    }
    _xControlModel->setPropertyValue( "Step", makeAny( nLong ) );

    importStringProperty("Tag", "tag", xAttributes );
    importStringProperty( "HelpText", "help-text", xAttributes );
    importStringProperty( "HelpURL", "help-url", xAttributes );
}

//##################################################################################################

//__________________________________________________________________________________________________
Reference< xml::input::XElement > ElementBase::getParent()
    throw (RuntimeException)
{
    return static_cast< xml::input::XElement * >( _pParent );
}
//__________________________________________________________________________________________________
OUString ElementBase::getLocalName()
    throw (RuntimeException)
{
    return _aLocalName;
}
//__________________________________________________________________________________________________
sal_Int32 ElementBase::getUid()
    throw (RuntimeException)
{
    return _nUid;
}
//__________________________________________________________________________________________________
Reference< xml::input::XAttributes > ElementBase::getAttributes()
    throw (RuntimeException)
{
    return _xAttributes;
}
//__________________________________________________________________________________________________
void ElementBase::ignorableWhitespace(
    OUString const & /*rWhitespaces*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    // not used
}
//__________________________________________________________________________________________________
void ElementBase::characters( OUString const & /*rChars*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    // not used, all characters ignored
}
//__________________________________________________________________________________________________
void ElementBase::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
}
//______________________________________________________________________________
void ElementBase::processingInstruction(
    OUString const & /*Target*/, OUString const & /*Data*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > ElementBase::startChildElement(
    sal_Int32 /*nUid*/, OUString const & /*rLocalName*/,
    Reference< xml::input::XAttributes > const & /*xAttributes*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException( "unexpected element!", Reference< XInterface >(), Any() );
}

//__________________________________________________________________________________________________
ElementBase::ElementBase(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ElementBase * pParent, DialogImport * pImport )
    SAL_THROW(())
    : _pImport( pImport )
    , _pParent( pParent )
    , _nUid( nUid )
    , _aLocalName( rLocalName )
    , _xAttributes( xAttributes )
{
    _pImport->acquire();

    if (_pParent)
    {
        _pParent->acquire();
    }
}
//__________________________________________________________________________________________________
ElementBase::~ElementBase()
    SAL_THROW(())
{
    _pImport->release();

    if (_pParent)
    {
        _pParent->release();
    }

#if OSL_DEBUG_LEVEL > 1
    OString aStr( OUStringToOString(
                             _aLocalName, RTL_TEXTENCODING_ASCII_US ) );
    SAL_INFO("xmlscript.xmldlg", "ElementBase::~ElementBase(): " << aStr.getStr() );
#endif
}

//##################################################################################################

// XRoot
//

//______________________________________________________________________________
void DialogImport::startDocument(
    Reference< xml::input::XNamespaceMapping > const & xNamespaceMapping )
    throw (xml::sax::SAXException, RuntimeException)
{
    XMLNS_DIALOGS_UID = xNamespaceMapping->getUidByUri( XMLNS_DIALOGS_URI );
    XMLNS_SCRIPT_UID = xNamespaceMapping->getUidByUri( XMLNS_SCRIPT_URI );
}
//__________________________________________________________________________________________________
void DialogImport::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // ignored
}
//__________________________________________________________________________________________________
void DialogImport::processingInstruction(
    OUString const & /*rTarget*/, OUString const & /*rData*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    // ignored for now: xxx todo
}
//__________________________________________________________________________________________________
void DialogImport::setDocumentLocator(
    Reference< xml::sax::XLocator > const & /*xLocator*/ )
    throw (xml::sax::SAXException, RuntimeException)
{
    // ignored for now: xxx todo
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > DialogImport::startRootElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException( "illegal namespace!", Reference< XInterface >(), Any() );
    }
    // window
    else if ( rLocalName == "window" )
    {
        return new WindowElement( rLocalName, xAttributes, 0, this );
    }
    else
    {
        throw xml::sax::SAXException( "illegal root element (expected window) given: " + rLocalName, Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
DialogImport::~DialogImport()
    SAL_THROW(())
{
#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("xmlscript.xmldlg", "DialogImport::~DialogImport()." );
#endif
}
//__________________________________________________________________________________________________
Reference< util::XNumberFormatsSupplier > const & DialogImport::getNumberFormatsSupplier()
{
    if (! _xSupplier.is())
    {
        Reference< util::XNumberFormatsSupplier > xSupplier = util::NumberFormatsSupplier::createWithDefaultLocale( getComponentContext() );

        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! _xSupplier.is())
        {
            _xSupplier = xSupplier;
        }
    }
    return _xSupplier;
}

//__________________________________________________________________________________________________
void DialogImport::addStyle(
    OUString const & rStyleId,
    Reference< xml::input::XElement > const & xStyle )
    SAL_THROW(())
{
    (*_pStyleNames).push_back( rStyleId );
    (*_pStyles).push_back( xStyle );
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > DialogImport::getStyle(
    OUString const & rStyleId ) const
    SAL_THROW(())
{
    for ( size_t nPos = 0; nPos < (*_pStyleNames).size(); ++nPos )
    {
        if ( (*_pStyleNames)[ nPos ] == rStyleId)
        {
            return (*_pStyles)[ nPos ];
        }
    }
    return 0;
}

Reference< xml::sax::XDocumentHandler > SAL_CALL importDialogModel(
    Reference< container::XNameContainer > const & xDialogModel,
    Reference< XComponentContext > const & xContext,
    Reference< XModel > const & xDocument )
    SAL_THROW( (Exception) )
{
    // single set of styles and stylenames apply to all containees
    :: boost::shared_ptr< ::std::vector< OUString > > pStyleNames( new ::std::vector< OUString > );
    :: boost::shared_ptr< ::std::vector< css::uno::Reference< css::xml::input::XElement > > > pStyles( new ::std::vector< css::uno::Reference< css::xml::input::XElement > > );
     return ::xmlscript::createDocumentHandler(
         static_cast< xml::input::XRoot * >(
            new DialogImport( xContext, xDialogModel, pStyleNames, pStyles, xDocument ) ) );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
