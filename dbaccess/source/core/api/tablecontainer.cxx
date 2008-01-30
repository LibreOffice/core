/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablecontainer.cxx,v $
 *
 *  $Revision: 1.67 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:30:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef _DBA_CORE_TABLECONTAINER_HXX_
#include "tablecontainer.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_CORE_TABLE_HXX_
#include "table.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBA_CORE_TABLEDECORATOR_HXX_
#include "TableDeco.hxx"
#endif
#ifndef DBACORE_SDBCORETOOLS_HXX
#include "sdbcoretools.hxx"
#endif
#ifndef DBA_CONTAINERMEDIATOR_HXX
#include "ContainerMediator.hxx"
#endif
#ifndef _DBACORE_DEFINITIONCOLUMN_HXX_
#include "definitioncolumn.hxx"
#endif
#ifndef DBACCESS_OBJECTNAMEAPPROVAL_HXX
#include "objectnameapproval.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

using namespace dbaccess;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;
using namespace ::connectivity::sdbcx;

namespace
{
    sal_Bool lcl_isPropertySetDefaulted(const Sequence< ::rtl::OUString>& _aNames,const Reference<XPropertySet>& _xProp)
    {
        Reference<XPropertyState> xState(_xProp,UNO_QUERY);
        if ( xState.is() )
        {
            const ::rtl::OUString* pIter = _aNames.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + _aNames.getLength();
            for(;pIter != pEnd;++pIter)
            {
                try
                {
                    PropertyState aState = xState->getPropertyState(*pIter);
                    if ( aState != PropertyState_DEFAULT_VALUE )
                        break;
                }
                catch(Exception)
                {
                    OSL_ENSURE( 0, "lcl_isPropertySetDefaulted: Exception caught!" );
                }
            }
            // the code below doesn't function -> I don't kow why
//          Sequence<PropertyState> aStates = xState->getPropertyStates(_aNames);
//          const PropertyState* pIter = aStates.getConstArray();
//          const PropertyState* pEnd  = pIter + aStates.getLength();
//          for( ; pIter != pEnd && *pIter == PropertyState_DEFAULT_VALUE; ++pIter)
//              ;
            return ( pIter == pEnd );
        }
        return sal_False;
    }
}
//==========================================================================
//= OTableContainer
//==========================================================================
DBG_NAME(OTableContainer)
//------------------------------------------------------------------------------
OTableContainer::OTableContainer(::cppu::OWeakObject& _rParent,
                                 ::osl::Mutex& _rMutex,
                                 const Reference< XConnection >& _xCon,
                                 sal_Bool _bCase,
                                 const Reference< XNameContainer >& _xTableDefinitions,
                                 IRefreshListener*  _pRefreshListener,
                                 IWarningsContainer* _pWarningsContainer
                                 ,oslInterlockedCount& _nInAppend)
    :OFilteredContainer(_rParent,_rMutex,_xCon,_bCase,_pRefreshListener,_pWarningsContainer,_nInAppend)
    ,m_xTableDefinitions(_xTableDefinitions)
    ,m_pTableMediator( NULL )
    ,m_bInDrop(sal_False)
{
    DBG_CTOR(OTableContainer, NULL);
}

//------------------------------------------------------------------------------
OTableContainer::~OTableContainer()
{
    //  dispose();
    DBG_DTOR(OTableContainer, NULL);
}

// -----------------------------------------------------------------------------
void OTableContainer::removeMasterContainerListener()
{
    Reference<XContainer> xCont(m_xMasterContainer,UNO_QUERY);
    if(xCont.is())
        xCont->removeContainerListener(this);
}
// -----------------------------------------------------------------------------
// XServiceInfo
//------------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO2(OTableContainer, "com.sun.star.sdb.dbaccess.OTableContainer", SERVICE_SDBCX_CONTAINER, SERVICE_SDBCX_TABLES)
// -------------------------------------------------------------------------
sal_Bool OTableContainer::isNameValid(  const ::rtl::OUString& _rName,
                                        const Sequence< ::rtl::OUString >& _rTableFilter,
                                        const Sequence< ::rtl::OUString >& _rTableTypeFilter,
                                        const ::std::vector< WildCard >& _rWCSearch) const
{
    if ( OFilteredContainer::isNameValid(_rName,_rTableFilter,_rTableTypeFilter,_rWCSearch) )
    {// the table name is allowed (not filtered out)
        // no type filter

        sal_Int32   nTableTypeFilterLen = _rTableTypeFilter.getLength();
        sal_Bool bNoTableFilters = ((nTableTypeFilterLen == 1) && _rTableTypeFilter[0].equalsAsciiL("%", 1));

        if ( bNoTableFilters || !nTableTypeFilterLen )
            return sal_True;

        // this is expensive but there is no other way to get the type of the table
        Reference<XPropertySet> xTable;
        ::cppu::extractInterface(xTable,m_xMasterContainer->getByName(_rName));
        ::rtl::OUString aTypeName;
        xTable->getPropertyValue(PROPERTY_TYPE) >>= aTypeName;
        const ::rtl::OUString* pTypeBegin   = _rTableTypeFilter.getConstArray();
        const ::rtl::OUString* pTypeEnd     = pTypeBegin + _rTableTypeFilter.getLength();
        for(;pTypeBegin != pTypeEnd;++pTypeBegin)
        {
            if(*pTypeBegin == aTypeName)
                return sal_True; // same as break and then checking
        }
    }
    return sal_False;
}
// -----------------------------------------------------------------------------
namespace
{
void lcl_createDefintionObject(const ::rtl::OUString& _rName
                           ,const Reference< XNameContainer >& _xTableDefinitions
                           ,Reference<XPropertySet>& _xTableDefinition
                           ,Reference<XNameAccess>& _xColumnDefinitions
                           ,sal_Bool _bModified)
{
    if ( _xTableDefinitions.is() )
    {
        if ( _xTableDefinitions->hasByName(_rName) )
            _xTableDefinition.set(_xTableDefinitions->getByName(_rName),UNO_QUERY);
        else
        {
            Sequence< Any > aArguments(1);
            PropertyValue aValue;
            // set as folder
            aValue.Name = PROPERTY_NAME;
            aValue.Value <<= _rName;
            aArguments[0] <<= aValue;
            _xTableDefinition.set(::comphelper::getProcessServiceFactory()->createInstanceWithArguments(SERVICE_SDB_TABLEDEFINITION,aArguments),UNO_QUERY);
            _xTableDefinitions->insertByName(_rName,makeAny(_xTableDefinition));
            ::dbaccess::notifyDataSourceModified(_xTableDefinitions,_bModified);
        }
        Reference<XColumnsSupplier> xColumnsSupplier(_xTableDefinition,UNO_QUERY);
        if ( xColumnsSupplier.is() )
            _xColumnDefinitions = xColumnsSupplier->getColumns();
    }
}
// -------------------------------------------------------------------------
}
// -------------------------------------------------------------------------
connectivity::sdbcx::ObjectType OTableContainer::createObject(const ::rtl::OUString& _rName)
{
    Reference<XColumnsSupplier > xSup;
    if(m_xMasterContainer.is() && m_xMasterContainer->hasByName(_rName))
        xSup.set(m_xMasterContainer->getByName(_rName),UNO_QUERY);

    connectivity::sdbcx::ObjectType xRet;
    if ( m_xMetaData.is() )
    {
        Reference<XPropertySet> xTableDefinition;
        Reference<XNameAccess> xColumnDefinitions;
        lcl_createDefintionObject(_rName,m_xTableDefinitions,xTableDefinition,xColumnDefinitions,sal_False);

        if ( xSup.is() )
        {
            ODBTableDecorator* pTable = new ODBTableDecorator( m_xConnection, xSup, ::dbtools::getNumberFormats( m_xConnection ) ,xColumnDefinitions);
            xRet = pTable;
            pTable->construct();
        }
        else
        {
            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(m_xMetaData,
                                                _rName,
                                                sCatalog,
                                                sSchema,
                                                sTable,
                                                ::dbtools::eInDataManipulation);
            Any aCatalog;
            if(sCatalog.getLength())
                aCatalog <<= sCatalog;
            ::rtl::OUString sType,sDescription;
            Sequence< ::rtl::OUString> aTypeFilter;
            getAllTableTypeFilter( aTypeFilter );

            Reference< XResultSet > xRes =  m_xMetaData.is() ? m_xMetaData->getTables(aCatalog,sSchema,sTable,aTypeFilter) : Reference< XResultSet >();
            if(xRes.is() && xRes->next())
            {
                Reference< XRow > xRow(xRes,UNO_QUERY);
                if(xRow.is())
                {
                    sType           = xRow->getString(4);
                    sDescription    = xRow->getString(5);
                }
            }
            ::comphelper::disposeComponent(xRes);
            ODBTable* pTable = new ODBTable(this
                                ,m_xConnection
                                ,sCatalog
                                ,sSchema
                                ,sTable
                                ,sType
                                ,sDescription
                                ,xColumnDefinitions);
            xRet = pTable;
            pTable->construct();
        }
        Reference<XPropertySet> xDest(xRet,UNO_QUERY);
        if ( xTableDefinition.is() )
            ::comphelper::copyProperties(xTableDefinition,xDest);

        if ( !m_pTableMediator.is() )
            m_pTableMediator = new OContainerMediator(
                    this, m_xTableDefinitions.get(), m_xConnection, OContainerMediator::eTables );
        if ( m_pTableMediator.is() )
            m_pTableMediator->notifyElementCreated(_rName,xDest);
    }

    return xRet;
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OTableContainer::createDescriptor()
{
    Reference< XPropertySet > xRet;

    // frist we have to look if the master tables does support this
    // and if then create a table object as well with the master tables
    Reference<XColumnsSupplier > xMasterColumnsSup;
    Reference<XDataDescriptorFactory> xDataFactory(m_xMasterContainer,UNO_QUERY);
    if ( xDataFactory.is() && m_xMetaData.is() )
    {
        xMasterColumnsSup = Reference< XColumnsSupplier >( xDataFactory->createDataDescriptor(), UNO_QUERY );
        ODBTableDecorator* pTable = new ODBTableDecorator( m_xConnection, xMasterColumnsSup, ::dbtools::getNumberFormats( m_xConnection ) ,NULL);
        xRet = pTable;
        pTable->construct();
    }
    else
    {
        ODBTable* pTable = new ODBTable(this, m_xConnection);
        xRet = pTable;
        pTable->construct();
    }
    return xRet;
}
// -----------------------------------------------------------------------------
// XAppend
ObjectType OTableContainer::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    // append the new table with a create stmt
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    if(m_xMasterContainer.is() && m_xMasterContainer->hasByName(aName))
    {
        String sMessage(DBACORE_RESSTRING(RID_STR_TABLE_IS_FILTERED));
        sMessage.SearchAndReplaceAscii("$name$", aName);
        throw SQLException(sMessage,static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)),SQLSTATE_GENERAL,1000,Any());
    }

    Reference< XConnection > xConnection( m_xConnection.get(), UNO_QUERY );
    PContainerApprove pApprove( new ObjectNameApproval( xConnection, ObjectNameApproval::TypeTable ) );
    pApprove->approveElement( aName, descriptor );

    try
    {
        EnsureReset aReset(m_nInAppend);
        Reference<XAppend> xAppend(m_xMasterContainer,UNO_QUERY);
        if(xAppend.is())
        {
            xAppend->appendByDescriptor(descriptor);
        }
        else
        {
            ::rtl::OUString aSql = ::dbtools::createSqlCreateTableStatement(descriptor,m_xConnection);

            Reference<XConnection> xCon = m_xConnection;
            OSL_ENSURE(xCon.is(),"Connection is null!");
            if ( xCon.is() )
            {
                Reference< XStatement > xStmt = xCon->createStatement(  );
                if ( xStmt.is() )
                    xStmt->execute(aSql);
                ::comphelper::disposeComponent(xStmt);
            }
        }
    }
    catch(Exception&)
    {
        throw;
    }

    Reference<XPropertySet> xTableDefinition;
    Reference<XNameAccess> xColumnDefinitions;
    lcl_createDefintionObject(getNameForObject(descriptor),m_xTableDefinitions,xTableDefinition,xColumnDefinitions,sal_False);
    Reference<XColumnsSupplier> xSup(descriptor,UNO_QUERY);
    Reference<XDataDescriptorFactory> xFac(xColumnDefinitions,UNO_QUERY);
    Reference<XAppend> xAppend(xColumnDefinitions,UNO_QUERY);
    sal_Bool bModified = sal_False;
    if ( xSup.is() && xColumnDefinitions.is() && xFac.is() && xAppend.is() )
    {
        Reference<XNameAccess> xNames = xSup->getColumns();
        if ( xNames.is() )
        {
            Reference<XPropertySet> xProp = xFac->createDataDescriptor();
            Sequence< ::rtl::OUString> aSeq = xNames->getElementNames();
            const ::rtl::OUString* pIter = aSeq.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
            for(;pIter != pEnd;++pIter)
            {
                if ( !xColumnDefinitions->hasByName(*pIter) )
                {
                    Reference<XPropertySet> xColumn(xNames->getByName(*pIter),UNO_QUERY);
                    OColumnSettings* pColumnSettings = NULL;
                    if ( ::comphelper::getImplementation( pColumnSettings, xColumn ) )
                    {
                        if ( ( pColumnSettings && !pColumnSettings->isDefaulted() ) )
                        {
                            ::comphelper::copyProperties(xColumn,xProp);
                            xAppend->appendByDescriptor(xProp);
                            bModified = sal_True;
                        }
                    }
                }
            }
        }
    }
    const static ::rtl::OUString s_pTableProps[] = {    ::rtl::OUString(PROPERTY_FILTER), ::rtl::OUString(PROPERTY_ORDER)
                                                    , ::rtl::OUString(PROPERTY_APPLYFILTER), ::rtl::OUString(PROPERTY_FONT)
                                                    , ::rtl::OUString(PROPERTY_ROW_HEIGHT), ::rtl::OUString(PROPERTY_TEXTCOLOR)
                                                    , ::rtl::OUString(PROPERTY_TEXTLINECOLOR), ::rtl::OUString(PROPERTY_TEXTEMPHASIS)
                                                    , ::rtl::OUString(PROPERTY_TEXTRELIEF) };
    Sequence< ::rtl::OUString> aNames(s_pTableProps,sizeof(s_pTableProps)/sizeof(s_pTableProps[0]));
    if ( bModified || !lcl_isPropertySetDefaulted(aNames,xTableDefinition) )
        ::dbaccess::notifyDataSourceModified(m_xTableDefinitions,sal_True);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OTableContainer::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    m_bInDrop = sal_True;
    try
    {
        Reference< XDrop > xDrop(m_xMasterContainer,UNO_QUERY);
        if(xDrop.is())
            xDrop->dropByName(_sElementName);
        else
        {
            ::rtl::OUString sCatalog,sSchema,sTable,sComposedName;

            sal_Bool bIsView = sal_False;
            Reference<XPropertySet> xTable(getObject(_nPos),UNO_QUERY);
            if ( xTable.is() && m_xMetaData.is() )
            {
                if( m_xMetaData.is() && m_xMetaData->supportsCatalogsInTableDefinitions() )
                    xTable->getPropertyValue(PROPERTY_CATALOGNAME)  >>= sCatalog;
                if( m_xMetaData.is() && m_xMetaData->supportsSchemasInTableDefinitions() )
                    xTable->getPropertyValue(PROPERTY_SCHEMANAME)   >>= sSchema;
                xTable->getPropertyValue(PROPERTY_NAME)         >>= sTable;

                sComposedName = ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sTable, sal_True, ::dbtools::eInTableDefinitions );

                ::rtl::OUString sType;
                xTable->getPropertyValue(PROPERTY_TYPE)         >>= sType;
                bIsView = sType.equalsIgnoreAsciiCase(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW")));
            }

            if(!sComposedName.getLength())
                ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(static_cast<OFilteredContainer*>(this)));

            ::rtl::OUString aSql = ::rtl::OUString::createFromAscii("DROP ");

            // #104282# OJ
            if ( bIsView ) // here we have a view
                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("VIEW "));
            else
                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TABLE "));
            aSql += sComposedName;
            Reference<XConnection> xCon = m_xConnection;
            OSL_ENSURE(xCon.is(),"Connection is null!");
            if ( xCon.is() )
            {
                Reference< XStatement > xStmt = xCon->createStatement(  );
                if(xStmt.is())
                    xStmt->execute(aSql);
                ::comphelper::disposeComponent(xStmt);
            }
        }

        if ( m_xTableDefinitions.is() && m_xTableDefinitions->hasByName(_sElementName) )
        {
            m_xTableDefinitions->removeByName(_sElementName);
        }
    }
    catch(Exception&)
    {
        m_bInDrop = sal_False;
        throw;
    }
    m_bInDrop = sal_False;
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableContainer::elementInserted( const ContainerEvent& Event ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString sName;
    Event.Accessor >>= sName;
    if ( !m_nInAppend && !hasByName(sName) )
    {
        if(!m_xMasterContainer.is() || m_xMasterContainer->hasByName(sName))
        {
            ObjectType xName = createObject(sName);
            insertElement(sName,xName);
            // and notify our listeners
            ContainerEvent aEvent(static_cast<XContainer*>(this), makeAny(sName), makeAny(xName), Any());
            m_aContainerListeners.notifyEach( &XContainerListener::elementInserted, aEvent );
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableContainer::elementRemoved( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableContainer::elementReplaced( const ContainerEvent& Event ) throw (RuntimeException)
{
    // create a new config entry
    {
        ::rtl::OUString sOldComposedName,sNewComposedName;
        Event.ReplacedElement   >>= sOldComposedName;
        Event.Accessor          >>= sNewComposedName;

        renameObject(sOldComposedName,sNewComposedName);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableContainer::disposing()
{
    OFilteredContainer::disposing();
    // say our listeners goobye
    m_xTableDefinitions = NULL;
    m_pTableMediator = NULL;
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableContainer::disposing( const ::com::sun::star::lang::EventObject& /*Source*/ ) throw (::com::sun::star::uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > OTableContainer::getTableTypeFilter(const Sequence< ::rtl::OUString >& _rTableTypeFilter) const
{
    Sequence< ::rtl::OUString > sTableTypes;
    if ( _rTableTypeFilter.getLength() == 0 )
    {
        getAllTableTypeFilter( sTableTypes );
    }
    else
    {
        sTableTypes = _rTableTypeFilter;
    }
    return sTableTypes;
}
// -----------------------------------------------------------------------------
void OTableContainer::addMasterContainerListener()
{
    // we have to listen at the mastertables because it could happen that another inserts new tables
    Reference<XContainer> xCont(m_xMasterContainer,UNO_QUERY);
    if(xCont.is())
        xCont->addContainerListener(this);
}
// -----------------------------------------------------------------------------
// two ways to obtain all tables from XDatabaseMetaData::getTables, via passing a particular
// table type filter:
// adhere to the standard, which requests to pass a NULL table type filter, if
// you want to retrieve all tables
#define FILTER_MODE_STANDARD 0
// only pass %, which is not allowed by the standard, but understood by some drivers
#define FILTER_MODE_WILDCARD 1
// only pass TABLE and VIEW
#define FILTER_MODE_FIXED    2
// do the thing which showed to be the safest way, understood by nearly all
// drivers, even the ones which do not understand the standard
#define FILTER_MODE_MIX_ALL  3

void OTableContainer::getAllTableTypeFilter( Sequence< ::rtl::OUString >& /* [out] */ _rFilter ) const
{
    sal_Int32 nFilterMode = FILTER_MODE_MIX_ALL;
        // for compatibility reasons, this is the default: we used this way before we
        // introduced the TableTypeFilterMode setting

    // obtain the data source we belong to, and the TableTypeFilterMode setting
    Any aFilterModeSetting;
    if ( getDataSourceSetting( getDataSource( (Reference< XInterface >)m_rParent ), "TableTypeFilterMode", aFilterModeSetting ) )
    {
        OSL_VERIFY( aFilterModeSetting >>= nFilterMode );
    }

    const ::rtl::OUString sAll( RTL_CONSTASCII_USTRINGPARAM( "%" ) );
    const ::rtl::OUString sView( RTL_CONSTASCII_USTRINGPARAM( "VIEW" ) );
    const ::rtl::OUString sTable( RTL_CONSTASCII_USTRINGPARAM( "TABLE" ) );

    switch ( nFilterMode )
    {
    default:
        OSL_ENSURE( sal_False, "OTableContainer::getAllTableTypeFilter: unknown TableTypeFilterMode!" );
    case FILTER_MODE_MIX_ALL:
        _rFilter.realloc( 3 );
        _rFilter[0] = sView;
        _rFilter[1] = sTable;
        _rFilter[2] = sAll;
        break;
    case FILTER_MODE_FIXED:
        _rFilter.realloc( 2 );
        _rFilter[0] = sView;
        _rFilter[1] = sTable;
        break;
    case FILTER_MODE_WILDCARD:
        _rFilter.realloc( 1 );
        _rFilter[0] = sAll;
        break;
    case FILTER_MODE_STANDARD:
        _rFilter.realloc( 0 );
        break;
    }
}

// -----------------------------------------------------------------------------

