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


#include "ComboBox.hxx"
#include <property.hxx>
#include <services.hxx>

#include <frm_resource.hxx>
#include <strings.hrc>
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

#include <comphelper/basicio.hxx>
#include <comphelper/property.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/dbconversion.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/sharedunocomponent.hxx>

#include <limits.h>

using namespace dbtools;


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


// class OComboBoxModel
Sequence<Type> OComboBoxModel::_getTypes()
{
    return ::comphelper::concatSequences(
        OBoundControlModel::_getTypes(),
        OEntryListHelper::getTypes(),
        OErrorBroadcaster::getTypes()
    );
}

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL OComboBoxModel::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControlModel::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 9 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_COMBOBOX;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_COMBOBOX;
    *pStoreTo++ = BINDABLE_DATABASE_COMBO_BOX;

    *pStoreTo++ = FRM_COMPONENT_COMBOBOX;

    return aSupported;
}


Any SAL_CALL OComboBoxModel::queryAggregation(const Type& _rType)
{
    Any aReturn = OBoundControlModel::queryAggregation( _rType );
    if ( !aReturn.hasValue() )
        aReturn = OEntryListHelper::queryInterface( _rType );
    if ( !aReturn.hasValue() )
        aReturn = OErrorBroadcaster::queryInterface( _rType );
    return aReturn;
}


OComboBoxModel::OComboBoxModel(const Reference<XComponentContext>& _rxFactory)
    :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_COMBOBOX, FRM_SUN_CONTROL_COMBOBOX, true, true, true )
     // use the old control name for compatibility reasons
    ,OEntryListHelper( static_cast<OControlModel&>(*this) )
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
    ,m_aListRowSet()
    ,m_eListSourceType(ListSourceType_TABLE)
    ,m_bEmptyIsNull(true)
{
    m_nClassId = FormComponentType::COMBOBOX;
    initValueProperty( PROPERTY_TEXT, PROPERTY_ID_TEXT );
}


OComboBoxModel::OComboBoxModel( const OComboBoxModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory )
    ,OEntryListHelper( *_pOriginal, static_cast<OControlModel&>(*this) )
    ,OErrorBroadcaster( OComponentHelper::rBHelper )
    ,m_aListRowSet()
    ,m_aListSource( _pOriginal->m_aListSource )
    ,m_aDefaultText( _pOriginal->m_aDefaultText )
    ,m_eListSourceType( _pOriginal->m_eListSourceType )
    ,m_bEmptyIsNull( _pOriginal->m_bEmptyIsNull )
{
}


OComboBoxModel::~OComboBoxModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

}

// XCloneable

IMPLEMENT_DEFAULT_CLONING( OComboBoxModel )


void OComboBoxModel::disposing()
{
    OBoundControlModel::disposing();
    OEntryListHelper::disposing();
    OErrorBroadcaster::disposing();
}


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
            _rValue <<= comphelper::containerToSequence(getStringItemList());
            break;

        case PROPERTY_ID_TYPEDITEMLIST:
            _rValue <<= getTypedItemList();
            break;

        default:
            OBoundControlModel::getFastPropertyValue(_rValue, _nHandle);
    }
}


void OComboBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_LISTSOURCETYPE :
            DBG_ASSERT(_rValue.getValueType().equals(::cppu::UnoType<ListSourceType>::get()),
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
                // FIXME: this is bogus. setNewStringItemList expects a guard which has the *only*
                // lock to the mutex, but setFastPropertyValue_NoBroadcast is already called with
                // a lock - so we effectively has two locks here, of which setNewStringItemList can
                // only control one.
        }
        break;

        case PROPERTY_ID_TYPEDITEMLIST:
        {
            ControlModelLock aLock( *this );
            setNewTypedItemList( _rValue, aLock );
            // Same FIXME as above.
        }
        break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }
}

sal_Bool OComboBoxModel::convertFastPropertyValue(
                        Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
{
    bool bModified(false);
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

        case PROPERTY_ID_TYPEDITEMLIST :
            if (hasExternalListSource())
                throw IllegalArgumentException();
            bModified = tryPropertyValue( _rConvertedValue, _rOldValue, _rValue, getTypedItemList());
            break;

        default:
            bModified = OBoundControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
            break;
    }
    return bModified;
}

void OComboBoxModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 7, OBoundControlModel )
        DECL_PROP1(TABINDEX,            sal_Int16,                  BOUND);
        DECL_PROP1(LISTSOURCETYPE,      ListSourceType, BOUND);
        DECL_PROP1(LISTSOURCE,          OUString,            BOUND);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                              BOUND);
        DECL_PROP1(DEFAULT_TEXT,        OUString,            BOUND);
        DECL_PROP1(STRINGITEMLIST,      Sequence< OUString >,BOUND);
        DECL_PROP1(TYPEDITEMLIST,       Sequence< Any >,        OPTIONAL);
    END_DESCRIBE_PROPERTIES();
}


void OComboBoxModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
{
    OBoundControlModel::describeAggregateProperties( _rAggregateProps );

    // superseded properties:
    RemoveProperty( _rAggregateProps, PROPERTY_STRINGITEMLIST );
    RemoveProperty( _rAggregateProps, PROPERTY_TYPEDITEMLIST );
}


OUString SAL_CALL OComboBoxModel::getServiceName()
{
    return OUString(FRM_COMPONENT_COMBOBOX);  // old (non-sun) name for compatibility !
}


void SAL_CALL OComboBoxModel::write(const Reference<css::io::XObjectOutputStream>& _rxOutStream)
{
    OBoundControlModel::write(_rxOutStream);

    // Version
    // Version 0x0002:  EmptyIsNull
    // Version 0x0003:  ListSource->Seq
    // Version 0x0004:  DefaultText
    // Version 0x0005:  HelpText
    _rxOutStream->writeShort(0x0006);

    // Mask for Any
    sal_uInt16 nAnyMask = 0;
    if (m_aBoundColumn.getValueType().getTypeClass() == TypeClass_SHORT)
        nAnyMask |= BOUNDCOLUMN;
    _rxOutStream << nAnyMask;

    css::uno::Sequence<OUString> aListSourceSeq(&m_aListSource, 1);
    _rxOutStream << aListSourceSeq;
    _rxOutStream << static_cast<sal_Int16>(m_eListSourceType);

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nBoundColumn = 0;
        m_aBoundColumn >>= nBoundColumn;
        _rxOutStream << nBoundColumn;
    }

    _rxOutStream << m_bEmptyIsNull;
    _rxOutStream << m_aDefaultText;
    writeHelpTextCompatibly(_rxOutStream);

    // from version 0x0006 : common properties
    writeCommonProperties(_rxOutStream);
}


void SAL_CALL OComboBoxModel::read(const Reference<css::io::XObjectInputStream>& _rxInStream)
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
        m_aListSource.clear();
        m_aBoundColumn <<= sal_Int16(0);
        m_aDefaultText.clear();
        m_eListSourceType = ListSourceType_TABLE;
        m_bEmptyIsNull = true;
        defaultCommonProperties();
        return;
    }

    // Mask for Any
    sal_uInt16 nAnyMask;
    _rxInStream >> nAnyMask;

    // ListSource
    if (nVersion < 0x0003)
    {
        _rxInStream >> m_aListSource;
    }
    else // nVersion == 4
    {
        m_aListSource.clear();
        css::uno::Sequence<OUString> aListSource;
        _rxInStream >> aListSource;
        for (const OUString& rToken : aListSource)
            m_aListSource += rToken;
    }

    sal_Int16 nListSourceType;
    _rxInStream >> nListSourceType;
    m_eListSourceType = static_cast<ListSourceType>(nListSourceType);

    if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
    {
        sal_Int16 nValue;
        _rxInStream >> nValue;
        m_aBoundColumn <<= nValue;
    }

    if (nVersion > 0x0001)
    {
        bool bNull;
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
        setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( css::uno::Sequence<OUString>() ) );
        setFastPropertyValue( PROPERTY_ID_TYPEDITEMLIST, makeAny( css::uno::Sequence<css::uno::Any>() ) );
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

                OUString aFieldName;
                if ( xFieldsByName.is() && xFieldsByName->hasByName( getControlSource() ) )
                {
                    aFieldName = getControlSource();
                }
                else
                {
                    // otherwise look for the alias
                    Reference<XPropertySet> xFormProp(xForm,UNO_QUERY);
                    Reference< XColumnsSupplier > xSupplyFields;
                    xFormProp->getPropertyValue("SingleSelectQueryComposer") >>= xSupplyFields;

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
                    OUString aQuote = xMeta->getIdentifierQuoteString();

                    OUString sCatalog, sSchema, sTable;
                    qualifiedNameComponents( xMeta, m_aListSource, sCatalog, sSchema, sTable, EComposeRule::InDataManipulation );

                    OUStringBuffer aStatement;
                    aStatement.append( "SELECT DISTINCT " );
                    aStatement.append     ( quoteName( aQuote, aFieldName ) );
                    aStatement.append( " FROM " );
                    aStatement.append     ( composeTableNameForSelect( xConnection, sCatalog, sSchema, sTable ) );

                    m_aListRowSet.setEscapeProcessing( false );
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
        DBG_UNHANDLED_EXCEPTION("forms.component");
        return;
    }

    ::std::vector< OUString >    aStringList;
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
                    xColumns.set(xSupplyCols->getColumns(), UNO_QUERY);
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
                    for (const OUString& rustrNames : xFieldNames->getElementNames())
                        aStringList.push_back(rustrNames);
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
        DBG_UNHANDLED_EXCEPTION("forms.component");
        return;
    }

    // Set String-Sequence at ListBox
    setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( comphelper::containerToSequence(aStringList) ) );
    // Reset TypedItemList, no matching data.
    setFastPropertyValue( PROPERTY_ID_TYPEDITEMLIST, makeAny( css::uno::Sequence<css::uno::Any>() ) );
}


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


void OComboBoxModel::onDisconnectedDbColumn()
{
    m_pValueFormatter.reset();

    // reset the string item list
    if ( !hasExternalListSource() )
        setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( m_aDesignModeStringItems ) );

    m_aListRowSet.dispose();
}


void SAL_CALL OComboBoxModel::reloaded( const EventObject& aEvent )
{
    OBoundControlModel::reloaded(aEvent);

    // reload data if we have a list source
    if ( !m_aListSource.isEmpty() && m_xCursor.is() && !hasExternalListSource() )
        loadData( false );
}


void OComboBoxModel::resetNoBroadcast()
{
    OBoundControlModel::resetNoBroadcast();
    m_aLastKnownValue.clear();
}


bool OComboBoxModel::commitControlValueToDbColumn( bool _bPostReset )
{
    Any aNewValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );

    OUString sNewValue;
    aNewValue >>= sNewValue;

    bool bModified = ( aNewValue != m_aLastKnownValue );
    if ( bModified )
    {
        if  (   !aNewValue.hasValue()
            ||  (   sNewValue.isEmpty() // an empty string
                &&  m_bEmptyIsNull      // which should be interpreted as NULL
                )
            )
        {
            m_xColumnUpdate->updateNull();
        }
        else
        {
            try
            {
                OSL_PRECOND(m_pValueFormatter,
                            "OComboBoxModel::commitControlValueToDbColumn: no value formatter!");
                if (m_pValueFormatter)
                {
                    if ( !m_pValueFormatter->setFormattedValue( sNewValue ) )
                        return false;
                }
                else
                    m_xColumnUpdate->updateString( sNewValue );
            }
            catch ( const Exception& )
            {
                return false;
            }
        }

        m_aLastKnownValue = aNewValue;
    }

    // add the new value to the list
    bool bAddToList = bModified && !_bPostReset;
    // (only if this is not the "commit" triggered by a "reset")

    if ( bAddToList )
    {
        css::uno::Sequence<OUString> aStringItemList;
        if ( getPropertyValue( PROPERTY_STRINGITEMLIST ) >>= aStringItemList )
        {
            bool bFound = false;
            for (const OUString& rStringItem : aStringItemList)
            {
                if ( (bFound = rStringItem == sNewValue) )
                    break;
            }

            // not found -> add
            if (!bFound)
            {
                sal_Int32 nOldLen = aStringItemList.getLength();
                aStringItemList.realloc( nOldLen + 1 );
                aStringItemList[ nOldLen ] = sNewValue;

                setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( aStringItemList ) );
                setFastPropertyValue( PROPERTY_ID_TYPEDITEMLIST, makeAny( css::uno::Sequence<css::uno::Any>() ) );
            }
        }
    }

    return true;
}

// XPropertiesChangeListener

Any OComboBoxModel::translateDbColumnToControlValue()
{
    OSL_PRECOND(m_pValueFormatter,
                "OComboBoxModel::translateDbColumnToControlValue: no value formatter!");
    if (m_pValueFormatter)
    {
        OUString sValue( m_pValueFormatter->getFormattedValue() );
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

    return m_aLastKnownValue.hasValue() ? m_aLastKnownValue : makeAny( OUString() );
        // (m_aLastKnownValue is allowed to be VOID, the control value isn't)
}


Any OComboBoxModel::getDefaultForReset() const
{
    return makeAny( m_aDefaultText );
}


void OComboBoxModel::stringItemListChanged( ControlModelLock& /*_rInstanceLock*/ )
{
    if ( m_xAggregateSet.is() )
    {
        m_xAggregateSet->setPropertyValue( PROPERTY_STRINGITEMLIST, makeAny( comphelper::containerToSequence(getStringItemList()) ) );
        m_xAggregateSet->setPropertyValue( PROPERTY_TYPEDITEMLIST, makeAny( getTypedItemList()) ) ;
    }
}


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


void SAL_CALL OComboBoxModel::disposing( const EventObject& _rSource )
{
    if ( !OEntryListHelper::handleDisposing( _rSource ) )
        OBoundControlModel::disposing( _rSource );
}


//= OComboBoxControl

OComboBoxControl::OComboBoxControl(const Reference<XComponentContext>& _rxContext)
    :OBoundControl(_rxContext, VCL_CONTROL_COMBOBOX)
{
}


css::uno::Sequence<OUString> SAL_CALL OComboBoxControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_COMBOBOX;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_COMBOBOX;
    return aSupported;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OComboBoxModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OComboBoxModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OComboBoxControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OComboBoxControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
