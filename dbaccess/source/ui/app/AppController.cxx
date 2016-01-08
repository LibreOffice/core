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

#include "AppController.hxx"
#include "dbustrings.hrc"
#include "advancedsettingsdlg.hxx"
#include "subcomponentmanager.hxx"
#include "uiservices.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/ErrorMessageDialog.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>
#include <com/sun/star/sdbcx/XAlterView.hpp>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/sdbcx/XRename.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#include <com/sun/star/sdb/application/MacroMigrationWizard.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/sdb/application/DatabaseObject.hpp>
#include <com/sun/star/sdb/application/DatabaseObjectContainer.hpp>
#include <com/sun/star/document/XDocumentEventBroadcaster.hpp>
#include <com/sun/star/container/XHierarchicalName.hpp>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>

#include <svl/urihelper.hxx>
#include <svl/filenotation.hxx>
#include <svtools/treelistbox.hxx>
#include <svtools/transfer.hxx>
#include <svtools/cliplistener.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/insdlg.hxx>

#include <comphelper/sequence.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/types.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/stdtext.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menu.hxx>
#include <vcl/lstbox.hxx>

#include <unotools/closeveto.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/historyoptions.hxx>

#include <sfx2/mailmodelapi.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/QuerySaveDocument.hxx>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <connectivity/dbtools.hxx>
#include <connectivity/dbexception.hxx>

#include <svx/dbaexchange.hxx>
#include <svx/dbaobjectex.hxx>
#include <svx/svxdlg.hxx>

#include <osl/mutex.hxx>
#include "AppView.hxx"
#include "browserids.hxx"
#include "dbu_reghelper.hxx"
#include "dbu_app.hrc"
#include "defaultobjectnamecheck.hxx"
#include "databaseobjectview.hxx"
#include "listviewitems.hxx"
#include "AppDetailView.hxx"
#include "linkeddocuments.hxx"
#include "sqlmessage.hxx"
#include "UITools.hxx"
#include "dsntypes.hxx"
#include "dbaccess_helpid.hrc"
#include "dlgsave.hxx"
#include "dbaccess_slotid.hrc"

#include <functional>

#include <boost/noncopyable.hpp>

extern "C" void SAL_CALL createRegistryInfo_ODBApplication()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OApplicationController > aAutoRegistration;
}

namespace dbaui
{
using namespace ::dbtools;
using namespace ::svx;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::task;
using ::com::sun::star::document::XEmbeddedScripts;
using ::com::sun::star::document::XDocumentEventBroadcaster;
using ::com::sun::star::document::DocumentEvent;
using ::com::sun::star::sdb::application::NamedDatabaseObject;

namespace DatabaseObject = ::com::sun::star::sdb::application::DatabaseObject;
namespace DatabaseObjectContainer = ::com::sun::star::sdb::application::DatabaseObjectContainer;

OUString SAL_CALL OApplicationController::getImplementationName() throw( RuntimeException, std::exception )
{
    return getImplementationName_Static();
}

OUString OApplicationController::getImplementationName_Static() throw( RuntimeException )
{
    return OUString(SERVICE_SDB_APPLICATIONCONTROLLER);
}

Sequence< OUString> OApplicationController::getSupportedServiceNames_Static() throw( RuntimeException )
{
    Sequence<OUString> aSupported { "com.sun.star.sdb.application.DefaultViewController" };
    return aSupported;
}

Sequence< OUString> SAL_CALL OApplicationController::getSupportedServiceNames() throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

Reference< XInterface > SAL_CALL OApplicationController::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new OApplicationController( comphelper::getComponentContext(_rxFactory)));
}

// OApplicationController
class SelectionNotifier : public ::boost::noncopyable
{
private:
    ::cppu::OInterfaceContainerHelper   m_aSelectionListeners;
    ::cppu::OWeakObject&                m_rContext;
    sal_Int32                           m_nSelectionNestingLevel;

public:
    SelectionNotifier( ::osl::Mutex& _rMutex, ::cppu::OWeakObject& _rContext )
        :m_aSelectionListeners( _rMutex )
        ,m_rContext( _rContext )
        ,m_nSelectionNestingLevel( 0 )
    {
    }

    void addListener( const Reference< XSelectionChangeListener >& _Listener )
    {
        m_aSelectionListeners.addInterface( _Listener );
    }

    void removeListener( const Reference< XSelectionChangeListener >& _Listener )
    {
        m_aSelectionListeners.removeInterface( _Listener );
    }

    void disposing()
    {
        EventObject aEvent( m_rContext );
        m_aSelectionListeners.disposeAndClear( aEvent );
    }

    ~SelectionNotifier()
    {
    }

    struct SelectionGuardAccess { friend class SelectionGuard; private: SelectionGuardAccess() { }  };

    /** enters a block which modifies the selection of our owner.

        Can be called multiple times, the only important thing is to call leaveSelection
        equally often.
    */
    void    enterSelection( SelectionGuardAccess )
    {
        ++m_nSelectionNestingLevel;
    }

    /** leaves a block which modifies the selection of our owner

        Must be paired with enterSelection calls.

        When the last block is left, i.e. the last leaveSelection call is made on the current stack,
        then our SelectionChangeListeners are notified
    */
    void    leaveSelection( SelectionGuardAccess )
    {
        if ( --m_nSelectionNestingLevel == 0 )
        {
            EventObject aEvent( m_rContext );
            m_aSelectionListeners.notifyEach( &XSelectionChangeListener::selectionChanged, aEvent );
        }
    }
};

class SelectionGuard : public ::boost::noncopyable
{
public:
    explicit SelectionGuard( SelectionNotifier& _rNotifier )
        :m_rNotifier( _rNotifier )
    {
        m_rNotifier.enterSelection( SelectionNotifier::SelectionGuardAccess() );
    }

    ~SelectionGuard()
    {
        m_rNotifier.leaveSelection( SelectionNotifier::SelectionGuardAccess() );
    }

private:
    SelectionNotifier&  m_rNotifier;
};

// OApplicationController
OApplicationController::OApplicationController(const Reference< XComponentContext >& _rxORB)
    :OGenericUnoController( _rxORB )
    ,m_aContextMenuInterceptors( getMutex() )
    ,m_pSubComponentManager( new SubComponentManager( *this, getSharedMutex() ) )
    ,m_aTypeCollection( _rxORB )
    ,m_aTableCopyHelper(this)
    ,m_pClipbordNotifier(nullptr)
    ,m_nAsyncDrop(nullptr)
    ,m_aSelectContainerEvent( LINK( this, OApplicationController, OnSelectContainer ) )
    ,m_ePreviewMode(E_PREVIEWNONE)
    ,m_eCurrentType(E_NONE)
    ,m_bNeedToReconnect(false)
    ,m_bSuspended( false )
    ,m_pSelectionNotifier( new SelectionNotifier( getMutex(), *this ) )
{
}

OApplicationController::~OApplicationController()
{
    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        OSL_FAIL("Please check who doesn't dispose this component!");
        // increment ref count to prevent double call of Dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
    clearView();
}

IMPLEMENT_FORWARD_XTYPEPROVIDER2(OApplicationController,OGenericUnoController,OApplicationController_Base)
IMPLEMENT_FORWARD_XINTERFACE2(OApplicationController,OGenericUnoController,OApplicationController_Base)
void OApplicationController::disconnect()
{
    if ( m_xDataSourceConnection.is() )
        stopConnectionListening( m_xDataSourceConnection );

    try
    {
        // temporary (hopefully!) hack for #i55274#
        Reference< XFlushable > xFlush( m_xDataSourceConnection, UNO_QUERY );
        if ( xFlush.is() && m_xMetaData.is() && !m_xMetaData->isReadOnly() )
            xFlush->flush();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    m_xDataSourceConnection.clear();
    m_xMetaData.clear();

    InvalidateAll();
}

void SAL_CALL OApplicationController::disposing()
{
    for( const auto& rContainerListener : m_aCurrentContainers )
    {
        if( rContainerListener.is() )
        {
            rContainerListener->removeContainerListener( this );
        }
    }

    m_aCurrentContainers.clear();
    m_pSubComponentManager->disposing();
    m_pSelectionNotifier->disposing();

    if ( getView() )
    {
        getContainer()->showPreview(nullptr);
        m_pClipbordNotifier->ClearCallbackLink();
        m_pClipbordNotifier->AddRemoveListener( getView(), false );
        m_pClipbordNotifier->release();
        m_pClipbordNotifier = nullptr;
    }

    disconnect();
    try
    {
        Reference < XFrame > xFrame;
        attachFrame( xFrame );

        if ( m_xDataSource.is() )
        {
            m_xDataSource->removePropertyChangeListener(OUString(), this);
            m_xDataSource->removePropertyChangeListener(PROPERTY_INFO, this);
            m_xDataSource->removePropertyChangeListener(PROPERTY_URL, this);
            m_xDataSource->removePropertyChangeListener(PROPERTY_ISPASSWORDREQUIRED, this);
            m_xDataSource->removePropertyChangeListener(PROPERTY_LAYOUTINFORMATION, this);
            m_xDataSource->removePropertyChangeListener(PROPERTY_SUPPRESSVERSIONCL, this);
            m_xDataSource->removePropertyChangeListener(PROPERTY_TABLEFILTER, this);
            m_xDataSource->removePropertyChangeListener(PROPERTY_TABLETYPEFILTER, this);
            m_xDataSource->removePropertyChangeListener(PROPERTY_USER, this);
            // otherwise we may delete our datasource twice
            Reference<XPropertySet> xProp = m_xDataSource;
            m_xDataSource = nullptr;
        }

        Reference< XModifyBroadcaster > xBroadcaster( m_xModel, UNO_QUERY );
        if ( xBroadcaster.is() )
            xBroadcaster->removeModifyListener(static_cast<XModifyListener*>(this));

        if ( m_xModel.is() )
        {
            OUString sUrl = m_xModel->getURL();
            if ( !sUrl.isEmpty() )
            {
                ::comphelper::NamedValueCollection aArgs( m_xModel->getArgs() );
                if ( aArgs.getOrDefault( "PickListEntry", true ) )
                {
                    OUString     aFilter;
                    INetURLObject       aURL( m_xModel->getURL() );
                    const SfxFilter* pFilter = getStandardDatabaseFilter();
                    if ( pFilter )
                        aFilter = pFilter->GetFilterName();

                    // add to svtool history options
                    SvtHistoryOptions().AppendItem( ePICKLIST,
                            aURL.GetURLNoPass( INetURLObject::NO_DECODE ),
                            aFilter,
                            getStrippedDatabaseName(),
                            OUString(),
                            boost::none);

                    // add to recent document list
                    if ( aURL.GetProtocol() == INetProtocol::File )
                        Application::AddToRecentDocumentList( aURL.GetURLNoPass( INetURLObject::NO_DECODE ),
                                                              (pFilter) ? pFilter->GetMimeType() : OUString(),
                                                              (pFilter) ? pFilter->GetServiceName() : OUString() );
                }
            }

            m_xModel->disconnectController( this );

            m_xModel.clear();
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    clearView();
    OGenericUnoController::disposing(); // here the m_refCount must be equal 5
}

bool OApplicationController::Construct(vcl::Window* _pParent)
{
    setView( VclPtr<OApplicationView>::Create( _pParent, getORB(), *this, m_ePreviewMode ) );
    getView()->SetUniqueId(UID_APP_VIEW);

    // late construction
    bool bSuccess = false;
    try
    {
        getContainer()->Construct();
        bSuccess = true;
    }
    catch(const SQLException&)
    {
    }
    catch(const Exception&)
    {
        OSL_FAIL("OApplicationController::Construct : the construction of UnoDataBrowserView failed !");
    }

    if ( !bSuccess )
    {
        clearView();
        return false;
    }

    // now that we have a view we can create the clipboard listener
    m_aSystemClipboard = TransferableDataHelper::CreateFromSystemClipboard( getView() );
    m_aSystemClipboard.StartClipboardListening( );

    m_pClipbordNotifier = new TransferableClipboardListener( LINK( this, OApplicationController, OnClipboardChanged ) );
    m_pClipbordNotifier->acquire();
    m_pClipbordNotifier->AddRemoveListener( getView(), true );

    OGenericUnoController::Construct( _pParent );
    getView()->Show();

    return true;
}

void SAL_CALL OApplicationController::disposing(const EventObject& _rSource) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( getMutex() );
    Reference<XConnection> xCon(_rSource.Source, UNO_QUERY);
    if ( xCon.is() )
    {
        OSL_ENSURE( m_xDataSourceConnection == xCon,
            "OApplicationController::disposing: which connection does this come from?" );

        if ( getContainer() && getContainer()->getElementType() == E_TABLE )
            getContainer()->clearPages();
        if ( m_xDataSourceConnection == xCon )
        {
            m_xMetaData.clear();
            m_xDataSourceConnection.clear();
        }
    }
    else if ( _rSource.Source == m_xModel )
    {
        m_xModel.clear();
    }
    else if ( _rSource.Source == m_xDataSource )
    {
        m_xDataSource = nullptr;
    }
    else
    {
        Reference<XContainer> xContainer( _rSource.Source, UNO_QUERY );
        if ( xContainer.is() )
        {
            TContainerVector::iterator aFind = ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xContainer);
            if ( aFind != m_aCurrentContainers.end() )
                m_aCurrentContainers.erase(aFind);
        }
        OGenericUnoController::disposing( _rSource );
    }
}

sal_Bool SAL_CALL OApplicationController::suspend(sal_Bool bSuspend) throw( RuntimeException, std::exception )
{
    // notify the OnPrepareViewClosing event (before locking any mutex)
    Reference< XDocumentEventBroadcaster > xBroadcaster( m_xModel, UNO_QUERY );
    if ( xBroadcaster.is() )
    {
        xBroadcaster->notifyDocumentEvent(
            "OnPrepareViewClosing",
            this,
            Any()
        );
    }

    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    if ( getView() && getView()->IsInModalMode() )
        return sal_False;

    bool bCanSuspend = true;

    if ( m_bSuspended != bool(bSuspend) )
    {
        if ( bSuspend && !closeSubComponents() )
            return sal_False;

        Reference<XModifiable> xModi(m_xModel,UNO_QUERY);
        Reference<XStorable> xStor(getModel(),UNO_QUERY);

        if  (   bSuspend
            &&  xStor.is()
            &&  !xStor->isReadonly()
            &&  (   xModi.is()
                &&  xModi->isModified()
                )
            )
        {
            switch (ExecuteQuerySaveDocument(getView(),getStrippedDatabaseName()))
            {
                case RET_YES:
                    Execute(ID_BROWSER_SAVEDOC,Sequence<PropertyValue>());
                    bCanSuspend = !xModi->isModified();
                    // when we save the document this must be false else some press cancel
                    break;
                case RET_CANCEL:
                    bCanSuspend = false;
                default:
                    break;
            }
        }
    }

    if ( bCanSuspend )
        m_bSuspended = bSuspend;

    return bCanSuspend;
}

FeatureState OApplicationController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    aReturn.bEnabled = false;
    // check this first
    if ( !getContainer() || m_bReadOnly )
        return aReturn;

    try
    {
        switch (_nId)
        {
            case SID_OPENURL:
                aReturn.bEnabled = true;
                if ( m_xModel.is() )
                    aReturn.sTitle = m_xModel->getURL();
                break;
            case ID_BROWSER_COPY:
                {
                    sal_Int32 nCount = getContainer()->getSelectionCount();
                    aReturn.bEnabled = nCount >= 1;
                    if ( aReturn.bEnabled && nCount == 1 && getContainer()->getElementType() == E_TABLE )
                        aReturn.bEnabled = getContainer()->isALeafSelected();
                }
                break;
            case ID_BROWSER_CUT:
                aReturn.bEnabled = !isDataSourceReadOnly() && getContainer()->getSelectionCount() >= 1;
                aReturn.bEnabled = aReturn.bEnabled && ( !(ID_BROWSER_CUT == _nId && getContainer()->getElementType() == E_TABLE) || getContainer()->isCutAllowed() );
                break;
            case ID_BROWSER_PASTE:
                switch( getContainer()->getElementType() )
                {
                    case E_TABLE:
                        aReturn.bEnabled = !isDataSourceReadOnly() && !isConnectionReadOnly() && isTableFormat();
                        break;
                    case E_QUERY:
                        aReturn.bEnabled = !isDataSourceReadOnly() && getViewClipboard().HasFormat(SotClipboardFormatId::DBACCESS_QUERY);
                        break;
                    default:
                        aReturn.bEnabled = !isDataSourceReadOnly() && OComponentTransferable::canExtractComponentDescriptor(getViewClipboard().GetDataFlavorExVector(),getContainer()->getElementType() == E_FORM);
                }
                break;
            case SID_DB_APP_PASTE_SPECIAL:
                aReturn.bEnabled = getContainer()->getElementType() == E_TABLE && !isDataSourceReadOnly() && !isConnectionReadOnly() && isTableFormat();
                break;
            case SID_OPENDOC:
                aReturn.bEnabled = true;
                break;
            case ID_BROWSER_SAVEDOC:
                aReturn.bEnabled = !isDataSourceReadOnly();
                break;
            case ID_BROWSER_SAVEASDOC:
                aReturn.bEnabled = true;
                break;
            case ID_BROWSER_SORTUP:
                aReturn.bEnabled = getContainer()->isFilled() && getContainer()->getElementCount();
                aReturn.bChecked = aReturn.bEnabled && getContainer()->isSortUp();
                break;
            case ID_BROWSER_SORTDOWN:
                aReturn.bEnabled = getContainer()->isFilled() && getContainer()->getElementCount();
                aReturn.bChecked = aReturn.bEnabled && !getContainer()->isSortUp();
                break;

            case SID_NEWDOC:
            case SID_APP_NEW_FORM:
            case ID_DOCUMENT_CREATE_REPWIZ:
                aReturn.bEnabled = !isDataSourceReadOnly() && SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::EModule::WRITER);
                break;
            case SID_APP_NEW_REPORT:
                aReturn.bEnabled = !isDataSourceReadOnly()
                                    && SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::EModule::WRITER);
                if ( aReturn.bEnabled )
                {
                    Reference< XContentEnumerationAccess > xEnumAccess(m_xContext->getServiceManager(), UNO_QUERY);
                    aReturn.bEnabled = xEnumAccess.is();
                    if ( aReturn.bEnabled )
                    {
                        const OUString sReportEngineServiceName = ::dbtools::getDefaultReportEngineServiceName(m_xContext);
                        aReturn.bEnabled = !sReportEngineServiceName.isEmpty();
                        if ( aReturn.bEnabled )
                        {
                            const Reference< XEnumeration > xEnumDrivers = xEnumAccess->createContentEnumeration(sReportEngineServiceName);
                            aReturn.bEnabled = xEnumDrivers.is() && xEnumDrivers->hasMoreElements();
                        }
                    }
                }
                break;
            case SID_DB_APP_VIEW_TABLES:
                aReturn.bEnabled = true;
                aReturn.bChecked = getContainer()->getElementType() == E_TABLE;
                break;
            case SID_DB_APP_VIEW_QUERIES:
                aReturn.bEnabled = true;
                aReturn.bChecked = getContainer()->getElementType() == E_QUERY;
                break;
            case SID_DB_APP_VIEW_FORMS:
                aReturn.bEnabled = true;
                aReturn.bChecked = getContainer()->getElementType() == E_FORM;
                break;
            case SID_DB_APP_VIEW_REPORTS:
                aReturn.bEnabled = true;
                aReturn.bChecked = getContainer()->getElementType() == E_REPORT;
                break;
            case ID_NEW_QUERY_DESIGN:
            case ID_NEW_QUERY_SQL:
            case ID_APP_NEW_QUERY_AUTO_PILOT:
            case SID_DB_FORM_NEW_PILOT:
                aReturn.bEnabled = !isDataSourceReadOnly();
                break;
            case ID_NEW_VIEW_DESIGN:
            case SID_DB_NEW_VIEW_SQL:
            case ID_NEW_VIEW_DESIGN_AUTO_PILOT:
                aReturn.bEnabled = !isDataSourceReadOnly() && !isConnectionReadOnly();
                if ( aReturn.bEnabled )
                {
                    Reference<XViewsSupplier> xViewsSup( getConnection(), UNO_QUERY );
                    aReturn.bEnabled = xViewsSup.is();
                }
                break;
            case ID_NEW_TABLE_DESIGN:
            case ID_NEW_TABLE_DESIGN_AUTO_PILOT:
                aReturn.bEnabled = !isDataSourceReadOnly() && !isConnectionReadOnly();
                break;
            case ID_DIRECT_SQL:
                aReturn.bEnabled = true;
                break;
            case ID_MIGRATE_SCRIPTS:
            {
                // Our document supports embedding scripts into it, if and only if there are no
                // forms/reports with macros/scripts into them. So, we need to enable migration
                // if and only if the database document does *not* support embedding scripts.
                bool bAvailable =
                        !Reference< XEmbeddedScripts >( m_xModel, UNO_QUERY ).is()
                    &&  !Reference< XStorable >( m_xModel, UNO_QUERY_THROW )->isReadonly();
                aReturn.bEnabled = bAvailable;
                if ( !bAvailable )
                    aReturn.bInvisible = true;
            }
            break;
            case SID_APP_NEW_FOLDER:
                aReturn.bEnabled = !isDataSourceReadOnly() && getContainer()->getSelectionCount() <= 1;
                if ( aReturn.bEnabled )
                {
                    const ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = eType == E_REPORT || eType == E_FORM;
                }
                break;
            case SID_FORM_CREATE_REPWIZ_PRE_SEL:
            case SID_REPORT_CREATE_REPWIZ_PRE_SEL:
            case SID_APP_NEW_REPORT_PRE_SEL:
                aReturn.bEnabled = !isDataSourceReadOnly()
                                    && SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::EModule::WRITER)
                                    && getContainer()->isALeafSelected();
                if ( aReturn.bEnabled )
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = eType == E_QUERY || eType == E_TABLE;
                    if ( aReturn.bEnabled && SID_APP_NEW_REPORT_PRE_SEL == _nId )
                    {
                        Reference< XContentEnumerationAccess > xEnumAccess(m_xContext->getServiceManager(), UNO_QUERY);
                        aReturn.bEnabled = xEnumAccess.is();
                        if ( aReturn.bEnabled )
                        {
                            static const char s_sReportDesign[] = "org.libreoffice.report.pentaho.SOReportJobFactory";
                            Reference< XEnumeration > xEnumDrivers = xEnumAccess->createContentEnumeration(s_sReportDesign);
                            aReturn.bEnabled = xEnumDrivers.is() && xEnumDrivers->hasMoreElements();
                        }
                    }
                }
                break;
            case SID_DB_APP_DELETE:
            case SID_DB_APP_RENAME:
                aReturn.bEnabled = isRenameDeleteAllowed(getContainer()->getElementType(), _nId == SID_DB_APP_DELETE);
                break;
            case SID_DB_APP_TABLE_DELETE:
            case SID_DB_APP_TABLE_RENAME:
                aReturn.bEnabled = isRenameDeleteAllowed(E_TABLE, _nId == SID_DB_APP_TABLE_DELETE);
                break;
            case SID_DB_APP_QUERY_DELETE:
            case SID_DB_APP_QUERY_RENAME:
                aReturn.bEnabled = isRenameDeleteAllowed(E_QUERY, _nId == SID_DB_APP_QUERY_DELETE);
                break;
            case SID_DB_APP_FORM_DELETE:
            case SID_DB_APP_FORM_RENAME:
                aReturn.bEnabled = isRenameDeleteAllowed(E_FORM, _nId == SID_DB_APP_FORM_DELETE);
                break;
            case SID_DB_APP_REPORT_DELETE:
            case SID_DB_APP_REPORT_RENAME:
                aReturn.bEnabled = isRenameDeleteAllowed(E_REPORT, _nId == SID_DB_APP_REPORT_DELETE);
                break;

            case SID_SELECTALL:
                aReturn.bEnabled = getContainer()->getElementCount() > 0 && getContainer()->getSelectionCount() != getContainer()->getElementCount();
                break;
            case SID_DB_APP_EDIT:
            case SID_DB_APP_TABLE_EDIT:
            case SID_DB_APP_QUERY_EDIT:
            case SID_DB_APP_FORM_EDIT:
            case SID_DB_APP_REPORT_EDIT:
                aReturn.bEnabled = !isDataSourceReadOnly() && getContainer()->getSelectionCount() > 0
                                    && getContainer()->isALeafSelected();
                break;
            case SID_DB_APP_EDIT_SQL_VIEW:
                if ( isDataSourceReadOnly() )
                    aReturn.bEnabled = false;
                else
                {
                    switch ( getContainer()->getElementType() )
                    {
                    case E_QUERY:
                        aReturn.bEnabled =  ( getContainer()->getSelectionCount() > 0 )
                                        &&  ( getContainer()->isALeafSelected() );
                        break;
                    case E_TABLE:
                        aReturn.bEnabled = false;
                        // there's one exception: views which support altering their underlying
                        // command can be edited in SQL view, too
                        if  (   ( getContainer()->getSelectionCount() > 0 )
                            &&  ( getContainer()->isALeafSelected() )
                            )
                        {
                            ::std::vector< OUString > aSelected;
                            getSelectionElementNames( aSelected );
                            bool bAlterableViews = true;
                            for (   ::std::vector< OUString >::const_iterator selectedName = aSelected.begin();
                                    bAlterableViews && ( selectedName != aSelected.end() ) ;
                                    ++selectedName
                                )
                            {
                                bAlterableViews &= impl_isAlterableView_nothrow( *selectedName );
                            }
                            aReturn.bEnabled = bAlterableViews;
                        }
                        break;
                    default:
                        break;
                    }
                }
                break;
            case SID_DB_APP_OPEN:
            case SID_DB_APP_TABLE_OPEN:
            case SID_DB_APP_QUERY_OPEN:
            case SID_DB_APP_FORM_OPEN:
            case SID_DB_APP_REPORT_OPEN:
                aReturn.bEnabled = getContainer()->getSelectionCount() > 0 && getContainer()->isALeafSelected();
                break;
            case SID_DB_APP_DSUSERADMIN:
                aReturn.bEnabled = !dbaccess::ODsnTypeCollection::isEmbeddedDatabase(::comphelper::getString(m_xDataSource->getPropertyValue(PROPERTY_URL)));
                break;
            case SID_DB_APP_DSRELDESIGN:
                aReturn.bEnabled = true;
                break;
            case SID_DB_APP_TABLEFILTER:
                aReturn.bEnabled = !isDataSourceReadOnly();
                break;
            case SID_DB_APP_REFRESH_TABLES:
                aReturn.bEnabled = getContainer()->getElementType() == E_TABLE && isConnected();
                break;
            case SID_DB_APP_DSPROPS:
                aReturn.bEnabled = m_xDataSource.is() && dbaccess::ODsnTypeCollection::isShowPropertiesEnabled(::comphelper::getString(m_xDataSource->getPropertyValue(PROPERTY_URL)));
                break;
            case SID_DB_APP_DSCONNECTION_TYPE:
                aReturn.bEnabled = !isDataSourceReadOnly() && m_xDataSource.is() && !dbaccess::ODsnTypeCollection::isEmbeddedDatabase(::comphelper::getString(m_xDataSource->getPropertyValue(PROPERTY_URL)));
                break;
            case SID_DB_APP_DSADVANCED_SETTINGS:
                aReturn.bEnabled = m_xDataSource.is() && AdvancedSettingsDialog::doesHaveAnyAdvancedSettings( m_aTypeCollection.getType(::comphelper::getString( m_xDataSource->getPropertyValue( PROPERTY_URL ) )) );
                break;
            case SID_DB_APP_CONVERTTOVIEW:
                aReturn.bEnabled = !isDataSourceReadOnly();
                if ( aReturn.bEnabled )
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = eType == E_QUERY && getContainer()->getSelectionCount() > 0;
                    if ( aReturn.bEnabled )
                    {
                        Reference<XViewsSupplier> xViewSup( getConnection(), UNO_QUERY );
                        aReturn.bEnabled = xViewSup.is() && Reference<XAppend>(xViewSup->getViews(),UNO_QUERY).is();
                    }
                }
                break;
            case SID_DB_APP_DISABLE_PREVIEW:
                aReturn.bEnabled = true;
                aReturn.bChecked = getContainer()->getPreviewMode() == E_PREVIEWNONE;
                break;
            case SID_DB_APP_VIEW_DOCINFO_PREVIEW:
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = (E_REPORT == eType || E_FORM == eType);
                    aReturn.bChecked = getContainer()->getPreviewMode() == E_DOCUMENTINFO;
                }
                break;
            case SID_DB_APP_VIEW_DOC_PREVIEW:
                aReturn.bEnabled = true;
                aReturn.bChecked = getContainer()->getPreviewMode() == E_DOCUMENT;
                break;
            case ID_BROWSER_UNDO:
                aReturn.bEnabled = false;
                break;
            case SID_MAIL_SENDDOC:
                aReturn.bEnabled = true;
                break;
            case SID_DB_APP_SENDREPORTASMAIL:
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = E_REPORT == eType && getContainer()->getSelectionCount() > 0 && getContainer()->isALeafSelected();
                }
                break;
            case SID_DB_APP_SENDREPORTTOWRITER:
            case SID_DB_APP_DBADMIN:
                aReturn.bEnabled = false;
                break;
            case SID_DB_APP_STATUS_TYPE:
                aReturn.bEnabled = m_xDataSource.is();
                if ( aReturn.bEnabled )
                {
                    OUString sURL;
                    m_xDataSource->getPropertyValue(PROPERTY_URL) >>= sURL;
                    OUString sDSTypeName;
                    if ( dbaccess::ODsnTypeCollection::isEmbeddedDatabase( sURL ) )
                    {
                        sDSTypeName = OUString( ModuleRes( RID_STR_EMBEDDED_DATABASE ) );
                    }
                    else
                    {
                        sDSTypeName = m_aTypeCollection.getTypeDisplayName(sURL);
                    }
                    aReturn.sTitle = sDSTypeName;
                }
                break;
            case SID_DB_APP_STATUS_DBNAME:
                aReturn.bEnabled = m_xDataSource.is();
                if ( aReturn.bEnabled )
                {
                    OUString sURL;
                    m_xDataSource->getPropertyValue(PROPERTY_URL) >>= sURL;
                    OUString sDatabaseName;
                    OUString sHostName;
                    sal_Int32 nPortNumber( -1 );

                    m_aTypeCollection.extractHostNamePort( sURL, sDatabaseName, sHostName, nPortNumber );

                    if ( sDatabaseName.isEmpty() )
                        sDatabaseName = m_aTypeCollection.cutPrefix( sURL );
                    if ( m_aTypeCollection.isFileSystemBased(sURL) )
                    {
                        sDatabaseName = SvtPathOptions().SubstituteVariable( sDatabaseName );
                        if ( !sDatabaseName.isEmpty() )
                        {
                            ::svt::OFileNotation aFileNotation(sDatabaseName);
                            // set this decoded URL as text
                            sDatabaseName = aFileNotation.get(::svt::OFileNotation::N_SYSTEM);
                        }
                    }

                    if ( sDatabaseName.isEmpty() )
                        sDatabaseName = m_aTypeCollection.getTypeDisplayName( sURL );

                    aReturn.sTitle = sDatabaseName;
                }
                break;
            case SID_DB_APP_STATUS_USERNAME:
                aReturn.bEnabled = m_xDataSource.is();
                if ( aReturn.bEnabled )
                    m_xDataSource->getPropertyValue( PROPERTY_USER ) >>= aReturn.sTitle;
                break;
            case SID_DB_APP_STATUS_HOSTNAME:
                aReturn.bEnabled = m_xDataSource.is();
                if ( aReturn.bEnabled )
                {
                    OUString sURL;
                    m_xDataSource->getPropertyValue( PROPERTY_URL ) >>= sURL;

                    OUString sHostName, sDatabaseName;
                    sal_Int32 nPortNumber = -1;
                    m_aTypeCollection.extractHostNamePort( sURL, sDatabaseName, sHostName, nPortNumber );
                    aReturn.sTitle = sHostName;
                }
                break;
            default:
                aReturn = OGenericUnoController::GetState(_nId);
        }
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return aReturn;
}

namespace
{
    bool lcl_handleException_nothrow( const Reference< XModel >& _rxDocument, const Any& _rException )
    {
        bool bHandled = false;

        // try handling the error with an interaction handler
        ::comphelper::NamedValueCollection aArgs( _rxDocument->getArgs() );
        Reference< XInteractionHandler > xHandler( aArgs.getOrDefault( "InteractionHandler", Reference< XInteractionHandler >() ) );
        if ( xHandler.is() )
        {
            Reference< ::comphelper::OInteractionRequest > pRequest( new ::comphelper::OInteractionRequest( _rException ) );
            Reference< ::comphelper::OInteractionApprove > pApprove( new ::comphelper::OInteractionApprove );
            pRequest->addContinuation( pApprove.get() );

            try
            {
                xHandler->handle( pRequest.get() );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }

            bHandled = pApprove->wasSelected();
        }
        return bHandled;
    }
}

void OApplicationController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    if ( isUserDefinedFeature( _nId ) )
    {
        OGenericUnoController::Execute( _nId, aArgs );
        return;
    }

    if ( !getContainer() || m_bReadOnly )
        return; // return without execution

    try
    {
        switch(_nId)
        {
            case ID_BROWSER_CUT:
                getContainer()->cut();
                break;
            case ID_BROWSER_COPY:
                {
                    TransferableHelper* pTransfer = copyObject( );
                    Reference< XTransferable> aEnsureDelete = pTransfer;

                    if ( pTransfer )
                        pTransfer->CopyToClipboard(getView());
                }
                break;
            case ID_BROWSER_PASTE:
                {
                    const TransferableDataHelper& rTransferData( getViewClipboard() );
                    ElementType eType = getContainer()->getElementType();

                    switch( eType )
                    {
                        case E_TABLE:
                            {
                                // get the selected tablename
                                ::std::vector< OUString > aList;
                                getSelectionElementNames( aList );
                                if ( !aList.empty() )
                                    m_aTableCopyHelper.SetTableNameForAppend( *aList.begin() );
                                else
                                    m_aTableCopyHelper.ResetTableNameForAppend();

                                m_aTableCopyHelper.pasteTable( rTransferData , getDatabaseName(), ensureConnection() );
                            }
                            break;

                        case E_QUERY:
                            if ( rTransferData.HasFormat(SotClipboardFormatId::DBACCESS_QUERY) )
                                paste( E_QUERY, ODataAccessObjectTransferable::extractObjectDescriptor( rTransferData ) );
                            break;
                        default:
                            {
                                ::std::vector< OUString> aList;
                                getSelectionElementNames(aList);
                                OUString sFolderNameToInsertInto;
                                if ( !aList.empty() )
                                {
                                    Reference< XHierarchicalNameAccess > xContainer(getElements(eType),UNO_QUERY);
                                    if ( xContainer.is()
                                        && xContainer->hasByHierarchicalName(*aList.begin())
                                        && (xContainer->getByHierarchicalName(*aList.begin()) >>= xContainer)
                                        && xContainer.is()
                                        )
                                        sFolderNameToInsertInto = *aList.begin();
                                }
                                paste( eType, OComponentTransferable::extractComponentDescriptor( rTransferData ),
                                    sFolderNameToInsertInto );
                            }
                            break;
                    }
                }
                break;
            case SID_DB_APP_PASTE_SPECIAL:
                {
                    if ( !aArgs.getLength() )
                    {
                        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                        ::std::unique_ptr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog( getView() ));
                        ::std::vector<SotClipboardFormatId> aFormatIds;
                        getSupportedFormats(getContainer()->getElementType(),aFormatIds);
                        const ::std::vector<SotClipboardFormatId>::iterator aEnd = aFormatIds.end();
                        for (::std::vector<SotClipboardFormatId>::iterator aIter = aFormatIds.begin();aIter != aEnd; ++aIter)
                            pDlg->Insert(*aIter,"");

                        const TransferableDataHelper& rClipboard = getViewClipboard();
                        pasteFormat(pDlg->GetFormat(rClipboard.GetTransferable()));
                    }
                    else
                    {
                        const PropertyValue* pIter = aArgs.getConstArray();
                        const PropertyValue* pEnd  = pIter + aArgs.getLength();
                        for( ; pIter != pEnd ; ++pIter)
                        {
                            if ( pIter->Name == "FormatStringId" )
                            {
                                sal_uLong nTmp;
                                if ( pIter->Value >>= nTmp )
                                    pasteFormat(static_cast<SotClipboardFormatId>(nTmp));
                                break;
                            }
                        }
                    }
                }
                break;
            case SID_OPENDOC:
                {
                    Reference < XDispatchProvider > xProv( getFrame(), UNO_QUERY );
                    if ( xProv.is() )
                    {
                        URL aURL;
                        switch(_nId)
                        {
                            case SID_OPENDOC:
                                aURL.Complete = ".uno:Open";
                                break;
                        }

                        if ( m_xUrlTransformer.is() )
                            m_xUrlTransformer->parseStrict( aURL );
                        Reference < XDispatch > xDisp = xProv->queryDispatch( aURL, OUString(), 0 );
                        if ( xDisp.is() )
                            xDisp->dispatch( aURL, Sequence < PropertyValue >() );
                    }
                }
                break;
            case ID_BROWSER_SAVEDOC:
                {
                    Reference< XStorable > xStore( m_xModel, UNO_QUERY_THROW );
                    try
                    {
                        xStore->store();
                    }
                    catch( const Exception& )
                    {
                        lcl_handleException_nothrow( m_xModel, ::cppu::getCaughtException() );
                    }
                }
                break;

            case ID_BROWSER_SAVEASDOC:
                {
                    OUString sUrl;
                    if ( m_xModel.is() )
                        sUrl = m_xModel->getURL();
                    if ( sUrl.isEmpty() )
                        sUrl = SvtPathOptions().GetWorkPath();

                    ::sfx2::FileDialogHelper aFileDlg(
                        ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,
                        0, getView());
                    aFileDlg.SetDisplayDirectory( sUrl );

                    const SfxFilter* pFilter = getStandardDatabaseFilter();
                    if ( pFilter )
                    {
                        aFileDlg.AddFilter(pFilter->GetUIName(),pFilter->GetDefaultExtension());
                        aFileDlg.SetCurrentFilter(pFilter->GetUIName());
                    }

                    if ( aFileDlg.Execute() != ERRCODE_NONE )
                        break;

                    Reference<XStorable> xStore( m_xModel, UNO_QUERY_THROW );
                    INetURLObject aURL( aFileDlg.GetPath() );
                    try
                    {
                        xStore->storeAsURL( aURL.GetMainURL( INetURLObject::NO_DECODE ), Sequence< PropertyValue >() );
                    }
                    catch( const Exception& )
                    {
                        lcl_handleException_nothrow( m_xModel, ::cppu::getCaughtException() );
                    }

                    /*updateTitle();*/
                    m_bCurrentlyModified = false;
                    InvalidateFeature(ID_BROWSER_SAVEDOC);
                    if ( getContainer()->getElementType() == E_NONE )
                    {
                        getContainer()->selectContainer(E_NONE);
                        getContainer()->selectContainer(E_TABLE);
                        // #i95524#
                        getContainer()->Invalidate();
                        refreshTables();
                    }

                }
                break;
            case ID_BROWSER_SORTUP:
                getContainer()->sortUp();
                InvalidateFeature(ID_BROWSER_SORTDOWN);
                break;
            case ID_BROWSER_SORTDOWN:
                getContainer()->sortDown();
                InvalidateFeature(ID_BROWSER_SORTUP);
                break;

            case ID_NEW_TABLE_DESIGN_AUTO_PILOT:
            case ID_NEW_VIEW_DESIGN_AUTO_PILOT:
            case ID_APP_NEW_QUERY_AUTO_PILOT:
            case SID_DB_FORM_NEW_PILOT:
            case SID_REPORT_CREATE_REPWIZ_PRE_SEL:
            case SID_APP_NEW_REPORT_PRE_SEL:
            case SID_FORM_CREATE_REPWIZ_PRE_SEL:
            case ID_DOCUMENT_CREATE_REPWIZ:
            case SID_APP_NEW_FORM:
            case SID_APP_NEW_REPORT:
            case ID_NEW_QUERY_SQL:
            case ID_NEW_QUERY_DESIGN:
            case ID_NEW_TABLE_DESIGN:
                {
                    ElementType eType = E_TABLE;
                    bool bAutoPilot = false;
                    ::comphelper::NamedValueCollection aCreationArgs;

                    switch( _nId )
                    {
                        case SID_DB_FORM_NEW_PILOT:
                        case SID_FORM_CREATE_REPWIZ_PRE_SEL:
                            bAutoPilot = true;
                            // run through
                        case SID_APP_NEW_FORM:
                            eType = E_FORM;
                            break;
                        case ID_DOCUMENT_CREATE_REPWIZ:
                        case SID_REPORT_CREATE_REPWIZ_PRE_SEL:
                            bAutoPilot = true;
                            // run through
                        case SID_APP_NEW_REPORT:
                        case SID_APP_NEW_REPORT_PRE_SEL:
                            eType = E_REPORT;
                            break;
                        case ID_APP_NEW_QUERY_AUTO_PILOT:
                            bAutoPilot = true;
                            eType = E_QUERY;
                            break;
                        case ID_NEW_QUERY_DESIGN:
                            aCreationArgs.put( OUString(PROPERTY_GRAPHICAL_DESIGN), sal_True );
                            // run through
                        case ID_NEW_QUERY_SQL:
                            eType = E_QUERY;
                            break;
                         case ID_NEW_TABLE_DESIGN_AUTO_PILOT:
                             bAutoPilot = true;
                             // run through
                        case ID_NEW_TABLE_DESIGN:
                            break;
                        default:
                            OSL_FAIL("illegal switch call!");
                    }
                    if ( bAutoPilot )
                        getContainer()->PostUserEvent( LINK( this, OApplicationController, OnCreateWithPilot ), reinterpret_cast< void* >( eType ) );
                    else
                    {
                        Reference< XComponent > xDocDefinition;
                        newElement( eType, aCreationArgs, xDocDefinition );
                    }
                }
                break;
            case SID_APP_NEW_FOLDER:
                {
                    ElementType eType = getContainer()->getElementType();
                    OUString sName = getContainer()->getQualifiedName( nullptr );
                    insertHierachyElement(eType,sName);
                }
                break;
            case ID_NEW_VIEW_DESIGN:
            case SID_DB_NEW_VIEW_SQL:
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                    {
                        QueryDesigner aDesigner( getORB(), this, getFrame(), true );

                        ::comphelper::NamedValueCollection aCreationArgs;
                        aCreationArgs.put( OUString(PROPERTY_GRAPHICAL_DESIGN), ID_NEW_VIEW_DESIGN == _nId );

                        const Reference< XDataSource > xDataSource( m_xDataSource, UNO_QUERY );
                        const Reference< XComponent > xComponent( aDesigner.createNew( xDataSource, aCreationArgs ), UNO_QUERY );
                        onDocumentOpened( OUString(), E_QUERY, E_OPEN_DESIGN, xComponent, nullptr );
                    }
                }
                break;
            case SID_DB_APP_DELETE:
            case SID_DB_APP_TABLE_DELETE:
            case SID_DB_APP_QUERY_DELETE:
            case SID_DB_APP_FORM_DELETE:
            case SID_DB_APP_REPORT_DELETE:
                deleteEntries();
                break;
            case SID_DB_APP_RENAME:
            case SID_DB_APP_TABLE_RENAME:
            case SID_DB_APP_QUERY_RENAME:
            case SID_DB_APP_FORM_RENAME:
            case SID_DB_APP_REPORT_RENAME:
                renameEntry();
                break;
            case SID_DB_APP_EDIT:
            case SID_DB_APP_EDIT_SQL_VIEW:
            case SID_DB_APP_TABLE_EDIT:
            case SID_DB_APP_QUERY_EDIT:
            case SID_DB_APP_FORM_EDIT:
            case SID_DB_APP_REPORT_EDIT:
                doAction( _nId, E_OPEN_DESIGN );
                break;
            case SID_DB_APP_OPEN:
            case SID_DB_APP_TABLE_OPEN:
            case SID_DB_APP_QUERY_OPEN:
            case SID_DB_APP_FORM_OPEN:
            case SID_DB_APP_REPORT_OPEN:
                doAction( _nId, E_OPEN_NORMAL );
                break;
            case SID_DB_APP_CONVERTTOVIEW:
                doAction( _nId, E_OPEN_NORMAL );
                break;
            case SID_SELECTALL:
                getContainer()->selectAll();
                InvalidateAll();
                break;
            case SID_DB_APP_DSRELDESIGN:
            {
                Reference< XComponent > xRelationDesigner;
                if ( !m_pSubComponentManager->activateSubFrame( OUString(), SID_DB_APP_DSRELDESIGN, E_OPEN_DESIGN, xRelationDesigner ) )
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                    {
                        RelationDesigner aDesigner( getORB(), this, m_aCurrentFrame.getFrame() );

                        const Reference< XDataSource > xDataSource( m_xDataSource, UNO_QUERY );
                        const Reference< XComponent > xComponent( aDesigner.createNew( xDataSource ), UNO_QUERY );
                        onDocumentOpened( OUString(), SID_DB_APP_DSRELDESIGN, E_OPEN_DESIGN, xComponent, nullptr );
                    }
                }
            }
            break;
            case SID_DB_APP_DSUSERADMIN:
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                        openDialog("com.sun.star.sdb.UserAdministrationDialog");
                }
                break;
            case SID_DB_APP_TABLEFILTER:
                openTableFilterDialog();
                askToReconnect();
                break;
            case SID_DB_APP_REFRESH_TABLES:
                refreshTables();
                break;
            case SID_DB_APP_DSPROPS:
                openDataSourceAdminDialog();
                askToReconnect();
                break;
            case SID_DB_APP_DSADVANCED_SETTINGS:
                openDialog("com.sun.star.sdb.AdvancedDatabaseSettingsDialog");
                askToReconnect();
                break;
            case SID_DB_APP_DSCONNECTION_TYPE:
                openDialog("com.sun.star.sdb.DataSourceTypeChangeDialog");
                askToReconnect();
                break;
            case ID_DIRECT_SQL:
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                        openDirectSQLDialog();
                }
                break;
            case ID_MIGRATE_SCRIPTS:
                impl_migrateScripts_nothrow();
                break;
            case SID_DB_APP_VIEW_TABLES:
                m_aSelectContainerEvent.Call( reinterpret_cast< void* >( E_TABLE ) );
                break;
            case SID_DB_APP_VIEW_QUERIES:
                m_aSelectContainerEvent.Call( reinterpret_cast< void* >( E_QUERY ) );
                break;
            case SID_DB_APP_VIEW_FORMS:
                m_aSelectContainerEvent.Call( reinterpret_cast< void* >( E_FORM ) );
                break;
            case SID_DB_APP_VIEW_REPORTS:
                m_aSelectContainerEvent.Call( reinterpret_cast< void* >( E_REPORT ) );
                break;
            case SID_DB_APP_DISABLE_PREVIEW:
                m_ePreviewMode = E_PREVIEWNONE;
                getContainer()->switchPreview(m_ePreviewMode);
                break;
            case SID_DB_APP_VIEW_DOCINFO_PREVIEW:
                m_ePreviewMode = E_DOCUMENTINFO;
                getContainer()->switchPreview(m_ePreviewMode);
                break;
            case SID_DB_APP_VIEW_DOC_PREVIEW:
                m_ePreviewMode = E_DOCUMENT;
                getContainer()->switchPreview(m_ePreviewMode);
                break;
            case SID_MAIL_SENDDOC:
                {
                    SfxMailModel aSendMail;
                    if ( aSendMail.AttachDocument(OUString(),getModel(), OUString()) == SfxMailModel::SEND_MAIL_OK )
                        aSendMail.Send( getFrame() );
                }
                break;
            case SID_DB_APP_SENDREPORTASMAIL:
                doAction( _nId, E_OPEN_FOR_MAIL );
                break;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    InvalidateFeature(_nId);
}

void OApplicationController::describeSupportedFeatures()
{
    OGenericUnoController::describeSupportedFeatures();

    implDescribeSupportedFeature( ".uno:Save",               ID_BROWSER_SAVEDOC,        CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:SaveAs",             ID_BROWSER_SAVEASDOC,      CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:SendMail",           SID_MAIL_SENDDOC,          CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:DBSendReportAsMail",SID_DB_APP_SENDREPORTASMAIL,
                                                                                        CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:DBSendReportToWriter",SID_DB_APP_SENDREPORTTOWRITER,
                                                                                        CommandGroup::DOCUMENT );
    implDescribeSupportedFeature( ".uno:DBNewForm",          SID_APP_NEW_FORM,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewFolder",        SID_APP_NEW_FOLDER,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewFormAutoPilot", SID_DB_FORM_NEW_PILOT,     CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewFormAutoPilotWithPreSelection",
                                                             SID_FORM_CREATE_REPWIZ_PRE_SEL,
                                                                                        CommandGroup::APPLICATION );

    implDescribeSupportedFeature( ".uno:DBNewReport",        SID_APP_NEW_REPORT,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewReportAutoPilot",
                                                             ID_DOCUMENT_CREATE_REPWIZ, CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewReportAutoPilotWithPreSelection",
                                                             SID_REPORT_CREATE_REPWIZ_PRE_SEL,
                                                                                        CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:DBNewQuery",         ID_NEW_QUERY_DESIGN,       CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewQuerySql",      ID_NEW_QUERY_SQL,          CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewQueryAutoPilot",ID_APP_NEW_QUERY_AUTO_PILOT,
                                                                                        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewTable",         ID_NEW_TABLE_DESIGN,       CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewTableAutoPilot",ID_NEW_TABLE_DESIGN_AUTO_PILOT,
                                                                                        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewView",          ID_NEW_VIEW_DESIGN,        CommandGroup::INSERT );
    implDescribeSupportedFeature( ".uno:DBNewViewSQL",       SID_DB_NEW_VIEW_SQL,       CommandGroup::INSERT );

    implDescribeSupportedFeature( ".uno:DBDelete",           SID_DB_APP_DELETE,         CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Delete",             SID_DB_APP_DELETE,         CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBRename",           SID_DB_APP_RENAME,         CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBEdit",             SID_DB_APP_EDIT,           CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBEditSqlView",      SID_DB_APP_EDIT_SQL_VIEW,  CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBOpen",             SID_DB_APP_OPEN,           CommandGroup::EDIT );

    implDescribeSupportedFeature( ".uno:DBTableDelete",      SID_DB_APP_TABLE_DELETE,   CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBTableRename",      SID_DB_APP_TABLE_RENAME,   CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBTableEdit",        SID_DB_APP_TABLE_EDIT,     CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBTableOpen",        SID_DB_APP_TABLE_OPEN,     CommandGroup::EDIT );

    implDescribeSupportedFeature( ".uno:DBQueryDelete",      SID_DB_APP_QUERY_DELETE,   CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBQueryRename",      SID_DB_APP_QUERY_RENAME,   CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBQueryEdit",        SID_DB_APP_QUERY_EDIT,     CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBQueryOpen",        SID_DB_APP_QUERY_OPEN,     CommandGroup::EDIT );

    implDescribeSupportedFeature( ".uno:DBFormDelete",       SID_DB_APP_FORM_DELETE,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBFormRename",       SID_DB_APP_FORM_RENAME,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBFormEdit",         SID_DB_APP_FORM_EDIT,      CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBFormOpen",         SID_DB_APP_FORM_OPEN,      CommandGroup::EDIT );

    implDescribeSupportedFeature( ".uno:DBReportDelete",     SID_DB_APP_REPORT_DELETE,  CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBReportRename",     SID_DB_APP_REPORT_RENAME,  CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBReportEdit",       SID_DB_APP_REPORT_EDIT,    CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBReportOpen",       SID_DB_APP_REPORT_OPEN,    CommandGroup::EDIT );

    implDescribeSupportedFeature( ".uno:SelectAll",          SID_SELECTALL,             CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Undo",               ID_BROWSER_UNDO,           CommandGroup::EDIT );

    implDescribeSupportedFeature( ".uno:Sortup",             ID_BROWSER_SORTUP,         CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:SortDown",           ID_BROWSER_SORTDOWN,       CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBRelationDesign",   SID_DB_APP_DSRELDESIGN,    CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:DBUserAdmin",        SID_DB_APP_DSUSERADMIN,    CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:DBTableFilter",      SID_DB_APP_TABLEFILTER,    CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:DBDSProperties",     SID_DB_APP_DSPROPS,        CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBDSConnectionType", SID_DB_APP_DSCONNECTION_TYPE,
                                                                                        CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBDSAdvancedSettings",
                                                             SID_DB_APP_DSADVANCED_SETTINGS,
                                                                                        CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:PasteSpecial",       SID_DB_APP_PASTE_SPECIAL,  CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBConvertToView",    SID_DB_APP_CONVERTTOVIEW,  CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:DBRefreshTables",    SID_DB_APP_REFRESH_TABLES, CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:DBDirectSQL",        ID_DIRECT_SQL,             CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:DBMigrateScripts",   ID_MIGRATE_SCRIPTS,        CommandGroup::APPLICATION );
    implDescribeSupportedFeature( ".uno:DBViewTables",       SID_DB_APP_VIEW_TABLES,    CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBViewQueries",      SID_DB_APP_VIEW_QUERIES,   CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBViewForms",        SID_DB_APP_VIEW_FORMS,     CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBViewReports",      SID_DB_APP_VIEW_REPORTS,   CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBDisablePreview",   SID_DB_APP_DISABLE_PREVIEW,CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBShowDocInfoPreview",
                                                             SID_DB_APP_VIEW_DOCINFO_PREVIEW,
                                                                                        CommandGroup::VIEW );
    implDescribeSupportedFeature( ".uno:DBShowDocPreview",   SID_DB_APP_VIEW_DOC_PREVIEW,
                                                                                        CommandGroup::VIEW );

    implDescribeSupportedFeature( ".uno:OpenUrl",            SID_OPENURL,               CommandGroup::APPLICATION );

    // this one should not appear under Tools->Customize->Keyboard
    implDescribeSupportedFeature( ".uno:DBNewReportWithPreSelection",
                                                             SID_APP_NEW_REPORT_PRE_SEL );
    implDescribeSupportedFeature( ".uno:DBDSImport",         SID_DB_APP_DSIMPORT);
    implDescribeSupportedFeature( ".uno:DBDSExport",         SID_DB_APP_DSEXPORT);
    implDescribeSupportedFeature( ".uno:DBDBAdmin",          SID_DB_APP_DBADMIN);

    // status info
    implDescribeSupportedFeature( ".uno:DBStatusType",       SID_DB_APP_STATUS_TYPE);
    implDescribeSupportedFeature( ".uno:DBStatusDBName",     SID_DB_APP_STATUS_DBNAME);
    implDescribeSupportedFeature( ".uno:DBStatusUserName",   SID_DB_APP_STATUS_USERNAME);
    implDescribeSupportedFeature( ".uno:DBStatusHostName",   SID_DB_APP_STATUS_HOSTNAME);
}

OApplicationView*   OApplicationController::getContainer() const
{
    return static_cast< OApplicationView* >( getView() );
}

// css::container::XContainerListener
void SAL_CALL OApplicationController::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    Reference< XContainer > xContainer(_rEvent.Source, UNO_QUERY);
    if ( ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xContainer) != m_aCurrentContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        if ( getContainer() )
        {
            OUString sName;
            _rEvent.Accessor >>= sName;
            ElementType eType = getElementType(xContainer);

            switch( eType )
            {
                case E_TABLE:
                    ensureConnection();
                    break;
                case E_FORM:
                case E_REPORT:
                    {
                        Reference< XContainer > xSubContainer(_rEvent.Element,UNO_QUERY);
                        if ( xSubContainer.is() )
                            containerFound(xSubContainer);
                    }
                    break;
                default:
                    break;
            }
            getContainer()->elementAdded(eType,sName,_rEvent.Element);
        }
    }
}

void SAL_CALL OApplicationController::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    Reference< XContainer > xContainer(_rEvent.Source, UNO_QUERY);
    if ( ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xContainer) != m_aCurrentContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        OUString sName;
        _rEvent.Accessor >>= sName;
        ElementType eType = getElementType(xContainer);
        switch( eType )
        {
            case E_TABLE:
                ensureConnection();
                break;
            case E_FORM:
            case E_REPORT:
                {
                    Reference<XContent> xContent(xContainer,UNO_QUERY);
                    if ( xContent.is() )
                    {
                        sName = xContent->getIdentifier()->getContentIdentifier() + "/" + sName;
                    }
                }
                break;
            default:
                break;
        }
        getContainer()->elementRemoved(eType,sName);
    }
}

void SAL_CALL OApplicationController::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    Reference< XContainer > xContainer(_rEvent.Source, UNO_QUERY);
    if ( ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xContainer) != m_aCurrentContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        OUString sName;
        try
        {
            _rEvent.Accessor >>= sName;
            Reference<XConnection> xConnection;
            Reference<XPropertySet> xProp(_rEvent.Element,UNO_QUERY);
            OUString sNewName;

            ElementType eType = getElementType(xContainer);
            switch( eType )
            {
                case E_TABLE:
                {
                    ensureConnection();
                    if ( xProp.is() && m_xMetaData.is() )
                        sNewName = ::dbaui::composeTableName( m_xMetaData, xProp, ::dbtools::eInDataManipulation, false, false, false );
                }
                break;
                case E_FORM:
                case E_REPORT:
                    {
                        Reference<XContent> xContent(xContainer,UNO_QUERY);
                        if ( xContent.is() )
                        {
                            sName = xContent->getIdentifier()->getContentIdentifier() + "/" + sName;
                        }
                    }
                    break;
                default:
                    break;
            }
            //  getContainer()->elementReplaced(getContainer()->getElementType(),sName,sNewName);
        }
        catch( Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

namespace
{
    OUString lcl_getToolBarResource(ElementType _eType)
    {
        OUString sToolbar;
        switch(_eType)
        {
            case E_TABLE:
                sToolbar = "private:resource/toolbar/tableobjectbar";
                break;
            case E_QUERY:
                sToolbar = "private:resource/toolbar/queryobjectbar";
                break;
            case E_FORM:
                sToolbar = "private:resource/toolbar/formobjectbar";
                break;
            case E_REPORT:
                sToolbar = "private:resource/toolbar/reportobjectbar";
                break;
            case E_NONE:
                break;
            default:
                OSL_FAIL("Invalid ElementType!");
                break;
        }
        return sToolbar;
    }
}

bool OApplicationController::onContainerSelect(ElementType _eType)
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");

    if ( m_eCurrentType != _eType && _eType != E_NONE )
    {
        SelectionGuard aSelGuard( *m_pSelectionNotifier );

        if ( _eType == E_TABLE )
        {
            try
            {
                SharedConnection xConnection( ensureConnection() );
                if ( xConnection.is() && getContainer()->getDetailView() )
                {
                    getContainer()->getDetailView()->createTablesPage(xConnection);
                    Reference<XTablesSupplier> xTabSup(xConnection,UNO_QUERY);
                    if ( xTabSup.is() )
                        addContainerListener(xTabSup->getTables());
                }
                else
                {
                    return false;
                }
            }
            catch( const Exception& )
            {
                return false;
            }
        }
        Reference< XLayoutManager > xLayoutManager = getLayoutManager( getFrame() );
        if ( xLayoutManager.is() )
        {
            OUString sToolbar = lcl_getToolBarResource(_eType);
            OUString sDestroyToolbar = lcl_getToolBarResource(m_eCurrentType);

            xLayoutManager->lock();
            xLayoutManager->destroyElement( sDestroyToolbar );
            if ( !sToolbar.isEmpty() )
            {
                xLayoutManager->createElement( sToolbar );
                xLayoutManager->requestElement( sToolbar );
            }
            xLayoutManager->unlock();
            xLayoutManager->doLayout();
        }

        if ( _eType != E_TABLE && getContainer()->getDetailView() )
        {
            Reference< XNameAccess > xContainer = getElements(_eType);
            addContainerListener(xContainer);
            getContainer()->getDetailView()->createPage(_eType,xContainer);
        }

        SelectionByElementType::iterator pendingSelection = m_aPendingSelection.find( _eType );
        if ( pendingSelection != m_aPendingSelection.end() )
        {
            getContainer()->selectElements( comphelper::containerToSequence(pendingSelection->second) );

            m_aPendingSelection.erase( pendingSelection );
        }

        InvalidateAll();
    }
    m_eCurrentType = _eType;

    return true;
}

bool OApplicationController::onEntryDoubleClick( SvTreeListBox& _rTree )
{
    if ( getContainer() && getContainer()->isLeaf( _rTree.GetHdlEntry() ) )
    {
        try
        {
            openElement(
                getContainer()->getQualifiedName( _rTree.GetHdlEntry() ),
                getContainer()->getElementType(),
                E_OPEN_NORMAL
            );
            return true;    // handled
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return false;   // not handled
}

bool OApplicationController::impl_isAlterableView_nothrow( const OUString& _rTableOrViewName ) const
{
    OSL_PRECOND( m_xDataSourceConnection.is(), "OApplicationController::impl_isAlterableView_nothrow: no connection!" );

    bool bIsAlterableView( false );
    try
    {
        Reference< XViewsSupplier > xViewsSupp( m_xDataSourceConnection, UNO_QUERY );
        Reference< XNameAccess > xViews;
        if ( xViewsSupp.is() )
            xViews = xViewsSupp->getViews();

        Reference< XAlterView > xAsAlterableView;
        if ( xViews.is() && xViews->hasByName( _rTableOrViewName ) )
            xAsAlterableView.set( xViews->getByName( _rTableOrViewName ), UNO_QUERY );

        bIsAlterableView = xAsAlterableView.is();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return bIsAlterableView;
}

Reference< XComponent > OApplicationController::openElement(const OUString& _sName, ElementType _eType,
    ElementOpenMode _eOpenMode, sal_uInt16 _nInstigatorCommand )
{
    return openElementWithArguments( _sName, _eType, _eOpenMode, _nInstigatorCommand, ::comphelper::NamedValueCollection() );
}

Reference< XComponent > OApplicationController::openElementWithArguments( const OUString& _sName, ElementType _eType,
    ElementOpenMode _eOpenMode, sal_uInt16 _nInstigatorCommand, const ::comphelper::NamedValueCollection& _rAdditionalArguments )
{
    OSL_PRECOND( getContainer(), "OApplicationController::openElementWithArguments: no view!" );
    if ( !getContainer() )
        return nullptr;

    Reference< XComponent > xRet;
    if ( _eOpenMode == E_OPEN_DESIGN )
    {
        // OJ: http://www.openoffice.org/issues/show_bug.cgi?id=30382
        getContainer()->showPreview(nullptr);
    }

    bool isStandaloneDocument = false;
    switch ( _eType )
    {
    case E_REPORT:
        if ( _eOpenMode != E_OPEN_DESIGN )
        {
            // reports which are opened in a mode other than design are no sub components of our application
            // component, but standalone documents.
            isStandaloneDocument = true;
        }
        // NO break!
    case E_FORM:
    {
        if ( isStandaloneDocument || !m_pSubComponentManager->activateSubFrame( _sName, _eType, _eOpenMode, xRet ) )
        {
            ::std::unique_ptr< OLinkedDocumentsAccess > aHelper = getDocumentsAccess( _eType );
            if ( !aHelper->isConnected() )
                break;

            Reference< XComponent > xDefinition;
            xRet = aHelper->open( _sName, xDefinition, _eOpenMode, _rAdditionalArguments );

            if ( !isStandaloneDocument )
                onDocumentOpened( _sName, _eType, _eOpenMode, xRet, xDefinition );
        }
    }
    break;

    case E_QUERY:
    case E_TABLE:
    {
        if ( !m_pSubComponentManager->activateSubFrame( _sName, _eType, _eOpenMode, xRet ) )
        {
            SharedConnection xConnection( ensureConnection() );
            if ( !xConnection.is() )
                break;

            ::std::unique_ptr< DatabaseObjectView > pDesigner;
            ::comphelper::NamedValueCollection aArguments( _rAdditionalArguments );

            Any aDataSource;
            if ( _eOpenMode == E_OPEN_DESIGN )
            {
                bool bAddViewTypeArg = false;

                if ( _eType == E_TABLE )
                {
                    if ( impl_isAlterableView_nothrow( _sName ) )
                    {
                        pDesigner.reset( new QueryDesigner( getORB(), this, m_aCurrentFrame.getFrame(), true ) );
                        bAddViewTypeArg = true;
                    }
                    else
                    {
                        pDesigner.reset( new TableDesigner( getORB(), this, m_aCurrentFrame.getFrame() ) );
                    }
                }
                else if ( _eType == E_QUERY )
                {
                    pDesigner.reset( new QueryDesigner( getORB(), this, m_aCurrentFrame.getFrame(), false ) );
                    bAddViewTypeArg = true;
                }
                aDataSource <<= m_xDataSource;

                if ( bAddViewTypeArg )
                {
                    const bool bQueryGraphicalMode =( _nInstigatorCommand != SID_DB_APP_EDIT_SQL_VIEW );
                    aArguments.put( OUString(PROPERTY_GRAPHICAL_DESIGN), bQueryGraphicalMode );
                }

            }
            else
            {
                pDesigner.reset( new ResultSetBrowser( getORB(), this, m_aCurrentFrame.getFrame(), _eType == E_TABLE ) );

                if ( !aArguments.has( OUString(PROPERTY_SHOWMENU) ) )
                    aArguments.put( OUString(PROPERTY_SHOWMENU), makeAny( true ) );

                aDataSource <<= getDatabaseName();
            }

            xRet.set( pDesigner->openExisting( aDataSource, _sName, aArguments ) );
            onDocumentOpened( _sName, _eType, _eOpenMode, xRet, nullptr );
        }
    }
    break;

    default:
        OSL_FAIL( "OApplicationController::openElement: illegal object type!" );
        break;
    }
    return xRet;
}

IMPL_LINK_TYPED( OApplicationController, OnSelectContainer, void*, _pType, void )
{
    ElementType eType = (ElementType)reinterpret_cast< sal_IntPtr >( _pType );
    if (getContainer())
        getContainer()->selectContainer(eType);
}

IMPL_LINK_TYPED( OApplicationController, OnCreateWithPilot, void*, _pType, void )
{
    ElementType eType = (ElementType)reinterpret_cast< sal_IntPtr >( _pType );
    newElementWithPilot( eType );
}

void OApplicationController::newElementWithPilot( ElementType _eType )
{
    utl::CloseVeto aKeepDoc( getFrame() );
        // prevent the document being closed while the wizard is open

    OSL_ENSURE( getContainer(), "OApplicationController::newElementWithPilot: without a view?" );

    switch ( _eType )
    {
        case E_REPORT:
        case E_FORM:
        {
            ::std::unique_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
            if ( aHelper->isConnected() )
            {
                sal_Int32 nCommandType = -1;
                const OUString sCurrentSelected( getCurrentlySelectedName( nCommandType ) );
                if ( E_REPORT == _eType )
                    aHelper->newReportWithPilot( nCommandType, sCurrentSelected );
                else
                    aHelper->newFormWithPilot( nCommandType, sCurrentSelected );
            }
        }
        break;
        case E_QUERY:
        case E_TABLE:
         {
            ::std::unique_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
            if ( aHelper->isConnected() )
            {
                if ( E_QUERY == _eType )
                    aHelper->newQueryWithPilot();
                else
                    aHelper->newTableWithPilot();
            }
         }
         break;
        case E_NONE:
            break;
    }

    // no need for onDocumentOpened, the table wizard opens the created table by using
    // XDatabaseDocumentUI::loadComponent method.
}

Reference< XComponent > OApplicationController::newElement( ElementType _eType, const ::comphelper::NamedValueCollection& i_rAdditionalArguments,
                                                           Reference< XComponent >& o_rDocumentDefinition )
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");

    Reference< XComponent > xComponent;
    o_rDocumentDefinition.clear();

    switch ( _eType )
    {
        case E_FORM:
        case E_REPORT:
        {
            ::std::unique_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess( _eType );
            if ( !aHelper->isConnected() )
                break;

            xComponent = aHelper->newDocument( _eType == E_FORM ? ID_FORM_NEW_TEXT : ID_REPORT_NEW_TEXT, i_rAdditionalArguments, o_rDocumentDefinition );
        }
        break;

        case E_QUERY:
        case E_TABLE:
        {
            ::std::unique_ptr< DatabaseObjectView > pDesigner;
            SharedConnection xConnection( ensureConnection() );
            if ( !xConnection.is() )
                break;

            if ( _eType == E_TABLE )
            {
                pDesigner.reset( new TableDesigner( getORB(), this, getFrame() ) );
            }
            else if ( _eType == E_QUERY )
            {
                pDesigner.reset( new QueryDesigner( getORB(), this, getFrame(), false ) );
            }

            Reference< XDataSource > xDataSource( m_xDataSource, UNO_QUERY );
            xComponent.set( pDesigner->createNew( xDataSource, i_rAdditionalArguments ), UNO_QUERY );
        }
        break;

        default:
            OSL_FAIL( "OApplicationController::newElement: illegal type!" );
            break;
    }

    if ( xComponent.is() )
        onDocumentOpened( OUString(), _eType, E_OPEN_DESIGN, xComponent, o_rDocumentDefinition );

    return xComponent;
}

void OApplicationController::addContainerListener(const Reference<XNameAccess>& _xCollection)
{
    try
    {
        Reference< XContainer > xCont(_xCollection, UNO_QUERY);
        if ( xCont.is() )
        {
            // add as listener to get notified if elements are inserted or removed
            TContainerVector::iterator aFind = ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xCont);
            if ( aFind == m_aCurrentContainers.end() )
            {
                xCont->addContainerListener(this);
                m_aCurrentContainers.push_back(xCont);
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OApplicationController::renameEntry()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
    ::std::vector< OUString> aList;
    getSelectionElementNames(aList);

    Reference< XNameAccess > xContainer = getElements(getContainer()->getElementType());
    OSL_ENSURE(aList.size() == 1,"Invalid rename call here. More than one element!");
    if ( aList.empty() )
        return;

    try
    {
        if ( xContainer.is() )
        {
            ::std::unique_ptr< IObjectNameCheck > pNameChecker;
            VclPtr< OSaveAsDlg > aDialog;

            Reference<XRename> xRename;
            const ElementType eType = getContainer()->getElementType();
            switch( eType )
            {
                case E_FORM:
                case E_REPORT:
                    {
                        Reference<XHierarchicalNameContainer> xHNames(xContainer, UNO_QUERY);
                        if ( xHNames.is() )
                        {
                            OUString sLabel;
                            if ( eType == E_FORM )
                                sLabel = OUString(ModuleRes( STR_FRM_LABEL ));
                            else
                                sLabel = OUString(ModuleRes( STR_RPT_LABEL ));

                            OUString sName = *aList.begin();
                            if ( xHNames->hasByHierarchicalName(sName) )
                            {
                                xRename.set(xHNames->getByHierarchicalName(sName),UNO_QUERY);
                                Reference<XChild> xChild(xRename,UNO_QUERY);
                                if ( xChild.is() )
                                {
                                    Reference<XHierarchicalNameContainer> xParent(xChild->getParent(),UNO_QUERY);
                                    if ( xParent.is() )
                                    {
                                        xHNames = xParent;
                                        Reference<XPropertySet>(xRename,UNO_QUERY)->getPropertyValue(PROPERTY_NAME) >>= sName;
                                    }
                                }
                                pNameChecker.reset( new HierarchicalNameCheck( xHNames.get(), OUString() ) );
                                aDialog.reset( VclPtr<OSaveAsDlg>::Create(

                                    getView(), getORB(), sName, sLabel, *pNameChecker, SAD_TITLE_RENAME ) );
                            }
                        }
                    }
                    break;
                case E_TABLE:
                    ensureConnection();
                    if ( !getConnection().is() )
                        break;
                    // NO break
                case E_QUERY:
                    if ( xContainer->hasByName(*aList.begin()) )
                    {
                        xRename.set(xContainer->getByName(*aList.begin()),UNO_QUERY);
                        sal_Int32 nCommandType = eType == E_QUERY ? CommandType::QUERY : CommandType::TABLE;

                        ensureConnection();
                        pNameChecker.reset( new DynamicTableOrQueryNameCheck( getConnection(), nCommandType ) );
                        aDialog.reset( VclPtr<OSaveAsDlg>::Create(

                            getView(), nCommandType, getORB(), getConnection(),
                                *aList.begin(), *pNameChecker, SAD_TITLE_RENAME ) );
                    }
                    break;
                default:
                    break;
            }

            if ( xRename.is() && aDialog.get() )
            {

                bool bTryAgain = true;
                while( bTryAgain )
                {
                    if ( aDialog->Execute() == RET_OK )
                    {
                        try
                        {
                            OUString sNewName;
                            if ( eType == E_TABLE )
                            {
                                OUString sName = aDialog->getName();
                                OUString sCatalog = aDialog->getCatalog();
                                OUString sSchema  = aDialog->getSchema();

                                sNewName = ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sName, false, ::dbtools::eInDataManipulation );
                            }
                            else
                                sNewName = aDialog->getName();

                            OUString sOldName = *aList.begin();
                            if ( eType == E_FORM || eType == E_REPORT )
                            {
                                Reference<XContent> xContent(xRename,UNO_QUERY);
                                if ( xContent.is() )
                                {
                                    sOldName = xContent->getIdentifier()->getContentIdentifier();
                                }
                            }

                            xRename->rename(sNewName);

                            if ( eType == E_TABLE )
                            {
                                Reference<XPropertySet> xProp(xRename,UNO_QUERY);
                                sNewName = ::dbaui::composeTableName( m_xMetaData, xProp, ::dbtools::eInDataManipulation, false, false, false );
                            }
                            getContainer()->elementReplaced( eType , sOldName, sNewName );

                            bTryAgain = false;
                        }
                        catch(const SQLException& )
                        {
                            showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );

                        }
                        catch(const ElementExistException& e)
                        {
                            OUString sStatus("S1000");
                            OUString sMsg = OUString( ModuleRes( STR_NAME_ALREADY_EXISTS ) );
                            showError(SQLExceptionInfo(SQLException(sMsg.replaceAll("#", e.Message), e.Context, sStatus, 0, Any())));
                        }
                        catch(const Exception& )
                        {
                            DBG_UNHANDLED_EXCEPTION();
                        }
                    }
                    else
                        bTryAgain = false;
                }
            }
        }
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void OApplicationController::onSelectionChanged()
{
    InvalidateAll();

    SelectionGuard aSelGuard( *m_pSelectionNotifier );

    OApplicationView* pView = getContainer();
    if ( !pView )
        return;

    if ( pView->getSelectionCount() == 1 )
    {
        const ElementType eType = pView->getElementType();
          if ( pView->isALeafSelected() )
        {
            const OUString sName = pView->getQualifiedName( nullptr /* means 'first selected' */ );
            showPreviewFor( eType, sName );
        }
    }
}

void OApplicationController::showPreviewFor(const ElementType _eType,const OUString& _sName)
{
    if ( m_ePreviewMode == E_PREVIEWNONE )
        return;

    OApplicationView* pView = getContainer();
    if ( !pView )
        return;

    try
    {
        switch( _eType )
        {
            case E_FORM:
            case E_REPORT:
            {
                Reference< XHierarchicalNameAccess > xContainer( getElements( _eType ), UNO_QUERY_THROW );
                Reference< XContent> xContent( xContainer->getByHierarchicalName( _sName ), UNO_QUERY_THROW );
                pView->showPreview( xContent );
            }
            break;

            case E_TABLE:
            case E_QUERY:
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                        pView->showPreview( getDatabaseName(), xConnection, _sName, _eType == E_TABLE );
                }
                return;

            default:
                OSL_FAIL( "OApplicationController::showPreviewFor: unexpected element type!" );
                break;
        }
    }
    catch( const SQLException& )
    {
        showError( SQLExceptionInfo( ::cppu::getCaughtException() ) );
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

IMPL_LINK_NOARG_TYPED(OApplicationController, OnClipboardChanged, TransferableDataHelper*, void)
{
    OnInvalidateClipboard();
}

void OApplicationController::OnInvalidateClipboard()
{
    InvalidateFeature(ID_BROWSER_CUT);
    InvalidateFeature(ID_BROWSER_COPY);
    InvalidateFeature(ID_BROWSER_PASTE);
    InvalidateFeature(SID_DB_APP_PASTE_SPECIAL);
}

void OApplicationController::onCopyEntry()
{
    Execute(ID_BROWSER_COPY,Sequence<PropertyValue>());
}

void OApplicationController::onPasteEntry()
{
    Execute(ID_BROWSER_PASTE,Sequence<PropertyValue>());
}

void OApplicationController::onDeleteEntry()
{
    ElementType eType = getContainer()->getElementType();
    sal_uInt16 nId = 0;
    switch(eType)
    {
        case E_TABLE:
            nId = SID_DB_APP_TABLE_DELETE;
            break;
        case E_QUERY:
            nId = SID_DB_APP_QUERY_DELETE;
            break;
        case E_FORM:
            nId = SID_DB_APP_FORM_DELETE;
            break;
        case E_REPORT:
            nId = SID_DB_APP_REPORT_DELETE;
            break;
        default:
            OSL_FAIL("Invalid ElementType!");
            break;
    }
    executeChecked(nId,Sequence<PropertyValue>());
}

void OApplicationController::executeUnChecked(const URL& _rCommand, const Sequence< PropertyValue>& aArgs)
{
    OGenericUnoController::executeUnChecked( _rCommand, aArgs );
}

void OApplicationController::executeChecked(const URL& _rCommand, const Sequence< PropertyValue>& aArgs)
{
    OGenericUnoController::executeChecked( _rCommand, aArgs );
}

void OApplicationController::executeUnChecked(sal_uInt16 _nCommandId, const Sequence< PropertyValue>& aArgs)
{
    OGenericUnoController::executeUnChecked( _nCommandId, aArgs );
}

void OApplicationController::executeChecked(sal_uInt16 _nCommandId, const Sequence< PropertyValue>& aArgs)
{
    OGenericUnoController::executeChecked( _nCommandId, aArgs );
}

bool OApplicationController::isCommandEnabled(sal_uInt16 _nCommandId) const
{
    return OGenericUnoController::isCommandEnabled( _nCommandId );
}

bool OApplicationController::isCommandEnabled( const OUString& _rCompleteCommandURL ) const
{
    return OGenericUnoController::isCommandEnabled( _rCompleteCommandURL );
}

sal_uInt16 OApplicationController::registerCommandURL( const OUString& _rCompleteCommandURL )
{
    return OGenericUnoController::registerCommandURL( _rCompleteCommandURL );
}

void OApplicationController::notifyHiContrastChanged()
{
    OGenericUnoController::notifyHiContrastChanged();
}

Reference< XController > OApplicationController::getXController() throw( RuntimeException )
{
    return OGenericUnoController::getXController();
}

bool OApplicationController::interceptUserInput( const NotifyEvent& _rEvent )
{
    return OGenericUnoController::interceptUserInput( _rEvent );
}

PopupMenu* OApplicationController::getContextMenu( Control& /*_rControl*/ ) const
{
    return new PopupMenu( ModuleRes( RID_MENU_APP_EDIT ) );
}

IController& OApplicationController::getCommandController()
{
    return *this;
}

::cppu::OInterfaceContainerHelper* OApplicationController::getContextMenuInterceptors()
{
    return &m_aContextMenuInterceptors;
}

Any OApplicationController::getCurrentSelection( Control& _rControl ) const
{
    Sequence< NamedDatabaseObject > aSelection;
    getContainer()->describeCurrentSelectionForControl( _rControl, aSelection );
    return makeAny( aSelection );
}

bool OApplicationController::requestQuickHelp( const SvTreeListEntry* /*_pEntry*/, OUString& /*_rText*/ ) const
{
    return false;
}

bool OApplicationController::requestDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
{
    TransferableHelper* pTransfer = nullptr;
    if ( getContainer() && getContainer()->getSelectionCount() )
    {
        try
        {
            pTransfer = copyObject( );
            Reference< XTransferable> xEnsureDelete = pTransfer;

            if ( pTransfer && getContainer()->getDetailView() )
            {
                ElementType eType = getContainer()->getElementType();
                pTransfer->StartDrag( getContainer()->getDetailView()->getTreeWindow(), ((eType == E_FORM || eType == E_REPORT) ? DND_ACTION_COPYMOVE : DND_ACTION_COPY) );
            }
        }
        catch(const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return nullptr != pTransfer;
}

sal_Int8 OApplicationController::queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors )
{
    sal_Int8 nActionAskedFor = _rEvt.mnAction;
    // check if we're a table or query container
    OApplicationView* pView = getContainer();
    if ( pView && !isDataSourceReadOnly() )
    {
        ElementType eType = pView->getElementType();
        if ( eType != E_NONE && (eType != E_TABLE || !isConnectionReadOnly()) )
        {
            // check for the concrete type
            if(::std::any_of(_rFlavors.begin(),_rFlavors.end(),TAppSupportedSotFunctor(eType,true)))
                return DND_ACTION_COPY;
            if ( eType == E_FORM || eType == E_REPORT )
            {
                sal_Int8 nAction = OComponentTransferable::canExtractComponentDescriptor(_rFlavors,eType == E_FORM) ? DND_ACTION_COPY : DND_ACTION_NONE;
                if ( nAction != DND_ACTION_NONE )
                {
                    SvTreeListEntry* pHitEntry = pView->getEntry(_rEvt.maPosPixel);
                    OUString sName;
                    if ( pHitEntry )
                    {
                        sName = pView->getQualifiedName( pHitEntry );
                        if ( !sName.isEmpty() )
                        {
                            Reference< XHierarchicalNameAccess > xContainer(getElements(pView->getElementType()),UNO_QUERY);
                            if ( xContainer.is() && xContainer->hasByHierarchicalName(sName) )
                            {
                                Reference< XHierarchicalNameAccess > xHitObject(xContainer->getByHierarchicalName(sName),UNO_QUERY);
                                if ( xHitObject.is() )
                                    nAction = nActionAskedFor & DND_ACTION_COPYMOVE;
                            }
                            else
                                nAction = DND_ACTION_NONE;
                        }
                    }
                }
                return nAction;
            }
        }
    }

    return DND_ACTION_NONE;
}

sal_Int8 OApplicationController::executeDrop( const ExecuteDropEvent& _rEvt )
{
    OApplicationView* pView = getContainer();
    if ( !pView || pView->getElementType() == E_NONE )
    {
        OSL_FAIL("OApplicationController::executeDrop: what the hell did queryDrop do?");
            // queryDrop should not have allowed us to reach this situation ....
        return DND_ACTION_NONE;
    }

    // a TransferableDataHelper for accessing the dropped data
    TransferableDataHelper aDroppedData(_rEvt.maDropEvent.Transferable);

    // reset the data of the previous async drop (if any)
    if ( m_nAsyncDrop )
        Application::RemoveUserEvent(m_nAsyncDrop);

    m_nAsyncDrop = nullptr;
    m_aAsyncDrop.aDroppedData.clear();
    m_aAsyncDrop.nType          = pView->getElementType();
    m_aAsyncDrop.nAction        = _rEvt.mnAction;
    m_aAsyncDrop.bError         = false;
    m_aAsyncDrop.bHtml          = false;
    m_aAsyncDrop.aUrl.clear();

    // loop through the available formats and see what we can do ...
    // first we have to check if it is our own format, if not we have to copy the stream :-(
    if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(aDroppedData.GetDataFlavorExVector()) )
    {
        m_aAsyncDrop.aDroppedData   = ODataAccessObjectTransferable::extractObjectDescriptor(aDroppedData);

        // asynchron because we some dialogs and we aren't allowed to show them while in D&D
        m_nAsyncDrop = Application::PostUserEvent(LINK(this, OApplicationController, OnAsyncDrop));
        return DND_ACTION_COPY;
    }
    else if ( OComponentTransferable::canExtractComponentDescriptor(aDroppedData.GetDataFlavorExVector(),m_aAsyncDrop.nType == E_FORM) )
    {
        m_aAsyncDrop.aDroppedData = OComponentTransferable::extractComponentDescriptor(aDroppedData);
        SvTreeListEntry* pHitEntry = pView->getEntry(_rEvt.maPosPixel);
        if ( pHitEntry )
            m_aAsyncDrop.aUrl = pView->getQualifiedName( pHitEntry );

        sal_Int8 nAction = _rEvt.mnAction;
        Reference<XContent> xContent;
        m_aAsyncDrop.aDroppedData[daComponent] >>= xContent;
        if ( xContent.is() )
        {
            OUString sName = xContent->getIdentifier()->getContentIdentifier();
            sal_Int32 nIndex = 0;
            sName = sName.copy(sName.getToken(0,'/',nIndex).getLength() + 1);
            if ( m_aAsyncDrop.aUrl.getLength() >= sName.getLength() && m_aAsyncDrop.aUrl.startsWith(sName) )
            {
                m_aAsyncDrop.aDroppedData.clear();
                return DND_ACTION_NONE;
            }

            // check if move is allowed, if another object with the same name exists only copy is allowed
            Reference< XHierarchicalNameAccess > xContainer(getElements(m_aAsyncDrop.nType),UNO_QUERY);
            Reference<XNameAccess> xNameAccess(xContainer,UNO_QUERY);

            if ( !m_aAsyncDrop.aUrl.isEmpty() && xContainer.is() && xContainer->hasByHierarchicalName(m_aAsyncDrop.aUrl) )
                xNameAccess.set(xContainer->getByHierarchicalName(m_aAsyncDrop.aUrl),UNO_QUERY);

            if ( xNameAccess.is() )
            {
                Reference<XPropertySet> xProp(xContent,UNO_QUERY);
                if ( xProp.is() )
                {
                    xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
                    if ( xNameAccess.is() && xNameAccess->hasByName(sName) )
                        nAction &= ~DND_ACTION_MOVE;
                }
                else
                    nAction &= ~DND_ACTION_MOVE;
            }
        }
        if ( nAction != DND_ACTION_NONE )
        {
            m_aAsyncDrop.nAction = nAction;
            // asynchron because we some dialogs and we aren't allowed to show them while in D&D
            m_nAsyncDrop = Application::PostUserEvent(LINK(this, OApplicationController, OnAsyncDrop));
        }
        else
            m_aAsyncDrop.aDroppedData.clear();
        return nAction;
    }
    else
    {
        SharedConnection xConnection( ensureConnection() );
        if ( xConnection.is() && m_aTableCopyHelper.copyTagTable( aDroppedData, m_aAsyncDrop, xConnection ) )
        {
            // asynchron because we some dialogs and we aren't allowed to show them while in D&D
            m_nAsyncDrop = Application::PostUserEvent(LINK(this, OApplicationController, OnAsyncDrop));
            return DND_ACTION_COPY;
        }
    }

    return DND_ACTION_NONE;
}

Reference< XModel >  SAL_CALL OApplicationController::getModel() throw( RuntimeException, std::exception )
{
    return m_xModel;
}

void OApplicationController::onAttachedFrame()
{
    sal_Int32 nConnectedControllers( 0 );
    try
    {
        Reference< XModel2 > xModel( m_xModel, UNO_QUERY_THROW );
        Reference< XEnumeration > xEnumControllers( xModel->getControllers(), UNO_SET_THROW );
        while ( xEnumControllers->hasMoreElements() )
        {
            Reference< XController > xController( xEnumControllers->nextElement(), UNO_QUERY_THROW );
            ++nConnectedControllers;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( nConnectedControllers > 1 )
    {   // we are not the first connected controller, there were already others
        return;
    }

    OnFirstControllerConnected();
}

void OApplicationController::OnFirstControllerConnected()
{
    if ( !m_xModel.is() )
    {
        OSL_FAIL( "OApplicationController::OnFirstControllerConnected: too late!" );
    }

    // if we have forms or reports which contain macros/scripts, then show a warning
    // which suggests the user to migrate them to the database document
    Reference< XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
    if ( xDocumentScripts.is() )
    {
        // no need to show this warning, obviously the document supports embedding scripts
        // into itself, so there are no "old-style" forms/reports which have macros/scripts
        // themselves
        return;
    }

    try
    {
        // If the migration just happened, but was not successful, the document is reloaded.
        // In this case, we should not show the warning, again.
        ::comphelper::NamedValueCollection aModelArgs( m_xModel->getArgs() );
        if ( aModelArgs.getOrDefault( "SuppressMigrationWarning", sal_False ) )
            return;

        // also, if the document is read-only, then no migration is possible, and the
        // respective menu entry is hidden. So, don't show the warning in this case, too.
        if ( Reference< XStorable >( m_xModel, UNO_QUERY_THROW )->isReadonly() )
            return;

        SQLWarning aWarning;
        aWarning.Message = OUString( ModuleRes( STR_SUB_DOCS_WITH_SCRIPTS ) );
        SQLException aDetail;
        aDetail.Message = OUString( ModuleRes( STR_SUB_DOCS_WITH_SCRIPTS_DETAIL ) );
        aWarning.NextException <<= aDetail;

        Reference< XExecutableDialog > xDialog = ErrorMessageDialog::create( getORB(), "", nullptr, makeAny( aWarning ) );
        xDialog->execute();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return;
}

void SAL_CALL OApplicationController::attachFrame( const Reference< XFrame > & i_rxFrame ) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( getMutex() );

    OGenericUnoController::attachFrame( i_rxFrame );
    if ( getFrame().is() )
        onAttachedFrame();
}

sal_Bool SAL_CALL OApplicationController::attachModel(const Reference< XModel > & _rxModel) throw( RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( getMutex() );
    const Reference< XOfficeDatabaseDocument > xOfficeDoc( _rxModel, UNO_QUERY );
    const Reference< XModifiable > xDocModify( _rxModel, UNO_QUERY );
    if ( ( !xOfficeDoc.is() || !xDocModify.is() ) && _rxModel.is() )
    {
        OSL_FAIL( "OApplicationController::attachModel: invalid model!" );
        return sal_False;
    }

    if ( m_xModel.is() && ( m_xModel != _rxModel ) && ( _rxModel.is() ) )
    {
        OSL_ENSURE( false, "OApplicationController::attachModel: missing implementation: setting a new model while we have another one!" );
        // we'd need to completely update our view here, close sub components, and the like
        return sal_False;
    }

    const OUString aPropertyNames[] =
    {
        OUString(PROPERTY_URL), OUString(PROPERTY_USER)
    };

    // disconnect from old model
    try
    {
        if ( m_xDataSource.is() )
        {
            for ( size_t i=0; i < sizeof( aPropertyNames ) / sizeof( aPropertyNames[0] ); ++i )
            {
                m_xDataSource->removePropertyChangeListener( aPropertyNames[i], this );
            }
        }

        Reference< XModifyBroadcaster >  xBroadcaster( m_xModel, UNO_QUERY );
        if ( xBroadcaster.is() )
            xBroadcaster->removeModifyListener( this );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    m_xModel = _rxModel;
    m_xDataSource.set( xOfficeDoc.is() ? xOfficeDoc->getDataSource() : Reference< XDataSource >(), UNO_QUERY );

    // connect to new model
    try
    {
        if ( m_xDataSource.is() )
        {
            for ( size_t i=0; i < sizeof( aPropertyNames ) / sizeof( aPropertyNames[0] ); ++i )
            {
                m_xDataSource->addPropertyChangeListener( aPropertyNames[i], this );
            }
        }

        Reference< XModifyBroadcaster >  xBroadcaster( m_xModel, UNO_QUERY_THROW );
        xBroadcaster->addModifyListener( this );

    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // initial preview mode
    if ( m_xDataSource.is() )
    {
        try
        {
            // to get the 'modified' for the data source
            ::comphelper::NamedValueCollection aLayoutInfo( m_xDataSource->getPropertyValue( PROPERTY_LAYOUTINFORMATION ) );
            if ( aLayoutInfo.has( OUString(INFO_PREVIEW) ) )
            {
                const sal_Int32 nPreviewMode( aLayoutInfo.getOrDefault( INFO_PREVIEW, (sal_Int32)0 ) );
                m_ePreviewMode = static_cast< PreviewMode >( nPreviewMode );
                if ( getView() )
                    getContainer()->switchPreview( m_ePreviewMode );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    return sal_True;
}

void OApplicationController::containerFound( const Reference< XContainer >& _xContainer)
{
    try
    {
        if ( _xContainer.is() )
        {
            m_aCurrentContainers.push_back(_xContainer);
            _xContainer->addContainerListener(this);
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

OUString OApplicationController::getCurrentlySelectedName(sal_Int32& _rnCommandType) const
{
    _rnCommandType = ( (getContainer()->getElementType() == E_QUERY)
                                ? CommandType::QUERY : ( (getContainer()->getElementType() == E_TABLE) ? CommandType::TABLE : -1 ));

    OUString sName;
    if ( _rnCommandType != -1 )
    {
        try
        {
            sName = getContainer()->getQualifiedName( nullptr );
            OSL_ENSURE( !sName.isEmpty(), "OApplicationController::getCurrentlySelectedName: no name given!" );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return sName;
}

void SAL_CALL OApplicationController::addSelectionChangeListener( const Reference< view::XSelectionChangeListener >& _Listener ) throw (RuntimeException, std::exception)
{
    m_pSelectionNotifier->addListener( _Listener );
}

void SAL_CALL OApplicationController::removeSelectionChangeListener( const Reference< view::XSelectionChangeListener >& _Listener ) throw (RuntimeException, std::exception)
{
    m_pSelectionNotifier->removeListener( _Listener );
}

sal_Bool SAL_CALL OApplicationController::select( const Any& _aSelection ) throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );
    Sequence< OUString> aSelection;
    if ( !_aSelection.hasValue() || !getView() )
    {
        getContainer()->selectElements(aSelection);
        return sal_True;
    }

    // BEGIN compatibility
    Sequence< NamedValue > aCurrentSelection;
    if ( (_aSelection >>= aCurrentSelection) && aCurrentSelection.getLength() )
    {
        ElementType eType = E_NONE;
        const NamedValue* pIter = aCurrentSelection.getConstArray();
        const NamedValue* pEnd  = pIter + aCurrentSelection.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( pIter->Name == "Type" )
            {
                sal_Int32 nType = 0;
                pIter->Value >>= nType;
                if ( nType < DatabaseObject::TABLE || nType > DatabaseObject::REPORT )
                    throw IllegalArgumentException();
                eType = static_cast< ElementType >( nType );
            }
            else if ( pIter->Name == "Selection" )
                pIter->Value >>= aSelection;
        }

        m_aSelectContainerEvent.CancelCall();   // just in case the async select request was running
        getContainer()->selectContainer(eType);
        getContainer()->selectElements(aSelection);
        return sal_True;
    }
    // END compatibility

    Sequence< NamedDatabaseObject > aSelectedObjects;
    if ( !( _aSelection >>= aSelectedObjects ) )
    {
        aSelectedObjects.realloc( 1 );
        if ( !( _aSelection >>= aSelectedObjects[0] ) )
            throw IllegalArgumentException();
    }

    SelectionByElementType aSelectedElements;
    ElementType eSelectedCategory = E_NONE;
    for (   const NamedDatabaseObject* pObject = aSelectedObjects.getConstArray();
            pObject != aSelectedObjects.getConstArray() + aSelectedObjects.getLength();
            ++pObject
        )
    {
        switch ( pObject->Type )
        {
            case DatabaseObject::TABLE:
            case DatabaseObjectContainer::SCHEMA:
            case DatabaseObjectContainer::CATALOG:
                aSelectedElements[ E_TABLE ].push_back( pObject->Name );
                break;
            case DatabaseObject::QUERY:
                aSelectedElements[ E_QUERY ].push_back( pObject->Name );
                break;
            case DatabaseObject::FORM:
            case DatabaseObjectContainer::FORMS_FOLDER:
                aSelectedElements[ E_FORM ].push_back( pObject->Name );
                break;
            case DatabaseObject::REPORT:
            case DatabaseObjectContainer::REPORTS_FOLDER:
                aSelectedElements[ E_REPORT ].push_back( pObject->Name );
                break;
            case DatabaseObjectContainer::TABLES:
            case DatabaseObjectContainer::QUERIES:
            case DatabaseObjectContainer::FORMS:
            case DatabaseObjectContainer::REPORTS:
                if ( eSelectedCategory != E_NONE )
                    throw IllegalArgumentException(
                        OUString(ModuleRes(RID_STR_NO_DIFF_CAT)),
                        *this, sal_Int16( pObject - aSelectedObjects.getConstArray() ) );
                eSelectedCategory =
                        ( pObject->Type == DatabaseObjectContainer::TABLES )  ? E_TABLE
                    :   ( pObject->Type == DatabaseObjectContainer::QUERIES ) ? E_QUERY
                    :   ( pObject->Type == DatabaseObjectContainer::FORMS )   ? E_FORM
                    :   ( pObject->Type == DatabaseObjectContainer::REPORTS ) ? E_REPORT
                    :   E_NONE;
                break;

            default:
            case DatabaseObjectContainer::DATA_SOURCE:
            {
                OUString sMessage(
                        OUString(ModuleRes(RID_STR_UNSUPPORTED_OBJECT_TYPE)).
                    replaceFirst("$type$", OUString::number(pObject->Type)));
                throw IllegalArgumentException(sMessage, *this, sal_Int16( pObject - aSelectedObjects.getConstArray() ));
            }
        }
    }

    for (   SelectionByElementType::const_iterator sel = aSelectedElements.begin();
            sel != aSelectedElements.end();
            ++sel
        )
    {
        if ( sel->first == m_eCurrentType )
        {
            getContainer()->selectElements( comphelper::containerToSequence(sel->second) );
        }
        else
        {
            m_aPendingSelection[ sel->first ] = sel->second;
        }
    }

    m_aSelectContainerEvent.CancelCall();   // just in case the async select request was running
    getContainer()->selectContainer( eSelectedCategory );

    return sal_True;
}

Any SAL_CALL OApplicationController::getSelection(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getMutex() );

    Sequence< NamedDatabaseObject > aCurrentSelection;
    const ElementType eType( getContainer()->getElementType() );
    if ( eType != E_NONE )
    {
        getContainer()->describeCurrentSelectionForType( eType, aCurrentSelection );
        if ( aCurrentSelection.getLength() == 0 )
        {   // if no objects are selected, add an entry to the sequence which describes the overall category
            // which is selected currently
            aCurrentSelection.realloc(1);
            aCurrentSelection[0].Name = getDatabaseName();
            switch ( eType )
            {
            case E_TABLE:   aCurrentSelection[0].Type = DatabaseObjectContainer::TABLES;   break;
            case E_QUERY:   aCurrentSelection[0].Type = DatabaseObjectContainer::QUERIES;  break;
            case E_FORM:    aCurrentSelection[0].Type = DatabaseObjectContainer::FORMS;    break;
            case E_REPORT:  aCurrentSelection[0].Type = DatabaseObjectContainer::REPORTS;  break;
            default:
                OSL_FAIL( "OApplicationController::getSelection: unexpected current element type!" );
                break;
            }
        }
    }
    return makeAny( aCurrentSelection );
}

void OApplicationController::impl_migrateScripts_nothrow()
{
    try
    {
        Reference< XExecutableDialog > xDialog = css::sdb::application::MacroMigrationWizard::createWithDocument( getORB(), Reference< XOfficeDatabaseDocument >( m_xModel, UNO_QUERY_THROW ) );
        xDialog->execute();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
