/*************************************************************************
 *
 *  $RCSfile: RelationController.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-23 09:08:27 $
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
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
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
#ifndef _COM_SUN_STAR_UI_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/XExecutableDialog.hpp>
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
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
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
using namespace ::com::sun::star::ui;
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
// -----------------------------------------------------------------------------
ORelationController::ORelationController(const Reference< XMultiServiceFactory >& _rM)
    : OJoinController(_rM)
    ,m_bRelationsPossible(sal_True)
{
    InvalidateAll();
}
// -----------------------------------------------------------------------------
ORelationController::~ORelationController()
{
}
// -----------------------------------------------------------------------------
FeatureState ORelationController::GetState(sal_uInt16 _nId)
{
    FeatureState aReturn;
    aReturn.bEnabled = m_bRelationsPossible;
    switch (_nId)
    {
        case ID_BROWSER_EDITDOC:
            aReturn.aState = ::cppu::bool2any(m_bEditable);
            break;
        case ID_REALTION_ADD_RELATION:
            aReturn.bEnabled = m_vTableData.size() > 1;
            aReturn.aState = ::cppu::bool2any(sal_False);
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
                OSL_ENSURE(m_bEditable,"Slot ID_BROWSER_SAVEDOC should not be enabled!");
                // now we save the layout information
                //  create the output stream
                try
                {

                    Sequence< sal_Int8 > aOutputSeq;
                    if(m_xDataSource->getPropertySetInfo()->hasPropertyByName(PROPERTY_LAYOUTINFORMATION))
                    {
                        Reference< XOutputStream>       xOutStreamHelper = new OSequenceOutputStream(aOutputSeq);
                        Reference< XObjectOutputStream> xOutStream(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")),UNO_QUERY);
                        Reference< XOutputStream>   xMarkOutStream(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")),UNO_QUERY);
                        Reference< XActiveDataSource >(xMarkOutStream,UNO_QUERY)->setOutputStream(xOutStreamHelper);
                        Reference< XActiveDataSource > xOutDataSource(xOutStream, UNO_QUERY);
                        OSL_ENSURE(xOutDataSource.is(),"Couldn't create com.sun.star.io.ObjectOutputStream!");
                        xOutDataSource->setOutputStream(xMarkOutStream);
                        Save(xOutStream);

                        m_xDataSource->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aOutputSeq));
                        Reference<XFlushable> xFlush(m_xDataSource,UNO_QUERY);
                        if(xFlush.is())
                            xFlush->flush();
                        setModified(sal_False);
                    }
                }
                catch(Exception&)
                {
                }
            }
            break;
        case ID_REALTION_ADD_RELATION:
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
void SAL_CALL ORelationController::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
{
    try
    {
        OGenericUnoController::initialize(aArguments);

        //  m_pWindow->initialize(m_xCurrentFrame);

        PropertyValue aValue;
        const Any* pBegin   = aArguments.getConstArray();
        const Any* pEnd     = pBegin + aArguments.getLength();

        for(;pBegin != pEnd;++pBegin)
        {
            if((*pBegin >>= aValue) && aValue.Name == PROPERTY_ACTIVECONNECTION)
            {
                aValue.Value >>= m_xConnection;
                OSL_ENSURE(m_xConnection.is(),"We need at least a connection!");
                // be notified when connection is in disposing
                Reference< XComponent >  xComponent(m_xConnection, UNO_QUERY);
                if (xComponent.is())
                {
                    Reference< ::com::sun::star::lang::XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
                    xComponent->addEventListener(xEvtL);
                }
            }
            else if(aValue.Name == PROPERTY_DATASOURCENAME)
            {
                aValue.Value >>= m_sDataSourceName;
            }
        }

        if (!m_xConnection.is())
        {   // whoever instantiated us did not give us a connection to share. Okay, create an own one
            createNewConnection(sal_False);
        }
        if(!m_xConnection.is()) // so what should otherwise
        {
            m_bEditable             = sal_False;
            m_bRelationsPossible    = sal_False;
            {
                String aMessage(ModuleRes(RID_STR_CONNECTION_LOST));
                ODataView* pWindow = getView();
                InfoBox(pWindow, aMessage).Execute();
            }
        }
        else if(!m_xConnection->getMetaData()->supportsIntegrityEnhancementFacility())
        {// check if this database supports relations

            m_bEditable             = sal_False;
            m_bRelationsPossible    = sal_False;
            {
                OSQLMessageBox aDlg(getView(),ModuleRes(STR_RELATIONDESIGN),ModuleRes(STR_RELATIONDESIGN_NOT_AVAILABLE));
                aDlg.Execute();
            }
        }
        if(!m_bRelationsPossible)
            InvalidateAll();

        // we need a datasource
        if(m_xConnection.is())
        {
            Reference<XChild> xChild(m_xConnection,UNO_QUERY);
            if(xChild.is())
                m_xDataSource = Reference< XPropertySet >(xChild->getParent(),UNO_QUERY);
        }
        else
        {
            Reference<XNameAccess> xDatabaseContext = Reference< XNameAccess >(getORB()->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
            xDatabaseContext->getByName(m_sDataSourceName) >>= m_xDataSource;
            OSL_ENSURE(m_xDataSource.is(),"We need a datasource!");
        }

        Reference<XTablesSupplier> xSup(m_xConnection,UNO_QUERY);
        OSL_ENSURE(xSup.is(),"Connection isn't a XTablesSupplier!");
        if(xSup.is())
            m_xTables = xSup->getTables();
        // load the layoutInformation
        try
        {
            OSL_ENSURE(m_xDataSource.is(),"We need a datasource from our connection!");
            if(m_xDataSource.is())
            {
                Sequence< sal_Int8 > aInputSequence;
                if(m_xDataSource->getPropertySetInfo()->hasPropertyByName(PROPERTY_LAYOUTINFORMATION))
                {
                    m_xDataSource->getPropertyValue(PROPERTY_LAYOUTINFORMATION) >>= aInputSequence;
                    {
                        Reference< XInputStream>       xInStreamHelper = new SequenceInputStream(aInputSequence);;  // used for wrapping sequence to xinput
                        Reference< XObjectInputStream> xInStream = Reference< XObjectInputStream >(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectInputStream")),UNO_QUERY);
                        Reference< XInputStream> xMarkInStream = Reference< XInputStream >(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableInputStream")),UNO_QUERY);
                        Reference< XActiveDataSink >(xMarkInStream,UNO_QUERY)->setInputStream(xInStreamHelper);
                        Reference< XActiveDataSink >   xInDataSource(xInStream, UNO_QUERY);
                        OSL_ENSURE(xInDataSource.is(),"Couldn't create com.sun.star.io.ObjectInputStream!");
                        xInDataSource->setInputStream(xMarkInStream);
                        Load(xInStream);
                    }
                }
            }
        }
        catch(Exception&)
        {
        }

        loadData();
        getView()->initialize();    // show the windows and fill with our informations
        getUndoMgr()->Clear();      // clear all undo redo things
        setModified(sal_False);     // and we are not modified yet
    }
    catch(SQLException&)
    {
        OSL_ASSERT(0);
    }

}
// -----------------------------------------------------------------------------
sal_Bool ORelationController::Construct(Window* pParent)
{
    m_pView = new ORelationDesignView(pParent,this,m_xMultiServiceFacatory);
    OGenericUnoController::Construct(pParent);
    m_pView->Construct(NULL);
    m_pView->Show();
    return sal_True;
}
// -----------------------------------------------------------------------------
void SAL_CALL ORelationController::propertyChange(const PropertyChangeEvent& evt)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL ORelationController::elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( RuntimeException )
{
}
// -----------------------------------------------------------------------------
void SAL_CALL ORelationController::elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( RuntimeException )
{
}
// -----------------------------------------------------------------------------
void SAL_CALL ORelationController::elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( RuntimeException )
{
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ORelationController::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    if(m_xConnection.is() && m_bModified)
    {
        QueryBox aQry(getView(), ModuleRes(RELATION_DESIGN_SAVEMODIFIED));
        switch (aQry.Execute())
        {
            case RET_YES:
                Execute(ID_BROWSER_SAVEDOC);
                break;
            case RET_CANCEL:
                return sal_False;
            default:
                break;
        }
    }
    return sal_True;
}
// -----------------------------------------------------------------------------
void ORelationController::AddSupportedFeatures()
{
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Redo")] = ID_BROWSER_REDO;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Save")] = ID_BROWSER_SAVEDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Undo")] = ID_BROWSER_UNDO;

    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:BrowserMode")] = SID_BROWSER_MODE;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:HelpMenu")]    = SID_HELPMENU;
    //  m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:NewDoc")]      = SID_NEWDOC;
    //  m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:SaveAsDoc")]   = SID_SAVEASDOC;
    //  m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:ExplorerContentOpen")]         = SID_EXPLORERCONTENT_OPEN;
    //  m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:ExplorerContentOpenDocument")] = SID_EXPLORERCONTENT_OPEN_DOCUMENT;

}
// -----------------------------------------------------------------------------
ToolBox* ORelationController::CreateToolBox(Window* _pParent)
{
    return new ToolBox(_pParent, ModuleRes(RID_BRW_REALTIONDESIGN_TOOLBOX));
}
// -----------------------------------------------------------------------------
void ORelationController::loadData()
{
    try
    {
        if(!m_xTables.is())
            return;

        Sequence< ::rtl::OUString> aNames = m_xTables->getElementNames();
        const ::rtl::OUString* pBegin = aNames.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + aNames.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            Reference<XKeysSupplier> xKeySup;
            m_xTables->getByName(*pBegin) >>= xKeySup;
            Reference<XIndexAccess> xKeys;
            if(xKeySup.is())
                xKeys = xKeySup->getKeys();
            if(xKeys.is())
            {
                Reference<XPropertySet> xKey;
                for(sal_Int32 i=0;i< xKeys->getCount();++i)
                {
                    xKeys->getByIndex(i) >>= xKey;
                    sal_Int32 nKeyType = 0;
                    xKey->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                    if(KeyType::FOREIGN == nKeyType)
                    {
                        ::rtl::OUString sSourceName,sReferencedTable;
                        Reference<XPropertySet> xTableProp(xKeySup,UNO_QUERY);

                        ::dbaui::composeTableName(m_xConnection->getMetaData(),xTableProp,sSourceName,sal_False);
                        xKey->getPropertyValue(PROPERTY_REFERENCEDTABLE) >>= sReferencedTable;
                        //////////////////////////////////////////////////////////////////////
                        // insert windows
                        if( !existsTable(sSourceName) )
                        {
                            OTableWindowData* pData = new OTableWindowData(sSourceName, sSourceName);
                            pData->ShowAll(FALSE);
                            m_vTableData.push_back(pData);
                        }

                        if( !existsTable(sReferencedTable) )
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
                            xPropSet->getPropertyValue(PROPERTY_NAME)           >>= sColumnName;
                            xPropSet->getPropertyValue(PROPERTY_RELATEDCOLUMN)  >>= sRelatedName;
                            pTabConnData->SetConnLine( j, sColumnName, sRelatedName );
                        }
                        //////////////////////////////////////////////////////////////////////
                        // Update/Del-Flags setzen
                        sal_Int32   nUpdateRule,
                                    nDeleteRule;
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
    catch(SQLException& e)
    {
        showError(SQLExceptionInfo(e));
    }
    catch(Exception&)
    {
    }

}
// -----------------------------------------------------------------------------
sal_Bool ORelationController::existsTable(const ::rtl::OUString& _rComposedTableName)  const
{
    ::comphelper::UStringMixEqual bCase(m_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
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
//void ORelationController::appendColumns(Reference<XColumnsSupplier>& _rxColSup,sal_Bool _bKeyColumns)
//{
//  try
//  {
//      // now append the columns
//      OSL_ENSURE(_rxColSup.is(),"No columns supplier");
//      if(!_rxColSup.is())
//          return;
//      Reference<XNameAccess> xColumns = _rxColSup->getColumns();
//      OSL_ENSURE(xColumns.is(),"No columns");
//      Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);
//
//      Reference<XAppend> xAppend(xColumns,UNO_QUERY);
//      OSL_ENSURE(xAppend.is(),"No XAppend Interface!");
//
//      ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
//      for(;aIter != m_vRowList.end();++aIter)
//      {
//          OSL_ENSURE(*aIter,"OTableRow is null!");
//          OFieldDescription* pField = (*aIter)->GetActFieldDescr();
//          if(!pField)
//              continue;
//
//          Reference<XPropertySet> xColumn;
//          if(pField->IsPrimaryKey() || !_bKeyColumns)
//              xColumn = xColumnFactory->createDataDescriptor();
//          if(xColumn.is())
//          {
//              if(!_bKeyColumns)
//                  setColumnProperties(xColumn,pField);
//              else
//                  xColumn->setPropertyValue(PROPERTY_NAME,makeAny(pField->GetName()));
//
//              xAppend->appendByDescriptor(xColumn);
//              xColumn = NULL;
//              // now only the settings are missing
//              if(xColumns->hasByName(pField->GetName()))
//              {
//                  xColumns->getByName(pField->GetName()) >>= xColumn;
//                  if(xColumn.is())
//                  {
//                      if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
//                          xColumn->setPropertyValue(PROPERTY_FORMATKEY,makeAny(pField->GetFormatKey()));
//                      if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
//                          xColumn->setPropertyValue(PROPERTY_ALIGN,makeAny((sal_Int32)pField->GetHorJustify()));
//                  }
//              }
//              else
//              {
//                  OSL_ASSERT(0);
//              }
//
//          }
//      }
//  }
//  catch(SQLException& e)
//  {
//      showError(SQLExceptionInfo(e));
//  }
//  catch(Exception&)
//  {
//      OSL_ASSERT(0);
//  }
//}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//void ORelationController::alterColumns()
//{
//  Reference<XColumnsSupplier> xColSup(m_xTable,UNO_QUERY);
//  OSL_ENSURE(xColSup.is(),"What happen here?!");
//
//  Reference<XNameAccess> xColumns = xColSup->getColumns();
//  OSL_ENSURE(xColumns.is(),"No columns");
//  Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);
//  Reference<XDrop> xDrop(xColumns,UNO_QUERY);
//  Reference<XAppend> xAppend(xColumns,UNO_QUERY);
//  Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);
//
//  sal_Bool bReload = sal_False; // refresh the data
//
//  ::std::map< ::rtl::OUString,sal_Bool,::comphelper::UStringMixLess> aColumns(m_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers());
//  ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
//  for(;aIter != m_vRowList.end();++aIter)
//  {
//      OSL_ENSURE(*aIter,"OTableRow is null!");
//      OFieldDescription* pField = (*aIter)->GetActFieldDescr();
//      if(!pField)
//          continue;
//
//      Reference<XPropertySet> xColumn;
//      if(xColumns->hasByName(pField->GetName()))
//      {
//          aColumns[pField->GetName()] = sal_True;
//          xColumns->getByName(pField->GetName()) >>= xColumn;
//          OSL_ENSURE(xColumn.is(),"Column is null!");
//
//          sal_Int32 nType,nPrecision,nScale,nNullable,nFormatKey,nAlignment;
//          sal_Bool bAutoIncrement;
//          ::rtl::OUString sDescription,sDefaultValue;
//
//          xColumn->getPropertyValue(PROPERTY_TYPE)            >>= nType;
//          xColumn->getPropertyValue(PROPERTY_PRECISION)       >>= nPrecision;
//          xColumn->getPropertyValue(PROPERTY_SCALE)           >>= nScale;
//          xColumn->getPropertyValue(PROPERTY_ISNULLABLE)      >>= nNullable;
//          bAutoIncrement = ::cppu::any2bool(xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT));
//          //  xColumn->getPropertyValue(PROPERTY_ISCURRENCY,::cppu::bool2any(pField->IsCurrency()));
//          if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION))
//              xColumn->getPropertyValue(PROPERTY_DESCRIPTION) >>= sDescription;
//          if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_DEFAULTVALUE))
//              xColumn->getPropertyValue(PROPERTY_DEFAULTVALUE) >>= sDefaultValue;
//          if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
//              xColumn->getPropertyValue(PROPERTY_FORMATKEY)   >>= nFormatKey;
//          if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
//              xColumn->getPropertyValue(PROPERTY_ALIGN)       >>= nAlignment;
//
//          // check if something changed
//          if((nType != pField->GetType()                  ||
//              nPrecision != pField->GetPrecision()        ||
//              nScale != pField->GetScale()                ||
//              nNullable != pField->GetIsNullable()        ||
//              bAutoIncrement != pField->IsAutoIncrement() ||
//              sDescription != pField->GetDescription()    ||
//              sDefaultValue != pField->GetDefaultValue()) &&
//              xColumnFactory.is())
//          {
//              Reference<XPropertySet> xNewColumn;
//              xNewColumn = xColumnFactory->createDataDescriptor();
//              setColumnProperties(xNewColumn,pField);
//              // first try to alter the column
//              sal_Bool bNotOk = sal_False;
//              try
//              {
//                  if(xAlter.is())
//                      xAlter->alterColumnByName(pField->GetName(),xNewColumn);
//              }
//              catch(SQLException&)
//              {
//                  bNotOk = sal_True;
//              }
//              if((!xAlter.is() || bNotOk) && xDrop.is() && xAppend.is())
//              {
//                  xDrop->dropByName(pField->GetName());
//                  xAppend->appendByDescriptor(xNewColumn);
//              }
//              xNewColumn = NULL;
//              xColumns->getByName(pField->GetName()) >>= xColumn;
//              bReload = sal_True;
//          }
//
//          if(nFormatKey != pField->GetFormatKey())
//          {
//              if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
//                  xColumn->setPropertyValue(PROPERTY_FORMATKEY,makeAny(pField->GetFormatKey()));
//          }
//          if(nAlignment != (sal_Int32)pField->GetHorJustify())
//          {
//              if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
//                  xColumn->setPropertyValue(PROPERTY_ALIGN,makeAny(nAlignment));
//          }
//      }
//      else if(xColumnFactory.is())
//      {
//          // Column is new
//          xColumn = xColumnFactory->createDataDescriptor();
//          setColumnProperties(xColumn,pField);
//          xAppend->appendByDescriptor(xColumn);
//          if(xColumns->hasByName(pField->GetName()))
//          {
//              xColumns->getByName(pField->GetName()) >>= xColumn;
//              if(xColumn.is())
//              {
//                  if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
//                      xColumn->setPropertyValue(PROPERTY_FORMATKEY,makeAny(pField->GetFormatKey()));
//                  if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
//                      xColumn->setPropertyValue(PROPERTY_ALIGN,makeAny((sal_Int32)pField->GetHorJustify()));
//              }
//          }
//          else
//          {
//              OSL_ASSERT(0);
//          }
//      }
//  }
//
//  Reference<XNameAccess> xKeyColumns  = getKeyColumns();
//  // now we have to look for the columns who where deleted
//  if(xDrop.is())
//  {
//      Sequence< ::rtl::OUString> aColumnNames = xColumns->getElementNames();
//      const ::rtl::OUString* pBegin = aColumnNames.getConstArray();
//      const ::rtl::OUString* pEnd = pBegin + aColumnNames.getLength();
//      for(;pBegin != pEnd;++pBegin)
//      {
//          if(aColumns.find(*pBegin) == aColumns.end()) // found a column to delete
//          {
//              if(xKeyColumns.is() && xKeyColumns->hasByName(*pBegin)) // check if this column is a memeber of the primary key
//              {
//                  String aMsgT(ModuleRes(STR_TBL_COLUMN_IS_KEYCOLUMN));
//                  aMsgT.SearchAndReplace(String::CreateFromAscii("\"$column$\""),*pBegin);
//                  String aTitle(ModuleRes(STR_TBL_COLUMN_IS_KEYCOLUMN_TITLE));
//                  OSQLMessageBox aMsg(getView(),aTitle,aMsgT,WB_YES_NO| WB_DEF_YES);
//                  if(aMsg.Execute() == RET_YES)
//                  {
//                      xKeyColumns = NULL;
//                      dropKey();
//                  }
//                  else
//                  {
//                      bReload = sal_True;
//                      continue;
//                  }
//              }
//              Reference<XDrop> xDrop(xColumns,UNO_QUERY);
//              xDrop->dropByName(*pBegin);
//          }
//      }
//  }
//  // check if we have to do something with the primary key
//  if(xKeyColumns.is())
//  {
//      aIter = m_vRowList.begin();
//      for(;aIter != m_vRowList.end();++aIter)
//      {
//          OSL_ENSURE(*aIter,"OTableRow is null!");
//          OFieldDescription* pField = (*aIter)->GetActFieldDescr();
//          if(!pField)
//              continue;
//          if(pField->IsPrimaryKey() && !xKeyColumns->hasByName(pField->GetName()))
//          {
//              dropKey();
//              Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
//              appendKey(xKeySup);
//              break;
//          }
//      }
//  }
//  else
//  {
//      Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
//      appendKey(xKeySup);
//  }
//
//  if(bReload)
//  {
//      loadData();                 // fill the column information form the table
//      getView()->initialize();    // show the windows and fill with our informations
//      getUndoMgr()->Clear();      // clear all undo redo things
//      setModified(sal_False);     // and we are not modified yet
//  }
//}
// -----------------------------------------------------------------------------
//void ORelationController::dropKey()
//{
//  Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
//  Reference<XIndexAccess> xKeys;
//  if(xKeySup.is())
//      xKeys = xKeySup->getKeys();
//
//  if(xKeys.is())
//  {
//      Reference<XPropertySet> xProp;
//      for(sal_Int32 i=0;i< xKeys->getCount();++i)
//      {
//          xKeys->getByIndex(i) >>= xProp;
//          sal_Int32 nKeyType = 0;
//          xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
//          if(KeyType::PRIMARY == nKeyType)
//          {
//              Reference<XDrop> xDrop(xKeys,UNO_QUERY);
//              xDrop->dropByIndex(i); // delete the key
//              break;
//          }
//      }
//  }
//}
// -----------------------------------------------------------------------------






