/*************************************************************************
 *
 *  $RCSfile: AppController.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:28:11 $
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
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XBOOKMARKSSUPPLIER_HPP_
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XRENAME_HPP_
#include <com/sun/star/sdbcx/XRename.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XVIEWSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XViewsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#include <com/sun/star/sdb/SQLContext.hpp>
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMECONTAINER_HPP_
#include <com/sun/star/container/XHierarchicalNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
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
#ifndef _SV_TOOLBOX_HXX //autogen wg. ToolBox
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _DBAUI_QUERYDESIGNACCESS_HXX_
#include "querydesignaccess.hxx"
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
#ifndef _DBAUI_DLGRENAME_HXX
#include "dlgrename.hxx"
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef _PASTEDLG_HXX
#include <so3/pastedlg.hxx>
#endif
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
#ifndef INCLUDED_SVTOOLS_HISTORYOPTIONS_HXX
#include <svtools/historyoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif

#include <algorithm>
#include <functional>

#define APP_SIZE_WIDTH  350
#define APP_SIZE_HEIGHT 250


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
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::datatransfer;

//------------------------------------------------------------------------------
const SfxFilter* OApplicationController::getStandardFilter()
{
    static const String s_sDatabaseType = String::CreateFromAscii("StarOffice XML (Base)");
    const SfxFilter* pFilter = SfxFilter::GetFilterByName( s_sDatabaseType);
    OSL_ENSURE(pFilter,"Filter: StarOffice XML (Base) could not be found!");
    return pFilter;
}
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
//--------------------------------------------------------------------
OApplicationController::OApplicationController(const Reference< XMultiServiceFactory >& _rxORB)
    :OApplicationController_CBASE( _rxORB )
    ,m_bSuspended( sal_False )
    ,m_pClipbordNotifier(NULL)
    ,m_nAsyncDrop(0)
    ,m_ePreviewMode(E_PREVIEWNONE)
    ,m_bNeedToReconnect(sal_False)
    ,m_bPreviewEnabled(sal_True)
{
    m_aRefreshMenu.SetTimeoutHdl( LINK( this, OApplicationController, OnShowRefreshDropDown ) );
    m_aRefreshMenu.SetTimeout( 300 );

    m_aPasteSpecialMenu.SetTimeoutHdl( LINK( this, OApplicationController, OnPasteSpecialDropDown ) );
    m_aPasteSpecialMenu.SetTimeout( 300 );

    m_aLastSelectedPullDownActions.insert(::std::map<sal_uInt16,sal_uInt16>::value_type(ID_APP_NEW_FORM,0));
    m_aLastSelectedPullDownActions.insert(::std::map<sal_uInt16,sal_uInt16>::value_type(SID_DB_APP_PASTE_SPECIAL,0));
    m_aLastSelectedPullDownActions.insert(::std::map<sal_uInt16,sal_uInt16>::value_type(ID_BROWSER_PASTE,0));
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
}
//--------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2(OApplicationController,OApplicationController_CBASE,OApplicationController_Base)
IMPLEMENT_FORWARD_XINTERFACE2(OApplicationController,OApplicationController_CBASE,OApplicationController_Base)
//-------------------------------------------------------------------------
void OApplicationController::disconnect(Reference< XConnection >& _xConnection)
{
    ::comphelper::disposeComponent( _xConnection );
    stopConnectionListening(_xConnection);

    InvalidateAll();
}
// -----------------------------------------------------------------------------
void OApplicationController::clearConnections()
{
    try
    {
        TDataSourceConnections::iterator aIter = m_aDataSourceConnections.begin();
        TDataSourceConnections::iterator aEnd = m_aDataSourceConnections.end();
        for (; aIter != aEnd; ++aIter)
        {
            ::comphelper::disposeComponent(aIter->second);
        }
    }
    catch(Exception)
    {}
    m_aDataSourceConnections.clear();
}
//--------------------------------------------------------------------
void SAL_CALL OApplicationController::disposing()
{
    if ( m_xCurrentContainer.is() )
        m_xCurrentContainer->removeContainerListener(this);
    ::std::for_each(m_aCurrentSubContainers.begin(),m_aCurrentSubContainers.end(),XContainerFunctor(this));
    m_aCurrentSubContainers.clear();

    m_aDocuments.clear();

    if ( getView() )
    {
        m_pClipbordNotifier->ClearCallbackLink();
        m_pClipbordNotifier->AddRemoveListener( getView(), sal_False );
        m_pClipbordNotifier->release();
        m_pClipbordNotifier = NULL;
    }

    clearConnections();
    try
    {
        m_xCurrentContainer = NULL;
        if ( m_xDataSource.is() )
        {
            Reference<XModel> xModel(m_xDataSource,UNO_QUERY);
            if ( xModel.is() )
            {
                ::rtl::OUString sUrl = xModel->getURL();
                if ( sUrl.getLength() )
                {
                    ::rtl::OUString     aFilter;
                    INetURLObject       aURL( sUrl );
                    const SfxFilter* pFilter = getStandardFilter();
                    if ( pFilter )
                        aFilter = pFilter->GetFilterName();

                    // add to svtool history options
                    SvtHistoryOptions().AppendItem( ePICKLIST,
                            aURL.GetURLNoPass( INetURLObject::NO_DECODE ),
                            aFilter,
                            getStrippedDatabaseName(),
                            ::rtl::OUString() );
                }

                xModel->disconnectController( this );
            }
            Reference < XFrame > xFrame;
            attachFrame( xFrame );

            m_xDataSource->removePropertyChangeListener(::rtl::OUString(), this);
            Reference< XModifyBroadcaster >  xBroadcaster(m_xDataSource, UNO_QUERY);
            if ( xBroadcaster.is() )
                xBroadcaster->removeModifyListener(static_cast<XModifyListener*>(this));

            m_xDataSource = NULL;
        }
    }
    catch(Exception)
    {
    }

    m_pView = NULL;
    OApplicationController_CBASE::disposing();
}

//--------------------------------------------------------------------
sal_Bool OApplicationController::Construct(Window* _pParent)
{
    //  _pParent->SetSizePixel(_pParent->LogicToPixel( Size(APP_SIZE_WIDTH,APP_SIZE_HEIGHT), MAP_APPFONT ));
    m_pView = new OApplicationView(_pParent,getORB(),this,this,this,this,this,this);
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

    if ( getContainer() && m_ePreviewMode != getContainer()->getPreviewMode() )
        getContainer()->switchPreview(m_ePreviewMode);

    if ( getContainer() && m_xDataSource.is() )
        getContainer()->setStatusInformations(m_xDataSource);

    // now that we have a view we can create the clipboard listener
    m_aSystemClipboard = TransferableDataHelper::CreateFromSystemClipboard( getView() );
    m_aSystemClipboard.StartClipboardListening( );

    m_pClipbordNotifier = new TransferableClipboardListener( LINK( this, OApplicationController, OnClipboardChanged ) );
    m_pClipbordNotifier->acquire();
    m_pClipbordNotifier->AddRemoveListener( getView(), sal_True );

    OApplicationController_CBASE::Construct( _pParent );
    getContainer()->Show();

    return sal_True;
}

//--------------------------------------------------------------------
void SAL_CALL OApplicationController::disposing(const EventObject& _rSource) throw( RuntimeException )
{
    Reference<XConnection> xCon(_rSource.Source, UNO_QUERY);
    if ( xCon.is() )
    {
        if ( getContainer()&& getContainer()->getElementType() == E_TABLE )
        {
            TDataSourceConnections::iterator aIter = m_aDataSourceConnections.begin();
            TDataSourceConnections::iterator aEnd = m_aDataSourceConnections.end();
            for (;aIter != aEnd ; ++aIter)
            {
                if ( aIter->second.is() && aIter->second == xCon )
                {
                    getContainer()->clearPages();
                    break;
                }
            }
        }
    }
    else
    {
        Reference<XComponent> xComp(_rSource.Source,UNO_QUERY);
        TDocuments::iterator aFind = ::std::find_if(m_aDocuments.begin(),m_aDocuments.end(),
            ::std::compose1(::std::bind2nd(::std::equal_to<Reference<XComponent> >(),xComp),::std::select1st<TDocuments::value_type>()));
        if ( aFind != m_aDocuments.end() )
        {
            m_aDocuments.erase(aFind);
        }
        else
            OApplicationController_CBASE::disposing( _rSource );
    }
}
//--------------------------------------------------------------------
sal_Bool SAL_CALL OApplicationController::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    m_bSuspended = bSuspend;
    if ( bSuspend && !suspendDocuments( bSuspend ))
        return sal_False;

    sal_Bool bCheck = sal_True;
    Reference<XModifiable> xModi(m_xDataSource,UNO_QUERY);
    if ( xModi.is() && xModi->isModified() )
    {
        QueryBox aQry(getView(), ModuleRes(APP_SAVEMODIFIED));
        switch (aQry.Execute())
        {
            case RET_YES:
                Execute(ID_BROWSER_SAVEDOC);
                bCheck = !xModi->isModified(); // when we save the table this must be false else some press cancel
                break;
            case RET_CANCEL:
                bCheck = sal_False;
            default:
                break;
        }
    }

    return bCheck;
}
// -----------------------------------------------------------------------------
FeatureState OApplicationController::GetState(sal_uInt16 _nId) const
{
    FeatureState aReturn;
    aReturn.bEnabled = sal_False;
    // check this first
    if ( !getContainer() )
        return aReturn;

    try
    {
        switch (_nId)
        {
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
                        aReturn.bEnabled = !isDataSourceReadOnly() && OComponentTransferable::canExtractComponentDescriptor(getViewClipboard().GetDataFlavorExVector());
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
                aReturn.bEnabled = !isDataSourceReadOnly() && m_bCurrentlyModified;
                break;
            case ID_BROWSER_SAVEASDOC:
                aReturn.bEnabled = sal_True;
                break;
            case ID_BROWSER_SORTUP:
                aReturn.bEnabled = getContainer()->isFilled() && getContainer()->getElementCount();
                aReturn.aState <<= (aReturn.bEnabled && getContainer()->isSortUp());
                break;
            case ID_BROWSER_SORTDOWN:
                aReturn.bEnabled = getContainer()->isFilled() && getContainer()->getElementCount();
                aReturn.aState <<= (aReturn.bEnabled && !getContainer()->isSortUp());
                break;

            case SID_NEWDOC:
            case ID_APP_NEW_FORM:
            case ID_DOCUMENT_CREATE_REPWIZ:
                aReturn.bEnabled = !isDataSourceReadOnly() && SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SWRITER);
                break;
            case SID_DB_APP_VIEW_TABLES:
                aReturn.bEnabled = sal_True;
                aReturn.aState <<= getContainer()->getElementType() == E_TABLE;
                break;
            case SID_DB_APP_VIEW_QUERIES:
                aReturn.bEnabled = sal_True;
                aReturn.aState <<= getContainer()->getElementType() == E_QUERY;
                break;
            case SID_DB_APP_VIEW_FORMS:
                aReturn.bEnabled = sal_True;
                aReturn.aState <<= getContainer()->getElementType() == E_FORM;
                break;
            case SID_DB_APP_VIEW_REPORTS:
                aReturn.bEnabled = sal_True;
                aReturn.aState <<= getContainer()->getElementType() == E_REPORT;
                break;
            case ID_NEW_QUERY_DESIGN:
            case ID_NEW_QUERY_SQL:
            case ID_APP_NEW_QUERY_AUTO_PILOT:
            case ID_FORM_NEW_PILOT:
                aReturn.bEnabled = !isDataSourceReadOnly();
                break;
            case ID_NEW_TABLE_DESIGN:
            case ID_NEW_VIEW_DESIGN:
            case ID_NEW_VIEW_SQL:
            case ID_NEW_TABLE_DESIGN_AUTO_PILOT:
            case ID_NEW_VIEW_DESIGN_AUTO_PILOT:
                aReturn.bEnabled = !isDataSourceReadOnly() && !isConnectionReadOnly();
                break;
            case ID_DIRECT_SQL:
                aReturn.bEnabled = !isConnectionReadOnly();
                break;
            case ID_APP_NEW_FOLDER:
                aReturn.bEnabled = !isDataSourceReadOnly() && getContainer()->getSelectionCount() <= 1;
                if ( aReturn.bEnabled )
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = eType == E_REPORT || eType == E_FORM;
                }
                break;
            case ID_FORM_NEW_PILOT_PRE_SEL:
            case ID_DOCUMENT_CREATE_REPWIZ_PRE_SEL:
                aReturn.bEnabled = !isDataSourceReadOnly()
                                    && SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SWRITER)
                                    && getContainer()->isALeafSelected();
                if ( aReturn.bEnabled )
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = eType == E_QUERY || eType == E_TABLE;
                }
                break;

            case SID_DB_APP_DELETE:
            case SID_DB_APP_RENAME:
                aReturn.bEnabled = !isDataSourceReadOnly();
                if ( aReturn.bEnabled )
                {
                    ElementType eType = getContainer()->getElementType();
                    if ( E_TABLE == eType )
                        aReturn.bEnabled = !isConnectionReadOnly() && getContainer()->isALeafSelected();

                    sal_Bool bCompareRes = sal_False;
                    if ( SID_DB_APP_DELETE == _nId )
                        bCompareRes = getContainer()->getSelectionCount() > 0;
                    else
                    {
                        bCompareRes = getContainer()->getSelectionCount() == 1;
                        if ( aReturn.bEnabled && bCompareRes && E_TABLE == eType )
                        {
                            ::std::vector< ::rtl::OUString> aList;
                            const_cast<OApplicationController*>(this)->getSelectionElementNames(aList);

                            try
                            {
                                Reference< XNameAccess > xContainer = const_cast<OApplicationController*>(this)->getElements(eType);
                                if ( aReturn.bEnabled = (xContainer.is() && xContainer->hasByName(*aList.begin())) )
                                    aReturn.bEnabled = Reference<XRename>(xContainer->getByName(*aList.begin()),UNO_QUERY).is();
                            }
                            catch(Exception&)
                            {
                                aReturn.bEnabled = sal_False;
                            }
                        }
                    }

                    aReturn.bEnabled = aReturn.bEnabled && bCompareRes;
                }
                break;
            case SID_DB_APP_SELECTALL:
                aReturn.bEnabled = getContainer()->getElementCount() > 0 && getContainer()->getSelectionCount() != getContainer()->getElementCount();
                break;
            case SID_DB_APP_EDIT:
                aReturn.bEnabled = !isDataSourceReadOnly() && getContainer()->getSelectionCount() > 0
                                    && getContainer()->isALeafSelected();
                break;
            case SID_DB_APP_OPEN:
                aReturn.bEnabled = getContainer()->getSelectionCount() > 0 && getContainer()->isALeafSelected();
                break;
            case SID_DB_APP_DSIMPORT:
                aReturn.bEnabled = sal_True;
                break;
            case SID_DB_APP_DSEXPORT:
                aReturn.bEnabled = sal_True;
                break;
            case SID_DB_APP_DSRELDESIGN:
            case SID_DB_APP_DSUSERADMIN:
                aReturn.bEnabled = sal_True;
                break;
            case SID_DB_APP_TABLEFILTER:
                aReturn.bEnabled = !isDataSourceReadOnly();
                break;
            case SID_DB_APP_REFRESH_TABLES:
                aReturn.bEnabled = getContainer()->getElementType() == E_TABLE && getActiveConnection().is();
                break;
            case SID_DB_APP_DSCONNECTION_TYPE:
                aReturn.bEnabled = !isDataSourceReadOnly();
                break;
            case SID_DB_APP_DSPROPS:
                aReturn.bEnabled = sal_True;
                break;
            case SID_DB_APP_DSADVANCED_SETTINGS:
                if ( m_xDataSource.is() )
                {
                    static ODsnTypeCollection aTypeCollection;
                    DATASOURCE_TYPE eType = aTypeCollection.getType(::comphelper::getString(m_xDataSource->getPropertyValue(PROPERTY_URL)));
                    aReturn.bEnabled = DST_LDAP != eType && DST_CALC != eType && DST_MOZILLA != eType && DST_EVOLUTION != eType && DST_OUTLOOK != eType && DST_OUTLOOKEXP != eType;
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
                        Reference<XConnection> xConnection = getActiveConnection();
                        Reference<XViewsSupplier> xViewSup(xConnection,UNO_QUERY);
                        aReturn.bEnabled = xViewSup.is() && Reference<XAppend>(xViewSup->getViews(),UNO_QUERY).is();
                    }
                }
                break;
            case SID_DB_APP_DISABLE_PREVIEW:
                aReturn.bEnabled = !isDataSourceReadOnly();
                aReturn.aState <<= getContainer()->getPreviewMode() == E_PREVIEWNONE;
                break;
            case SID_DB_APP_VIEW_DOCINFO_PREVIEW:
                {
                    ElementType eType = getContainer()->getElementType();
                    aReturn.bEnabled = !isDataSourceReadOnly() && (E_REPORT == eType || E_FORM == eType);
                    aReturn.aState <<= getContainer()->getPreviewMode() == E_DOCUMENTINFO;
                }
                break;
            case SID_DB_APP_VIEW_DOC_PREVIEW:
                aReturn.bEnabled = !isDataSourceReadOnly();
                aReturn.aState <<= getContainer()->getPreviewMode() == E_DOCUMENT;
                break;
            case SID_DB_APP_DBADMIN:
                aReturn.bEnabled = sal_False;
                break;
            default:
                aReturn = OApplicationController_CBASE::GetState(_nId);
        }
    }
    catch(Exception& e)
    {
#if DBG_UTIL
        String sMessage("OApplicationController::GetState(", RTL_TEXTENCODING_ASCII_US);
        sMessage += String::CreateFromInt32(_nId);
        sMessage.AppendAscii(") : caught an exception ! message : ");
        sMessage += (const sal_Unicode*)e.Message;
        DBG_ERROR(ByteString(sMessage, gsl_getSystemTextEncoding()).GetBuffer());
#else
        e;  // make compiler happy
#endif
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void OApplicationController::Execute(sal_uInt16 _nId)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);
    if ( !getContainer() )
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
                    TransferableDataHelper aTransferData(TransferableDataHelper::CreateFromSystemClipboard(getView()));
                    ElementType eType = getContainer()->getElementType();
                    switch( eType )
                    {
                        case E_TABLE:
                            pasteTable( aTransferData );
                            break;
                        case E_QUERY:
                            paste( E_QUERY,ODataAccessObjectTransferable::extractObjectDescriptor(aTransferData) );
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
                                paste( eType,OComponentTransferable::extractComponentDescriptor(aTransferData),sFolderNameToInsertInto );
                            }
                            break;
                    }
                }
                break;
            case SID_DB_APP_PASTE_SPECIAL:
                {
                    SvPasteObjectDialog aDlg;
                    ::std::vector<SotFormatStringId> aFormatIds;
                    getSupportedFormats(getContainer()->getElementType(),aFormatIds);
                    ::std::vector<SotFormatStringId>::iterator aEnd = aFormatIds.end();
                    for (::std::vector<SotFormatStringId>::iterator aIter = aFormatIds.begin();aIter != aEnd; ++aIter)
                    {
                        aDlg.Insert(*aIter,SvPasteObjectDialog::GetSotFormatUIName(*aIter));
                    }

                    const TransferableDataHelper& rClipboard = getViewClipboard();
                    pasteFormat(aDlg.Execute(getView(),rClipboard.GetTransferable()));
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
                    Reference<XStorable> xStore(m_xDataSource,UNO_QUERY);
                    if ( xStore.is() )
                    {
                        xStore->store();
                        m_bCurrentlyModified = sal_False;
                    }
                }
                break;
            case ID_BROWSER_SAVEASDOC:
                {
                    WinBits nBits(WB_STDMODAL|WB_SAVEAS);

                    ::sfx2::FileDialogHelper aFileDlg( ::sfx2::FILESAVE_AUTOEXTENSION,static_cast<sal_uInt32>(nBits) ,getView());
                    aFileDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

                    const SfxFilter* pFilter = getStandardFilter();
                    if ( pFilter )
                    {
                        aFileDlg.AddFilter(pFilter->GetFilterName(),pFilter->GetDefaultExtension());
                        aFileDlg.SetCurrentFilter(pFilter->GetFilterName());
                    }

                    if ( aFileDlg.Execute() == ERRCODE_NONE )
                    {
                        Reference<XStorable> xStore(m_xDataSource,UNO_QUERY);
                        if ( xStore.is() )
                        {
                            INetURLObject aURL( aFileDlg.GetPath() );
                            if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
                            {
                                xStore->storeAsURL(aURL.GetMainURL( INetURLObject::NO_DECODE ),Sequence<PropertyValue>());
                                m_sDatabaseName = ::rtl::OUString();
                                updateTitle();
                                m_bCurrentlyModified = sal_False;
                                InvalidateFeature(ID_BROWSER_SAVEDOC);
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
                OSL_ENSURE(0,"NYI");
                break;
            case ID_NEW_VIEW_DESIGN_AUTO_PILOT:
                OSL_ENSURE(0,"NYI");
                break;
            case ID_APP_NEW_QUERY_AUTO_PILOT:
            case ID_FORM_NEW_PILOT:
            case ID_FORM_NEW_PILOT_PRE_SEL:
            case ID_DOCUMENT_CREATE_REPWIZ_PRE_SEL:
            case ID_DOCUMENT_CREATE_REPWIZ:
            case ID_APP_NEW_FORM:
            case ID_NEW_QUERY_SQL:
            case ID_NEW_QUERY_DESIGN:
            case ID_NEW_TABLE_DESIGN:
                {
                    ElementType eType = E_TABLE;
                    sal_Bool bAutoPilot = sal_False;
                    sal_Bool bSQLView = sal_False;

                    switch( _nId )
                    {
                        case ID_FORM_NEW_PILOT:
                        case ID_FORM_NEW_PILOT_PRE_SEL:
                            bAutoPilot = sal_True;
                            // run through
                        case ID_APP_NEW_FORM:
                            eType = E_FORM;
                            break;
                        case ID_DOCUMENT_CREATE_REPWIZ:
                        case ID_DOCUMENT_CREATE_REPWIZ_PRE_SEL:
                            bAutoPilot = sal_True;
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
                        case ID_NEW_TABLE_DESIGN:
                            break;
                        default:
                            OSL_ENSURE(0,"illegal switch call!");
                    }
                    newElement(eType,bAutoPilot,bSQLView);
                }
                break;
            case ID_APP_NEW_FOLDER:
                {
                    ElementType eType = getContainer()->getElementType();
                    ::rtl::OUString sName = getContainer()->getQualifiedName(NULL,NULL);
                    insertHierachyElement(eType,sName);
                }
                break;
            case ID_NEW_VIEW_DESIGN:
            case ID_NEW_VIEW_SQL:
                {
                    Reference<XConnection> xConnection;
                    ensureConnection(xConnection);
                    if ( xConnection.is() )
                    {
                        OQueryDesignAccess aHelper(getORB(),sal_True,ID_NEW_VIEW_SQL == _nId );
                        Reference< XComponent > xComponent(aHelper.create(Reference<XDataSource>(m_xDataSource,UNO_QUERY),xConnection),UNO_QUERY);
                        addDocumentListener(xComponent,NULL);
                    }
                }
                break;
            case SID_DB_APP_DELETE:
                deleteEntries();
                break;
            case SID_DB_APP_RENAME:
                renameEntry();
                break;
            case SID_DB_APP_EDIT:
            case SID_DB_APP_OPEN:
            case SID_DB_APP_CONVERTTOVIEW:
                {
                    ::std::vector< ::rtl::OUString> aList;
                    getSelectionElementNames(aList);
                    ElementType eType = getContainer()->getElementType();

                    ::std::vector< ::rtl::OUString>::iterator aEnd = aList.end();
                    for (::std::vector< ::rtl::OUString>::iterator aIter = aList.begin(); aIter != aEnd; ++aIter)
                    {
                        if ( SID_DB_APP_CONVERTTOVIEW == _nId )
                            convertToView(*aIter);
                        else
                            openElement(*aIter,eType, _nId == SID_DB_APP_EDIT );
                    }
                }
                break;
            case SID_DB_APP_SELECTALL:
                getContainer()->selectAll();
                break;
            case SID_DB_APP_DSIMPORT:
                OSL_ENSURE(0,"NYI");
                // TODO ds import
                break;
            case SID_DB_APP_DSEXPORT:
                OSL_ENSURE(0,"NYI");
                // TODO ds export
                break;
            case SID_DB_APP_DSRELDESIGN:
                {
                    Reference<XConnection> xConnection;
                    ensureConnection(xConnection);
                    if ( xConnection.is() )
                    {
                        ORelationDesignAccess aHelper(getORB());
                        Reference< XComponent > xComponent(aHelper.create(Reference<XDataSource>(m_xDataSource,UNO_QUERY), Reference<XConnection>()),UNO_QUERY);
                        addDocumentListener(xComponent,NULL);
                    }
                }
                break;
            case SID_DB_APP_DSUSERADMIN:
                openDialog(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.UserAdministrationDialog")));
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
                openDirectSQLDialog();
                break;
            case SID_DB_APP_VIEW_TABLES:
                getContainer()->changeContainer(E_TABLE);
                break;
            case SID_DB_APP_VIEW_QUERIES:
                getContainer()->changeContainer(E_QUERY);
                break;
            case SID_DB_APP_VIEW_FORMS:
                getContainer()->changeContainer(E_FORM);
                break;
            case SID_DB_APP_VIEW_REPORTS:
                getContainer()->changeContainer(E_REPORT);
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
            case SID_DB_APP_DBADMIN:
                break;
        }
    }
    catch(Exception& e)
    {
#if DBG_UTIL
        String sMessage("OApplicationController::Execute(", RTL_TEXTENCODING_ASCII_US);
        sMessage += String::CreateFromInt32(_nId);
        sMessage.AppendAscii(") : caught an exception ! message : ");
        sMessage += (const sal_Unicode*)e.Message;
        DBG_ERROR(ByteString(sMessage, gsl_getSystemTextEncoding()).GetBuffer());
#else
        e;  // make compiler happy
#endif
    }
    InvalidateFeature(_nId);
}
// -----------------------------------------------------------------------------
ToolBox* OApplicationController::CreateToolBox(Window* _pParent)
{
    return new ToolBox( _pParent, ModuleRes( RID_BRW_APPLICATION_TOOLBOX ) );
}
// -----------------------------------------------------------------------------
void OApplicationController::AddSupportedFeatures()
{
    OApplicationController_CBASE::AddSupportedFeatures();
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Copy"))]                    = ID_BROWSER_COPY;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Cut"))]                     = ID_BROWSER_CUT;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Paste"))]                   = ID_BROWSER_PASTE;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Save"))]                    = ID_BROWSER_SAVEDOC;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:SaveAs"))]                  = ID_BROWSER_SAVEASDOC;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewForm"))]              = ID_APP_NEW_FORM;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewFolder"))]            = ID_APP_NEW_FOLDER;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewFormAutoPilot"))]     = ID_FORM_NEW_PILOT;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewFormAutoPilotWithPreSelection"))]     = ID_FORM_NEW_PILOT_PRE_SEL;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewReportAutoPilot"))]   = ID_DOCUMENT_CREATE_REPWIZ;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewReportAutoPilotWithPreSelection"))]   = ID_DOCUMENT_CREATE_REPWIZ_PRE_SEL;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewQuery"))]             = ID_NEW_QUERY_DESIGN;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewQuerySql"))]          = ID_NEW_QUERY_SQL;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewQueryAutoPilot"))]    = ID_APP_NEW_QUERY_AUTO_PILOT;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewTable"))]             = ID_NEW_TABLE_DESIGN;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewTableAutoPilot"))]    = ID_NEW_TABLE_DESIGN_AUTO_PILOT;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewView"))]              = ID_NEW_VIEW_DESIGN;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewViewSQL"))]           = ID_NEW_VIEW_SQL;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/NewViewAutoPilot"))]     = ID_NEW_VIEW_DESIGN_AUTO_PILOT;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/Delete"))]               = SID_DB_APP_DELETE;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/Rename"))]               = SID_DB_APP_RENAME;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/Edit"))]                 = SID_DB_APP_EDIT;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/Open"))]                 = SID_DB_APP_OPEN;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/SelectAll"))]            = SID_DB_APP_SELECTALL;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:Sortup"))]                  = ID_BROWSER_SORTUP;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:SortDown"))]                = ID_BROWSER_SORTDOWN;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/DSImport"))]             = SID_DB_APP_DSIMPORT;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/DSExport"))]             = SID_DB_APP_DSEXPORT;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/RelationDesign"))]       = SID_DB_APP_DSRELDESIGN;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/UserAdmin"))]            = SID_DB_APP_DSUSERADMIN;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/TableFilter"))]          = SID_DB_APP_TABLEFILTER;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/DSProperties"))]         = SID_DB_APP_DSPROPS;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/DSConnectionType"))]     = SID_DB_APP_DSCONNECTION_TYPE;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/DSAdvancedSettings"))]   = SID_DB_APP_DSADVANCED_SETTINGS;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/PasteSpecial"))]         = SID_DB_APP_PASTE_SPECIAL;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/ConvertToView"))]        = SID_DB_APP_CONVERTTOVIEW;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/RefreshTables"))]        = SID_DB_APP_REFRESH_TABLES;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/DirectSQL"))]            = ID_DIRECT_SQL;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/ViewTables"))]           = SID_DB_APP_VIEW_TABLES;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/ViewQueries"))]          = SID_DB_APP_VIEW_QUERIES;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/ViewForms"))]            = SID_DB_APP_VIEW_FORMS;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/ViewReports"))]          = SID_DB_APP_VIEW_REPORTS;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/DisablePreview"))]       = SID_DB_APP_DISABLE_PREVIEW;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/ShowDocInfoPreview"))]   = SID_DB_APP_VIEW_DOCINFO_PREVIEW;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/ShowDocPreview"))]       = SID_DB_APP_VIEW_DOC_PREVIEW;
    m_aSupportedFeatures[ ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DB/DBAdmin"))]              = SID_DB_APP_DBADMIN;
}
// -----------------------------------------------------------------------------
OApplicationView*   OApplicationController::getContainer() const
{
    return static_cast< OApplicationView* >( getView() );
}
// -----------------------------------------------------------------------------
void OApplicationController::onCreationClick(sal_uInt16 _nId)
{
    executeChecked(_nId);
}
// -----------------------------------------------------------------------------
namespace
{
    template <class T> void lcl_addImage(T* _pMenu,USHORT _nMenuId,USHORT _nStartId,USHORT _nImageListId)
    {
        ModuleRes aRes(_nImageListId);
        ImageList aImageList(aRes);
        _pMenu->SetItemImage(_nMenuId,aImageList.GetImage(_nStartId));
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::openToolBoxPopup(USHORT _nSlotId)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard(m_aMutex);

    ToolBox* pToolBox = NULL;
    if ( getView() )
        pToolBox = getView()->getToolBox();
    OSL_ENSURE( pToolBox, "OApplicationController::openToolBoxPopup: no toolbox (anymore)!" );

    if ( !pToolBox )
        return ;

    pToolBox->EndSelection();

    // tell the toolbox that the item is pressed down
    pToolBox->SetItemDown( _nSlotId, sal_True );

    // simulate a mouse move (so the "down" state is really painted)
    Point aPoint = pToolBox->GetItemRect( _nSlotId ).TopLeft();
    MouseEvent aMove( aPoint, 0, MOUSE_SIMPLEMOVE | MOUSE_SYNTHETIC );
    pToolBox->MouseMove( aMove );

    pToolBox->Update();

    // execute the menu
    ::std::auto_ptr<PopupMenu> pMenu;
    if ( _nSlotId == ID_APP_NEW_FORM )
    {
        pMenu.reset(new PopupMenu( ModuleRes( RID_MENU_APP_NEW ) ));

        sal_Bool bIsWriterInstalled = SvtModuleOptions().IsModuleInstalled(SvtModuleOptions::E_SWRITER);

        sal_Bool bHighContrast = getView()->GetBackground().GetColor().IsDark();

        lcl_addImage(pMenu.get(),ID_NEW_TABLE_DESIGN,ID_NEW_TABLE_DESIGN,               bHighContrast ? IMG_TABLESUBCRIPTION_SCH    :IMG_TABLESUBCRIPTION_SC        );
        lcl_addImage(pMenu.get(),ID_NEW_VIEW_DESIGN,ID_NEW_VIEW_DESIGN,                 bHighContrast ? IMG_TABLESUBCRIPTION_SCH    :IMG_TABLESUBCRIPTION_SC        );
        lcl_addImage(pMenu.get(),ID_NEW_VIEW_SQL,ID_NEW_VIEW_SQL,                       bHighContrast ? IMG_TABLESUBCRIPTION_SCH    :IMG_TABLESUBCRIPTION_SC        );
        lcl_addImage(pMenu.get(),ID_NEW_QUERY_DESIGN,ID_NEW_QUERY_DESIGN,               bHighContrast ? IMG_QUERYADMINISTRATION_SCH:IMG_QUERYADMINISTRATION_SC          );
        lcl_addImage(pMenu.get(),ID_NEW_QUERY_SQL,ID_NEW_QUERY_SQL,                     bHighContrast ? IMG_QUERYADMINISTRATION_SCH:IMG_QUERYADMINISTRATION_SC      );
        lcl_addImage(pMenu.get(),ID_APP_NEW_FORM,ID_APP_NEW_FORM,                       bHighContrast ? IMG_DOCUMENTLINKS_SCH       :IMG_DOCUMENTLINKS_SC           );
        lcl_addImage(pMenu.get(),ID_DOCUMENT_CREATE_REPWIZ,ID_DOCUMENT_CREATE_REPWIZ,   bHighContrast ? IMG_DOCUMENTLINKS_SCH       :IMG_DOCUMENTLINKS_SC        );

        USHORT nCount = pMenu->GetItemCount();
        for (USHORT i=0; i < nCount; ++i)
            pMenu->EnableItem(pMenu->GetItemId(i),GetState(pMenu->GetItemId(i)).bEnabled);
    }
    else if ( _nSlotId == ID_BROWSER_PASTE )
    {
        pMenu.reset(new PopupMenu( ));

        const TransferableDataHelper& rClipboard = getViewClipboard();
        sal_uInt32 nCount = rClipboard.GetFormatCount();
        ::std::vector<SotFormatStringId> aFormatIds;
        getSupportedFormats(getContainer()->getElementType(),aFormatIds);

        for (sal_uInt32 i = 0;  i < nCount;  ++i)
        {
            SotFormatStringId nSotId = rClipboard.GetFormat(i);
            if ( ::std::find(aFormatIds.begin(),aFormatIds.end(),nSotId) != aFormatIds.end() )
            {
                String aFmtStr( SvPasteObjectDialog::GetSotFormatUIName(nSotId) );
                pMenu->InsertItem( static_cast<USHORT>(nSotId), aFmtStr );
            }
        }

    }
    else
        return;

    // no disabled entries
    pMenu->RemoveDisabledEntries();

    ::std::map<sal_uInt16,sal_uInt16>::iterator aFind = m_aLastSelectedPullDownActions.find(_nSlotId);
    OSL_ENSURE( aFind != m_aLastSelectedPullDownActions.end() ,"Invalid slot id!");
    aFind->second = pMenu->Execute(pToolBox, pToolBox->GetItemRect( _nSlotId ));
    // "cleanup" the toolbox state
    MouseEvent aLeave( aPoint, 0, MOUSE_LEAVEWINDOW | MOUSE_SYNTHETIC );
    pToolBox->MouseMove( aLeave );
    pToolBox->SetItemDown( _nSlotId, sal_False);

    if ( aFind->second )
    {
        if ( _nSlotId == ID_BROWSER_PASTE )
            pasteFormat(aFind->second);
        else
        {
            pToolBox->SetItemImage(_nSlotId, pMenu->GetItemImage(aFind->second));
            Execute( aFind->second );
        }
    }
}

// -----------------------------------------------------------------------------
IMPL_LINK( OApplicationController, OnShowRefreshDropDown, void*, NOTINTERESTEDIN )
{
    openToolBoxPopup(ID_APP_NEW_FORM);
    return 1L;
}
// -----------------------------------------------------------------------------
IMPL_LINK( OApplicationController, OnPasteSpecialDropDown, void*, NOTINTERESTEDIN )
{
    openToolBoxPopup(ID_BROWSER_PASTE);
    return 1L;
}
// -----------------------------------------------------------------------------
void OApplicationController::onToolBoxSelected( sal_uInt16 _nSelectedItem )
{
    if ( ID_APP_NEW_FORM == _nSelectedItem )
    {
        if ( m_aRefreshMenu.IsActive() )
            m_aRefreshMenu.Stop();
    }
    else if ( SID_DB_APP_PASTE_SPECIAL == _nSelectedItem || ID_BROWSER_PASTE == _nSelectedItem)
    {
        if ( m_aPasteSpecialMenu.IsActive() )
            m_aPasteSpecialMenu.Stop();
    }
    ::std::map<sal_uInt16,sal_uInt16>::iterator aFind = m_aLastSelectedPullDownActions.find(_nSelectedItem);
    if ( aFind != m_aLastSelectedPullDownActions.end() && aFind->second )
        Execute( aFind->second );
    else
        OGenericUnoController::onToolBoxSelected(_nSelectedItem);
}
// -----------------------------------------------------------------------------
void OApplicationController::onToolBoxClicked( sal_uInt16 _nClickedItem )
{
    if ( ID_APP_NEW_FORM == _nClickedItem )
        m_aRefreshMenu.Start();
    else if ( SID_DB_APP_PASTE_SPECIAL == _nClickedItem || ID_BROWSER_PASTE == _nClickedItem )
        m_aPasteSpecialMenu.Start();
    else
        OGenericUnoController::onToolBoxClicked( _nClickedItem );
}
// -----------------------------------------------------------------------------
// ::com::sun::star::container::XContainerListener
void SAL_CALL OApplicationController::elementInserted( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    if ( xNames == m_xCurrentContainer || ::std::find(m_aCurrentSubContainers.begin(),m_aCurrentSubContainers.end(),Reference< XContainer >(xNames,UNO_QUERY)) != m_aCurrentSubContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        if ( getContainer() )
        {
            ::rtl::OUString sName;
            _rEvent.Accessor >>= sName;
            Reference<XConnection> xConnection;
            ElementType eType = getContainer()->getElementType();

            switch( eType )
            {
                case E_TABLE:
                    ensureConnection(xConnection);
                    break;
                case E_FORM:
                case E_REPORT:
                    {
                        Reference< XContainer > xContainer(_rEvent.Element,UNO_QUERY);
                        if ( xContainer.is() )
                            containerFound(xContainer);
                    }
                    break;
            }
            getContainer()->elementAdded(eType,sName,_rEvent.Element,xConnection);
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OApplicationController::elementRemoved( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    if ( xNames == m_xCurrentContainer || ::std::find(m_aCurrentSubContainers.begin(),m_aCurrentSubContainers.end(),Reference< XContainer >(xNames,UNO_QUERY)) != m_aCurrentSubContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        ::rtl::OUString sName;
        _rEvent.Accessor >>= sName;
        Reference<XConnection> xConnection;
        ElementType eType = getContainer()->getElementType();
        switch( eType )
        {
            case E_TABLE:
                ensureConnection(xConnection);
                break;
            case E_FORM:
            case E_REPORT:
                {
                    Reference<XContent> xContent(xNames,UNO_QUERY);
                    if ( xContent.is() )
                    {
                        sName = xContent->getIdentifier()->getContentIdentifier() + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sName;
                    }
                }
                break;
        }
        getContainer()->elementRemoved(sName,xConnection);
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OApplicationController::elementReplaced( const ContainerEvent& _rEvent ) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    ::osl::MutexGuard aGuard(m_aMutex);

    Reference< XNameAccess > xNames(_rEvent.Source, UNO_QUERY);
    if ( xNames == m_xCurrentContainer || ::std::find(m_aCurrentSubContainers.begin(),m_aCurrentSubContainers.end(),Reference< XContainer >(xNames,UNO_QUERY)) != m_aCurrentSubContainers.end() )
    {
        OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
        ::rtl::OUString sName;
        try
        {
            _rEvent.Accessor >>= sName;
            Reference<XConnection> xConnection;
            Reference<XPropertySet> xProp(_rEvent.Element,UNO_QUERY);
            ::rtl::OUString sNewName;

            ElementType eType = getContainer()->getElementType();
            switch( eType )
            {
                case E_TABLE:
                    ensureConnection(xConnection);
                    if ( xProp.is() && xConnection.is() )
                    {
                        sNewName = ::dbaui::composeTableName(xConnection->getMetaData(),xProp,sal_False,::dbtools::eInDataManipulation);
                    }
                    break;
                case E_FORM:
                case E_REPORT:
                    {
                        Reference<XContent> xContent(xNames,UNO_QUERY);
                        if ( xContent.is() )
                        {
                            sName = xContent->getIdentifier()->getContentIdentifier() + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sName;
                        }
                    }
                    break;
            }
            //  getContainer()->elementReplaced(getContainer()->getElementType(),sName,sNewName,xConnection);
        }
        catch( Exception& )
        {
            OSL_ENSURE(0,"Exception catched");
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::onContainerSelect(ElementType _eType)
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");

    ToolBox* pToolBox = getView()->getToolBox();

    sal_Bool bAdd = sal_True;
    sal_Bool bHighContrast = getView()->GetBackground().GetColor().IsDark();
    USHORT nImageListId = 0;
    ::std::vector<USHORT> aImageIds;
    switch( _eType )
    {
        case E_FORM:
            nImageListId = bHighContrast ? IMP_FORM_SCH : IMP_FORM_SC;
            aImageIds.push_back(SID_DB_FORM_DELETE);
            aImageIds.push_back(SID_DB_FORM_RENAME);
            aImageIds.push_back(SID_DB_FORM_EDIT);
            aImageIds.push_back(SID_DB_FORM_OPEN);
            break;
        case E_REPORT:
            nImageListId = bHighContrast ? IMP_REPORT_SCH : IMP_REPORT_SC;
            aImageIds.push_back(SID_DB_REPORT_DELETE);
            aImageIds.push_back(SID_DB_REPORT_RENAME);
            aImageIds.push_back(SID_DB_REPORT_EDIT);
            aImageIds.push_back(SID_DB_REPORT_OPEN);
            break;
        case E_QUERY:
            nImageListId = bHighContrast ? IMG_QUERYADMINISTRATION_SCH : IMG_QUERYADMINISTRATION_SC;
            aImageIds.push_back(ID_DROP_QUERY);
            aImageIds.push_back(ID_RENAME_ENTRY);
            aImageIds.push_back(SID_DB_QUERY_EDIT);
            aImageIds.push_back(SID_DB_QUERY_OPEN);
            break;
        case E_TABLE:
            {
                bAdd = sal_False;
                nImageListId = bHighContrast ? IMG_TABLESUBCRIPTION_SCH : IMG_TABLESUBCRIPTION_SC;
                aImageIds.push_back(ID_DROP_TABLE);
                aImageIds.push_back(ID_RENAME_ENTRY);
                aImageIds.push_back(ID_EDIT_TABLE);
                aImageIds.push_back(SID_DB_TABLE_OPEN);
                Reference<XConnection> xConnection;
                try
                {
                    ensureConnection(xConnection);

                    if ( xConnection.is() )
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
                catch(Exception)
                {
                    return sal_False;
                }
            }
            break;
        case E_NONE:
            break;
        default:
            OSL_ENSURE(0,"Illegal call!");
    }

    USHORT pIds[] = { SID_DB_APP_DELETE, SID_DB_APP_RENAME, SID_DB_APP_EDIT, SID_DB_APP_OPEN };
    OSL_ENSURE(aImageIds.size() == sizeof(pIds)/sizeof(pIds[0]),"Illegal size of vector or id list!");
    for (USHORT i = 0; i < sizeof(pIds)/sizeof(pIds[0]); ++i)
    {
        lcl_addImage(pToolBox,pIds[i],aImageIds[i],nImageListId);
    }

    if ( bAdd )
    {
        Reference< XNameAccess > xContainer = getElements(_eType);
        addContainerListener(xContainer);
        getContainer()->getDetailView()->createPage(_eType,xContainer);
    }

    InvalidateAll();

    return sal_True;
}
// -----------------------------------------------------------------------------
void OApplicationController::onEntryDoubleClick(SvTreeListBox* _pTree)
{
    OSL_ENSURE(_pTree != NULL,"Who called me without a svtreelsiboc! ->GPF ");
    if ( getContainer() && getContainer()->isLeaf(_pTree->GetHdlEntry()) )
    {
        ElementType eType = getContainer()->getElementType();

        Reference<XDatabaseMetaData> xMetaData;
        if ( eType == E_TABLE )
        {
            Reference<XConnection> xConnection;
            ensureConnection(xConnection);
            if ( xConnection.is() )
            {
                try
                {
                    xMetaData = xConnection->getMetaData();
                }
                catch(const Exception&)
                {
                    OSL_ENSURE(0,"Exception catched!");
                }
            }
        }
        try
        {
            openElement(getContainer()->getQualifiedName(_pTree->GetHdlEntry(),xMetaData),eType);
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Could not open element!");
        }
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::openElement(const ::rtl::OUString& _sName,ElementType _eType,sal_Bool _bOpenDesignMode)
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");
    if ( _bOpenDesignMode )
    {
        // OJ: http://www.openoffice.org/issues/show_bug.cgi?id=30382
        getContainer()->showPreview(NULL);
    }
    switch ( _eType )
    {
        case E_REPORT: // TODO: seperate handling of forms and reports
        case E_FORM:
            {
                ::std::auto_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
                Reference< XComponent > xDefinition;
                Reference< XComponent > xComponent(aHelper->open(_sName, xDefinition,!_bOpenDesignMode),UNO_QUERY);
                addDocumentListener(xComponent,xDefinition);
            }
            break;
        case E_QUERY:
        case E_TABLE:
            {
                ::std::auto_ptr< ODesignAccess> pDispatcher;
                Reference<XConnection> xConnection;
                ensureConnection(xConnection);
                if ( xConnection.is() )
                {
                    Sequence < PropertyValue > aArgs;
                    Any aDataSource;
                    if ( _bOpenDesignMode )
                    {
                        if ( _eType == E_TABLE )
                        {
                            pDispatcher.reset(new OTableDesignAccess(getORB()));
                        }
                        else
                        {
                            pDispatcher.reset(new OQueryDesignAccess(getORB(), sal_False, sal_False));
                        }
                        aDataSource <<= m_xDataSource;
                    }
                    else
                    {
                        pDispatcher.reset(new OTableAccess(getORB(),_eType == E_TABLE));

                        aArgs.realloc(1);
                        aArgs[0].Name = PROPERTY_SHOWMENU;
                        aArgs[0].Value <<= sal_True;

                        aDataSource <<= getDatabaseName();
                    }

                    Reference< XComponent > xComponent(pDispatcher->edit(aDataSource, _sName,xConnection,aArgs),UNO_QUERY);
                    addDocumentListener(xComponent,NULL);
                }
            }
            break;
    }
}
// -----------------------------------------------------------------------------
void OApplicationController::newElement(ElementType _eType,sal_Bool _bAutoPilot,sal_Bool _bSQLView)
{
    OSL_ENSURE(getContainer(),"View is NULL! -> GPF");

    switch ( _eType )
    {
        case E_REPORT: // TODO: seperate handling of forms and reports
        case E_FORM:
            {
                ::std::auto_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
                Reference< XComponent > xComponent,xDefinition;
                if ( _bAutoPilot )
                {
                    sal_Int32 nCommandType = ( (getContainer()->getElementType() == E_QUERY)
                                                ? CommandType::QUERY : ( (getContainer()->getElementType() == E_TABLE) ? CommandType::TABLE : -1 ));
                    Reference<XConnection> xConnection;

                    ::rtl::OUString sName;
                    if ( nCommandType != -1 )
                    {
                        try
                        {
                            Reference< XDatabaseMetaData> xMetaData;
                            if ( CommandType::TABLE == nCommandType )
                            {
                                ensureConnection(xConnection,sal_False);

                                if ( xConnection.is() )
                                    xMetaData = xConnection->getMetaData();
                            }

                            sName = getContainer()->getQualifiedName(NULL,xMetaData);
                            OSL_ENSURE(sName.getLength(),"NO name given!");
                        }
                        catch(Exception&)
                        {
                            OSL_ENSURE(0,"Exception catched!");
                        }
                    }

                    try
                    {
                        ensureConnection(xConnection,sal_True);
                    }
                    catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
                    catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
                    catch(SQLException& e) { showError(SQLExceptionInfo(e)); }

                    if ( xConnection.is() )
                    {
                        if ( E_REPORT == _eType )
                            aHelper->newReportWithPilot(getDatabaseName(),nCommandType,sName,xConnection);
                        else
                            aHelper->newFormWithPilot(getDatabaseName(),nCommandType,sName,xConnection);
                    }
                }
                else if ( E_FORM == _eType )
                    xComponent = aHelper->newForm(ID_FORM_NEW_TEXT,xDefinition);

                addDocumentListener(xComponent,xDefinition);
            }
            break;
        case E_QUERY:
            {
                if ( _bAutoPilot )
                {
                    ::std::auto_ptr<OLinkedDocumentsAccess> aHelper = getDocumentsAccess(_eType);
                    Reference< XComponent > xComponent,xDefinition;
                    Reference<XConnection> xConnection;
                    try
                    {
                        ensureConnection(xConnection,sal_True);
                        aHelper->newQueryWithPilot(getDatabaseName(),-1,::rtl::OUString(),xConnection);
                        addDocumentListener(xComponent,xDefinition);
                    }
                    catch(SQLContext& e) { showError(SQLExceptionInfo(e)); }
                    catch(SQLWarning& e) { showError(SQLExceptionInfo(e)); }
                    catch(SQLException& e) { showError(SQLExceptionInfo(e)); }
                    break;
                }
            } // run through
        case E_TABLE:
            {
                ::std::auto_ptr< ODesignAccess> pDispatcher;
                Reference<XConnection> xConnection;
                ensureConnection(xConnection);
                if ( xConnection.is() )
                {
                    if ( _eType == E_TABLE )
                    {
                        pDispatcher.reset(new OTableDesignAccess(getORB()));
                    }
                    else
                    {
                        pDispatcher.reset(new OQueryDesignAccess(getORB(), sal_False, _bSQLView));
                    }
                    Reference< XComponent > xComponent(pDispatcher->create(Reference<XDataSource>(m_xDataSource,UNO_QUERY), xConnection),UNO_QUERY);
                    addDocumentListener(xComponent,NULL);
                }
            }
            break;
    }
}
// -----------------------------------------------------------------------------
Reference< XNumberFormatter > OApplicationController::getNumberFormatter(const Reference< XConnection >& _rxConnection ) const
{
    // ---------------------------------------------------------------
    // create a formatter working with the connections format supplier
    Reference< XNumberFormatter > xFormatter;

    try
    {
        Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(::dbtools::getNumberFormats(_rxConnection, sal_True,getORB()));

        if ( xSupplier.is() )
        {
            // create a new formatter
            xFormatter = Reference< ::com::sun::star::util::XNumberFormatter > (
                getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatter"))), UNO_QUERY);
            if ( xFormatter.is() )
                xFormatter->attachNumberFormatsSupplier(xSupplier);
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched!");
    }
    return xFormatter;
}
// -----------------------------------------------------------------------------
void OApplicationController::addContainerListener(const Reference<XNameAccess>& _xCollection)
{
    try
    {
        Reference< XContainer > xCont(_xCollection, UNO_QUERY);
        if ( xCont.is() && m_xCurrentContainer != xCont )
        {
            if ( m_xCurrentContainer.is() )
                m_xCurrentContainer->removeContainerListener(this);
            ::std::for_each(m_aCurrentSubContainers.begin(),m_aCurrentSubContainers.end(),XContainerFunctor(this));
            m_aCurrentSubContainers.clear();
            // add as listener to get notified if elements are inserted or removed
            xCont->addContainerListener(this);
            m_xCurrentContainer = xCont;
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

    try
    {
        if ( xContainer.is() )
        {
            Reference<XConnection> xConnection = getActiveConnection();
            ::std::auto_ptr<OSaveAsDlg> aDlg;
            Reference<XRename> xRename;
            ElementType eType = getContainer()->getElementType();
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
                            {
                                sLabel = String(ModuleRes( STR_FRM_LABEL ));
                            }
                            else
                            {
                                sLabel = String(ModuleRes( STR_RPT_LABEL ));
                            }

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
                                aDlg.reset( new OSaveAsDlg(getView(),xHNames.get(),sName,sLabel,String(),SAD_TITLE_RENAME) );
                            }
                        }
                    }
                    break;
                case E_QUERY:
                    if ( xContainer->hasByName(*aList.begin()) )
                    {
                        xRename.set(xContainer->getByName(*aList.begin()),UNO_QUERY);
                        aDlg.reset( new OSaveAsDlg(getView(),CommandType::QUERY,xContainer,xConnection->getMetaData(),xConnection,*aList.begin(),SAD_TITLE_RENAME) );
                    }
                    break;
                case E_TABLE:
                    if ( xContainer->hasByName(*aList.begin()) )
                    {
                        xRename.set(xContainer->getByName(*aList.begin()),UNO_QUERY);
                        aDlg.reset( new OSaveAsDlg(getView(),CommandType::TABLE,xContainer,xConnection->getMetaData(),xConnection,*aList.begin(),SAD_TITLE_RENAME) );
                    }
                    break;
            }

            if ( xRename.is() && aDlg.get() )
            {

                sal_Bool bTryAgain = sal_True;
                while( bTryAgain )
                {
                    if ( aDlg->Execute() == RET_OK )
                    {
                        try
                        {
                            ::rtl::OUString sNewName;
                            if ( eType == E_TABLE )
                            {
                                ::rtl::OUString sName = aDlg->getName();
                                ::rtl::OUString sCatalog = aDlg->getCatalog();
                                ::rtl::OUString sSchema  = aDlg->getSchema();

                                ::dbtools::composeTableName(xConnection->getMetaData(),sCatalog,sSchema,sName,sNewName,sal_False,::dbtools::eInTableDefinitions);
                            }
                            else
                                sNewName = aDlg->getName();

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

                            if ( ! Reference< XNameAccess >(xRename,UNO_QUERY).is() )
                                getContainer()->elementReplaced(getContainer()->getElementType(),sOldName,sNewName,xConnection);

                            bTryAgain = sal_False;
                        }
                        catch(const SQLException& e)
                        {
                            showError(SQLExceptionInfo(e));

                        }
                        catch(const ElementExistException& e)
                        {
                            static ::rtl::OUString sStatus = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("S1000"));
                            String sMsg = String(ModuleRes(STR_OBJECT_ALREADY_EXISTS));
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
void OApplicationController::onEntryDeSelect(SvTreeListBox* _pTree)
{
    getContainer()->showPreview(NULL);
    InvalidateAll();
}
// -----------------------------------------------------------------------------
void OApplicationController::onEntrySelect(SvLBoxEntry* _pEntry)
{
    InvalidateAll();

    OApplicationView* pView = getContainer();
    if ( pView )
    {
        ElementType eType = pView->getElementType();
        Reference< XContent> xContent;
        if ( _pEntry && pView->isALeafSelected() )
        {
            try
            {
                switch( eType )
                {
                    case E_FORM:
                    case E_REPORT:
                        {
                            ::rtl::OUString sName = pView->getQualifiedName( _pEntry,NULL);
                            if ( sName.getLength() )
                            {
                                Reference< XHierarchicalNameAccess > xContainer(getElements(eType),UNO_QUERY);
                                if ( xContainer.is() && xContainer->hasByHierarchicalName(sName) )
                                    xContent.set(xContainer->getByHierarchicalName(sName),UNO_QUERY);
                            }
                        }
                        break;
                    case E_QUERY:
                    case E_TABLE:
                        {
                            Reference<XConnection> xConnection;
                            ensureConnection(xConnection);
                            if ( xConnection.is() )
                            {
                                ::rtl::OUString sName = pView->getQualifiedName( _pEntry,xConnection->getMetaData());
                                pView->showPreview(getDatabaseName(),xConnection,sName,eType == E_TABLE);
                                return;
                            }
                        }
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
        }
        pView->showPreview(xContent);
    }
}
//------------------------------------------------------------------------------
void OApplicationController::frameAction(const FrameActionEvent& aEvent) throw( RuntimeException )
{
    if ((XFrame*)aEvent.Frame.get() == (XFrame*)m_xCurrentFrame.get())
        switch (aEvent.Action)
        {
            case FrameAction_FRAME_ACTIVATED:
            case FrameAction_FRAME_UI_ACTIVATED:
                m_bFrameUiActive = sal_True;
                break;
            case FrameAction_FRAME_DEACTIVATING:
            case FrameAction_FRAME_UI_DEACTIVATING:
                m_bFrameUiActive = sal_False;
                break;
        }
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
void OApplicationController::onCutEntry(SvLBoxEntry* _pEntry)
{
}
// -----------------------------------------------------------------------------
void OApplicationController::onCopyEntry(SvLBoxEntry* _pEntry)
{
    Execute(ID_BROWSER_COPY);
}
// -----------------------------------------------------------------------------
void OApplicationController::onPasteEntry(SvLBoxEntry* _pEntry)
{
    Execute(ID_BROWSER_PASTE);
}
// -----------------------------------------------------------------------------
void OApplicationController::onDeleteEntry(SvLBoxEntry* _pEntry)
{
    executeChecked(SID_DB_APP_DELETE);
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::requestContextMenu( const CommandEvent& _rEvent )
{
    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool OApplicationController::requestDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    TransferableHelper* pTransfer = NULL;
    if ( getContainer() && getContainer()->getSelectionCount() )
    {
        try
        {
            pTransfer = copyObject( );
            Reference< XTransferable> xEnsureDelete = pTransfer;

            if ( pTransfer )
            {
                ElementType eType = getContainer()->getElementType();
                pTransfer->StartDrag( getContainer()->getDetailView(), ((eType == E_FORM || eType == E_REPORT) ? DND_ACTION_COPYMOVE : DND_ACTION_COPY) );
            }
        }
        catch(const Exception& )
        {
            OSL_ENSURE(0,"Exception catched!");
        }
    }

    return NULL != pTransfer;
}
/// unary_function Functor object for class DataFlavorExVector::value_type returntype is bool
struct TAppSupportedSotFunctor : ::std::unary_function<DataFlavorExVector::value_type,bool>
{
    ElementType eEntryType;
    sal_Bool    bQueryDrop;
    TAppSupportedSotFunctor(const ElementType& _eEntryType,sal_Bool _bQueryDrop)
        : eEntryType(_eEntryType)
        , bQueryDrop(_bQueryDrop)
    {
    }

    inline bool operator()(const DataFlavorExVector::value_type& _aType)
    {
        switch (_aType.mnSotId)
        {
            case SOT_FORMAT_RTF:                    // RTF data descriptions
            case SOT_FORMATSTR_ID_HTML:             // HTML data descriptions
            case SOT_FORMATSTR_ID_HTML_SIMPLE:      // HTML data descriptions
            case SOT_FORMATSTR_ID_DBACCESS_TABLE:   // table descriptor
                return (E_TABLE == eEntryType);
                break;
            case SOT_FORMATSTR_ID_DBACCESS_QUERY:   // query descriptor
            case SOT_FORMATSTR_ID_DBACCESS_COMMAND: // SQL command
                return ((E_QUERY == eEntryType) || ( !bQueryDrop && E_TABLE == eEntryType));
                break;
        }
        return false;
    }
};
// -----------------------------------------------------------------------------
sal_Int8 OApplicationController::queryDrop( const AcceptDropEvent& _rEvt, const DataFlavorExVector& _rFlavors )
{
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
                sal_Int8 nAction = OComponentTransferable::canExtractComponentDescriptor(_rFlavors) ? DND_ACTION_COPY : DND_ACTION_NONE;
                if ( nAction != DND_ACTION_NONE )
                {
                    SvLBoxEntry* pHitEntry = pView->getEntry(_rEvt.maPosPixel);
                    ::rtl::OUString sName;
                    if ( pHitEntry )
                    {
                        sName = pView->getQualifiedName( pHitEntry,NULL);
                        if ( sName.getLength() )
                        {
                            Reference< XHierarchicalNameAccess > xContainer(getElements(pView->getElementType()),UNO_QUERY);
                            if ( xContainer.is() && xContainer->hasByHierarchicalName(sName) )
                            {
                                Reference< XHierarchicalNameAccess > xHitObject(xContainer->getByHierarchicalName(sName),UNO_QUERY);
                                if ( xHitObject.is() )
                                    nAction |= DND_ACTION_MOVE;
                            }
                            else
                                nAction = DND_ACTION_NONE;
                        }
                    }
                    else
                        nAction = DND_ACTION_COPY;
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
    else if ( OComponentTransferable::canExtractComponentDescriptor(aDroppedData.GetDataFlavorExVector()) )
    {
        m_aAsyncDrop.aDroppedData = OComponentTransferable::extractComponentDescriptor(aDroppedData);
        SvLBoxEntry* pHitEntry = pView->getEntry(_rEvt.maPosPixel);
        if ( pHitEntry )
            m_aAsyncDrop.aUrl = pView->getQualifiedName( pHitEntry,NULL);

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
        sal_Bool bHtml = aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML) || aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML_SIMPLE);
        if ( bHtml || aDroppedData.HasFormat(SOT_FORMAT_RTF))
        {
            if ( bHtml )
                const_cast<TransferableDataHelper&>(aDroppedData).GetSotStorageStream(aDroppedData.HasFormat(SOT_FORMATSTR_ID_HTML) ? SOT_FORMATSTR_ID_HTML : SOT_FORMATSTR_ID_HTML_SIMPLE,m_aAsyncDrop.aHtmlRtfStorage);
            else
                const_cast<TransferableDataHelper&>(aDroppedData).GetSotStorageStream(SOT_FORMAT_RTF,m_aAsyncDrop.aHtmlRtfStorage);

            m_aAsyncDrop.bHtml          = bHtml;
            m_aAsyncDrop.bError         = !copyTagTable(m_aAsyncDrop,sal_True);

            if ( !m_aAsyncDrop.bError && m_aAsyncDrop.aHtmlRtfStorage.Is() )
            {
                // now we need to copy the stream
                ::utl::TempFile aTmp;
                aTmp.EnableKillingFile(sal_False);
                m_aAsyncDrop.aUrl = aTmp.GetURL();
                SotStorageStreamRef aNew = new SotStorageStream( aTmp.GetFileName() );
                m_aAsyncDrop.aHtmlRtfStorage->Seek(STREAM_SEEK_TO_BEGIN);
                m_aAsyncDrop.aHtmlRtfStorage->CopyTo( aNew );
                aNew->Commit();
                m_aAsyncDrop.aHtmlRtfStorage = aNew;
            }
            else
                m_aAsyncDrop.aHtmlRtfStorage = NULL;

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
    ::osl::MutexGuard aGuard(m_aMutex);
    return Reference< XModel >(m_xDataSource,UNO_QUERY);
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OApplicationController::attachModel(const Reference< XModel > & xModel) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aMutex);
    m_xDataSource.set(xModel,UNO_QUERY);
    if ( m_xDataSource.is() )
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
            Reference< XModifyBroadcaster >  xBroadcaster(m_xDataSource, UNO_QUERY);
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
            OSL_ENSURE(0,"Exception catched while set property listener");
        }
    }
    return m_sDatabaseName.getLength() != 0;
}
// -----------------------------------------------------------------------------
void OApplicationController::containerFound( const Reference< XContainer >& _xContainer)
{
    try
    {
        if ( _xContainer.is() )
        {
            m_aCurrentSubContainers.push_back(_xContainer);
            _xContainer->addContainerListener(this);
        }
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Could not listener on the container!");
    }
}
//........................................................................
}   // namespace dbaui
//........................................................................


