/*************************************************************************
 *
 *  $RCSfile: imp_share.hxx,v $
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
#include <vector>

#include <xmlscript/xmldlg_imexp.hxx>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>

#include <com/sun/star/xml/sax2/XExtendedAttributes.hpp>
#include <com/sun/star/xml/XImportContext.hpp>
#include <com/sun/star/xml/XImporter.hpp>


using namespace ::rtl;
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace xmlscript
{
//
inline sal_Int32 toInt32( OUString const & rStr )
{
    sal_Int32 nVal;
    if (rStr.getLength() > 2 && rStr[ 0 ] == '0' && rStr[ 1 ] == 'x')
    {
        nVal = rStr.toInt32( 16 );
    }
    else
    {
        nVal = rStr.toInt32();
    }
    return nVal;
}
inline bool getBoolAttr(
    sal_Bool * pRet, OUString const & rAttrName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
{
    OUString aValue( xAttributes->getValueByUidName( XMLNS_DIALOGS_UID, rAttrName ) );
    if (aValue.getLength())
    {
        if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("true") ))
        {
            *pRet = sal_True;
            return true;
        }
        else if (aValue.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("false") ))
        {
            *pRet = sal_False;
            return true;
        }
        else
        {
            throw xml::sax::SAXException(
                rAttrName + OUString( RTL_CONSTASCII_USTRINGPARAM(": no boolean value (true|false)!") ),
                Reference< XInterface >(), Any() );
        }
    }
    return false;
}
inline bool getStringAttr(
    OUString * pRet, OUString const & rAttrName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
{
    *pRet = xAttributes->getValueByUidName( XMLNS_DIALOGS_UID, rAttrName );
    return (pRet->getLength() > 0);
}
inline bool getLongAttr(
    sal_Int32 * pRet, OUString const & rAttrName,
    Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
{
    OUString aValue( xAttributes->getValueByUidName( XMLNS_DIALOGS_UID, rAttrName ) );
    if (aValue.getLength())
    {
        *pRet = toInt32( aValue );
        return true;
    }
    return false;
}
//

//==================================================================================================
struct DialogImport
    : public ::cppu::WeakImplHelper1< xml::XImporter >
{
    vector< OUString > _styleNames;
    vector< Reference< xml::XImportContext > > _styles;

public:
    Reference< container::XNameContainer > _xDialogModel;
    Reference< lang::XMultiServiceFactory > _xDialogModelFactory;

    void addStyle(
        OUString const & rStyleId,
        Reference< xml::XImportContext > const & xStyle )
        throw ();
    Reference< xml::XImportContext > getStyle(
        OUString const & rStyleId ) const
        throw ();

    inline DialogImport(
        Reference< container::XNameContainer > const & xDialogModel )
        : _xDialogModel( xDialogModel )
        , _xDialogModelFactory( xDialogModel, UNO_QUERY )
        {}

    // XImporter
    virtual void SAL_CALL startDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        Reference< xml::sax::XLocator > const & xLocator )
        throw (xml::sax::SAXException, RuntimeException);
    virtual Reference< xml::XImportContext > SAL_CALL createRootContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//==================================================================================================
class ElementBase
    : public ::cppu::WeakImplHelper1< xml::XImportContext >
{
public:
    OUString _aLocalName;
    Reference< xml::sax2::XExtendedAttributes > _xAttributes;

    ElementBase * _pParent;
    DialogImport * _pImport;

    ElementBase(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ();
    virtual ~ElementBase()
        throw ();

    // XImportContext
    virtual Reference< xml::XImportContext > SAL_CALL getParent()
        throw (RuntimeException);
    virtual OUString SAL_CALL getLocalName()
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getUid()
        throw (RuntimeException);
    virtual Reference< xml::sax2::XExtendedAttributes > SAL_CALL getAttributes()
        throw (RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL characters( OUString const & rChars )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//==================================================================================================
class StylesElement
    : public ElementBase
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);

    StylesElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ElementBase( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class StyleElement
    : public ElementBase
{
    sal_Int32 _backgroundColor;
    sal_Int32 _textColor;
    sal_Int16 _border;
    awt::FontDescriptor _descr;

    short _inited, _hasValue;

public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    bool importTextColorStyle(
        Reference< beans::XPropertySet > const & xProps );
    bool importBackgroundColorStyle(
        Reference< beans::XPropertySet > const & xProps );
    bool importFontStyle(
        Reference< beans::XPropertySet > const & xProps );
    bool importBorderStyle(
        Reference< beans::XPropertySet > const & xProps );

    StyleElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ElementBase( rLocalName, xAttributes, pParent, pImport )
        , _inited( 0 )
        , _hasValue( 0 )
        {}
};
//==================================================================================================
class MenuPopupElement
    : public ElementBase
{
    vector< OUString > _itemValues;
    vector< sal_Int16 > _itemSelected;
public:
    Sequence< OUString > getItemValues();
    Sequence< sal_Int16 > getSelectedItems();

    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);

    MenuPopupElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ElementBase( rLocalName, xAttributes, pParent, pImport )
        {}
};

//==================================================================================================
class ControlElement
    : public ElementBase
{
protected:
    sal_Int32 _nBasePosX, _nBasePosY;

    OUString getControlId();
    Reference< xml::XImportContext > getStyle();

public:

    ControlElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ();
};
//==================================================================================================
class ControlImportContext
{
    DialogImport * _pImport;
    OUString _aId;

    Reference< beans::XPropertySet > _xControlModel;

public:
    inline ControlImportContext( DialogImport * pImport,
                                 OUString const & rId, OUString const & rControlName )
        : _pImport( pImport )
        , _aId( rId )
        , _xControlModel( pImport->_xDialogModelFactory->createInstance( rControlName ), UNO_QUERY )
        { OSL_ASSERT( _xControlModel.is() ); }
    inline ~ControlImportContext()
        { _pImport->_xDialogModel->insertByName( _aId, makeAny( Reference< awt::XControlModel >::query( _xControlModel ) ) ); }

    inline Reference< beans::XPropertySet > getControlModel()
        { return _xControlModel; }

    void importDefaults(
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );

    bool importStringProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
    bool importBooleanProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
    bool importShortProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
    bool importLongProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
};

//==================================================================================================
class WindowElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    WindowElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class BulletinBoardElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    BulletinBoardElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class ButtonElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    ButtonElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class CheckBoxElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    CheckBoxElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class ComboBoxElement
    : public ControlElement
{
    Reference< xml::XImportContext > _popup;
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    ComboBoxElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class MenuListElement
    : public ControlElement
{
    Reference< xml::XImportContext > _popup;
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    MenuListElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        throw ()
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

};
