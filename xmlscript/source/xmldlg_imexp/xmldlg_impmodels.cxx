/*************************************************************************
 *
 *  $RCSfile: xmldlg_impmodels.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-16 14:14:48 $
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
//      // event
//      else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
//      {
//          return new EventElement( rLocalName, xAttributes, this, _pImport );
//      }
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
        _pImport, getControlId(),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlListBoxModel") ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::XImportContext > xStyle( getStyle() );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _xAttributes );

    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MultiSelection") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("multiselection") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin-button") ),
                               _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("linecount") ),
                             _xAttributes );

    MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
    Reference< beans::XPropertySet > xProps( xControlModel );
    xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ),
                              makeAny( p->getItemValues() ) );
    xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("SelectedItems") ),
                              makeAny( p->getSelectedItems() ) );
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
//      // event
//      else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
//      {
//          return new EventElement( rLocalName, xAttributes, this, _pImport );
//      }
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
        _pImport, getControlId(),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlComboBoxModel") ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::XImportContext > xStyle( getStyle() );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importBorderStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _xAttributes );

    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("ReadOnly") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("readonly") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Autocomplete") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("autocomplete") ),
                               _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Dropdown") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("spin-button") ),
                               _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("MaxTextLen") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("max-textlen") ),
                             _xAttributes );
    ctx.importShortProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("LineCount") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("linecount") ),
                             _xAttributes );

    MenuPopupElement * p = static_cast< MenuPopupElement * >( _popup.get() );
    Reference< beans::XPropertySet > xProps( xControlModel );
    xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("StringItemList") ),
                              makeAny( p->getItemValues() ) );

    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Text") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
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
//      // event
//      else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
//      {
//          return new EventElement( rLocalName, xAttributes, this, _pImport );
//      }
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
        _pImport, getControlId(),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlCheckBoxModel") ) );
    Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );

    Reference< xml::XImportContext > xStyle( getStyle() );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _xAttributes );

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
        sal_Int16 nVal = (bTriState ? 2 : 0);
        xControlModel->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("State") ),
                                         makeAny( nVal ) );
    }
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
//      // event
//      else if (rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("event") ))
//      {
//          return new EventElement( rLocalName, xAttributes, this, _pImport );
//      }
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
        _pImport, getControlId(),
        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlButtonModel") ) );

    Reference< xml::XImportContext > xStyle( getStyle() );
    if (xStyle.is())
    {
        StyleElement * pStyle = static_cast< StyleElement * >( xStyle.get () );
        Reference< beans::XPropertySet > xControlModel( ctx.getControlModel() );
        pStyle->importBackgroundColorStyle( xControlModel );
        pStyle->importTextColorStyle( xControlModel );
        pStyle->importFontStyle( xControlModel );
    }

    ctx.importDefaults( _xAttributes );

    ctx.importStringProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("Label") ),
                              OUString( RTL_CONSTASCII_USTRINGPARAM("value") ),
                              _xAttributes );
    ctx.importBooleanProperty( OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultButton") ),
                               OUString( RTL_CONSTASCII_USTRINGPARAM("default") ),
                               _xAttributes );
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
void BulletinBoardElement::endElement()
    throw (xml::sax::SAXException, RuntimeException)
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

};
