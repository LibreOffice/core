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

#include "Columns.hxx"
#include <property.hxx>
#include <componenttools.hxx>
#include "findpos.hxx"
#include <com/sun/star/io/XPersistObject.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/text/XText.hpp>
#include <comphelper/property.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/types.hxx>
#include <services.hxx>
#include <tools/debug.hxx>
#include <o3tl/sorted_vector.hxx>


namespace frm
{

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


const css::uno::Sequence<OUString>& getColumnTypes()
{
    static css::uno::Sequence<OUString> aColumnTypes = [&]()
    {
        css::uno::Sequence<OUString> tmp(10);
        OUString* pNames = tmp.getArray();
        pNames[TYPE_CHECKBOX]       = "CheckBox";
        pNames[TYPE_COMBOBOX]       = "ComboBox";
        pNames[TYPE_CURRENCYFIELD]  = "CurrencyField";
        pNames[TYPE_DATEFIELD]      = "DateField";
        pNames[TYPE_FORMATTEDFIELD] = "FormattedField";
        pNames[TYPE_LISTBOX]        = "ListBox";
        pNames[TYPE_NUMERICFIELD]   = "NumericField";
        pNames[TYPE_PATTERNFIELD]   = "PatternField";
        pNames[TYPE_TEXTFIELD]      = "TextField";
        pNames[TYPE_TIMEFIELD]      = "TimeField";
        return tmp;
    }();
    return aColumnTypes;
}


sal_Int32 getColumnTypeByModelName(const OUString& aModelName)
{
    static const OUStringLiteral aModelPrefix (u"com.sun.star.form.component.");
    static const OUStringLiteral aCompatibleModelPrefix (u"stardiv.one.form.component.");

    sal_Int32 nTypeId = -1;
    if (aModelName == FRM_COMPONENT_EDIT)
        nTypeId = TYPE_TEXTFIELD;
    else
    {
        sal_Int32 nPrefixPos = aModelName.indexOf(aModelPrefix);
#ifdef DBG_UTIL
        sal_Int32 nCompatiblePrefixPos = aModelName.indexOf(aCompatibleModelPrefix);
        DBG_ASSERT( (nPrefixPos != -1) ||   (nCompatiblePrefixPos != -1),
                "::getColumnTypeByModelName() : wrong service!");
#endif

        OUString aColumnType = (nPrefixPos != -1)
            ? aModelName.copy(aModelPrefix.getLength())
            : aModelName.copy(aCompatibleModelPrefix.getLength());

        const css::uno::Sequence<OUString>& rColumnTypes = getColumnTypes();
        nTypeId = ::detail::findPos(aColumnType, rColumnTypes);
    }
    return nTypeId;
}

const Sequence<sal_Int8>& OGridColumn::getUnoTunnelId()
{
    static const UnoTunnelIdInit theOGridColumnImplementationId;
    return theOGridColumnImplementationId.getSeq();
}


sal_Int64 SAL_CALL OGridColumn::getSomething( const Sequence<sal_Int8>& _rIdentifier)
{
    sal_Int64 nReturn(0);

    if ( isUnoTunnelId<OGridColumn>(_rIdentifier) )
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


Sequence<sal_Int8> SAL_CALL OGridColumn::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}


Sequence<Type> SAL_CALL OGridColumn::getTypes()
{
    TypeBag aTypes( OGridColumn_BASE::getTypes() );
    // erase the types which we do not support
    aTypes.removeType( cppu::UnoType<XFormComponent>::get() );
    aTypes.removeType( cppu::UnoType<XServiceInfo>::get() );
    aTypes.removeType( cppu::UnoType<XBindableValue>::get() );
    aTypes.removeType( cppu::UnoType<XPropertyContainer>::get() );

    // but re-add their base class(es)
    aTypes.addType( cppu::UnoType<XChild>::get() );

    Reference< XTypeProvider > xProv;
    if ( query_aggregation( m_xAggregate, xProv ))
        aTypes.addTypes( xProv->getTypes() );

    aTypes.removeType( cppu::UnoType<XTextRange>::get() );
    aTypes.removeType( cppu::UnoType<XSimpleText>::get() );
    aTypes.removeType( cppu::UnoType<XText>::get() );

    return aTypes.getTypes();
}


Any SAL_CALL OGridColumn::queryAggregation( const Type& _rType )
{
    Any aReturn;
    // some functionality at our aggregate cannot be reasonably fulfilled here.
    if  (   _rType.equals(cppu::UnoType<XFormComponent>::get())
        ||  _rType.equals(cppu::UnoType<XServiceInfo>::get())
        ||  _rType.equals(cppu::UnoType<XBindableValue>::get())
        ||  _rType.equals(cppu::UnoType<XPropertyContainer>::get())
        ||  comphelper::isAssignableFrom(cppu::UnoType<XTextRange>::get(),_rType)
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


OGridColumn::OGridColumn( const Reference<XComponentContext>& _rContext, const OUString& _sModelName )
    :OGridColumn_BASE(m_aMutex)
    ,OPropertySetAggregationHelper(OGridColumn_BASE::rBHelper)
    ,m_aHidden( makeAny( false ) )
    ,m_aModelName(_sModelName)
{

    // Create the UnoControlModel
    if ( m_aModelName.isEmpty() ) // is there a to-be-aggregated model?
        return;

    osl_atomic_increment( &m_refCount );

    {
        m_xAggregate.set( _rContext->getServiceManager()->createInstanceWithContext( m_aModelName, _rContext ), UNO_QUERY );
        setAggregation( m_xAggregate );
    }

    if ( m_xAggregate.is() )
    {   // don't omit those brackets - they ensure that the following temporary is properly deleted
        m_xAggregate->setDelegator( static_cast< ::cppu::OWeakObject* >( this ) );
    }

    // Set refcount back to zero
    osl_atomic_decrement( &m_refCount );
}


OGridColumn::OGridColumn( const OGridColumn* _pOriginal )
    :OGridColumn_BASE( m_aMutex )
    ,OPropertySetAggregationHelper( OGridColumn_BASE::rBHelper )
{

    m_aWidth = _pOriginal->m_aWidth;
    m_aAlign = _pOriginal->m_aAlign;
    m_aHidden = _pOriginal->m_aHidden;
    m_aModelName = _pOriginal->m_aModelName;
    m_aLabel = _pOriginal->m_aLabel;

    osl_atomic_increment( &m_refCount );
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
    osl_atomic_decrement( &m_refCount );
}


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
        css::uno::Reference<css::uno::XInterface>  xIface;
        m_xAggregate->setDelegator(xIface);
    }

}

// XEventListener

void SAL_CALL OGridColumn::disposing(const EventObject& _rSource)
{
    OPropertySetAggregationHelper::disposing(_rSource);

    Reference<XEventListener>  xEvtLstner;
    if (query_aggregation(m_xAggregate, xEvtLstner))
        xEvtLstner->disposing(_rSource);
}

// OGridColumn_BASE

void OGridColumn::disposing()
{
    OGridColumn_BASE::disposing();
    OPropertySetAggregationHelper::disposing();

    Reference<XComponent>  xComp;
    if (query_aggregation(m_xAggregate, xComp))
        xComp->dispose();
}


void OGridColumn::clearAggregateProperties( Sequence< Property >& _rProps, bool bAllowDropDown )
{
    // some properties are not to be exposed to the outer world
    static const o3tl::sorted_vector< OUString > aForbiddenProperties {
      PROPERTY_ALIGN,
      PROPERTY_AUTOCOMPLETE,
      PROPERTY_BACKGROUNDCOLOR,
      PROPERTY_BORDER,
      PROPERTY_BORDERCOLOR,
      PROPERTY_ECHO_CHAR,
      PROPERTY_FILLCOLOR,
      PROPERTY_FONT,
      PROPERTY_FONT_NAME,
      PROPERTY_FONT_STYLENAME,
      PROPERTY_FONT_FAMILY,
      PROPERTY_FONT_CHARSET,
      PROPERTY_FONT_HEIGHT,
      PROPERTY_FONT_WEIGHT,
      PROPERTY_FONT_SLANT,
      PROPERTY_FONT_UNDERLINE,
      PROPERTY_FONT_STRIKEOUT,
      PROPERTY_FONT_WORDLINEMODE,
      PROPERTY_TEXTLINECOLOR,
      PROPERTY_FONTEMPHASISMARK,
      PROPERTY_FONTRELIEF,
      PROPERTY_HARDLINEBREAKS,
      PROPERTY_HSCROLL,
      PROPERTY_LABEL,
      PROPERTY_LINECOLOR,
      PROPERTY_MULTISELECTION,
      PROPERTY_PRINTABLE,
      PROPERTY_TABINDEX,
      PROPERTY_TABSTOP,
      PROPERTY_TEXTCOLOR,
      PROPERTY_VSCROLL,
      PROPERTY_CONTROLLABEL,
      PROPERTY_RICH_TEXT,
      PROPERTY_VERTICAL_ALIGN,
      PROPERTY_IMAGE_URL,
      PROPERTY_IMAGE_POSITION,
      PROPERTY_ENABLEVISIBLE
    };

    Sequence< Property > aNewProps( _rProps.getLength() );
    Property* pNewProps = aNewProps.getArray();

    const Property* pProps = _rProps.getConstArray();
    const Property* pPropsEnd = pProps + _rProps.getLength();
    for ( ; pProps != pPropsEnd; ++pProps )
    {
        if ( aForbiddenProperties.find( pProps->Name ) == aForbiddenProperties.end()
            && (bAllowDropDown || pProps->Name != PROPERTY_DROPDOWN))
            *pNewProps++ = *pProps;
    }

    aNewProps.realloc( pNewProps - aNewProps.getArray() );
    _rProps = aNewProps;
}


void OGridColumn::setOwnProperties(Sequence<Property>& aDescriptor)
{
    aDescriptor.realloc(5);
    Property* pProperties = aDescriptor.getArray();
    DECL_PROP1(LABEL,               OUString,    BOUND);
    DECL_PROP3(WIDTH,               sal_Int32,          BOUND, MAYBEVOID, MAYBEDEFAULT);
    DECL_PROP3(ALIGN,               sal_Int16,          BOUND, MAYBEVOID, MAYBEDEFAULT);
    DECL_BOOL_PROP2(HIDDEN,                             BOUND, MAYBEDEFAULT);
    DECL_PROP1(COLUMNSERVICENAME,   OUString,    READONLY);
}

// Reference<XPropertySet>

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


sal_Bool OGridColumn::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue,
                                            sal_Int32 nHandle, const Any& rValue )
{
    bool bModified(false);
    switch (nHandle)
    {
        case PROPERTY_ID_LABEL:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aLabel);
            break;
        case PROPERTY_ID_WIDTH:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, m_aWidth, cppu::UnoType<sal_Int32>::get());
            break;
        case PROPERTY_ID_ALIGN:
            bModified = tryPropertyValue( rConvertedValue, rOldValue, rValue, m_aAlign, cppu::UnoType<sal_Int32>::get());
            // strange enough, css.awt.TextAlign is a 32-bit integer, while the Align property (both here for grid controls
            // and for ordinary toolkit controls) is a 16-bit integer. So, allow for 32 bit, but normalize it to 16 bit
            if ( bModified )
            {
                sal_Int32 nAlign( 0 );
                if ( rConvertedValue >>= nAlign )
                    rConvertedValue <<= static_cast<sal_Int16>(nAlign);
            }
            break;
        case PROPERTY_ID_HIDDEN:
            bModified = tryPropertyValue(rConvertedValue, rOldValue, rValue, getBOOL(m_aHidden));
            break;
    }
    return bModified;
}


void OGridColumn::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
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

Any OGridColumn::getPropertyDefaultByHandle( sal_Int32 nHandle ) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_WIDTH:
        case PROPERTY_ID_ALIGN:
            return Any();
        case PROPERTY_ID_HIDDEN:
            return makeAny(false);
        default:
            return OPropertySetAggregationHelper::getPropertyDefaultByHandle(nHandle);
    }
}

// XCloneable

Reference< XCloneable > SAL_CALL OGridColumn::createClone(  )
{
    return createCloneColumn();
}

// XPersistObject

void OGridColumn::write(const Reference<XObjectOutputStream>& _rxOutStream)
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


void OGridColumn::read(const Reference<XObjectInputStream>& _rxInStream)
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
    _rxInStream->readShort(); // version;
    sal_uInt16 nAnyMask = _rxInStream->readShort();

    if (nAnyMask & WIDTH)
    {
        sal_Int32 nValue = _rxInStream->readLong();
        m_aWidth <<= nValue;
    }

    if (nAnyMask & ALIGN)
    {
        sal_Int16 nValue = _rxInStream->readShort();
        m_aAlign <<= nValue;
    }
    if (nAnyMask & OLD_HIDDEN)
    {
        bool bValue = _rxInStream->readBoolean();
        m_aHidden <<= bValue;
    }

    // Name
    _rxInStream >> m_aLabel;

    if (nAnyMask & COMPATIBLE_HIDDEN)
    {
        bool bValue = _rxInStream->readBoolean();
        m_aHidden <<= bValue;
    }
}


IMPL_COLUMN(TextFieldColumn,        FRM_SUN_COMPONENT_TEXTFIELD,        false);
IMPL_COLUMN(PatternFieldColumn,     FRM_SUN_COMPONENT_PATTERNFIELD,     false);
IMPL_COLUMN(DateFieldColumn,        FRM_SUN_COMPONENT_DATEFIELD,        true);
IMPL_COLUMN(TimeFieldColumn,        FRM_SUN_COMPONENT_TIMEFIELD,        false);
IMPL_COLUMN(NumericFieldColumn,     FRM_SUN_COMPONENT_NUMERICFIELD,     false);
IMPL_COLUMN(CurrencyFieldColumn,    FRM_SUN_COMPONENT_CURRENCYFIELD,    false);
IMPL_COLUMN(CheckBoxColumn,         FRM_SUN_COMPONENT_CHECKBOX,         false);
IMPL_COLUMN(ComboBoxColumn,         FRM_SUN_COMPONENT_COMBOBOX,         false);
IMPL_COLUMN(ListBoxColumn,          FRM_SUN_COMPONENT_LISTBOX,          false);
IMPL_COLUMN(FormattedFieldColumn,   FRM_SUN_COMPONENT_FORMATTEDFIELD,   false);


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
