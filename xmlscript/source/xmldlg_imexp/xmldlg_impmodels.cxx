/*************************************************************************
 *
 *  $RCSfile: xmldlg_impmodels.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-28 18:22:07 $
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

#include <com/sun/star/beans/XPropertySet.hpp>


using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{

// patternfield
//__________________________________________________________________________________________________
Reference< xml::XImportContext > PatternFieldElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void PatternFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlPatternFieldModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("EditMask") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("edit-mask") ),
                              _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LiteralMask") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("literal-mask") ),
                              _xAttributes );
    ctx.importEvents( _events );
}

//##################################################################################################

// timefield
//__________________________________________________________________________________________________
Reference< xml::XImportContext > TimeFieldElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TimeFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlTimeFieldModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importTimeFormatProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeFormat") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("time-format") ),
                                  _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Time") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeMin") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("TimeMax") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                            _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    ctx.importEvents( _events );
}

//##################################################################################################

// numericfield
//__________________________________________________________________________________________________
Reference< xml::XImportContext > NumericFieldElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void NumericFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlNumericFieldModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DecimalAccuracy") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("decimal-accuracy") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowThousandsSeparator") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("thousands-separator") ),
                               _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Value") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMin") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMax") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueStep") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-step") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    ctx.importEvents( _events );
}

//##################################################################################################

// datefield
//__________________________________________________________________________________________________
Reference< xml::XImportContext > DateFieldElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void DateFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlDateFieldModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importDateFormatProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DateFormat") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("date-format") ),
                                  _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Date") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DateMin") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                            _xAttributes );
    ctx.importLongProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DateMax") ),
                            OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                            _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    ctx.importEvents( _events );
}

//##################################################################################################

// currencyfield
//__________________________________________________________________________________________________
Reference< xml::XImportContext > CurrencyFieldElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void CurrencyFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlCurrencyFieldModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("StrictFormat") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("strict-format") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("CurrencySymbol") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("currency-symbol") ),
                              _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DecimalAccuracy") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("decimal-accuracy") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ShowThousandsSeparator") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("thousands-separator") ),
                               _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Value") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMin") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-min") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueMax") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-max") ),
                              _xAttributes );
    ctx.importDoubleProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ValueStep") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value-step") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Spin") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    ctx.importEvents( _events );
}

//##################################################################################################

// filecontrol
//__________________________________________________________________________________________________
Reference< xml::XImportContext > FileControlElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void FileControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFileControlModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importEvents( _events );
}

//##################################################################################################

// imagecontrol
//__________________________________________________________________________________________________
Reference< xml::XImportContext > ImageControlElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ImageControlElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlImageControlModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ImageURL") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("src") ),
                              _xAttributes );
    ctx.importEvents( _events );
}

//##################################################################################################

// textfield
//__________________________________________________________________________________________________
Reference< xml::XImportContext > TextElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TextElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedTextModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                               _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importEvents( _events );
}

//##################################################################################################

// edit
//__________________________________________________________________________________________________
Reference< xml::XImportContext > TextFieldElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void TextFieldElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlEditModel") ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importAlignProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Align") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("align") ),
                             _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("HardLineBreaks") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("hard-linebreaks") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("HScroll") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("hscroll") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("VScroll") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("vscroll") ),
                               _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("maxlength") ),
                             _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiLine") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiline") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                                  _xAttributes );
    OUString aValue;
    if (getStringAttr( &aValue, OUString( RTL_CONSTASCII_USTRINGPARAM("echochar") ), _xAttributes ) &&
        aValue.getLength() > 0)
    {
        OSL_ENSURE( aValue.getLength() == 1, "### more than one character given for echochar!" );
        sal_Int16 nChar = (sal_Int16)aValue[ 0 ];
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("EchoChar") ),
                                         makeAny( nChar ) );
    }

    ctx.importEvents( _events );
}

//##################################################################################################

// titledbox
//__________________________________________________________________________________________________
Reference< xml::XImportContext > TitledBoxElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // title
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("title") ))
    {
        getStringAttr( &_label, OUString( RTL_CONSTASCII_USTRINGPARAM("value") ), xAttributes );

        return new ElementBase( rLocalName, xAttributes, this, _pImport );
    }
    // radio
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("radio") ))
    {
        // dont create radios here, => titledbox must be inserted first due to radio grouping,
        // possible predecessors!
        Reference< xml::XImportContext > xRet(
            new RadioElement( rLocalName, xAttributes, this, _pImport ) );
        _radios.push_back( xRet );
        return xRet;
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        return BulletinBoardElement::createChildContext( nUid, rLocalName, xAttributes );
    }
}
//__________________________________________________________________________________________________
void TitledBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    {
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlGroupBoxModel") ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( 0, 0, _xAttributes ); // inherited from BulletinBoardElement

    if (_label.getLength())
    {
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                                         makeAny( _label ) );
    }
    }

    // create radios AFTER group box!
    for ( size_t nPos = 0; nPos < _radios.size(); ++nPos )
    {
        Reference< xml::XImportContext > xRadio( _radios[ nPos ] );
        Reference< xml::sax2::XExtendedAttributes > xAttributes( xRadio->getAttributes() );

        ControlImportContext ctx(
            _pImport, getControlId( xAttributes ),
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ) );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

        Reference< xml::XImportContext > xStyle( getStyle( xAttributes ) );
        if (xStyle.is())
        {
            StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
            pStyle->importTextColorStyle( xControlModel );
            pStyle->importFontStyle( xControlModel );
        }

        ctx.importDefaults( _nBasePosX, _nBasePosY, xAttributes );
        ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                                   xAttributes );
        ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                                  xAttributes );

        sal_Int16 nVal;
        sal_Bool bTriState = sal_False;
        sal_Bool bChecked = sal_False;
        getBoolAttr( &bTriState, OUString( RTL_CONSTASCII_USTRINGPARAM("tristate") ), xAttributes );
        if (getBoolAttr( &bChecked, OUString( RTL_CONSTASCII_USTRINGPARAM("checked") ), xAttributes ))
        {
            // has "checked" attribute
            nVal = (bChecked ? 1 : 0);
        }
        else
        {
            nVal = (bTriState ? 2 : 0); // if tristate set, but checked omitted => dont know!
        }

        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
                                         makeAny( nVal ) );

        ctx.importEvents( * static_cast< RadioElement * >( xRadio.get() )->getEvents() );
    }
}

//##################################################################################################

// radio
//__________________________________________________________________________________________________
Reference< xml::XImportContext > RadioElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}

//##################################################################################################

// radiogroup
//__________________________________________________________________________________________________
Reference< xml::XImportContext > RadioGroupElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // radio
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("radio") ))
    {
        // dont create radios here, => titledbox must be inserted first due to radio grouping,
        // possible predecessors!
        Reference< xml::XImportContext > xRet(
            new RadioElement( rLocalName, xAttributes, this, _pImport ) );
        _radios.push_back( xRet );
        return xRet;
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void RadioGroupElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    for ( size_t nPos = 0; nPos < _radios.size(); ++nPos )
    {
        Reference< xml::XImportContext > xRadio( _radios[ nPos ] );
        Reference< xml::sax2::XExtendedAttributes > xAttributes( xRadio->getAttributes() );

        ControlImportContext ctx(
            _pImport, getControlId( xAttributes ),
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ) );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

        Reference< xml::XImportContext > xStyle( getStyle( xAttributes ) );
        if (xStyle.is())
        {
            StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
            pStyle->importTextColorStyle( xControlModel );
            pStyle->importFontStyle( xControlModel );
        }

        ctx.importDefaults( _nBasePosX, _nBasePosY, xAttributes );
        ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                                   OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                                   xAttributes );
        ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                                  OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                                  xAttributes );

        sal_Int16 nVal;
        sal_Bool bTriState = sal_False;
        sal_Bool bChecked = sal_False;
        getBoolAttr( &bTriState, OUString( RTL_CONSTASCII_USTRINGPARAM("tristate") ), xAttributes );
        if (getBoolAttr( &bChecked, OUString( RTL_CONSTASCII_USTRINGPARAM("checked") ), xAttributes ))
        {
            // has "checked" attribute
            nVal = (bChecked ? 1 : 0);
        }
        else
        {
            nVal = (bTriState ? 2 : 0); // if tristate set, but checked omitted => dont know!
        }

        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
                                         makeAny( nVal ) );

        ctx.importEvents( * static_cast< RadioElement * >( xRadio.get() )->getEvents() );
    }
}

//##################################################################################################

// menupopup
//__________________________________________________________________________________________________
Reference< xml::XImportContext > MenuPopupElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // menuitem
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("menuitem") ))
    {
        OUString aValue( xAttributes->getValueByUidName(
            XMLNS_DIALOGS_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("value") ) ) );
        OSL_ENSURE( aValue.getLength(), "### menuitem has no value?" );
        if (aValue.getLength())
        {
            _itemValues.push_back( aValue );

            OUString aSel( xAttributes->getValueByUidName(
                XMLNS_DIALOGS_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("selected") ) ) );
            if (aSel.getLength() && aSel.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("true") ))
            {
                _itemSelected.push_back( _itemValues.size() -1 );
            }
        }
        return new ElementBase( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected menuitem!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
Sequence< OUString > MenuPopupElement::getItemValues()
{
    Sequence< OUString > aRet( _itemValues.size() );
    OUString * pRet = aRet.getArray();
    for ( size_t nPos = _itemValues.size(); nPos--; )
    {
        pRet[ nPos ] = _itemValues[ nPos ];
    }
    return aRet;
}
//__________________________________________________________________________________________________
Sequence< sal_Int16 > MenuPopupElement::getSelectedItems()
{
    Sequence< sal_Int16 > aRet( _itemSelected.size() );
    sal_Int16 * pRet = aRet.getArray();
    for ( size_t nPos = _itemSelected.size(); nPos--; )
    {
        pRet[ nPos ] = _itemSelected[ nPos ];
    }
    return aRet;
}

//##################################################################################################

// menulist
//__________________________________________________________________________________________________
Reference< xml::XImportContext > MenuListElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // menupopup
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("menupopup") ))
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, _pImport );
        return _popup;
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void MenuListElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlListBoxModel") ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiSelection") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiselection") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("linecount") ),
                             _xAttributes );

    MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
    xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ),
                                     makeAny( p->getItemValues() ) );
    xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("SelectedItems") ),
                                     makeAny( p->getSelectedItems() ) );

    ctx.importEvents( _events );
}

//##################################################################################################

// combobox
//__________________________________________________________________________________________________
Reference< xml::XImportContext > ComboBoxElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // menupopup
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("menupopup") ))
    {
        _popup = new MenuPopupElement( rLocalName, xAttributes, this, _pImport );
        return _popup;
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ComboBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlComboBoxModel") ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Autocomplete") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("autocomplete") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin") ),
                               _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("maxlength") ),
                             _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("linecount") ),
                             _xAttributes );

    MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
    xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ),
                                     makeAny( p->getItemValues() ) );

    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );

    ctx.importEvents( _events );
}

//##################################################################################################

// checkbox
//__________________________________________________________________________________________________
Reference< xml::XImportContext > CheckBoxElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void CheckBoxElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlCheckBoxModel") ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );

    sal_Bool bTriState = sal_False;
    if (getBoolAttr( &bTriState, OUString( RTL_CONSTASCII_USTRINGPARAM("tristate") ), _xAttributes ))
    {
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("TriState") ),
                                         makeAny( bTriState ) );
    }
    sal_Bool bChecked = sal_False;
    if (getBoolAttr( &bChecked, OUString( RTL_CONSTASCII_USTRINGPARAM("checked") ), _xAttributes ))
    {
        // has "checked" attribute
        sal_Int16 nVal = (bChecked ? 1 : 0);
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
                                         makeAny( nVal ) );
    }
    else
    {
        sal_Int16 nVal = (bTriState ? 2 : 0); // if tristate set, but checked omitted => dont know!
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
                                         makeAny( nVal ) );
    }

    ctx.importEvents( _events );
}

//##################################################################################################

// button
//__________________________________________________________________________________________________
Reference< xml::XImportContext > ButtonElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // event
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
    {
        return new EventElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected event element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void ButtonElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    ControlImportContext ctx(
        _pImport, getControlId( _xAttributes ),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlButtonModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle( _xAttributes ) );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _nBasePosX, _nBasePosY, _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Tabstop") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("tabstop") ),
                               _xAttributes );
    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultButton") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("default") ),
                               _xAttributes );

    ctx.importEvents( _events );
}

//##################################################################################################

// bulletinboard
//__________________________________________________________________________________________________
Reference< xml::XImportContext > BulletinBoardElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // button
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("button") ))
    {
        return new ButtonElement( rLocalName, xAttributes, this, _pImport );
    }
    // checkbox
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("checkbox") ))
    {
        return new CheckBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // combobox
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("combobox") ))
    {
        return new ComboBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // listbox
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("menulist") ))
    {
        return new MenuListElement( rLocalName, xAttributes, this, _pImport );
    }
    // radiogroup
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("radiogroup") ))
    {
        return new RadioGroupElement( rLocalName, xAttributes, this, _pImport );
    }
    // titledbox
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("titledbox") ))
    {
        return new TitledBoxElement( rLocalName, xAttributes, this, _pImport );
    }
    // text
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("text") ))
    {
        return new TextElement( rLocalName, xAttributes, this, _pImport );
    }
    // textfield
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("textfield") ))
    {
        return new TextFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // img
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("img") ))
    {
        return new ImageControlElement( rLocalName, xAttributes, this, _pImport );
    }
    // filecontrol
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("filecontrol") ))
    {
        return new FileControlElement( rLocalName, xAttributes, this, _pImport );
    }
    // currency-field
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("currencyfield") ))
    {
        return new CurrencyFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // date-field
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("datefield") ))
    {
        return new DateFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // date-field
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("numericfield") ))
    {
        return new NumericFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // time-field
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("timefield") ))
    {
        return new TimeFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // pattern-field
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("patternfield") ))
    {
        return new PatternFieldElement( rLocalName, xAttributes, this, _pImport );
    }
    // bulletinboard
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bulletinboard") ))
    {
        return new BulletinBoardElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected styles, bulletinboard or bulletinboard element, not: ") ) + rLocalName,
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
BulletinBoardElement::BulletinBoardElement(
    OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
    ElementBase * pParent, DialogImport * pImport )
    throw ()
    : ControlElement( rLocalName, xAttributes, pParent, pImport )
{
    OUString aValue( _xAttributes->getValueByUidName(
        XMLNS_DIALOGS_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("left") ) ) );
    if (aValue.getLength())
    {
        _nBasePosX += toInt32( aValue );
    }
    aValue = _xAttributes->getValueByUidName(
        XMLNS_DIALOGS_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("top") ) );
    if (aValue.getLength())
    {
        _nBasePosY += toInt32( aValue );
    }
}

//##################################################################################################

// style
//__________________________________________________________________________________________________
Reference< xml::XImportContext > StyleElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    throw xml::sax::SAXException(
        OUString( RTL_CONSTASCII_USTRINGPARAM("unexpected sub elements of style!") ),
        Reference< XInterface >(), Any() );
}
//__________________________________________________________________________________________________
void StyleElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< xml::sax2::XExtendedAttributes > xStyle;
    OUString aStyleId( _xAttributes->getValueByUidName(
        XMLNS_DIALOGS_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("style-id") ) ) );
    if (aStyleId.getLength())
    {
        _pImport->addStyle( aStyleId, this );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("missing style-id attribute!") ),
            Reference< XInterface >(), Any() );
    }
}

//##################################################################################################

// styles
//__________________________________________________________________________________________________
Reference< xml::XImportContext > StylesElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // style
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("style") ))
    {
        return new StyleElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected style element!") ),
            Reference< XInterface >(), Any() );
    }
}

//##################################################################################################

// window
//__________________________________________________________________________________________________
Reference< xml::XImportContext > WindowElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
    throw (xml::sax::SAXException, RuntimeException)
{
    if (XMLNS_DIALOGS_UID != nUid)
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("illegal namespace!") ),
            Reference< XInterface >(), Any() );
    }
    // styles
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("styles") ))
    {
        return new StylesElement( rLocalName, xAttributes, this, _pImport );
    }
    // bulletinboard
    else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("bulletinboard") ))
    {
        return new BulletinBoardElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected styles ot bulletinboard element!") ),
            Reference< XInterface >(), Any() );
    }
}
//__________________________________________________________________________________________________
void WindowElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
{
    Reference< beans::XPropertySet > xProps( _pImport->_xDialogModel, UNO_QUERY );
    OSL_ASSERT( xProps.is() );

    OUString aValue( _xAttributes->getValueByUidName(
        XMLNS_DIALOGS_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("width") ) ) );
    if (aValue.getLength())
    {
        xProps->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Width") ),
            makeAny( aValue.toInt32() ) );
    }
    aValue = _xAttributes->getValueByUidName(
        XMLNS_DIALOGS_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("height") ) );
    if (aValue.getLength())
    {
        xProps->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Height") ),
            makeAny( aValue.toInt32() ) );
    }
    aValue = _xAttributes->getValueByUidName(
        XMLNS_DIALOGS_UID, OUString( RTL_CONSTASCII_USTRINGPARAM("label") ) );
    if (aValue.getLength())
    {
        xProps->setPropertyValue(
            OUString( RTL_CONSTASCII_USTRINGPARAM("Title") ),
            makeAny( aValue ) );
    }
}

//##################################################################################################

// dialogs
//__________________________________________________________________________________________________
Reference< xml::XImportContext > DialogsElement::createChildContext(
    sal_Int32 nUid, OUString const & rLocalName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
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
        // new model
        _pImport->_xDialogModel = Reference< container::XNameContainer >::query( _pImport->_xMgr->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlDialogModel") ) ) );
        OSL_ASSERT( _pImport->_xDialogModel.is() );
        if (! _pImport->_xDialogModel.is())
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("could not instanciate dialog model!") ),
                Reference< XInterface >() );
        }

        _pImport->_xDialogModelFactory = Reference< lang::XMultiServiceFactory >::query(
            _pImport->_xDialogModel );
        OSL_ASSERT( _pImport->_xDialogModelFactory.is() );
        if (! _pImport->_xDialogModel.is())
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM("could not query for dialog model factory!") ),
                Reference< XInterface >() );
        }

        // new style set
        _pImport->_styleNames.clear();
        _pImport->_styles.clear();

        sal_Int32 nSize = _pImport->_pOutModels->getLength();
        _pImport->_pOutModels->realloc( nSize +1 );
        _pImport->_pOutModels->getArray()[ nSize ] = _pImport->_xDialogModel;

        return new WindowElement( rLocalName, xAttributes, this, _pImport );
    }
    else
    {
        throw xml::sax::SAXException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("expected window element, not ") ) +
            rLocalName, Reference< XInterface >(), Any() );
    }
}

};
