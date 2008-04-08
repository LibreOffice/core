/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppController.cxx,v $
 *
 *  $Revision: 1.58 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-08 12:42:49 $
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

#include "AppController.hxx"
#include "dbustrings.hrc"
#include "advancedsettingsdlg.hxx"

/** === begin UNO includes === **/
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
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/document/XEmbeddedScripts.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/beans/NamedValue.hpp"
#include <com/sun/star/awt/XTopWindow.hpp>
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#include <comphelper/componentcontext.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBAUI_APPVIEW_HXX
#include "AppView.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _DBAU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef _DBU_APP_HRC_
#include "dbu_app.hrc"
#endif
#ifndef DBACCESS_SOURCE_UI_MISC_DEFAULTOBJECTNAMECHECK_HXX
#include "defaultobjectnamecheck.hxx"
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_INTERNALOPTIONS_HXX
#include <svtools/internaloptions.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef DBACCESS_DATABASE_OBJECT_VIEW_HXX
#include "databaseobjectview.hxx"
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SVLBOXITM_HXX
#include <svtools/svlbitm.hxx>
#endif
#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif
#include "ExtensionNotPresent.hxx"
#ifndef DBAUI_APPDETAILVIEW_HXX
#include "AppDetailView.hxx"
#endif
#ifndef _DBAUI_LINKEDDOCUMENTS_HXX_
#include "linkeddocuments.hxx"
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef INCLUDED_SFX_MAILMODELAPI_HXX
#include <sfx2/mailmodelapi.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _CLIPLISTENER_HXX
#include <svtools/cliplistener.hxx>
#endif
#ifndef _SVX_DBAEXCHANGE_HXX_
#include <svx/dbaexchange.hxx>
#endif
#ifndef SVX_DBAOBJECTEX_HXX
#include <svx/dbaobjectex.hxx>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#include <svx/svxdlg.hxx>
#include <svtools/insdlg.hxx>
#ifndef _UNOTOOLS_TEMPFILE_HXX
#include <unotools/tempfile.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif
#ifndef DBAUI_DLGSAVE_HXX
#include "dlgsave.hxx"
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
#ifndef SFX_QUERYSAVEDOCUMENT_HXX
#include <sfx2/QuerySaveDocument.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _DBACCESS_SLOTID_HRC_
#include "dbaccess_slotid.hrc"
#endif

#include <boost/mem_fn.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <algorithm>
#include <functional>

extern "C" void SAL_CALL createRegistryInfo_ODBApplication()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OApplicationController > aAutoRegistration;
}
//........................................................................
namespace dbaui
{
//........................................................................
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
using ::com::sun::star::document::XEmbeddedScripts;

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OApplicationController::getImplementationName() throw( RuntimeException )
{
    return getImplementationName_Static();
}

//------------------------------------------------------------------------------
::rtl::OUString OApplicationController::getImplementationName_Static() throw( RuntimeException )
{
    return ::rtl::OUString(SERVICE_SDB_APPLICATIONCONTROLLER);
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString> OApplicationController::getSupportedServiceNames_Static(void) throw( RuntimeException )
{
    Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.ApplicationController");
    return aSupported;
}
//-------------------------------------------------------------------------
Sequence< ::rtl::OUString> SAL_CALL OApplicationController::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OApplicationController::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new OApplicationController(_rxFactory));
}
// -----------------------------------------------------------------------------

struct XContainerFunctor : public ::std::unary_function< OApplicationController::TContainerVector::value_type , bool>
{
    Reference<XContainerListener> m_xContainerListener;
    XContainerFunctor( const Reference<XContainerListener>& _xContainerListener)
        : m_xContainerListener(_xContainerListener){}

    bool operator() (const OApplicationController::TContainerVector::value_type& lhs) const
    {
        if ( lhs.is() )
            lhs->removeContainerListener(m_xContainerListener);
        return true;
    }
};

//====================================================================
//= OApplicationController
//====================================================================
DBG_NAME(OApplicationController)
//--------------------------------------------------------------------
OApplicationController::OApplicationController(const Reference< XMultiServiceFactory >& _rxORB)
    :OApplicationController_CBASE( _rxORB )
    ,m_aTableCopyHelper(this)
    ,m_pClipbordNotifier(NULL)
    ,m_nAsyncDrop(0)
    ,m_aControllerConnectedEvent( LINK( this, OApplicationController, OnFirstControllerConnected ) )
    ,m_ePreviewMode(E_PREVIEWNONE)
    ,m_eCurrentType(E_NONE)
    ,m_bNeedToReconnect(sal_False)
    ,m_bSuspended( sal_False )
{
    DBG_CTOR(OApplicationController,NULL);

    m_aTypeCollection.initUserDriverTypes(_rxORB);
}
//------------------------------------------------------------------------------
OApplicationController::~OApplicationController()
{
    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        OSL_ENSURE(0,"Please check who doesn't dispose this component!");
        // increment ref count to prevent double call of Dtor
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
    ::std::auto_ptr< Window> aTemp(m_pView);
    m_pView = NULL;

    DBG_DTOR(OApplicationController,NULL);
}
//--------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2(OApplicationController,OApplicationController_CBASE,OApplicationController_Base)
IMPLEMENT_FORWARD_XINTERFACE2(OApplicationController,OApplicationController_CBASE,OApplicationController_Base)
// -----------------------------------------------------------------------------
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

//--------------------------------------------------------------------
void SAL_CALL OApplicationController::disposing()
{
    m_aControllerConnectedEvent.CancelCall();

    ::std::for_each(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),XContainerFunctor(this));
    m_aCurrentContainers.clear();
    m_aSpecialSubFrames.clear();
    m_aDocuments.clear();

    if ( getView() )
    {
        getContainer()->showPreview(NULL);
        m_pClipbordNotifier->ClearCallbackLink();
        m_pClipbordNotifier->AddRemoveListener( getView(), sal_False );
        m_pClipbordNotifier->release();
        m_pClipbordNotifier = NULL;
    }

    disconnect();
    try
    {
        Reference < XFrame > xFrame;
        attachFrame( xFrame );

        if ( m_xDataSource.is() )
        {
            m_xDataSource->removePropertyChangeListener(::rtl::OUString(), this);
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
            m_xDataSource = NULL;
        }

        Reference< XModifyBroadcaster > xBroadcaster( m_xModel, UNO_QUERY );
        if ( xBroadcaster.is() )
            xBroadcaster->removeModifyListener(static_cast<XModifyListener*>(this));

        if ( m_xModel.is() )
        {
            ::rtl::OUString sUrl = m_xModel->getURL();
            if ( sUrl.getLength() )
            {
                ::rtl::OUString     aFilter;
                INetURLObject       aURL( m_xModel->getURL() );
                const SfxFilter* pFilter = getStandardDatabaseFilter();
                if ( pFilter )
                    aFilter = pFilter->GetFilterName();

                // add to svtool history options
                SvtHistoryOptions().AppendItem( ePICKLIST,
                        aURL.GetURLNoPass( INetURLObject::NO_DECODE ),
                        aFilter,
                        getStrippedDatabaseName(),
                        ::rtl::OUString() );
            }

            m_aModelConnector.clear();
            m_xModel.clear();
        }
    }
    catch(Exception)
    {
    }

    m_pView = NULL;
    OApplicationController_CBASE::disposing(); // here the m_refCount must be equal 5
}

//--------------------------------------------------------------------
sal_Bool OApplicationController::Construct(Window* _pParent)
{
    m_pView = new OApplicationView(_pParent,getORB(),this,this,this,this,this,this,m_ePreviewMode);
    m_pView->SetUniqueId(UID_APP_VIEW);

    // late construction
    sal_Bool bSuccess = sal_False;
    try
    {
        getContainer()->Construct();
        bSuccess = sal_True;
    }
    catch(SQLException&)
    {
    }
    catch(Exception&)
    {
        DBG_ERROR("OApplicationController::Construct : the construction of UnoDataBrowserView failed !");
    }

    if ( !bSuccess )
    {
        ::std::auto_ptr< Window> aTemp(m_pView);
        m_pView = NULL;
        return sal_False;
    }

    DBG_ASSERT( getView(), "OApplicationController::Construct: have no view!" );
    if ( getView() )
        getView()->enableSeparator( );

    // now that we have a view we can create the clipboard listener
    m_aSystemClipboard = TransferableDataHelper::CreateFromSystemClipboard( getView() );
    m_aSystemClipboard.StartClipboardListening( );

    m_pClipbordNotifier = new TransferableClipboardListener( LINK( this, OApplicationController, OnClipboardChanged ) );
    m_pClipbordNotifier->acquire();
    m_pClipbordNotifier->AddRemoveListener( getView(), sal_True );

    OApplicationController_CBASE::Construct( _pParent );
    getView()->Show();

    return sal_True;
}

//--------------------------------------------------------------------
void SAL_CALL OApplicationController::disposing(const EventObject& _rSource) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    Reference<XConnection> xCon(_rSource.Source, UNO_QUERY);
    if ( xCon.is() )
    {
        DBG_ASSERT( ( m_xDataSourceConnection == xCon ) && getContainer() && ( getContainer()->getElementType() == E_TABLE ),
            "OApplicationController::disposing: the below code will ignore this call - why?" );

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
        m_aModelConnector.clear();
    }
    else if ( _rSource.Source == m_xDataSource )
    {
        m_xDataSource = NULL;
    }
    else
    {
        Reference<XComponent> xComp(_rSource.Source,UNO_QUERY);
        Reference<XContainer> xContainer(_rSource.Source,UNO_QUERY);
        if ( xComp.is() )
        {
            TDocuments::iterator aFind = ::std::find_if(m_aDocuments.begin(),m_aDocuments.end(),
                ::std::compose1(::std::bind2nd(::std::equal_to<Reference<XComponent> >(),xComp),::std::select1st<TDocuments::value_type>()));
            if ( aFind != m_aDocuments.end() )
                m_aDocuments.erase(aFind);

            TFrames::iterator aFind2 = ::std::find_if(m_aSpecialSubFrames.begin(),m_aSpecialSubFrames.end(),
                ::std::compose1(::std::bind2nd(::std::equal_to<Reference<XComponent> >(),xComp),
                    ::std::compose1(::std::select2nd<TTypeFrame>(),::std::select2nd<TFrames::value_type>())));
            if ( aFind2 != m_aSpecialSubFrames.end() )
                m_aSpecialSubFrames.erase(aFind2);

        }
        if ( xContainer.is() )
        {
            TContainerVector::iterator aFind = ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xContainer);
            if ( aFind != m_aCurrentContainers.end() )
                m_aCurrentContainers.erase(aFind);
        }
        OApplicationController_CBASE::disposing( _rSource );
    }
}
//--------------------------------------------------------------------
sal_Bool SAL_CALL OApplicationController::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    if ( getView() && getView()->IsInModalMode() )
        return sal_False;

    sal_Bool bCanSuspend = sal_True;

    if ( m_bSuspended != bSuspend )
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
                    bCanSuspend = sal_False;
                default:
                    break;
            }
        }
    }

    if ( bCanSuspend )
        m_bSuspended = bSuspend;

    return bCanSuspend;
}
// -----------------------------------------------------------------------------
FeatureState OApplicationController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    aReturn.bEnabled = sal_False;
    // check this first
    if ( !getContainer() || m_bReadOnly )
        return aReturn;

    try
    {
        switch (_nId)
        {
            case SID_OPENURL:
                aReturn.bEnabled = sal_True;
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
                aReturn.bEnabled = aReturn.bEnabled && ( (ID_BROWSER_CUT == _nId && getContainer()->getElementType() == E_TABLE) ? getContainer()->isCutAllowed() : sal_True);
                break;
            case ID_BROWSER_PASTE:
                switch( getContainer()->getElementType() )
                {
                    case E_TABLE:
                        aReturn.bEnabled = !isDataSourceReadOnly() && !isConnectionReadOnly() && isTableFormat();
                        break;
                    case E_QUERY:
                        aReturn.bEnabled = !isDataSourceReadOnly() && getViewClipboard().HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY);
                        break;
                    default:
                        aReturn.bEnabled = !isDataSourceReadOnly() && OComponentTransferable::canExtractComponentDescriptor(getViewClipboard().GetDataFlavorExVector(),getContainer()->getElementType() == E_FORM);
                }
                break;
            case SID_DB_APP_PASTE_SPECIAL:
                aReturn.bEnabled = getContainer()->getElementType() == E_TABLE && !isDataSourceReadOnly() && !isConnectionReadOnly() && isTableFormat();
                break;
            case SID_OPENDOC:
            case SID_HELP_INDEX:
                aReturn.bEnabled = sal_True;
                break;
            case ID_BROWSER_SAVEDOC:
                aReturn.bEnabled = !isDataSourceReadOnly() && m_xDocumentModify.is() && m_xDocumentModify->isModified();
                break;
            case ID_BROWSER_SAVEASDOC:
                aReturn.bEnabled = sal_True;
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
                aReturn.bEnabled = !isDataSourceReadOnly() && SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SWRITER);
                break;
            case SID_APP_NEW_REPORT:
                aReturn.bEnabled = !isDataSourceReadOnly()
                                    && SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SWRITER);
                if ( aReturn.bEnabled )
                {
                    Reference< XContentEnumerationAccess > xEnumAccess(m_xServiceFactory, UNO_QUERY);
                    aReturn.bEnabled = xEnumAccess.is();
                    if ( aReturn.bEnabled )
                    {
                        const ::rtl::OUString sReportEngineServiceName = ::dbtools::getDefaultReportEngineServiceName(m_xServiceFactory);
                        aReturn.bEnabled = sReportEngineServiceName.getLength() != 0;
                        if ( aReturn.bEnabled )
                        {
                            const Reference< XEnumeration > xEnumDrivers = xEnumAccess->createContentEnumeration(sReportEngineServiceName);
                            aReturn.bEnabled = xEnumDrivers.is() && xEnumDrivers->hasMoreElements();
                        }
                    }
                }
                break;
            case SID_DB_APP_VIEW_TABLES:
                aReturn.bEnabled = sal_True;
                aReturn.bChecked = getContainer()->getElementType() == E_TABLE;
                break;
            case SID_DB_APP_VIEW_QUERIES:
                aReturn.bEnabled = sal_True;
                aReturn.bChecked = getContainer()->getElementType() == E_QUERY;
                break;
            case SID_DB_APP_VIEW_FORMS:
                aReturn.bEnabled = sal_True;
                aReturn.bChecked = getContainer()->getElementType() == E_FORM;
                break;
            case SID_DB_APP_VIEW_REPORTS:
                aReturn.bEnabled = sal_True;
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
                aReturn.bEnabled = sal_True;
                break;
            case ID_MIGRATE_SCRIPTS:
            {
                // Our document supports embedding scripts into it, if and only if there are no
                // forms/reports with macros/scripts into them. So, we need to enable migration
                // if and only if the database document does *not* support embedding scripts.
//                bool bAvailable = !Reference< XEmbeddedScripts >( m_xModel, UNO_QUERY ).is();
                // TODO: revert to the disabled code. The current version is just to be able
                // to integrate an intermediate version of the CWS, which should behave as
                // if no macros in DB docs are allowed
                bool bAvailable = false;
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
                                    && SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SWRITER)
                                    && getContainer()->isALeafSelected();
                if ( aReturn.bEnabled )
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = eType == E_QUERY || eType == E_TABLE;
                    if ( aReturn.bEnabled && SID_APP_NEW_REPORT_PRE_SEL == _nId )
                    {
                        Reference< XContentEnumerationAccess > xEnumAccess(m_xServiceFactory, UNO_QUERY);
                        aReturn.bEnabled = xEnumAccess.is();
                        if ( aReturn.bEnabled )
                        {
                            static ::rtl::OUString s_sReportDesign(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.report.pentaho.SOReportJobFactory"));
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
                    aReturn.bEnabled = sal_False;
                else
                {
                    switch ( getContainer()->getElementType() )
                    {
                    case E_QUERY:
                        aReturn.bEnabled =  ( getContainer()->getSelectionCount() > 0 )
                                        &&  ( getContainer()->isALeafSelected() );
                        break;
                    case E_TABLE:
                        aReturn.bEnabled = sal_False;
                        // there's one exception: views which support altering their underlying
                        // command can be edited in SQL view, too
                        if  (   ( getContainer()->getSelectionCount() > 0 )
                            &&  ( getContainer()->isALeafSelected() )
                            )
                        {
                            ::std::vector< ::rtl::OUString > aSelected;
                            getSelectionElementNames( aSelected );
                            OSL_ENSURE( aSelected.size() == 1, "OApplicationController::GetState: inconsistency!" );
                            if ( aSelected.size() == 1 )
                                if ( impl_isAlterableView_nothrow( aSelected[0] ) )
                                    aReturn.bEnabled = sal_True;
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
            {
                DATASOURCE_TYPE eType = m_aTypeCollection.getType(::comphelper::getString(m_xDataSource->getPropertyValue(PROPERTY_URL)));
                aReturn.bEnabled = DST_EMBEDDED_HSQLDB != eType;
            }
            break;
            case SID_DB_APP_DSRELDESIGN:
                aReturn.bEnabled = sal_True;
                break;
            case SID_DB_APP_TABLEFILTER:
                aReturn.bEnabled = !isDataSourceReadOnly();
                break;
            case SID_DB_APP_REFRESH_TABLES:
                aReturn.bEnabled = getContainer()->getElementType() == E_TABLE && isConnected();
                break;
            case SID_DB_APP_DSPROPS:
                aReturn.bEnabled = m_xDataSource.is();
                if ( aReturn.bEnabled )
                {
                    DATASOURCE_TYPE eType = m_aTypeCollection.getType(::comphelper::getString(m_xDataSource->getPropertyValue(PROPERTY_URL)));
                    aReturn.bEnabled = DST_EMBEDDED_HSQLDB != eType && DST_MOZILLA != eType && DST_EVOLUTION != eType && DST_KAB != eType && DST_MACAB != eType && DST_OUTLOOK != eType && DST_OUTLOOKEXP != eType;
                }
                break;
            case SID_DB_APP_DSCONNECTION_TYPE:
                aReturn.bEnabled = !isDataSourceReadOnly() && m_xDataSource.is();
                if ( aReturn.bEnabled )
                {
                    DATASOURCE_TYPE eType = m_aTypeCollection.getType(::comphelper::getString(m_xDataSource->getPropertyValue(PROPERTY_URL)));
                    aReturn.bEnabled = DST_EMBEDDED_HSQLDB != eType;
                }
                break;
            case SID_DB_APP_DSADVANCED_SETTINGS:
                aReturn.bEnabled = m_xDataSource.is();
                if ( aReturn.bEnabled )
                {
                    DATASOURCE_TYPE eType = m_aTypeCollection.getType( ::comphelper::getString( m_xDataSource->getPropertyValue( PROPERTY_URL ) ) );
                    aReturn.bEnabled = AdvancedSettingsDialog::doesHaveAnyAdvancedSettings( eType );
                }
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
                aReturn.bEnabled = sal_True;
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
                aReturn.bEnabled = sal_True;
                aReturn.bChecked = getContainer()->getPreviewMode() == E_DOCUMENT;
                break;
            case ID_BROWSER_UNDO:
                aReturn.bEnabled = sal_False;
                break;
            case SID_MAIL_SENDDOC:
                aReturn.bEnabled = sal_True;
                break;
            case SID_DB_APP_SENDREPORTASMAIL:
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = E_REPORT == eType && getContainer()->getSelectionCount() > 0 && getContainer()->isALeafSelected();
                }
                break;
            case SID_DB_APP_SENDREPORTTOWRITER:
            case SID_DB_APP_DBADMIN:
                aReturn.bEnabled = sal_False;
                break;
            case SID_DB_APP_STATUS_TYPE:
                aReturn.bEnabled = m_xDataSource.is();
                if ( aReturn.bEnabled )
                {
                    DATASOURCE_TYPE eType = m_aTypeCollection.getType(::comphelper::getString(m_xDataSource->getPropertyValue(PROPERTY_URL)));
                    ::rtl::OUString sDSTypeName = m_aTypeCollection.getTypeDisplayName(eType);
                    aReturn.sTitle = sDSTypeName;
                }
                break;
            case SID_DB_APP_STATUS_DBNAME:
                aReturn.bEnabled = m_xDataSource.is();
                if ( aReturn.bEnabled )
                {
                    ::rtl::OUString sURL;
                    m_xDataSource->getPropertyValue(PROPERTY_URL) >>= sURL;
                    DATASOURCE_TYPE eType = m_aTypeCollection.getType( sURL );

                    String sDatabaseName;
                    String sHostName;
                    sal_Int32 nPortNumber( -1 );

                    m_aTypeCollection.extractHostNamePort( sURL, sDatabaseName, sHostName, nPortNumber );

                    if ( !sDatabaseName.Len() )
                        sDatabaseName = m_aTypeCollection.cutPrefix( sURL );
                    if ( m_aTypeCollection.isFileSystemBased(eType) )
                    {
                        sDatabaseName = SvtPathOptions().SubstituteVariable( sDatabaseName );
                        if ( sDatabaseName.Len() )
                        {
                            ::svt::OFileNotation aFileNotation(sDatabaseName);
                            // set this decoded URL as text
                            sDatabaseName = aFileNotation.get(::svt::OFileNotation::N_SYSTEM);
                        }
                    }

                    if ( sDatabaseName.Len() == 0 )
                        sDatabaseName = m_aTypeCollection.getTypeDisplayName( eType );

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
                    ::rtl::OUString sURL;
                    m_xDataSource->getPropertyValue( PROPERTY_URL ) >>= sURL;

                    String sHostName, sDatabaseName;
                    sal_Int32 nPortNumber = -1;
                    m_aTypeCollection.extractHostNamePort( sURL, sDatabaseName, sHostName, nPortNumber );
                    aReturn.sTitle = sHostName;
                }
                break;
            default:
                aReturn = OApplicationController_CBASE::GetState(_nId);
        }
    }
    catch(const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void OApplicationController::Execute(sal_uInt16 _nId, const Sequence< PropertyValue >& aArgs)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
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
                                ::std::vector< ::rtl::OUString > aList;
                                getSelectionElementNames( aList );
                                if ( !aList.empty() )
                                    m_aTableCopyHelper.SetTableNameForAppend( *aList.begin() );
                                else
                                    m_aTableCopyHelper.ResetTableNameForAppend();

                                m_aTableCopyHelper.pasteTable( rTransferData , getDatabaseName(), ensureConnection() );
                            }
                            break;

                        case E_QUERY:
                            if ( rTransferData.HasFormat(SOT_FORMATSTR_ID_DBACCESS_QUERY) )
                                paste( E_QUERY, ODataAccessObjectTransferable::extractObjectDescriptor( rTransferData ) );
                            break;
                        default:
                            {
                                ::std::vector< ::rtl::OUString> aList;
                                getSelectionElementNames(aList);
                                ::rtl::OUString sFolderNameToInsertInto;
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
                                paste( eType, OComponentTransferable::extractComponentDescriptor( rTransferData ), sFolderNameToInsertInto );
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
                        ::std::auto_ptr<SfxAbstractPasteDialog> pDlg(pFact->CreatePasteDialog( getView() ));
                        ::std::vector<SotFormatStringId> aFormatIds;
                        getSupportedFormats(getContainer()->getElementType(),aFormatIds);
                        const ::std::vector<SotFormatStringId>::iterator aEnd = aFormatIds.end();
                        ::rtl::OUString sEmpty;
                        for (::std::vector<SotFormatStringId>::iterator aIter = aFormatIds.begin();aIter != aEnd; ++aIter)
                            pDlg->Insert(*aIter,sEmpty);

                        const TransferableDataHelper& rClipboard = getViewClipboard();
                        pasteFormat(pDlg->GetFormat(rClipboard.GetTransferable()));
                    }
                    else
                    {
                        const PropertyValue* pIter = aArgs.getConstArray();
                        const PropertyValue* pEnd  = pIter + aArgs.getLength();
                        for( ; pIter != pEnd ; ++pIter)
                        {
                            if ( pIter->Name.equalsAscii("FormatStringId") )
                            {
                                SotFormatStringId nFormatId = 0;
                                if ( pIter->Value >>= nFormatId )
                                    pasteFormat(nFormatId);
                                break;
                            }
                        }
                    }
                }
                break;
            case SID_OPENDOC:
            case SID_HELP_INDEX:
                {
                    Reference < XDispatchProvider > xProv( getFrame(), UNO_QUERY );
                    if ( xProv.is() )
                    {
                        URL aURL;
                        switch(_nId)
                        {
                            case SID_HELP_INDEX:
                                aURL.Complete = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:HelpIndex"));
                                break;
                            case SID_OPENDOC:
                                aURL.Complete = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Open"));
                                break;
                        }

                        if ( m_xUrlTransformer.is() )
                            m_xUrlTransformer->parseStrict( aURL );
                        Reference < XDispatch > xDisp = xProv->queryDispatch( aURL, String(), 0 );
                        if ( xDisp.is() )
                            xDisp->dispatch( aURL, Sequence < PropertyValue >() );
                    }
                }
                break;
            case ID_BROWSER_SAVEDOC:
                {
                    Reference<XStorable> xStore(m_xModel,UNO_QUERY);
                    if ( xStore.is() )
                        xStore->store();
                }
                break;
            case ID_BROWSER_SAVEASDOC:
                {
                    WinBits nBits(WB_STDMODAL|WB_SAVEAS);
                    ::rtl::OUString sUrl;
                    if ( m_xModel.is() )
                        sUrl = m_xModel->getURL();
                    if ( !sUrl.getLength() )
                        sUrl = SvtPathOptions().GetWorkPath();

                    ::sfx2::FileDialogHelper aFileDlg( com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION,static_cast<sal_uInt32>(nBits) ,getView());
                    aFileDlg.SetDisplayDirectory( sUrl );

                    const SfxFilter* pFilter = getStandardDatabaseFilter();
                    if ( pFilter )
                    {
                        aFileDlg.AddFilter(pFilter->GetUIName(),pFilter->GetDefaultExtension());
                        aFileDlg.SetCurrentFilter(pFilter->GetUIName());
                    }

                    if ( aFileDlg.Execute() == ERRCODE_NONE )
                    {
                        Reference<XStorable> xStore(m_xModel,UNO_QUERY);
                        if ( xStore.is() )
                        {
                            INetURLObject aURL( aFileDlg.GetPath() );
                            if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                            {
                                xStore->storeAsURL(aURL.GetMainURL( INetURLObject::NO_DECODE ),Sequence<PropertyValue>());
                                m_sDatabaseName = ::rtl::OUString();
                                /*updateTitle();*/
                                m_bCurrentlyModified = sal_False;
                                InvalidateFeature(ID_BROWSER_SAVEDOC);
                                if ( getContainer()->getElementType() == E_NONE )
                                {
                                    getContainer()->selectContainer(E_NONE);
                                    getContainer()->selectContainer(E_TABLE);
                                }
                            }
                        }
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
                    sal_Bool bAutoPilot = sal_False;
                    sal_Bool bSQLView = sal_False;

                    switch( _nId )
                    {
                        case SID_DB_FORM_NEW_PILOT:
                        case SID_FORM_CREATE_REPWIZ_PRE_SEL:
                            bAutoPilot = sal_True;
                            // run through
                        case SID_APP_NEW_FORM:
                            eType = E_FORM;
                            break;
                        case ID_DOCUMENT_CREATE_REPWIZ:
                        case SID_REPORT_CREATE_REPWIZ_PRE_SEL:
                            bAutoPilot = sal_True;
                            // run through
                        case SID_APP_NEW_REPORT:
                        case SID_APP_NEW_REPORT_PRE_SEL:
                            eType = E_REPORT;
                            break;
                        case ID_APP_NEW_QUERY_AUTO_PILOT:
                            bAutoPilot = sal_True;
                            eType = E_QUERY;
                            break;
                        case ID_NEW_QUERY_SQL:
                            bSQLView = sal_True;
                            // run through
                        case ID_NEW_QUERY_DESIGN:
                            eType = E_QUERY;
                            break;
                         case ID_NEW_TABLE_DESIGN_AUTO_PILOT:
                             bAutoPilot = sal_True;
                             // run through
                        case ID_NEW_TABLE_DESIGN:
                            break;
                        default:
                            OSL_ENSURE(0,"illegal switch call!");
                    }
                    if ( bAutoPilot )
                        getContainer()->PostUserEvent( LINK( this, OApplicationController, OnCreateWithPilot ), reinterpret_cast< void* >( eType ) );
                    else
                        newElement( eType, bSQLView );
                }
                break;
            case SID_APP_NEW_FOLDER:
                {
                    ElementType eType = getContainer()->getElementType();
                    ::rtl::OUString sName = getContainer()->getQualifiedName( NULL );
                    insertHierachyElement(eType,sName);
                }
                break;
            case ID_NEW_VIEW_DESIGN:
            case SID_DB_NEW_VIEW_SQL:
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                    {
                        QueryDesigner aDesigner( getORB(), this, getFrame(), true, SID_DB_NEW_VIEW_SQL == _nId );

                        Reference< XDataSource > xDataSource( m_xDataSource, UNO_QUERY );
                        Reference< XComponent > xComponent( aDesigner.createNew( xDataSource ), UNO_QUERY );
                        addDocumentListener( xComponent, NULL );
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
                doAction(_nId,OLinkedDocumentsAccess::OPEN_DESIGN);
                break;
            case SID_DB_APP_OPEN:
            case SID_DB_APP_TABLE_OPEN:
            case SID_DB_APP_QUERY_OPEN:
            case SID_DB_APP_FORM_OPEN:
            case SID_DB_APP_REPORT_OPEN:
                doAction(_nId,OLinkedDocumentsAccess::OPEN_NORMAL);
                break;
            case SID_DB_APP_CONVERTTOVIEW:
                doAction(_nId,OLinkedDocumentsAccess::OPEN_NORMAL);
                break;
            case SID_SELECTALL:
                getContainer()->selectAll();
                InvalidateAll();
                break;
            case SID_DB_APP_DSRELDESIGN:
                if ( !impl_activateSubFrame_throw(::rtl::OUString(),SID_DB_APP_DSRELDESIGN,OLinkedDocumentsAccess::OPEN_DESIGN) )
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                    {
                        RelationDesigner aDesigner( getORB(), this, m_aCurrentFrame.getFrame() );
                        Reference< XDataSource > xDataSource( m_xDataSource, UNO_QUERY );
                        Reference< XComponent > xComponent( aDesigner.createNew( xDataSource ), UNO_QUERY );
                        addDocumentListener( xComponent, NULL );
                        m_aSpecialSubFrames.insert(TFrames::value_type(::rtl::OUString(),
                            TTypeFrame(TTypeOpenMode(SID_DB_APP_DSRELDESIGN,OLinkedDocumentsAccess::OPEN_DESIGN),xComponent)));
                    } // if ( xConnection.is() )
                }
                break;
            case SID_DB_APP_DSUSERADMIN:
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                        openDialog(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.UserAdministrationDialog")));
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
                openDialog(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.AdvancedDatabaseSettingsDialog")));
                askToReconnect();
                break;
            case SID_DB_APP_DSCONNECTION_TYPE:
                openDialog(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.DataSourceTypeChangeDialog")));
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
                getContainer()->selectContainer(E_TABLE);
                break;
            case SID_DB_APP_VIEW_QUERIES:
                getContainer()->selectContainer(E_QUERY);
                break;
            case SID_DB_APP_VIEW_FORMS:
                getContainer()->selectContainer(E_FORM);
                break;
            case SID_DB_APP_VIEW_REPORTS:
                getContainer()->selectContainer(E_REPORT);
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
                    if ( aSendMail.AttachDocument(rtl::OUString(),getModel(), rtl::OUString()) == SfxMailModel::SEND_MAIL_OK )
                        aSendMail.Send( getFrame() );
                }
                break;
            case SID_DB_APP_SENDREPORTASMAIL:
                doAction(_nId,OLinkedDocumentsAccess::OPEN_FORMAIL);
                break;
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    InvalidateFeature(_nId);
}
// -----------------------------------------------------------------------------
void OApplicationController::describeSupportedFeatures()
{
    OApplicationController_CBASE::describeSupportedFeatures();

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
    implDescribeSupportedFeature( ".uno:DBNewReportWithPreSelection",
                                                             SID_APP_NEW_REPORT_PRE_SEL,CommandGroup::APPLICATION );
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
    implDescribeSupportedFeature( ".uno:DBDSImport",        SID_DB_APP_DSIMPORT, CommandGroup::INTERNAL);
    implDescribeSupportedFeature( ".uno:DBDSExport",        SID_DB_APP_DSEXPORT, CommandGroup::INTERNAL);
    implDescribeSupportedFeature( ".uno:DBDBAdmin",         SID_DB_APP_DBADMIN, CommandGroup::INTERNAL);

    // status info
    implDescribeSupportedFeature( ".uno:DBStatusType",      SID_DB_APP_STATUS_TYPE, CommandGroup::INTERNAL);
    implDescribeSupportedFeature( ".uno:DBStatusDBName",    SID_DB_APP_STATUS_DBNAME, CommandGroup::INTERNAL);
    implDescribeSupportedFeature( ".uno:DBStatusUserName",  SID_DB_APP_STATUS_USERNAME, CommandGroup::INTERNAL);
    implDescribeSupportedFeature( ".uno:DBStatusHostName",  SID_DB_APP_STATUS_HOSTNAME, CommandGroup::INTERNAL);
}
// -----------------------------------------------------------------------------
OApplicationView*   OApplicationController::getContainer() const
{
    return static_cast< OApplicationView* >( getView() );
}
// -----------------------------------------------------------------------------
void OApplicationController::onCreationClick(const ::rtl::OUString& _sCommand)
{
    URL aCommand;
    aCommand.Complete = _sCommand;
    executeChecked(aCommand,Sequence<PropertyValue>());
}
// -----------------------------------------------------------------------------
// ::com::sun::star::container::XContainerListener
void SAL_CALL OApplicationController::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< XContainer > xContainer(_rEvent.Source, UNO_QUERY);
    if ( ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xContainer) != m_aCurrentContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        if ( getContainer() )
        {
            ::rtl::OUString sName;
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
// -----------------------------------------------------------------------------
void SAL_CALL OApplicationController::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< XContainer > xContainer(_rEvent.Source, UNO_QUERY);
    if ( ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xContainer) != m_aCurrentContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        ::rtl::OUString sName;
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
                        sName = xContent->getIdentifier()->getContentIdentifier() + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sName;
                    }
                }
                break;
            default:
                break;
        }
        getContainer()->elementRemoved(eType,sName);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OApplicationController::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< XContainer > xContainer(_rEvent.Source, UNO_QUERY);
    if ( ::std::find(m_aCurrentContainers.begin(),m_aCurrentContainers.end(),xContainer) != m_aCurrentContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        ::rtl::OUString sName;
        try
        {
            _rEvent.Accessor >>= sName;
            Reference<XConnection> xConnection;
            Reference<XPropertySet> xProp(_rEvent.Element,UNO_QUERY);
            ::rtl::OUString sNewName;

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
                            sName = xContent->getIdentifier()->getContentIdentifier() + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sName;
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
            OSL_ENSURE(0,"Exception catched");
        }
    }
}
namespace
{
    ::rtl::OUString lcl_getToolBarResource(ElementType _eType)
    {
        ::rtl::OUString sToolbar;
        switch(_eType)
        {
            case E_TABLE:
                sToolbar = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/tableobjectbar" ));
                break;
            case E_QUERY:
                sToolbar = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/queryobjectbar" ));
                break;
            case E_FORM:
                sToolbar = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/formobjectbar" ));
                break;
            case E_REPORT:
                sToolbar = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/reportobjectbar" ));
                break;
            case E_NONE:
                break;
            default:
                OSL_ENSURE(0,"Invalid ElementType!");
                break;
        }
        return sToolbar;
    }
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::onContainerSelect(ElementType _eType)
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");

    if ( m_eCurrentType != _eType && _eType != E_NONE )
    {
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
                    return sal_False;
                }
            }
            catch( const Exception& )
            {
                return sal_False;
            }
        }
        Reference< XLayoutManager > xLayoutManager = getLayoutManager( getFrame() );
        if ( xLayoutManager.is() )
        {
            ::rtl::OUString sToolbar = lcl_getToolBarResource(_eType);
            ::rtl::OUString sDestroyToolbar = lcl_getToolBarResource(m_eCurrentType);

            xLayoutManager->lock();
            xLayoutManager->destroyElement( sDestroyToolbar );
            if ( sToolbar.getLength() )
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

        InvalidateAll();
        EventObject aEvent(*this);
        m_aSelectionListeners.forEach<XSelectionChangeListener>(
            ::boost::bind(&XSelectionChangeListener::selectionChanged,_1,boost::cref(aEvent)));
    }
    m_eCurrentType = _eType;

    return sal_True;
}
// -----------------------------------------------------------------------------
bool OApplicationController::onEntryDoubleClick(SvTreeListBox* _pTree)
{
    OSL_ENSURE(_pTree != NULL,"Who called me without a svtreelsiboc! ->GPF ");
    if ( getContainer() && getContainer()->isLeaf(_pTree->GetHdlEntry()) )
    {
        try
        {
            openElement(
                getContainer()->getQualifiedName( _pTree->GetHdlEntry() ),
                getContainer()->getElementType(),
                OLinkedDocumentsAccess::OPEN_NORMAL
            );
            return true;    // handled
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Could not open element!");
        }
    }
    return false;   // not handled
}
// -----------------------------------------------------------------------------
bool OApplicationController::impl_isAlterableView_nothrow( const ::rtl::OUString& _rTableOrViewName ) const
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

// -----------------------------------------------------------------------------
Reference< XComponent > OApplicationController::openElement(const ::rtl::OUString& _sName, ElementType _eType,
    OLinkedDocumentsAccess::EOpenMode _eOpenMode, sal_uInt16 _nInstigatorCommand )
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
    Reference< XComponent > xRet;
    if ( _eOpenMode == OLinkedDocumentsAccess::OPEN_DESIGN )
    {
        // OJ: http://www.openoffice.org/issues/show_bug.cgi?id=30382
        getContainer()->showPreview(NULL);
    }

    switch ( _eType )
    {
    case E_REPORT:
    case E_FORM:
    {
        ::std::auto_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
        Reference< XComponent > xDefinition;
        xRet.set(aHelper->open(_sName, xDefinition,_eOpenMode),UNO_QUERY);
        if (_eOpenMode == OLinkedDocumentsAccess::OPEN_DESIGN || _eType == E_FORM )
        {
        //  // LLA: close only if in EDIT mode
            addDocumentListener(xRet,xDefinition);
        }
    }
    break;

    case E_QUERY:
    case E_TABLE:
    {
        if ( !impl_activateSubFrame_throw(_sName,_eType,_eOpenMode) )
        {

            SharedConnection xConnection( ensureConnection() );
            if ( !xConnection.is() )
                break;

            ::std::auto_ptr< DatabaseObjectView > pDesigner;
            Sequence < PropertyValue > aArgs;
            Any aDataSource;
            if ( _eOpenMode == OLinkedDocumentsAccess::OPEN_DESIGN )
            {
                sal_Bool bQuerySQLMode =( _nInstigatorCommand == SID_DB_APP_EDIT_SQL_VIEW );

                if ( _eType == E_TABLE )
                {
                    if ( impl_isAlterableView_nothrow( _sName ) )
                        pDesigner.reset( new QueryDesigner( getORB(), this, m_aCurrentFrame.getFrame(), true, bQuerySQLMode ) );
                    else
                        pDesigner.reset( new TableDesigner( getORB(), this, m_aCurrentFrame.getFrame() ) );
                }
                else if ( _eType == E_QUERY )
                {
                    pDesigner.reset( new QueryDesigner( getORB(), this, m_aCurrentFrame.getFrame(), false, bQuerySQLMode ) );
                }
                else if ( _eType == E_REPORT )
                {
                    pDesigner.reset( new ReportDesigner( getORB(),this, m_aCurrentFrame.getFrame() ) );
                }
                aDataSource <<= m_xDataSource;
            }
            else
            {
                pDesigner.reset( new ResultSetBrowser( getORB(), this, m_aCurrentFrame.getFrame(), _eType == E_TABLE ) );

                aArgs.realloc(1);
                aArgs[0].Name = PROPERTY_SHOWMENU;
                aArgs[0].Value <<= sal_True;

                aDataSource <<= getDatabaseName();
            }

            Reference< XComponent > xComponent( pDesigner->openExisting( aDataSource, _sName, aArgs ), UNO_QUERY );
            addDocumentListener( xComponent, NULL );
            m_aSpecialSubFrames.insert(TFrames::value_type(_sName,
                            TTypeFrame(TTypeOpenMode(_eType,_eOpenMode),xComponent)));
        }
    }
    break;

    default:
        break;
    }
    return xRet;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OApplicationController, OnCreateWithPilot, void*, _pType )
{
    ElementType eType = (ElementType)reinterpret_cast< sal_IntPtr >( _pType );
    newElementWithPilot( eType );
    return 0L;
}

// -----------------------------------------------------------------------------
void OApplicationController::newElementWithPilot( ElementType _eType )
{
    OSL_ENSURE( getContainer(), "OApplicationController::newElementWithPilot: without a view?" );

    switch ( _eType )
    {
        case E_REPORT:
        case E_FORM:
        {
            ::std::auto_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
            if ( aHelper->isConnected() )
            {
                sal_Int32 nCommandType = -1;
                const ::rtl::OUString sName(getCurrentlySelectedName(nCommandType));
                Reference< XComponent > xComponent,xDefinition;
                if ( E_REPORT == _eType )
                    xComponent = aHelper->newReportWithPilot(xDefinition,nCommandType,sName);
                else
                    xComponent = aHelper->newFormWithPilot(xDefinition,nCommandType,sName);

                addDocumentListener(xComponent,xDefinition);
            }
        }
        break;
        case E_QUERY:
        case E_TABLE:
         {
             ::std::auto_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
            if ( aHelper->isConnected() )
            {
                Reference< XComponent > xComponent,xDefinition;
                 if ( E_QUERY == _eType )
                     xComponent = aHelper->newQueryWithPilot( );
                 else
                     xComponent = aHelper->newTableWithPilot( );
                addDocumentListener(xComponent,xDefinition);
             }
         }
         break;
        case E_NONE:
            break;
    }
}

// -----------------------------------------------------------------------------
void OApplicationController::newElement( ElementType _eType, sal_Bool _bSQLView )
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");

    switch ( _eType )
    {
        case E_FORM:
        case E_REPORT:
            {
                ::std::auto_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
                Reference< XComponent > xComponent,xDefinition;
                sal_Int32 nCommandType = -1;
                const ::rtl::OUString sName(getCurrentlySelectedName(nCommandType));
                xComponent = aHelper->newDocument(_eType == E_FORM ? ID_FORM_NEW_TEXT : ID_REPORT_NEW_TEXT,xDefinition,nCommandType,sName);
                addDocumentListener(xComponent,xDefinition);
            }
            break;
        case E_QUERY:
        case E_TABLE:

            {
                ::std::auto_ptr< DatabaseObjectView > pDesigner;
                SharedConnection xConnection( ensureConnection() );
                if ( xConnection.is() )
                {
                    if ( _eType == E_TABLE )
                    {
                        pDesigner.reset( new TableDesigner( getORB(), this, getFrame() ) );
                    }
                    else if ( _eType == E_QUERY )
                    {
                        pDesigner.reset( new QueryDesigner( getORB(), this, getFrame(), false, _bSQLView ) );
                    }
                    else
                        pDesigner.reset( new ReportDesigner( getORB(), this, getFrame() ) );

                    Reference< XDataSource > xDataSource( m_xDataSource, UNO_QUERY );
                    Reference< XComponent > xComponent( pDesigner->createNew( xDataSource ), UNO_QUERY );
                    addDocumentListener(xComponent,NULL);
                }
            }
            break;
        default:
            break;
    }
}
// -----------------------------------------------------------------------------
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
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched!");
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::renameEntry()
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
    ::std::vector< ::rtl::OUString> aList;
    getSelectionElementNames(aList);

    Reference< XNameAccess > xContainer = getElements(getContainer()->getElementType());
    OSL_ENSURE(aList.size() == 1,"Invalid rename call here. More than one element!");
  if ( aList.empty() )
    return;

    try
    {
        if ( xContainer.is() )
        {
            ::std::auto_ptr< IObjectNameCheck > pNameChecker;
            ::std::auto_ptr< OSaveAsDlg > aDialog;

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
                            String sLabel;
                            if ( eType == E_FORM )
                                sLabel = String(ModuleRes( STR_FRM_LABEL ));
                            else
                                sLabel = String(ModuleRes( STR_RPT_LABEL ));

                            ::rtl::OUString sName = *aList.begin();
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
                                pNameChecker.reset( new HierarchicalNameCheck( xHNames.get(), String() ) );
                                aDialog.reset( new OSaveAsDlg(
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
                        aDialog.reset( new OSaveAsDlg(
                            getView(), nCommandType, getORB(), getConnection(),
                                *aList.begin(), *pNameChecker, SAD_TITLE_RENAME ) );
                    }
                    break;
                default:
                    break;
            }

            if ( xRename.is() && aDialog.get() )
            {

                sal_Bool bTryAgain = sal_True;
                while( bTryAgain )
                {
                    if ( aDialog->Execute() == RET_OK )
                    {
                        try
                        {
                            ::rtl::OUString sNewName;
                            if ( eType == E_TABLE )
                            {
                                ::rtl::OUString sName = aDialog->getName();
                                ::rtl::OUString sCatalog = aDialog->getCatalog();
                                ::rtl::OUString sSchema  = aDialog->getSchema();

                                sNewName = ::dbtools::composeTableName( m_xMetaData, sCatalog, sSchema, sName, sal_False, ::dbtools::eInDataManipulation );
                            }
                            else
                                sNewName = aDialog->getName();

                            ::rtl::OUString sOldName = *aList.begin();
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

                            bTryAgain = sal_False;
                        }
                        catch(const SQLException& e)
                        {
                            showError(SQLExceptionInfo(e));

                        }
                        catch(const ElementExistException& e)
                        {
                            static ::rtl::OUString sStatus = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000"));
                            String sMsg = String( ModuleRes( STR_NAME_ALREADY_EXISTS ) );
                            sMsg.SearchAndReplace('#',e.Message);
                            showError(SQLExceptionInfo(SQLException(sMsg, e.Context, sStatus, 0, Any())));
                        }
                        catch(const Exception& )
                        {
                            OSL_ENSURE(0,"Exception catched!");
                        }
                    }
                    else
                        bTryAgain = sal_False;
                }
            }
        }
    }
    catch(const Exception& )
    {
        OSL_ENSURE(0,"Exception catched!");
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::onEntryDeSelect(SvTreeListBox* /*_pTree*/)
{
    InvalidateAll();
    EventObject aEvent(*this);
    m_aSelectionListeners.forEach<XSelectionChangeListener>(
        ::boost::bind(&XSelectionChangeListener::selectionChanged,_1,boost::cref(aEvent)));
}
// -----------------------------------------------------------------------------
void OApplicationController::onEntrySelect(SvLBoxEntry* _pEntry)
{
    InvalidateAll();

    OApplicationView* pView = getContainer();
    if ( pView )
    {
        const ElementType eType = pView->getElementType();
          if ( _pEntry && pView->isALeafSelected() )
        {
            const ::rtl::OUString sName = pView->getQualifiedName( _pEntry );
            selectEntry(eType,sName);
        }

        EventObject aEvent(*this);
        m_aSelectionListeners.forEach<XSelectionChangeListener>(
            ::boost::bind(&XSelectionChangeListener::selectionChanged,_1,boost::cref(aEvent)));
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::selectEntry(const ElementType _eType,const ::rtl::OUString& _sName)
{
    OApplicationView* pView = getContainer();
    Reference< XContent> xContent;
    try
    {
        switch( _eType )
        {
            case E_FORM:
            case E_REPORT:
                if ( _sName.getLength() )
                {
                    Reference< XHierarchicalNameAccess > xContainer(getElements(_eType),UNO_QUERY);
                    if ( xContainer.is() && xContainer->hasByHierarchicalName(_sName) )
                        xContent.set(xContainer->getByHierarchicalName(_sName),UNO_QUERY);
                }
                break;
            case E_QUERY:
                if ( pView->isPreviewEnabled() )
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                        pView->showPreview(getDatabaseName(),xConnection,_sName,sal_False);
                }
                return;
            case E_TABLE:
                {
                    SharedConnection xConnection( ensureConnection() );
                    if ( xConnection.is() )
                    {
                        pView->showPreview(getDatabaseName(),xConnection,_sName,_eType == E_TABLE);
                        return;
                    }
                }
                break;
            default:
                break;
        }
    }
    catch(SQLException e)
    {
        showError(e);
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Exception catched while previewing!");
    }

    pView->showPreview(xContent);
}

//------------------------------------------------------------------------------
IMPL_LINK( OApplicationController, OnClipboardChanged, void*, EMPTYARG )
{
    return OnInvalidateClipboard( NULL );
}
//------------------------------------------------------------------------------
IMPL_LINK(OApplicationController, OnInvalidateClipboard, void*, EMPTYARG)
{
    InvalidateFeature(ID_BROWSER_CUT);
    InvalidateFeature(ID_BROWSER_COPY);
    InvalidateFeature(ID_BROWSER_PASTE);
    InvalidateFeature(SID_DB_APP_PASTE_SPECIAL);
    return 0L;
}
// -----------------------------------------------------------------------------
void OApplicationController::onCutEntry(SvLBoxEntry* /*_pEntry*/)
{
}
// -----------------------------------------------------------------------------
void OApplicationController::onCopyEntry(SvLBoxEntry* /*_pEntry*/)
{
    Execute(ID_BROWSER_COPY,Sequence<PropertyValue>());
}
// -----------------------------------------------------------------------------
void OApplicationController::onPasteEntry(SvLBoxEntry* /*_pEntry*/)
{
    Execute(ID_BROWSER_PASTE,Sequence<PropertyValue>());
}
// -----------------------------------------------------------------------------
void OApplicationController::onDeleteEntry(SvLBoxEntry* /*_pEntry*/)
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
            OSL_ENSURE(0,"Invalid ElementType!");
            break;
    }
    executeChecked(nId,Sequence<PropertyValue>());
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::requestContextMenu( const CommandEvent& /*_rEvent*/ )
{
    return sal_False;
}

// -----------------------------------------------------------------------------
sal_Bool OApplicationController::requestQuickHelp( const SvLBoxEntry* /*_pEntry*/, String& /*_rText*/ ) const
{
    return sal_False;
}

// -----------------------------------------------------------------------------
sal_Bool OApplicationController::requestDrag( sal_Int8 /*_nAction*/, const Point& /*_rPosPixel*/ )
{
    TransferableHelper* pTransfer = NULL;
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
            OSL_ENSURE(0,"Exception catched!");
        }
    }

    return NULL != pTransfer;
}
// -----------------------------------------------------------------------------
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
            if(::std::find_if(_rFlavors.begin(),_rFlavors.end(),TAppSupportedSotFunctor(eType,sal_True)) != _rFlavors.end())
                return DND_ACTION_COPY;
            if ( eType == E_FORM || eType == E_REPORT )
            {
                sal_Int8 nAction = OComponentTransferable::canExtractComponentDescriptor(_rFlavors,eType == E_FORM) ? DND_ACTION_COPY : DND_ACTION_NONE;
                if ( nAction != DND_ACTION_NONE )
                {
                    SvLBoxEntry* pHitEntry = pView->getEntry(_rEvt.maPosPixel);
                    ::rtl::OUString sName;
                    if ( pHitEntry )
                    {
                        sName = pView->getQualifiedName( pHitEntry );
                        if ( sName.getLength() )
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
                    /*else
                        nAction = nActionAskedFor & DND_ACTION_COPYMOVE;
                    */
                }
                return nAction;
            }
        }
    }

    return DND_ACTION_NONE;
}
// -----------------------------------------------------------------------------
sal_Int8 OApplicationController::executeDrop( const ExecuteDropEvent& _rEvt )
{
    OApplicationView* pView = getContainer();
    if ( !pView || pView->getElementType() == E_NONE )
    {
        DBG_ERROR("OApplicationController::executeDrop: what the hell did queryDrop do?");
            // queryDrop shoud not have allowed us to reach this situation ....
        return DND_ACTION_NONE;
    }

    // a TransferableDataHelper for accessing the dropped data
    TransferableDataHelper aDroppedData(_rEvt.maDropEvent.Transferable);


    // reset the data of the previous async drop (if any)
    if ( m_nAsyncDrop )
        Application::RemoveUserEvent(m_nAsyncDrop);


    m_nAsyncDrop = 0;
    m_aAsyncDrop.aDroppedData.clear();
    m_aAsyncDrop.nType          = pView->getElementType();
    m_aAsyncDrop.nAction        = _rEvt.mnAction;
    m_aAsyncDrop.bError         = sal_False;
    m_aAsyncDrop.bHtml          = sal_False;
    m_aAsyncDrop.aUrl           = ::rtl::OUString();


    // loop through the available formats and see what we can do ...
    // first we have to check if it is our own format, if not we have to copy the stream :-(
    if ( ODataAccessObjectTransferable::canExtractObjectDescriptor(aDroppedData.GetDataFlavorExVector()) )
    {
        m_aAsyncDrop.aDroppedData   = ODataAccessObjectTransferable::extractObjectDescriptor(aDroppedData);

        // asyncron because we some dialogs and we aren't allowed to show them while in D&D
        m_nAsyncDrop = Application::PostUserEvent(LINK(this, OApplicationController, OnAsyncDrop));
        return DND_ACTION_COPY;
    }
    else if ( OComponentTransferable::canExtractComponentDescriptor(aDroppedData.GetDataFlavorExVector(),m_aAsyncDrop.nType == E_FORM) )
    {
        m_aAsyncDrop.aDroppedData = OComponentTransferable::extractComponentDescriptor(aDroppedData);
        SvLBoxEntry* pHitEntry = pView->getEntry(_rEvt.maPosPixel);
        if ( pHitEntry )
            m_aAsyncDrop.aUrl = pView->getQualifiedName( pHitEntry );

        sal_Int8 nAction = _rEvt.mnAction;
        Reference<XContent> xContent;
        m_aAsyncDrop.aDroppedData[daComponent] >>= xContent;
        if ( xContent.is() )
        {
            ::rtl::OUString sName = xContent->getIdentifier()->getContentIdentifier();
            sal_Int32 nIndex = 0;
            sName = sName.copy(sName.getToken(0,'/',nIndex).getLength() + 1);
            if ( m_aAsyncDrop.aUrl.Len() >= sName.getLength() && 0 == sName.compareTo(m_aAsyncDrop.aUrl,sName.getLength()) )
            {
                m_aAsyncDrop.aDroppedData.clear();
                return DND_ACTION_NONE;
            }

            // check if move is allowed, if another object with the same name exists only copy is allowed
            Reference< XHierarchicalNameAccess > xContainer(getElements(m_aAsyncDrop.nType),UNO_QUERY);
            Reference<XNameAccess> xNameAccess(xContainer,UNO_QUERY);

            if ( m_aAsyncDrop.aUrl.Len() && xContainer.is() && xContainer->hasByHierarchicalName(m_aAsyncDrop.aUrl) )
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
            // asyncron because we some dialogs and we aren't allowed to show them while in D&D
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
            // asyncron because we some dialogs and we aren't allowed to show them while in D&D
            m_nAsyncDrop = Application::PostUserEvent(LINK(this, OApplicationController, OnAsyncDrop));
            return DND_ACTION_COPY;
        }
    }

    return DND_ACTION_NONE;
}
// -----------------------------------------------------------------------------
Reference< XModel >  SAL_CALL OApplicationController::getModel(void) throw( RuntimeException )
{
    return m_xModel;
}

// -----------------------------------------------------------------------------
void OApplicationController::onConnectedModel()
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

    m_aControllerConnectedEvent.Call();
}

// -----------------------------------------------------------------------------
IMPL_LINK( OApplicationController, OnFirstControllerConnected, void*, /**/ )
{
    // if we have forms or reports which contain macros/scripts, then show a warning
    // which suggests the user to migrate them to the database document

    // TODO: remove the following line. The current version is just to be able
    // to integrate an intermediate version of the CWS, which should behave as
    // if no macros in DB docs are allowed
    return 0L;
/*
    Reference< XEmbeddedScripts > xDocumentScripts( m_xModel, UNO_QUERY );
    if ( xDocumentScripts.is() )
    {
        // no need to show this warning, obviously the document supports embedding scripts
        // into itself, so there are no "old-style" forms/reports which have macros/scripts
        // themselves
        return 0L;
    }

    SQLWarning aWarning;
    aWarning.Message = String( ModuleRes( STR_SUB_DOCS_WITH_SCRIPTS ) );
    SQLException aDetail;
    aDetail.Message = String( ModuleRes( STR_SUB_DOCS_WITH_SCRIPTS_DETAIL ) );
    aWarning.NextException <<= aDetail;

    try
    {
        ::comphelper::ComponentContext aContext( getORB() );
        Sequence< Any > aArgs(1);
        aArgs[0] <<= NamedValue( PROPERTY_SQLEXCEPTION, makeAny( aWarning ) );
        Reference< XExecutableDialog > xDialog(
            aContext.createComponentWithArguments( "com.sun.star.sdb.ErrorMessageDialog", aArgs ),
            UNO_QUERY_THROW );
        xDialog->execute();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return 1L;
*/
}

// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OApplicationController::attachModel(const Reference< XModel > & _rxModel) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    Reference< XOfficeDatabaseDocument > xOfficeDoc( _rxModel, UNO_QUERY );
    if ( !xOfficeDoc.is() && _rxModel.is() )
    {
        DBG_ERROR( "OApplicationController::attachModel: invalid model!" );
        return sal_False;
    }

    DBG_ASSERT( !( m_xModel.is() && ( m_xModel != _rxModel ) ),
        "OApplicationController::attachModel: missing implementation: setting a new model while we have another one!" );
        // at least: remove as property change listener from the old model/data source

    m_xModel = _rxModel;
    if ( _rxModel.is() )
    {
        m_xDocumentModify.set( m_xModel, UNO_QUERY_THROW );
        m_aModelConnector = ModelControllerConnector( _rxModel, this );
        onConnectedModel();
    }
    else
    {
        m_xDocumentModify.clear();
        m_aModelConnector.clear();
    }

    m_xDataSource.set(xOfficeDoc.is() ? xOfficeDoc->getDataSource() : Reference<XDataSource>(),UNO_QUERY);
    if ( !m_xDataSource.is() )
        m_sDatabaseName = ::rtl::OUString();
    else
    {
        try
        {
            m_xDataSource->getPropertyValue(PROPERTY_NAME) >>= m_sDatabaseName;

            m_xDataSource->addPropertyChangeListener(PROPERTY_INFO, this);
            m_xDataSource->addPropertyChangeListener(PROPERTY_URL, this);
            m_xDataSource->addPropertyChangeListener(PROPERTY_ISPASSWORDREQUIRED, this);
            m_xDataSource->addPropertyChangeListener(PROPERTY_LAYOUTINFORMATION, this);
            m_xDataSource->addPropertyChangeListener(PROPERTY_SUPPRESSVERSIONCL, this);
            m_xDataSource->addPropertyChangeListener(PROPERTY_TABLEFILTER, this);
            m_xDataSource->addPropertyChangeListener(PROPERTY_TABLETYPEFILTER, this);
            m_xDataSource->addPropertyChangeListener(PROPERTY_USER, this);
            // to get the 'modified' for the data source
            Reference< XModifyBroadcaster >  xBroadcaster(m_xModel, UNO_QUERY);
            if ( xBroadcaster.is() )
                xBroadcaster->addModifyListener(static_cast<XModifyListener*>(this));

            Sequence<PropertyValue> aFields;
            m_xDataSource->getPropertyValue(PROPERTY_LAYOUTINFORMATION) >>= aFields;
            PropertyValue *pIter = aFields.getArray();
            PropertyValue *pEnd = pIter + aFields.getLength();
            for (; pIter != pEnd && pIter->Name != INFO_PREVIEW; ++pIter)
                ;

            if ( pIter != pEnd )
            {
                sal_Int32 nValue = 0;
                pIter->Value >>= nValue;
                m_ePreviewMode = static_cast<PreviewMode>(nValue);
            }
        }
        catch(Exception)
        {
            OSL_ENSURE( false, "OApplicationController::attachModel: caught an exception while doing the property stuff!" );
        }
    }

    return sal_True;
}
// -----------------------------------------------------------------------------
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
    catch(Exception)
    {
        OSL_ENSURE(0,"Could not listener on the container!");
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString OApplicationController::getCurrentlySelectedName(sal_Int32& _rnCommandType) const
{
    _rnCommandType = ( (getContainer()->getElementType() == E_QUERY)
                                ? CommandType::QUERY : ( (getContainer()->getElementType() == E_TABLE) ? CommandType::TABLE : -1 ));


    ::rtl::OUString sName;
    if ( _rnCommandType != -1 )
    {
        try
        {
            sName = getContainer()->getQualifiedName( NULL );
            OSL_ENSURE( sName.getLength(), "OApplicationController::newElementWithPilot: no name given!" );
        }
        catch(Exception)
        {
            OSL_ENSURE( 0, "OApplicationController::newElementWithPilot: Exception catched!" );
        }
    }
    return sName;
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OApplicationController::select( const Any& _aSelection ) throw (IllegalArgumentException, RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    Sequence< ::rtl::OUString> aSelection;
    if ( !_aSelection.hasValue() || !getView() )
    {
        getContainer()->selectElements(aSelection);
        return sal_True;
    }
    Sequence< NamedValue > aCurrentSelection;
    if ( (_aSelection >>= aCurrentSelection) && aCurrentSelection.getLength() )
    {
        ElementType eType = E_NONE;
        const NamedValue* pIter = aCurrentSelection.getConstArray();
        const NamedValue* pEnd  = pIter + aCurrentSelection.getLength();
        for(;pIter != pEnd;++pIter)
        {
            if ( pIter->Name.equalsAscii("Type") )
            {
                sal_Int32 nType = 0;
                pIter->Value >>= nType;
                if ( nType < 0 || nType > 4)
                    throw IllegalArgumentException();
                eType = static_cast<ElementType>(nType);
            }
            else if ( pIter->Name.equalsAscii("Selection") )
                pIter->Value >>= aSelection;
        }

        getContainer()->selectContainer(eType);
        getContainer()->selectElements(aSelection);
        return sal_True;
    }
    throw IllegalArgumentException();
}
// -----------------------------------------------------------------------------
Any SAL_CALL OApplicationController::getSelection(  ) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    Sequence< NamedValue > aCurrentSelection;
    if ( getContainer() )
    {
        ::std::vector< ::rtl::OUString> aList;
        getSelectionElementNames(aList);
        NamedValue aType;
        aType.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Type"));
        aType.Value <<= static_cast<sal_Int32>(getContainer()->getElementType());
        NamedValue aNames;
        aNames.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Selection"));
        if ( !aList.empty() )
            aNames.Value <<= Sequence< ::rtl::OUString>(&aList[0],aList.size());

        aCurrentSelection.realloc(2);
        aCurrentSelection[0] = aType;
        aCurrentSelection[1] = aNames;
    }

    return makeAny(aCurrentSelection);
}
// -----------------------------------------------------------------------------
void OApplicationController::impl_migrateScripts_nothrow()
{
    try
    {
        ::rtl::OUString sDialogService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sdb.application.MacroMigrationWizard" ) );
        ::comphelper::ComponentContext aContext( getORB() );
        Sequence< Any > aDialogArgs(1);
        aDialogArgs[0] <<= Reference< XOfficeDatabaseDocument >( m_xModel, UNO_QUERY_THROW );
        Reference< XExecutableDialog > xDialog(
            aContext.createComponentWithArguments( sDialogService, aDialogArgs ),
            UNO_QUERY );

        if ( !xDialog.is() )
        {
            ShowServiceNotAvailableError( getView(), sDialogService, true );
            return;
        }

        xDialog->execute();
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// -----------------------------------------------------------------------------
bool OApplicationController::impl_activateSubFrame_throw(const ::rtl::OUString& _sName,const sal_Int32 _nKind,const OLinkedDocumentsAccess::EOpenMode _eOpenMode) const
{
    bool bFound = false;
    TFrames::const_iterator aFind = m_aSpecialSubFrames.find(_sName);
    for(;aFind != m_aSpecialSubFrames.end();++aFind)
    {
        if ( aFind->second.first.first == _nKind && aFind->second.first.second == _eOpenMode )
        {
            const Reference< XFrame> xFrame(aFind->second.second,UNO_QUERY);
            if ( xFrame.is() )
            {
                Reference< awt::XTopWindow> xTopWindow(xFrame->getContainerWindow(),UNO_QUERY);
                if ( xTopWindow.is() )
                    xTopWindow->toFront();
                bFound = true;
            }
            break;
        } // if ( aFind->second.first.first == _nKind && aFind->second.first.second == _eOpenMode )
    } // while ( aFind != m_aSpecialSubFrames.end() )
    return bFound;
}
//........................................................................
}   // namespace dbaui
//........................................................................


