/*************************************************************************
*
*   $RCSfile: ListBox.cxx,v $
*
*   $Revision: 1.33 $
*
*   last change: $Author: hr $ $Date: 2004-08-02 16:28:56 $
*
*   The Contents of this file are made available subject to the terms of
*   either of the following licenses
*
*          - GNU Lesser General Public License Version 2.1
*          - Sun Industry Standards Source License Version 1.1
*
*   Sun Microsystems Inc., October, 2000
*
*   GNU Lesser General Public License Version 2.1
*   =============================================
*   Copyright 2000 by Sun Microsystems, Inc.
*   901 San Antonio Road, Palo Alto, CA 94303, USA
*
*   This library is free software; you can redistribute it and/or
*   modify it under the terms of the GNU Lesser General Public
*   License version 2.1, as published by the Free Software Foundation.
*
*   This library is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   Lesser General Public License for more details.
*
*   You should have received a copy of the GNU Lesser General Public
*   License along with this library; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*   MA  02111-1307  USA
*
*
*   Sun Industry Standards Source License Version 1.1
*   =================================================
*   The contents of this file are subject to the Sun Industry Standards
*   Source License Version 1.1 (the "License"); You may not use this file
*   except in compliance with the License. You may obtain a copy of the
*   License at http://www.openoffice.org/license.html.
*
*   Software provided under this License is provided on an "AS IS" basis,
*   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*   See the License for the specific provisions governing your rights and
*   obligations concerning the Software.
*
*   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*   Copyright: 2000 by Sun Microsystems, Inc.
*
*   All Rights Reserved.
*
*   Contributor(s): _______________________________________
*
*
************************************************************************/

#ifndef _FORMS_LISTBOX_HXX_
#include "ListBox.hxx"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _FRM_RESOURCE_HXX_
#include "frm_resource.hxx"
#endif
#ifndef _FRM_RESOURCE_HRC_
#include "frm_resource.hrc"
#endif
#ifndef _FORMS_BASELISTBOX_HXX_
#include "BaseListBox.hxx"
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif
#ifndef _COMPHELPER_DATETIME_HXX_
#include <comphelper/datetime.hxx>
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include <connectivity/dbconversion.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLERROREVENT_HPP_
#include <com/sun/star/sdb/SQLErrorEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLISTBOX_HPP_
#include <com/sun/star/awt/XListBox.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif

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
        return ::comphelper::concatSequences(
            ::comphelper::concatSequences(
            OBoundControlModel::_getTypes(),
            OListBoxModel_BASE::getTypes(),
            OEntryListHelper::getTypes()
            ),
            OErrorBroadcaster::getTypes()
            );
    }


    DBG_NAME(OListBoxModel);
    //------------------------------------------------------------------
    OListBoxModel::OListBoxModel(const Reference<XMultiServiceFactory>& _rxFactory)
        :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_LISTBOX, FRM_SUN_CONTROL_LISTBOX, sal_True, sal_True, sal_True )
        // use the old control name for compytibility reasons
        ,OEntryListHelper( m_aMutex )
        ,OErrorBroadcaster( OComponentHelper::rBHelper )
        ,m_aRefreshListeners(m_aMutex)
        ,m_bBoundComponent(sal_False)
        ,m_eTransferSelectionAs( tsEntry )
        ,m_nNULLPos(-1)
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
        ,m_aRefreshListeners( m_aMutex )
        ,m_bBoundComponent(sal_False)
        ,m_eTransferSelectionAs( tsEntry )
        ,m_nNULLPos(-1)
    {
        DBG_CTOR(OListBoxModel,NULL);
        m_eListSourceType = _pOriginal->m_eListSourceType;
        m_aBoundColumn = _pOriginal->m_aBoundColumn;
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
        NOTIFY_LISTENERS(m_aRefreshListeners, XRefreshListener, refreshed, aEvt);
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
            else if ( m_xCursor.is() && !getField().is() && !hasExternalListSource() )
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
                setControlValue( _rValue );
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
    Reference<XPropertySetInfo> SAL_CALL OListBoxModel::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //------------------------------------------------------------------------------
    cppu::IPropertyArrayHelper& OListBoxModel::getInfoHelper()
    {
        return *const_cast<OListBoxModel*>(this)->getArrayHelper();
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
    {
        BEGIN_DESCRIBE_PROPERTIES( 7, OBoundControlModel )
            RemoveProperty( _rAggregateProps, PROPERTY_STRINGITEMLIST );
                // we want to "override" this property

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
            sal_Int16 nBoundColumn;
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
        if (m_aControlSource.getLength())
            // (not if we don't have a control source - the "State" property acts like it is persistent, then
            resetNoBroadcast();
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::loadData()
    {
        DBG_ASSERT( m_eListSourceType != ListSourceType_VALUELIST, "OListBoxModel::loadData: cannot load value list from DB!" );
        DBG_ASSERT( !hasExternalListSource(), "OListBoxModel::loadData: cannot load from DB when I have an external list source!" );

        m_nNULLPos = -1;
        m_bBoundComponent = sal_False;

        // Connection holen
        Reference<XRowSet> xForm(m_xCursor, UNO_QUERY);
        if (!xForm.is())
            return;
        Reference<XConnection> xConnection = getConnection(xForm);
        if (!xConnection.is())
            return;

        // we need a com::sun::star::sdb::Connection for some of the code below ...
        Reference<XServiceInfo> xServiceInfo(xConnection, UNO_QUERY);
        if (!xServiceInfo.is() || !xServiceInfo->supportsService(SRV_SDB_CONNECTION))
        {
            DBG_ERROR("OListBoxModel::loadData : invalid connection !");
            return;
        }

        Reference< XRowSet > xContentRowSet(m_xServiceFactory->createInstance(SRV_SDB_ROWSET), UNO_QUERY);
        Reference< XPropertySet > xContentSetProperties(xContentRowSet, UNO_QUERY);
        Reference<XResultSet> xListCursor(xContentSetProperties, UNO_QUERY);
        if (!xListCursor.is())
        {
            DBG_ERROR("OListBoxModel::loadData: could not instantiate a RowSet!");
            return;
        }

        // Wenn der ListSourceType keine Werteliste ist,
        // muss die String-Seq zu einem String zusammengefasst werden
        ::rtl::OUString sListSource;
        const ::rtl::OUString* pustrListSouceStrings = m_aListSourceSeq.getConstArray();
        sal_Int32 i;
        for (i=0; i<m_aListSourceSeq.getLength(); ++i)
            sListSource = sListSource + pustrListSouceStrings[i];
        if (!sListSource.getLength())
            return;

        sal_Int16 nBoundColumn = 0;
        if (m_aBoundColumn.getValueType().getTypeClass() == TypeClass_SHORT)
            m_aBoundColumn >>= nBoundColumn;

        try
        {
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
                        if (xFieldsByName->hasByName(m_aControlSource))
                            aFieldName = m_aControlSource;
                        else
                        {
                            // otherwise look for the alias
                            Reference<XSQLQueryComposerFactory> xFactory(xConnection, UNO_QUERY);
                            if (!xFactory.is())
                                break;

                            Reference<XSQLQueryComposer> xComposer = xFactory->createQueryComposer();
                            try
                            {
                                Reference<XPropertySet> xFormAsSet(xForm, UNO_QUERY);
                                ::rtl::OUString aStatement;
                                xFormAsSet->getPropertyValue(PROPERTY_ACTIVECOMMAND) >>= aStatement;
                                xComposer->setQuery(aStatement);
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
                            if (xFieldNames->hasByName(m_aControlSource))
                            {
                                Reference<XPropertySet> xComposerFieldAsSet;
                                xFieldNames->getByName(m_aControlSource) >>= xComposerFieldAsSet;
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
                    sal_Bool bUseCatalogInSelect = ::dbtools::isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalogInSelect")),sal_True);
                    sal_Bool bUseSchemaInSelect = ::dbtools::isDataSourcePropertyEnabled(xConnection,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSchemaInSelect")),sal_True);
                    aStatement += quoteTableName(xMeta, sListSource,::dbtools::eInDataManipulation,bUseCatalogInSelect,bUseSchemaInSelect);

                    xContentSetProperties->setPropertyValue(PROPERTY_COMMAND, makeAny(aStatement));
                    bExecute = sal_True;
                }
                break;

            case ListSourceType_QUERY:
                {
                    Reference<XQueriesSupplier> xSupplyQueries(xConnection, UNO_QUERY);
                    Reference<XPropertySet> xQuery(*(InterfaceRef*)xSupplyQueries->getQueries()->getByName(sListSource).getValue(), UNO_QUERY);
                    xContentSetProperties->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, xQuery->getPropertyValue(PROPERTY_ESCAPE_PROCESSING));

                    xContentSetProperties->setPropertyValue(PROPERTY_COMMAND, xQuery->getPropertyValue(PROPERTY_COMMAND));
                    bExecute = sal_True;
                }
                break;

            default:
                {
                    if (ListSourceType_SQLPASSTHROUGH == m_eListSourceType)
                        xContentSetProperties->setPropertyValue(PROPERTY_ESCAPE_PROCESSING, ::cppu::bool2any((sal_False)));
                    xContentSetProperties->setPropertyValue(PROPERTY_COMMAND, makeAny(sListSource));
                    bExecute = sal_True;
                }
            }

            if (bExecute)
            {
                Reference< XPropertySet > xFormProps(xForm, UNO_QUERY);

                xContentSetProperties->setPropertyValue( PROPERTY_COMMANDTYPE, makeAny( CommandType::COMMAND ) );
                xContentSetProperties->setPropertyValue( PROPERTY_DATASOURCE, xFormProps->getPropertyValue( PROPERTY_DATASOURCE ) );

                // try to give the row set the connection of our form - this saves the rowset from creating an own one
                xContentSetProperties->setPropertyValue( PROPERTY_ACTIVE_CONNECTION, xFormProps->getPropertyValue( PROPERTY_ACTIVE_CONNECTION ) );

                xContentRowSet->execute();
            }
        }
        catch(SQLException& eSQL)
        {
            onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
            disposeComponent(xListCursor);
            return;
        }
        catch(Exception& eUnknown)
        {
            eUnknown;
            disposeComponent(xListCursor);
            return;
        }

        if (ListSourceType_TABLEFIELDS != m_eListSourceType && !xListCursor.is())
            // something went wrong ...
            return;

        // Anzeige- und Werteliste fuellen
        vector< ::rtl::OUString >   aValueList, aStringList;
        aValueList.reserve(16);
        aStringList.reserve(16);
        sal_Bool bUseNULL = getField().is() && !m_bRequired;
        try
        {
            switch (m_eListSourceType)
            {
            case ListSourceType_SQL:
            case ListSourceType_SQLPASSTHROUGH:
            case ListSourceType_TABLE:
            case ListSourceType_QUERY:
                {
                    // Feld der 1. Column des ResultSets holen
                    Reference<XColumnsSupplier> xSupplyCols(xListCursor, UNO_QUERY);
                    DBG_ASSERT(xSupplyCols.is(), "OListBoxModel::loadData : cursor supports the row set service but is no column supplier ??!");
                    Reference<XIndexAccess> xColumns;
                    if (xSupplyCols.is())
                    {
                        xColumns = Reference<XIndexAccess>(xSupplyCols->getColumns(), UNO_QUERY);
                        DBG_ASSERT(xColumns.is(), "OListBoxModel::loadData : no columns supplied by the row set !");
                    }
                    Reference<XColumn> xDataField;
                    if (xColumns.is())
                        xColumns->getByIndex(0) >>= xDataField;
                    if (!xDataField.is())
                    {
                        disposeComponent(xListCursor);
                        return;
                    }

                    Reference<XNumberFormatsSupplier> xSupplier = getNumberFormats(xConnection, sal_False, m_xServiceFactory);

                    ::com::sun::star::util::Date aNullDate(DBTypeConversion::getStandardDate());
                    sal_Int32 nFormatKey = 0;
                    sal_Int32 nFieldType = DataType::OTHER;
                    sal_Int16 nKeyType   = NumberFormat::UNDEFINED;
                    sal_Bool bHaveFormat = sal_False;
                    Reference<XPropertySet> xFieldAsSet(xDataField, UNO_QUERY);
                    try
                    {
                        xFieldAsSet->getPropertyValue(PROPERTY_FIELDTYPE) >>= nFieldType;
                        bHaveFormat = (xFieldAsSet->getPropertyValue(PROPERTY_FORMATKEY) >>= nFormatKey);
                    }
                    catch(Exception&)
                    {
                        DBG_ERROR("OListBoxModel::loadData: could not obtain the field type and/or format key of the bound column!");
                    }

                    if (!bHaveFormat)
                    {
                        Locale aAppLanguage = Application::GetSettings().GetUILocale();
                        if (xSupplier.is())
                        {
                            Reference< XNumberFormatTypes > xNumTypes(xSupplier->getNumberFormats(), UNO_QUERY);
                            if (xNumTypes.is())
                                nFormatKey = getDefaultNumberFormat(xFieldAsSet, xNumTypes, aAppLanguage);
                        }
                    }

                    Reference<XNumberFormatter> xFormatter;
                    if (xSupplier.is())
                    {
                        xFormatter = Reference<XNumberFormatter>(
                            m_xServiceFactory->createInstance(FRM_NUMBER_FORMATTER),
                            UNO_QUERY
                            );
                        if (xFormatter.is())
                        {
                            xFormatter->attachNumberFormatsSupplier(xSupplier);
                            xFormatter->getNumberFormatsSupplier()->getNumberFormatSettings()->getPropertyValue(
                                ::rtl::OUString::createFromAscii("NullDate")) >>= aNullDate;
                            nKeyType = getNumberFormatType(xFormatter->getNumberFormatsSupplier()->getNumberFormats(), nFormatKey);
                        }
                    }

                    // Feld der BoundColumn des ResultSets holen
                    Reference<XColumn> xBoundField;
                    if ((nBoundColumn > 0) && m_xColumn.is())
                        // don't look for a bound column if we're not connected to a field
                        xColumns->getByIndex(nBoundColumn) >>= xBoundField;
                    m_bBoundComponent = xBoundField.is();

                    //  Ist die LB an ein Feld gebunden und sind Leereinträge zulaessig
                    //  dann wird die Position fuer einen Leereintrag gemerkt

                    ::rtl::OUString aStr;
                    sal_Int16 i = 0;
                    // per definitionem the list cursor is positioned _before_ the first row at the moment
                    while (xListCursor->next() && (i++<SHRT_MAX)) // max anzahl eintraege
                    {
                        aStr = DBTypeConversion::getValue(xDataField,
                            xFormatter,
                            aNullDate,
                            nFormatKey,
                            nKeyType);

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
            }
        }
        catch(SQLException& eSQL)
        {
            onError(eSQL, FRM_RES_STRING(RID_BASELISTBOX_ERROR_FILLLIST));
            disposeComponent(xListCursor);
            return;
        }
        catch(Exception& eUnknown)
        {
            eUnknown;
            disposeComponent(xListCursor);
            return;
        }


        // Value-Sequence erzeugen
        // NULL eintrag hinzufuegen
        if (bUseNULL && m_nNULLPos == -1)
        {
            if (m_bBoundComponent)
                aValueList.insert(aValueList.begin());

            aStringList.insert(aStringList.begin());
            m_nNULLPos = 0;
        }

        m_aValueSeq.realloc(aValueList.size());
        ::rtl::OUString* pustrValues = m_aValueSeq.getArray();
        for (i = 0; i < (sal_Int32)aValueList.size(); ++i)
            pustrValues[i] = aValueList[i];

        // String-Sequence fuer ListBox erzeugen
        StringSequence aStringSeq(aStringList.size());
        ::rtl::OUString* pustrStrings = aStringSeq.getArray();
        for (i = 0; i < (sal_Int32)aStringList.size(); ++i)
            pustrStrings[i] = aStringList[i];

        setFastPropertyValue(PROPERTY_ID_STRINGITEMLIST, makeAny(aStringSeq));

        // Statement + Cursor zerstoeren
        disposeComponent(xListCursor);
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::implRefreshListFromDbBinding( )
    {
        DBG_ASSERT( !hasExternalListSource( ), "OListBoxModel::implRefreshListFromDbBinding: invalid call!" );

        if ( m_eListSourceType != ListSourceType_VALUELIST )
        {
            if ( getField().is() )
                m_aValueSeq = StringSequence();

            if ( m_xCursor.is() )
                loadData();
        }
    }

    //------------------------------------------------------------------------------
    void OListBoxModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
    {
        // list boxes which are bound to a db column don't have multi selection
        // - this would be unable to reflect in the db column
        if ( getField().is() )
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
            StringSequence  aValues;
            if ( m_aValueSeq.getLength() )
            {
                aValues = m_aValueSeq;
            }
            else
            {
                aValues = getStringItemList();
            }

            const ::rtl::OUString *pVals    = aValues.getConstArray();
            sal_Int32 nValCnt               = aValues.getLength();

            if (nSelCount > 1)
            {
                // Einfach- oder Mehrfach-Selektion
                sal_Bool bMultiSel;
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
    sal_Bool OListBoxModel::commitControlValueToDbColumn( bool _bPostReset )
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

            // In der Werteliste nur einzelne Werte suchen, wenn das Control mit einem Datenbankfeld verbunden ist
            if ( m_aValueSeq.getLength() )  // value list
            {
                aSelSeq = findValue( m_aValueSeq, sValue, m_bBoundComponent );
            }
            else
            {
                aSelSeq = findValue( getStringItemList(), sValue, m_bBoundComponent );
            }
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
    Any OListBoxModel::translateExternalValueToControlValue( )
    {
        OSL_PRECOND( hasExternalValueBinding(),
            "OListBoxModel::translateExternalValueToControlValue: precondition not met!" );

        Sequence< sal_Int16 > aSelectIndexes;
        if ( m_xExternalBinding.is() )
        {
            switch ( m_eTransferSelectionAs )
            {
            case tsIndexList:
                {
                    // unfortunately, our select sequence is a sequence<short>, while the external binding
                    // supplies sequence<int> only -> transform this
                    Sequence< sal_Int32 > aSelectIndexesPure;
                    m_xExternalBinding->getValue( ::getCppuType( static_cast< Sequence< sal_Int32 >* >( NULL ) ) ) >>= aSelectIndexesPure;
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
                    m_xExternalBinding->getValue( ::getCppuType( static_cast< sal_Int32* >( NULL ) ) ) >>= nSelectIndex;
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
                    m_xExternalBinding->getValue( ::getCppuType( static_cast< Sequence< ::rtl::OUString >* >( NULL ) ) ) >>= aSelectEntries;

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
                            insert_iterator< ::std::set< sal_Int16 > >( aSelectionSet, aSelectionSet.begin() )
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
                    m_xExternalBinding->getValue( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) ) >>= sStringToSelect;

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
        if ( m_xExternalBinding.is() )
        {
            if ( m_xExternalBinding->supportsType( ::getCppuType( static_cast< Sequence< sal_Int32 >* >( NULL ) ) ) )
            {
                m_eTransferSelectionAs = tsIndexList;
            }
            else if ( m_xExternalBinding->supportsType( ::getCppuType( static_cast< sal_Int32* >( NULL ) ) ) )
            {
                m_eTransferSelectionAs = tsIndex;
            }
            else if ( m_xExternalBinding->supportsType( ::getCppuType( static_cast< Sequence< ::rtl::OUString >* >( NULL ) ) ) )
            {
                m_eTransferSelectionAs = tsEntryList;
            }
            else
            {
                OSL_ENSURE( m_xExternalBinding->supportsType( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) ),
                    "OListBoxModel::onConnectedExternalValue: this should not have survived approveValueBinding!" );
                m_eTransferSelectionAs = tsEntry;
            }
        }

        OBoundControlModel::onConnectedExternalValue( );
    }

    //....................................................................
    namespace
    {
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
    }

    //--------------------------------------------------------------------
    Any OListBoxModel::translateControlValueToExternalValue( )
    {
        OSL_PRECOND( hasExternalValueBinding(), "OListBoxModel::translateControlValueToExternalValue: no binding!" );

        Sequence< sal_Int16 > aSelectSequence;
        getPropertyValue( PROPERTY_SELECT_SEQ ) >>= aSelectSequence;

        Any aReturn;
        switch ( m_eTransferSelectionAs )
        {
        case tsIndexList:
            {
                OSL_ENSURE( m_xExternalBinding->supportsType( ::getCppuType( static_cast< Sequence< sal_Int32 >* >( NULL ) ) ),
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
            {
                Sequence< ::rtl::OUString > aSelectedEntriesTexts( aSelectSequence.getLength() );
                ::std::transform(
                    aSelectSequence.getConstArray(),
                    aSelectSequence.getConstArray() + aSelectSequence.getLength(),
                    aSelectedEntriesTexts.getArray(),
                    ExtractStringFromSequence_Safe( getStringItemList() )
                    );
            }
            break;

        case tsEntry:
            // by definition, multiple selected entries are transfered as NULL if the
            // binding does not support string lists
            if ( aSelectSequence.getLength() <= 1 )
            {
                ::rtl::OUString sSelectedEntry;

                if ( aSelectSequence.getLength() == 1 )
                    sSelectedEntry = ExtractStringFromSequence_Safe( getStringItemList() )( aSelectSequence[0] );

                aReturn <<= sSelectedEntry;
            }
            break;
        }

        return aReturn;
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
                // TODO: update the selection in case we're bound to a database column
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
        return concatSequences(
            OBoundControl::_getTypes(),
            OListBoxControl_BASE::getTypes()
            );
    }

    //------------------------------------------------------------------
    Any SAL_CALL OListBoxControl::queryAggregation(const Type& _rType) throw (RuntimeException)
    {
        Any aReturn = OBoundControl::queryAggregation(_rType);
        if (!aReturn.hasValue())
            aReturn = OListBoxControl_BASE::queryInterface(_rType);

        return aReturn;
    }

    DBG_NAME(OListBoxControl);
    //------------------------------------------------------------------------------
    OListBoxControl::OListBoxControl(const Reference<XMultiServiceFactory>& _rxFactory)
        :OBoundControl(_rxFactory, VCL_CONTROL_LISTBOX)
        ,m_aChangeListeners(m_aMutex)
    {
        DBG_CTOR(OListBoxControl,NULL);

        increment(m_refCount);
        {
            // als FocusListener anmelden
            Reference<XWindow> xComp;
            if (query_aggregation(m_xAggregate, xComp))
                xComp->addFocusListener(this);

            // als ItemListener anmelden
            Reference<XListBox> xListbox;
            if (query_aggregation(m_xAggregate, xListbox))
                xListbox->addItemListener(this);
        }
        // Refcount bei 2 fuer angemeldete Listener
        decrement(m_refCount);

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
    void SAL_CALL OListBoxControl::focusGained(const FocusEvent& _rEvent) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_aChangeListeners.getLength()) // only if there are listeners
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
    void SAL_CALL OListBoxControl::focusLost(const FocusEvent& _rEvent) throw(RuntimeException)
    {
        m_aCurrentSelection.clear();
    }

    // XItemListener
    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxControl::itemStateChanged(const ItemEvent& _rEvent) throw(RuntimeException)
    {
        // call the changelistener delayed
        ::osl::ClearableMutexGuard aGuard(m_aMutex);
        if (m_aChangeTimer.IsActive())
        {
            Reference<XPropertySet> xSet(getModel(), UNO_QUERY);
            m_aCurrentSelection = xSet->getPropertyValue(PROPERTY_SELECT_SEQ);

            m_aChangeTimer.Stop();
            m_aChangeTimer.Start();
        }
        else
        {
            if (m_aChangeListeners.getLength() && m_aCurrentSelection.hasValue())
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
        m_aChangeListeners.addInterface(_rxListener);
    }

    //------------------------------------------------------------------------------
    void SAL_CALL OListBoxControl::removeChangeListener(const Reference<XChangeListener>& _rxListener) throw(RuntimeException)
    {
        m_aChangeListeners.removeInterface(_rxListener);
    }

    // OComponentHelper
    //------------------------------------------------------------------------------
    void OListBoxControl::disposing()
    {
        if (m_aChangeTimer.IsActive())
            m_aChangeTimer.Stop();

        EventObject aEvt(static_cast< XWeak*>(this));
        m_aChangeListeners.disposeAndClear(aEvt);

        OBoundControl::disposing();
    }

    //------------------------------------------------------------------------------
    IMPL_LINK(OListBoxControl, OnTimeout, void*, EMPTYTAG)
    {
        EventObject aEvt(static_cast< XWeak*>(this));
        NOTIFY_LISTENERS(m_aChangeListeners, XChangeListener, changed, aEvt);
        return 1;
    }

//.........................................................................
}
//.........................................................................

