/*************************************************************************
 *
 *  $RCSfile: imp_share.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 09:19:30 $
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

#include <xmlscript/xmldlg_imexp.hxx>
#include <xmlscript/xmllib_imexp.hxx>
#include <xmlscript/xmlmod_imexp.hxx>

#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <com/sun/star/util/XNumberFormatsSupplier.hpp>

#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>

#include <com/sun/star/xml/input/XRoot.hpp>

#include <vector>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


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
        nVal = rStr.copy( 2 ).toInt32( 16 );
    }
    else
    {
        nVal = rStr.toInt32();
    }
    return nVal;
}
inline bool getBoolAttr(
    sal_Bool * pRet, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes,
    sal_Int32 nUid )
{
    OUString aValue( xAttributes->getValueByUidName( nUid, rAttrName ) );
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
    Reference< xml::input::XAttributes > const & xAttributes,
    sal_Int32 nUid )
{
    *pRet = xAttributes->getValueByUidName( nUid, rAttrName );
    return (pRet->getLength() > 0);
}
inline bool getLongAttr(
    sal_Int32 * pRet, OUString const & rAttrName,
    Reference< xml::input::XAttributes > const & xAttributes,
    sal_Int32 nUid )
{
    OUString aValue( xAttributes->getValueByUidName( nUid, rAttrName ) );
    if (aValue.getLength())
    {
        *pRet = toInt32( aValue );
        return true;
    }
    return false;
}

class ImportContext;

//==================================================================================================
struct DialogImport
    : public ::cppu::WeakImplHelper1< xml::input::XRoot >
{
    friend class ImportContext;

    Reference< XComponentContext > _xContext;
    Reference< util::XNumberFormatsSupplier > _xSupplier;

    vector< OUString > _styleNames;
    vector< Reference< xml::input::XElement > > _styles;

    Reference< container::XNameContainer > _xDialogModel;
    Reference< lang::XMultiServiceFactory > _xDialogModelFactory;

    sal_Int32 XMLNS_DIALOGS_UID, XMLNS_SCRIPT_UID;

public:
    inline bool isEventElement( sal_Int32 nUid, OUString const & rLocalName )
    {
        return ((XMLNS_SCRIPT_UID == nUid &&
                 (rLocalName.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM("event") ) ||
                  rLocalName.equalsAsciiL(
                      RTL_CONSTASCII_STRINGPARAM("listener-event") ))) ||
                (XMLNS_DIALOGS_UID == nUid &&
                 rLocalName.equalsAsciiL(
                     RTL_CONSTASCII_STRINGPARAM("event") )));
    }

    void addStyle(
        OUString const & rStyleId,
        Reference< xml::input::XElement > const & xStyle )
        SAL_THROW( () );
    Reference< xml::input::XElement > getStyle(
        OUString const & rStyleId ) const
        SAL_THROW( () );

    inline Reference< XComponentContext > const & getComponentContext() SAL_THROW( () )
        { return _xContext; }
    Reference< util::XNumberFormatsSupplier > const & getNumberFormatsSupplier();

    inline DialogImport(
        Reference< XComponentContext > const & xContext,
        Reference< container::XNameContainer > const & xDialogModel )
        SAL_THROW( () )
        : _xContext( xContext )
        , _xDialogModel( xDialogModel )
        , _xDialogModelFactory( xDialogModel, UNO_QUERY_THROW )
        { OSL_ASSERT( _xDialogModel.is() && _xDialogModelFactory.is() &&
                      _xContext.is() ); }
    virtual ~DialogImport()
        SAL_THROW( () );

    // XRoot
    virtual void SAL_CALL startDocument(
        Reference< container::XNameAccess > const & xUidMapping )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endDocument()
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(
        OUString const & rTarget, OUString const & rData )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL setDocumentLocator(
        Reference< xml::sax::XLocator > const & xLocator )
        throw (xml::sax::SAXException, RuntimeException);
    virtual Reference< xml::input::XElement > SAL_CALL startRootElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//==================================================================================================
class ElementBase
    : public ::cppu::WeakImplHelper1< xml::input::XElement >
{
protected:
    DialogImport * _pImport;
    ElementBase * _pParent;

    sal_Int32 _nUid;
    OUString _aLocalName;
    Reference< xml::input::XAttributes > _xAttributes;

public:
    ElementBase(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () );
    virtual ~ElementBase()
        SAL_THROW( () );

    // XElement
    virtual Reference< xml::input::XElement > SAL_CALL getParent()
        throw (RuntimeException);
    virtual OUString SAL_CALL getLocalName()
        throw (RuntimeException);
    virtual sal_Int32 SAL_CALL getUid()
        throw (RuntimeException);
    virtual Reference< xml::input::XAttributes > SAL_CALL getAttributes()
        throw (RuntimeException);
    virtual void SAL_CALL ignorableWhitespace(
        OUString const & rWhitespaces )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL characters( OUString const & rChars )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL processingInstruction(
        OUString const & Target, OUString const & Data )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
};

//==================================================================================================
class StylesElement
    : public ElementBase
{
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);

    inline StylesElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ElementBase( pImport->XMLNS_DIALOGS_UID,
                       rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class StyleElement
    : public ElementBase
{
    sal_Int32 _backgroundColor;
    sal_Int32 _textColor;
    sal_Int32 _textLineColor;
    sal_Int16 _border;
    awt::FontDescriptor _descr;
    sal_Int16 _fontRelief;
    sal_Int16 _fontEmphasisMark;
    sal_Int32 _fillColor;

    short _inited, _hasValue;

    void setFontProperties( Reference< beans::XPropertySet > const & xProps );

public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    bool importTextColorStyle(
        Reference< beans::XPropertySet > const & xProps );
    bool importTextLineColorStyle(
        Reference< beans::XPropertySet > const & xProps );
    bool importFillColorStyle(
        Reference< beans::XPropertySet > const & xProps );
    bool importBackgroundColorStyle(
        Reference< beans::XPropertySet > const & xProps );
    bool importFontStyle(
        Reference< beans::XPropertySet > const & xProps );
    bool importBorderStyle(
        Reference< beans::XPropertySet > const & xProps );

    inline StyleElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ElementBase( pImport->XMLNS_DIALOGS_UID,
                       rLocalName, xAttributes, pParent, pImport )
        , _fontRelief( awt::FontRelief::NONE )
        , _fontEmphasisMark( awt::FontEmphasisMark::NONE )
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

    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);

    inline MenuPopupElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ElementBase( pImport->XMLNS_DIALOGS_UID,
                       rLocalName, xAttributes, pParent, pImport )
        {}
};

//==================================================================================================
class ControlElement
    : public ElementBase
{
    friend class EventElement;

protected:
    sal_Int32 _nBasePosX, _nBasePosY;

    vector< Reference< xml::input::XElement > > _events;

    OUString getControlId(
        Reference< xml::input::XAttributes > const & xAttributes );
    Reference< xml::input::XElement > getStyle(
        Reference< xml::input::XAttributes > const & xAttributes );
public:
    vector< Reference< xml::input::XElement > > * getEvents() SAL_THROW( () )
        { return &_events; }

    ControlElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () );
};

//==============================================================================
class ImportContext
{
protected:
    DialogImport * _pImport;
    Reference< beans::XPropertySet > _xControlModel;
    OUString _aId;

public:
    inline ImportContext(
        DialogImport * pImport,
        Reference< beans::XPropertySet > const & xControlModel_,
        OUString const & id )
        : _pImport( pImport ),
          _xControlModel( xControlModel_ ),
          _aId( id )
        { OSL_ASSERT( _xControlModel.is() ); }

    inline Reference< beans::XPropertySet > getControlModel()
        { return _xControlModel; }

    void importDefaults(
        sal_Int32 nBaseX, sal_Int32 nBaseY,
        Reference< xml::input::XAttributes > const & xAttributes,
        bool supportPrintable = true );
    void importEvents(
        vector< Reference< xml::input::XElement > > const & rEvents );

    bool importStringProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importDoubleProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importBooleanProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importShortProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importLongProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importLongProperty(
        sal_Int32 nOffset,
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importAlignProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importImageAlignProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importDateFormatProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importTimeFormatProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importOrientationProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
    bool importButtonTypeProperty(
        OUString const & rPropName, OUString const & rAttrName,
        Reference< xml::input::XAttributes > const & xAttributes );
};

//==============================================================================
class ControlImportContext : public ImportContext
{
public:
    inline ControlImportContext(
        DialogImport * pImport,
        OUString const & rId, OUString const & rControlName )
        : ImportContext(
            pImport,
            Reference< beans::XPropertySet >(
                pImport->_xDialogModelFactory->createInstance( rControlName ),
                UNO_QUERY_THROW ), rId )
        {}
    inline ~ControlImportContext()
    {
        _pImport->_xDialogModel->insertByName(
            _aId, makeAny(
                Reference< awt::XControlModel >::query( _xControlModel ) ) );
    }
};

//==================================================================================================
class WindowElement
    : public ControlElement
{
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline WindowElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ElementBase( nUid, rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class BulletinBoardElement
    : public ControlElement
{
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);

    inline BulletinBoardElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () );
};
//==================================================================================================
class ButtonElement
    : public ControlElement
{
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline ButtonElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline CheckBoxElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class ComboBoxElement
    : public ControlElement
{
    Reference< xml::input::XElement > _popup;
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline ComboBoxElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class MenuListElement
    : public ControlElement
{
    Reference< xml::input::XElement > _popup;
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline MenuListElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);

    inline RadioElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class RadioGroupElement
    : public ControlElement
{
    vector< Reference< xml::input::XElement > > _radios;
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline RadioGroupElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    vector< Reference< xml::input::XElement > > _radios;
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline TitledBoxElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline TextElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline TextFieldElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline ImageControlElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline FileControlElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline CurrencyFieldElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline DateFieldElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline NumericFieldElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline TimeFieldElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
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
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline PatternFieldElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class FormattedFieldElement
    : public ControlElement
{
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline FormattedFieldElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class FixedLineElement
    : public ControlElement
{
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline FixedLineElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class ScrollBarElement
    : public ControlElement
{
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline ScrollBarElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};
//==================================================================================================
class ProgressBarElement
    : public ControlElement
{
public:
    virtual Reference< xml::input::XElement > SAL_CALL startChildElement(
        sal_Int32 nUid, OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes )
        throw (xml::sax::SAXException, RuntimeException);
    virtual void SAL_CALL endElement()
        throw (xml::sax::SAXException, RuntimeException);

    inline ProgressBarElement(
        OUString const & rLocalName,
        Reference< xml::input::XAttributes > const & xAttributes,
        ElementBase * pParent, DialogImport * pImport )
        SAL_THROW( () )
        : ControlElement( rLocalName, xAttributes, pParent, pImport )
        {}
};

}
