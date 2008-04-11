/*************************************************************************
*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ListBox.cxx,v $
 * $Revision: 1.58 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"

#include "ListBox.hxx"
#include "property.hxx"
#include "property.hrc"
#include "services.hxx"
#include "frm_resource.hxx"
#include "frm_resource.hrc"
#include "BaseListBox.hxx"
#include "listenercontainers.hxx"
#include "componenttools.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
/** === end UNO includes === **/

#include <connectivity/dbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include <connectivity/dbconversion.hxx>

#include <vcl/svapp.hxx>

#include <unotools/sharedunocomponent.hxx>

#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>

#include <comphelper/basicio.hxx>
#include <comphelper/container.hxx>
#include <comphelper/numbers.hxx>
#include <comphelper/listenernotification.hxx>

#include <cppuhelper/queryinterface.hxx>

#include <rtl/logfile.hxx>

#include <algorithm>


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
    using namespace ::dbtools;

    //==================================================================
    //= ItemEvent
    //==================================================================
    typedef ::comphelper::EventHolder< ItemEvent >    ItemEventDescription;

    //==================================================================
    //= OListBoxModel
    //==================================================================
    //------------------------------------------------------------------
    InterfaceRef SAL_CALL OListBoxModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
    {
        return *(new OListBoxModel(_rxFactory));
    }

    //------------------------------------------------------------------------------
    Sequence< Type> OListBoxModel::_getTypes()
    {
        return TypeBag(
            OBoundControlModel::_getTypes(),
            OListBoxModel_BASE::getTypes(),
            OEntryListHelper::getTypes(),
            OErrorBroadcaster::getTypes()
        ).getTypes();
    }


    DBG_NAME(OListBoxModel);
    //------------------------------------------------------------------
    OListBoxModel::OListBoxModel(const Reference<XMultiServiceFactory>& _rxFactory)
        :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_LISTBOX, FRM_SUN_CONTROL_LISTBOX, sal_True, sal_True, sal_True )
        // use the old control name for compytibility reasons
        ,OEntryListHelper( m_aMutex )
        ,OErrorBroadcaster( OComponentHelper::rBHelper )
        ,m_aListRowSet( getContext() )
        ,m_aRefreshListeners(m_aMutex)
        ,m_nNULLPos(-1)
        ,m_bBoundComponent(sal_False)
        ,m_eTransferSelectionAs( tsEntry )
    {
        DBG_CTOR(OListBoxModel,NULL);

        m_nClassId = FormComponentType::LISTBOX;
        m_eListSourceType = ListSourceType_VALUELIST;
        m_aBoundColumn <<= (sal_Int16)1;
        initValueProperty( PROPERTY_SELECT_SEQ, PROPERTY_ID_SELECT_SEQ);
    }

    //------------------------------------------------------------------
    OListBoxModel::OListBoxModel( const OListBoxModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
        :OBoundControlModel( _pOriginal, _rxFactory )
        ,OEntryListHelper( *_pOriginal, m_aMutex )
        ,OErrorBroadcaster( OComponentHelper::rBHelper )
        ,m_aListRowSet( getContext() )
        ,m_eListSourceType( _pOriginal->m_eListSourceType )
        ,m_aBoundColumn( _pOriginal->m_aBoundColumn )
        ,m_aListSourceSeq( _pOriginal->m_aListSourceSeq )
        ,m_aValueSeq( _pOriginal->m_aValueSeq )
        ,m_aDefaultSelectSeq( _pOriginal->m_aDefaultSelectSeq )
        ,m_aRefreshListeners( m_aMutex )
        ,m_nNULLPos(-1)
        ,m_bBoundComponent(sal_False)
        ,m_eTransferSelectionAs( tsEntry )
    {
        DBG_CTOR(OListBoxModel,NULL);
    }

    //------------------------------------------------------------------
    OListBoxModel::~OListBoxModel()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }

        DBG_DTOR(OListBoxModel,NULL);
    }

    // XCloneable
    //------------------------------------------------------------------------------
    IMPLEMENT_DEFAULT_CLONING( OListBoxModel )

    // XServiceInfo
    //------------------------------------------------------------------------------
    StringSequence SAL_CALL OListBoxModel::getSupportedServiceNames() throw(RuntimeException)
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

        *pStoreTo++ = FRM_SUN_COMPONENT_LISTBOX;
        *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_LISTBOX;
        *pStoreTo++ = BINDABLE_DATABASE_LIST_BOX;

        return aSupported;
    }

    //------------------------------------------------------------------------------
    Any SAL_CALL OListBoxModel::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        Any aReturn = OBoundControlModel::queryAggregation( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OListBoxModel_BASE::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OEntryListHelper::queryInterface( _rType );
        if ( !aReturn.hasValue() )
            aReturn = OErrorBroadcaster::queryInterface( _rType );
        return aReturn;
    }

    // OComponentHelper
    //------------------------------------------------------------------------------
    void OListBoxModel::disposing()
    {
        EventObject aEvt( static_cast< XWeak* >( this ) );
        m_aRefreshListeners.disposeAndClear(aEvt);

        OBoundControlModel::disposing();
        OEntryListHelper::disposing();
        OErrorBroadcaster::disposing();
    }

    // XRefreshable
    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxModel::addRefreshListener(const Reference<XRefreshListener>& _rxListener) throw(RuntimeException)
    {
        m_aRefreshListeners.addInterface(_rxListener);
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxModel::removeRefreshListener(const Reference<XRefreshListener>& _rxListener) throw(RuntimeException)
    {
        m_aRefreshListeners.removeInterface(_rxListener);
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxModel::refresh() throw(RuntimeException)
    {
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( !hasExternalListSource() )
                implRefreshListFromDbBinding( );
        }

        EventObject aEvt(static_cast< XWeak*>(this));
        m_aRefreshListeners.notifyEach( &XRefreshListener::refreshed, aEvt );
    }

    //------------------------------------------------------------------------------
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
            _rValue <<= m_aListSourceSeq;
            break;

        case PROPERTY_ID_VALUE_SEQ:
            _rValue <<= m_aValueSeq;
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

    //------------------------------------------------------------------------------
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
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<ListSourceType*>(NULL))),
                "OComboBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_eListSourceType;
            break;

        case PROPERTY_ID_LISTSOURCE :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<StringSequence*>(NULL))),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aListSourceSeq;

            if (m_eListSourceType == ListSourceType_VALUELIST)
                m_aValueSeq = m_aListSourceSeq;
            else if ( m_xCursor.is() && !hasField() && !hasExternalListSource() )
                // listbox is already connected to a database, and no external list source
                // data source changed -> refresh
                loadData();
            break;

        case PROPERTY_ID_VALUE_SEQ :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast<StringSequence*>(NULL))),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aValueSeq;
            break;

        case PROPERTY_ID_DEFAULT_SELECT_SEQ :
            DBG_ASSERT(_rValue.getValueType().equals(::getCppuType(reinterpret_cast< Sequence<sal_Int16>*>(NULL))),
                "OListBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_aDefaultSelectSeq;

            DBG_ASSERT(m_xAggregateFastSet.is(), "OListBoxModel::setFastPropertyValue_NoBroadcast(DEFAULT_SELECT_SEQ) : invalid aggregate !");
            if ( m_xAggregateFastSet.is() )
                setControlValue( _rValue, eOther );
            break;

        case PROPERTY_ID_STRINGITEMLIST:
            setNewStringItemList( _rValue );
            resetNoBroadcast();
            break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
        }
    }

    //------------------------------------------------------------------------------
    sal_Bool OListBoxModel::convertFastPropertyValue(
        Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
        throw (IllegalArgumentException)
    {
        sal_Bool bModified(sal_False);
        switch (_nHandle)
        {
        case PROPERTY_ID_BOUNDCOLUMN :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aBoundColumn, ::getCppuType(reinterpret_cast<sal_Int16*>(NULL)));
            break;

        case PROPERTY_ID_LISTSOURCETYPE:
            bModified = tryPropertyValueEnum(_rConvertedValue, _rOldValue, _rValue, m_eListSourceType);
            break;

        case PROPERTY_ID_LISTSOURCE:
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aListSourceSeq);
            break;

        case PROPERTY_ID_VALUE_SEQ :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_aValueSeq);
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

    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxModel::setPropertyValues( const Sequence< ::rtl::OUString >& _rPropertyNames, const Sequence< Any >& _rValues ) throw(PropertyVetoException, IllegalArgumentException, WrappedTargetException, RuntimeException)
    {
        // if both SelectedItems and StringItemList are set, care for this
        // #i27024# / 2004-04-05 / fs@openoffice.org
        const Any* pSelectSequenceValue = NULL;

        const ::rtl::OUString* pStartPos = _rPropertyNames.getConstArray();
        const ::rtl::OUString* pEndPos   = _rPropertyNames.getConstArray() + _rPropertyNames.getLength();
        const ::rtl::OUString* pSelectedItemsPos = ::std::find_if(
            pStartPos, pEndPos,
             ::std::bind2nd( ::std::equal_to< ::rtl::OUString >(), PROPERTY_SELECT_SEQ )
        );
        const ::rtl::OUString* pStringItemListPos = ::std::find_if(
            pStartPos, pEndPos,
             ::std::bind2nd( ::std::equal_to< ::rtl::OUString >(), PROPERTY_STRINGITEMLIST )
        );
        if ( ( pSelectedItemsPos != pEndPos ) && ( pStringItemListPos != pEndPos ) )
        {
            // both properties are present
            // -> remember the value for the select sequence
            pSelectSequenceValue = _rValues.getConstArray() + ( pSelectedItemsPos - pStartPos );
        }

        OBoundControlModel::setPropertyValues( _rPropertyNames, _rValues );

        if ( pSelectSequenceValue )
        {
            setPropertyValue( PROPERTY_SELECT_SEQ, *pSelectSequenceValue );
            // Note that this is the only reliable way, since one of the properties is implemented
            // by ourself, and one is implemented by the aggregate, we cannot rely on any particular
            // results when setting them both - too many undocumented behavior in all the involved
            // classes
        }
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::describeFixedProperties( Sequence< Property >& _rProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( 7, OBoundControlModel )
            DECL_PROP1(TABINDEX,            sal_Int16,                      BOUND);
            DECL_PROP2(BOUNDCOLUMN,         sal_Int16,                      BOUND, MAYBEVOID);
            DECL_PROP1(LISTSOURCETYPE,      ListSourceType,                 BOUND);
            DECL_PROP1(LISTSOURCE,          StringSequence,                 BOUND);
            DECL_PROP3(VALUE_SEQ,           StringSequence,                 BOUND, READONLY, TRANSIENT);
            DECL_PROP1(DEFAULT_SELECT_SEQ,  Sequence<sal_Int16>,            BOUND);
            DECL_PROP1(STRINGITEMLIST,      Sequence< ::rtl::OUString >,    BOUND);
        END_DESCRIBE_PROPERTIES();
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
    {
        OBoundControlModel::describeAggregateProperties( _rAggregateProps );

        // superseded properties:
        RemoveProperty( _rAggregateProps, PROPERTY_STRINGITEMLIST );
    }

    //------------------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OListBoxModel::getServiceName() throw(RuntimeException)
    {
        return FRM_COMPONENT_LISTBOX;   // old (non-sun) name for compatibility !
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
        throw(IOException, RuntimeException)
    {
        OBoundControlModel::write(_rxOutStream);

        // Dummy-Seq, um Kompatible zu bleiben, wenn SelectSeq nicht mehr gespeichert wird
        Sequence<sal_Int16> aDummySeq;

        // Version
        // Version 0x0002: ListSource wird StringSeq
        _rxOutStream->writeShort(0x0004);

        // Maskierung fuer any
        sal_uInt16 nAnyMask = 0;
        if (m_aBoundColumn.getValueType().getTypeClass() != TypeClass_VOID)
            nAnyMask |= BOUNDCOLUMN;

        _rxOutStream << nAnyMask;

        _rxOutStream << m_aListSourceSeq;
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

        // from version 0x0004 : common properties
        writeCommonProperties(_rxOutStream);
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxModel::read(const Reference<XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException)
    {
        // Bei manchen Variblen muessen Abhaengigkeiten beruecksichtigt werden.
        // Deshalb muessen sie explizit ueber setPropertyValue() gesetzt werden.

        OBoundControlModel::read(_rxInStream);
        ::osl::MutexGuard aGuard(m_aMutex);

        // since we are "overwriting" the StringItemList of our aggregate (means we have
        // an own place to store the value, instead of relying on our aggregate storing it),
        // we need to respect what the aggregate just read for the StringItemList property.
        try
        {
            if ( m_xAggregateSet.is() )
                setNewStringItemList( m_xAggregateSet->getPropertyValue( PROPERTY_STRINGITEMLIST ) );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "OComboBoxModel::read: caught an exception while examining the aggregate's string item list!" );
        }

        // Version
        sal_uInt16 nVersion = _rxInStream->readShort();
        DBG_ASSERT(nVersion > 0, "OListBoxModel::read : version 0 ? this should never have been written !");

        if (nVersion > 0x0004)
        {
            DBG_ERROR("OListBoxModel::read : invalid (means unknown) version !");
            m_aListSourceSeq.realloc(0);
            m_aBoundColumn <<= (sal_Int16)0;
            m_aValueSeq.realloc(0);
            m_eListSourceType = ListSourceType_VALUELIST;
            m_aDefaultSelectSeq.realloc(0);
            defaultCommonProperties();
            return;
        }

        // Maskierung fuer any
        sal_uInt16 nAnyMask;
        _rxInStream >> nAnyMask;

        // ListSourceSeq
        StringSequence aListSourceSeq;
        if (nVersion == 0x0001)
        {
            // ListSourceSeq aus String zusammenstellen;
            ::rtl::OUString sListSource;
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

        // Dummy-Seq, um Kompatible zu bleiben, wenn SelectSeq nicht mehr gespeichert wird
        Sequence<sal_Int16> aDummySeq;
        _rxInStream >> aDummySeq;

        // DefaultSelectSeq
        Sequence<sal_Int16> aDefaultSelectSeq;
        _rxInStream >> aDefaultSelectSeq;
        Any aDefaultSelectSeqAny;
        aDefaultSelectSeqAny <<= aDefaultSelectSeq;
        setFastPropertyValue(PROPERTY_ID_DEFAULT_SELECT_SEQ, aDefaultSelectSeqAny);

        // BoundColumn
        if ((nAnyMask & BOUNDCOLUMN) == BOUNDCOLUMN)
        {
            sal_Int16 nValue;
            _rxInStream >> nValue;
            m_aBoundColumn <<= nValue;
        }

        if (nVersion > 2)
            readHelpTextCompatibly(_rxInStream);

        // if our string list is not filled from the value list, we must empty it
        // this can be the case when somebody saves in alive mode
        if  (   ( m_eListSourceType != ListSourceType_VALUELIST )
            &&  !hasExternalListSource()
            )
        {
            setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( StringSequence() ) );
        }

        if (nVersion > 3)
            readCommonProperties(_rxInStream);

        // Nach dem Lesen die Defaultwerte anzeigen
        if ( getControlSource().getLength() )
            // (not if we don't have a control source - the "State" property acts like it is persistent, then
            resetNoBroadcast();
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::loadData()
    {
        RTL_LOGFILE_CONTEXT( aLogContext, "OListBoxModel::loadData" );
        DBG_ASSERT( m_eListSourceType != ListSourceType_VALUELIST, "OListBoxModel::loadData: cannot load value list from DB!" );
        DBG_ASSERT( !hasExternalListSource(), "OListBoxModel::loadData: cannot load from DB when I have an external list source!" );

        m_nNULLPos = -1;
        m_bBoundComponent = sal_False;

        // pre-requisites:
        // PRE1: connection
        Reference< XConnection > xConnection;
        // is the active connection of our form
        Reference< XPropertySet > xFormProps( m_xCursor, UNO_QUERY );
        if ( xFormProps.is() )
            xFormProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) >>= xConnection;

        // PRE2: list source
        ::rtl::OUString sListSource;
        // if our list source type is no value list, we need to concatenete
        // the single list source elements
        const ::rtl::OUString* pListSourceItem = m_aListSourceSeq.getConstArray();
        sal_Int32 i(0);
        for ( i=0; i<m_aListSourceSeq.getLength(); ++i, ++pListSourceItem )
            sListSource = sListSource + *pListSourceItem;

        // outta here if we don't have all pre-requisites
        if ( !xConnection.is() || !sListSource.getLength() )
        {
            m_aValueSeq = StringSequence();
            return;
        }

        sal_Int16 nBoundColumn = 0;
        if (m_aBoundColumn.getValueType().getTypeClass() == TypeClass_SHORT)
            m_aBoundColumn >>= nBoundColumn;

        ::utl::SharedUNOComponent< XResultSet > xListCursor;
        try
        {
            m_aListRowSet.setConnection( xConnection );

            sal_Bool bExecute = sal_False;
            switch (m_eListSourceType)
            {
            case ListSourceType_TABLEFIELDS:
                // don't work with a statement here, the fields will be collected below
                break;

            case ListSourceType_TABLE:
                {
                    Reference<XNameAccess> xFieldsByName = getTableFields(xConnection, sListSource);
                    Reference<XIndexAccess> xFieldsByIndex(xFieldsByName, UNO_QUERY);

                    // do we have a bound column if yes we have to select it
                    // and the displayed column is the first column othwhise we act as a combobox
                    ::rtl::OUString aFieldName;
                    ::rtl::OUString aBoundFieldName;

                    if ((nBoundColumn > 0) && xFieldsByIndex.is())
                    {
                        if (xFieldsByIndex->getCount() <= nBoundColumn)
                            break;

                        Reference<XPropertySet> xFieldAsSet(xFieldsByIndex->getByIndex(nBoundColumn),UNO_QUERY);
                        xFieldAsSet->getPropertyValue(PROPERTY_NAME) >>= aBoundFieldName;
                        nBoundColumn = 1;

                        xFieldAsSet.set(xFieldsByIndex->getByIndex(0),UNO_QUERY);
                        xFieldAsSet->getPropertyValue(PROPERTY_NAME) >>= aFieldName;
                    }
                    else if (xFieldsByName.is())
                    {
                        if ( xFieldsByName->hasByName( getControlSource() ) )
                            aFieldName = getControlSource();
                        else
                        {
                            // otherwise look for the alias
                            Reference<XSQLQueryComposerFactory> xFactory(xConnection, UNO_QUERY);
                            if (!xFactory.is())
                                break;

                            Reference<XSQLQueryComposer> xComposer = xFactory->createQueryComposer();
                            try
                            {
                                ::rtl::OUString aStatement;
                                xFormProps->getPropertyValue( PROPERTY_ACTIVECOMMAND ) >>= aStatement;
                                xComposer->setQuery( aStatement );
                            }
                            catch(Exception&)
                            {
                                disposeComponent(xComposer);
                                break;
                            }

                            // search the field
                            Reference<XColumnsSupplier> xSupplyFields(xComposer, UNO_QUERY);
                            DBG_ASSERT(xSupplyFields.is(), "OListBoxModel::loadData : invalid query composer !");

                            Reference<XNameAccess> xFieldNames = xSupplyFields->getColumns();
                            if ( xFieldNames->hasByName( getControlSource() ) )
                            {
                                Reference<XPropertySet> xComposerFieldAsSet;
                                xFieldNames->getByName( getControlSource() ) >>= xComposerFieldAsSet;
                                if (hasProperty(PROPERTY_FIELDSOURCE, xComposerFieldAsSet))
                                    xComposerFieldAsSet->getPropertyValue(PROPERTY_FIELDSOURCE) >>= aFieldName;
                            }
                            disposeComponent(xComposer);
                        }
                    }
                    if (!aFieldName.getLength())
                        break;

                    Reference<XDatabaseMetaData> xMeta = xConnection->getMetaData();
                    ::rtl::OUString aQuote = xMeta->getIdentifierQuoteString();
                    ::rtl::OUString aStatement = ::rtl::OUString::createFromAscii("SELECT ");
                    if (!aBoundFieldName.getLength())   // act like a combobox
                        aStatement += ::rtl::OUString::createFromAscii("DISTINCT ");

                    aStatement += quoteName(aQuote,aFieldName);
                    if (aBoundFieldName.getLength())
                    {
                        aStatement += ::rtl::OUString::createFromAscii(", ");
                        aStatement += quoteName(aQuote, aBoundFieldName);
                    }
                    aStatement += ::rtl::OUString::createFromAscii(" FROM ");

                    ::rtl::OUString sCatalog, sSchema, sTable;
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
            }

            if (bExecute)
            {
                if ( !m_aListRowSet.isDirty() )
                {
                    // if none of the settings of the row set changed, compared to the last
                    // invocation of loadData, then don't re-fill the list. Instead, assume
                    // the list entries are the same.
                    return;
                }
                xListCursor.reset( m_aListRowSet.execute() );
            }
        }
        catch(SQLException& eSQL)
        {
            onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
            return;
        }
        catch(const Exception& eUnknown)
        {
            (void)eUnknown;
            return;
        }

        // Anzeige- und Werteliste fuellen
        ::std::vector< ::rtl::OUString >   aValueList, aStringList;
        aValueList.reserve(16);
        aStringList.reserve(16);
        sal_Bool bUseNULL = hasField() && !isRequired();

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
                    // Feld der 1. Column des ResultSets holen
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

                    // Feld der BoundColumn des ResultSets holen
                    Reference<XColumn> xBoundField;
                    if ((nBoundColumn > 0) && m_xColumn.is())
                        // don't look for a bound column if we're not connected to a field
                        xColumns->getByIndex(nBoundColumn) >>= xBoundField;
                    m_bBoundComponent = xBoundField.is();

                    //  Ist die LB an ein Feld gebunden und sind Leereintraege zulaessig
                    //  dann wird die Position fuer einen Leereintrag gemerkt

                    RTL_LOGFILE_CONTEXT( aLogContext, "OListBoxModel::loadData: string collection" );
                    ::rtl::OUString aStr;
                    sal_Int16 entryPos = 0;
                    // per definitionem the list cursor is positioned _before_ the first row at the moment
                    while ( xListCursor->next() && ( entryPos++ < SHRT_MAX ) ) // SHRT_MAX is the maximum number of entries
                    {
                        aStr = aValueFormatter.getFormattedValue();
                        aStringList.push_back(aStr);

                        if (m_bBoundComponent)
                        {
                            aStr = xBoundField->getString();
                            aValueList.push_back(aStr);
                        }

                        if (bUseNULL && (m_nNULLPos == -1) && !aStr.getLength())
                            m_nNULLPos = (sal_Int16)aStringList.size() - 1;
                    }
                }
                break;

            case ListSourceType_TABLEFIELDS:
                {
                    Reference<XNameAccess> xFieldNames = getTableFields(xConnection, sListSource);
                    if (xFieldNames.is())
                    {
                        StringSequence seqNames = xFieldNames->getElementNames();
                        sal_Int32 nFieldsCount = seqNames.getLength();
                        const ::rtl::OUString* pustrNames = seqNames.getConstArray();

                        for (sal_Int32 k=0; k<nFieldsCount; ++k, ++pustrNames)
                            aStringList.push_back(*pustrNames);
                    }
                }
                break;
                default:
                    OSL_ENSURE( false, "OListBoxModel::loadData: unreachable!" );
                    break;
            }
        }
        catch(SQLException& eSQL)
        {
            onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
            return;
        }
        catch( const Exception& eUnknown )
        {
            (void)eUnknown;
            return;
        }


        // Value-Sequence erzeugen
        // NULL eintrag hinzufuegen
        if (bUseNULL && m_nNULLPos == -1)
        {
            if (m_bBoundComponent)
                aValueList.insert(aValueList.begin(), ::rtl::OUString());

            aStringList.insert(aStringList.begin(), ::rtl::OUString());
            m_nNULLPos = 0;
        }

        m_aValueSeq.realloc(aValueList.size());
        ::rtl::OUString* pValues = m_aValueSeq.getArray();
        for ( i = 0; i < (sal_Int32)aValueList.size(); ++i, ++pValues)
            *pValues = aValueList[i];

        // String-Sequence fuer ListBox erzeugen
        StringSequence aStringSeq(aStringList.size());
        ::rtl::OUString* pStrings = aStringSeq.getArray();
        for ( i = 0; i < (sal_Int32)aStringList.size(); ++i, ++pStrings )
            *pStrings = aStringList[i];

        setFastPropertyValue(PROPERTY_ID_STRINGITEMLIST, makeAny(aStringSeq));
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::implRefreshListFromDbBinding( )
    {
        DBG_ASSERT( !hasExternalListSource( ), "OListBoxModel::implRefreshListFromDbBinding: invalid call!" );

        if ( m_eListSourceType != ListSourceType_VALUELIST )
        {
            if ( m_xCursor.is() )
                loadData();
        }
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::onConnectedDbColumn( const Reference< XInterface >& /*_rxForm*/ )
    {
        // list boxes which are bound to a db column don't have multi selection
        // - this would be unable to reflect in the db column
        if ( hasField() )
        {
            setFastPropertyValue( PROPERTY_ID_MULTISELECTION, ::cppu::bool2any( ( sal_False ) ) );
        }

        if ( !hasExternalListSource() )
            implRefreshListFromDbBinding( );
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::onDisconnectedDbColumn()
    {
        if (m_eListSourceType != ListSourceType_VALUELIST)
        {
            m_aValueSeq = StringSequence();
            m_nNULLPos = -1;
            m_bBoundComponent = sal_False;

            if ( !hasExternalListSource() )
                setFastPropertyValue( PROPERTY_ID_STRINGITEMLIST, makeAny( StringSequence() ) );

            m_aListRowSet.dispose();
        }
    }

    //------------------------------------------------------------------------------
    StringSequence OListBoxModel::GetCurValueSeq() const
    {
        StringSequence aCurValues;

        // Aus den selektierten Indizes Werte-Sequence aufbauen
        DBG_ASSERT(m_xAggregateFastSet.is(), "OListBoxModel::GetCurValueSeq : invalid aggregate !");
        if (!m_xAggregateFastSet.is())
            return aCurValues;

        Any aTmp = m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() );

        Sequence<sal_Int16> aSelectSeq; aTmp >>= aSelectSeq;
        const sal_Int16 *pSels = aSelectSeq.getConstArray();
        sal_uInt32 nSelCount = aSelectSeq.getLength();

        if (nSelCount)
        {
            StringSequence aValues( impl_getValues() );

            const ::rtl::OUString *pVals    = aValues.getConstArray();
            sal_Int32 nValCnt               = aValues.getLength();

            if (nSelCount > 1)
            {
                // Einfach- oder Mehrfach-Selektion
                sal_Bool bMultiSel = false;
                const_cast<OListBoxModel*>(this)->OPropertySetAggregationHelper::getFastPropertyValue(PROPERTY_ID_MULTISELECTION) >>= bMultiSel;
                if (bMultiSel)
                    nSelCount = 1;
            }

            // ist der Eintrag fuer NULL selektiert ?
            // dann leere Selektion liefern
            if (m_nNULLPos != -1 && nSelCount == 1 && pSels[0] == m_nNULLPos)
                nSelCount = 0;

            aCurValues.realloc(nSelCount);
            ::rtl::OUString *pCurVals = aCurValues.getArray();

            for (sal_uInt16 i = 0; i < nSelCount; i++)
            {
                if (pSels[i] < nValCnt)
                    pCurVals[i] = pVals[pSels[i]];
            }
        }
        return aCurValues;
    }

    //------------------------------------------------------------------------------
    sal_Bool OListBoxModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
    {
        // current selektion list
        Any aCurrentValue;
        StringSequence aCurValueSeq = GetCurValueSeq();
        if ( aCurValueSeq.getLength() )
            aCurrentValue <<= aCurValueSeq.getConstArray()[0];

        if ( !compare( aCurrentValue, m_aSaveValue ) )
        {
            if ( !aCurrentValue.hasValue() )
                m_xColumnUpdate->updateNull();
            else
            {
                try
                {
                    ::rtl::OUString sNewValue;
                    aCurrentValue >>= sNewValue;
                    m_xColumnUpdate->updateString( sNewValue );
                }
                catch(Exception&)
                {
                    return sal_False;
                }
            }
            m_aSaveValue = aCurrentValue;
        }
        return sal_True;
    }

    // XPropertiesChangeListener
    //------------------------------------------------------------------------------
    Any OListBoxModel::translateDbColumnToControlValue()
    {
        DBG_ASSERT( m_xAggregateFastSet.is() && m_xAggregateSet.is(), "OListBoxModel::translateDbColumnToControlValue: invalid aggregate !" );
        if ( !m_xAggregateFastSet.is() || !m_xAggregateSet.is() )
            return Any();

        Sequence<sal_Int16> aSelSeq;

        // Bei NULL-Eintraegen Selektion aufheben!
        ::rtl::OUString sValue = m_xColumn->getString();
        if (m_xColumn->wasNull())
        {
            m_aSaveValue.clear();
            if (m_nNULLPos != -1)
            {
                aSelSeq.realloc(1);
                aSelSeq.getArray()[0] = m_nNULLPos;
            }
        }
        else
        {
            m_aSaveValue <<= sValue;

            aSelSeq = findValue( impl_getValues(), sValue, m_bBoundComponent );
        }
        return makeAny( aSelSeq );
    }

    // XReset
    //------------------------------------------------------------------------------
    Any OListBoxModel::getDefaultForReset() const
    {
        Any aValue;
        if (m_aDefaultSelectSeq.getLength())
            aValue <<= m_aDefaultSelectSeq;
        else if (m_nNULLPos != -1)  // gebundene Listbox
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

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxModel::disposing( const EventObject& _rSource ) throw ( RuntimeException )
    {
        if ( !OEntryListHelper::handleDisposing( _rSource ) )
            OBoundControlModel::disposing( _rSource );
    }

    //--------------------------------------------------------------------
    Any OListBoxModel::translateExternalValueToControlValue( ) const
    {
        OSL_PRECOND( hasExternalValueBinding(),
            "OListBoxModel::translateExternalValueToControlValue: precondition not met!" );

        Sequence< sal_Int16 > aSelectIndexes;
        if ( hasExternalValueBinding() )
        {
            switch ( m_eTransferSelectionAs )
            {
            case tsIndexList:
                {
                    // unfortunately, our select sequence is a sequence<short>, while the external binding
                    // supplies sequence<int> only -> transform this
                    Sequence< sal_Int32 > aSelectIndexesPure;
                    getExternalValueBinding()->getValue( ::getCppuType( static_cast< Sequence< sal_Int32 >* >( NULL ) ) ) >>= aSelectIndexesPure;
                    aSelectIndexes.realloc( aSelectIndexesPure.getLength() );
                    ::std::copy(
                        aSelectIndexesPure.getConstArray(),
                        aSelectIndexesPure.getConstArray() + aSelectIndexesPure.getLength(),
                        aSelectIndexes.getArray()
                    );
                }
                break;

            case tsIndex:
                {
                    sal_Int32 nSelectIndex = -1;
                    getExternalValueBinding()->getValue( ::getCppuType( static_cast< sal_Int32* >( NULL ) ) ) >>= nSelectIndex;
                    if ( ( nSelectIndex >= 0 ) && ( nSelectIndex < getStringItemList().getLength() ) )
                    {
                        aSelectIndexes.realloc( 1 );
                        aSelectIndexes[ 0 ] = static_cast< sal_Int16 >( nSelectIndex );
                    }
                }
                break;

            case tsEntryList:
                {
                    // we can retrieve a string list from the binding for multiple selection
                    Sequence< ::rtl::OUString > aSelectEntries;
                    getExternalValueBinding()->getValue( ::getCppuType( static_cast< Sequence< ::rtl::OUString >* >( NULL ) ) ) >>= aSelectEntries;

                    ::std::set< sal_Int16 > aSelectionSet;

                    // find the selection entries in our item list
                    const ::rtl::OUString* pSelectEntries = aSelectEntries.getArray();
                    const ::rtl::OUString* pSelectEntriesEnd = pSelectEntries + aSelectEntries.getLength();
                    while ( pSelectEntries != pSelectEntriesEnd )
                    {
                        // the indexes where the current string appears in our string items
                        Sequence< sal_Int16 > aThisEntryIndexes;
                        aThisEntryIndexes = findValue( getStringItemList(), *pSelectEntries++, sal_False );

                        // insert all the indexes of this entry into our set
                        ::std::copy(
                            aThisEntryIndexes.getConstArray(),
                            aThisEntryIndexes.getConstArray() + aThisEntryIndexes.getLength(),
                            ::std::insert_iterator< ::std::set< sal_Int16 > >( aSelectionSet, aSelectionSet.begin() )
                            );
                    }

                    // copy the indexes to the sequence
                    aSelectIndexes.realloc( aSelectionSet.size() );
                    ::std::copy(
                        aSelectionSet.begin(),
                        aSelectionSet.end(),
                        aSelectIndexes.getArray()
                        );
                }
                break;

            case tsEntry:
                {
                    ::rtl::OUString sStringToSelect;
                    getExternalValueBinding()->getValue( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) ) >>= sStringToSelect;

                    aSelectIndexes = findValue( getStringItemList(), sStringToSelect, sal_False );
                }
                break;
            }
        }

        return makeAny( aSelectIndexes );
    }

    //--------------------------------------------------------------------
    void OListBoxModel::onConnectedExternalValue( )
    {
        OSL_ENSURE( hasExternalValueBinding(), "OListBoxModel::onConnectedExternalValue: no external value binding!" );

        // if the binding supports string sequences, we prefer this
        if ( getExternalValueBinding().is() )
        {
            if ( getExternalValueBinding()->supportsType( ::getCppuType( static_cast< Sequence< sal_Int32 >* >( NULL ) ) ) )
            {
                m_eTransferSelectionAs = tsIndexList;
            }
            else if ( getExternalValueBinding()->supportsType( ::getCppuType( static_cast< sal_Int32* >( NULL ) ) ) )
            {
                m_eTransferSelectionAs = tsIndex;
            }
            else if ( getExternalValueBinding()->supportsType( ::getCppuType( static_cast< Sequence< ::rtl::OUString >* >( NULL ) ) ) )
            {
                m_eTransferSelectionAs = tsEntryList;
            }
            else
            {
                OSL_ENSURE( getExternalValueBinding()->supportsType( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) ),
                    "OListBoxModel::onConnectedExternalValue: this should not have survived approveValueBinding!" );
                m_eTransferSelectionAs = tsEntry;
            }
        }

        OBoundControlModel::onConnectedExternalValue( );
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        struct ExtractStringFromSequence_Safe : public ::std::unary_function< sal_Int16, ::rtl::OUString >
        {
        protected:
            const Sequence< ::rtl::OUString >&  m_rList;

        public:
            ExtractStringFromSequence_Safe( const Sequence< ::rtl::OUString >& _rList ) : m_rList( _rList ) { }

            ::rtl::OUString operator ()( sal_Int16 _nIndex )
            {
                OSL_ENSURE( _nIndex < m_rList.getLength(), "ExtractStringFromSequence_Safe: inconsistence!" );
                if ( _nIndex < m_rList.getLength() )
                    return m_rList[ _nIndex ];
                return ::rtl::OUString();
            }
        };

        //................................................................
        Any lcl_getSingleSelectedEntry( const Sequence< sal_Int16 >& _rSelectSequence, const Sequence< ::rtl::OUString >& _rStringList )
        {
            Any aReturn;

            // by definition, multiple selected entries are transfered as NULL if the
            // binding does not support string lists
            if ( _rSelectSequence.getLength() <= 1 )
            {
                ::rtl::OUString sSelectedEntry;

                if ( _rSelectSequence.getLength() == 1 )
                    sSelectedEntry = ExtractStringFromSequence_Safe( _rStringList )( _rSelectSequence[0] );

                aReturn <<= sSelectedEntry;
            }

            return aReturn;
        }

        //................................................................
        Any lcl_getMultiSelectedEntries( const Sequence< sal_Int16 >& _rSelectSequence, const Sequence< ::rtl::OUString >& _rStringList )
        {
            Sequence< ::rtl::OUString > aSelectedEntriesTexts( _rSelectSequence.getLength() );
            ::std::transform(
                _rSelectSequence.getConstArray(),
                _rSelectSequence.getConstArray() + _rSelectSequence.getLength(),
                aSelectedEntriesTexts.getArray(),
                ExtractStringFromSequence_Safe( _rStringList )
            );
            return makeAny( aSelectedEntriesTexts );
        }
    }

    //--------------------------------------------------------------------
    Any OListBoxModel::translateControlValueToExternalValue( ) const
    {
        OSL_PRECOND( hasExternalValueBinding(), "OListBoxModel::translateControlValueToExternalValue: no binding!" );

        Sequence< sal_Int16 > aSelectSequence;
        const_cast< OListBoxModel* >( this )->getPropertyValue( PROPERTY_SELECT_SEQ ) >>= aSelectSequence;

        Any aReturn;
        switch ( m_eTransferSelectionAs )
        {
        case tsIndexList:
            {
                OSL_ENSURE( getExternalValueBinding()->supportsType( ::getCppuType( static_cast< Sequence< sal_Int32 >* >( NULL ) ) ),
                    "OListBoxModel::translateControlValueToExternalValue: how this? It does not support string sequences!" );
                // unfortunately, the select sequence is a sequence<short>, but our binding
                // expects int's
                Sequence< sal_Int32 > aTransformed( aSelectSequence.getLength() );
                ::std::copy(
                    aSelectSequence.getConstArray(),
                    aSelectSequence.getConstArray() + aSelectSequence.getLength(),
                    aTransformed.getArray()
                );
                aReturn <<= aTransformed;
            }
            break;

        case tsIndex:
            if ( aSelectSequence.getLength() <= 1 )
            {
                sal_Int32 nIndex = -1;

                if ( aSelectSequence.getLength() == 1 )
                    nIndex = aSelectSequence[0];

                aReturn <<= nIndex;
            }
            break;

        case tsEntryList:
            aReturn = lcl_getMultiSelectedEntries( aSelectSequence, getStringItemList() );
            break;

        case tsEntry:
            aReturn = lcl_getSingleSelectedEntry( aSelectSequence, getStringItemList() );
            break;
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    Any OListBoxModel::getCurrentFormComponentValue() const
    {
        if ( hasValidator() )
            return OBoundControlModel::getCurrentFormComponentValue();

        Any aCurretnValue;

        try
        {
            Sequence< sal_Int16 > aSelectSequence;
            OSL_VERIFY( const_cast< OListBoxModel* >( this )->getPropertyValue( PROPERTY_SELECT_SEQ ) >>= aSelectSequence );

            sal_Bool bMultiSelection( sal_False );
            OSL_VERIFY( const_cast< OListBoxModel* >( this )->getPropertyValue( PROPERTY_MULTISELECTION ) >>= bMultiSelection );

            if ( bMultiSelection )
                aCurretnValue = lcl_getMultiSelectedEntries( aSelectSequence, getStringItemList() );
            else
                aCurretnValue = lcl_getSingleSelectedEntry( aSelectSequence, getStringItemList() );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return aCurretnValue;
    }

    //--------------------------------------------------------------------
    sal_Bool OListBoxModel::approveValueBinding( const Reference< XValueBinding >& _rxBinding )
    {
        OSL_PRECOND( _rxBinding.is(), "OListBoxModel::approveValueBinding: precondition not met!" );

        // only strings are accepted for simplicity
        return  _rxBinding.is()
            &&  (   _rxBinding->supportsType( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) )
                ||  _rxBinding->supportsType( ::getCppuType( static_cast< Sequence< ::rtl::OUString >* >( NULL ) ) )
                ||  _rxBinding->supportsType( ::getCppuType( static_cast< sal_Int32* >( NULL ) ) )
                ||  _rxBinding->supportsType( ::getCppuType( static_cast< Sequence< sal_Int32 >* >( NULL ) ) )
                );
    }

    //--------------------------------------------------------------------
    void OListBoxModel::stringItemListChanged( )
    {
        if ( m_xAggregateSet.is() )
        {
            suspendValueListening();
            try
            {
                m_xAggregateSet->setPropertyValue( PROPERTY_STRINGITEMLIST, makeAny( getStringItemList() ) );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "OListBoxModel::stringItemListChanged: caught an exception!" );
            }
            resumeValueListening();

            // update the selection here
            if ( hasExternalValueBinding( ) )
                transferExternalValueToControl( );
            else
            {
                if ( hasField() )
                {
                    // TODO: update the selection in case we're bound to a database column
                }
                else
                {
                    if ( m_aDefaultSelectSeq.getLength() )
                        setControlValue( makeAny( m_aDefaultSelectSeq ), eOther );
                }
            }
        }
    }

    //--------------------------------------------------------------------
    void OListBoxModel::connectedExternalListSource( )
    {
        // TODO?
    }

    //--------------------------------------------------------------------
    void OListBoxModel::disconnectedExternalListSource( )
    {
        // TODO: in case we're part of an already loaded form, we should probably simulate
        // an onConnectedDbColumn, so our list get's filled with the data as indicated
        // by our SQL-binding related properties
    }

    //==================================================================
    // OListBoxControl
    //==================================================================

    //------------------------------------------------------------------
    InterfaceRef SAL_CALL OListBoxControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
    {
        return *(new OListBoxControl(_rxFactory));
    }

    //------------------------------------------------------------------------------
    Sequence< Type> OListBoxControl::_getTypes()
    {
        return TypeBag(
            OBoundControl::_getTypes(),
            OListBoxControl_BASE::getTypes()
        ).getTypes();
    }

    //------------------------------------------------------------------
    Any SAL_CALL OListBoxControl::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        Any aReturn = OListBoxControl_BASE::queryInterface( _rType );

        if  (   !aReturn.hasValue()
            ||  _rType.equals( XTypeProvider::static_type() )
            )
            aReturn = OBoundControl::queryAggregation( _rType );

        return aReturn;
    }

    DBG_NAME(OListBoxControl);
    //------------------------------------------------------------------------------
    OListBoxControl::OListBoxControl(const Reference<XMultiServiceFactory>& _rxFactory)
        :OBoundControl( _rxFactory, VCL_CONTROL_LISTBOX, sal_False )
        ,m_aChangeListeners( m_aMutex )
        ,m_aItemListeners( m_aMutex )
        ,m_pItemBroadcaster( NULL )
    {
        DBG_CTOR(OListBoxControl,NULL);

        increment(m_refCount);
        {
            // als FocusListener anmelden
            Reference<XWindow> xComp;
            if (query_aggregation(m_xAggregate, xComp))
                xComp->addFocusListener(this);

            // als ItemListener anmelden
            if ( query_aggregation( m_xAggregate, m_xAggregateListBox ) )
                m_xAggregateListBox->addItemListener(this);
        }
        // Refcount bei 2 fuer angemeldete Listener
        decrement(m_refCount);

        doSetDelegator();

        m_aChangeTimer.SetTimeout(500);
        m_aChangeTimer.SetTimeoutHdl(LINK(this,OListBoxControl,OnTimeout));
    }

    //------------------------------------------------------------------------------
    OListBoxControl::~OListBoxControl()
    {
        if (!OComponentHelper::rBHelper.bDisposed)
        {
            acquire();
            dispose();
        }

        doResetDelegator();
        m_xAggregateListBox.clear();

        DBG_DTOR(OListBoxControl,NULL);
    }

    //------------------------------------------------------------------------------
    StringSequence SAL_CALL OListBoxControl::getSupportedServiceNames() throw(RuntimeException)
    {
        StringSequence aSupported = OBoundControl::getSupportedServiceNames();
        aSupported.realloc(aSupported.getLength() + 1);

        ::rtl::OUString* pArray = aSupported.getArray();
        pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_LISTBOX;
        return aSupported;
    }


    // XFocusListener
    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxControl::focusGained(const FocusEvent& /*_rEvent*/) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( m_aChangeListeners.getLength() ) // only if there are listeners
        {
            Reference<XPropertySet> xSet(getModel(), UNO_QUERY);
            if (xSet.is())
            {
                // memorize the current selection for posting the change event
                m_aCurrentSelection = xSet->getPropertyValue(PROPERTY_SELECT_SEQ);
            }
        }
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxControl::focusLost(const FocusEvent& /*_rEvent*/) throw(RuntimeException)
    {
        m_aCurrentSelection.clear();
    }

    // XItemListener
    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxControl::itemStateChanged(const ItemEvent& _rEvent) throw(RuntimeException)
    {
        // forward this to our listeners
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_aItemListeners.getLength() )
            {
                if ( !m_pItemBroadcaster.is() )
                {
                    m_pItemBroadcaster.set( new ::comphelper::AsyncEventNotifier );
                    m_pItemBroadcaster->create();
                }
                m_pItemBroadcaster->addEvent( new ItemEventDescription( _rEvent ), this );
            }
        }

        // and do the handling for the ChangeListeners
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
                    // Has the selection been changed?
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

    // XEventListener
    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxControl::disposing(const EventObject& _rSource) throw (RuntimeException)
    {
        OBoundControl::disposing(_rSource);
    }

    // XChangeBroadcaster
    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxControl::addChangeListener(const Reference<XChangeListener>& _rxListener) throw(RuntimeException)
    {
        m_aChangeListeners.addInterface( _rxListener );
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxControl::removeChangeListener(const Reference<XChangeListener>& _rxListener) throw(RuntimeException)
    {
        m_aChangeListeners.removeInterface( _rxListener );
    }

    // OComponentHelper
    //------------------------------------------------------------------------------
    void OListBoxControl::disposing()
    {
        if (m_aChangeTimer.IsActive())
            m_aChangeTimer.Stop();

        EventObject aEvent( *this );
        m_aChangeListeners.disposeAndClear( aEvent );
        m_aItemListeners.disposeAndClear( aEvent );

        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pItemBroadcaster.is() )
            {
                m_pItemBroadcaster->removeEventsForProcessor( this );
                m_pItemBroadcaster->terminate();
                m_pItemBroadcaster = NULL;
            }
        }

        OBoundControl::disposing();
    }

    //------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------
    IMPL_LINK(OListBoxControl, OnTimeout, void*, /*EMPTYTAG*/)
    {
        m_aChangeListeners.notifyEach( &XChangeListener::changed, EventObject( *this ) );
        return 0L;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::addItemListener( const Reference< XItemListener >& l ) throw (RuntimeException)
    {
        m_aItemListeners.addInterface( l );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::removeItemListener( const Reference< XItemListener >& l ) throw (RuntimeException)
    {
        m_aItemListeners.removeInterface( l );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::addActionListener( const Reference< XActionListener >& l ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->addActionListener( l );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::removeActionListener( const Reference< XActionListener >& l ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->removeActionListener( l );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::addItem( const ::rtl::OUString& aItem, ::sal_Int16 nPos ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->addItem( aItem, nPos );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::addItems( const Sequence< ::rtl::OUString >& aItems, ::sal_Int16 nPos ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->addItems( aItems, nPos );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::removeItems( ::sal_Int16 nPos, ::sal_Int16 nCount ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->removeItems( nPos, nCount );
    }

    //--------------------------------------------------------------------
    ::sal_Int16 SAL_CALL OListBoxControl::getItemCount(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getItemCount();
        return 0;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OListBoxControl::getItem( ::sal_Int16 nPos ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getItem( nPos );
        return ::rtl::OUString( );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OListBoxControl::getItems(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getItems();
        return Sequence< ::rtl::OUString >( );
    }

    //--------------------------------------------------------------------
    ::sal_Int16 SAL_CALL OListBoxControl::getSelectedItemPos(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getSelectedItemPos();
        return 0;
    }

    //--------------------------------------------------------------------
    Sequence< ::sal_Int16 > SAL_CALL OListBoxControl::getSelectedItemsPos(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getSelectedItemsPos();
        return Sequence< ::sal_Int16 >( );
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OListBoxControl::getSelectedItem(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getSelectedItem();
        return ::rtl::OUString( );
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL OListBoxControl::getSelectedItems(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getSelectedItems();
        return Sequence< ::rtl::OUString >( );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::selectItemPos( ::sal_Int16 nPos, ::sal_Bool bSelect ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->selectItemPos( nPos, bSelect );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::selectItemsPos( const Sequence< ::sal_Int16 >& aPositions, ::sal_Bool bSelect ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->selectItemsPos( aPositions, bSelect );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::selectItem( const ::rtl::OUString& aItem, ::sal_Bool bSelect ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->selectItem( aItem, bSelect );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL OListBoxControl::isMutipleMode(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->isMutipleMode();
        return sal_False;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::setMultipleMode( ::sal_Bool bMulti ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->setMultipleMode( bMulti );
    }

    //--------------------------------------------------------------------
    ::sal_Int16 SAL_CALL OListBoxControl::getDropDownLineCount(  ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            return m_xAggregateListBox->getDropDownLineCount();
        return 0;
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::setDropDownLineCount( ::sal_Int16 nLines ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->setDropDownLineCount( nLines );
    }

    //--------------------------------------------------------------------
    void SAL_CALL OListBoxControl::makeVisible( ::sal_Int16 nEntry ) throw (RuntimeException)
    {
        if ( m_xAggregateListBox.is() )
            m_xAggregateListBox->makeVisible( nEntry );
    }

//.........................................................................
}
//.........................................................................

