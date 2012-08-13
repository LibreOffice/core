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


#include "ComboBox.hxx"
#include "property.hxx"
#include "property.hrc"
#include "services.hxx"

#include "frm_resource.hxx"
#include "frm_resource.hrc"
#include "BaseListBox.hxx"

#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include <comphelper/numbers.hxx>
#include <comphelper/basicio.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/sharedunocomponent.hxx>

#include <limits.h>

using namespace dbtools;

//.........................................................................
namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::form::binding;

//========================================================================
// class OComboBoxModel
//========================================================================
//------------------------------------------------------------------
InterfaceRef SAL_CALL OComboBoxModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return (*new OComboBoxModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OComboBoxModel::_getTypes()
{
    return ::comphelper::concatSequences(
        OBoundControlModel::_getTypes(),
        OEntryListHelper::getTypes(),
        OErrorBroadcaster::getTypes()
    );
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OComboBoxModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 8 );
    ::rtl::OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_COMBOBOX;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_COMBOBOX;
    *pStoreTo++ = BINDABLE_DATABASE_COMBO_BOX;

    return aSupported;
}

//------------------------------------------------------------------------------
Any SAL_CALL OComboBoxModel::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OBoundControlModel::queryAggregation( _rType );
    if ( !aReturn.hasValue() )
        aReturn = OEntryListHelper::queryInterface( _rType );
    if ( !aReturn.hasValue() )
        aReturn = OErrorBroadcaster::queryInterface( _rType );
    return aReturn;
}

//------------------------------------------------------------------
DBG_NAME( OComboBoxModel )
//------------------------------------------------------------------
OComboBoxModel::OComboBoxModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_COMBOBOX, FRM_SUN_CONTROL_COMBOBOX, sal_True, sal_True, sal_True )
                    // use the old control name for compytibility reasons
    ,OEntryListHelper( (OControlModel&)*this )
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
    ,m_aListRowSet( getContext() )
    ,m_eListSourceType(ListSourceType_TABLE)
    ,m_bEmptyIsNull(sal_True)
{
    DBG_CTOR( OComboBoxModel, NULL );

    m_nClassId = FormComponentType::COMBOBOX;
    initValueProperty( PROPERTY_TEXT, PROPERTY_ID_TEXT );
}

//------------------------------------------------------------------
OComboBoxModel::OComboBoxModel( const OComboBoxModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory )
    ,OEntryListHelper( *_pOriginal, (OControlModel&)*this )
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
    ,m_aListRowSet( getContext() )
    ,m_aListSource( _pOriginal->m_aListSource )
    ,m_aDefaultText( _pOriginal->m_aDefaultText )
    ,m_eListSourceType( _pOriginal->m_eListSourceType )
    ,m_bEmptyIsNull( _pOriginal->m_bEmptyIsNull )
{
    DBG_CTOR( OComboBoxModel, NULL );
}

//------------------------------------------------------------------
OComboBoxModel::~OComboBoxModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR( OComboBoxModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OComboBoxModel )

//------------------------------------------------------------------------------
void OComboBoxModel::disposing()
{
    OBoundControlModel::disposing();
    OEntryListHelper::disposing();
    OErrorBroadcaster::disposing();
    m_xFormatter = NULL;
}

//------------------------------------------------------------------------------
void OComboBoxModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE:
            _rValue <<= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE:
            _rValue <<= m_aListSource;
            break;

        case PROPERTY_ID_EMPTY_IS_NULL:
            _rValue <<= m_bEmptyIsNull;
            break;

        case PROPERTY_ID_DEFAULT_TEXT:
            _rValue <<= m_aDefaultText;
            break;

        case PROPERTY_ID_STRINGITEMLIST:
            _rValue <<= getStringItemList();
            break;

        default:
            OBoundControlModel::getFastPropertyValue(_rValue, _nHandle);
    }
}

//------------------------------------------------------------------------------
void OComboBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
                        throw (Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(static_cast<ListSourceType*>(0))),
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_STRING,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aListSource;
            // The ListSource has changed -> reload
            if (ListSourceType_VALUELIST != m_eListSourceType)
            {
                if ( m_xCursor.is() && !hasField() && !hasExternalListSource() )
                    // combo box is already connected to a database, and no external list source
                    // data source changed -> refresh
                    loadData( false );
            }
            break;

        case PROPERTY_ID_EMPTY_IS_NULL :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_BOOLEAN,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_bEmptyIsNull;
            break;

        case PROPERTY_ID_DEFAULT_TEXT :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_STRING,
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aDefaultText;
            resetNoBroadcast();
            break;

        case PROPERTY_ID_STRINGITEMLIST:
        {
            ControlModelLock aLock( *this );
            setNewStringItemList( _rValue, aLock );
                // TODO: this is bogus. setNewStringItemList expects a guard which has the *only*
                // lock to the mutex, but setFastPropertyValue_NoBroadcast is already called with
                // a lock - so we effectively has two locks here, of which setNewStringItemList can
                // only control one.
        }
        break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OComboBoxModel::convertFastPropertyValue(
                        Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
                        throw (IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            bModified = tryPropertyValueEnum(_rConvertedValue, _rOldValue, _rValue, m_eListSourceType);
            break;

        case PROPERTY_ID_LISTSOURCE :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aListSource);
            break;

        case PROPERTY_ID_EMPTY_IS_NULL :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_bEmptyIsNull);
            break;

        case PROPERTY_ID_DEFAULT_TEXT :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aDefaultText);
            break;

        case PROPERTY_ID_STRINGITEMLIST:
            bModified = convertNewListSourceProperty( _rConvertedValue, _rOldValue, _rValue );
            break;

        default:
            bModified = OBoundControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
void OComboBoxModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 6, OBoundControlModel )
        DECL_PROP1(TABINDEX,            sal_Int16,                  BOUND);
        DECL_PROP1(LISTSOURCETYPE,      ListSourceType, BOUND);
        DECL_PROP1(LISTSOURCE,          ::rtl::OUString,            BOUND);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                              BOUND);
        DECL_PROP1(DEFAULT_TEXT,        ::rtl::OUString,            BOUND);
        DECL_PROP1(STRINGITEMLIST,      Sequence< ::rtl::OUString >,BOUND);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
void OComboBoxModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
{
    OBoundControlModel::describeAggregateProperties( _rAggregateProps );

    // superseded properties:
    RemoveProperty( _rAggregateProps, PROPERTY_STRINGITEMLIST );
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OComboBoxModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_COMBOBOX;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream)
        throw(stario::IOException, RuntimeException)
{
    OBoundControlModel::write(_rxOutStream);

    // Version
    // Version 0x0002:  EmptyIsNull
    // Version 0x0003:  ListSource->Seq
    // Version 0x0004:  DefaultText
    // Version 0x0005:  HelpText
    _rxOutStream->writeShort(0x0006);

    // Maskierung fuer any
    sal_uInt16 nAnyMask = 0;
    if (m_aBoundColumn.getValueType().getTypeClass() == TypeClass_SHORT)
        nAnyMask |= BOUNDCOLUMN;
    _rxOutStream << nAnyMask;

    StringSequence aListSourceSeq(&m_aListSource, 1);
    _rxOutStream << aListSourceSeq;
    _rxOutStream << (sal_Int16)m_eListSourceType;

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nBoundColumn = 0;
        m_aBoundColumn >>= nBoundColumn;
        _rxOutStream << nBoundColumn;
    }

    _rxOutStream << (sal_Bool)m_bEmptyIsNull;
    _rxOutStream << m_aDefaultText;
    writeHelpTextCompatibly(_rxOutStream);

    // from version 0x0006 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::read(const Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, RuntimeException)
{
    OBoundControlModel::read(_rxInStream);
    ControlModelLock aLock( *this );

    // since we are "overwriting" the StringItemList of our aggregate (means we have
    // an own place to store the value, instead of relying on our aggregate storing it),
    // we need to respect what the aggregate just read for the StringItemList property.
    try
    {
        if ( m_xAggregateSet.is() )
            setNewStringItemList( m_xAggregateSet->getPropertyValue( PROPERTY_STRINGITEMLIST ), aLock );
    }
    catch( const Exception& )
    {
        OSL_FAIL( "OComboBoxModel::read: caught an exception while examining the aggregate's string item list!" );
    }

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    DBG_ASSERT(nVersion > 0, "OComboBoxModel::read : version 0 ? this should never have been written !");

    if (nVersion > 0x0006)
    {
        OSL_FAIL("OComboBoxModel::read : invalid (means unknown) version !");
        m_aListSource = ::rtl::OUString();
        m_aBoundColumn <<= (sal_Int16)0;
        m_aDefaultText = ::rtl::OUString();
        m_eListSourceType = ListSourceType_TABLE;
        m_bEmptyIsNull = sal_True;
        defaultCommonProperties();
        return;
    }

    // Masking for any
    sal_uInt16 nAnyMask;
    _rxInStream >> nAnyMask;

    // ListSource
    if (nVersion < 0x0003)
    {
        _rxInStream >> m_aListSource;
    }
    else // nVersion == 4
    {
        m_aListSource = ::rtl::OUString();
        StringSequence aListSource;
        _rxInStream >> aListSource;
        const ::rtl::OUString* pToken = aListSource.getConstArray();
        sal_Int32 nLen = aListSource.getLength();
        for (sal_Int32 i = 0; i < nLen; ++i, ++pToken)
            m_aListSource += *pToken;
    }

    sal_Int16 nListSourceType;
    _rxInStream >> nListSourceType;
    m_eListSourceType = (ListSourceType)nListSourceType;

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nValue;
        _rxInStream >> nValue;
        m_aBoundColumn <<= nValue;
    }

    if (nVersion > 0x0001)
    {
        sal_Bool bNull;
        _rxInStream >> bNull;
        m_bEmptyIsNull = bNull;
    }

    if (nVersion > 0x0003)  // nVersion == 4
        _rxInStream >> m_aDefaultText;

    // StringList must be emptied if a ListSource is set.
    // This can be the case if we save in alive mode.
    if  (   !m_aListSource.isEmpty()
        &&  !hasExternalListSource()
        )
    {
        setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( StringSequence() ) );
    }

    if (nVersion > 0x0004)
        readHelpTextCompatibly(_rxInStream);

    if (nVersion > 0x0005)
        readCommonProperties(_rxInStream);

    // After reading in, display the default values
    if ( !getControlSource().isEmpty() )
    {
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        resetNoBroadcast();
    }
}

//------------------------------------------------------------------------------
void OComboBoxModel::loadData( bool _bForce )
{
    DBG_ASSERT(m_eListSourceType != ListSourceType_VALUELIST, "OComboBoxModel::loadData : do not call for a value list !");
    DBG_ASSERT( !hasExternalListSource(), "OComboBoxModel::loadData: cannot load from DB when I have an external list source!" );

    if ( hasExternalListSource() )
        return;

    // Get Connection
    Reference<XRowSet> xForm(m_xCursor, UNO_QUERY);
    if (!xForm.is())
        return;
    Reference<XConnection> xConnection = getConnection(xForm);
    if (!xConnection.is())
        return;

    Reference<XServiceInfo> xServiceInfo(xConnection, UNO_QUERY);
    if (!xServiceInfo.is() || !xServiceInfo->supportsService(SRV_SDB_CONNECTION))
    {
        OSL_FAIL("OComboBoxModel::loadData : invalid connection !");
        return;
    }

    if (m_aListSource.isEmpty() || m_eListSourceType == ListSourceType_VALUELIST)
        return;

    ::utl::SharedUNOComponent< XResultSet > xListCursor;
    try
    {
        m_aListRowSet.setConnection( xConnection );

        bool bExecuteRowSet( false );
        switch (m_eListSourceType)
        {
            case ListSourceType_TABLEFIELDS:
                // don't work with a statement here, the fields will be collected below
                break;
            case ListSourceType_TABLE:
            {
                // does the bound field belong to the table ?
                // if we use an alias for the bound field, we won't find it
                // in that case we use the first field of the table

                Reference<XNameAccess> xFieldsByName = getTableFields(xConnection, m_aListSource);
                Reference<XIndexAccess> xFieldsByIndex(xFieldsByName, UNO_QUERY);

                ::rtl::OUString aFieldName;
                if ( xFieldsByName.is() && xFieldsByName->hasByName( getControlSource() ) )
                {
                    aFieldName = getControlSource();
                }
                else
                {
                    // otherwise look for the alias
                    Reference<XPropertySet> xFormProp(xForm,UNO_QUERY);
                    Reference< XColumnsSupplier > xSupplyFields;
                    xFormProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SingleSelectQueryComposer"))) >>= xSupplyFields;

                    // search the field
                    DBG_ASSERT(xSupplyFields.is(), "OComboBoxModel::loadData : invalid query composer !");

                    Reference< XNameAccess > xFieldNames = xSupplyFields->getColumns();
                    if ( xFieldNames->hasByName( getControlSource() ) )
                    {
                        Reference< XPropertySet > xComposerFieldAsSet;
                        xFieldNames->getByName( getControlSource() ) >>= xComposerFieldAsSet;
                        if (hasProperty(PROPERTY_FIELDSOURCE, xComposerFieldAsSet))
                            xComposerFieldAsSet->getPropertyValue(PROPERTY_FIELDSOURCE) >>= aFieldName;
                    }
                }

                if (aFieldName.isEmpty())
                    break;

                Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
                OSL_ENSURE(xMeta.is(),"No database meta data!");
                if ( xMeta.is() )
                {
                    ::rtl::OUString aQuote = xMeta->getIdentifierQuoteString();

                    ::rtl::OUString sCatalog, sSchema, sTable;
                    qualifiedNameComponents( xMeta, m_aListSource, sCatalog, sSchema, sTable, eInDataManipulation );

                    ::rtl::OUStringBuffer aStatement;
                    aStatement.appendAscii( "SELECT DISTINCT " );
                    aStatement.append     ( quoteName( aQuote, aFieldName ) );
                    aStatement.appendAscii( " FROM " );
                    aStatement.append     ( composeTableNameForSelect( xConnection, sCatalog, sSchema, sTable ) );

                    m_aListRowSet.setEscapeProcessing( sal_False );
                    m_aListRowSet.setCommand( aStatement.makeStringAndClear() );
                    bExecuteRowSet = true;
                }
            }   break;
            case ListSourceType_QUERY:
            {
                m_aListRowSet.setCommandFromQuery( m_aListSource );
                bExecuteRowSet = true;
            }
            break;

            default:
            {
                m_aListRowSet.setEscapeProcessing( ListSourceType_SQLPASSTHROUGH != m_eListSourceType );
                m_aListRowSet.setCommand( m_aListSource );
                bExecuteRowSet = true;
            }
        }

        if ( bExecuteRowSet )
        {
            if ( !_bForce && !m_aListRowSet.isDirty() )
            {
                // if none of the settings of the row set changed, compared to the last
                // invocation of loadData, then don't re-fill the list. Instead, assume
                // the list entries are the same.
                return;
            }
            xListCursor.reset( m_aListRowSet.execute() );
        }
    }
    catch(const SQLException& eSQL)
    {
        onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
        return;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        return;
    }

    ::std::vector< ::rtl::OUString >    aStringList;
    aStringList.reserve(16);
    try
    {
        OSL_ENSURE( xListCursor.is() || ( ListSourceType_TABLEFIELDS == m_eListSourceType ),
            "OComboBoxModel::loadData: logic error!" );
        if ( !xListCursor.is() && ( ListSourceType_TABLEFIELDS != m_eListSourceType ) )
            return;

        switch (m_eListSourceType)
        {
            case ListSourceType_SQL:
            case ListSourceType_SQLPASSTHROUGH:
            case ListSourceType_TABLE:
            case ListSourceType_QUERY:
            {
                // The XDatabaseVariant of the first column
                Reference<XColumnsSupplier> xSupplyCols(xListCursor, UNO_QUERY);
                DBG_ASSERT(xSupplyCols.is(), "OComboBoxModel::loadData : cursor supports the row set service but is no column supplier?!");
                Reference<XIndexAccess> xColumns;
                if (xSupplyCols.is())
                {
                    xColumns = Reference<XIndexAccess>(xSupplyCols->getColumns(), UNO_QUERY);
                    DBG_ASSERT(xColumns.is(), "OComboBoxModel::loadData : no columns supplied by the row set !");
                }
                Reference< XPropertySet > xDataField;
                if ( xColumns.is() )
                    xColumns->getByIndex(0) >>= xDataField;
                if ( !xDataField.is() )
                    return;

                ::dbtools::FormattedColumnValue aValueFormatter( getContext(), xForm, xDataField );

                // Fill Lists
                sal_Int16 i = 0;
                // At the moment by definition the list cursor is positioned _before_ the first row
                while (xListCursor->next() && (i++<SHRT_MAX)) // Set max. count
                {
                    aStringList.push_back( aValueFormatter.getFormattedValue() );
                }
            }
            break;
            case ListSourceType_TABLEFIELDS:
            {
                Reference<XNameAccess> xFieldNames = getTableFields(xConnection, m_aListSource);
                if (xFieldNames.is())
                {
                    StringSequence seqNames = xFieldNames->getElementNames();
                    sal_Int32 nFieldsCount = seqNames.getLength();
                    const ::rtl::OUString* pustrNames = seqNames.getConstArray();

                    for (sal_Int32 k=0; k<nFieldsCount; ++k)
                        aStringList.push_back(pustrNames[k]);
                }
            }
            break;
            default:
                OSL_FAIL( "OComboBoxModel::loadData: unreachable!" );
                break;
        }
    }
    catch(const SQLException& eSQL)
    {
        onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
        return;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        return;
    }

    // Create StringSequence for ListBox
    StringSequence aStringSeq(aStringList.size());
    ::rtl::OUString* pStringAry = aStringSeq.getArray();
    for (sal_Int32 i = 0; i<aStringSeq.getLength(); ++i)
        pStringAry[i] = aStringList[i];

    // Set String-Sequence at ListBox
    setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( aStringSeq ) );
}

//------------------------------------------------------------------------------
void OComboBoxModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    Reference<XPropertySet> xField = getField();
    if ( xField.is() )
        m_pValueFormatter.reset( new ::dbtools::FormattedColumnValue( getContext(), Reference< XRowSet >( _rxForm, UNO_QUERY ), xField ) );
    getPropertyValue( PROPERTY_STRINGITEMLIST ) >>= m_aDesignModeStringItems;

    // Only load data if a ListSource was supplied
    if ( !m_aListSource.isEmpty() && m_xCursor.is() && !hasExternalListSource() )
        loadData( false );
}

//------------------------------------------------------------------------------
void OComboBoxModel::onDisconnectedDbColumn()
{
    m_pValueFormatter.reset();

    // reset the string item list
    if ( !hasExternalListSource() )
        setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( m_aDesignModeStringItems ) );

    m_aListRowSet.dispose();
}

//------------------------------------------------------------------------------
void SAL_CALL OComboBoxModel::reloaded( const EventObject& aEvent ) throw(RuntimeException)
{
    OBoundControlModel::reloaded(aEvent);

    // reload data if we have a list source
    if ( !m_aListSource.isEmpty() && m_xCursor.is() && !hasExternalListSource() )
        loadData( false );
}

//------------------------------------------------------------------------------
void OComboBoxModel::resetNoBroadcast()
{
    OBoundControlModel::resetNoBroadcast();
    m_aLastKnownValue.clear();
}

//-----------------------------------------------------------------------------
sal_Bool OComboBoxModel::commitControlValueToDbColumn( bool _bPostReset )
{
    Any aNewValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );

    ::rtl::OUString sNewValue;
    aNewValue >>= sNewValue;

    bool bModified = ( aNewValue != m_aLastKnownValue );
    if ( bModified )
    {
        if  (   !aNewValue.hasValue()
            ||  (   sNewValue.isEmpty()         // an empty string
                &&  m_bEmptyIsNull              // which should be interpreted as NULL
                )
            )
        {
            m_xColumnUpdate->updateNull();
        }
        else
        {
            try
            {
                OSL_PRECOND( m_pValueFormatter.get(), "OComboBoxModel::commitControlValueToDbColumn: no value formatter!" );
                if ( m_pValueFormatter.get() )
                {
                    if ( !m_pValueFormatter->setFormattedValue( sNewValue ) )
                        return sal_False;
                }
                else
                    m_xColumnUpdate->updateString( sNewValue );
            }
            catch ( const Exception& )
            {
                return sal_False;
            }
        }

        m_aLastKnownValue = aNewValue;
    }

    // add the new value to the list
    sal_Bool bAddToList = bModified && !_bPostReset;
        // (only if this is not the "commit" triggered by a "reset")

    if ( bAddToList )
    {
        StringSequence aStringItemList;
        if ( getPropertyValue( PROPERTY_STRINGITEMLIST ) >>= aStringItemList )
        {
            const ::rtl::OUString* pStringItems = aStringItemList.getConstArray();
            sal_Int32 i;
            for (i=0; i<aStringItemList.getLength(); ++i, ++pStringItems)
            {
                if ( pStringItems->equals( sNewValue ) )
                    break;
            }

            // not found -> add
            if (i >= aStringItemList.getLength())
            {
                sal_Int32 nOldLen = aStringItemList.getLength();
                aStringItemList.realloc( nOldLen + 1 );
                aStringItemList.getArray()[ nOldLen ] = sNewValue;

                setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( aStringItemList ) );
            }
        }
    }

    return sal_True;
}

// XPropertiesChangeListener
//------------------------------------------------------------------------------
Any OComboBoxModel::translateDbColumnToControlValue()
{
    OSL_PRECOND( m_pValueFormatter.get(), "OComboBoxModel::translateDbColumnToControlValue: no value formatter!" );
    if ( m_pValueFormatter.get() )
    {
        ::rtl::OUString sValue( m_pValueFormatter->getFormattedValue() );
        if  (   sValue.isEmpty()
            &&  m_pValueFormatter->getColumn().is()
            &&  m_pValueFormatter->getColumn()->wasNull()
            )
        {
            m_aLastKnownValue.clear();
        }
        else
        {

            m_aLastKnownValue <<= sValue;
        }
    }
    else
        m_aLastKnownValue.clear();

    return m_aLastKnownValue.hasValue() ? m_aLastKnownValue : makeAny( ::rtl::OUString() );
        // (m_aLastKnownValue is alllowed to be VOID, the control value isn't)
}

//------------------------------------------------------------------------------
Any OComboBoxModel::getDefaultForReset() const
{
    return makeAny( m_aDefaultText );
}

//--------------------------------------------------------------------
void OComboBoxModel::stringItemListChanged( ControlModelLock& /*_rInstanceLock*/ )
{
    if ( m_xAggregateSet.is() )
        m_xAggregateSet->setPropertyValue( PROPERTY_STRINGITEMLIST, makeAny( getStringItemList() ) );
}

//--------------------------------------------------------------------
void OComboBoxModel::connectedExternalListSource( )
{
    // TODO?
}

//--------------------------------------------------------------------
void OComboBoxModel::disconnectedExternalListSource( )
{
    // TODO?
}

//--------------------------------------------------------------------
void OComboBoxModel::refreshInternalEntryList()
{
    DBG_ASSERT( !hasExternalListSource(), "OComboBoxModel::refreshInternalEntryList: invalid call!" );

    if  (   !hasExternalListSource( )
        &&  ( m_eListSourceType != ListSourceType_VALUELIST )
        &&  ( m_xCursor.is() )
        )
    {
        loadData( true );
    }
}

//--------------------------------------------------------------------
void SAL_CALL OComboBoxModel::disposing( const EventObject& _rSource ) throw ( RuntimeException )
{
    if ( !OEntryListHelper::handleDisposing( _rSource ) )
        OBoundControlModel::disposing( _rSource );
}

//========================================================================
//= OComboBoxControl
//========================================================================

//------------------------------------------------------------------
InterfaceRef SAL_CALL OComboBoxControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OComboBoxControl(_rxFactory));
}

//------------------------------------------------------------------------------
OComboBoxControl::OComboBoxControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_COMBOBOX)
{
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OComboBoxControl::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_COMBOBOX;
    return aSupported;
}

//.........................................................................
}
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
