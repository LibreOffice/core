/*************************************************************************
 *
 *  $RCSfile: AppControllerGen.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:28:39 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef DBAUI_APPCONTROLLER_HXX
#include "AppController.hxx"
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef DBAUI_DLGSAVE_HXX
#include "dlgsave.hxx"
#endif
#ifndef _DBAUI_DLGRENAME_HXX
#include "dlgrename.hxx"
#endif
#ifndef DBAUI_APPVIEW_HXX
#include "AppView.hxx"
#endif
#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef DBAUI_APPDETAILVIEW_HXX
#include "AppDetailView.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _DBACCESS_SLOTID_HRC_
#include "dbaccess_slotid.hrc"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMECONTAINER_HPP_
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

//........................................................................
namespace dbaui
{
using namespace ::dbtools;
using namespace ::svx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ucb;
//........................................................................
void OApplicationController::convertToView(const ::rtl::OUString& _sName)
{
    try
    {
        Reference<XConnection> xConnection = getActiveConnection();
        Reference<XQueriesSupplier> xSup(xConnection,UNO_QUERY);
        if ( xSup.is() )
        {
            Reference<XNameAccess> xQueries = xSup->getQueries();
            if ( xQueries.is() && xQueries->hasByName(_sName) )
            {
                Reference<XPropertySet> xSourceObject(xQueries->getByName( _sName ),UNO_QUERY);

                OSL_ENSURE(xSourceObject.is(),"Query is NULL!");
                if ( xSourceObject.is() )
                {
                    Reference<XTablesSupplier> xTablesSup(xConnection,UNO_QUERY);
                    Reference<XNameAccess> xTables;
                    if ( xTablesSup.is() )
                        xTables = xTablesSup->getTables();

                    Reference<XDatabaseMetaData > xMeta = xConnection->getMetaData();

                    String aName = String(ModuleRes(STR_TBL_TITLE));
                    aName = aName.GetToken(0,' ');
                    String aDefaultName = ::dbaui::createDefaultName(xMeta,xTables,aName);

                    OSaveAsDlg aDlg(getView(),CommandType::TABLE,xTables,xMeta,xConnection,aDefaultName);
                    if ( aDlg.Execute() == RET_OK )
                    {
                        ::rtl::OUString sName = aDlg.getName();
                        ::rtl::OUString sCatalog = aDlg.getCatalog();
                        ::rtl::OUString sSchema  = aDlg.getSchema();
                        ::rtl::OUString sNewName;
                        ::dbtools::composeTableName(xMeta,sCatalog,sSchema,sName,sNewName,sal_False,::dbtools::eInTableDefinitions);
                        Reference<XPropertySet> xView = ::dbaui::createView(sNewName,xConnection,xSourceObject);
                        if ( !xView.is() )
                            throw SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")) ,0,Any());
                        getContainer()->elementAdded(E_TABLE,sNewName,makeAny(xView),xConnection);
                    }
                }
            }
        }
    }
    catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
    catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
    catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
    catch(Exception& )
    {
        OSL_ENSURE(sal_False, "OApplicationController::convertToView: caught a generic exception!");
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::pasteFormat(sal_uInt32 _nFormatId)
{
    if ( _nFormatId )
    {
        try
        {
            const TransferableDataHelper& rClipboard = getViewClipboard();
            ElementType eType = getContainer()->getElementType();
            if ( eType == E_TABLE )
                pasteTable(_nFormatId, rClipboard );
            else
                paste( eType,ODataAccessObjectTransferable::extractObjectDescriptor(rClipboard) );

        }
        catch(Exception& )
        {
            OSL_ENSURE(0,"Exception catched!");
        }
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::openDataSourceAdminDialog()
{
    openDialog(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DatasourceAdministrationDialog")));
}
// -----------------------------------------------------------------------------
void OApplicationController::openDialog(const ::rtl::OUString& _sServiceName)
{
    try
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::MutexGuard aGuard(m_aMutex);
        WaitObject aWO(getView());

        Sequence< Any > aArgs(2);

        Reference< ::com::sun::star::awt::XWindow> xWindow = getTopMostContainerWindow();
        if ( !xWindow.is() )
        {
            DBG_ASSERT( getContainer(), "OApplicationController::Construct: have no view!" );
            if ( getContainer() )
                xWindow = VCLUnoHelper::GetInterface(getView()->Window::GetParent());
        }
        // the parent window
        aArgs[0] <<= PropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow")),
                                    0,
                                    makeAny(xWindow),
                                    PropertyState_DIRECT_VALUE);

        // the initial selection
        ::rtl::OUString sInitialSelection;
        if ( getContainer() )
            sInitialSelection = getDatabaseName();
        aArgs[1] <<= PropertyValue(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InitialSelection")), 0,
            makeAny(sInitialSelection), PropertyState_DIRECT_VALUE);

        // create the dialog
        Reference< XExecutableDialog > xAdminDialog;
        xAdminDialog = Reference< XExecutableDialog >(
            m_xMultiServiceFacatory->createInstanceWithArguments(_sServiceName,aArgs), UNO_QUERY);

        // execute it
        if (xAdminDialog.is())
            xAdminDialog->execute();
    }
    catch(Exception&)
    {
        DBG_ERROR("OApplicationController::implAdministrate: caught an exception while creating/executing the dialog!");
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::openTableFilterDialog()
{
    openDialog(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.TableFilterDialog")));
}
// -----------------------------------------------------------------------------
void OApplicationController::closeConnection()
{
    if ( getContainer() )
    {
        TDataSourceConnections::iterator aFind = m_aDataSourceConnections.find(getDatabaseName());
        if ( aFind != m_aDataSourceConnections.end() )
            disconnect(aFind->second);
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::refreshTables()
{
    if ( getContainer() && getContainer()->getDetailView() )
    {
        WaitObject aWO(getView());
        OSL_ENSURE(getContainer()->getElementType() == E_TABLE,"Only allowed when the tables container is selected!");
        try
        {
            Reference<XRefreshable> xRefresh(getElements(E_TABLE),UNO_QUERY);
            if ( xRefresh.is() )
                xRefresh->refresh();
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Could not refresh tables!");
        }

        getContainer()->getDetailView()->clearPages(sal_False);
        Reference<XConnection> xConnection;
        ensureConnection(xConnection);
        getContainer()->getDetailView()->createTablesPage(xConnection);
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::openDirectSQLDialog()
{
    openDialog(::rtl::OUString(SERVICE_SDB_DIRECTSQLDIALOG));
}
// -----------------------------------------------------------------------------
void SAL_CALL OApplicationController::propertyChange( const PropertyChangeEvent& evt ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    m_bNeedToReconnect = sal_True;
    if ( getContainer() && m_xDataSource.is() )
        getContainer()->setStatusInformations(m_xDataSource);

    EventObject aEvt;
    aEvt.Source = m_xDataSource;
    modified(aEvt);
}
// -----------------------------------------------------------------------------
void OApplicationController::previewChanged( sal_Int32 _nMode )
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( m_xDataSource.is() )
    {
        try
        {
            Sequence<PropertyValue> aFields;
            m_xDataSource->getPropertyValue(PROPERTY_LAYOUTINFORMATION) >>= aFields;
            PropertyValue *pIter = aFields.getArray();
            PropertyValue *pEnd = pIter + aFields.getLength();
            const static ::rtl::OUString s_sPreview(RTL_CONSTASCII_USTRINGPARAM("Preview"));
            for (; pIter != pEnd && pIter->Name != s_sPreview; ++pIter)
                ;

            if ( pIter == pEnd )
            {
                sal_Int32 nLen = aFields.getLength();
                aFields.realloc( nLen + 1 );
                pIter = aFields.getArray() + nLen;
                pIter->Name = s_sPreview;
            }
            sal_Int32 nOldMode = 0;
            pIter->Value >>= nOldMode;
            if ( nOldMode != _nMode )
            {
                pIter->Value <<= _nMode;
                m_xDataSource->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aFields));
            }
        }
        catch(Exception)
        {
            OSL_ENSURE(0,"Exception caught!");
        }
    }
    InvalidateFeature(SID_DB_APP_DISABLE_PREVIEW);
    InvalidateFeature(SID_DB_APP_VIEW_DOCINFO_PREVIEW);
    InvalidateFeature(SID_DB_APP_VIEW_DOC_PREVIEW);
}
// -----------------------------------------------------------------------------
void OApplicationController::updateTitle()
{
    ::rtl::OUString sName = getStrippedDatabaseName();

    String sTitle = String(ModuleRes(STR_APP_TITLE));
    sName = sName + sTitle;
    OGenericUnoController::setTitle(sName);
}
// -----------------------------------------------------------------------------
void OApplicationController::askToReconnect()
{
    if ( m_bNeedToReconnect )
    {
        m_bNeedToReconnect = sal_False;
        sal_Bool bClear = sal_True;
        if ( !m_aDocuments.empty() )
        {
            QueryBox aQry(getView(), ModuleRes(APP_CLOSEDOCUMENTS));
            switch (aQry.Execute())
            {
                case RET_YES:
                    suspendDocuments(sal_True);
                    break;
                default:
                    bClear = sal_False;
                    break;
            }
        }
        if ( bClear )
        {
            ElementType eType = getContainer()->getElementType();
            clearConnections();
            getContainer()->changeContainer(E_NONE); // invalidate the old selection
            getContainer()->changeContainer(eType); // reselect the current one again
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::suspendDocuments(sal_Bool bSuspend)
{
    sal_Bool bSubSuspended = sal_True;
    Reference<XModel> xModel;
    TDocuments::iterator aIter = m_aDocuments.begin();
    TDocuments::iterator aEnd = m_aDocuments.end();
    try
    {
        for (; aIter != aEnd && bSubSuspended; ++aIter)
        {
            Reference<XController> xController;
            xModel.set(aIter->first,UNO_QUERY);
            if ( xModel.is() )
                xController = xModel->getCurrentController();
            else
            {
                xController.set(aIter->first,UNO_QUERY);
                if ( !xController.is() )
                {
                    Reference<XFrame> xFrame(aIter->first,UNO_QUERY);
                    if ( xFrame.is() )
                        xController = xFrame->getController();
                }
            }


            if ( xController.is() && xController != *this )
            {
                bSubSuspended = xController->suspend(bSuspend);
            }
        }
    }
    catch(Exception)
    {
    }
    if ( bSubSuspended && !m_aDocuments.empty() )
    {
        try
        {
            TDocuments::iterator aIter = m_aDocuments.begin();
            TDocuments::iterator aEnd = m_aDocuments.end();
            for (; aIter != aEnd ;  ++aIter)
            {
                Reference< XComponent > xDocument = aIter->first;
                if ( xDocument.is() )
                    xDocument->removeEventListener(static_cast<XFrameActionListener*>(this));
            }
            aIter = m_aDocuments.begin();
            // first of all we have to set the second to NULL
            for (; aIter != aEnd ;  )
            {
                TDocuments::iterator aPos = aIter++;
                aPos->second = NULL; // this may also dispose the document
            }
            // work on copy
            TDocuments  aDocuments = m_aDocuments;
            aIter = aDocuments.begin();
            aEnd = aDocuments.end();
            for (; aIter != aEnd ; ++aIter )
            {
                Reference<XController> xController;
                xModel.set(aIter->first,UNO_QUERY);
                if ( xModel.is() )
                    xController = xModel->getCurrentController();
                else
                {
                    xController.set(aIter->first,UNO_QUERY);
                    if ( !xController.is() )
                    {
                        Reference<XFrame> xFrame(aIter->first,UNO_QUERY);
                        if ( xFrame.is() )
                            xController = xFrame->getController();
                    }
                }

                if ( xController.is() && xController != *this )
                {
                    Reference< com::sun::star::util::XCloseable> xCloseable(xController->getFrame(),UNO_QUERY);
                    if ( xCloseable.is() )
                        xCloseable->close(sal_True);
                }
            }
        }
        catch(Exception)
        {
        }
        m_aDocuments.clear();
    }

    return bSubSuspended;
}
// -----------------------------------------------------------------------------
::rtl::OUString OApplicationController::getStrippedDatabaseName() const
{
    return ::dbaui::getStrippedDatabaseName(m_xDataSource,m_sDatabaseName);
}
// -----------------------------------------------------------------------------
void OApplicationController::addDocumentListener(const Reference< XComponent >& _xDocument,const Reference< XComponent >& _xDefintion)
{
    if ( _xDocument.is() )
    {
        m_aDocuments[_xDocument] = _xDefintion;
        _xDocument->addEventListener(static_cast<XFrameActionListener*>(this));
    }
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::insertHierachyElement(ElementType _eType,const String& _sParentFolder,sal_Bool _bCollection,const Reference<XContent>& _xContent,sal_Bool _bMove)
{
    Reference<XHierarchicalNameContainer> xNames(getElements(_eType), UNO_QUERY);
    return dbaui::insertHierachyElement(getView()
                           ,xNames
                           ,_sParentFolder
                           ,_eType == E_FORM
                           ,_bCollection
                           ,_xContent
                           ,_bMove);
}
//........................................................................
}   // namespace dbaui
//........................................................................


