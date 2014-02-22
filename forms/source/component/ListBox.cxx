/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "ListBox.hxx"
#include "property.hxx"
#include "property.hrc"
#include "services.hxx"
#include "frm_resource.hxx"
#include "frm_resource.hrc"
#include "BaseListBox.hxx"
#include "listenercontainers.hxx"
#include "componenttools.hxx"

#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include <comphelper/basicio.hxx>
#include <comphelper/container.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/listenernotification.hxx>
#include <connectivity/dbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include <connectivity/dbconversion.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <unotools/sharedunocomponent.hxx>
#include <vcl/svapp.hxx>

#include <boost/optional.hpp>

#include <algorithm>
#include <functional>
#include <iterator>



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
    using namespace ::dbtools;

    using ::connectivity::ORowSetValue;

    
    
    
    namespace
    {
        
        struct RowSetValueToString : public ::std::unary_function< ORowSetValue, OUString >
        {
            OUString operator()( const ORowSetValue& _value ) const
            {
                return _value.getString();
            }
        };

        
        struct AppendRowSetValueString : public ::std::unary_function< OUString, void >
        {
            AppendRowSetValueString( OUString& _string )
                :m_string( _string )
            {
            }

            void operator()( const OUString _append )
            {
                m_string += _append;
            }

        private:
            OUString&    m_string;
        };

        
        Sequence< OUString > lcl_convertToStringSequence( const ValueList& _values )
        {
            Sequence< OUString > aStrings( _values.size() );
            ::std::transform(
                _values.begin(),
                _values.end(),
                aStrings.getArray(),
                RowSetValueToString()
            );
            return aStrings;
        }
    }

    
    
    
    typedef ::comphelper::EventHolder< ItemEvent >    ItemEventDescription;

    
    
    
    
    InterfaceRef SAL_CALL OListBoxModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
    {
        return *(new OListBoxModel( comphelper::getComponentContext(_rxFactory) ));
    }

    
    Sequence< Type> OListBoxModel::_getTypes()
    {
        return TypeBag(
            OBoundControlModel::_getTypes(),
            OEntryListHelper::getTypes(),
            OErrorBroadcaster::getTypes()
        ).getTypes();
    }

    
    void OListBoxModel::init()
    {
        startAggregatePropertyListening( PROPERTY_STRINGITEMLIST );
    }

    
    OListBoxModel::OListBoxModel(const Reference<XComponentContext>& _rxFactory)
        :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_LISTBOX, FRM_SUN_CONTROL_LISTBOX, sal_True, sal_True, sal_True )
        
        ,OEntryListHelper( (OControlModel&)*this )
        ,OErrorBroadcaster( OComponentHelper::rBHelper )
        ,m_aListRowSet()
        ,m_nConvertedBoundValuesType(0)
        ,m_nNULLPos(-1)
        ,m_nBoundColumnType( DataType::SQLNULL )
    {

        m_nClassId = FormComponentType::LISTBOX;
        m_eListSourceType = ListSourceType_VALUELIST;
        m_aBoundColumn <<= (sal_Int16)1;
        initValueProperty( PROPERTY_SELECT_SEQ, PROPERTY_ID_SELECT_SEQ);

        init();
    }

    
    OListBoxModel::OListBoxModel( const OListBoxModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
        :OBoundControlModel( _pOriginal, _rxFactory )
        ,OEntryListHelper( *_pOriginal, (OControlModel&)*this )
        ,OErrorBroadcaster( OComponentHelper::rBHelper )
        ,m_aListRowSet()
        ,m_eListSourceType( _pOriginal->m_eListSourceType )
        ,m_aBoundColumn( _pOriginal->m_aBoundColumn )
        ,m_aListSourceValues( _pOriginal->m_aListSourceValues )
        ,m_aBoundValues( _pOriginal->m_aBoundValues )
        ,m_nConvertedBoundValuesType(0)
        ,m_aDefaultSelectSeq( _pOriginal->m_aDefaultSelectSeq )
        ,m_nNULLPos(-1)
        ,m_nBoundColumnType( DataType::SQLNULL )
    {

        init();
    }

    
    OListBoxModel::~OListBoxModel()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }

    }

    
    
    IMPLEMENT_DEFAULT_CLONING( OListBoxModel )

    
    
    StringSequence SAL_CALL OListBoxModel::getSupportedServiceNames() throw(RuntimeException)
    {
        StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();

        sal_Int32 nOldLen = aSupported.getLength();
        aSupported.realloc( nOldLen + 8 );
        OUString* pStoreTo = aSupported.getArray() + nOldLen;

        *pStoreTo++ = BINDABLE_CONTROL_MODEL;
        *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
        *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

        *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
        *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

        *pStoreTo++ = FRM_SUN_COMPONENT_LISTBOX;
        *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_LISTBOX;
        *pStoreTo++ = BINDABLE_DATABASE_LIST_BOX;

        return aSupported;
    }

    
    Any SAL_CALL OListBoxModel::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        Any aReturn = OBoundControlModel::queryAggregation( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OEntryListHelper::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OErrorBroadcaster::queryInterface( _rType );
        return aReturn;
    }

    
    
    void OListBoxModel::disposing()
    {
        OBoundControlModel::disposing();
        OEntryListHelper::disposing();
        OErrorBroadcaster::disposing();
    }

    
    void OListBoxModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
    {
        switch (_nHandle)
        {
        case PROPERTY_ID_BOUNDCOLUMN:
            _rValue <<= m_aBoundColumn;
            break;

        case PROPERTY_ID_LISTSOURCETYPE:
            _rValue <<= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE:
            _rValue <<= lcl_convertToStringSequence( m_aListSourceValues );
            break;

        case PROPERTY_ID_VALUE_SEQ:
            _rValue <<= lcl_convertToStringSequence( m_aBoundValues );
            break;

        case PROPERTY_ID_SELECT_VALUE_SEQ:
            _rValue = getCurrentMultiValue();
            break;

        case PROPERTY_ID_SELECT_VALUE:
           _rValue = getCurrentSingleValue();
            break;

        case PROPERTY_ID_DEFAULT_SELECT_SEQ:
            _rValue <<= m_aDefaultSelectSeq;
            break;

        case PROPERTY_ID_STRINGITEMLIST:
            _rValue <<= getStringItemList();
            break;

        default:
            OBoundControlModel::getFastPropertyValue(_rValue, _nHandle);
        }
    }

    
    void OListBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (com::sun::star::uno::Exception)
    {
        switch (_nHandle)
        {
        case PROPERTY_ID_BOUNDCOLUMN :
            DBG_ASSERT((_rValue.getValueType().getTypeClass() == TypeClass_SHORT) || (_rValue.getValueType().getTypeClass() == TypeClass_VOID),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            m_aBoundColumn = _rValue;
            break;

        case PROPERTY_ID_LISTSOURCETYPE :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(static_cast<ListSourceType*>(0))),
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE:
        {
            
            Sequence< OUString > aListSource;
            OSL_VERIFY( _rValue >>= aListSource );

            
            ValueList().swap(m_aListSourceValues);
            ::std::copy(
                aListSource.getConstArray(),
                aListSource.getConstArray() + aListSource.getLength(),
                ::std::insert_iterator< ValueList >( m_aListSourceValues, m_aListSourceValues.end() )
            );

            
            if ( m_eListSourceType == ListSourceType_VALUELIST )
            {
                setBoundValues(m_aListSourceValues);
            }
            else
            {
                if ( m_xCursor.is() && !hasField() && !hasExternalListSource() )
                    
                    
                    loadData( false );
            }
        }
        break;

        case PROPERTY_ID_VALUE_SEQ :
            SAL_WARN( "forms.component", "ValueItemList is read-only!" );
            throw PropertyVetoException();

        case PROPERTY_ID_SELECT_VALUE_SEQ :
        {
            Sequence< const Any > v;
            _rValue >>= v;
            Any newSelectSeq(translateBindingValuesToControlValue(v));
            setControlValue( newSelectSeq, eOther );
        }
        break;

        case PROPERTY_ID_SELECT_VALUE :
        {
            ORowSetValue v;
            v.fill(_rValue);
            Any newSelectSeq(translateDbValueToControlValue(v));
            setControlValue( newSelectSeq, eOther );
        }
        break;

        case PROPERTY_ID_DEFAULT_SELECT_SEQ :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(static_cast< Sequence<sal_Int16>*>(0))),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aDefaultSelectSeq;

            DBG_ASSERT(m_xAggregateFastSet.is(), "OListBoxModel::setFastPropertyValue_NoBroadcast(DEFAULT_SELECT_SEQ) : invalid aggregate !");
            if ( m_xAggregateFastSet.is() )
                setControlValue( _rValue, eOther );
            break;

        case PROPERTY_ID_STRINGITEMLIST:
        {
            ControlModelLock aLock( *this );
            setNewStringItemList( _rValue, aLock );
                
                
                
                
        }
        resetNoBroadcast();
        break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
        }
    }

    
    sal_Bool OListBoxModel::convertFastPropertyValue(
        Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
        throw (IllegalArgumentException)
    {
        sal_Bool bModified(sal_False);
        switch (_nHandle)
        {
        case PROPERTY_ID_BOUNDCOLUMN :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aBoundColumn, ::getCppuType(static_cast<sal_Int16*>(0)));
            break;

        case PROPERTY_ID_LISTSOURCETYPE:
            bModified = tryPropertyValueEnum(_rConvertedValue, _rOldValue, _rValue, m_eListSourceType);
            break;

        case PROPERTY_ID_LISTSOURCE:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, lcl_convertToStringSequence( m_aListSourceValues ) );
            break;

        case PROPERTY_ID_VALUE_SEQ :
            SAL_WARN( "forms.component", "ValueItemList is read-only!" );
            throw PropertyVetoException();

        case PROPERTY_ID_SELECT_VALUE_SEQ :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, getCurrentMultiValue());
            break;

        case PROPERTY_ID_SELECT_VALUE :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, getCurrentSingleValue());
            break;

        case PROPERTY_ID_DEFAULT_SELECT_SEQ :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aDefaultSelectSeq);
            break;

        case PROPERTY_ID_STRINGITEMLIST:
            bModified = convertNewListSourceProperty( _rConvertedValue, _rOldValue, _rValue );
            break;

        default:
            return OBoundControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
        }
        return bModified;
    }

    
    void SAL_CALL OListBoxModel::setPropertyValues( const Sequence< OUString >& _rPropertyNames, const Sequence< Any >& _rValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
    {
        
        
        const Any* pSelectSequenceValue = NULL;

        const OUString* pStartPos = _rPropertyNames.getConstArray();
        const OUString* pEndPos   = _rPropertyNames.getConstArray() + _rPropertyNames.getLength();
        const OUString* pSelectedItemsPos = ::std::find_if(
            pStartPos, pEndPos,
             ::std::bind2nd( ::std::equal_to< OUString >(), PROPERTY_SELECT_SEQ )
        );
        const OUString* pStringItemListPos = ::std::find_if(
            pStartPos, pEndPos,
             ::std::bind2nd( ::std::equal_to< OUString >(), PROPERTY_STRINGITEMLIST )
        );
        if ( ( pSelectedItemsPos != pEndPos ) && ( pStringItemListPos != pEndPos ) )
        {
            
            
            pSelectSequenceValue = _rValues.getConstArray() + ( pSelectedItemsPos - pStartPos );
        }

        OBoundControlModel::setPropertyValues( _rPropertyNames, _rValues );

        if ( pSelectSequenceValue )
        {
            setPropertyValue( PROPERTY_SELECT_SEQ, *pSelectSequenceValue );
            
            
            

        }
    }

    
    void OListBoxModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( 9, OBoundControlModel )
            DECL_PROP1(TABINDEX,            sal_Int16,                      BOUND);
            DECL_PROP2(BOUNDCOLUMN,         sal_Int16,                      BOUND, MAYBEVOID);
            DECL_PROP1(LISTSOURCETYPE,      ListSourceType,                 BOUND);
            DECL_PROP1(LISTSOURCE,          StringSequence,                 BOUND);
            DECL_PROP3(VALUE_SEQ,           StringSequence,                 BOUND, READONLY, TRANSIENT);
            DECL_PROP2(SELECT_VALUE_SEQ,    Sequence< Any >,                BOUND, TRANSIENT);
            DECL_PROP2(SELECT_VALUE,        Any,                            BOUND, TRANSIENT);
            DECL_PROP1(DEFAULT_SELECT_SEQ,  Sequence<sal_Int16>,            BOUND);
            DECL_PROP1(STRINGITEMLIST,      Sequence< OUString >,    BOUND);
        END_DESCRIBE_PROPERTIES();
    }

    
    void OListBoxModel::_propertyChanged( const PropertyChangeEvent& i_rEvent ) throw ( RuntimeException )
    {
        if ( i_rEvent.PropertyName == PROPERTY_STRINGITEMLIST )
        {
            ControlModelLock aLock( *this );
            
            
            
            setNewStringItemList( i_rEvent.NewValue, aLock );
            
            return;
        }
        OBoundControlModel::_propertyChanged( i_rEvent );
    }

    
    void OListBoxModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
    {
        OBoundControlModel::describeAggregateProperties( _rAggregateProps );

        
        RemoveProperty( _rAggregateProps, PROPERTY_STRINGITEMLIST );
    }

    
    OUString SAL_CALL OListBoxModel::getServiceName() throw(RuntimeException)
    {
        return OUString(FRM_COMPONENT_LISTBOX);   
    }

    
    void SAL_CALL OListBoxModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
        throw(IOException, RuntimeException)
    {
        OBoundControlModel::write(_rxOutStream);

        
        Sequence<sal_Int16> aDummySeq;

        
        
        _rxOutStream->writeShort(0x0004);

        
        sal_uInt16 nAnyMask = 0;
        if (m_aBoundColumn.getValueType().getTypeClass() != TypeClass_VOID)
            nAnyMask |= BOUNDCOLUMN;

        _rxOutStream << nAnyMask;

        _rxOutStream << lcl_convertToStringSequence( m_aListSourceValues );
        _rxOutStream << (sal_Int16)m_eListSourceType;
        _rxOutStream << aDummySeq;
        _rxOutStream << m_aDefaultSelectSeq;

        if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
        {
            sal_Int16 nBoundColumn = 0;
            m_aBoundColumn >>= nBoundColumn;
            _rxOutStream << nBoundColumn;
        }
        writeHelpTextCompatibly(_rxOutStream);

        
        writeCommonProperties(_rxOutStream);
    }

    
    void SAL_CALL OListBoxModel::read(const Reference<XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException)
    {
        
        

        OBoundControlModel::read(_rxInStream);
        ControlModelLock aLock( *this );

        
        
        
        try
        {
            if ( m_xAggregateSet.is() )
                setNewStringItemList( m_xAggregateSet->getPropertyValue( PROPERTY_STRINGITEMLIST ), aLock );
        }
        catch( const Exception& )
        {
            SAL_WARN( "forms.component", "OComboBoxModel::read: caught an exception while examining the aggregate's string item list!" );
        }

        
        sal_uInt16 nVersion = _rxInStream->readShort();
        DBG_ASSERT(nVersion > 0, "OListBoxModel::read : version 0 ? this should never have been written !");

        if (nVersion > 0x0004)
        {
            SAL_WARN( "forms.component", "OListBoxModel::read : invalid (means unknown) version !");
            ValueList().swap(m_aListSourceValues);
            m_aBoundColumn <<= (sal_Int16)0;
            clearBoundValues();
            m_eListSourceType = ListSourceType_VALUELIST;
            m_aDefaultSelectSeq.realloc(0);
            defaultCommonProperties();
            return;
        }

        
        sal_uInt16 nAnyMask;
        _rxInStream >> nAnyMask;

        
        StringSequence aListSourceSeq;
        if (nVersion == 0x0001)
        {
            
            OUString sListSource;
            _rxInStream >> sListSource;

            sal_Int32 nTokens = 1;
            const sal_Unicode* pStr = sListSource.getStr();
            while ( *pStr )
            {
                if ( *pStr == ';' )
                    nTokens++;
                pStr++;
            }
            aListSourceSeq.realloc( nTokens );
            for (sal_uInt16 i=0; i<nTokens; ++i)
            {
                sal_Int32 nTmp = 0;
                aListSourceSeq.getArray()[i] = sListSource.getToken(i,';',nTmp);
            }
        }
        else
            _rxInStream >> aListSourceSeq;

        sal_Int16 nListSourceType;
        _rxInStream >> nListSourceType;
        m_eListSourceType = (ListSourceType)nListSourceType;
        Any aListSourceSeqAny;
        aListSourceSeqAny <<= aListSourceSeq;

        setFastPropertyValue(PROPERTY_ID_LISTSOURCE, aListSourceSeqAny );

        
        Sequence<sal_Int16> aDummySeq;
        _rxInStream >> aDummySeq;

        
        Sequence<sal_Int16> aDefaultSelectSeq;
        _rxInStream >> aDefaultSelectSeq;
        Any aDefaultSelectSeqAny;
        aDefaultSelectSeqAny <<= aDefaultSelectSeq;
        setFastPropertyValue(PROPERTY_ID_DEFAULT_SELECT_SEQ, aDefaultSelectSeqAny);

        
        if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
        {
            sal_Int16 nValue;
            _rxInStream >> nValue;
            m_aBoundColumn <<= nValue;
        }
        else 
             
        {
            m_aBoundColumn = Any();
        }

        if (nVersion > 2)
            readHelpTextCompatibly(_rxInStream);

        
        
        if  (   ( m_eListSourceType != ListSourceType_VALUELIST )
            &&  !hasExternalListSource()
            )
        {
            setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( StringSequence() ) );
        }

        if (nVersion > 3)
            readCommonProperties(_rxInStream);

        
        if ( !getControlSource().isEmpty() )
            
            resetNoBroadcast();
    }

    
    void OListBoxModel::loadData( bool _bForce )
    {
        SAL_INFO( "forms.component", "OListBoxModel::loadData" );
        DBG_ASSERT( m_eListSourceType != ListSourceType_VALUELIST, "OListBoxModel::loadData: cannot load value list from DB!" );
        DBG_ASSERT( !hasExternalListSource(), "OListBoxModel::loadData: cannot load from DB when I have an external list source!" );

        const sal_Int16 nNULLPosBackup( m_nNULLPos );
        const sal_Int32 nBoundColumnTypeBackup( m_nBoundColumnType );
        m_nNULLPos = -1;
        m_nBoundColumnType = DataType::SQLNULL;

        
        
        Reference< XConnection > xConnection;
        
        Reference< XPropertySet > xFormProps( m_xCursor, UNO_QUERY );
        if ( xFormProps.is() )
            xFormProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;

        
        OUString sListSource;
        
        
        ::std::for_each(
            m_aListSourceValues.begin(),
            m_aListSourceValues.end(),
            AppendRowSetValueString( sListSource )
        );

        
        if ( !xConnection.is() || sListSource.isEmpty() )
        {
            clearBoundValues();
            return;
        }

        ::boost::optional< sal_Int16 > aBoundColumn(boost::none);
        if ( m_aBoundColumn.getValueType().getTypeClass() == TypeClass_SHORT )
        {
            sal_Int16 nBoundColumn( 0 );
            m_aBoundColumn >>= nBoundColumn;
            aBoundColumn.reset( nBoundColumn );
        }

        ::utl::SharedUNOComponent< XResultSet > xListCursor;
        try
        {
            m_aListRowSet.setConnection( xConnection );

            sal_Bool bExecute = sal_False;
            switch (m_eListSourceType)
            {
            case ListSourceType_TABLEFIELDS:
                
                break;

            case ListSourceType_TABLE:
                {
                    Reference<XNameAccess> xFieldsByName = getTableFields(xConnection, sListSource);
                    Reference<XIndexAccess> xFieldsByIndex(xFieldsByName, UNO_QUERY);

                    
                    
                    OUString aFieldName;
                    OUString aBoundFieldName;

                    if ( !!aBoundColumn && ( *aBoundColumn >= 0 ) && xFieldsByIndex.is() )
                    {
                        if ( *aBoundColumn >= xFieldsByIndex->getCount() )
                            break;

                        Reference<XPropertySet> xFieldAsSet(xFieldsByIndex->getByIndex( *aBoundColumn ),UNO_QUERY);
                        assert(xFieldAsSet.is());
                        xFieldAsSet->getPropertyValue(PROPERTY_NAME) >>= aBoundFieldName;
                        aBoundColumn.reset( 1 );

                        xFieldAsSet.set(xFieldsByIndex->getByIndex(0),UNO_QUERY);
                        xFieldAsSet->getPropertyValue(PROPERTY_NAME) >>= aFieldName;
                    }
                    else if (xFieldsByName.is())
                    {
                        if ( xFieldsByName->hasByName( getControlSource() ) )
                            aFieldName = getControlSource();
                        else
                        {
                            
                            Reference< XColumnsSupplier > xSupplyFields;
                            xFormProps->getPropertyValue("SingleSelectQueryComposer") >>= xSupplyFields;

                            
                            DBG_ASSERT(xSupplyFields.is(), "OListBoxModel::loadData : invalid query composer !");

                            Reference<XNameAccess> xFieldNames = xSupplyFields->getColumns();
                            if ( xFieldNames->hasByName( getControlSource() ) )
                            {
                                Reference<XPropertySet> xComposerFieldAsSet;
                                xFieldNames->getByName( getControlSource() ) >>= xComposerFieldAsSet;
                                if (hasProperty(PROPERTY_FIELDSOURCE, xComposerFieldAsSet))
                                    xComposerFieldAsSet->getPropertyValue(PROPERTY_FIELDSOURCE) >>= aFieldName;
                            }
                        }
                    }
                    if (aFieldName.isEmpty())
                        break;

                    Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
                    OUString aQuote = xMeta->getIdentifierQuoteString();
                    OUString aStatement("SELECT ");
                    if (aBoundFieldName.isEmpty())   
                        aStatement += "DISTINCT ";

                    aStatement += quoteName(aQuote,aFieldName);
                    if (!aBoundFieldName.isEmpty())
                    {
                        aStatement += ", ";
                        aStatement += quoteName(aQuote, aBoundFieldName);
                    }
                    aStatement += " FROM ";

                    OUString sCatalog, sSchema, sTable;
                    qualifiedNameComponents( xMeta, sListSource, sCatalog, sSchema, sTable, eInDataManipulation );
                    aStatement += composeTableNameForSelect( xConnection, sCatalog, sSchema, sTable );

                    m_aListRowSet.setEscapeProcessing( sal_False );
                    m_aListRowSet.setCommand( aStatement );
                    bExecute = sal_True;
                }
                break;

            case ListSourceType_QUERY:
                m_aListRowSet.setCommandFromQuery( sListSource );
                bExecute = sal_True;
                break;

            default:
                m_aListRowSet.setEscapeProcessing( ListSourceType_SQLPASSTHROUGH != m_eListSourceType );
                m_aListRowSet.setCommand( sListSource );
                bExecute = sal_True;
                break;
            }

            if (bExecute)
            {
                if ( !_bForce && !m_aListRowSet.isDirty() )
                {
                    
                    
                    
                    m_nNULLPos = nNULLPosBackup;
                    m_nBoundColumnType = nBoundColumnTypeBackup;
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
        catch(const Exception& eUnknown)
        {
            (void)eUnknown;
            return;
        }

        
        ValueList aDisplayList, aValueList;
        sal_Bool bUseNULL = hasField() && !isRequired();

        
        if(!aBoundColumn)
            aBoundColumn = 0;

        try
        {
            OSL_ENSURE( xListCursor.is() || ( ListSourceType_TABLEFIELDS == m_eListSourceType ),
                "OListBoxModel::loadData: logic error!" );
            if ( !xListCursor.is() && ( ListSourceType_TABLEFIELDS != m_eListSourceType ) )
                return;

            switch (m_eListSourceType)
            {
            case ListSourceType_SQL:
            case ListSourceType_SQLPASSTHROUGH:
            case ListSourceType_TABLE:
            case ListSourceType_QUERY:
                {
                    
                    Reference<XColumnsSupplier> xSupplyCols(xListCursor, UNO_QUERY);
                    DBG_ASSERT(xSupplyCols.is(), "OListBoxModel::loadData : cursor supports the row set service but is no column supplier?!");
                    Reference<XIndexAccess> xColumns;
                    if (xSupplyCols.is())
                    {
                        xColumns = Reference<XIndexAccess>(xSupplyCols->getColumns(), UNO_QUERY);
                        DBG_ASSERT(xColumns.is(), "OListBoxModel::loadData : no columns supplied by the row set !");
                    }

                    Reference< XPropertySet > xDataField;
                    if ( xColumns.is() )
                        xColumns->getByIndex(0) >>= xDataField;
                    if ( !xDataField.is() )
                        return;

                    ::dbtools::FormattedColumnValue aValueFormatter( getContext(), m_xCursor, xDataField );

                    
                    m_nBoundColumnType = DataType::SQLNULL;
                    if ( *aBoundColumn >= 0 )
                    {
                        try
                        {
                            Reference< XPropertySet > xBoundField( xColumns->getByIndex( *aBoundColumn ), UNO_QUERY_THROW );
                            OSL_VERIFY( xBoundField->getPropertyValue("Type") >>= m_nBoundColumnType );
                        }
                        catch( const Exception& )
                        {
                            DBG_UNHANDLED_EXCEPTION();
                        }
                    }
                    else if ( *aBoundColumn == -1)
                        m_nBoundColumnType = DataType::SMALLINT;

                    
                    
                    SAL_INFO( "forms.component", "OListBoxModel::loadData: string collection" );
                    OUString aStr;
                    sal_Int16 entryPos = 0;
                    ORowSetValue aBoundValue;
                    Reference< XRow > xCursorRow( xListCursor, UNO_QUERY_THROW );
                    while ( xListCursor->next() && ( entryPos++ < SHRT_MAX ) ) 
                    {
                        aStr = aValueFormatter.getFormattedValue();
                        aDisplayList.push_back( aStr );

                        if(*aBoundColumn >= 0)
                            aBoundValue.fill( *aBoundColumn + 1, m_nBoundColumnType, xCursorRow );
                        else
                            
                            aBoundValue = static_cast<sal_Int16>(xListCursor->getRow()-1);
                        aValueList.push_back( aBoundValue );

                        if ( m_nNULLPos == -1 && aBoundValue.isNull() )
                            m_nNULLPos = sal_Int16( aDisplayList.size() - 1 );
                        if ( bUseNULL && ( m_nNULLPos == -1 ) && aStr.isEmpty() )
                            
                            
                            
                            bUseNULL = false;
                    }
                }
                break;

            case ListSourceType_TABLEFIELDS:
                {
                    Reference<XNameAccess> xFieldNames = getTableFields(xConnection, sListSource);
                    if (xFieldNames.is())
                    {
                        StringSequence seqNames = xFieldNames->getElementNames();
                        ::std::copy(
                            seqNames.getConstArray(),
                            seqNames.getConstArray() + seqNames.getLength(),
                            ::std::insert_iterator< ValueList >( aDisplayList, aDisplayList.end() )
                        );
                        if(*aBoundColumn == -1)
                        {
                            
                            for(sal_Int16 i=0; static_cast<ValueList::size_type>(i) < aDisplayList.size(); ++i)
                            {
                                aValueList.push_back(i);
                            }
                        }
                        else
                        {
                            aValueList = aDisplayList;
                        }
                    }
                }
                break;
            default:
                SAL_WARN( "forms.component", "OListBoxModel::loadData: unreachable!" );
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


        
        
        if (bUseNULL && m_nNULLPos == -1)
        {
            aValueList.insert( aValueList.begin(), ORowSetValue() );

            aDisplayList.insert( aDisplayList.begin(), ORowSetValue( OUString() ) );
            m_nNULLPos = 0;
        }

        setBoundValues(aValueList);

        setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( lcl_convertToStringSequence( aDisplayList ) ) );
    }

    
    void OListBoxModel::onConnectedDbColumn( const Reference< XInterface >& /*_rxForm*/ )
    {
        
        
        if ( hasField() )
        {
            setFastPropertyValue( PROPERTY_ID_MULTISELECTION, ::cppu::bool2any( false ) );
        }

        if ( !hasExternalListSource() )
            impl_refreshDbEntryList( false );
    }

    
    void OListBoxModel::onDisconnectedDbColumn()
    {
        if ( m_eListSourceType != ListSourceType_VALUELIST )
        {
            clearBoundValues();
            m_nNULLPos = -1;
            m_nBoundColumnType = DataType::SQLNULL;

            if ( !hasExternalListSource() )
                setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( StringSequence() ) );

            m_aListRowSet.dispose();
        }
    }

    
    void OListBoxModel::setBoundValues(const ValueList &l)
    {
        m_aConvertedBoundValues.clear();
        m_aBoundValues = l;
    }

    
    void OListBoxModel::clearBoundValues()
    {
        ValueList().swap(m_aConvertedBoundValues);
        ValueList().swap(m_aBoundValues);
    }

    
    void OListBoxModel::convertBoundValues(const sal_Int32 nFieldType) const
    {
        m_aConvertedBoundValues.resize(m_aBoundValues.size());
        ValueList::const_iterator src = m_aBoundValues.begin();
        const ValueList::const_iterator end = m_aBoundValues.end();
        ValueList::iterator dst = m_aConvertedBoundValues.begin();
        for (; src != end; ++src, ++dst )
        {
            *dst = *src;
            dst->setTypeKind(nFieldType);
        }
        m_nConvertedBoundValuesType = nFieldType;
        OSL_ENSURE(dst == m_aConvertedBoundValues.end(), "OListBoxModel::convertBoundValues expected to have overwritten all of m_aConvertedBoundValues, but did not.");
        assert(dst == m_aConvertedBoundValues.end());
    }
    
    sal_Int32 OListBoxModel::getValueType() const
    {
        return impl_hasBoundComponent() ? m_nBoundColumnType : getFieldType();
    }
    
    ValueList OListBoxModel::impl_getValues() const
    {
        const sal_Int32 nFieldType = getValueType();

        if ( !m_aConvertedBoundValues.empty() && m_nConvertedBoundValuesType == nFieldType )
            return m_aConvertedBoundValues;

        if ( !m_aBoundValues.empty() )
        {
            convertBoundValues(nFieldType);
            return m_aConvertedBoundValues;
        }

        Sequence< OUString > aStringItems( getStringItemList() );
        ValueList aValues( aStringItems.getLength() );
        ValueList::iterator dst = aValues.begin();
        const OUString *src (aStringItems.getConstArray());
        const OUString * const end = src + aStringItems.getLength();
        for (; src < end; ++src, ++dst )
        {
            *dst = *src;
            dst->setTypeKind(nFieldType);
        }
        m_nConvertedBoundValuesType = nFieldType;
        OSL_ENSURE(dst == aValues.end(), "OListBoxModel::impl_getValues expected to have set all of aValues, but did not.");
        assert(dst == aValues.end());
        return aValues;
    }
    
    ORowSetValue OListBoxModel::getFirstSelectedValue() const
    {
        static const ORowSetValue s_aEmptyVaue;

        DBG_ASSERT( m_xAggregateFastSet.is(), "OListBoxModel::getFirstSelectedValue: invalid aggregate!" );
        if ( !m_xAggregateFastSet.is() )
            return s_aEmptyVaue;

        Sequence< sal_Int16 > aSelectedIndices;
        OSL_VERIFY( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) >>= aSelectedIndices );
        if ( !aSelectedIndices.getLength() )
            
            return s_aEmptyVaue;

        if ( ( m_nNULLPos != -1 ) && ( aSelectedIndices[0] == m_nNULLPos ) )
            
            return s_aEmptyVaue;

        ValueList aValues( impl_getValues() );

        size_t selectedValue = aSelectedIndices[0];
        if ( selectedValue >= aValues.size() )
        {
            SAL_WARN( "forms.component", "OListBoxModel::getFirstSelectedValue: inconsistent selection/valuelist!" );
            return s_aEmptyVaue;
        }

        return aValues[ selectedValue ];
    }

    
    sal_Bool OListBoxModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
    {
        
        const ORowSetValue aCurrentValue( getFirstSelectedValue() );
        if ( aCurrentValue != m_aSaveValue )
        {
            if ( aCurrentValue.isNull() )
                m_xColumnUpdate->updateNull();
            else
            {
                try
                {
                    m_xColumnUpdate->updateObject( aCurrentValue.makeAny() );
                }
                catch ( const Exception& )
                {
                    return sal_False;
                }
            }
            m_aSaveValue = aCurrentValue;
        }
        return sal_True;
    }

    
    Sequence< sal_Int16 > OListBoxModel::translateDbValueToControlValue(const ORowSetValue &i_aValue) const
    {
        Sequence< sal_Int16 > aSelectionIndicies;

        
        if ( i_aValue.isNull() )
        {
            if ( m_nNULLPos != -1 )
            {
                aSelectionIndicies.realloc(1);
                aSelectionIndicies[0] = m_nNULLPos;
            }
        }
        else
        {
            ValueList aValues( impl_getValues() );
            assert( m_nConvertedBoundValuesType == getValueType());
            ORowSetValue v(i_aValue);
            v.setTypeKind( m_nConvertedBoundValuesType );
            ValueList::const_iterator curValuePos = ::std::find( aValues.begin(), aValues.end(), v );
            if ( curValuePos != aValues.end() )
            {
                aSelectionIndicies.realloc( 1 );
                aSelectionIndicies[0] = curValuePos - aValues.begin();
            }
        }

        return aSelectionIndicies;
    }
    
    Sequence< sal_Int16 > OListBoxModel::translateBindingValuesToControlValue(const Sequence< const Any > &i_aValues) const
    {
        const ValueList aValues( impl_getValues() );
        assert( m_nConvertedBoundValuesType == getValueType());
        Sequence< sal_Int16 > aSelectionIndicies(i_aValues.getLength());
        sal_Int32 nCount(0);

        sal_Int16 *pIndex = aSelectionIndicies.getArray();
        const Any *pValue = i_aValues.getConstArray();
        const Any * const pValueEnd = i_aValues.getConstArray() + i_aValues.getLength();
        for (;pValue < pValueEnd; ++pValue)
        {
            if ( pValue->hasValue() )
            {
                ORowSetValue v;
                v.fill(*pValue);
                v.setTypeKind( m_nConvertedBoundValuesType );
                ValueList::const_iterator curValuePos = ::std::find( aValues.begin(), aValues.end(), v );
                if ( curValuePos != aValues.end() )
                {
                    *pIndex = curValuePos - aValues.begin();
                    ++pIndex;
                    ++nCount;
                }
            }
            else
            {
                if ( m_nNULLPos != -1 )
                {
                    *pIndex = m_nNULLPos;
                    ++pIndex;
                    ++nCount;
                }
            }
        }
        assert(aSelectionIndicies.getArray() + nCount == pIndex);
        aSelectionIndicies.realloc(nCount);
        return aSelectionIndicies;
    }
    
    Any OListBoxModel::translateDbColumnToControlValue()
    {
        Reference< XPropertySet > xBoundField( getField() );
        if ( !xBoundField.is() )
        {
            SAL_WARN( "forms.component", "OListBoxModel::translateDbColumnToControlValue: no field? How could that happen?!" );
            return Any();
        }

        ORowSetValue aCurrentValue;
        aCurrentValue.fill( getValueType(), m_xColumn );

        m_aSaveValue = aCurrentValue;

        return makeAny( translateDbValueToControlValue(aCurrentValue) );
    }

    
    
    Any OListBoxModel::getDefaultForReset() const
    {
        Any aValue;
        if (m_aDefaultSelectSeq.getLength())
            aValue <<= m_aDefaultSelectSeq;
        else if (m_nNULLPos != -1)  
        {
            Sequence<sal_Int16> aSeq(1);
            aSeq.getArray()[0] = m_nNULLPos;
            aValue <<= aSeq;
        }
        else
        {
            Sequence<sal_Int16> aSeq;
            aValue <<= aSeq;
        }

        return aValue;
    }

    
    void OListBoxModel::resetNoBroadcast()
    {
        OBoundControlModel::resetNoBroadcast();
        m_aSaveValue.setNull();
    }

    
    void SAL_CALL OListBoxModel::disposing( const EventObject& _rSource ) throw ( RuntimeException )
    {
        if ( !OEntryListHelper::handleDisposing( _rSource ) )
            OBoundControlModel::disposing( _rSource );
    }

    
    namespace
    {
        
        enum ExchangeType
        {
            eIndexList,     
            eIndex,         
            eEntryList,     
            eEntry,         
            eValueList,     
            eValue          
        };

        
        ExchangeType lcl_getCurrentExchangeType( const Type& _rExchangeType )
        {
            switch ( _rExchangeType.getTypeClass() )
            {
            case TypeClass_ANY:
                return eValue;
            case TypeClass_STRING:
                return eEntry;
            case TypeClass_LONG:
                return eIndex;
            case TypeClass_SEQUENCE:
            {
                Type aElementType = ::comphelper::getSequenceElementType( _rExchangeType );
                switch ( aElementType.getTypeClass() )
                {
                case TypeClass_ANY:
                    return eValueList;
                case TypeClass_STRING:
                    return eEntryList;
                case TypeClass_LONG:
                    return eIndexList;
                default:
                    break;
                }
            }
            default:
                break;
            }
            SAL_WARN( "forms.component", "lcl_getCurrentExchangeType: unsupported (unexpected) exchange type!" );
            return eEntry;
        }
    }

    
    Any OListBoxModel::translateExternalValueToControlValue( const Any& _rExternalValue ) const
    {
        Sequence< sal_Int16 > aSelectIndexes;

        switch ( lcl_getCurrentExchangeType( getExternalValueType() ) )
        {
        case eValueList:
        {
            Sequence< const Any > aExternalValues;
            OSL_VERIFY( _rExternalValue >>= aExternalValues );
            aSelectIndexes = translateBindingValuesToControlValue( aExternalValues );
        }
        break;

        case eValue:
        {
            ORowSetValue v;
            v.fill(_rExternalValue);
            aSelectIndexes = translateDbValueToControlValue(v);
        }
        break;

        case eIndexList:
        {
            
            
            Sequence< sal_Int32 > aSelectIndexesPure;
            OSL_VERIFY( _rExternalValue >>= aSelectIndexesPure );
            aSelectIndexes.realloc( aSelectIndexesPure.getLength() );
            ::std::copy(
                aSelectIndexesPure.getConstArray(),
                aSelectIndexesPure.getConstArray() + aSelectIndexesPure.getLength(),
                aSelectIndexes.getArray()
            );
        }
        break;

        case eIndex:
        {
            sal_Int32 nSelectIndex = -1;
            OSL_VERIFY( _rExternalValue >>= nSelectIndex );
            if ( ( nSelectIndex >= 0 ) && ( nSelectIndex < getStringItemList().getLength() ) )
            {
                aSelectIndexes.realloc( 1 );
                aSelectIndexes[ 0 ] = static_cast< sal_Int16 >( nSelectIndex );
            }
        }
        break;

        case eEntryList:
        {
            
            Sequence< OUString > aSelectEntries;
            OSL_VERIFY( _rExternalValue >>= aSelectEntries );

            ::std::set< sal_Int16 > aSelectionSet;

            
            const OUString* pSelectEntries = aSelectEntries.getArray();
            const OUString* pSelectEntriesEnd = pSelectEntries + aSelectEntries.getLength();
            while ( pSelectEntries != pSelectEntriesEnd )
            {
                
                Sequence< sal_Int16 > aThisEntryIndexes;
                aThisEntryIndexes = findValue( getStringItemList(), *pSelectEntries++, false );

                
                ::std::copy(
                    aThisEntryIndexes.getConstArray(),
                    aThisEntryIndexes.getConstArray() + aThisEntryIndexes.getLength(),
                    ::std::insert_iterator< ::std::set< sal_Int16 > >( aSelectionSet, aSelectionSet.begin() )
                );
            }

            
            aSelectIndexes.realloc( aSelectionSet.size() );
            ::std::copy(
                aSelectionSet.begin(),
                aSelectionSet.end(),
                aSelectIndexes.getArray()
            );
        }
        break;

        case eEntry:
        {
            OUString sStringToSelect;
            OSL_VERIFY( _rExternalValue >>= sStringToSelect );

            aSelectIndexes = findValue( getStringItemList(), sStringToSelect, false );
        }
        break;
        }

        return makeAny( aSelectIndexes );
    }

    
    namespace
    {
        
        struct ExtractStringFromSequence_Safe : public ::std::unary_function< sal_Int16, OUString >
        {
        protected:
            const Sequence< OUString >&  m_rList;

        public:
            ExtractStringFromSequence_Safe( const Sequence< OUString >& _rList ) : m_rList( _rList ) { }

            OUString operator ()( sal_Int16 _nIndex )
            {
                OSL_ENSURE( _nIndex < m_rList.getLength(), "ExtractStringFromSequence_Safe: inconsistence!" );
                if ( _nIndex < m_rList.getLength() )
                    return m_rList[ _nIndex ];
                return OUString();
            }
        };

        
        Any lcl_getSingleSelectedEntry( const Sequence< sal_Int16 >& _rSelectSequence, const Sequence< OUString >& _rStringList )
        {
            Any aReturn;

            
            
            if ( _rSelectSequence.getLength() <= 1 )
            {
                OUString sSelectedEntry;

                if ( _rSelectSequence.getLength() == 1 )
                    sSelectedEntry = ExtractStringFromSequence_Safe( _rStringList )( _rSelectSequence[0] );

                aReturn <<= sSelectedEntry;
            }

            return aReturn;
        }

        
        Any lcl_getMultiSelectedEntries( const Sequence< sal_Int16 >& _rSelectSequence, const Sequence< OUString >& _rStringList )
        {
            Sequence< OUString > aSelectedEntriesTexts( _rSelectSequence.getLength() );
            ::std::transform(
                _rSelectSequence.getConstArray(),
                _rSelectSequence.getConstArray() + _rSelectSequence.getLength(),
                aSelectedEntriesTexts.getArray(),
                ExtractStringFromSequence_Safe( _rStringList )
            );
            return makeAny( aSelectedEntriesTexts );
        }

        
        struct ExtractAnyFromValueList_Safe : public ::std::unary_function< sal_Int16, Any >
        {
        protected:
            const ValueList&  m_rList;

        public:
            ExtractAnyFromValueList_Safe( const ValueList& _rList ) : m_rList( _rList ) { }

            Any operator ()( sal_Int16 _nIndex )
            {
                OSL_ENSURE( static_cast<ValueList::size_type>(_nIndex) < m_rList.size(), "ExtractAnyFromValueList: inconsistence!" );
                if ( static_cast<ValueList::size_type>(_nIndex) < m_rList.size() )
                    return m_rList[ _nIndex ].makeAny();
                return Any();
            }
        };

        
        Any lcl_getSingleSelectedEntryAny( const Sequence< sal_Int16 >& _rSelectSequence, const ValueList& _rStringList )
        {
            Any aReturn;

            
            
            if ( _rSelectSequence.getLength() <= 1 )
            {
                if ( _rSelectSequence.getLength() == 1 )
                    aReturn = ExtractAnyFromValueList_Safe( _rStringList )( _rSelectSequence[0] );
            }

            return aReturn;
        }

        
        Any lcl_getMultiSelectedEntriesAny( const Sequence< sal_Int16 >& _rSelectSequence, const ValueList& _rStringList )
        {
            Sequence< Any > aSelectedEntriesValues( _rSelectSequence.getLength() );
            ::std::transform(
                _rSelectSequence.getConstArray(),
                _rSelectSequence.getConstArray() + _rSelectSequence.getLength(),
                aSelectedEntriesValues.getArray(),
                ExtractAnyFromValueList_Safe( _rStringList )
            );
            return makeAny( aSelectedEntriesValues );
        }
    }

    
    Any OListBoxModel::translateControlValueToExternalValue( ) const
    {
        OSL_PRECOND( hasExternalValueBinding(), "OListBoxModel::translateControlValueToExternalValue: no binding!" );

        Sequence< sal_Int16 > aSelectSequence;
        OSL_VERIFY( getControlValue() >>= aSelectSequence );

        Any aReturn;
        switch ( lcl_getCurrentExchangeType( getExternalValueType() ) )
        {
        case eValueList:
            aReturn = getCurrentMultiValue();
            break;

        case eValue:
            aReturn = getCurrentSingleValue();
            break;

        case eIndexList:
        {
            
            
            Sequence< sal_Int32 > aTransformed( aSelectSequence.getLength() );
            ::std::copy(
                aSelectSequence.getConstArray(),
                aSelectSequence.getConstArray() + aSelectSequence.getLength(),
                aTransformed.getArray()
            );
            aReturn <<= aTransformed;
        }
        break;

        case eIndex:
        if ( aSelectSequence.getLength() <= 1 )
        {
            sal_Int32 nIndex = -1;

            if ( aSelectSequence.getLength() == 1 )
                nIndex = aSelectSequence[0];

            aReturn <<= nIndex;
        }
        break;

        case eEntryList:
            aReturn = lcl_getMultiSelectedEntries( aSelectSequence, getStringItemList() );
            break;

        case eEntry:
            aReturn = lcl_getSingleSelectedEntry( aSelectSequence, getStringItemList() );
            break;
        }

        return aReturn;
    }

    
    Any OListBoxModel::translateControlValueToValidatableValue( ) const
    {
        OSL_PRECOND( hasValidator(), "OListBoxModel::translateControlValueToValidatableValue: no validator, so why should I?" );
        return getCurrentFormComponentValue();
    }

    
    Any OListBoxModel::getCurrentSingleValue() const
    {
        Any aCurrentValue;

        try
        {
            Sequence< sal_Int16 > aSelectSequence;
            OSL_VERIFY( getControlValue() >>= aSelectSequence );
            aCurrentValue = lcl_getSingleSelectedEntryAny( aSelectSequence, impl_getValues() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return aCurrentValue;
    }
    
    Any OListBoxModel::getCurrentMultiValue() const
    {
        Any aCurrentValue;

        try
        {
            Sequence< sal_Int16 > aSelectSequence;
            OSL_VERIFY( getControlValue() >>= aSelectSequence );
            aCurrentValue = lcl_getMultiSelectedEntriesAny( aSelectSequence, impl_getValues() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return aCurrentValue;
    }
    
    Any OListBoxModel::getCurrentFormComponentValue() const
    {
        {
            Reference< com::sun::star::form::validation::XValidator > vtor (const_cast<OListBoxModel*>(this)->getValidator());
            Reference< XValueBinding > extBinding (const_cast<OListBoxModel*>(this)->getValueBinding());
            if ( vtor.is() && vtor == extBinding )
                return translateControlValueToExternalValue();
        }

        Any aCurrentValue;

        try
        {
            sal_Bool bMultiSelection( sal_False );
            OSL_VERIFY( const_cast< OListBoxModel* >( this )->getPropertyValue( PROPERTY_MULTISELECTION ) >>= bMultiSelection );

            if ( bMultiSelection )
                aCurrentValue = getCurrentMultiValue();
            else
                aCurrentValue = getCurrentSingleValue();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return aCurrentValue;
    }

    
    Sequence< Type > OListBoxModel::getSupportedBindingTypes()
    {
        Sequence< Type > aTypes(6);
        aTypes[0] = ::getCppuType( static_cast< Sequence< Any >* >( NULL ) );
        aTypes[1] = ::getCppuType( static_cast< Any* >( NULL ) );
        aTypes[2] = ::getCppuType( static_cast< Sequence< sal_Int32 >* >( NULL ) );
        aTypes[3] = ::getCppuType( static_cast< sal_Int32* >( NULL ) );
        aTypes[4] = ::getCppuType( static_cast< Sequence< OUString >* >( NULL ) );
        aTypes[5] = ::getCppuType( static_cast< OUString* >( NULL ) );
        return aTypes;
    }

    
    void OListBoxModel::stringItemListChanged( ControlModelLock& _rInstanceLock )
    {
        if ( !m_xAggregateSet.is() )
            return;

        suspendValueListening();
        try
        {
            m_xAggregateSet->setPropertyValue( PROPERTY_STRINGITEMLIST, makeAny( getStringItemList() ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        resumeValueListening();

        
        if ( hasExternalValueBinding( ) )
            transferExternalValueToControl( _rInstanceLock );
        else
        {
            if ( hasField() )
            {
                
            }
            else
            {
                if ( m_aDefaultSelectSeq.getLength() )
                    setControlValue( makeAny( m_aDefaultSelectSeq ), eOther );
            }
        }
    }

    
    void OListBoxModel::connectedExternalListSource( )
    {
        
    }

    
    void OListBoxModel::disconnectedExternalListSource( )
    {
        
        
        
    }

    
    void OListBoxModel::impl_refreshDbEntryList( bool _bForce )
    {
        DBG_ASSERT( !hasExternalListSource(), "OListBoxModel::impl_refreshDbEntryList: invalid call!" );

        if  (   !hasExternalListSource( )
            &&  ( m_eListSourceType != ListSourceType_VALUELIST )
            &&  ( m_xCursor.is() )
            )
        {
            loadData( _bForce );
        }
    }

    
    void OListBoxModel::refreshInternalEntryList()
    {
        impl_refreshDbEntryList( true );
        if ( hasField() && m_xCursor.is() )
            initFromField( m_xCursor );
    }

    
    
    

    
    InterfaceRef SAL_CALL OListBoxControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
    {
        return *(new OListBoxControl( comphelper::getComponentContext(_rxFactory) ));
    }

    
    Sequence< Type> OListBoxControl::_getTypes()
    {
        return TypeBag(
            OBoundControl::_getTypes(),
            OListBoxControl_BASE::getTypes()
        ).getTypes();
    }

    
    Any SAL_CALL OListBoxControl::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        Any aReturn = OListBoxControl_BASE::queryInterface( _rType );

        if  (   !aReturn.hasValue()
            ||  _rType.equals( cppu::UnoType<XTypeProvider>::get() )
            )
            aReturn = OBoundControl::queryAggregation( _rType );

        return aReturn;
    }

    
    OListBoxControl::OListBoxControl(const Reference<XComponentContext>& _rxFactory)
        :OBoundControl( _rxFactory, VCL_CONTROL_LISTBOX, sal_False )
        ,m_aChangeListeners( m_aMutex )
        ,m_aItemListeners( m_aMutex )
    {

        increment(m_refCount);
        {
            
            Reference<XWindow> xComp;
            if (query_aggregation(m_xAggregate, xComp))
                xComp->addFocusListener(this);

            
            if ( query_aggregation( m_xAggregate, m_xAggregateListBox ) )
                m_xAggregateListBox->addItemListener(this);
        }
        
        decrement(m_refCount);

        doSetDelegator();

        m_aChangeTimer.SetTimeout(500);
        m_aChangeTimer.SetTimeoutHdl(LINK(this,OListBoxControl,OnTimeout));
    }

    
    OListBoxControl::~OListBoxControl()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }

        doResetDelegator();
        m_xAggregateListBox.clear();

    }

    
    StringSequence SAL_CALL OListBoxControl::getSupportedServiceNames() throw(RuntimeException)
    {
        StringSequence aSupported = OBoundControl::getSupportedServiceNames();
        aSupported.realloc(aSupported.getLength() + 1);

        OUString* pArray = aSupported.getArray();
        pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_LISTBOX;
        return aSupported;
    }


    
    
    void SAL_CALL OListBoxControl::focusGained(const FocusEvent& /*_rEvent*/) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( m_aChangeListeners.getLength() ) 
        {
            Reference<XPropertySet> xSet(getModel(), UNO_QUERY);
            if (xSet.is())
            {
                
                m_aCurrentSelection = xSet->getPropertyValue(PROPERTY_SELECT_SEQ);
            }
        }
    }

    
    void SAL_CALL OListBoxControl::focusLost(const FocusEvent& /*_rEvent*/) throw(RuntimeException)
    {
        m_aCurrentSelection.clear();
    }

    
    
    void SAL_CALL OListBoxControl::itemStateChanged(const ItemEvent& _rEvent) throw(RuntimeException)
    {
        
        Reference< XChild > xChild( getModel(), UNO_QUERY );
        if ( xChild.is() && xChild->getParent().is() )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_aItemListeners.getLength() )
            {
                if ( !m_pItemBroadcaster.is() )
                {
                    m_pItemBroadcaster.set(
                        new ::comphelper::AsyncEventNotifier("ListBox"));
                    m_pItemBroadcaster->launch();
                }
                m_pItemBroadcaster->addEvent( new ItemEventDescription( _rEvent ), this );
            }
        }
        else
            m_aItemListeners.notifyEach( &XItemListener::itemStateChanged, _rEvent );

        
        ::osl::ClearableMutexGuard aGuard(m_aMutex);
        if ( m_aChangeTimer.IsActive() )
        {
            Reference<XPropertySet> xSet(getModel(), UNO_QUERY);
            m_aCurrentSelection = xSet->getPropertyValue(PROPERTY_SELECT_SEQ);

            m_aChangeTimer.Stop();
            m_aChangeTimer.Start();
        }
        else
        {
            if ( m_aChangeListeners.getLength() && m_aCurrentSelection.hasValue() )
            {
                Reference<XPropertySet> xSet(getModel(), UNO_QUERY);
                if (xSet.is())
                {
                    
                    sal_Bool bModified(sal_False);
                    Any aValue = xSet->getPropertyValue(PROPERTY_SELECT_SEQ);

                    Sequence<sal_Int16>& rSelection = *(Sequence<sal_Int16> *)aValue.getValue();
                    Sequence<sal_Int16>& rOldSelection = *(Sequence<sal_Int16> *)m_aCurrentSelection.getValue();
                    sal_Int32 nLen = rSelection.getLength();
                    if (nLen != rOldSelection.getLength())
                        bModified = sal_True;
                    else
                    {
                        const sal_Int16* pVal = rSelection.getConstArray();
                        const sal_Int16* pCompVal = rOldSelection.getConstArray();

                        while (nLen-- && !bModified)
                            bModified = pVal[nLen] != pCompVal[nLen];
                    }

                    if (bModified)
                    {
                        m_aCurrentSelection = aValue;
                        m_aChangeTimer.Start();
                    }
                }
            }
            else if (m_aCurrentSelection.hasValue())
                m_aCurrentSelection.clear();
        }
    }

    
    
    void SAL_CALL OListBoxControl::disposing(const EventObject& _rSource) throw (RuntimeException)
    {
        OBoundControl::disposing(_rSource);
    }

    
    
    void SAL_CALL OListBoxControl::addChangeListener(const Reference<XChangeListener>& _rxListener) throw(RuntimeException)
    {
        m_aChangeListeners.addInterface( _rxListener );
    }

    
    void SAL_CALL OListBoxControl::removeChangeListener(const Reference<XChangeListener>& _rxListener) throw(RuntimeException)
    {
        m_aChangeListeners.removeInterface( _rxListener );
    }

    
    
    void OListBoxControl::disposing()
    {
        if (m_aChangeTimer.IsActive())
            m_aChangeTimer.Stop();

        EventObject aEvent( *this );
        m_aChangeListeners.disposeAndClear( aEvent );
        m_aItemListeners.disposeAndClear( aEvent );

        rtl::Reference< comphelper::AsyncEventNotifier > t;
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pItemBroadcaster.is() )
            {
                t = m_pItemBroadcaster;
                m_pItemBroadcaster->removeEventsForProcessor( this );
                m_pItemBroadcaster->terminate();
                m_pItemBroadcaster = NULL;
            }
        }
        if (t.is()) {
            t->join();
        }

        OBoundControl::disposing();
    }

    
    void OListBoxControl::processEvent( const AnyEvent& _rEvent )
    {
        Reference< XListBox > xKeepAlive( this );
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( OComponentHelper::rBHelper.bDisposed )
                return;
        }
        const ItemEventDescription& rItemEvent = static_cast< const ItemEventDescription& >( _rEvent );
        m_aItemListeners.notifyEach( &XItemListener::itemStateChanged, rItemEvent.getEventObject() );
    }

    
    IMPL_LINK(OListBoxControl, OnTimeout, void*, /*EMPTYTAG*/)
    {
        m_aChangeListeners.notifyEach( &XChangeListener::changed, EventObject( *this ) );
        return 0L;
    }

    
    void SAL_CALL OListBoxControl::addItemListener( const Reference< XItemListener >& l ) throw (RuntimeException)
    {
        m_aItemListeners.addInterface( l );
    }

    
    void SAL_CALL OListBoxControl::removeItemListener( const Reference< XItemListener >& l ) throw (RuntimeException)
    {
        m_aItemListeners.removeInterface( l );
    }

    
    void SAL_CALL OListBoxControl::addActionListener( const Reference< XActionListener >& l ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->addActionListener( l );
    }

    
    void SAL_CALL OListBoxControl::removeActionListener( const Reference< XActionListener >& l ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->removeActionListener( l );
    }

    
    void SAL_CALL OListBoxControl::addItem( const OUString& aItem, ::sal_Int16 nPos ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->addItem( aItem, nPos );
    }

    
    void SAL_CALL OListBoxControl::addItems( const Sequence< OUString >& aItems, ::sal_Int16 nPos ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->addItems( aItems, nPos );
    }

    
    void SAL_CALL OListBoxControl::removeItems( ::sal_Int16 nPos, ::sal_Int16 nCount ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->removeItems( nPos, nCount );
    }

    
    ::sal_Int16 SAL_CALL OListBoxControl::getItemCount(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getItemCount();
        return 0;
    }

    
    OUString SAL_CALL OListBoxControl::getItem( ::sal_Int16 nPos ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getItem( nPos );
        return OUString( );
    }

    
    Sequence< OUString > SAL_CALL OListBoxControl::getItems(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getItems();
        return Sequence< OUString >( );
    }

    
    ::sal_Int16 SAL_CALL OListBoxControl::getSelectedItemPos(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getSelectedItemPos();
        return 0;
    }

    
    Sequence< ::sal_Int16 > SAL_CALL OListBoxControl::getSelectedItemsPos(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getSelectedItemsPos();
        return Sequence< ::sal_Int16 >( );
    }

    
    OUString SAL_CALL OListBoxControl::getSelectedItem(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getSelectedItem();
        return OUString( );
    }

    
    Sequence< OUString > SAL_CALL OListBoxControl::getSelectedItems(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getSelectedItems();
        return Sequence< OUString >( );
    }

    
    void SAL_CALL OListBoxControl::selectItemPos( ::sal_Int16 nPos, ::sal_Bool bSelect ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->selectItemPos( nPos, bSelect );
    }

    
    void SAL_CALL OListBoxControl::selectItemsPos( const Sequence< ::sal_Int16 >& aPositions, ::sal_Bool bSelect ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->selectItemsPos( aPositions, bSelect );
    }

    
    void SAL_CALL OListBoxControl::selectItem( const OUString& aItem, ::sal_Bool bSelect ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->selectItem( aItem, bSelect );
    }

    
    ::sal_Bool SAL_CALL OListBoxControl::isMutipleMode(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->isMutipleMode();
        return sal_False;
    }

    
    void SAL_CALL OListBoxControl::setMultipleMode( ::sal_Bool bMulti ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->setMultipleMode( bMulti );
    }

    
    ::sal_Int16 SAL_CALL OListBoxControl::getDropDownLineCount(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getDropDownLineCount();
        return 0;
    }

    
    void SAL_CALL OListBoxControl::setDropDownLineCount( ::sal_Int16 nLines ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->setDropDownLineCount( nLines );
    }

    
    void SAL_CALL OListBoxControl::makeVisible( ::sal_Int16 nEntry ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->makeVisible( nEntry );
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
