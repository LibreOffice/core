/*************************************************************************
 *
 *  $RCSfile: RelationController.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:17:27 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef _DBAU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _DBU_REL_HRC_
#include "dbu_rel.hrc"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX
#include <vcl/toolbox.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XALTERTABLE_HPP_
#include <com/sun/star/sdbcx/XAlterTable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XKEYSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_SQLWARNING_HPP_
#include <com/sun/star/sdbc/SQLWarning.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef _COMPHELPER_SEQSTREAM_HXX
#include <comphelper/seqstream.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef DBAUI_RELATIONCONTROLLER_HXX
#include "RelationController.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef DBAUI_TABLEWINDOWDATA_HXX
#include "TableWindowData.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_RTABLECONNECTIONDATA_HXX
#include "RTableConnectionData.hxx"
#endif
#ifndef DBAUI_RELATION_TABLEVIEW_HXX
#include "RelationTableView.hxx"
#endif
#ifndef DBAUI_RELATIONDESIGNVIEW_HXX
#include "RelationDesignView.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif

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
//  using namespace ::com::sun::star::sdbcx;
//  using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::dbaui;
using namespace ::comphelper;

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORelationController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString ORelationController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.ORelationDesign");
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> ORelationController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.RelationDesign");
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
    ,m_bRelationsPossible(sal_True)
{
    DBG_CTOR(ORelationController,NULL);
    m_bViewsAllowed = sal_False;
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
        case ID_RELATION_ADD_RELATION:
            aReturn.bEnabled = m_vTableData.size() > 1 && isConnected() && isEditable();
            aReturn.aState = ::cppu::bool2any(sal_False);
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = haveDataSource() && isModified();
            break;
        default:
            aReturn = OJoinController::GetState(_nId);
            break;
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void ORelationController::Execute(sal_uInt16 _nId)
{
    switch(_nId)
    {
        case ID_BROWSER_SAVEDOC:
            {
                OSL_ENSURE(isEditable(),"Slot ID_BROWSER_SAVEDOC should not be enabled!");
                if(!::dbaui::checkDataSourceAvailable(::comphelper::getString(getDataSource()->getPropertyValue(PROPERTY_NAME)),getORB()))
                {
                    String aMessage(ModuleRes(STR_DATASOURCE_DELETED));
                    String sTitle(ModuleRes(STR_STAT_WARNING));
                    OSQLMessageBox aMsg(getView(),sTitle,aMessage);
                    aMsg.Execute();
                }
                else
                {
                    // now we save the layout information
                    //  create the output stream
                    try
                    {
                        if ( haveDataSource() && getDataSource()->getPropertySetInfo()->hasPropertyByName(PROPERTY_LAYOUTINFORMATION) )
                        {
                            Sequence<PropertyValue> aWindows;
                            getDataSource()->getPropertyValue(PROPERTY_LAYOUTINFORMATION) >>= aWindows;
                            saveTableWindows(aWindows);
                            getDataSource()->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aWindows));
                            setModified(sal_False);
                        }
                    }
                    catch(Exception&)
                    {
                    }
                }
            }
            break;
        case ID_RELATION_ADD_RELATION:
            static_cast<ORelationTableView*>(static_cast<ORelationDesignView*>(m_pView)->getTableView())->AddNewRelation();
            break;
        default:
            OJoinController::Execute(_nId);
            return;
            break;
    }
    InvalidateFeature(_nId);
}
// -----------------------------------------------------------------------------
void ORelationController::impl_initialize( const Sequence< Any >& aArguments )
{
    PropertyValue aValue;
    const Any* pBegin   = aArguments.getConstArray();
    const Any* pEnd     = pBegin + aArguments.getLength();

    for(;pBegin != pEnd;++pBegin)
    {
        if((*pBegin >>= aValue) && (0 == aValue.Name.compareToAscii(PROPERTY_ACTIVECONNECTION)))
        {
            Reference< XConnection > xConn;
            aValue.Value >>= xConn;
            initializeConnection( xConn );
            break;
        }
    }

    if ( !ensureConnected( sal_False ) )
    {
        setEditable(sal_False);
        m_bRelationsPossible    = sal_False;
        {
            {
                String aMessage(ModuleRes(RID_STR_CONNECTION_LOST));
                ODataView* pWindow = getView();
                InfoBox(pWindow, aMessage).Execute();
            }
            throw SQLException();
        }
    }
    else if(getMetaData().is() && !getMetaData()->supportsIntegrityEnhancementFacility())
    {// check if this database supports relations

        setEditable(sal_False);
        m_bRelationsPossible    = sal_False;
        {
            OSQLMessageBox aDlg(getView(),ModuleRes(STR_RELATIONDESIGN),ModuleRes(STR_RELATIONDESIGN_NOT_AVAILABLE));
            aDlg.Execute();
        }
        throw SQLException();
    }

    OJoinController::impl_initialize(aArguments);

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
        getView()->initialize();    // show the windows and fill with our informations
        getView()->Invalidate(INVALIDATE_NOERASE);
        getUndoMgr()->Clear();      // clear all undo redo things
        setModified(sal_False);     // and we are not modified yet

        if(m_vTableData.empty())
            Execute(ID_BROWSER_ADDTABLE);
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "ORelationController::initialize: caught an exception!");
    }

}
// -----------------------------------------------------------------------------
void ORelationController::updateTitle()
{
    ::rtl::OUString sName;
    sName = String(ModuleRes(STR_RELATIONDESIGN));
    ::rtl::OUString sDataSourceName = getDataSourceName();
    sName += ::dbaui::getStrippedDatabaseName(getDataSource(),sDataSourceName) ;
    OGenericUnoController::setTitle(sName);
}
// -----------------------------------------------------------------------------
sal_Bool ORelationController::Construct(Window* pParent)
{
    m_pView = new ORelationDesignView(pParent,this,m_xMultiServiceFacatory);
    OJoinController::Construct(pParent);
//  m_pView->Construct();
//  m_pView->Show();
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
            Execute(ID_BROWSER_SAVEDOC);
    }
    return nSaved;
}
// -----------------------------------------------------------------------------
void ORelationController::AddSupportedFeatures()
{
    OJoinController::AddSupportedFeatures();
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DB/AddRelation")]  = ID_RELATION_ADD_RELATION;
}
// -----------------------------------------------------------------------------
ToolBox* ORelationController::CreateToolBox(Window* _pParent)
{
    return new ToolBox(_pParent, ModuleRes(RID_BRW_REALTIONDESIGN_TOOLBOX));
}
// -----------------------------------------------------------------------------
void ORelationController::loadData()
{
    WaitObject aWaitCursor(getView());
    try
    {
        if ( !m_xTables.is() )
            return;
        // this may take some time

        Reference< XDatabaseMetaData> xMetaData = getConnection()->getMetaData();
        Sequence< ::rtl::OUString> aNames = m_xTables->getElementNames();
        const ::rtl::OUString* pBegin = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            ::rtl::OUString sCatalog,sSchema,sTable;
            ::dbtools::qualifiedNameComponents(xMetaData,
                                                *pBegin,
                                                sCatalog,
                                                sSchema,
                                                sTable,
                                                ::dbtools::eInDataManipulation);
            Any aCatalog;
            if ( sCatalog.getLength() )
                aCatalog <<= sCatalog;

            Reference< XResultSet > xResult = xMetaData->getImportedKeys(aCatalog, sSchema,sTable);
            if ( xResult.is() && xResult->next() )
                loadTableData(m_xTables->getByName(*pBegin));
        }
    }
    catch(SQLException& e)
    {
        showError(SQLExceptionInfo(e));
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched!");
    }
}
// -----------------------------------------------------------------------------
void ORelationController::loadTableData(const Any& _aTable)
{
    Reference<XIndexAccess> xKeys;
    Reference<XKeysSupplier> xKeySup;
    _aTable >>= xKeySup;

    if ( xKeySup.is() )
    {
        xKeys = xKeySup->getKeys();
        if ( xKeys.is() )
        {
            Reference<XPropertySet> xKey;
            for(sal_Int32 i=0;i< xKeys->getCount();++i)
            {
                xKeys->getByIndex(i) >>= xKey;
                sal_Int32 nKeyType = 0;
                xKey->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                if ( KeyType::FOREIGN == nKeyType )
                {
                    ::rtl::OUString sSourceName,sReferencedTable;
                    Reference<XPropertySet> xTableProp(xKeySup,UNO_QUERY);

                    sSourceName = ::dbtools::composeTableName(getConnection()->getMetaData(),xTableProp,sal_False,::dbtools::eInTableDefinitions);
                    xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sReferencedTable;
                    //////////////////////////////////////////////////////////////////////
                    // insert windows
                    if ( !existsTable(sSourceName) )
                    {
                        OTableWindowData* pData = new OTableWindowData(sSourceName, sSourceName);
                        pData->ShowAll(FALSE);
                        m_vTableData.push_back(pData);
                    }

                    if ( !existsTable(sReferencedTable) )
                    {
                        OTableWindowData* pData = new OTableWindowData(sReferencedTable, sReferencedTable);
                        pData->ShowAll(FALSE);
                        m_vTableData.push_back(pData);
                    }

                    ::rtl::OUString sKeyName;
                    xKey->getPropertyValue(PROPERTY_NAME) >>= sKeyName;
                    //////////////////////////////////////////////////////////////////////
                    // insert connection
                    ORelationTableConnectionData* pTabConnData = new ORelationTableConnectionData( m_xTables, sSourceName, sReferencedTable, sKeyName );
                    m_vTableConnectionData.push_back(pTabConnData);
                    //////////////////////////////////////////////////////////////////////
                    // insert columns
                    Reference<XColumnsSupplier> xColsSup(xKey,UNO_QUERY);
                    OSL_ENSURE(xColsSup.is(),"Key is no XColumnsSupplier!");
                    Reference<XNameAccess> xColumns     = xColsSup->getColumns();
                    Sequence< ::rtl::OUString> aNames   = xColumns->getElementNames();
                    const ::rtl::OUString* pBegin   = aNames.getConstArray();
                    const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();
                    ::rtl::OUString sColumnName,sRelatedName;
                    for(sal_uInt16 j=0;pBegin != pEnd;++pBegin,++j)
                    {
                        Reference<XPropertySet> xPropSet;
                        xColumns->getByName(*pBegin) >>= xPropSet;
                        OSL_ENSURE(xPropSet.is(),"Invalid column found in KeyColumns!");
                        xPropSet->getPropertyValue(PROPERTY_NAME)           >>= sColumnName;
                        xPropSet->getPropertyValue(PROPERTY_RELATEDCOLUMN)  >>= sRelatedName;
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
// -----------------------------------------------------------------------------
sal_Bool ORelationController::existsTable(const ::rtl::OUString& _rComposedTableName)  const
{
    Reference<XDatabaseMetaData> xMeta = getConnection()->getMetaData();
    ::comphelper::UStringMixEqual bCase(xMeta.is() && xMeta->storesMixedCaseQuotedIdentifiers());
    ::std::vector<OTableWindowData*>::const_iterator aIter = m_vTableData.begin();
    for(;aIter != m_vTableData.end();++aIter)
    {
        if(bCase((*aIter)->GetComposedName(),_rComposedTableName))
            break;
    }
    return aIter != m_vTableData.end();
}

// -----------------------------------------------------------------------------
OTableWindowData* ORelationController::createTableWindowData()
{
    return new OTableWindowData();
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


