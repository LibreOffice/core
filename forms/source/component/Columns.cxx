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

#include <string.h>

#include "Columns.hxx"
#include "property.hrc"
#include "property.hxx"
#include "componenttools.hxx"
#include "ids.hxx"
#include "findpos.hxx"
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/text/XText.hpp>
#include <comphelper/sequence.hxx>
#include <comphelper/property.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/types.hxx>
#include <comphelper/servicehelper.hxx>
#include "services.hxx"
#include "frm_resource.hrc"
#include <tools/debug.hxx>

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::form::binding;

const sal_uInt16 WIDTH              = 0x0001;
const sal_uInt16 ALIGN              = 0x0002;
const sal_uInt16 OLD_HIDDEN         = 0x0004;
const sal_uInt16 COMPATIBLE_HIDDEN  = 0x0008;

//------------------------------------------------------------------------------
const StringSequence& getColumnTypes()
{
    static StringSequence aColumnTypes(10);
    if (aColumnTypes.getConstArray()[0].isEmpty())
    {
        ::rtl::OUString* pNames = aColumnTypes.getArray();
        pNames[TYPE_CHECKBOX]       = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CheckBox" ) );
        pNames[TYPE_COMBOBOX]       = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ComboBox" ) );
        pNames[TYPE_CURRENCYFIELD]  = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CurrencyField" ) );
        pNames[TYPE_DATEFIELD]      = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DateField" ) );
        pNames[TYPE_FORMATTEDFIELD] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FormattedField" ) );
        pNames[TYPE_LISTBOX]        = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ListBox" ) );
        pNames[TYPE_NUMERICFIELD]   = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "NumericField" ) );
        pNames[TYPE_PATTERNFIELD]   = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PatternField" ) );
        pNames[TYPE_TEXTFIELD]      = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TextField" ) );
        pNames[TYPE_TIMEFIELD]      = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "TimeField" ) );
    }
    return aColumnTypes;
}

//------------------------------------------------------------------------------
sal_Int32 getColumnTypeByModelName(const ::rtl::OUString& aModelName)
{
    const ::rtl::OUString aModelPrefix (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.form.component.") );
    const ::rtl::OUString aCompatibleModelPrefix (RTL_CONSTASCII_USTRINGPARAM("stardiv.one.form.component.") );

    sal_Int32 nTypeId = -1;
    if (aModelName == FRM_COMPONENT_EDIT)
        nTypeId = TYPE_TEXTFIELD;
    else
    {
        sal_Int32 nPrefixPos = aModelName.indexOf(aModelPrefix);
#ifdef DBG_UTIL
        sal_Int32 nCompatiblePrefixPos = aModelName.indexOf(aCompatibleModelPrefix);
#endif
        DBG_ASSERT( (nPrefixPos != -1) ||   (nCompatiblePrefixPos != -1),
                "::getColumnTypeByModelName() : wrong servivce !");

        ::rtl::OUString aColumnType = (nPrefixPos != -1)
            ? aModelName.copy(aModelPrefix.getLength())
            : aModelName.copy(aCompatibleModelPrefix.getLength());

        const StringSequence& rColumnTypes = getColumnTypes();
        nTypeId = ::detail::findPos(aColumnType, rColumnTypes);
    }
    return nTypeId;
}

/*************************************************************************/

namespace
{
    class theOGridColumnImplementationId : public rtl::Static< UnoTunnelIdInit, theOGridColumnImplementationId > {};
}

const Sequence<sal_Int8>& OGridColumn::getUnoTunnelImplementationId()
{
    return theOGridColumnImplementationId::get().getSeq();
}

//------------------------------------------------------------------
sal_Int64 SAL_CALL OGridColumn::getSomething( const Sequence<sal_Int8>& _rIdentifier) throw(RuntimeException)
{
    sal_Int64 nReturn(0);

    if  (   (_rIdentifier.getLength() == 16)
        &&  (0 == memcmp( getUnoTunnelImplementationId().getConstArray(), _rIdentifier.getConstArray(), 16 ))
        )
    {
        nReturn = reinterpret_cast<sal_Int64>(this);
    }
    else
    {
        Reference< XUnoTunnel > xAggTunnel;
        if ( query_aggregation( m_xAggregate, xAggTunnel ) )
            return xAggTunnel->getSomething( _rIdentifier );
    }
    return nReturn;
}

//------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL OGridColumn::getImplementationId() throw(RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------
Sequence<Type> SAL_CALL OGridColumn::getTypes() throw(RuntimeException)
{
    TypeBag aTypes( OGridColumn_BASE::getTypes() );
    // erase the types which we do not support
    aTypes.removeType( XFormComponent::static_type() );
    aTypes.removeType( XServiceInfo::static_type() );
    aTypes.removeType( XBindableValue::static_type() );
    aTypes.removeType( XPropertyContainer::static_type() );

    // but re-add their base class(es)
    aTypes.addType( XChild::static_type() );

    Reference< XTypeProvider > xProv;
    if ( query_aggregation( m_xAggregate, xProv ))
        aTypes.addTypes( xProv->getTypes() );

    aTypes.removeType( XTextRange::static_type() );
    aTypes.removeType( XSimpleText::static_type() );
    aTypes.removeType( XText::static_type() );

    return aTypes.getTypes();
}

//------------------------------------------------------------------
Any SAL_CALL OGridColumn::queryAggregation( const Type& _rType ) throw (RuntimeException)
{
    Any aReturn;
    // some functionality at our aggregate cannot be reasonably fullfilled here.
    if  (   _rType.equals(::getCppuType(static_cast< Reference< XFormComponent >* >(NULL)))
        ||  _rType.equals(::getCppuType(static_cast< Reference< XServiceInfo >* >(NULL)))
        ||  _rType.equals(::getCppuType(static_cast< Reference< XBindableValue >* >(NULL)))
        ||  _rType.equals(::getCppuType(static_cast< Reference< XPropertyContainer >* >(NULL)))
        ||  comphelper::isAssignableFrom(::getCppuType(static_cast< Reference< XTextRange >* >(NULL)),_rType)
        )
        return aReturn;

    aReturn = OGridColumn_BASE::queryAggregation(_rType);
    if (!aReturn.hasValue())
    {
        aReturn = OPropertySetAggregationHelper::queryInterface(_rType);
        if (!aReturn.hasValue() && m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    return aReturn;
}

DBG_NAME(OGridColumn);
//------------------------------------------------------------------------------
OGridColumn::OGridColumn( const comphelper::ComponentContext& _rContext, const ::rtl::OUString& _sModelName )
    :OGridColumn_BASE(m_aMutex)
    ,OPropertySetAggregationHelper(OGridColumn_BASE::rBHelper)
    ,m_aHidden( makeAny( sal_False ) )
    ,m_aContext( _rContext )
    ,m_aModelName(_sModelName)
{
    DBG_CTOR(OGridColumn,NULL);

    // Create the UnoControlModel
    if ( !m_aModelName.isEmpty() ) // is there a to-be-aggregated model?
    {
        increment( m_refCount );

        {
            m_xAggregate.set( m_aContext.createComponent( m_aModelName ), UNO_QUERY );
            setAggregation( m_xAggregate );
        }

        if ( m_xAggregate.is() )
        {   // don't omit those brackets - they ensure that the following temporary is properly deleted
            m_xAggregate->setDelegator( static_cast< ::cppu::OWeakObject* >( this ) );
        }

        // Set refcount back to zero
        decrement( m_refCount );
    }
}

//------------------------------------------------------------------------------
OGridColumn::OGridColumn( const OGridColumn* _pOriginal )
    :OGridColumn_BASE( m_aMutex )
    ,OPropertySetAggregationHelper( OGridColumn_BASE::rBHelper )
    ,m_aContext( _pOriginal->m_aContext )
{
    DBG_CTOR(OGridColumn,NULL);

    m_aWidth = _pOriginal->m_aWidth;
    m_aAlign = _pOriginal->m_aAlign;
    m_aHidden = _pOriginal->m_aHidden;
    m_aModelName = _pOriginal->m_aModelName;
    m_aLabel = _pOriginal->m_aLabel;

    increment( m_refCount );
    {
        {
            m_xAggregate = createAggregateClone( _pOriginal );
            setAggregation( m_xAggregate );
        }

        if ( m_xAggregate.is() )
        {   // don't omit this brackets - they ensure that the following temporary is properly deleted
            m_xAggregate->setDelegator( static_cast< ::cppu::OWeakObject* >( this ) );
        }
    }
    decrement( m_refCount );
}

//------------------------------------------------------------------------------
OGridColumn::~OGridColumn()
{
    if (!OGridColumn_BASE::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    // Free the aggregate
    if (m_xAggregate.is())
    {
        InterfaceRef  xIface;
        m_xAggregate->setDelegator(xIface);
    }

    DBG_DTOR(OGridColumn,NULL);
}

// XEventListener
//------------------------------------------------------------------------------
void SAL_CALL OGridColumn::disposing(const EventObject& _rSource) throw(RuntimeException)
{
    OPropertySetAggregationHelper::disposing(_rSource);

    Reference<XEventListener>  xEvtLstner;
    if (query_aggregation(m_xAggregate, xEvtLstner))
        xEvtLstner->disposing(_rSource);
}

// OGridColumn_BASE
//-----------------------------------------------------------------------------
void OGridColumn::disposing()
{
    OGridColumn_BASE::disposing();
    OPropertySetAggregationHelper::disposing();

    Reference<XComponent>  xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();
}

//------------------------------------------------------------------------------
void OGridColumn::clearAggregateProperties( Sequence< Property >& _rProps, sal_Bool bAllowDropDown )
{
    // some properties are not to be exposed to the outer world
    ::std::set< ::rtl::OUString > aForbiddenProperties;
    aForbiddenProperties.insert( PROPERTY_ALIGN );
    aForbiddenProperties.insert( PROPERTY_AUTOCOMPLETE );
    aForbiddenProperties.insert( PROPERTY_BACKGROUNDCOLOR );
    aForbiddenProperties.insert( PROPERTY_BORDER );
    aForbiddenProperties.insert( PROPERTY_BORDERCOLOR );
    aForbiddenProperties.insert( PROPERTY_ECHO_CHAR );
    aForbiddenProperties.insert( PROPERTY_FILLCOLOR );
    aForbiddenProperties.insert( PROPERTY_FONT );
    aForbiddenProperties.insert( PROPERTY_FONT_NAME );
    aForbiddenProperties.insert( PROPERTY_FONT_STYLENAME );
    aForbiddenProperties.insert( PROPERTY_FONT_FAMILY );
    aForbiddenProperties.insert( PROPERTY_FONT_CHARSET );
    aForbiddenProperties.insert( PROPERTY_FONT_HEIGHT );
    aForbiddenProperties.insert( PROPERTY_FONT_WEIGHT );
    aForbiddenProperties.insert( PROPERTY_FONT_SLANT );
    aForbiddenProperties.insert( PROPERTY_FONT_UNDERLINE );
    aForbiddenProperties.insert( PROPERTY_FONT_STRIKEOUT );
    aForbiddenProperties.insert( PROPERTY_FONT_WORDLINEMODE );
    aForbiddenProperties.insert( PROPERTY_TEXTLINECOLOR );
    aForbiddenProperties.insert( PROPERTY_FONTEMPHASISMARK );
    aForbiddenProperties.insert( PROPERTY_FONTRELIEF );
    aForbiddenProperties.insert( PROPERTY_HARDLINEBREAKS );
    aForbiddenProperties.insert( PROPERTY_HSCROLL );
    aForbiddenProperties.insert( PROPERTY_LABEL );
    aForbiddenProperties.insert( PROPERTY_LINECOLOR );
    aForbiddenProperties.insert( PROPERTY_MULTISELECTION );
    aForbiddenProperties.insert( PROPERTY_PRINTABLE );
    aForbiddenProperties.insert( PROPERTY_TABINDEX );
    aForbiddenProperties.insert( PROPERTY_TABSTOP );
    aForbiddenProperties.insert( PROPERTY_TEXTCOLOR );
    aForbiddenProperties.insert( PROPERTY_VSCROLL );
    aForbiddenProperties.insert( PROPERTY_CONTROLLABEL );
    aForbiddenProperties.insert( PROPERTY_RICH_TEXT );
    aForbiddenProperties.insert( PROPERTY_VERTICAL_ALIGN );
    aForbiddenProperties.insert( PROPERTY_IMAGE_URL );
    aForbiddenProperties.insert( PROPERTY_IMAGE_POSITION );
    aForbiddenProperties.insert( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableVisible" ) ) );
    if ( !bAllowDropDown )
        aForbiddenProperties.insert( PROPERTY_DROPDOWN );

    Sequence< Property > aNewProps( _rProps.getLength() );
    Property* pNewProps = aNewProps.getArray();

    const Property* pProps = _rProps.getConstArray();
    const Property* pPropsEnd = pProps + _rProps.getLength();
    for ( ; pProps != pPropsEnd; ++pProps )
    {
        if ( aForbiddenProperties.find( pProps->Name ) == aForbiddenProperties.end() )
            *pNewProps++ = *pProps;
    }

    aNewProps.realloc( pNewProps - aNewProps.getArray() );
    _rProps = aNewProps;
}

//------------------------------------------------------------------------------
void OGridColumn::setOwnProperties(Sequence<Property>& aDescriptor)
{
    aDescriptor.realloc(5);
    Property* pProperties = aDescriptor.getArray();
    DECL_PROP1(LABEL,               ::rtl::OUString,    BOUND);
    DECL_PROP3(WIDTH,               sal_Int32,          BOUND, MAYBEVOID, MAYBEDEFAULT);
    DECL_PROP3(ALIGN,               sal_Int16,          BOUND, MAYBEVOID, MAYBEDEFAULT);
    DECL_BOOL_PROP2(HIDDEN,                             BOUND, MAYBEDEFAULT);
    DECL_PROP1(COLUMNSERVICENAME,   ::rtl::OUString,    READONLY);
}

// Reference<XPropertySet>
//------------------------------------------------------------------------------
void OGridColumn::getFastPropertyValue(Any& rValue, sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_COLUMNSERVICENAME:
            rValue <<= m_aModelName;
            break;
        case PROPERTY_ID_LABEL:
            rValue <<= m_aLabel;
            break;
        case PROPERTY_ID_WIDTH:
            rValue = m_aWidth;
            break;
        case PROPERTY_ID_ALIGN:
            rValue = m_aAlign;
            break;
        case PROPERTY_ID_HIDDEN:
            rValue = m_aHidden;
            break;
        default:
            OPropertySetAggregationHelper::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
sal_Bool OGridColumn::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue,
                                            sal_Int32 nHandle, const Any& rValue )throw( IllegalArgumentException )
{
    sal_Bool bModified(sal_False);
    switch (nHandle)
    {
        case PROPERTY_ID_LABEL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aLabel);
            break;
        case PROPERTY_ID_WIDTH:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth, ::getCppuType((const sal_Int32*)NULL));
            break;
        case PROPERTY_ID_ALIGN:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_aAlign, ::getCppuType( (const sal_Int32*)NULL ) );
            // strange enough, css.awt.TextAlign is a 32-bit integer, while the Align property (both here for grid controls
            // and for ordinary toolkit controls) is a 16-bit integer. So, allow for 32 bit, but normalize it to 16 bit
            if ( bModified )
            {
                sal_Int32 nAlign( 0 );
                if ( rConvertedValue >>= nAlign )
                    rConvertedValue <<= (sal_Int16)nAlign;
            }
            break;
        case PROPERTY_ID_HIDDEN:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, getBOOL(m_aHidden));
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OGridColumn::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw (::com::sun::star::uno::Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_LABEL:
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "invalid type" );
            rValue >>= m_aLabel;
            break;
        case PROPERTY_ID_WIDTH:
            m_aWidth = rValue;
            break;
        case PROPERTY_ID_ALIGN:
            m_aAlign = rValue;
            break;
        case PROPERTY_ID_HIDDEN:
            m_aHidden = rValue;
            break;
    }
}


// XPropertyState
//------------------------------------------------------------------------------
Any OGridColumn::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_ALIGN:
            return Any();
        case PROPERTY_ID_HIDDEN:
            return makeAny((sal_Bool)sal_False);
        default:
            return OPropertySetAggregationHelper::getPropertyDefaultByHandle(nHandle);
    }
}

// XCloneable
//------------------------------------------------------------------------------
Reference< XCloneable > SAL_CALL OGridColumn::createClone(  ) throw (RuntimeException)
{
    OGridColumn* pNewColumn = createCloneColumn();
    return pNewColumn;
}

// XPersistObject
//------------------------------------------------------------------------------
void SAL_CALL OGridColumn::write(const Reference<XObjectOutputStream>& _rxOutStream)
{
    // 1. Write the UnoControl
    Reference<XMarkableStream>  xMark(_rxOutStream, UNO_QUERY);
    sal_Int32 nMark = xMark->createMark();

    sal_Int32 nLen = 0;
    _rxOutStream->writeLong(nLen);

    Reference<XPersistObject>  xPersist;
    if (query_aggregation(m_xAggregate, xPersist))
        xPersist->write(_rxOutStream);

    // Calculate the length
    nLen = xMark->offsetToMark(nMark) - 4;
    xMark->jumpToMark(nMark);
    _rxOutStream->writeLong(nLen);
    xMark->jumpToFurthest();
    xMark->deleteMark(nMark);

    // 2. Write a version number
    _rxOutStream->writeShort(0x0002);

    sal_uInt16 nAnyMask = 0;
    if (m_aWidth.getValueType().getTypeClass() == TypeClass_LONG)
        nAnyMask |= WIDTH;

    if (m_aAlign.getValueTypeClass() == TypeClass_SHORT)
        nAnyMask |= ALIGN;

    nAnyMask |= COMPATIBLE_HIDDEN;

    _rxOutStream->writeShort(nAnyMask);

    if (nAnyMask & WIDTH)
        _rxOutStream->writeLong(getINT32(m_aWidth));
    if (nAnyMask & ALIGN)
        _rxOutStream->writeShort(getINT16(m_aAlign));

    // Name
    _rxOutStream << m_aLabel;

    // the new place for the hidden flag : after m_aLabel, so older office version read the correct label, too
    if (nAnyMask & COMPATIBLE_HIDDEN)
        _rxOutStream->writeBoolean(getBOOL(m_aHidden));
}

//------------------------------------------------------------------------------
void SAL_CALL OGridColumn::read(const Reference<XObjectInputStream>& _rxInStream)
{
    // 1. Read the UnoControl
    sal_Int32 nLen = _rxInStream->readLong();
    if (nLen)
    {
        Reference<XMarkableStream>  xMark(_rxInStream, UNO_QUERY);
        sal_Int32 nMark = xMark->createMark();
        Reference<XPersistObject>  xPersist;
        if (query_aggregation(m_xAggregate, xPersist))
            xPersist->read(_rxInStream);

        xMark->jumpToMark(nMark);
        _rxInStream->skipBytes(nLen);
        xMark->deleteMark(nMark);
    }

    // 2. Write a version number
    sal_uInt16 nVersion = _rxInStream->readShort(); (void)nVersion;
    sal_uInt16 nAnyMask = _rxInStream->readShort();

    if (nAnyMask & WIDTH)
    {
        sal_Int32 nValue = _rxInStream->readLong();
        m_aWidth <<= (sal_Int32)nValue;
    }

    if (nAnyMask & ALIGN)
    {
        sal_Int16 nValue = _rxInStream->readShort();
        m_aAlign <<= nValue;
    }
    if (nAnyMask & OLD_HIDDEN)
    {
        sal_Bool bValue = _rxInStream->readBoolean();
        m_aHidden <<= (sal_Bool)bValue;
    }

    // Name
    _rxInStream >> m_aLabel;

    if (nAnyMask & COMPATIBLE_HIDDEN)
    {
        sal_Bool bValue = _rxInStream->readBoolean();
        m_aHidden <<= (sal_Bool)bValue;
    }
}

//------------------------------------------------------------------------------
IMPL_COLUMN(TextFieldColumn,        FRM_SUN_COMPONENT_TEXTFIELD,        sal_False);
IMPL_COLUMN(PatternFieldColumn,     FRM_SUN_COMPONENT_PATTERNFIELD,     sal_False);
IMPL_COLUMN(DateFieldColumn,        FRM_SUN_COMPONENT_DATEFIELD,        sal_True);
IMPL_COLUMN(TimeFieldColumn,        FRM_SUN_COMPONENT_TIMEFIELD,        sal_False);
IMPL_COLUMN(NumericFieldColumn,     FRM_SUN_COMPONENT_NUMERICFIELD,     sal_False);
IMPL_COLUMN(CurrencyFieldColumn,    FRM_SUN_COMPONENT_CURRENCYFIELD,    sal_False);
IMPL_COLUMN(CheckBoxColumn,         FRM_SUN_COMPONENT_CHECKBOX,         sal_False);
IMPL_COLUMN(ComboBoxColumn,         FRM_SUN_COMPONENT_COMBOBOX,         sal_False);
IMPL_COLUMN(ListBoxColumn,          FRM_SUN_COMPONENT_LISTBOX,          sal_False);
IMPL_COLUMN(FormattedFieldColumn,   FRM_SUN_COMPONENT_FORMATTEDFIELD,   sal_False);

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
