/*************************************************************************
 *
 *  $RCSfile: querycontroller.cxx,v $
 *
 *  $Revision: 1.28 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-06 13:46:40 $
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
#ifndef DBAUI_QUERYCONTROLLER_HXX
#include "querycontroller.hxx"
#endif
#ifndef DBAUI_QUERYVIEW_HXX
#include "queryview.hxx"
#endif
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
#ifndef DBAUI_QUERYVIEW_TEXT_HXX
#include "QueryTextView.hxx"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
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
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef DBAUI_DLGSAVE_HXX
#include "dlgsave.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
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
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/XExecutableDialog.hpp>
#endif
#ifndef DBAUI_TABLECONNECTIONDATA_HXX
#include "TableConnectionData.hxx"
#endif
#ifndef DBAUI_QYDLGTAB_HXX
#include "adtabdlg.hxx"
#endif
#ifndef DBAUI_QUERYVIEWSWITCH_HXX
#include "QueryViewSwitch.hxx"
#endif
#ifndef _COMPHELPER_STREAMSECTION_HXX_
#include <comphelper/streamsection.hxx>
#endif
#ifndef _COMPHELPER_BASIC_IO_HXX_
#include <comphelper/basicio.hxx>
#endif
#ifndef DBAUI_TABLEFIELDDESC_HXX
#include "TableFieldDescription.hxx"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
#include "QTableWindowData.hxx"
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
#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#include "QTableWindow.hxx"
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif


extern "C" void SAL_CALL createRegistryInfo_OQueryControl()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OQueryController > aAutoRegistration;
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
//  using namespace ::com::sun::star::sdbcx;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::dbaui;
using namespace ::comphelper;

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString OQueryController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.OQueryDesign");
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> OQueryController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.QueryDesign");
    return aSupported;
}
//-------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL OQueryController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OQueryController::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new OQueryController(_rxFactory));
}
// -----------------------------------------------------------------------------
OQueryController::OQueryController(const Reference< XMultiServiceFactory >& _rM)
        : OJoinController(_rM)
    ,m_bDesign(sal_False)
    ,m_bDistinct(sal_False)
    ,m_bViewAlias(sal_False)
    ,m_bViewTable(sal_False)
    ,m_bViewFunction(sal_False)
    ,m_bEsacpeProcessing(sal_True)
    ,m_pSqlIterator(NULL)
    ,m_nSplitPos(-1)
    ,m_nVisibleRows(0x400)
{
    m_pParseContext = new OQueryParseContext();
    m_pSqlParser    = new OSQLParser(_rM,m_pParseContext);
    InvalidateAll();
}
// -----------------------------------------------------------------------------
OQueryController::~OQueryController()
{
}
// -----------------------------------------------------------------------------
void OQueryController::deleteIterator()
{
    if(m_pSqlIterator)
    {
        delete m_pSqlIterator->getParseTree();
        m_pSqlIterator->dispose();
        delete m_pSqlIterator;
        m_pSqlIterator = NULL;
    }
}
// -----------------------------------------------------------------------------
void OQueryController::disposing()
{
    deleteIterator();

    delete m_pSqlParser;
    delete m_pParseContext;

    {
        ::std::vector< OTableFieldDesc*>::iterator aIter = m_vTableFieldDesc.begin();
        for(;aIter != m_vTableFieldDesc.end();++aIter)
            delete *aIter;
        m_vTableFieldDesc.clear();
    }

    m_pView     = NULL;
    m_pWindow   = NULL; // don't delete this window it will be deleted by the frame

    ::comphelper::disposeComponent(m_xComposer);
    OJoinController::disposing();
}
// -----------------------------------------------------------------------------
FeatureState OQueryController::GetState(sal_uInt16 _nId)
{
    FeatureState aReturn;
    aReturn.bEnabled = sal_True;
        // (disabled automatically)
//  aReturn.bEnabled = m_xConnection.is();
//  if(!m_xConnection.is()) // so what should otherwise happen
//  {
//      aReturn.aState = ::cppu::bool2any(sal_False);
//      return aReturn;
//  }

    switch (_nId)
    {
        case ID_BROWSER_ESACPEPROCESSING:
            aReturn.aState = ::cppu::bool2any(!m_bEsacpeProcessing);
            aReturn.bEnabled = m_pSqlIterator != NULL;
            break;
        case ID_BROWSER_EDITDOC:
            aReturn.aState = ::cppu::bool2any(m_bEditable);
            break;
        case ID_BROWSER_SAVEASDOC:
            aReturn.bEnabled = (!m_bDesign || (m_vTableFieldDesc.size() && m_vTableData.size()));
            break;
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = m_bModified && (!m_bDesign || (m_vTableFieldDesc.size() && m_vTableData.size()));
            break;
        case SID_PRINTDOCDIRECT:
            break;
        case ID_BROWSER_CUT:
            aReturn.bEnabled = m_bEditable && m_pWindow->getView()->isCutAllowed();
            break;
        case ID_BROWSER_COPY:
            break;
        case ID_BROWSER_PASTE:
            aReturn.bEnabled = m_bEditable;
            break;
        case ID_BROWSER_SQL:
            aReturn.bEnabled = m_bEsacpeProcessing && m_pSqlIterator;
            aReturn.aState = ::cppu::bool2any(m_bDesign);
            break;
        case ID_BROWSER_CLEAR_QUERY:
            aReturn.bEnabled = m_bEditable && (m_sStatement.getLength() || m_vTableData.size());
            break;
        case ID_BROWSER_QUERY_VIEW_FUNCTIONS:
        case ID_BROWSER_QUERY_VIEW_TABLES:
        case ID_BROWSER_QUERY_VIEW_ALIASES:
            aReturn.aState = ::cppu::bool2any(m_pWindow->getView()->isSlotEnabled(_nId));
            break;
        case ID_BROWSER_QUERY_DISTINCT_VALUES:
            aReturn.aState = ::cppu::bool2any(m_bDistinct);
            break;
        case ID_BROWSER_QUERY_EXECUTE:
            aReturn.bEnabled = sal_True;
            break;
        default:
            aReturn = OJoinController::GetState(_nId);
            break;
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void OQueryController::Execute(sal_uInt16 _nId)
{
    switch(_nId)
    {
        case ID_BROWSER_ESACPEPROCESSING:
            m_bEsacpeProcessing = !m_bEsacpeProcessing;
            InvalidateFeature(ID_BROWSER_SQL);
            break;
        case ID_BROWSER_EDITDOC:
            m_bEditable = !m_bEditable;
            m_pWindow->getView()->setReadOnly(!m_bEditable);
            InvalidateFeature(ID_BROWSER_PASTE);
            InvalidateFeature(ID_BROWSER_CLEAR_QUERY);
            break;
        case ID_BROWSER_SAVEASDOC:
        case ID_BROWSER_SAVEDOC:
            {
                OSL_ENSURE(m_bEditable,"Slot ID_BROWSER_SAVEDOC should not be enabled!");

                Reference<XNameAccess> xQueries = getQueries();
                if(xQueries.is())
                {
                    // first we need a name for our query so ask the user
                    sal_Bool bNew = 0 == m_sName.getLength();
                    bNew = bNew || (ID_BROWSER_SAVEASDOC == _nId);
                    if(bNew)
                    {
                        String aDefaultName;
                        if(ID_BROWSER_SAVEASDOC == _nId && !bNew)
                            aDefaultName = String(m_sName);
                        else
                        {
                            String aName = String(ModuleRes(STR_QRY_TITLE));
                            aName = aName.GetToken(0,' ');
                            aDefaultName = String(::dbtools::createUniqueName(xQueries,aName));
                        }

                        Reference<XDatabaseMetaData> xMeta;
                        if(m_xConnection.is())
                            xMeta = m_xConnection->getMetaData();
                        OSaveAsDlg aDlg(
                                getView(), CommandType::QUERY, xQueries,
                                xMeta, aDefaultName,
                                (ID_BROWSER_SAVEASDOC == _nId) ? SAD_OVERWRITE : SAD_DEFAULT);

                        if(aDlg.Execute() == RET_OK)
                            m_sName = aDlg.getName();
                    }
                    // did we get a name
                    if(m_sName.getLength())
                    {
                        SQLExceptionInfo aInfo;
                        try
                        {
                            Reference<XPropertySet> xQuery;
                            if(bNew || !xQueries->hasByName(m_sName)) // just to make sure the query already exists
                            {
                                if(xQueries->hasByName(m_sName))
                                {
                                    Reference<XDrop> xNameCont(xQueries,UNO_QUERY);
                                    OSL_ENSURE(xNameCont.is(),"Can not drop query!");
                                    if(xNameCont.is())
                                        xNameCont->dropByName(m_sName);
                                }

                                Reference<XDataDescriptorFactory> xFact(xQueries,UNO_QUERY);
                                OSL_ENSURE(xFact.is(),"No XDataDescriptorFactory available!");
                                xQuery = xFact->createDataDescriptor();
                                OSL_ENSURE(xQuery.is(),"OQueryController::Execute ID_BROWSER_SAVEDOC: Create query failed!");
                                // to set the name is only allowed when the wuery is new
                                xQuery->setPropertyValue(PROPERTY_NAME,makeAny(m_sName));
                            }
                            else
                            {
                                ::cppu::extractInterface(xQuery,xQueries->getByName(m_sName));
                            }
                            // now set the properties
                            m_sStatement = m_pWindow->getView()->getStatement();
                            ::rtl::OUString sTranslatedStmt;
                            if(m_sStatement.getLength() && m_xComposer.is() && m_bEsacpeProcessing)
                            {
                                try
                                {
                                    ::rtl::OUString aErrorMsg;
                                    ::connectivity::OSQLParseNode* pNode = m_pSqlParser->parseTree(aErrorMsg,m_sStatement,sal_True);
                                    //  m_pParseNode = pNode;
                                    if(pNode)
                                    {
                                        pNode->parseNodeToStr(  sTranslatedStmt,
                                                                m_xConnection->getMetaData());
                                        delete pNode;
                                    }
                                    m_xComposer->setQuery(sTranslatedStmt);
                                    sTranslatedStmt = m_xComposer->getComposedQuery();
                                }
                                catch(SQLException& e)
                                {
                                    ::dbtools::SQLExceptionInfo aInfo(e);
                                    showError(aInfo);
                                    break;
                                }
                            }
                            else if(!m_sStatement.getLength())
                            {
                                ErrorBox aBox( getQueryView(), ModuleRes( ERR_QRY_NOSELECT ) );
                                aBox.Execute();
                                break;
                            }
                            else
                                sTranslatedStmt = m_sStatement;

                            xQuery->setPropertyValue(PROPERTY_COMMAND,makeAny(sTranslatedStmt));
                            xQuery->setPropertyValue(CONFIGKEY_QRYDESCR_USE_ESCAPE_PROCESSING,::cppu::bool2any(m_bEsacpeProcessing));
                            xQuery->setPropertyValue(CONFIGKEY_QRYDESCR_UPDATE_TABLENAME,makeAny(m_sUpdateTableName));
                            xQuery->setPropertyValue(CONFIGKEY_QRYDESCR_UPDATE_CATALOGNAME,makeAny(m_sUpdateCatalogName));
                            xQuery->setPropertyValue(CONFIGKEY_QRYDESCR_UPDATE_SCHEMANAME,makeAny(m_sUpdateSchemaName));


                            // now we save the layout information
                            //  create the output stream
                            m_pWindow->getView()->SaveUIConfig();
                            Sequence< sal_Int8 > aOutputSeq;
                            {
                                Reference< XOutputStream>       xOutStreamHelper = new OSequenceOutputStream(aOutputSeq);
                                Reference< XObjectOutputStream> xOutStream(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.ObjectOutputStream")),UNO_QUERY);
                                Reference< XOutputStream>   xMarkOutStream(getORB()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.io.MarkableOutputStream")),UNO_QUERY);
                                Reference< XActiveDataSource >(xMarkOutStream,UNO_QUERY)->setOutputStream(xOutStreamHelper);
                                Reference< XActiveDataSource > xOutDataSource(xOutStream, UNO_QUERY);
                                OSL_ENSURE(xOutDataSource.is(),"Couldn't create com.sun.star.io.ObjectOutputStream!");
                                xOutDataSource->setOutputStream(xMarkOutStream);
                                Save(xOutStream);
                                xQuery->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aOutputSeq));
                            }

                            if(bNew)
                            {
                                Reference<XAppend> xAppend(xQueries,UNO_QUERY);
                                OSL_ENSURE(xAppend.is(),"No XAppend Interface!");
                                xAppend->appendByDescriptor(xQuery);
                            }
                            setModified(sal_False);
                        }
                        catch(SQLContext& e) { aInfo = SQLExceptionInfo(e); }
                        catch(SQLWarning& e) { aInfo = SQLExceptionInfo(e); }
                        catch(SQLException& e) { aInfo = SQLExceptionInfo(e); }
                        catch(Exception&)
                        {
                            OSL_ENSURE(0,"Query could not be inserted!");
                        }
                        showError(aInfo);
                    }
                }
            }
            break;
        case SID_PRINTDOCDIRECT:
            break;
        case ID_BROWSER_CUT:
            m_pWindow->getView()->cut();
            break;
        case ID_BROWSER_COPY:
            m_pWindow->getView()->copy();
            break;
        case ID_BROWSER_PASTE:
            m_pWindow->getView()->paste();
            break;
        case ID_BROWSER_SQL:
            {
                try
                {
                    ::rtl::OUString aErrorMsg;
                    m_sStatement = m_pWindow->getView()->getStatement();
                    if(!m_sStatement.getLength() && m_pSqlIterator)
                    {
                        // change the view of the data
                        delete m_pSqlIterator->getParseTree();
                        m_pSqlIterator->setParseTree(NULL);
                        m_bDesign = !m_bDesign;
                        m_pWindow->switchView();
                    }
                    else
                    {
                        ::connectivity::OSQLParseNode* pNode = m_pSqlParser->parseTree(aErrorMsg,m_sStatement,sal_True);
                        //  m_pParseNode = pNode;
                        if(pNode)
                        {
                            delete m_pSqlIterator->getParseTree();
                            m_pSqlIterator->setParseTree(pNode);
                            m_pSqlIterator->traverseAll();
                            SQLWarning aWarning = m_pSqlIterator->getWarning();
                            if(aWarning.Message.getLength())
                                showError(SQLExceptionInfo(aWarning));
                            else
                            {
                                const OSQLTables& xTabs = m_pSqlIterator->getTables();
                                if( m_pSqlIterator->getStatementType() != SQL_STATEMENT_SELECT && m_pSqlIterator->getStatementType() != SQL_STATEMENT_SELECT_COUNT || xTabs.begin() == xTabs.end())
                                {
                                    ErrorBox aBox( getQueryView(), ModuleRes( ERR_QRY_NOSELECT ) );
                                    aBox.Execute();
                                }
                                else
                                {
                                    // change the view of the data
                                    m_bDesign = !m_bDesign;
                                    m_sStatement = ::rtl::OUString();
                                    pNode->parseNodeToStr(  m_sStatement,
                                                            m_xConnection->getMetaData(),
                                                            &getParser()->getContext(),
                                                            sal_True,sal_True);
                                    m_pWindow->getView()->SaveUIConfig();
                                    m_pWindow->switchView();
                                }
                            }
                        }
                        else
                        {
                            ErrorBox aBox( getQueryView(), ModuleRes( ERR_QRY_SYNTAX ) );
                            aBox.Execute();
                        }
                    }
                }
                catch(SQLException& e)
                {
                    ::dbtools::SQLExceptionInfo aInfo(e);
                    showError(aInfo);
                }
                catch(Exception&)
                {
                }
                if(m_bDesign)
                    InvalidateFeature(ID_BROWSER_ADDTABLE);
            }
            break;
        case ID_BROWSER_CLEAR_QUERY:
            m_pWindow->getView()->clear();
            m_sStatement = ::rtl::OUString();
            if(m_bDesign)
                InvalidateFeature(ID_BROWSER_ADDTABLE);
            //  InvalidateFeature(ID_BROWSER_QUERY_EXECUTE);
            break;
        case ID_BROWSER_QUERY_VIEW_FUNCTIONS:
        case ID_BROWSER_QUERY_VIEW_TABLES:
        case ID_BROWSER_QUERY_VIEW_ALIASES:
            m_pWindow->getView()->setSlotEnabled(_nId,!m_pWindow->getView()->isSlotEnabled(_nId));
            break;
        case ID_BROWSER_QUERY_DISTINCT_VALUES:
            m_bDistinct = !m_bDistinct;
            break;
        case ID_BROWSER_QUERY_EXECUTE:
            {
                executeQuery();

            }
            break;
        case ID_QUERY_ZOOM_IN:
            {
//              m_aZoom *= Fraction(1,10);
//              static_cast<OQueryViewSwitch*>(getQueryView())->zoomTableView(m_aZoom);
            }
            break;
        case ID_QUERY_ZOOM_OUT:
            {
//              if(m_aZoom != Fraction(1,1))
//                  m_aZoom /= Fraction(1,10);
//              static_cast<OQueryViewSwitch*>(getQueryView())->zoomTableView(m_aZoom);
            }
            break;
        default:
            OJoinController::Execute(_nId);
            return; // else we would invalidate twice
    }
    InvalidateFeature(_nId);
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryController::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
{
    try
    {
        OJoinController::initialize(aArguments);

        //  m_pWindow->initialize(m_xCurrentFrame);

        PropertyValue aValue;
        const Any* pBegin   = aArguments.getConstArray();
        const Any* pEnd     = pBegin + aArguments.getLength();

        for(;pBegin != pEnd;++pBegin)
        {
            if((*pBegin >>= aValue) && aValue.Name == PROPERTY_ACTIVECONNECTION)
            {
                ::cppu::extractInterface(m_xConnection,aValue.Value);
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
            else if(aValue.Name == PROPERTY_CURRENTQUERY)
            {
                aValue.Value >>= m_sName;
            }
            else if(aValue.Name == PROPERTY_QUERYDESIGNVIEW)
            {
                m_bDesign = ::cppu::any2bool(aValue.Value);
            }
        }

        if (!m_xConnection.is())
        {   // whoever instantiated us did not give us a connection to share. Okay, create an own one
            createNewConnection(sal_False);
        }
        if (!m_xConnection.is())    // we have no connection so what else should we do
            m_bDesign = sal_False;

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

        // get command from the query if a query name was supplied
        if(m_sName.getLength())
        {
            Reference<XNameAccess> xQueries = getQueries();
            if(xQueries.is())
            {
                Reference<XPropertySet> xProp;
                if(xQueries->hasByName(m_sName) && ::cppu::extractInterface(xProp,xQueries->getByName(m_sName)) && xProp.is())
                {
                    xProp->getPropertyValue(PROPERTY_COMMAND) >>= m_sStatement;
                    m_bEsacpeProcessing = ::cppu::any2bool(xProp->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING));
                    // load the layoutInformation
                    try
                    {
                        Sequence< sal_Int8 > aInputSequence;
                        xProp->getPropertyValue(PROPERTY_LAYOUTINFORMATION) >>= aInputSequence;
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
                    catch(Exception&)
                    {
                    }
                    setQueryComposer();
                    ::rtl::OUString aErrorMsg;
                    ::connectivity::OSQLParseNode* pNode = m_pSqlParser->parseTree(aErrorMsg,m_sStatement,sal_True);
                    //  m_pParseNode = pNode;
                    if(pNode)
                    {
                        if(m_pSqlIterator)
                        {
                            delete m_pSqlIterator->getParseTree();
                            m_pSqlIterator->setParseTree(pNode);
                            m_pSqlIterator->traverseAll();
                            SQLWarning aWarning = m_pSqlIterator->getWarning();
                            if(aWarning.Message.getLength())
                            {
                                showError(SQLExceptionInfo(aWarning));
                                m_bDesign = sal_False;
                            }
                        }
                        else
                        {
                            delete pNode;
                            m_bDesign = sal_False;
                        }
                    }
                    else
                    {
                        String aTitle(ModuleRes(STR_SVT_SQL_SYNTAX_ERROR));
                        OSQLMessageBox aDlg(getView(),aTitle,aErrorMsg);
                        aDlg.Execute();
                        m_bDesign = sal_False; // the statement can't be parsed so we show the text view
                    }
                }
            }

        }
        else
            setQueryComposer();
        if(!m_xFormatter.is() && m_xDataSource.is())
        {
            Reference< XNumberFormatsSupplier> xSupplier;
            ::cppu::extractInterface(xSupplier,m_xDataSource->getPropertyValue(PROPERTY_NUMBERFORMATSSUPPLIER));
            if(xSupplier.is())
            {
                m_xFormatter = Reference< ::com::sun::star::util::XNumberFormatter >(getORB()
                    ->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
                m_xFormatter->attachNumberFormatsSupplier(xSupplier);
            }
            OSL_ENSURE(m_xFormatter.is(),"No NumberFormatter!");
        }
        m_pWindow->getView()->initialize();
        getUndoMgr()->Clear();
        if(m_bDesign && !m_sName.getLength())
            Execute(ID_BROWSER_ADDTABLE);
        setModified(sal_False);
    }
    catch(SQLException&)
    {
        OSL_ENSURE(sal_False, "OQueryController::initialize: caught an exception!");
    }

}
// -----------------------------------------------------------------------------
void OQueryController::setQueryComposer()
{
    if(m_xConnection.is())
    {
        Reference< XSQLQueryComposerFactory >  xFactory(m_xConnection, UNO_QUERY);
        OSL_ENSURE(xFactory.is(),"Connection doesn't support a querycomposer");
        if (xFactory.is())
        {
            try
            {
                m_xComposer = xFactory->createQueryComposer();
                m_pWindow->getView()->setStatement(m_sStatement);
                setModified(sal_False);
            }
            catch (Exception&)
            {
                m_xComposer = NULL;
            }
            OSL_ENSURE(m_xComposer.is(),"No querycomposer available!");
            Reference<XTablesSupplier> xTablesSup(m_xConnection, UNO_QUERY);
            deleteIterator();
            m_pSqlIterator = new ::connectivity::OSQLParseTreeIterator(xTablesSup->getTables(),m_xConnection->getMetaData(),NULL,m_pSqlParser);
        }
    }
}
// -----------------------------------------------------------------------------
VCLXWindow* OQueryController::getWindowPeer()
{
    return m_pWindow->GetWindowPeer();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryController::Construct(Window* pParent)
{
    // TODO: we have to check if we should create the text- or the design- view
    m_pWindow = new OQueryContainerWindow(pParent,this,m_xMultiServiceFacatory);
    //  OQueryTextView *pView = new OQueryTextView(pParent,this,m_xMultiServiceFacatory);
    m_pWindow->getView()->Construct(NULL);
    //  m_pView  = pView;
    m_pView  = m_pWindow->getView()->getRealView();
    OGenericUnoController::Construct(pParent);
    //  getView()->Show();
    m_pWindow->Show();
    return sal_True;
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryController::propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryController::elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw( RuntimeException )
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryController::elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw( RuntimeException )
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryController::elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw( RuntimeException )
{
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OQueryController::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    if(m_xConnection.is() && m_bModified && (!m_bDesign || (m_vTableFieldDesc.size() && m_vTableData.size())))
    {
        QueryBox aQry(getView(), ModuleRes(QUERY_DESIGN_SAVEMODIFIED));
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
void OQueryController::AddSupportedFeatures()
{
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Redo")] = ID_BROWSER_REDO;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Save")] = ID_BROWSER_SAVEDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:DBSlots/Undo")] = ID_BROWSER_UNDO;

    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:BrowserMode")] = SID_BROWSER_MODE;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:HelpMenu")]    = SID_HELPMENU;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:NewDoc")]      = SID_NEWDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:SaveAsDoc")]   = SID_SAVEASDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:ExplorerContentOpen")]         = SID_EXPLORERCONTENT_OPEN;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:ExplorerContentOpenDocument")] = SID_EXPLORERCONTENT_OPEN_DOCUMENT;

}
// -----------------------------------------------------------------------------
ToolBox* OQueryController::CreateToolBox(Window* _pParent)
{
    if(m_pView)
        return m_pView->getToolBox();
    return new ToolBox(_pParent, ModuleRes(RID_BRW_QUERYDESIGN_TOOLBOX));
}
// -----------------------------------------------------------------------------
void OQueryController::setModified(sal_Bool _bModified)
{
    OJoinController::setModified(_bModified);
    InvalidateFeature(ID_BROWSER_CLEAR_QUERY);
    InvalidateFeature(ID_BROWSER_SAVEASDOC);
    InvalidateFeature(ID_BROWSER_QUERY_EXECUTE);
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryController::disposing( const EventObject& Source ) throw(RuntimeException)
{
    if(Reference<XFrame>(Source.Source,UNO_QUERY).is())
    { // the beamer was closed so resize our window
        m_pWindow->hideBeamer();
    }
    else
        OJoinController::disposing(Source);
}
// -----------------------------------------------------------------------------
void OQueryController::createNewConnection(sal_Bool _bUI)
{
    deleteIterator();
    ::comphelper::disposeComponent(m_xComposer);

    OJoinController::createNewConnection(_bUI);
    if (m_xConnection.is())
    {
        // we hide the add table dialog because the tables in it are from the old connection
        if(m_pAddTabDlg)
            m_pAddTabDlg->Hide();
        InvalidateFeature(ID_BROWSER_ADDTABLE);
        setQueryComposer();
    }
    else
    {
        if(m_bDesign)
        {
            m_bDesign = sal_False;
            // don't call Execute(SQL) because this changes the sql statement
            m_pWindow->switchView();
        }
        InvalidateAll();
    }
}
// -----------------------------------------------------------------------------
void OQueryController::Save(const Reference< XObjectOutputStream>& _rxOut)
{
    OJoinController::Save(_rxOut);
    OStreamSection aSection(_rxOut.get());

    // some data
    _rxOut << m_nSplitPos;
    _rxOut << m_nVisibleRows;

    // the fielddata
    _rxOut << (sal_Int32)m_vTableFieldDesc.size();
    ::std::vector<OTableFieldDesc*>::const_iterator aFieldIter = m_vTableFieldDesc.begin();
    for(;aFieldIter != m_vTableFieldDesc.end();++aFieldIter)
        (*aFieldIter)->Save(_rxOut);
}
// -----------------------------------------------------------------------------
void OQueryController::Load(const Reference< XObjectInputStream>& _rxIn)
{
    OJoinController::Load(_rxIn);
    OStreamSection aSection(_rxIn.get());
    try
    {
        // some data
        _rxIn >> m_nSplitPos;
        _rxIn >> m_nVisibleRows;

        //////////////////////////////////////////////////////////////////////
        // Liste loeschen
        ::std::vector< OTableFieldDesc*>::iterator aFieldIter = m_vTableFieldDesc.begin();
        for(;aFieldIter != m_vTableFieldDesc.end();++aFieldIter)
            delete *aFieldIter;
        m_vTableFieldDesc.clear();

        sal_Int32 nCount = 0;
        _rxIn >> nCount;
        for(sal_Int32 j=0;j<nCount;++j)
        {
            OTableFieldDesc* pData = new OTableFieldDesc();
            pData->Load(_rxIn);
            m_vTableFieldDesc.push_back(pData);
        }
    }
    catch(Exception&)
    {
    }
}

// -----------------------------------------------------------------------------
OTableWindowData* OQueryController::createTableWindowData()
{
    return new OQueryTableWindowData();
}
// -----------------------------------------------------------------------------
Reference<XNameAccess> OQueryController::getQueries()
{
    Reference<XNameAccess> xQueries;
    Reference<XQueriesSupplier> xConSup(m_xConnection,UNO_QUERY);
    if(xConSup.is())
        xQueries = xConSup->getQueries();
    else
    {
        Reference<XQueryDefinitionsSupplier> xSup(m_xDataSource,UNO_QUERY);
        if(xSup.is())
            xQueries = xSup->getQueryDefinitions();
    }
    return xQueries;
}
// -----------------------------------------------------------------------------
void OQueryController::executeQuery()
{
    // we don't need to check the connection here because we already check the composer
    // which can't live without his connection
    m_sStatement = m_pWindow->getView()->getStatement();
    ::rtl::OUString sTranslatedStmt;
    if(m_sStatement.getLength() && m_xComposer.is() && m_bEsacpeProcessing)
    {
        try
        {
            ::rtl::OUString aErrorMsg;
            ::connectivity::OSQLParseNode* pNode = m_pSqlParser->parseTree(aErrorMsg,m_sStatement,sal_True);
            //  m_pParseNode = pNode;
            if(pNode)
            {
                pNode->parseNodeToStr(  sTranslatedStmt,
                                        m_xConnection->getMetaData());
                delete pNode;
            }
            m_xComposer->setQuery(sTranslatedStmt);
            sTranslatedStmt = m_xComposer->getComposedQuery();
        }
        catch(SQLException& e)
        {
            ::dbtools::SQLExceptionInfo aInfo(e);
            showError(aInfo);
            return;
        }
    }
    else
        sTranslatedStmt = m_sStatement;
    if(m_sDataSourceName.getLength() && sTranslatedStmt.getLength())
    {
        try
        {
            URL aWantToDispatch;
            aWantToDispatch.Complete = ::rtl::OUString::createFromAscii(".component:DB/DataSourceBrowser");

            ::rtl::OUString sFrameName = ::rtl::OUString::createFromAscii("_beamer");
            //  | ::com::sun::star::frame::FrameSearchFlag::CREATE
            sal_Int32 nSearchFlags = FrameSearchFlag::CHILDREN;
            m_pWindow->showBeamer(m_xCurrentFrame);

            Reference< ::com::sun::star::frame::XDispatch> xDisp;
            Reference< ::com::sun::star::frame::XDispatchProvider> xProv(m_xCurrentFrame->findFrame(sFrameName,nSearchFlags),UNO_QUERY);
            if(!xProv.is())
            {
                xProv = Reference< ::com::sun::star::frame::XDispatchProvider>(m_xCurrentFrame, UNO_QUERY);
                if (xProv.is())
                    xDisp = xProv->queryDispatch(aWantToDispatch, sFrameName, nSearchFlags);
            }
            else
            {
                xDisp = xProv->queryDispatch(aWantToDispatch, sFrameName, FrameSearchFlag::SELF);
            }
            if (xDisp.is())
            {
                Sequence< PropertyValue> aProps(10);
                aProps[0].Name = PROPERTY_DATASOURCENAME;
                aProps[0].Value <<= m_sDataSourceName;

                aProps[1].Name = PROPERTY_COMMANDTYPE;
                aProps[1].Value <<= CommandType::COMMAND;

                aProps[2].Name = PROPERTY_COMMAND;
                aProps[2].Value <<= sTranslatedStmt;

                aProps[3].Name = PROPERTY_SHOWTREEVIEW;
                aProps[3].Value = ::cppu::bool2any(sal_False);

                aProps[4].Name = PROPERTY_SHOWTREEVIEWBUTTON;
                aProps[4].Value = ::cppu::bool2any(sal_False);

                aProps[5].Name = PROPERTY_ACTIVECONNECTION;
                aProps[5].Value <<= m_xConnection;

                aProps[6].Name = PROPERTY_UPDATE_CATALOGNAME;
                aProps[6].Value <<= m_sUpdateCatalogName;

                aProps[7].Name = PROPERTY_UPDATE_SCHEMANAME;
                aProps[7].Value <<= m_sUpdateSchemaName;

                aProps[8].Name = PROPERTY_UPDATE_TABLENAME;
                aProps[8].Value <<= m_sUpdateTableName;

                aProps[9].Name = PROPERTY_USE_ESCAPE_PROCESSING;
                aProps[9].Value = ::cppu::bool2any(m_bEsacpeProcessing);

                xDisp->dispatch(aWantToDispatch, aProps);
                // check the state of the beamer
                // be notified when the beamer frame is closed
                Reference< XComponent >  xComponent(m_xCurrentFrame->findFrame(sFrameName,nSearchFlags), UNO_QUERY);
                if (xComponent.is())
                {
                    Reference< ::com::sun::star::lang::XEventListener> xEvtL((::cppu::OWeakObject*)this,UNO_QUERY);
                    xComponent->addEventListener(xEvtL);
                }
            }
            else
            {
                OSL_ENSURE(0,"Couldn't create a beamer window!");
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Couldn't create a beamer window!");
        }
    }
    else
    {
        sal_Bool bAllEmpty = sal_True;
        ::std::vector< OTableFieldDesc*>::const_iterator aIter = m_vTableFieldDesc.begin();
        for(;bAllEmpty && aIter != m_vTableFieldDesc.end();++aIter)
            bAllEmpty = (*aIter)->IsEmpty();

        ErrorBox(getQueryView(), ModuleRes(bAllEmpty ? ERR_QRY_NOCRITERIA : ERR_QRY_NOSTATEMENT)).Execute();
    }
}
// -----------------------------------------------------------------------------


