/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppControllerGen.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 13:59:39 $
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

#ifndef DBAUI_APPCONTROLLER_HXX
#include "AppController.hxx"
#endif

#include <com/sun/star/ucb/Command.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

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
#ifndef _COM_SUN_STAR_SDBCX_XRENAME_HPP_
#include <com/sun/star/sdbcx/XRename.hpp>
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
#ifndef INCLUDED_SFX_MAILMODELAPI_HXX
#include <sfx2/mailmodelapi.hxx>
#endif
#ifndef DBAUI_APPVIEW_HXX
#include "AppView.hxx"
#endif
#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif
#ifndef _UTL_BOOTSTRAP_HXX
#include <unotools/bootstrap.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_MNEMONIC_HXX
#include <vcl/mnemonic.hxx>
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
#ifndef DBACCESS_SOURCE_UI_MISC_DEFAULTOBJECTNAMECHECK_HXX
#include "defaultobjectnamecheck.hxx"
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif

//........................................................................
namespace dbaui
{
using namespace ::dbtools;
using namespace ::svx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
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
using ::com::sun::star::util::XCloseable;
//........................................................................
// -----------------------------------------------------------------------------

class CloseChecker : public ::cppu::WeakImplHelper1< com::sun::star::lang::XEventListener >
{
    bool    m_bClosed;

public:
    CloseChecker()
        :m_bClosed( false )
    {
    }

    virtual ~CloseChecker()
    {
    }

    bool isClosed()
    {
        return true;
    }

    // interface XEventListener
    virtual void SAL_CALL disposing( const EventObject& /*Source*/ ) throw( RuntimeException )
    {
        m_bClosed = true;
    }

};
// -----------------------------------------------------------------------------
void OApplicationController::convertToView(const ::rtl::OUString& _sName)
{
    try
    {
        SharedConnection xConnection( getConnection() );
        Reference< XQueriesSupplier > xSup( xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xQueries( xSup->getQueries(), UNO_QUERY_THROW );
        Reference< XPropertySet > xSourceObject( xQueries->getByName( _sName ), UNO_QUERY_THROW );

        Reference< XTablesSupplier > xTablesSup( xConnection, UNO_QUERY_THROW );
        Reference< XNameAccess > xTables( xTablesSup->getTables(), UNO_QUERY_THROW );

        Reference< XDatabaseMetaData  > xMeta = xConnection->getMetaData();

        String aName = String(ModuleRes(STR_TBL_TITLE));
        aName = aName.GetToken(0,' ');
        String aDefaultName = ::dbaui::createDefaultName(xMeta,xTables,aName);

        DynamicTableOrQueryNameCheck aNameChecker( xConnection, CommandType::TABLE );
        OSaveAsDlg aDlg( getView(), CommandType::TABLE, getORB(), xConnection, aDefaultName, aNameChecker );
        if ( aDlg.Execute() == RET_OK )
        {
            ::rtl::OUString sName = aDlg.getName();
            ::rtl::OUString sCatalog = aDlg.getCatalog();
            ::rtl::OUString sSchema  = aDlg.getSchema();
            ::rtl::OUString sNewName(
                ::dbtools::composeTableName( xMeta, sCatalog, sSchema, sName, sal_False, ::dbtools::eInTableDefinitions ) );
            Reference<XPropertySet> xView = ::dbaui::createView(sNewName,xConnection,xSourceObject);
            if ( !xView.is() )
                throw SQLException(String(ModuleRes(STR_NO_TABLE_FORMAT_INSIDE)),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000")) ,0,Any());
            getContainer()->elementAdded(E_TABLE,sNewName,makeAny(xView));
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
            {
                m_aTableCopyHelper.pasteTable( _nFormatId, rClipboard, getDatabaseName(), ensureConnection() );
            }
            else
                paste( eType, ODataAccessObjectTransferable::extractObjectDescriptor( rClipboard ) );

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
    openDialog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.DatasourceAdministrationDialog" ) ) );
}

// -----------------------------------------------------------------------------
void OApplicationController::openDialog( const ::rtl::OUString& _sServiceName )
{
    try
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
        ::osl::MutexGuard aGuard(m_aMutex);
        WaitObject aWO(getView());

        Sequence< Any > aArgs(3);
        sal_Int32 nArgPos = 0;

        Reference< ::com::sun::star::awt::XWindow> xWindow = getTopMostContainerWindow();
        if ( !xWindow.is() )
        {
            DBG_ASSERT( getContainer(), "OApplicationController::Construct: have no view!" );
            if ( getContainer() )
                xWindow = VCLUnoHelper::GetInterface(getView()->Window::GetParent());
        }
        // the parent window
        aArgs[nArgPos++] <<= PropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow")),
                                    0,
                                    makeAny(xWindow),
                                    PropertyState_DIRECT_VALUE);

        // the initial selection
        ::rtl::OUString sInitialSelection;
        if ( getContainer() )
            sInitialSelection = getDatabaseName();
        if ( sInitialSelection.getLength() )
        {
            aArgs[ nArgPos++ ] <<= PropertyValue(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InitialSelection" ) ), 0,
                makeAny( sInitialSelection ), PropertyState_DIRECT_VALUE );
        }

        SharedConnection xConnection( getConnection() );
        if ( xConnection.is() )
        {
            aArgs[ nArgPos++ ] <<= PropertyValue(
                PROPERTY_ACTIVE_CONNECTION, 0,
                makeAny( xConnection ), PropertyState_DIRECT_VALUE );
        }
        aArgs.realloc( nArgPos );

        // create the dialog
        Reference< XExecutableDialog > xAdminDialog;
        xAdminDialog = Reference< XExecutableDialog >(
            getORB()->createInstanceWithArguments(_sServiceName,aArgs), UNO_QUERY);

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
    openDialog( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.TableFilterDialog" ) ) );
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
        getContainer()->getDetailView()->createTablesPage( ensureConnection() );
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::openDirectSQLDialog()
{
    openDialog( SERVICE_SDB_DIRECTSQLDIALOG );
}
// -----------------------------------------------------------------------------
void SAL_CALL OApplicationController::propertyChange( const PropertyChangeEvent& evt ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( evt.PropertyName == PROPERTY_USER )
    {
        m_bNeedToReconnect = sal_True;
        InvalidateFeature(SID_DB_APP_STATUS_USERNAME);
    }
    else if ( evt.PropertyName == PROPERTY_URL )
    {
        m_bNeedToReconnect = sal_True;
        InvalidateFeature(SID_DB_APP_STATUS_DBNAME);
        InvalidateFeature(SID_DB_APP_STATUS_TYPE);
        InvalidateFeature(SID_DB_APP_STATUS_HOSTNAME);
    }
    else if ( PROPERTY_NAME == evt.PropertyName )
    {
        const ElementType eType = getContainer()->getElementType();
        if ( eType == E_FORM || eType == E_REPORT )
        {
            ::rtl::OUString sOldName,sNewName;
            evt.OldValue >>= sOldName;
            evt.NewValue >>= sNewName;
            Reference<XChild> xChild(evt.Source,UNO_QUERY);
            if ( xChild.is() )
            {
                Reference<XContent> xContent(xChild->getParent(),UNO_QUERY);
                if ( xContent.is() )
                    sOldName = xContent->getIdentifier()->getContentIdentifier() + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sOldName;
            }

            getContainer()->elementReplaced( eType , sOldName, sNewName );
        }
    }

    EventObject aEvt;
    aEvt.Source = m_xModel;
    modified(aEvt);
}

// -----------------------------------------------------------------------------
Reference< XDataSource > SAL_CALL OApplicationController::getDataSource() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    Reference< XDataSource > xDataSource( m_xDataSource, UNO_QUERY );
    return xDataSource;
}

// -----------------------------------------------------------------------------
Reference< XWindow > SAL_CALL OApplicationController::getApplicationMainWindow() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    Reference< XFrame > xFrame( getFrame(), UNO_QUERY_THROW );
    Reference< XWindow > xWindow( xFrame->getContainerWindow(), UNO_QUERY_THROW );
    return xWindow;
}

// -----------------------------------------------------------------------------
Sequence< Reference< XComponent > > SAL_CALL OApplicationController::getSubComponents() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    Sequence< Reference< XComponent > > aComponents( m_aDocuments.size() );
    ::std::transform( m_aDocuments.begin(), m_aDocuments.end(), aComponents.getArray(), ::std::select1st< TDocuments::value_type >() );
    return aComponents;
}

// -----------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OApplicationController::getActiveConnection() throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xDataSourceConnection.getTyped();
}

// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OApplicationController::isConnected(  ) throw (RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xDataSourceConnection.is();
}

// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OApplicationController::connect(  ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    ensureConnection();
    return isConnected();
}

// -----------------------------------------------------------------------------
namespace
{
    static Reference< XController > lcl_getController( const OApplicationController::TDocuments::iterator& _docPos )
    {
        Reference< XController > xController;

        Reference< XComponent > xComponent( _docPos->first );
        Reference< XModel > xModel( xComponent, UNO_QUERY );
        if ( xModel.is() )
            xController = xModel->getCurrentController();
        else
        {
            xController.set( xComponent, UNO_QUERY );
            if ( !xController.is() )
            {
                Reference<XFrame> xFrame( xComponent, UNO_QUERY );
                if ( xFrame.is() )
                    xController = xFrame->getController();
            }
        }
        return xController;
    }
}

// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OApplicationController::closeSubComponents(  ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( m_aMutex );

    try
    {
        typedef ::std::vector< Reference< XComponent > > ComponentArray;
        ComponentArray aClosedComponents;

        TDocuments aDocuments( m_aDocuments );
        for (   TDocuments::iterator doc = aDocuments.begin();
                doc != aDocuments.end();
                ++doc
            )
        {
            Reference< XController > xController( lcl_getController( doc ) );
            OSL_ENSURE( xController.is(), "OApplicationController::closeSubComponents: did not find the sub controller!" );

            // suspend the controller in the document
            if  (   !xController.is()
                ||  !xController->suspend( sal_True )
                )
                // break complete operation, no sense in continueing
                break;

            // revoke event listener
            Reference< XComponent > xDocument = doc->first;
            if ( xDocument.is() )
                xDocument->removeEventListener( static_cast< XFrameActionListener* >( this ) );

            bool bClosedSubDoc = false;
            try
            {
                Reference< XCloseable > xCloseable( xController->getFrame(), UNO_QUERY_THROW );
                xCloseable->close( sal_True );
                bClosedSubDoc = true;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            if ( !bClosedSubDoc )
                // no sense in continueing with the other docs
                break;

            aClosedComponents.push_back( doc->first );
        }

        // now remove all the components which we could successfully close
        // (this might be none, or all, or something inbetween) from m_aDocuments
        for (   ComponentArray::const_iterator comp = aClosedComponents.begin();
                comp != aClosedComponents.end();
                ++comp
            )
        {
            TDocuments::iterator pos = m_aDocuments.find( *comp );
            OSL_ENSURE( pos != m_aDocuments.end(),
                "OApplicationController::closeSubComponents: closed a component which doesn't exist anymore!" );
            if ( pos !=m_aDocuments.end() )
                m_aDocuments.erase( pos );
        }
    }
    catch ( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return m_aDocuments.empty();
}

// -----------------------------------------------------------------------------
void OApplicationController::previewChanged( sal_Int32 _nMode )
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( m_xDataSource.is() && !isDataSourceReadOnly() )
    {
        try
        {
            ::comphelper::NamedValueCollection aLayoutInfo( m_xDataSource->getPropertyValue( PROPERTY_LAYOUTINFORMATION ) );
            sal_Int32 nOldMode = aLayoutInfo.getOrDefault( "Preview", _nMode );
            if ( nOldMode != _nMode )
            {
                aLayoutInfo.put( "Preview", _nMode );
                m_xDataSource->setPropertyValue( PROPERTY_LAYOUTINFORMATION, makeAny( aLayoutInfo.getPropertyValues() ) );
            }
        }
        catch ( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    InvalidateFeature(SID_DB_APP_DISABLE_PREVIEW);
    InvalidateFeature(SID_DB_APP_VIEW_DOCINFO_PREVIEW);
    InvalidateFeature(SID_DB_APP_VIEW_DOC_PREVIEW);
}
// -----------------------------------------------------------------------------
//void OApplicationController::updateTitle()
//{
//  ::rtl::OUString sName = getStrippedDatabaseName();
//
//  String sTitle = String(ModuleRes(STR_APP_TITLE));
//  sName = sName + sTitle;
//#ifndef PRODUCT
//    ::rtl::OUString aDefault;
//  sName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ["));
//    sName += utl::Bootstrap::getBuildIdData( aDefault );
//  sName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("]"));
//#endif
//}
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
                    closeSubComponents();
                    break;
                default:
                    bClear = sal_False;
                    break;
            }
        }
        if ( bClear )
        {
            ElementType eType = getContainer()->getElementType();
            disconnect();
            getContainer()->getDetailView()->clearPages(sal_False);
            getContainer()->selectContainer(E_NONE); // invalidate the old selection
            m_eCurrentType = E_NONE;
            getContainer()->selectContainer(eType); // reselect the current one again
        }
    }
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
        try
        {
            m_aDocuments[_xDocument] = _xDefintion;
            _xDocument->addEventListener(static_cast<XFrameActionListener*>(this));
            Reference<XPropertySet> xProp(_xDefintion,UNO_QUERY_THROW);
            if ( xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_NAME) )
                xProp->addPropertyChangeListener(PROPERTY_NAME,static_cast<XPropertyChangeListener*>(this));
        }
        catch(Exception&)
        {
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::insertHierachyElement(ElementType _eType,const String& _sParentFolder,sal_Bool _bCollection,const Reference<XContent>& _xContent,sal_Bool _bMove)
{
    Reference<XHierarchicalNameContainer> xNames(getElements(_eType), UNO_QUERY);
    return dbaui::insertHierachyElement(getView()
                           ,getORB()
                           ,xNames
                           ,_sParentFolder
                           ,_eType == E_FORM
                           ,_bCollection
                           ,_xContent
                           ,_bMove);
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::isRenameDeleteAllowed(ElementType _eType,sal_Bool _bDelete) const
{
    ElementType eType = getContainer()->getElementType();
    sal_Bool bEnabled = !isDataSourceReadOnly() && eType == _eType;
    if ( bEnabled )
    {

        if ( E_TABLE == eType )
            bEnabled = !isConnectionReadOnly() && getContainer()->isALeafSelected();

        sal_Bool bCompareRes = sal_False;
        if ( _bDelete )
            bCompareRes = getContainer()->getSelectionCount() > 0;
        else
        {
            bCompareRes = getContainer()->getSelectionCount() == 1;
            if ( bEnabled && bCompareRes && E_TABLE == eType )
            {
                ::std::vector< ::rtl::OUString> aList;
                const_cast<OApplicationController*>(this)->getSelectionElementNames(aList);

                try
                {
                    Reference< XNameAccess > xContainer = const_cast<OApplicationController*>(this)->getElements(eType);
                    bEnabled = (xContainer.is() && xContainer->hasByName(*aList.begin()));
                    if ( bEnabled )
                        bEnabled = Reference<XRename>(xContainer->getByName(*aList.begin()),UNO_QUERY).is();
                }
                catch(Exception&)
                {
                    bEnabled = sal_False;
                }
            }
        }

        bEnabled = bEnabled && bCompareRes;
    }
    return bEnabled;
}
// -----------------------------------------------------------------------------
void OApplicationController::onLoadedMenu(const Reference< ::com::sun::star::frame::XLayoutManager >& _xLayoutManager)
{

    if ( _xLayoutManager.is() )
    {
        static ::rtl::OUString s_sStatusbar(RTL_CONSTASCII_USTRINGPARAM("private:resource/statusbar/statusbar"));
        _xLayoutManager->createElement( s_sStatusbar );
        _xLayoutManager->requestElement( s_sStatusbar );

        if ( getContainer() )
        {
            // we need to share the "mnemonic space":
            MnemonicGenerator aMnemonicGenerator;
            // - the menu already has mnemonics
            SystemWindow* pSystemWindow = getContainer()->GetSystemWindow();
            MenuBar* pMenu = pSystemWindow ? pSystemWindow->GetMenuBar() : NULL;
            if ( pMenu )
            {
                USHORT nMenuItems = pMenu->GetItemCount();
                for ( USHORT i = 0; i < nMenuItems; ++i )
                    aMnemonicGenerator.RegisterMnemonic( pMenu->GetItemText( pMenu->GetItemId( i ) ) );
            }
            // - the icons should use automatic ones
            getContainer()->createIconAutoMnemonics( aMnemonicGenerator );
            // - as well as the entries in the task pane
            getContainer()->setTaskExternalMnemonics( aMnemonicGenerator );
        }

        Execute( SID_DB_APP_VIEW_FORMS, Sequence< PropertyValue >() );
        InvalidateAll();
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::doAction(sal_uInt16 _nId ,OLinkedDocumentsAccess::EOpenMode _eOpenMode)
{
    ::std::vector< ::rtl::OUString> aList;
    getSelectionElementNames(aList);
    ElementType eType = getContainer()->getElementType();

    ::std::vector< ::std::pair< ::rtl::OUString ,Reference< XModel > > > aCompoments;
    ::std::vector< ::rtl::OUString>::iterator aEnd = aList.end();
    for (::std::vector< ::rtl::OUString>::iterator aIter = aList.begin(); aIter != aEnd; ++aIter)
    {
        if ( SID_DB_APP_CONVERTTOVIEW == _nId )
            convertToView(*aIter);
        else
        {
            Reference< XModel > xModel( openElement( *aIter, eType, _eOpenMode, _nId ), UNO_QUERY );
            aCompoments.push_back( ::std::pair< ::rtl::OUString, Reference< XModel > >( *aIter, xModel ) );
        }
    }

    // special handling for mail, if more than one document is selected attach them all
    if ( _eOpenMode == OLinkedDocumentsAccess::OPEN_FORMAIL )
    {
        ::std::vector< ::std::pair< ::rtl::OUString ,Reference< XModel > > >::iterator componentIter = aCompoments.begin();
        ::std::vector< ::std::pair< ::rtl::OUString ,Reference< XModel > > >::iterator componentEnd = aCompoments.end();
        ::rtl::OUString aDocTypeString;
        SfxMailModel aSendMail;
        SfxMailModel::SendMailResult eResult = SfxMailModel::SEND_MAIL_OK;
        for (; componentIter != componentEnd && SfxMailModel::SEND_MAIL_OK == eResult; ++componentIter)
        {
            Reference< XModel > xModel(componentIter->second,UNO_QUERY);

            // Send document as e-Mail using stored/default type
            eResult = aSendMail.AttachDocument(aDocTypeString,xModel,componentIter->first);
        }
        if ( !aSendMail.IsEmpty() )
            aSendMail.Send( getFrame() );
    }
}
// -----------------------------------------------------------------------------
ElementType OApplicationController::getElementType(const Reference< XContainer >& _xContainer) const
{
    ElementType eRet = E_NONE;
    Reference<XServiceInfo> xServiceInfo(_xContainer,UNO_QUERY);
    if ( xServiceInfo.is() )
    {
        if ( xServiceInfo->supportsService(SERVICE_SDBCX_TABLES) )
            eRet = E_TABLE;
        else if ( xServiceInfo->supportsService(SERVICE_NAME_FORM_COLLECTION) )
            eRet = E_FORM;
        else if ( xServiceInfo->supportsService(SERVICE_NAME_REPORT_COLLECTION) )
            eRet = E_REPORT;
        else
            eRet = E_QUERY;
    }
    return eRet;
}
//........................................................................
}   // namespace dbaui
//........................................................................


