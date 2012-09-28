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

#include "dbu_reghelper.hxx"
#include <sfx2/sfxsids.hrc>
#include "dbu_rel.hrc"
#include <vcl/svapp.hxx>
#include "browserids.hxx"
#include <comphelper/types.hxx>
#include "dbustrings.hrc"
#include <connectivity/dbtools.hxx>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdbc/SQLWarning.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbmetadata.hxx>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <comphelper/streamsection.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/seqstream.hxx>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include "sqlmessage.hxx"
#include "RelationController.hxx"
#include <vcl/msgbox.hxx>
#include "TableWindowData.hxx"
#include "UITools.hxx"
#include "RTableConnectionData.hxx"
#include "RelationTableView.hxx"
#include "RelationDesignView.hxx"
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/waitobj.hxx>
#include <osl/thread.hxx>
#include <osl/mutex.hxx>


#define MAX_THREADS 10

extern "C" void SAL_CALL createRegistryInfo_ORelationControl()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::ORelationController > aAutoRegistration;
}


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::util;
using namespace ::dbtools;
using namespace ::dbaui;
using namespace ::comphelper;
using namespace ::osl;

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORelationController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString ORelationController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.comp.dbu.ORelationDesign"));
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> ORelationController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.RelationDesign"));
    return aSupported;
}
//-------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL ORelationController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ORelationController::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new ORelationController(_rxFactory));
}
DBG_NAME(ORelationController);
// -----------------------------------------------------------------------------
ORelationController::ORelationController(const Reference< XMultiServiceFactory >& _rM)
    : OJoinController(_rM)
    ,m_nThreadEvent(0)
    ,m_bRelationsPossible(sal_True)
{
    DBG_CTOR(ORelationController,NULL);
    InvalidateAll();
}
// -----------------------------------------------------------------------------
ORelationController::~ORelationController()
{
    DBG_DTOR(ORelationController,NULL);
}
// -----------------------------------------------------------------------------
FeatureState ORelationController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    aReturn.bEnabled = m_bRelationsPossible;
    switch (_nId)
    {
        case SID_RELATION_ADD_RELATION:
            aReturn.bEnabled = !m_vTableData.empty() && isConnected() && isEditable();
            aReturn.bChecked = false;
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = haveDataSource() && impl_isModified();
            break;
        default:
            aReturn = OJoinController::GetState(_nId);
            break;
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void ORelationController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
{
    switch(_nId)
    {
        case ID_BROWSER_SAVEDOC:
            {
                OSL_ENSURE(isEditable(),"Slot ID_BROWSER_SAVEDOC should not be enabled!");
                if(!::dbaui::checkDataSourceAvailable(::comphelper::getString(getDataSource()->getPropertyValue(PROPERTY_NAME)), comphelper::getComponentContext(getORB())))
                {
                    String aMessage(ModuleRes(STR_DATASOURCE_DELETED));
                    OSQLWarningBox( getView(), aMessage ).Execute();
                }
                else
                {
                    // now we save the layout information
                    //  create the output stream
                    try
                    {
                        if ( haveDataSource() && getDataSource()->getPropertySetInfo()->hasPropertyByName(PROPERTY_LAYOUTINFORMATION) )
                        {
                            ::comphelper::NamedValueCollection aWindowsData;
                            saveTableWindows( aWindowsData );
                            getDataSource()->setPropertyValue( PROPERTY_LAYOUTINFORMATION, makeAny( aWindowsData.getPropertyValues() ) );
                            setModified(sal_False);
                        }
                    }
                    catch ( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }
            }
            break;
        case SID_RELATION_ADD_RELATION:
            static_cast<ORelationTableView*>(static_cast<ORelationDesignView*>( getView() )->getTableView())->AddNewRelation();
            break;
        default:
            OJoinController::Execute(_nId,aArgs);
            return;
    }
    InvalidateFeature(_nId);
}
// -----------------------------------------------------------------------------
void ORelationController::impl_initialize()
{
    OJoinController::impl_initialize();

    if( !getSdbMetaData().supportsRelations() )
    {// check if this database supports relations

        setEditable(sal_False);
        m_bRelationsPossible    = sal_False;
        {
            String sTitle(ModuleRes(STR_RELATIONDESIGN));
            sTitle.Erase(0,3);
            OSQLMessageBox aDlg(NULL,sTitle,ModuleRes(STR_RELATIONDESIGN_NOT_AVAILABLE));
            aDlg.Execute();
        }
        disconnect();
        throw SQLException();
    }

    if(!m_bRelationsPossible)
        InvalidateAll();

    // we need a datasource
    OSL_ENSURE(haveDataSource(),"ORelationController::initialize: need a datasource!");

    Reference<XTablesSupplier> xSup(getConnection(),UNO_QUERY);
    OSL_ENSURE(xSup.is(),"Connection isn't a XTablesSupplier!");
    if(xSup.is())
        m_xTables = xSup->getTables();
    // load the layoutInformation
    loadLayoutInformation();
    try
    {
        loadData();
        if ( !m_nThreadEvent )
            Application::PostUserEvent(LINK(this, ORelationController, OnThreadFinished));
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

}
// -----------------------------------------------------------------------------
::rtl::OUString ORelationController::getPrivateTitle( ) const
{
    ::rtl::OUString sName = getDataSourceName();
    return ::dbaui::getStrippedDatabaseName(getDataSource(),sName);
}
// -----------------------------------------------------------------------------
sal_Bool ORelationController::Construct(Window* pParent)
{
    setView( * new ORelationDesignView( pParent, *this, getORB() ) );
    OJoinController::Construct(pParent);
    return sal_True;
}
// -----------------------------------------------------------------------------
short ORelationController::saveModified()
{
    short nSaved = RET_YES;
    if(haveDataSource() && isModified())
    {
        QueryBox aQry(getView(), ModuleRes(RELATION_DESIGN_SAVEMODIFIED));
        nSaved = aQry.Execute();
        if(nSaved == RET_YES)
            Execute(ID_BROWSER_SAVEDOC,Sequence<PropertyValue>());
    }
    return nSaved;
}
// -----------------------------------------------------------------------------
void ORelationController::describeSupportedFeatures()
{
    OJoinController::describeSupportedFeatures();
    implDescribeSupportedFeature( ".uno:DBAddRelation", SID_RELATION_ADD_RELATION, CommandGroup::EDIT );
}
namespace
{
    class RelationLoader : public ::osl::Thread
    {
        DECLARE_STL_MAP(::rtl::OUString,::boost::shared_ptr<OTableWindowData>,::comphelper::UStringMixLess,TTableDataHelper);
        TTableDataHelper                    m_aTableData;
        TTableConnectionData                m_vTableConnectionData;
        const Sequence< ::rtl::OUString>    m_aTableList;
        ORelationController*                m_pParent;
        const Reference< XDatabaseMetaData> m_xMetaData;
        const Reference< XNameAccess >      m_xTables;
        const sal_Int32                     m_nStartIndex;
        const sal_Int32                     m_nEndIndex;

    public:
        RelationLoader(ORelationController* _pParent
                        ,const Reference< XDatabaseMetaData>& _xMetaData
                        ,const Reference< XNameAccess >& _xTables
                        ,const Sequence< ::rtl::OUString>& _aTableList
                        ,const sal_Int32 _nStartIndex
                        ,const sal_Int32 _nEndIndex)
            :m_aTableData(_xMetaData.is() && _xMetaData->supportsMixedCaseQuotedIdentifiers())
            ,m_aTableList(_aTableList)
            ,m_pParent(_pParent)
            ,m_xMetaData(_xMetaData)
            ,m_xTables(_xTables)
            ,m_nStartIndex(_nStartIndex)
            ,m_nEndIndex(_nEndIndex)
        {
        }

        /// Working method which should be overridden.
        virtual void SAL_CALL run();
        virtual void SAL_CALL onTerminated();
    protected:
        virtual ~RelationLoader(){}

        void loadTableData(const Any& _aTable);
    };

    void SAL_CALL RelationLoader::run()
    {
        const ::rtl::OUString* pIter = m_aTableList.getConstArray() + m_nStartIndex;
        for(sal_Int32 i = m_nStartIndex; i < m_nEndIndex;++i,++pIter)
        {
            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(m_xMetaData,
                                                *pIter,
                                                sCatalog,
                                                sSchema,
                                                sTable,
                                                ::dbtools::eInDataManipulation);
            Any aCatalog;
            if ( !sCatalog.isEmpty() )
                aCatalog <<= sCatalog;

            try
            {
                Reference< XResultSet > xResult = m_xMetaData->getImportedKeys(aCatalog, sSchema,sTable);
                if ( xResult.is() && xResult->next() )
                {
                    ::comphelper::disposeComponent(xResult);
                    loadTableData(m_xTables->getByName(*pIter));
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }
    void SAL_CALL RelationLoader::onTerminated()
    {
        m_pParent->mergeData(m_vTableConnectionData);
        delete this;
    }

    void RelationLoader::loadTableData(const Any& _aTable)
    {
        Reference<XPropertySet> xTableProp(_aTable,UNO_QUERY);
        const ::rtl::OUString sSourceName = ::dbtools::composeTableName( m_xMetaData, xTableProp, ::dbtools::eInTableDefinitions, false, false, false );
        TTableDataHelper::iterator aFind = m_aTableData.find(sSourceName);
        if ( aFind == m_aTableData.end() )
        {
            aFind = m_aTableData.insert(TTableDataHelper::value_type(sSourceName,::boost::shared_ptr<OTableWindowData>(new OTableWindowData(xTableProp,sSourceName, sSourceName)))).first;
            aFind->second->ShowAll(sal_False);
        }
        TTableWindowData::value_type pReferencingTable = aFind->second;
        Reference<XIndexAccess> xKeys = pReferencingTable->getKeys();
        const Reference<XKeysSupplier> xKeySup(xTableProp,UNO_QUERY);

        if ( !xKeys.is() && xKeySup.is() )
        {
            xKeys = xKeySup->getKeys();
        }

        if ( xKeys.is() )
        {
            Reference<XPropertySet> xKey;
            const sal_Int32 nCount = xKeys->getCount();
            for(sal_Int32 i = 0 ; i < nCount ; ++i)
            {
                xKeys->getByIndex(i) >>= xKey;
                sal_Int32 nKeyType = 0;
                xKey->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                if ( KeyType::FOREIGN == nKeyType )
                {
                    ::rtl::OUString sReferencedTable;
                    xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sReferencedTable;
                    //////////////////////////////////////////////////////////////////////
                    // insert windows
                    TTableDataHelper::iterator aRefFind = m_aTableData.find(sReferencedTable);
                    if ( aRefFind == m_aTableData.end() )
                    {
                        if ( m_xTables->hasByName(sReferencedTable) )
                        {
                            Reference<XPropertySet>  xReferencedTable(m_xTables->getByName(sReferencedTable),UNO_QUERY);
                            aRefFind = m_aTableData.insert(TTableDataHelper::value_type(sReferencedTable,::boost::shared_ptr<OTableWindowData>(new OTableWindowData(xReferencedTable,sReferencedTable, sReferencedTable)))).first;
                            aRefFind->second->ShowAll(sal_False);
                        }
                        else
                            continue; // table name could not be found so we do not show this table releation
                    }
                    TTableWindowData::value_type pReferencedTable = aRefFind->second;

                    ::rtl::OUString sKeyName;
                    xKey->getPropertyValue(PROPERTY_NAME) >>= sKeyName;
                    //////////////////////////////////////////////////////////////////////
                    // insert connection
                    ORelationTableConnectionData* pTabConnData = new ORelationTableConnectionData( pReferencingTable, pReferencedTable, sKeyName );
                    m_vTableConnectionData.push_back(TTableConnectionData::value_type(pTabConnData));
                    //////////////////////////////////////////////////////////////////////
                    // insert columns
                    const Reference<XColumnsSupplier> xColsSup(xKey,UNO_QUERY);
                    OSL_ENSURE(xColsSup.is(),"Key is no XColumnsSupplier!");
                    const Reference<XNameAccess> xColumns       = xColsSup->getColumns();
                    const Sequence< ::rtl::OUString> aNames = xColumns->getElementNames();
                    const ::rtl::OUString* pIter    = aNames.getConstArray();
                    const ::rtl::OUString* pEnd     = pIter + aNames.getLength();
                    ::rtl::OUString sColumnName,sRelatedName;
                    for(sal_uInt16 j=0;pIter != pEnd;++pIter,++j)
                    {
                        const Reference<XPropertySet> xPropSet(xColumns->getByName(*pIter),UNO_QUERY);
                        OSL_ENSURE(xPropSet.is(),"Invalid column found in KeyColumns!");
                        if ( xPropSet.is() )
                        {
                            xPropSet->getPropertyValue(PROPERTY_NAME)           >>= sColumnName;
                            xPropSet->getPropertyValue(PROPERTY_RELATEDCOLUMN)  >>= sRelatedName;
                        }
                        pTabConnData->SetConnLine( j, sColumnName, sRelatedName );
                    }
                    //////////////////////////////////////////////////////////////////////
                    // Update/Del-Flags setzen
                    sal_Int32   nUpdateRule = 0;
                    sal_Int32   nDeleteRule = 0;
                    xKey->getPropertyValue(PROPERTY_UPDATERULE) >>= nUpdateRule;
                    xKey->getPropertyValue(PROPERTY_DELETERULE) >>= nDeleteRule;

                    pTabConnData->SetUpdateRules( nUpdateRule );
                    pTabConnData->SetDeleteRules( nDeleteRule );

                    //////////////////////////////////////////////////////////////////////
                    // Kardinalitaet setzen
                    pTabConnData->SetCardinality();
                }
            }
        }
    }
}

void ORelationController::mergeData(const TTableConnectionData& _aConnectionData)
{
    ::osl::MutexGuard aGuard( getMutex() );

    ::std::copy( _aConnectionData.begin(), _aConnectionData.end(), ::std::back_inserter( m_vTableConnectionData ));
    //const Reference< XDatabaseMetaData> xMetaData = getConnection()->getMetaData();
    const sal_Bool bCase = sal_True;//xMetaData.is() && xMetaData->supportsMixedCaseQuotedIdentifiers();
    // here we are finished, so we can collect the table from connection data
    TTableConnectionData::iterator aConnDataIter = m_vTableConnectionData.begin();
    TTableConnectionData::iterator aConnDataEnd = m_vTableConnectionData.end();
    for(;aConnDataIter != aConnDataEnd;++aConnDataIter)
    {
        if ( !existsTable((*aConnDataIter)->getReferencingTable()->GetComposedName(),bCase) )
        {
            m_vTableData.push_back((*aConnDataIter)->getReferencingTable());
        }
        if ( !existsTable((*aConnDataIter)->getReferencedTable()->GetComposedName(),bCase) )
        {
            m_vTableData.push_back((*aConnDataIter)->getReferencedTable());
        }
    }
    if ( m_nThreadEvent )
    {
        --m_nThreadEvent;
        if ( !m_nThreadEvent )
            Application::PostUserEvent(LINK(this, ORelationController, OnThreadFinished));
    }
}
// -----------------------------------------------------------------------------
IMPL_LINK( ORelationController, OnThreadFinished, void*, /*NOTINTERESTEDIN*/ )
{
    ::SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    try
    {
        getView()->initialize();    // show the windows and fill with our informations
        getView()->Invalidate(INVALIDATE_NOERASE);
        ClearUndoManager();
        setModified(sal_False);     // and we are not modified yet

        if(m_vTableData.empty())
            Execute(ID_BROWSER_ADDTABLE,Sequence<PropertyValue>());
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    m_pWaitObject.reset();
    return 0L;
}
// -----------------------------------------------------------------------------
void ORelationController::loadData()
{
    m_pWaitObject.reset( new WaitObject(getView()) );
    try
    {
        if ( !m_xTables.is() )
            return;
        DatabaseMetaData aMeta(getConnection());
        // this may take some time
        const Reference< XDatabaseMetaData> xMetaData = getConnection()->getMetaData();
        const Sequence< ::rtl::OUString> aNames = m_xTables->getElementNames();
        const sal_Int32 nCount = aNames.getLength();
        if ( aMeta.supportsThreads() )
        {
            const sal_Int32 nMaxElements = (nCount / MAX_THREADS) +1;
            sal_Int32 nStart = 0,nEnd = ::std::min(nMaxElements,nCount);
            while(nStart != nEnd)
            {
                ++m_nThreadEvent;
                RelationLoader* pThread = new RelationLoader(this,xMetaData,m_xTables,aNames,nStart,nEnd);
                pThread->createSuspended();
                pThread->setPriority(osl_Thread_PriorityBelowNormal);
                pThread->resume();
                nStart = nEnd;
                nEnd += nMaxElements;
                nEnd = ::std::min(nEnd,nCount);
            }
        }
        else
        {
            RelationLoader* pThread = new RelationLoader(this,xMetaData,m_xTables,aNames,0,nCount);
            pThread->run();
            pThread->onTerminated();
        }
    }
    catch(SQLException& e)
    {
        showError(SQLExceptionInfo(e));
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
TTableWindowData::value_type ORelationController::existsTable(const ::rtl::OUString& _rComposedTableName,sal_Bool _bCase)  const
{
    ::comphelper::UStringMixEqual bCase(_bCase);
    TTableWindowData::const_iterator aIter = m_vTableData.begin();
    TTableWindowData::const_iterator aEnd = m_vTableData.end();
    for(;aIter != aEnd;++aIter)
    {
        if(bCase((*aIter)->GetComposedName(),_rComposedTableName))
            break;
    }
    return ( aIter != aEnd) ? *aIter : TTableWindowData::value_type();
}
// -----------------------------------------------------------------------------
void ORelationController::loadLayoutInformation()
{
    try
    {
        OSL_ENSURE(haveDataSource(),"We need a datasource from our connection!");
        if ( haveDataSource() )
        {
            if ( getDataSource()->getPropertySetInfo()->hasPropertyByName(PROPERTY_LAYOUTINFORMATION) )
            {
                Sequence<PropertyValue> aWindows;
                getDataSource()->getPropertyValue(PROPERTY_LAYOUTINFORMATION) >>= aWindows;
                loadTableWindows(aWindows);
            }
        }
    }
    catch(Exception&)
    {
    }
}
// -----------------------------------------------------------------------------
void ORelationController::reset()
{
    loadLayoutInformation();
    ODataView* pView = getView();
    OSL_ENSURE(pView,"No current view!");
    if(pView)
    {
        pView->initialize();
        pView->Invalidate(INVALIDATE_NOERASE);
    }
}

// -----------------------------------------------------------------------------
bool ORelationController::allowViews() const
{
    return false;
}

// -----------------------------------------------------------------------------
bool ORelationController::allowQueries() const
{
    return false;
}

// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
