/*************************************************************************
 *
 *  $RCSfile: imp_share.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ab $ $Date: 2001-03-27 17:44:31 $
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
#include <xmlscript/xmllib_imexp.hxx>
#include <xmlscript/xmlmod_imexp.hxx>

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
inline sal_Int32 toInt32( OUString const & rStr ) SAL_THROW( () )
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

class ControlImportContext;

//==================================================================================================
struct DialogImport
    : public ::cppu::WeakImplHelper1< xml::XImporter >
{
    friend class ControlImportContext;

    vector< OUString > _styleNames;
    vector< Reference< xml::XImportContext > > _styles;

    Reference< container::XNameContainer > _xDialogModel;
    Reference< lang::XMultiServiceFactory > _xDialogModelFactory;

public:
    void addStyle(
        OUString const & rStyleId,
        Reference< xml::XImportContext > const & xStyle )
        SAL_THROW( () );
    Reference< xml::XImportContext > getStyle(
        OUString const & rStyleId ) const
        SAL_THROW( () );

    inline DialogImport( Reference< container::XNameContainer > const & xDialogModel )
        SAL_THROW( () )
        : _xDialogModel( xDialogModel )
        , _xDialogModelFactory( xDialogModel, UNO_QUERY )
        { OSL_ASSERT( _xDialogModel.is() && _xDialogModelFactory.is() ); }
    virtual ~DialogImport()
        SAL_THROW( () );

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
protected:
    DialogImport * _pImport;
    ElementBase * _pParent;

    OUString _aLocalName;
    Reference< xml::sax2::XExtendedAttributes > _xAttributes;

public:
    ElementBase(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () );
    virtual ~ElementBase()
        SAL_THROW( () );

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

    inline StylesElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
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

    inline StyleElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
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

    inline MenuPopupElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ElementBase( rLocalName, xAttributes, pParent, pImport )
        {}
};

//==================================================================================================
class ControlElement
    : public ElementBase
{
    friend class EventElement;

protected:
    sal_Int32 _nBasePosX, _nBasePosY;

    vector< Reference< xml::sax2::XExtendedAttributes > > _events;

    OUString getControlId(
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
    Reference< xml::XImportContext > getStyle(
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
public:
    vector< Reference< xml::sax2::XExtendedAttributes > > const * getEvents() SAL_THROW( () )
        { return &_events; }

    ControlElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () );
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

    void importEvents(
        vector< Reference< xml::sax2::XExtendedAttributes > > const & rEvents );

    void importDefaults(
        sal_Int32 nBaseX, sal_Int32 nBaseY,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );

    bool importStringProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
    bool importDoubleProperty(
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
    bool importLongProperty(
        sal_Int32 nOffset,
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
    bool importAlignProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
    bool importDateFormatProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes );
    bool importTimeFormatProperty(
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

    inline WindowElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class EventElement
    : public ElementBase
{
public:
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline EventElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ElementBase( rLocalName, xAttributes, pParent, pImport )
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

    inline BulletinBoardElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () );
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

    inline ButtonElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
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

    inline CheckBoxElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
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

    inline ComboBoxElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
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

    inline MenuListElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class RadioElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);

    inline RadioElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class RadioGroupElement
    : public ControlElement
{
    vector< Reference< xml::XImportContext > > _radios;
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline RadioGroupElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class TitledBoxElement
    : public BulletinBoardElement
{
    OUString _label;
    vector< Reference< xml::XImportContext > > _radios;
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline TitledBoxElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : BulletinBoardElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class TextElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline TextElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class TextFieldElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline TextFieldElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class ImageControlElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline ImageControlElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class FileControlElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline FileControlElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class CurrencyFieldElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline CurrencyFieldElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class DateFieldElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline DateFieldElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class NumericFieldElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline NumericFieldElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class TimeFieldElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline TimeFieldElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class PatternFieldElement
    : public ControlElement
{
public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline PatternFieldElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};


//==================================================================================================
// Library import

//==================================================================================================
struct LibraryImport
    : public ::cppu::WeakImplHelper1< xml::XImporter >
{
    friend class LibrariesElement;

    LibDescriptor*& mrpLibs;
    sal_Int32& mrnLibCount;

public:
    inline LibraryImport( LibDescriptor*& rpLibs, sal_Int32& rnLibCount )
        SAL_THROW( () )
        : mrpLibs( rpLibs )
        , mrnLibCount( rnLibCount ) {}
    virtual ~LibraryImport()
        SAL_THROW( () );

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
class LibElementBase
    : public ::cppu::WeakImplHelper1< xml::XImportContext >
{
protected:
    LibraryImport * _pImport;
    LibElementBase * _pParent;

    OUString _aLocalName;
    Reference< xml::sax2::XExtendedAttributes > _xAttributes;

public:
    LibElementBase(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        LibElementBase * pParent, LibraryImport * pImport )
        SAL_THROW( () );
    virtual ~LibElementBase()
        SAL_THROW( () );

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

class LibrariesElement : public LibElementBase
{
    friend class LibraryElement;

protected:
    vector< LibDescriptor > mLibDescriptors;

public:
    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    LibrariesElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        LibElementBase * pParent, LibraryImport * pImport )
        SAL_THROW( () )
        : LibElementBase( rLocalName, xAttributes, pParent, pImport )
        {}
};

//==================================================================================================

class LibraryElement : public LibElementBase
{
protected:
    vector< OUString > mElements;

public:

    virtual Reference< xml::XImportContext > SAL_CALL createChildContext(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    LibraryElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        LibElementBase * pParent, LibraryImport * pImport )
        SAL_THROW( () )
        : LibElementBase( rLocalName, xAttributes, pParent, pImport )
    {}
};


//==================================================================================================
// Script module import

//==================================================================================================
struct ModuleImport
    : public ::cppu::WeakImplHelper1< xml::XImporter >
{
    friend class ModuleElement;

    ModuleDescriptor& mrModuleDesc;

public:
    inline ModuleImport( ModuleDescriptor& rModuleDesc )
        SAL_THROW( () )
        : mrModuleDesc( rModuleDesc ) {}
    virtual ~ModuleImport()
        SAL_THROW( () );

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
class ModuleElement
    : public ::cppu::WeakImplHelper1< xml::XImportContext >
{
protected:
    ModuleImport * _pImport;
    ModuleElement * _pParent;

    OUString _aLocalName;
    Reference< xml::sax2::XExtendedAttributes > _xAttributes;

public:
    ModuleElement(
        OUString const & rLocalName,
        Reference< xml::sax2::XExtendedAttributes > const & xAttributes,
        ModuleElement * pParent, ModuleImport * pImport )
        SAL_THROW( () );
    virtual ~ModuleElement()
        SAL_THROW( () );

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

};
