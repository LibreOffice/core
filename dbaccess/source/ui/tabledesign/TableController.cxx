/*************************************************************************
 *
 *  $RCSfile: TableController.cxx,v $
 *
 *  $Revision: 1.91 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:18:21 $
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
#ifndef DBUI_TABLECONTROLLER_HXX
#include "TableController.hxx"
#endif

#ifndef _DBAU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SFXSIDS_HRC
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _DBU_TBL_HRC_
#include "dbu_tbl.hrc"
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
#ifndef DBAUI_DLGSAVE_HXX
#include "dlgsave.hxx"
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
#ifndef _COM_SUN_STAR_SDBCX_XINDEXESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XIndexesSupplier.hpp>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASOURCE_HPP_
#include <com/sun/star/io/XActiveDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XACTIVEDATASINK_HPP_
#include <com/sun/star/io/XActiveDataSink.hpp>
#endif
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#include "TableDesignView.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef DBAUI_FIELDDESCRIPTIONS_HXX
#include "FieldDescriptions.hxx"
#endif
#ifndef DBAUI_TABLEROW_HXX
#include "TableRow.hxx"
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef DBAUI_TABLEEDITORCONTROL_HXX
#include "TEditControl.hxx"
#endif
#ifndef DBAUI_TABLEDESCRIPTIONWINDOW_HXX
#include "TableDescWin.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _DBAUI_INDEXDIALOG_HXX_
#include "indexdialog.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif

extern "C" void SAL_CALL createRegistryInfo_OTableControl()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OTableController > aAutoRegistration;
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
using namespace ::dbtools;
using namespace ::dbaui;
using namespace ::comphelper;

namespace
{
    void dropTable(const Reference<XNameAccess>& _rxTable,const ::rtl::OUString& _sTableName)
    {
        if ( _rxTable->hasByName(_sTableName) )
        {
            Reference<XDrop> xNameCont(_rxTable,UNO_QUERY);
            OSL_ENSURE(xNameCont.is(),"No drop interface for tables!");
            if ( xNameCont.is() )
                xNameCont->dropByName(_sTableName);
        }
    }
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTableController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString OTableController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.OTableDesign");
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> OTableController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.TableDesign");
    return aSupported;
}
//-------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL OTableController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OTableController::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new OTableController(_rxFactory));
}

// -----------------------------------------------------------------------------
OTableController::OTableController(const Reference< XMultiServiceFactory >& _rM) : OTableController_BASE(_rM)
    ,m_sTypeNames(ModuleRes(STR_TABLEDESIGN_DBFIELDTYPES))
    ,m_bNew(sal_True)
    ,m_pTypeInfo(NULL)
    ,m_bAllowAutoIncrementValue(sal_False)
{
    InvalidateAll();
    m_pTypeInfo = TOTypeInfoSP(new OTypeInfo());
    m_pTypeInfo->aUIName = m_sTypeNames.GetToken(TYPE_OTHER);
}
// -----------------------------------------------------------------------------
OTableController::~OTableController()
{
    m_aTypeInfoIndex.clear();
    m_aTypeInfo.clear();
}

// -----------------------------------------------------------------------------
void OTableController::startTableListening()
{
    Reference< XComponent >  xComponent(m_xTable, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(static_cast<XModifyListener*>(this));
}

// -----------------------------------------------------------------------------
void OTableController::stopTableListening()
{
    Reference< XComponent >  xComponent(m_xTable, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(static_cast<XModifyListener*>(this));
}

// -----------------------------------------------------------------------------
void OTableController::disposing()
{
    OTableController_BASE::disposing();
    m_pView     = NULL;

    ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
    for(;aIter != m_vRowList.end();++aIter)
        delete *aIter;
}
// -----------------------------------------------------------------------------
FeatureState OTableController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    // (disabled automatically)

    switch (_nId)
    {
        case ID_BROWSER_CLOSE:
            aReturn.bEnabled = sal_True;
            break;
        case ID_TABLE_DESIGN_NO_CONNECTION:
            aReturn.aState = ::cppu::bool2any( isConnected() );
            break;
        case ID_BROWSER_EDITDOC:
            aReturn.aState = ::cppu::bool2any(isEditable());
            aReturn.bEnabled = m_bNew || isEditable();// the editable flag is set through this one -> || isAddAllowed() || isDropAllowed() || isAlterAllowed();
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = isModified();
            if ( aReturn.bEnabled )
            {
                ::std::vector<OTableRow*>::const_iterator aIter = ::std::find_if(m_vRowList.begin(),m_vRowList.end(),::std::mem_fun(&OTableRow::isValid));
                aReturn.bEnabled = aIter != m_vRowList.end();
            }
            break;
        case ID_BROWSER_SAVEASDOC:
            aReturn.bEnabled = isConnected() && isEditable();
            if ( aReturn.bEnabled )
            {
                ::std::vector<OTableRow*>::const_iterator aIter = ::std::find_if(m_vRowList.begin(),m_vRowList.end(),::std::mem_fun(&OTableRow::isValid));
                aReturn.bEnabled = aIter != m_vRowList.end();
            }
            break;

        case ID_BROWSER_CUT:
            aReturn.bEnabled = isEditable() && m_bFrameUiActive && getView() && static_cast<OTableDesignView*>(getView())->isCutAllowed();
            break;
        case ID_BROWSER_COPY:
            aReturn.bEnabled = m_bFrameUiActive && getView() && static_cast<OTableDesignView*>(getView())->isCopyAllowed();
            break;
        case ID_BROWSER_PASTE:
            aReturn.bEnabled = isEditable() && m_bFrameUiActive && getView() && static_cast<OTableDesignView*>(getView())->isPasteAllowed();
            break;
        case SID_INDEXDESIGN:
            aReturn.bEnabled =
                (   (   ((!m_bNew && isModified()) || isModified())
                    ||  Reference< XIndexesSupplier >(m_xTable, UNO_QUERY).is()
                    )
                &&  isConnected()
                );
            if ( aReturn.bEnabled )
            {
                ::std::vector<OTableRow*>::const_iterator aIter = ::std::find_if(m_vRowList.begin(),m_vRowList.end(),::std::mem_fun(&OTableRow::isValid));
                aReturn.bEnabled = aIter != m_vRowList.end();
            }
            break;
        default:
            aReturn = OTableController_BASE::GetState(_nId);
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void OTableController::Execute(sal_uInt16 _nId)
{
    switch(_nId)
    {
        case ID_TABLE_DESIGN_NO_CONNECTION:
            if (!isConnected())
                reconnect( sal_False );
            break;
        case ID_BROWSER_EDITDOC:
            setEditable(!isEditable());
            static_cast<OTableDesignView*>(getView())->setReadOnly(!isEditable());
            InvalidateFeature(ID_BROWSER_PASTE);
            InvalidateFeature(ID_BROWSER_CLEAR_QUERY);
            break;
        case ID_BROWSER_SAVEASDOC:
            doSaveDoc(sal_True);
            break;
        case ID_BROWSER_SAVEDOC:
            static_cast<OTableDesignView*>(getView())->GetEditorCtrl()->SaveCurRow();
            doSaveDoc(sal_False);
            break;
        case ID_BROWSER_CUT:
            static_cast<OTableDesignView*>(getView())->cut();
            break;
        case ID_BROWSER_COPY:
            static_cast<OTableDesignView*>(getView())->copy();
            break;
        case ID_BROWSER_PASTE:
            static_cast<OTableDesignView*>(getView())->paste();
            break;
        case SID_INDEXDESIGN:
            doEditIndexes();
            break;
        default:
            OTableController_BASE::Execute(_nId);
    }
    InvalidateFeature(_nId);
}

// -----------------------------------------------------------------------------
sal_Bool OTableController::doSaveDoc(sal_Bool _bSaveAs)
{
    if (!isConnected())
        reconnect(sal_True); // ask the user for a new connection
    Reference<XTablesSupplier> xTablesSup(getConnection(),UNO_QUERY);

    if (!xTablesSup.is())
    {
        String aMessage(ModuleRes(STR_TABLEDESIGN_CONNECTION_MISSING));
        String sTitle(ModuleRes(STR_STAT_WARNING));
        OSQLMessageBox aMsg(getView(),sTitle,aMessage);
        aMsg.Execute();
        return sal_False;
    }

    // check if a column exists
    // TODO

    Reference<XNameAccess> xTables;
    ::rtl::OUString sCatalog, sSchema;

    sal_Bool bNew = (0 == m_sName.getLength());
    bNew = bNew || m_bNew || _bSaveAs;

    try
    {
        xTables = xTablesSup->getTables();
        OSL_ENSURE(xTables.is(),"The tables can't be null!");
        bNew = bNew || (xTables.is() && !xTables->hasByName(m_sName));

        // first we need a name for our query so ask the user
        if(bNew)
        {
            String aDefaultName;
            if (_bSaveAs && !bNew)
                 aDefaultName = String(m_sName);
            else
            {
                String aName = String(ModuleRes(STR_TBL_TITLE));
                aName = aName.GetToken(0,' ');
                aDefaultName = ::dbaui::createDefaultName(getConnection()->getMetaData(),xTables,aName);
            }

            OSaveAsDlg aDlg(getView(),CommandType::TABLE,xTables,getConnection()->getMetaData(),getConnection(),aDefaultName);
            if(aDlg.Execute() == RET_OK)
            {
                m_sName = aDlg.getName();
                sCatalog = aDlg.getCatalog();
                sSchema  = aDlg.getSchema();
            }
            else
                return sal_False;
        }

        // did we get a name
        if(!m_sName.getLength())
            return sal_False;
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "OTableController::doSaveDoc: nothing is expected to happen here!");
    }

    sal_Bool bAlter = sal_False;
    sal_Bool bError = sal_False;
    SQLExceptionInfo aInfo;
    try
    {
        // check the columns for double names
        if(!checkColumns(bNew || !xTables->hasByName(m_sName)))
        {
            // #105323# OJ
            return sal_False;
        }

        Reference<XPropertySet> xTable;
        if(bNew || !xTables->hasByName(m_sName)) // just to make sure the table already exists
        {
            dropTable(xTables,m_sName);

            Reference<XDataDescriptorFactory> xFact(xTables,UNO_QUERY);
            OSL_ENSURE(xFact.is(),"OTableController::doSaveDoc: No XDataDescriptorFactory available!");
            xTable = xFact->createDataDescriptor();
            OSL_ENSURE(xTable.is(),"OTableController::doSaveDoc: Create query failed!");
            // to set the name is only allowed when the wuery is new
            xTable->setPropertyValue(PROPERTY_CATALOGNAME,makeAny(sCatalog));
            xTable->setPropertyValue(PROPERTY_SCHEMANAME,makeAny(sSchema));
            xTable->setPropertyValue(PROPERTY_NAME,makeAny(m_sName));

            // now append the columns
            Reference<XColumnsSupplier> xColSup(xTable,UNO_QUERY);
            appendColumns(xColSup,bNew);
            // now append the primary key
            Reference<XKeysSupplier> xKeySup(xTable,UNO_QUERY);
            appendPrimaryKey(xKeySup,bNew);
        }
        // now set the properties
        if(bNew)
        {
            Reference<XAppend> xAppend(xTables,UNO_QUERY);
            OSL_ENSURE(xAppend.is(),"OTableController::doSaveDoc: No XAppend Interface!");
            xAppend->appendByDescriptor(xTable);

            assignTable();
            if(!m_xTable.is()) // correct name and try again
            {
                // it can be that someone inserted new data for us
                m_sName = ::dbtools::composeTableName(getConnection()->getMetaData(),xTable,sal_False,::dbtools::eInDataManipulation);
                assignTable();
            }
            // now check if our datasource has set a tablefilter and if append the new table name to it
            ::dbaui::appendToFilter(getConnection(),m_sName,getORB(),getView()); // we are not interessted in the return value
        }
        else if(m_xTable.is())
        {
            bAlter = sal_True;
            alterColumns();
        }
        reSyncRows();
    }
    catch(const SQLContext& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch(const SQLWarning& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch(const SQLException& e)
    {
        aInfo = SQLExceptionInfo(e);
    }
    catch(const ElementExistException& )
    {
        String sText( ModuleRes(STR_OBJECT_ALREADY_EXISTS));
        sText.SearchAndReplaceAscii( "#" , m_sName);
        OSQLMessageBox aDlg(getView(), String(ModuleRes(STR_OBJECT_ALREADY_EXSISTS)), sText, WB_OK, OSQLMessageBox::Error);

        aDlg.Execute();
        bError = sal_True;
    }
    catch(Exception&)
    {
        bError = sal_True;
        OSL_ENSURE(sal_False, "OTableController::doSaveDoc: table could not be inserted (caught a generic exception)!");
    }

    showError(aInfo);
    if (aInfo.isValid() || bError)
    {
        if(!bAlter || bNew)
        {
            m_sName = ::rtl::OUString();
            stopTableListening();
            m_xTable = NULL;
        }
        //  reload(); // a error occured so we have to reload
    }
    return ! (aInfo.isValid() || bError);
}

// -----------------------------------------------------------------------------
void OTableController::doEditIndexes()
{
    // table needs to be saved before editing indexes
    if (m_bNew || isModified())
    {
        QueryBox aAsk(getView(), ModuleRes(QUERY_SAVE_TABLE_EDIT_INDEXES));
        if (RET_YES != aAsk.Execute())
            return;

        if (!doSaveDoc(sal_False))
            return;

        OSL_ENSURE(!m_bNew && !isModified(), "OTableController::doEditIndexes: what the hell did doSaveDoc do?");
    }

    Reference< XNameAccess > xIndexes;          // will be the keys of the table
    Sequence< ::rtl::OUString > aFieldNames;    // will be the column names of the table
    try
    {
        // get the keys
        Reference< XIndexesSupplier > xIndexesSupp(m_xTable, UNO_QUERY);
        if (xIndexesSupp.is())
        {
            xIndexes = xIndexesSupp->getIndexes();
            OSL_ENSURE(xIndexes.is(), "OTableController::doEditIndexes: no keys got from the indexes supplier!");
        }
        else
            OSL_ENSURE(sal_False, "OTableController::doEditIndexes: should never have reached this (no indexes supplier)!");

        // get the field names
        Reference< XColumnsSupplier > xColSupp(m_xTable, UNO_QUERY);
        OSL_ENSURE(xColSupp.is(), "OTableController::doEditIndexes: no columns supplier!");
        if (xColSupp.is())
        {
            Reference< XNameAccess > xCols = xColSupp->getColumns();
            OSL_ENSURE(xCols.is(), "OTableController::doEditIndexes: no columns!");
            if (xCols.is())
                aFieldNames = xCols->getElementNames();
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(sal_False, "OTableController::doEditIndexes: caught an exception while retrieving the indexes/columns!");
    }

    if (!xIndexes.is())
        return;

    DbaIndexDialog aDialog(getView(), aFieldNames, xIndexes, getConnection(),getORB(),isConnected() ? getConnection()->getMetaData().is() && getConnection()->getMetaData()->getMaxColumnsInIndex() : sal_Int32(0));
    if (RET_OK != aDialog.Execute())
        return;

}

// -----------------------------------------------------------------------------
void OTableController::impl_initialize( const Sequence< Any >& aArguments )
{
    try
    {
        OTableController_BASE::impl_initialize(aArguments);

        PropertyValue aValue;
        const Any* pBegin   = aArguments.getConstArray();
        const Any* pEnd     = pBegin + aArguments.getLength();

        for(;pBegin != pEnd;++pBegin)
        {
            if((*pBegin >>= aValue) && (0 == aValue.Name.compareToAscii(PROPERTY_ACTIVECONNECTION)))
            {
                Reference< XConnection > xConn;
                aValue.Value >>= xConn;
                OSL_ENSURE( xConn.is(), "OTableController::initialize: invalid connection given!!" );
                if ( xConn.is() )
                    initializeConnection( xConn );
            }
            else if (0 == aValue.Name.compareToAscii(PROPERTY_CURRENTTABLE))
            {
                aValue.Value >>= m_sName;
            }
        }
        // read autoincrement value set in the datasource
        ::dbaui::fillAutoIncrementValue(getDataSource(),m_bAllowAutoIncrementValue,m_sAutoIncrementValue);

        sal_Bool bFirstTry = sal_False;
        if (!isConnected())
        {   // whoever instantiated us did not give us a connection to share. Okay, create an own one
            reconnect(sal_False);
            bFirstTry = sal_True;
        }
        if (!isConnected()) // so what should otherwise
        {
            if(!bFirstTry)
            {
                String aMessage(ModuleRes(RID_STR_CONNECTION_LOST));
                ODataView* pWindow = getView();
                InfoBox(pWindow, aMessage).Execute();
            }
            throw Exception();
        }

        assignTable();
        if(!m_xFormatter.is())
        {
            Reference< XNumberFormatsSupplier> xSupplier = ::dbtools::getNumberFormats(getConnection());
            if(xSupplier.is())
            {
                m_xFormatter = Reference< ::com::sun::star::util::XNumberFormatter >(getORB()
                    ->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
                m_xFormatter->attachNumberFormatsSupplier(xSupplier);
            }
            OSL_ENSURE(m_xFormatter.is(),"No NumberFormatter!");
        }
    }
    catch(const SQLException&)
    {
        OSL_ENSURE(sal_False, "OTableController::initialize: caught an exception!");
    }

    try
    {
        ::dbaui::fillTypeInfo(getConnection(),m_sTypeNames,m_aTypeInfo,m_aTypeInfoIndex);               // fill the needed type information
    }
    catch(const SQLException&)
    {
        OSQLMessageBox aErr(getView(),ModuleRes(STR_STAT_WARNING),ModuleRes(STR_NO_TYPE_INFO_AVAILABLE));
        aErr.Execute();
        throw;
    }
    try
    {
        loadData();                 // fill the column information form the table
        getView()->initialize();    // show the windows and fill with our informations
        getUndoMgr()->Clear();      // clear all undo redo things
        setModified(sal_False);     // and we are not modified yet
    }
    catch(const SQLException&)
    {
        OSL_ENSURE(sal_False, "OTableController::initialize: caught an exception!");
    }
}
// -----------------------------------------------------------------------------
sal_Bool OTableController::Construct(Window* pParent)
{
    m_pView = new OTableDesignView(pParent,m_xMultiServiceFacatory,this);
    OTableController_BASE::Construct(pParent);
//  m_pView->Construct();
//  m_pView->Show();
    return sal_True;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OTableController::suspend(sal_Bool _bSuspend) throw( RuntimeException )
{
    if ( getBroadcastHelper().bInDispose || getBroadcastHelper().bDisposed )
        return sal_True;

    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    sal_Bool bCheck = sal_True;
    if ( isModified() )
    {
        ::std::vector<OTableRow*>::iterator aIter = ::std::find_if(m_vRowList.begin(),m_vRowList.end(),::std::mem_fun(&OTableRow::isValid));
        if ( aIter != m_vRowList.end() )
        {
            QueryBox aQry(getView(), ModuleRes(TABLE_DESIGN_SAVEMODIFIED));
            switch (aQry.Execute())
            {
                case RET_YES:
                    Execute(ID_BROWSER_SAVEDOC);
                    if ( isModified() )
                        bCheck = sal_False; // when we save the table this must be false else some press cancel
                    break;
                case RET_CANCEL:
                    bCheck = sal_False;
                default:
                    break;
            }
        }
        else if ( !m_bNew )
        {
            QueryBox aQry(getView(), ModuleRes(TABLE_DESIGN_ALL_ROWS_DELETED));
            switch (aQry.Execute())
            {
                case RET_YES:
                    {
                        try
                        {
                            Reference<XTablesSupplier> xTablesSup(getConnection(),UNO_QUERY);
                            Reference<XNameAccess> xTables = xTablesSup->getTables();
                            dropTable(xTables,m_sName);
                        }
                        catch(const Exception&)
                        {
                            OSL_ENSURE(sal_False, "OTableController::suspend: nothing is expected to happen here!");
                        }

                    }
                    break;
                case RET_CANCEL:
                    bCheck = sal_False;
                default:
                    break;
            }
        }
    }
/*
    if ( bCheck )
        OSingleDocumentController::suspend(_bSuspend);
*/
    return bCheck;
}
// -----------------------------------------------------------------------------
void OTableController::AddSupportedFeatures()
{
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Redo")]            = ID_BROWSER_REDO;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Save")]            = ID_BROWSER_SAVEDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Undo")]            = ID_BROWSER_UNDO;

    //  m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:BrowserMode")] = SID_BROWSER_MODE;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:HelpMenu")]        = SID_HELPMENU;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:NewDoc")]          = SID_NEWDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:SaveAs")]          = ID_BROWSER_SAVEASDOC;

    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Copy")]            = ID_BROWSER_COPY;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Cut")]             = ID_BROWSER_CUT;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Paste")]           = ID_BROWSER_PASTE;

    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DB/IndexDesign")]  = SID_INDEXDESIGN;

    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/EditDoc")] = ID_BROWSER_EDITDOC;
}
// -----------------------------------------------------------------------------
ToolBox* OTableController::CreateToolBox(Window* _pParent)
{
    return new ToolBox(_pParent, ModuleRes(RID_BRW_TABLEDESIGN_TOOLBOX));
}
// -----------------------------------------------------------------------------
SfxUndoManager* OTableController::getUndoMgr()
{
    return &m_aUndoManager;
}
// -----------------------------------------------------------------------------
void OTableController::setModified(sal_Bool _bModified)
{
    OSingleDocumentController::setModified(_bModified);
    InvalidateFeature(SID_INDEXDESIGN);
}
// -----------------------------------------------------------------------------
void SAL_CALL OTableController::disposing( const EventObject& _rSource ) throw(RuntimeException)
{
    if ( _rSource.Source == m_xTable )
    {   // some deleted our table so we have a new one
        stopTableListening();
        m_xTable    = NULL;
        m_bNew      = sal_True;
        setModified(sal_True);
    }
    else
        OTableController_BASE::disposing( _rSource );
}
// -----------------------------------------------------------------------------
void OTableController::Save(const Reference< XObjectOutputStream>& _rxOut)
{
    OStreamSection aSection(_rxOut.get());

}
// -----------------------------------------------------------------------------
void OTableController::Load(const Reference< XObjectInputStream>& _rxIn)
{
    OStreamSection aSection(_rxIn.get());
}

// -----------------------------------------------------------------------------
void OTableController::losingConnection( )
{
    // let the base class do it's reconnect
    OTableController_BASE::losingConnection( );

    // remove from the table
    Reference< XComponent >  xComponent(m_xTable, UNO_QUERY);
    if (xComponent.is())
    {
        Reference<XEventListener> xEvtL( static_cast< ::cppu::OWeakObject*>(this), UNO_QUERY);
        xComponent->removeEventListener(xEvtL);
    }
    stopTableListening();
    m_xTable    = NULL;
    assignTable();
    if(!m_xTable.is())
    {
        m_bNew      = sal_True;
        setModified(sal_True);
    }
    InvalidateAll();
}

// -----------------------------------------------------------------------------
void OTableController::reconnect(sal_Bool _bUI)
{
    OTableController_BASE::reconnect( _bUI );

    ToolBox* pToolBox = getView()->getToolBox();
    if(pToolBox)
    {
        if ( isConnected() )
        {
            pToolBox->RemoveItem(pToolBox->GetItemPos(ID_TABLE_DESIGN_NO_CONNECTION)-1);
            pToolBox->HideItem(ID_TABLE_DESIGN_NO_CONNECTION);
        }
        else if(!pToolBox->IsItemVisible(ID_TABLE_DESIGN_NO_CONNECTION))
        {

            pToolBox->InsertSeparator(pToolBox->GetItemPos(ID_TABLE_DESIGN_NO_CONNECTION));
            pToolBox->ShowItem(ID_TABLE_DESIGN_NO_CONNECTION);
        }
    }
}
// -----------------------------------------------------------------------------
TOTypeInfoSP OTableController::getTypeInfoByType(sal_Int32 _nDataType) const
{
    return queryTypeInfoByType(_nDataType,m_aTypeInfo);
}
// -----------------------------------------------------------------------------
void OTableController::appendColumns(Reference<XColumnsSupplier>& _rxColSup,sal_Bool _bNew,sal_Bool _bKeyColumns)
{
    try
    {
        // now append the columns
        OSL_ENSURE(_rxColSup.is(),"No columns supplier");
        if(!_rxColSup.is())
            return;
        Reference<XNameAccess> xColumns = _rxColSup->getColumns();
        OSL_ENSURE(xColumns.is(),"No columns");
        Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY);

        Reference<XAppend> xAppend(xColumns,UNO_QUERY);
        OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

        ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
        ::std::vector<OTableRow*>::iterator aEnd = m_vRowList.end();
        for(;aIter != aEnd;++aIter)
        {
            OSL_ENSURE(*aIter,"OTableRow is null!");
            OFieldDescription* pField = (*aIter)->GetActFieldDescr();
            if ( !pField || (!_bNew && (*aIter)->IsReadOnly() && !_bKeyColumns) )
                continue;

            Reference<XPropertySet> xColumn;
            if(pField->IsPrimaryKey() || !_bKeyColumns)
                xColumn = xColumnFactory->createDataDescriptor();
            if(xColumn.is())
            {
                if(!_bKeyColumns)
                    ::dbaui::setColumnProperties(xColumn,pField);
                else
                    xColumn->setPropertyValue(PROPERTY_NAME,makeAny(pField->GetName()));

                xAppend->appendByDescriptor(xColumn);
                xColumn = NULL;
                // now only the settings are missing
                if(xColumns->hasByName(pField->GetName()))
                {
                    xColumns->getByName(pField->GetName()) >>= xColumn;
                    if(xColumn.is())
                        dbaui::setColumnUiProperties(xColumn,pField);
                }
                else
                {
                    OSL_ENSURE(sal_False, "OTableController::appendColumns: invalid field name!");
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
        OSL_ENSURE(sal_False, "OTableController::appendColumns: caught an exception!");
    }
}
// -----------------------------------------------------------------------------
void OTableController::appendPrimaryKey(Reference<XKeysSupplier>& _rxSup,sal_Bool _bNew)
{
    if(!_rxSup.is())
        return; // the database doesn't support keys

    OSL_ENSURE(_rxSup.is(),"No XKeysSupplier!");
    Reference<XDataDescriptorFactory> xKeyFactory(_rxSup->getKeys(),UNO_QUERY);
    OSL_ENSURE(xKeyFactory.is(),"No XDataDescriptorFactory Interface!");
    if ( !xKeyFactory.is() )
        return;
    Reference<XAppend> xAppend(xKeyFactory,UNO_QUERY);
    OSL_ENSURE(xAppend.is(),"No XAppend Interface!");

    Reference<XPropertySet> xKey = xKeyFactory->createDataDescriptor();
    OSL_ENSURE(xKey.is(),"Key is null!");
    xKey->setPropertyValue(PROPERTY_TYPE,makeAny(KeyType::PRIMARY));

    Reference<XColumnsSupplier> xColSup(xKey,UNO_QUERY);
    if(xColSup.is())
    {
        appendColumns(xColSup,_bNew,sal_True);
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        if(xColumns->hasElements())
            xAppend->appendByDescriptor(xKey);
    }
}
// -----------------------------------------------------------------------------
void OTableController::loadData()
{
    //////////////////////////////////////////////////////////////////////
    // Wenn Datenstruktur bereits vorhanden, Struktur leeren
    ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
    for(;aIter != m_vRowList.end();++aIter)
        delete *aIter;
    m_vRowList.clear();

    OTableRow* pTabEdRow = NULL;
    Reference< XDatabaseMetaData> xMetaData = getMetaData( );
    //////////////////////////////////////////////////////////////////////
    // Datenstruktur mit Daten aus DatenDefinitionsObjekt fuellen
    if(m_xTable.is() && xMetaData.is())
    {
        Reference<XColumnsSupplier> xColSup(m_xTable,UNO_QUERY);
        OSL_ENSURE(xColSup.is(),"No XColumnsSupplier!");
        Reference<XNameAccess> xColumns = xColSup->getColumns();
        OFieldDescription* pActFieldDescr = NULL;
        String aType;
        //////////////////////////////////////////////////////////////////////
        // ReadOnly-Flag
        // Bei Drop darf keine Zeile editierbar sein.
        // Bei Add duerfen nur die leeren Zeilen editierbar sein.
        // Bei Add und Drop koennen alle Zeilen editiert werden.
        //  sal_Bool bReadOldRow = xMetaData->supportsAlterTableWithAddColumn() && xMetaData->supportsAlterTableWithDropColumn();
        sal_Bool bIsAlterAllowed = isAlterAllowed();
        Sequence< ::rtl::OUString> aColumns = xColumns->getElementNames();
        const ::rtl::OUString* pBegin   = aColumns.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aColumns.getLength();

        for(;pBegin != pEnd;++pBegin)
        {
            Reference<XPropertySet> xColumn;
            xColumns->getByName(*pBegin) >>= xColumn;
            sal_Int32 nType         = 0;
            sal_Int32 nScale        = 0;
            sal_Int32 nPrecision    = 0;
            sal_Int32 nNullable     = 0;
            sal_Int32 nFormatKey    = 0;
            sal_Int32 nAlign        = 0;

            sal_Bool bIsAutoIncrement,bIsCurrency;
            ::rtl::OUString sName,sDescription,sTypeName;
            Any aControlDefault;

            // get the properties from the column
            xColumn->getPropertyValue(PROPERTY_NAME)            >>= sName;
            xColumn->getPropertyValue(PROPERTY_TYPENAME)        >>= sTypeName;
            xColumn->getPropertyValue(PROPERTY_ISNULLABLE)      >>= nNullable;
            xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT) >>= bIsAutoIncrement;
            xColumn->getPropertyValue(PROPERTY_ISCURRENCY)      >>= bIsCurrency;
            xColumn->getPropertyValue(PROPERTY_TYPE)            >>= nType;
            xColumn->getPropertyValue(PROPERTY_SCALE)           >>= nScale;
            xColumn->getPropertyValue(PROPERTY_PRECISION)       >>= nPrecision;


            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_HELPTEXT))
                xColumn->getPropertyValue(PROPERTY_HELPTEXT)    >>= sDescription;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_CONTROLDEFAULT))
                aControlDefault = xColumn->getPropertyValue(PROPERTY_CONTROLDEFAULT);
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
                xColumn->getPropertyValue(PROPERTY_FORMATKEY)   >>= nFormatKey;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
                xColumn->getPropertyValue(PROPERTY_ALIGN)       >>= nAlign;

            pTabEdRow = new OTableRow();
            pTabEdRow->SetReadOnly(!bIsAlterAllowed);
            // search for type
            sal_Bool bForce;
            ::rtl::OUString sCreate(RTL_CONSTASCII_USTRINGPARAM("x"));
            TOTypeInfoSP pTypeInfo = ::dbaui::getTypeInfoFromType(m_aTypeInfo,nType,sTypeName,sCreate,nPrecision,nScale,bIsAutoIncrement,bForce);
            if ( !pTypeInfo.get() )
                pTypeInfo = m_pTypeInfo;
            pTabEdRow->SetFieldType( pTypeInfo, bForce );

            pActFieldDescr = pTabEdRow->GetActFieldDescr();
            OSL_ENSURE(pActFieldDescr, "OTableController::loadData: invalid field description generated by the table row!");
            if ( pActFieldDescr )
            {
                pActFieldDescr->SetName(sName);
                pActFieldDescr->SetFormatKey(nFormatKey);
                //  pActFieldDescr->SetPrimaryKey(pPrimary->GetValue());
                pActFieldDescr->SetDescription(sDescription);
                pActFieldDescr->SetAutoIncrement(bIsAutoIncrement);
                pActFieldDescr->SetHorJustify(dbaui::mapTextJustify(nAlign));
                pActFieldDescr->SetCurrency(bIsCurrency);

                //////////////////////////////////////////////////////////////////////
                // Spezielle Daten
                pActFieldDescr->SetIsNullable(nNullable);
                pActFieldDescr->SetControlDefault(aControlDefault);
                pActFieldDescr->SetPrecision(nPrecision);
                pActFieldDescr->SetScale(nScale);
            }
            m_vRowList.push_back( pTabEdRow);
        }
        // fill the primary  key information
        Reference<XNameAccess> xKeyColumns  = getKeyColumns();
        if(xKeyColumns.is())
        {
            Sequence< ::rtl::OUString> aKeyColumns = xKeyColumns->getElementNames();
            const ::rtl::OUString* pKeyBegin    = aKeyColumns.getConstArray();
            const ::rtl::OUString* pKeyEnd      = pKeyBegin + aKeyColumns.getLength();

            for(;pKeyBegin != pKeyEnd;++pKeyBegin)
            {
                ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
                for(;aIter != m_vRowList.end();++aIter)
                {
                    if((*aIter)->GetActFieldDescr()->GetName() == *pKeyBegin)
                    {
                        (*aIter)->SetPrimaryKey(sal_True);
                        break;
                    }
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////
    // Leere Zeilen fuellen

    OTypeInfoMap::iterator aTypeIter = m_aTypeInfo.find(DataType::VARCHAR);
    if(aTypeIter == m_aTypeInfo.end())
        aTypeIter = m_aTypeInfo.begin();

    OSL_ENSURE(aTypeIter != m_aTypeInfo.end(),"We have no type infomation!");

    bool bReadRow = !isAddAllowed();
    for(sal_Int32 i=m_vRowList.size(); i<128; i++ )
    {
        pTabEdRow = new OTableRow();
        pTabEdRow->SetReadOnly(bReadRow);
        m_vRowList.push_back( pTabEdRow);
    }
}
// -----------------------------------------------------------------------------
Reference<XNameAccess> OTableController::getKeyColumns() const
{
    // use keys and indexes for excat postioning
    // first the keys
    Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
    Reference<XIndexAccess> xKeys;
    if(xKeySup.is())
        xKeys = xKeySup->getKeys();

    Reference<XColumnsSupplier> xKeyColsSup;
    Reference<XNameAccess> xKeyColumns;
    if(xKeys.is())
    {
        Reference<XPropertySet> xProp;
        sal_Int32 nCount = xKeys->getCount();
        for(sal_Int32 i=0;i< nCount;++i)
        {
            xKeys->getByIndex(i) >>= xProp;
            OSL_ENSURE(xProp.is(),"Key is invalid: NULL!");
            if ( xProp.is() )
            {
                sal_Int32 nKeyType = 0;
                xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
                if(KeyType::PRIMARY == nKeyType)
                {
                    xKeyColsSup.set(xProp,UNO_QUERY);
                    OSL_ENSURE(xKeyColsSup.is(),"Columnsupplier is null!");
                    xKeyColumns = xKeyColsSup->getColumns();
                    break;
                }
            }
        }
    }

    return xKeyColumns;
}
// -----------------------------------------------------------------------------
sal_Bool OTableController::checkColumns(sal_Bool _bNew) throw(::com::sun::star::sdbc::SQLException)
{
    sal_Bool bOk = sal_True;
    sal_Bool bFoundPKey = sal_False;
    Reference< XDatabaseMetaData> xMetaData = getMetaData( );

    ::comphelper::UStringMixEqual bCase(xMetaData.is() ? xMetaData->storesMixedCaseQuotedIdentifiers() : sal_True);
    ::std::vector<OTableRow*>::const_iterator aIter = m_vRowList.begin();
    for(;aIter != m_vRowList.end();++aIter)
    {
        OFieldDescription* pFieldDesc = (*aIter)->GetActFieldDescr();
        if (pFieldDesc && pFieldDesc->GetName().getLength())
        {
            bFoundPKey |=  (*aIter)->IsPrimaryKey();
            sal_uInt16 nErrorRes = sal_uInt16(-1);
            sal_uInt16 nFieldPos = sal_uInt16(-1);
            // first check for duplicate names
            ::std::vector<OTableRow*>::const_iterator aIter2 = aIter+1;
            for(;aIter2 != m_vRowList.end();++aIter2)
            {
                OFieldDescription* pCompareDesc = (*aIter2)->GetActFieldDescr();
                if (pCompareDesc && bCase(pCompareDesc->GetName(),pFieldDesc->GetName()))
                {
                    String strMessage = String(ModuleRes(STR_TABLEDESIGN_DUPLICATE_NAME));
                    strMessage.SearchAndReplaceAscii("$column$", pFieldDesc->GetName());
                    String sTitle(ModuleRes(STR_STAT_WARNING));
                    OSQLMessageBox aMsg(getView(),sTitle,strMessage,WB_OK | WB_DEF_OK,OSQLMessageBox::Error);
                    aMsg.Execute();
                    return sal_False;
                }
            }
        }
    }
    if(!bFoundPKey)
    {
        if(_bNew && xMetaData.is() && xMetaData->supportsCoreSQLGrammar())
        {
            String sTitle(ModuleRes(STR_TABLEDESIGN_NO_PRIM_KEY_HEAD));
            String sMsg(ModuleRes(STR_TABLEDESIGN_NO_PRIM_KEY));
            OSQLMessageBox aBox(getView(), sTitle,sMsg, WB_YES_NO_CANCEL | WB_DEF_YES);

            INT16 nReturn = aBox.Execute();

            if (nReturn == RET_YES)
            {
                OTableRow* pNewRow = new OTableRow();
                TOTypeInfoSP pTypeInfo = ::dbaui::queryPrimaryKeyType(m_aTypeInfo);

                if ( pTypeInfo.get() )
                {
                    pNewRow->SetFieldType( pTypeInfo );
                    OFieldDescription* pActFieldDescr = pNewRow->GetActFieldDescr();

                    pActFieldDescr->SetAutoIncrement(sal_False); // #95927# pTypeInfo->bAutoIncrement
                    pActFieldDescr->SetIsNullable(ColumnValue::NO_NULLS);


                    pActFieldDescr->SetName( createUniqueName(::rtl::OUString::createFromAscii("ID") ));
                    pActFieldDescr->SetPrimaryKey( sal_True );
                    m_vRowList.insert(m_vRowList.begin(),pNewRow);

                    static_cast<OTableDesignView*>(getView())->GetEditorCtrl()->Invalidate();
                    //  static_cast<OTableDesignView*>(getView())->GetEditorCtrl()->DisplayData(0);
                    static_cast<OTableDesignView*>(getView())->GetEditorCtrl()->RowInserted(0);
                }
            }
            else if (nReturn == RET_CANCEL)
                bOk = sal_False;
        }
    }
    return bOk;
}
// -----------------------------------------------------------------------------
void OTableController::alterColumns()
{
    Reference<XColumnsSupplier> xColSup(m_xTable,UNO_QUERY);
    OSL_ENSURE(xColSup.is(),"What happen here?!");

    Reference<XNameAccess> xColumns = xColSup->getColumns();
    OSL_ENSURE(xColumns.is(),"No columns");
    Reference<XAlterTable> xAlter(m_xTable,UNO_QUERY);  // can be null

    sal_Int32 nColumnCount = xColumns->getElementNames().getLength();
    Reference<XDrop> xDrop(xColumns,UNO_QUERY);         // can be null
    Reference<XAppend> xAppend(xColumns,UNO_QUERY);     // can be null
    Reference<XDataDescriptorFactory> xColumnFactory(xColumns,UNO_QUERY); // can be null

    sal_Bool bReload = sal_False; // refresh the data

    // contains all columns names which are already handled those which are not in the list will be deleted
    Reference< XDatabaseMetaData> xMetaData = getMetaData( );


    ::std::map< ::rtl::OUString,sal_Bool,::comphelper::UStringMixLess> aColumns(xMetaData.is() ? (xMetaData->storesMixedCaseQuotedIdentifiers() ? true : false): sal_True);
    ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
    ::std::vector<OTableRow*>::iterator aEnd = m_vRowList.end();
    // first look for columns where something other than the name changed
    for(sal_Int32 nPos = 0;aIter != aEnd;++aIter,++nPos)
    {
        OSL_ENSURE(*aIter,"OTableRow is null!");
        OFieldDescription* pField = (*aIter)->GetActFieldDescr();
        if ( !pField )
            continue;
        if ( (*aIter)->IsReadOnly() )
        {
            aColumns[pField->GetName()] = sal_True;
            continue;
        }

        Reference<XPropertySet> xColumn;
        if ( xColumns->hasByName(pField->GetName()) )
        {
            aColumns[pField->GetName()] = sal_True;
            xColumns->getByName(pField->GetName()) >>= xColumn;
            OSL_ENSURE(xColumn.is(),"Column is null!");

            sal_Int32 nType,nPrecision,nScale,nNullable,nFormatKey=0,nAlignment=0;
            sal_Bool bAutoIncrement;
            ::rtl::OUString sDescription;
            Any aControlDefault;

            xColumn->getPropertyValue(PROPERTY_TYPE)            >>= nType;
            xColumn->getPropertyValue(PROPERTY_PRECISION)       >>= nPrecision;
            xColumn->getPropertyValue(PROPERTY_SCALE)           >>= nScale;
            xColumn->getPropertyValue(PROPERTY_ISNULLABLE)      >>= nNullable;
            xColumn->getPropertyValue(PROPERTY_ISAUTOINCREMENT) >>= bAutoIncrement;
            //  xColumn->getPropertyValue(PROPERTY_ISCURRENCY,::cppu::bool2any(pField->IsCurrency()));
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_HELPTEXT))
                xColumn->getPropertyValue(PROPERTY_HELPTEXT) >>= sDescription;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_CONTROLDEFAULT))
                aControlDefault = xColumn->getPropertyValue(PROPERTY_CONTROLDEFAULT);
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
                xColumn->getPropertyValue(PROPERTY_FORMATKEY)   >>= nFormatKey;
            if(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
                xColumn->getPropertyValue(PROPERTY_ALIGN)       >>= nAlignment;

            // check if something changed
            if((nType != pField->GetType()                  ||
                nPrecision != pField->GetPrecision()        ||
                nScale != pField->GetScale()                ||
                nNullable != pField->GetIsNullable()        ||
                bAutoIncrement != pField->IsAutoIncrement())&&
                xColumnFactory.is())
            {
                Reference<XPropertySet> xNewColumn;
                xNewColumn = xColumnFactory->createDataDescriptor();
                ::dbaui::setColumnProperties(xNewColumn,pField);
                // first try to alter the column
                sal_Bool bNotOk = sal_False;
                try
                {
                    // first try if we can alter the column
                    if(xAlter.is())
                        xAlter->alterColumnByName(pField->GetName(),xNewColumn);
                }
                catch(const SQLException&)
                {
                    if(xDrop.is() && xAppend.is())
                    {
                        String aMessage(ModuleRes(STR_TABLEDESIGN_ALTER_ERROR));
                        aMessage.SearchAndReplaceAscii("$column$",pField->GetName());
                        String sTitle(ModuleRes(STR_STAT_WARNING));
                        OSQLMessageBox aMsg(getView(),sTitle,aMessage,WB_YES_NO|WB_DEF_YES,OSQLMessageBox::Warning);
                        bNotOk = aMsg.Execute() == RET_YES;
                    }
                    else
                        throw;
                }
                // if something went wrong or we can't alter columns
                // drop and append a new one
                if((!xAlter.is() || bNotOk) && xDrop.is() && xAppend.is())
                {
                    xDrop->dropByName(pField->GetName());
                    try
                    {
                        xAppend->appendByDescriptor(xNewColumn);
                    }
                    catch(const SQLException&)
                    { // an error occured so we try to reactivate the old one
                        xAppend->appendByDescriptor(xColumn);
                        throw;
                    }
                }
                // exceptions are catched outside
                xNewColumn = NULL;
                if(xColumns->hasByName(pField->GetName()))
                    xColumns->getByName(pField->GetName()) >>= xColumn;
                bReload = sal_True;
            }

            if(nFormatKey != pField->GetFormatKey())
            {
                if(xColumn.is() && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_FORMATKEY))
                    xColumn->setPropertyValue(PROPERTY_FORMATKEY,makeAny(pField->GetFormatKey()));
            }
            if(nAlignment != dbaui::mapTextAllign(pField->GetHorJustify()))
            {
                if(xColumn.is() && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_ALIGN))
                    xColumn->setPropertyValue(PROPERTY_ALIGN,makeAny(dbaui::mapTextAllign(pField->GetHorJustify())));
            }
            if(sDescription != pField->GetDescription())
            {
                if(xColumn.is() && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_HELPTEXT))
                    xColumn->setPropertyValue(PROPERTY_HELPTEXT,makeAny(pField->GetDescription()));
            }
            if ( aControlDefault != pField->GetControlDefault())
            {
                if(xColumn.is() && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_CONTROLDEFAULT))
                    xColumn->setPropertyValue(PROPERTY_CONTROLDEFAULT,pField->GetControlDefault());
            }
        }
        else if(xColumnFactory.is() && xAlter.is() && nPos < nColumnCount)
        { // we can't find the column so we could try it with the index before we drop and append a new column
            try
            {
                Reference<XPropertySet> xNewColumn;
                xNewColumn = xColumnFactory->createDataDescriptor();
                ::dbaui::setColumnProperties(xNewColumn,pField);
                xAlter->alterColumnByIndex(nPos,xNewColumn);
                if(xColumns->hasByName(pField->GetName()))
                {   // ask for the append by name
                    aColumns[pField->GetName()] = sal_True;
                    xColumns->getByName(pField->GetName()) >>= xColumn;
                    if(xColumn.is())
                        dbaui::setColumnUiProperties(xColumn,pField);
                }
                else
                {
                    OSL_ENSURE(sal_False, "OTableController::alterColumns: invalid column (2)!");
                }
            }
            catch(const SQLException&)
            { // we couldn't alter the column so we have to add new columns
                bReload = sal_True;
            }
        }
        else
            bReload = sal_True;
    }
    // second drop all columns which could be found by name
    Reference<XNameAccess> xKeyColumns  = getKeyColumns();
    // now we have to look for the columns who could be deleted
    if(xDrop.is())
    {
        Sequence< ::rtl::OUString> aColumnNames = xColumns->getElementNames();
        const ::rtl::OUString* pBegin = aColumnNames.getConstArray();
        const ::rtl::OUString* pEnd = pBegin + aColumnNames.getLength();
        for(;pBegin != pEnd;++pBegin)
        {
            if(aColumns.find(*pBegin) == aColumns.end()) // found a column to delete
            {
                if(xKeyColumns.is() && xKeyColumns->hasByName(*pBegin)) // check if this column is a member of the primary key
                {
                    String aMsgT(ModuleRes(STR_TBL_COLUMN_IS_KEYCOLUMN));
                    aMsgT.SearchAndReplaceAscii("$column$",*pBegin);
                    String aTitle(ModuleRes(STR_TBL_COLUMN_IS_KEYCOLUMN_TITLE));
                    OSQLMessageBox aMsg(getView(),aTitle,aMsgT,WB_YES_NO| WB_DEF_YES);
                    if(aMsg.Execute() == RET_YES)
                    {
                        xKeyColumns = NULL;
                        dropPrimaryKey();
                    }
                    else
                    {
                        bReload = sal_True;
                        continue;
                    }
                }
                Reference<XDrop> xDrop(xColumns,UNO_QUERY);
                xDrop->dropByName(*pBegin);
            }
        }
    }

    // third append the new columns
    aIter = m_vRowList.begin();
    for(;aIter != m_vRowList.end();++aIter)
    {
        OSL_ENSURE(*aIter,"OTableRow is null!");
        OFieldDescription* pField = (*aIter)->GetActFieldDescr();
        if ( !pField || (*aIter)->IsReadOnly() )
            continue;

        Reference<XPropertySet> xColumn;
        if(!xColumns->hasByName(pField->GetName()))
        {
            if(xColumnFactory.is() && xAppend.is())
            {// column not found by its name so we assume it is new
                // Column is new
                xColumn = xColumnFactory->createDataDescriptor();
                ::dbaui::setColumnProperties(xColumn,pField);
                xAppend->appendByDescriptor(xColumn);
                if(xColumns->hasByName(pField->GetName()))
                {   // ask for the append by name
                    aColumns[pField->GetName()] = sal_True;
                    xColumns->getByName(pField->GetName()) >>= xColumn;
                    if(xColumn.is())
                        dbaui::setColumnUiProperties(xColumn,pField);
                }
                else
                {
                    OSL_ENSURE(sal_False, "OTableController::alterColumns: invalid column!");
                }
            }
        }
    }


    // check if we have to do something with the primary key
    sal_Bool bNeedDropKey = sal_False;
    sal_Bool bNeedAppendKey = sal_False;
    if ( xKeyColumns.is() )
    {
        aIter = m_vRowList.begin();
        for(;aIter != m_vRowList.end();++aIter)
        {
            OSL_ENSURE(*aIter,"OTableRow is null!");
            OFieldDescription* pField = (*aIter)->GetActFieldDescr();
            if ( !pField )
                continue;

            if  (   pField->IsPrimaryKey()
                &&  !xKeyColumns->hasByName( pField->GetName() )
                )
            {   // new primary key column inserted which isn't already in the columns selection
                bNeedDropKey = bNeedAppendKey = sal_True;
                break;
            }
            else if (   !pField->IsPrimaryKey()
                    &&  xKeyColumns->hasByName( pField->GetName() )
                    )
            {   // found a column which currently is in the primary key, but is marked not to be anymore
                bNeedDropKey = bNeedAppendKey = sal_True;
                break;
            }
        }
    }
    else
    {   // no primary key available so we check if we should create one
        bNeedAppendKey = sal_True;
    }

    if ( bNeedDropKey )
        dropPrimaryKey();

    if ( bNeedAppendKey )
    {
        Reference< XKeysSupplier > xKeySup( m_xTable, UNO_QUERY );
        appendPrimaryKey( xKeySup ,sal_False);
    }

    reSyncRows();

    if ( bReload )
        reload();
}
// -----------------------------------------------------------------------------
void OTableController::dropPrimaryKey()
{
    Reference<XKeysSupplier> xKeySup(m_xTable,UNO_QUERY);
    Reference<XIndexAccess> xKeys;
    if(xKeySup.is())
        xKeys = xKeySup->getKeys();

    if(xKeys.is())
    {
        Reference<XPropertySet> xProp;
        for(sal_Int32 i=0;i< xKeys->getCount();++i)
        {
            xKeys->getByIndex(i) >>= xProp;
            sal_Int32 nKeyType = 0;
            xProp->getPropertyValue(PROPERTY_TYPE) >>= nKeyType;
            if(KeyType::PRIMARY == nKeyType)
            {
                Reference<XDrop> xDrop(xKeys,UNO_QUERY);
                xDrop->dropByIndex(i); // delete the key
                break;
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OTableController::assignTable()
{
    ::rtl::OUString sComposedName;
    // get the table
    if(m_sName.getLength())
    {
        Reference<XNameAccess> xNameAccess;
        Reference<XTablesSupplier> xSup(getConnection(),UNO_QUERY);
        if(xSup.is())
        {
            xNameAccess = xSup->getTables();
            OSL_ENSURE(xNameAccess.is(),"no nameaccess for the queries!");

            Reference<XPropertySet> xProp;
            if(xNameAccess->hasByName(m_sName) && ::cppu::extractInterface(xProp,xNameAccess->getByName(m_sName)) && xProp.is())
            {
                m_xTable = xProp;
                startTableListening();

                // check if we set the table editable
                Reference<XDatabaseMetaData> xMeta = getConnection()->getMetaData();
                setEditable( xMeta.is() && !xMeta->isReadOnly() && (isAlterAllowed() || isDropAllowed() || isAddAllowed()) );
                if(!isEditable())
                {
                    ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
                    for(; aIter != m_vRowList.end(); ++aIter)
                        (*aIter)->SetReadOnly(sal_True);
                }
                m_bNew = sal_False;
                // be notified when the table is in disposing
                InvalidateAll();
            }
        }
    }
    updateTitle();
}
// -----------------------------------------------------------------------------
sal_Bool OTableController::isAddAllowed() const
{
    Reference<XColumnsSupplier> xColsSup(m_xTable,UNO_QUERY);
    sal_Bool bAddAllowed = !m_xTable.is();
    if(xColsSup.is())
        bAddAllowed = Reference<XAppend>(xColsSup->getColumns(),UNO_QUERY).is();

    Reference< XDatabaseMetaData > xMetaData = getMetaData( );
    bAddAllowed = bAddAllowed || ( xMetaData.is() && xMetaData->supportsAlterTableWithAddColumn());

    return bAddAllowed;
}
// -----------------------------------------------------------------------------
sal_Bool OTableController::isDropAllowed() const
{
    Reference<XColumnsSupplier> xColsSup(m_xTable,UNO_QUERY);
    sal_Bool bDropAllowed = !m_xTable.is();
    if(xColsSup.is())
    {
        Reference<XNameAccess> xNameAccess = xColsSup->getColumns();
        bDropAllowed = Reference<XDrop>(xNameAccess,UNO_QUERY).is() && xNameAccess->hasElements();
    }

    Reference< XDatabaseMetaData> xMetaData = getMetaData( );
    bDropAllowed = bDropAllowed || ( xMetaData.is() && xMetaData->supportsAlterTableWithDropColumn());

    return bDropAllowed;
}
// -----------------------------------------------------------------------------
sal_Bool OTableController::isAlterAllowed() const
{
    sal_Bool bAllowed(!m_xTable.is() || Reference<XAlterTable>(m_xTable,UNO_QUERY).is());
    return bAllowed;
}
// -----------------------------------------------------------------------------
void OTableController::reSyncRows()
{
    sal_Bool bAlterAllowed  = isAlterAllowed();
    sal_Bool bAddAllowed    = isAddAllowed();
    ::std::vector<OTableRow*>::iterator aIter = m_vRowList.begin();
    for(;aIter != m_vRowList.end();++aIter)
    {
        OSL_ENSURE(*aIter,"OTableRow is null!");
        OFieldDescription* pField = (*aIter)->GetActFieldDescr();
        if ( pField )
            (*aIter)->SetReadOnly(!bAlterAllowed);
        else
            (*aIter)->SetReadOnly(!bAddAllowed);

    }
    static_cast<OTableDesignView*>(getView())->reSync();    // show the windows and fill with our informations

    getUndoMgr()->Clear();      // clear all undo redo things
    setModified(sal_False);     // and we are not modified yet
}
// -----------------------------------------------------------------------------
::rtl::OUString OTableController::createUniqueName(const ::rtl::OUString& _rName)
{
    ::rtl::OUString sName = _rName;
    Reference< XDatabaseMetaData> xMetaData = getMetaData( );

    ::comphelper::UStringMixEqual bCase(xMetaData.is() ? xMetaData->storesMixedCaseQuotedIdentifiers() : sal_True);

    ::std::vector<OTableRow*>::const_iterator aIter = m_vRowList.begin();
    for(sal_Int32 i=0;aIter != m_vRowList.end();++aIter)
    {
        OFieldDescription* pFieldDesc = (*aIter)->GetActFieldDescr();
        if (pFieldDesc && pFieldDesc->GetName().getLength() && bCase(sName,pFieldDesc->GetName()))
        { // found a second name of _rName so we need another
            sName = _rName + ::rtl::OUString::valueOf(++i);
            aIter = m_vRowList.begin(); // and retry
        }
    }
    return sName;
}
// -----------------------------------------------------------------------------
void OTableController::updateTitle()
{
    try
    {
        ::rtl::OUString sTitle;
        // get the table
        if ( m_sName.getLength() && getConnection().is() )
        {
            if ( m_xTable.is() )
                sTitle = ::dbtools::composeTableName(getConnection()->getMetaData(),m_xTable,sal_False,::dbtools::eInDataManipulation);
            else
                sTitle = m_sName;
        }
        ::rtl::OUString sName = String(ModuleRes(STR_TABLEDESIGN_TITLE));
        if(sTitle.getLength())
            sName = sTitle + sName;
        else
        {
            ::rtl::OUString sTemp(getDataSourceName());
            sName = ::dbaui::getStrippedDatabaseName(getDataSource(),sTemp) + sName;
        }

        OGenericUnoController::setTitle(sName);
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception catched while setting the title!");
    }
}
// -----------------------------------------------------------------------------
void OTableController::reload()
{
    loadData();                 // fill the column information form the table
    static_cast<OTableDesignView*>(getView())->reSync();    // show the windows and fill with our informations
    getUndoMgr()->Clear();      // clear all undo redo things
    setModified(sal_False);     // and we are not modified yet
    static_cast<OTableDesignView*>(getView())->Invalidate();
}
// -----------------------------------------------------------------------------



