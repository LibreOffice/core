/*************************************************************************
 *
 *  $RCSfile: xmldlg_import.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 13:00:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "imp_share.hxx"

#include <osl/diagnose.h>
#include <osl/mutex.hxx>

#include <rtl/ustrbuf.hxx>

#include <xmlscript/xml_import.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>

#include <com/sun/star/script/XScriptEventsSupplier.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>



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
    SAL_THROW( () )
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
    OUString aStyleId(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("style-id") ) ) );
    if (aStyleId.getLength())
    {
        return _pImport->getStyle( aStyleId );
    }
    return Reference< xml::input::XElement >();
}
//__________________________________________________________________________________________________
OUString ControlElement::getControlId(
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aId(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID,
            OUString( RTL_CONSTASCII_USTRINGPARAM("id") ) ) );
    if (! aId.getLength())
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("missing id attribute!") ),
            Reference< XInterface >(), Any() );
    }
    return aId;
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
            xProps->setPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ),
                makeAny( _textColor ) );
            return true;
        }
        return false;
    }
    _inited |= 0x2;

    if (getLongAttr( &_textColor,
                     OUString( RTL_CONSTASCII_USTRINGPARAM("text-color") ),
                     _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _hasValue |= 0x2;
        xProps->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("TextColor") ),
            makeAny( _textColor ) );
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
            xProps->setPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ),
                makeAny( _textLineColor ) );
            return true;
        }
        return false;
    }
    _inited |= 0x20;

    if (getLongAttr( &_textLineColor,
                     OUString( RTL_CONSTASCII_USTRINGPARAM("textline-color") ),
                     _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _hasValue |= 0x20;
        xProps->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("TextLineColor") ),
            makeAny( _textLineColor ) );
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
            xProps->setPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("FillColor") ), makeAny( _fillColor ) );
            return true;
        }
        return false;
    }
    _inited |= 0x10;

    if (getLongAttr(
            &_fillColor,
            OUString( RTL_CONSTASCII_USTRINGPARAM("fill-color") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _hasValue |= 0x10;
        xProps->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("FillColor") ),
            makeAny( _fillColor ) );
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
            xProps->setPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ), makeAny( _backgroundColor ) );
            return true;
        }
        return false;
    }
    _inited |= 0x1;

    if (getLongAttr(
            &_backgroundColor,
            OUString( RTL_CONSTASCII_USTRINGPARAM("background-color") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _hasValue |= 0x1;
        xProps->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("BackgroundColor") ), makeAny( _backgroundColor ) );
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
            xProps->setPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("Border") ), makeAny( _border ) );
            return true;
        }
        return false;
    }
    _inited |= 0x4;

    OUString aValue;
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("border") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("none") ))
        {
            _border = 0;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("3d") ))
        {
            _border = 1;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("simple") ))
        {
            _border = 2;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid border value!") ),
                Reference< XInterface >(), Any() );
        }

        _hasValue |= 0x4;
        xProps->setPropertyValue(
                OUString( RTL_CONSTASCII_USTRINGPARAM("Border") ), makeAny( _border ) );
    }
    return false;
}
//__________________________________________________________________________________________________
void StyleElement::setFontProperties(
    Reference< beans::XPropertySet > const & xProps )
{
    xProps->setPropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM("FontDescriptor") ), makeAny( _descr ) );
    xProps->setPropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM("FontEmphasisMark") ), makeAny( _fontEmphasisMark ) );
    xProps->setPropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM("FontRelief") ), makeAny( _fontRelief ) );
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
    bFontImport = getStringAttr(
        &_descr.Name, OUString( RTL_CONSTASCII_USTRINGPARAM("font-name") ),
        _xAttributes, _pImport->XMLNS_DIALOGS_UID );

    // dialog:font-height %numeric; #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-height") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.Height = (sal_Int16)toInt32( aValue );
        bFontImport = true;
    }
    // dialog:font-width %numeric; #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-width") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.Width = (sal_Int16)toInt32( aValue );
        bFontImport = true;
    }
    // dialog:font-stylename CDATA #IMPLIED
    bFontImport |= getStringAttr(
        &_descr.StyleName,
        OUString( RTL_CONSTASCII_USTRINGPARAM("font-stylename") ),
        _xAttributes, _pImport->XMLNS_DIALOGS_UID );

    // dialog:font-family "(decorative|modern|roman|script|swiss|system)" #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-family") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("decorative") ))
        {
            _descr.Family = awt::FontFamily::DECORATIVE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("modern") ))
        {
            _descr.Family = awt::FontFamily::MODERN;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("roman") ))
        {
            _descr.Family = awt::FontFamily::ROMAN;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("script") ))
        {
            _descr.Family = awt::FontFamily::SCRIPT;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("swiss") ))
        {
            _descr.Family = awt::FontFamily::SWISS;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("system") ))
        {
            _descr.Family = awt::FontFamily::SYSTEM;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-family style!") ),
                Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-charset "(ansi|mac|ibmpc_437|ibmpc_850|ibmpc_860|ibmpc_861|ibmpc_863|ibmpc_865|system|symbol)" #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-charset") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ansi") ))
        {
            _descr.CharSet = awt::CharSet::ANSI;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("mac") ))
        {
            _descr.CharSet = awt::CharSet::MAC;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ibmpc_437") ))
        {
            _descr.CharSet = awt::CharSet::IBMPC_437;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ibmpc_850") ))
        {
            _descr.CharSet = awt::CharSet::IBMPC_850;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ibmpc_860") ))
        {
            _descr.CharSet = awt::CharSet::IBMPC_860;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ibmpc_861") ))
        {
            _descr.CharSet = awt::CharSet::IBMPC_861;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ibmpc_863") ))
        {
            _descr.CharSet = awt::CharSet::IBMPC_863;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ibmpc_865") ))
        {
            _descr.CharSet = awt::CharSet::IBMPC_865;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("system") ))
        {
            _descr.CharSet = awt::CharSet::SYSTEM;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("symbol") ))
        {
            _descr.CharSet = awt::CharSet::SYMBOL;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-charset style!") ),
                Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-pitch "(fixed|variable)" #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-pitch") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("fixed") ))
        {
            _descr.Pitch = awt::FontPitch::FIXED;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("variable") ))
        {
            _descr.Pitch = awt::FontPitch::VARIABLE;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-pitch style!") ),
                Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-charwidth CDATA #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-charwidth") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.CharacterWidth = aValue.toFloat();
        bFontImport = true;
    }
    // dialog:font-weight CDATA #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-weight") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.Weight = aValue.toFloat();
        bFontImport = true;
    }

    // dialog:font-slant "(oblique|italic|reverse_oblique|reverse_italic)" #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-slant") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("oblique") ))
        {
            _descr.Slant = awt::FontSlant_OBLIQUE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("italic") ))
        {
            _descr.Slant = awt::FontSlant_ITALIC;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("reverse_oblique") ))
        {
            _descr.Slant = awt::FontSlant_REVERSE_OBLIQUE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("reverse_italic") ))
        {
            _descr.Slant = awt::FontSlant_REVERSE_ITALIC;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-slant style!") ),
                Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-underline "(single|double|dotted|dash|longdash|dashdot|dashdotdot|smallwave|wave|doublewave|bold|bolddotted|bolddash|boldlongdash|bolddashdot|bolddashdotdot|boldwave)" #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-underline") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("single") ))
        {
            _descr.Underline = awt::FontUnderline::SINGLE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("double") ))
        {
            _descr.Underline = awt::FontUnderline::DOUBLE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dotted") ))
        {
            _descr.Underline = awt::FontUnderline::DOTTED;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dash") ))
        {
            _descr.Underline = awt::FontUnderline::DASH;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("longdash") ))
        {
            _descr.Underline = awt::FontUnderline::LONGDASH;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dashdot") ))
        {
            _descr.Underline = awt::FontUnderline::DASHDOT;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dashdotdot") ))
        {
            _descr.Underline = awt::FontUnderline::DASHDOTDOT;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("smallwave") ))
        {
            _descr.Underline = awt::FontUnderline::SMALLWAVE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("wave") ))
        {
            _descr.Underline = awt::FontUnderline::WAVE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("doublewave") ))
        {
            _descr.Underline = awt::FontUnderline::DOUBLEWAVE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bold") ))
        {
            _descr.Underline = awt::FontUnderline::BOLD;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bolddotted") ))
        {
            _descr.Underline = awt::FontUnderline::BOLDDOTTED;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bolddash") ))
        {
            _descr.Underline = awt::FontUnderline::BOLDDASH;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("boldlongdash") ))
        {
            _descr.Underline = awt::FontUnderline::BOLDLONGDASH;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bolddashdot") ))
        {
            _descr.Underline = awt::FontUnderline::BOLDDASHDOT;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bolddashdotdot") ))
        {
            _descr.Underline = awt::FontUnderline::BOLDDASHDOTDOT;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("boldwave") ))
        {
            _descr.Underline = awt::FontUnderline::BOLDWAVE;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-underline style!") ),
                Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-strikeout "(single|double|bold|slash|x)" #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-strikeout") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("single") ))
        {
            _descr.Strikeout = awt::FontStrikeout::SINGLE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("double") ))
        {
            _descr.Strikeout = awt::FontStrikeout::DOUBLE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bold") ))
        {
            _descr.Strikeout = awt::FontStrikeout::BOLD;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("slash") ))
        {
            _descr.Strikeout = awt::FontStrikeout::SLASH;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("x") ))
        {
            _descr.Strikeout = awt::FontStrikeout::X;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-strikeout style!") ),
                Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // dialog:font-orientation CDATA #IMPLIED
    if (getStringAttr(
            &aValue,
            OUString( RTL_CONSTASCII_USTRINGPARAM("font-orientation") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        _descr.Orientation = aValue.toFloat();
        bFontImport = true;
    }
    // dialog:font-kerning %boolean; #IMPLIED
    bFontImport |= getBoolAttr(
        &_descr.Kerning,
        OUString( RTL_CONSTASCII_USTRINGPARAM("font-kerning") ),
        _xAttributes, _pImport->XMLNS_DIALOGS_UID );
    // dialog:font-wordlinemode %boolean; #IMPLIED
    bFontImport |= getBoolAttr(
        &_descr.WordLineMode,
        OUString( RTL_CONSTASCII_USTRINGPARAM("font-wordlinemode") ),
        _xAttributes, _pImport->XMLNS_DIALOGS_UID );

    // dialog:font-type "(raster|device|scalable)" #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-type") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("raster") ))
        {
            _descr.Type = awt::FontType::RASTER;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("device") ))
        {
            _descr.Type = awt::FontType::DEVICE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("scalable") ))
        {
            _descr.Type = awt::FontType::SCALABLE;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-type style!") ),
                Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }

    // additional properties which are not part of the FontDescriptor struct
    // dialog:font-relief (none|embossed|engraved) #IMPLIED
    if (getStringAttr(
            &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("font-relief") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("none") ))
        {
            _fontRelief = awt::FontRelief::NONE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("embossed") ))
        {
            _fontRelief = awt::FontRelief::EMBOSSED;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("engraved") ))
        {
            _fontRelief = awt::FontRelief::ENGRAVED;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-relief style!") ),
                Reference< XInterface >(), Any() );
        }
        bFontImport = true;
    }
    // dialog:font-emphasismark (none|dot|circle|disc|accent|above|below) #IMPLIED
    if (getStringAttr(
            &aValue,
            OUString( RTL_CONSTASCII_USTRINGPARAM("font-emphasismark") ),
            _xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("none") ))
        {
            _fontEmphasisMark = awt::FontEmphasisMark::NONE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dot") ))
        {
            _fontEmphasisMark = awt::FontEmphasisMark::DOT;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("circle") ))
        {
            _fontEmphasisMark = awt::FontEmphasisMark::CIRCLE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("disc") ))
        {
            _fontEmphasisMark = awt::FontEmphasisMark::DISC;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("accent") ))
        {
            _fontEmphasisMark = awt::FontEmphasisMark::ACCENT;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("above") ))
        {
            _fontEmphasisMark = awt::FontEmphasisMark::ABOVE;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("below") ))
        {
            _fontEmphasisMark = awt::FontEmphasisMark::BELOW;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid font-emphasismark style!") ),
                Reference< XInterface >(), Any() );
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
    if (aValue.getLength())
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
    if (aValue.getLength())
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
    if (aValue.getLength())
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
    if (aValue.getLength())
    {
        _xControlModel->setPropertyValue( rPropName, makeAny( toInt32( aValue ) + nOffset ) );
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
    if (aValue.getLength())
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
    if (aAlign.getLength())
    {
        sal_Int16 nAlign;
        if (aAlign.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("left") ))
        {
            nAlign = 0;
        }
        else if (aAlign.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("center") ))
        {
            nAlign = 1;
        }
        else if (aAlign.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("right") ))
        {
            nAlign = 2;
        }
        else if (aAlign.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("none") ))
        {
            nAlign = 0; // default
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid align value!") ),
                Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nAlign ) );
        return true;
    }
    return false;
}
//__________________________________________________________________________________________________
bool ImportContext::importImageAlignProperty(
    OUString const & rPropName, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes )
{
    OUString aAlign(
        xAttributes->getValueByUidName(
            _pImport->XMLNS_DIALOGS_UID, rAttrName ) );
    if (aAlign.getLength())
    {
        sal_Int16 nAlign;
        if (aAlign.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("left") ))
        {
            nAlign = 0;
        }
        else if (aAlign.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("top") ))
        {
            nAlign = 1;
        }
        else if (aAlign.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("right") ))
        {
            nAlign = 2;
        }
        else if (aAlign.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bottom") ))
        {
            nAlign = 3;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid image align value!") ),
                Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nAlign ) );
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
    if (buttonType.getLength())
    {
        sal_Int16 nButtonType;
        if (buttonType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("standard") ))
        {
            nButtonType = awt::PushButtonType_STANDARD;
        }
        else if (buttonType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ok") ))
        {
            nButtonType = awt::PushButtonType_OK;
        }
        else if (buttonType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("cancel") ))
        {
            nButtonType = awt::PushButtonType_CANCEL;
        }
        else if (buttonType.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("help") ))
        {
            nButtonType = awt::PushButtonType_HELP;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid button-type value!") ),
                Reference< XInterface >(), Any() );
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
    if (aFormat.getLength())
    {
        sal_Int16 nFormat;
        if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("system_short") ))
        {
            nFormat = 0;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("system_short_YY") ))
        {
            nFormat = 1;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("system_short_YYYY") ))
        {
            nFormat = 2;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("system_long") ))
        {
            nFormat = 3;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short_DDMMYY") ))
        {
            nFormat = 4;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short_MMDDYY") ))
        {
            nFormat = 5;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short_YYMMDD") ))
        {
            nFormat = 6;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short_DDMMYYYY") ))
        {
            nFormat = 7;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short_MMDDYYYY") ))
        {
            nFormat = 8;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short_YYYYMMDD") ))
        {
            nFormat = 9;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short_YYMMDD_DIN5008") ))
        {
            nFormat = 10;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("short_YYYYMMDD_DIN5008") ))
        {
            nFormat = 11;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid date-format value!") ),
                Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nFormat ) );
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
    if (aFormat.getLength())
    {
        sal_Int16 nFormat;
        if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("24h_short") ))
        {
            nFormat = 0;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("24h_long") ))
        {
            nFormat = 1;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("12h_short") ))
        {
            nFormat = 2;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("12h_long") ))
        {
            nFormat = 3;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Duration_short") ))
        {
            nFormat = 4;
        }
        else if (aFormat.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("Duration_long") ))
        {
            nFormat = 5;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid time-format value!") ),
                Reference< XInterface >(), Any() );
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
    if (aOrient.getLength())
    {
        sal_Int32 nOrient;
        if (aOrient.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("horizontal") ))
        {
            nOrient = 0;
        }
        else if (aOrient.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("vertical") ))
        {
            nOrient = 1;
        }
        else
        {
            throw xml::sax::SAXException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("invalid orientation value!") ),
                Reference< XInterface >(), Any() );
        }

        _xControlModel->setPropertyValue( rPropName, makeAny( nOrient ) );
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
    vector< Reference< xml::input::XElement > > const & rEvents )
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
                Reference< xml::input::XAttributes > xAttributes(
                    pEventElement->getAttributes() );

                // nowadays script events
                if (_pImport->XMLNS_SCRIPT_UID == nUid)
                {
                    if (!getStringAttr( &descr.ScriptType,
                                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                      "language") ),
                                        xAttributes,
                                        _pImport->XMLNS_SCRIPT_UID ) ||
                        !getStringAttr( &descr.ScriptCode,
                                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                      "macro-name") ),
                                        xAttributes,
                                        _pImport->XMLNS_SCRIPT_UID ))
                    {
                        throw xml::sax::SAXException(
                            OUString( RTL_CONSTASCII_USTRINGPARAM(
                                          "missing language or macro-name "
                                          "attribute(s) of event!") ),
                            Reference< XInterface >(), Any() );
                    }

                    OUString aLocation;
                    if (getStringAttr( &aLocation,
                                       OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                     "location") ),
                                       xAttributes,
                                       _pImport->XMLNS_SCRIPT_UID ))
                    {
                        // prepend location
                        OUStringBuffer buf( 48 );
                        buf.append( aLocation );
                        buf.append( (sal_Unicode)':' );
                        buf.append( descr.ScriptCode );
                        descr.ScriptCode = buf.makeStringAndClear();
                    }

                    // script:event element
                    if (aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
                    {
                        OUString aEventName;
                        if (! getStringAttr(
                                &aEventName,
                                OUString(
                                    RTL_CONSTASCII_USTRINGPARAM("event-name") ),
                                xAttributes,
                                _pImport->XMLNS_SCRIPT_UID ))
                        {
                            throw xml::sax::SAXException(
                                OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "missing event-name attribute!") ),
                                Reference< XInterface >(), Any() );
                        }

                        // lookup in table
                        OString str( OUStringToOString( aEventName, RTL_TEXTENCODING_ASCII_US ) );
                        StringTriple const * p = g_pEventTranslations;
                        while (p->first)
                        {
                            if (0 == ::rtl_str_compare( p->third, str.getStr() ))
                            {
                                descr.ListenerType = OUString( p->first, ::rtl_str_getLength( p->first ), RTL_TEXTENCODING_ASCII_US );
                                descr.EventMethod = OUString( p->second, ::rtl_str_getLength( p->second ), RTL_TEXTENCODING_ASCII_US );
                                break;
                            }
                            ++p;
                        }

                        if (! p->first)
                        {
                            throw xml::sax::SAXException(
                                OUString( RTL_CONSTASCII_USTRINGPARAM("no matching event-name found!") ),
                                Reference< XInterface >(), Any() );
                        }
                    }
                    else // script:listener-event element
                    {
                        OSL_ASSERT( aLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("listener-event") ) );

                        if (!getStringAttr(
                                &descr.ListenerType,
                                OUString( RTL_CONSTASCII_USTRINGPARAM(
                                              "listener-type") ),
                                xAttributes,
                                _pImport->XMLNS_SCRIPT_UID ) ||
                            !getStringAttr(
                                &descr.EventMethod,
                                OUString( RTL_CONSTASCII_USTRINGPARAM(
                                              "listener-method") ),
                                xAttributes, _pImport->XMLNS_SCRIPT_UID ))
                        {
                            throw xml::sax::SAXException(
                                OUString(
                                    RTL_CONSTASCII_USTRINGPARAM(
                                        "missing listener-type or "
                                        "listener-method attribute(s)!") ),
                                Reference< XInterface >(), Any() );
                        }
                        // optional listener param
                        getStringAttr(
                            &descr.AddListenerParam,
                            OUString( RTL_CONSTASCII_USTRINGPARAM(
                                          "listener-param") ),
                            xAttributes, _pImport->XMLNS_SCRIPT_UID );
                    }
                }
                else // deprecated dlg:event element
                {
                    OSL_ASSERT(
                        _pImport->XMLNS_DIALOGS_UID == nUid &&
                        aLocalName.equalsAsciiL(
                            RTL_CONSTASCII_STRINGPARAM("event") ) );

                    if (!getStringAttr( &descr.ListenerType,
                                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                      "listener-type") ),
                                        xAttributes,
                                        _pImport->XMLNS_DIALOGS_UID ) ||
                        !getStringAttr( &descr.EventMethod,
                                        OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                      "event-method") ),
                                        xAttributes,
                                        _pImport->XMLNS_DIALOGS_UID ))
                    {
                        throw xml::sax::SAXException(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("missing listener-type or event-method attribute(s)!") ),
                            Reference< XInterface >(), Any() );
                    }

                    getStringAttr(
                        &descr.ScriptType,
                        OUString( RTL_CONSTASCII_USTRINGPARAM("script-type") ),
                        xAttributes, _pImport->XMLNS_DIALOGS_UID );
                    getStringAttr(
                        &descr.ScriptCode,
                        OUString( RTL_CONSTASCII_USTRINGPARAM("script-code") ),
                        xAttributes, _pImport->XMLNS_DIALOGS_UID );
                    getStringAttr(
                        &descr.AddListenerParam,
                        OUString( RTL_CONSTASCII_USTRINGPARAM("param") ),
                        xAttributes, _pImport->XMLNS_DIALOGS_UID );
                }

                OUStringBuffer buf( 48 );
                buf.append( descr.ListenerType );
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("::") );
                buf.append( descr.EventMethod );
                xEvents->insertByName( buf.makeStringAndClear(), makeAny( descr ) );
            }
        }
    }
}
//__________________________________________________________________________________________________
void ImportContext::importDefaults(
    sal_Int32 nBaseX, sal_Int32 nBaseY,
    Reference< xml::input::XAttributes > const & xAttributes,
    bool supportPrintable )
{
    _xControlModel->setPropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Name") ),
        makeAny( _aId ) );

    importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("TabIndex") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("tab-index") ),
                         xAttributes );

    sal_Bool bDisable;
    if (getBoolAttr(
            &bDisable, OUString( RTL_CONSTASCII_USTRINGPARAM("disabled") ),
            xAttributes, _pImport->XMLNS_DIALOGS_UID ) &&
        bDisable)
    {
        _xControlModel->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Enabled") ), makeAny( sal_False ) );
    }

    if (!importLongProperty( nBaseX,
                             OUString( RTL_CONSTASCII_USTRINGPARAM("PositionX") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("left") ),
                             xAttributes ) ||
        !importLongProperty( nBaseY,
                             OUString( RTL_CONSTASCII_USTRINGPARAM("PositionY") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("top") ),
                             xAttributes ) ||
        !importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Width") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("width") ),
                             xAttributes ) ||
        !importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("height") ),
                             xAttributes ))
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("missing pos size attribute(s)!") ),
            Reference< XInterface >(), Any() );
    }

    if (supportPrintable)
    {
        importBooleanProperty(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Printable") ),
            OUString( RTL_CONSTASCII_USTRINGPARAM("printable") ),
            xAttributes );
    }

    sal_Int32 nLong;
    if (! getLongAttr(
            &nLong,
            OUString( RTL_CONSTASCII_USTRINGPARAM("page") ),
            xAttributes, _pImport->XMLNS_DIALOGS_UID ))
    {
        nLong = 0;
    }
    _xControlModel->setPropertyValue(
        OUString( RTL_CONSTASCII_USTRINGPARAM("Step") ),
        makeAny( nLong ) );

    importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tag") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("tag") ),
                          xAttributes );
    importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("HelpText") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("help-text") ),
                          xAttributes );
    importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("HelpURL") ),
                          OUString( RTL_CONSTASCII_USTRINGPARAM("help-url") ),
                          xAttributes );
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
    OUString const & rWhitespaces )
    throw (xml::sax::SAXException, RuntimeException)
{
    // not used
}
//__________________________________________________________________________________________________
void ElementBase::characters( OUString const & rChars )
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
    OUString const & Target, OUString const & Data )
    throw (xml::sax::SAXException, RuntimeException)
{
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > ElementBase::startChildElement(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("unexpected element!") ),
        Reference< XInterface >(), Any() );
}

//__________________________________________________________________________________________________
ElementBase::ElementBase(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::input::XAttributes > const & xAttributes,
    ElementBase * pParent, DialogImport * pImport )
    SAL_THROW( () )
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
    SAL_THROW( () )
{
    _pImport->release();

    if (_pParent)
    {
        _pParent->release();
    }

#if OSL_DEBUG_LEVEL > 1
    OString aStr( OUStringToOString( _aLocalName, RTL_TEXTENCODING_ASCII_US ) );
    OSL_TRACE( "ElementBase::~ElementBase(): %s\n", aStr.getStr() );
#endif
}

//##################################################################################################

// XRoot
//

//______________________________________________________________________________
void DialogImport::startDocument(
    Reference< container::XNameAccess > const & xUidMapping )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (!(xUidMapping->getByName( OUSTR(XMLNS_DIALOGS_URI) ) >>=
          XMLNS_DIALOGS_UID) ||
        !(xUidMapping->getByName( OUSTR(XMLNS_SCRIPT_URI) ) >>=
          XMLNS_SCRIPT_UID))
    {
        throw xml::sax::SAXException(
            OUSTR("cannot get uids!"), Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void DialogImport::endDocument()
    throw (xml::sax::SAXException, RuntimeException)
{
    // ignored
}
//__________________________________________________________________________________________________
void DialogImport::processingInstruction(
    OUString const & rTarget, OUString const & rData )
    throw (xml::sax::SAXException, RuntimeException)
{
    // ignored for now: xxx todo
}
//__________________________________________________________________________________________________
void DialogImport::setDocumentLocator(
    Reference< xml::sax::XLocator > const & xLocator )
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
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // window
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("window") ))
    {
        return new WindowElement( rLocalName, xAttributes, 0, this );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM(
                          "illegal root element (expected window) given: ") ) +
            rLocalName, Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
DialogImport::~DialogImport()
    SAL_THROW( () )
{
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "DialogImport::~DialogImport().\n" );
#endif
}
//__________________________________________________________________________________________________
Reference< util::XNumberFormatsSupplier > const & DialogImport::getNumberFormatsSupplier()
{
    if (! _xSupplier.is())
    {
        Reference< XComponentContext > xContext( getComponentContext() );
        Reference< util::XNumberFormatsSupplier > xSupplier(
            xContext->getServiceManager()->createInstanceWithContext(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                              "com.sun.star.util.NumberFormatsSupplier") ),
                xContext ), UNO_QUERY );

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
    SAL_THROW( () )
{
    _styleNames.push_back( rStyleId );
    _styles.push_back( xStyle );
}
//__________________________________________________________________________________________________
Reference< xml::input::XElement > DialogImport::getStyle(
    OUString const & rStyleId ) const
    SAL_THROW( () )
{
    for ( size_t nPos = 0; nPos < _styleNames.size(); ++nPos )
    {
        if (_styleNames[ nPos ] == rStyleId)
        {
            return _styles[ nPos ];
        }
    }
    return 0;
}

//##################################################################################################

//==================================================================================================
Reference< xml::sax::XDocumentHandler > SAL_CALL importDialogModel(
    Reference< container::XNameContainer > const & xDialogModel,
    Reference< XComponentContext > const & xContext )
    SAL_THROW( (Exception) )
{
    return ::xmlscript::createDocumentHandler(
        static_cast< xml::input::XRoot * >(
            new DialogImport( xContext, xDialogModel ) ) );
}

}
