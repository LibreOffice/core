/*************************************************************************
 *
 *  $RCSfile: brwctrlr.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: fs $ $Date: 2000-11-07 18:34:39 $
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


#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _SBA_BWRCTRLR_HXX
#include "brwctrlr.hxx"
#endif
#ifndef _SBX_BRWVIEW_HXX
#include "brwview.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_ //autogen wg. MutexGuard
#include <osl/mutex.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen wg. SFX_APP
#include <sfx2/app.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_TYPECLASS_HPP_
#include <com/sun/star/uno/TypeClass.hpp>
#endif
#ifndef _SVX_DBERRBOX_HXX
#include <svx/dbmsgbox.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSETLISTENER_HPP_
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSERFACTORY_HPP_
#include <com/sun/star/sdb/XSQLQueryComposerFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XROWLOCATE_HPP_
#include <com/sun/star/sdbcx/XRowLocate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLERRORBROADCASTER_HPP_
#include <com/sun/star/sdb/XSQLErrorBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DATA_XDATABASEDIALOGS_HPP_
#include <com/sun/star/data/XDatabaseDialogs.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCHANGELISTENER_HPP_
#include <com/sun/star/form/XChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITENCODING_HPP_
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMSUBMITMETHOD_HPP_
#include <com/sun/star/form/FormSubmitMethod.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XSUBMITLISTENER_HPP_
#include <com/sun/star/form/XSubmitListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XSUBMIT_HPP_
#include <com/sun/star/form/XSubmit.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XAPPROVEACTIONBROADCASTER_HPP_
#include <com/sun/star/form/XApproveActionBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XCHANGEBROADCASTER_HPP_
#include <com/sun/star/form/XChangeBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XRESETLISTENER_HPP_
#include <com/sun/star/form/XResetListener.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCANCELLABLE_HPP_
#include <com/sun/star/util/XCancellable.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XDATABASEPARAMETERBROADCASTER_HPP_
#include <com/sun/star/form/XDatabaseParameterBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XBOUNDCONTROL_HPP_
#include <com/sun/star/form/XBoundControl.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _FMSEARCH_HXX
#include <svx/fmsearch.hxx>
#endif
#ifndef _SV_CLIP_HXX //autogen
#include <vcl/clip.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX //autogen wg. ToolBox
#include <vcl/toolbox.hxx>
#endif
#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _SFX_HRC
#include <sfx2/sfx.hrc>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _COMPHELPER_INTERACTION_HXX_
#include <comphelper/interaction.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XINTERACTIONSUPPLYPARAMETERS_HPP_
#include <com/sun/star/sdb/XInteractionSupplyParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_PARAMETERSREQUEST_HPP_
#include <com/sun/star/sdb/ParametersRequest.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#define GRID_NAME   "MyOneAndOnlyGrid"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::dbtools;
using namespace ::comphelper;

::rtl::OUString URL_CONFIRM_DELETION(::rtl::OUString::createFromAscii(".uno:FormSlots/ConfirmDeletion"));

#define HANDLE_SQL_ERRORS( action, successflag, context, message )          \
    try                                                                     \
    {                                                                       \
        successflag = sal_False;                                                \
        action;                                                             \
        successflag = sal_True;                                                 \
    }                                                                       \
    catch(SQLException& e)                          \
    {                                                                       \
        ::com::sun::star::sdb::SQLContext eExtendedInfo =                                           \
        ::dbtools::prependContextInfo(e, Reference< XInterface > (), context);              \
        ::com::sun::star::sdb::SQLErrorEvent aEvent;                        \
        aEvent.Reason <<= eExtendedInfo;                                    \
        errorOccured(aEvent);                                               \
    }                                                                       \
    catch(...)                                                              \
    {                                                                       \
        DBG_ERROR(message);                                                 \
    }                                                                       \

#define DO_SAFE( action, message ) try { action; } catch(...) { DBG_ERROR(message); } ;

//..................................................................
namespace dbaui
{
//..................................................................

//==================================================================
// OParameterContinuation
//==================================================================
class OParameterContinuation : public OInteraction< XInteractionSupplyParameters >
{
    Sequence< PropertyValue >       m_aValues;

public:
    OParameterContinuation() { }

    Sequence< PropertyValue >   getValues() const { return m_aValues; }

// XInteractionSupplyParameters
    virtual void SAL_CALL setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException);
};

//------------------------------------------------------------------
void SAL_CALL OParameterContinuation::setParameters( const Sequence< PropertyValue >& _rValues ) throw(RuntimeException)
{
    m_aValues = _rValues;
}

//==================================================================
//= OAsyncronousLink
//==================================================================
//------------------------------------------------------------------
OAsyncronousLink::OAsyncronousLink(const Link& _rHandler, NAMESPACE_VOS(OMutex)* _pEventSafety, NAMESPACE_VOS(OMutex)* _pDestructionSafety)
    :m_aHandler(_rHandler)
    ,m_pEventSafety(NULL)
    ,m_pDestructionSafety(NULL)
    ,m_bOwnMutexes(FALSE)
    ,m_nEventId(0)
{
    if (_pEventSafety && _pDestructionSafety)
    {
        m_pEventSafety = _pEventSafety;
        m_pDestructionSafety = _pDestructionSafety;
        m_bOwnMutexes = FALSE;
    }
    else
    {
        m_pEventSafety = new ::vos::OMutex;
        m_pDestructionSafety = new ::vos::OMutex;
        m_bOwnMutexes = TRUE;
    }
}

//------------------------------------------------------------------
OAsyncronousLink::~OAsyncronousLink()
{
    {
        ::vos::OGuard aEventGuard(*m_pEventSafety);
        if (m_nEventId)
            Application::RemoveUserEvent(m_nEventId);
        m_nEventId = 0;
    }

    {
        ::vos::OGuard aDestructionGuard(*m_pDestructionSafety);
        // this is just for the case we're deleted while another thread just handled the event :
        // if this other thread called our link while we were deleting the event here, the
        // link handler blocked. With leaving the above block it continued, but now we are prevented
        // to leave this destructor 'til the link handler recognizes that nEvent == 0 and leaves.
    }
    if (m_bOwnMutexes)
    {
        delete m_pEventSafety;
        delete m_pDestructionSafety;
    }
    m_pEventSafety = NULL;
    m_pDestructionSafety = NULL;
}


//------------------------------------------------------------------
void OAsyncronousLink::Call(void* _pArgument)
{
    ::vos::OGuard aEventGuard(*m_pEventSafety);
    if (m_nEventId)
        Application::RemoveUserEvent(m_nEventId);
    m_nEventId = Application::PostUserEvent(LINK(this, OAsyncronousLink, OnAsyncCall));
}

//------------------------------------------------------------------
void OAsyncronousLink::CancelCall()
{
    ::vos::OGuard aEventGuard(*m_pEventSafety);
    if (m_nEventId)
        Application::RemoveUserEvent(m_nEventId);
    m_nEventId = 0;
}

//------------------------------------------------------------------
IMPL_LINK(OAsyncronousLink, OnAsyncCall, void*, _pArg)
{
    {
        ::vos::OGuard aDestructionGuard(*m_pDestructionSafety);
        {
            ::vos::OGuard aEventGuard(*m_pEventSafety);
            if (!m_nEventId)
                // our destructor deleted the event just while we we're waiting for m_pEventSafety
                // -> get outta here
                return 0;
            m_nEventId = 0;
        }
    }
    if (m_aHandler.IsSet())
        return m_aHandler.Call(_pArg);

    return 0L;
}

//==================================================================
//= SbaXDataBrowserController
//==================================================================
Any SAL_CALL SbaXDataBrowserController::queryInterface(const Type& _rType) throw (RuntimeException)
{
    Any aRet = SbaXDataBrowserController_Base::queryInterface(_rType);
    if(aRet.hasValue())
        return aRet;
    aRet = SbaXDataBrowserController_Base2::queryInterface(_rType);
    if(aRet.hasValue())
        return aRet;
    return OPropertySetHelper::queryInterface(_rType);
}

//------------------------------------------------------------------------------
SbaXDataBrowserController::SbaXDataBrowserController(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
: ::comphelper::OPropertyContainer(m_aPropertyBroadcastHelper)
    ,m_aPropertyBroadcastHelper(m_aPropertyMutex)
    ,m_pContent(NULL)
    ,m_bCurrentlyModified(sal_False)
    ,m_pLoadThread(NULL)
    ,m_bClosingKillOpen(sal_False)
    ,m_nPendingLoadFinished(0)
    ,m_sLoadStopperCaption(ModuleRes(RID_STR_LOADING_DATASOURCE))
    ,m_nFormActionNestingLevel(0)
    ,m_bErrorOccured(false)
    ,m_bFrameUiActive(sal_False)
    ,m_sStateSaveRecord(ModuleRes(RID_STR_SAVE_CURRENT_RECORD))
    ,m_sStateUndoRecord(ModuleRes(RID_STR_UNDO_MODIFY_RECORD))
    ,m_aDisposeListeners(m_aPropertyMutex)
    ,m_xMultiServiceFacatory(_rM)
    ,m_aAsynClose(LINK(this, SbaXDataBrowserController, OnAsyncClose))
    ,m_aAsyncGetCellFocus(LINK(this, SbaXDataBrowserController, OnAsyncGetCellFocus))
    ,m_aAsyncInvalidateAll(LINK(this, SbaXDataBrowserController, OnAsyncInvalidateAll))
{
    try
    {
        m_xUrlTransformer = Reference< ::com::sun::star::util::XURLTransformer > (_rM->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")), UNO_QUERY);
    }
    catch(Exception&)
    {
    }
    static ::rtl::OUString s_sHelpFileName(::rtl::OUString::createFromAscii("database.hlp"));
    sal_Int32 nAttrib = PropertyAttribute::READONLY | PropertyAttribute::TRANSIENT;
    registerProperty(PROPERTY_HELPFILENAME, PROPERTY_ID_HELPFILENAME,nAttrib,&s_sHelpFileName,  ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));

    DBG_ASSERT(m_xUrlTransformer.is(), "SbaXDataBrowserController::SbaXDataBrowserController : could not create the url transformer !");
}

//------------------------------------------------------------------------------
SbaXDataBrowserController::~SbaXDataBrowserController()
{
    m_pContent = NULL;
}
// -----------------------------------------------------------------------------
void SbaXDataBrowserController::initFormatter()
{
    // ---------------------------------------------------------------
    // create a formatter working with the connections format supplier
    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xSupplier(::dbtools::getNumberFormats(::dbtools::getConnection(m_xRowSet), sal_True,m_xMultiServiceFacatory));

    if(xSupplier.is())
    {
        // create a new formatter
        m_xFormatter = Reference< ::com::sun::star::util::XNumberFormatter > (
            m_xMultiServiceFacatory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
        if (m_xFormatter.is())
            m_xFormatter->attachNumberFormatsSupplier(xSupplier);
    }
    else // clear the formatter
        m_xFormatter = NULL;
}
//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::Construct(Window* pParent)
{
    // ---------------------------------------------
    // create/initialize the form and the grid model
    m_xRowSet = CreateForm();
    if (!m_xRowSet.is())
        return sal_False;

    if (!InitializeForm(m_xRowSet))
        return sal_False;

    m_xGridModel = CreateGridModel();
    if (!m_xGridModel.is())
        return sal_False;

    // set the formatter if available
    initFormatter();
    // ---------------------------------------------------------------

    // we want to have a grid with a "flat" border
    Reference< XPropertySet >  xGridSet(m_xGridModel, UNO_QUERY);
    if (xGridSet.is())
        xGridSet->setPropertyValue(PROPERTY_BORDER, makeAny((sal_Int16)2));

    // ----------
    // marry them
    Reference< ::com::sun::star::container::XNameContainer >  xNameCont(m_xRowSet, UNO_QUERY);
    xNameCont->insertByName(::rtl::OUString::createFromAscii(GRID_NAME), makeAny(m_xGridModel));



    // ---------------
    // create the view
    m_pContent = new UnoDataBrowserView(pParent,m_xMultiServiceFacatory);
    if (!m_pContent)
        return sal_False;

    // late construction
    sal_Bool bSuccess = sal_False;
    try
    {
        m_pContent->Construct(getControlModel());
        bSuccess = sal_True;
    }
    catch(SQLException&)
    {
    }
    catch(...)
    {
        DBG_ERROR("SbaXDataBrowserController::Construct : the construction of UnoDataBrowserView failed !");
    }

    if (!bSuccess)
    {
        delete m_pContent;
        m_pContent = NULL;
        return sal_False;
    }
    m_pContent->Show();

    AddSupportedFeatures();

    // want to have a toolbox ?
    ToolBox* pTB = CreateToolBox(m_pContent);
    m_pContent->setToolBox(pTB);
    if (pTB)
        // we want to handle the select
        pTB->SetSelectHdl(LINK(this, SbaXDataBrowserController, OnToolBoxSelected));

    // set the callbacks for the grid control
    SbaGridControl* pVclGrid = m_pContent->getVclControl();
    DBG_ASSERT(pVclGrid, "SbaXDataBrowserController::Construct : have no VCL control !");
    pVclGrid->SetMasterListener(this);

    // --------------------------
    // add listeners ...

    // ... to the form model
    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    if (xFormSet.is())
    {
        xFormSet->addPropertyChangeListener(PROPERTY_ISNEW, (XPropertyChangeListener*)this);
        xFormSet->addPropertyChangeListener(PROPERTY_ISMODIFIED, (XPropertyChangeListener*)this);
        xFormSet->addPropertyChangeListener(PROPERTY_ROWCOUNT, (XPropertyChangeListener*)this);
        xFormSet->addPropertyChangeListener(PROPERTY_ACTIVECOMMAND, (XPropertyChangeListener*)this);
        xFormSet->addPropertyChangeListener(PROPERTY_ORDER, (XPropertyChangeListener*)this);
        xFormSet->addPropertyChangeListener(PROPERTY_FILTER, (XPropertyChangeListener*)this);
        xFormSet->addPropertyChangeListener(PROPERTY_APPLYFILTER, (XPropertyChangeListener*)this);
    }
    Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
    if (xFormError.is())
        xFormError->addSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);

    Reference< ::com::sun::star::form::XLoadable >  xLoadable(getRowSet(), UNO_QUERY);
    if (xLoadable.is())
        xLoadable->addLoadListener(this);

    Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xFormParameter(getRowSet(), UNO_QUERY);
    if (xFormParameter.is())
        xFormParameter->addParameterListener((::com::sun::star::form::XDatabaseParameterListener*)this);

    addModelListeners(getControlModel());
    addControlListeners(m_pContent->getGridControl());

    // -------------
    // load the form
    return LoadForm();
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::LoadForm()
{
    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    sal_Bool bThreadSafe(sal_False);
    try
    {
        bThreadSafe = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_THREADSAFE));
    }
    catch(...)
    {
    }


    m_bLoadCanceled = sal_False;
    if (bThreadSafe)
    {   // load in an own thread so the office doesn't block meanwhile
        m_pLoadThread = new LoadFormThread(getRowSet(), m_sLoadStopperCaption);
        ((LoadFormThread*)m_pLoadThread)->SetTerminationHdl(LINK(this, SbaXDataBrowserController, OnOpenFinished));
        m_pLoadThread->create();

        InvalidateAll();
    }
    else
    {
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(getRowSet(),UNO_QUERY);
        xLoadable->load();
        FormLoaded(sal_True);
    }
    return sal_True;
}
//------------------------------------------------------------------------------
void SbaXDataBrowserController::AddColumnListener(const Reference< XPropertySet > & xCol)
{
    // we're not interested in any column properties ...
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::RemoveColumnListener(const Reference< XPropertySet > & xCol)
{
}
//-------------------------------------------------------------------------
sal_Bool SAL_CALL SbaXDataBrowserController::supportsService(const ::rtl::OUString& ServiceName) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pArray = aSupported.getConstArray();
    for (sal_Int32 i = 0; i < aSupported.getLength(); ++i, ++pArray)
        if (pArray->equals(ServiceName))
            return sal_True;
    return sal_False;
}
// -------------------------------------------------------------------------
void SAL_CALL SbaXDataBrowserController::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
{
    Reference< ::com::sun::star::awt::XWindow >     xParent;
    Reference< ::com::sun::star::frame::XFrame >    xFrame;

    const Any* pBegin   = aArguments.getConstArray();
    const Any* pEnd     = pBegin + aArguments.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        if((*pBegin >>= xFrame) && xFrame.is())
        {
            xParent = xFrame->getContainerWindow();
            VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(xParent);
            Window* pParentWin = pParentComponent ? pParentComponent->GetWindow() : NULL;
            if (!pParentWin)
            {
                throw Exception(::rtl::OUString::createFromAscii("Parent window is null"),*this);
            }

            if(xFrame.is() && Construct(pParentWin))
            {
                xFrame->setComponent(getContent()->GetWindowPeer(), this);
                attachFrame(xFrame);
                pParentComponent->setVisible(sal_True);
            }
        }
    }
}
//------------------------------------------------------------------------------
Reference< XRowSet >  SbaXDataBrowserController::CreateForm()
{
    return Reference< XRowSet > (m_xMultiServiceFacatory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.form.component.Form")), UNO_QUERY);
}

//------------------------------------------------------------------------------
Reference< ::com::sun::star::form::XFormComponent >  SbaXDataBrowserController::CreateGridModel()
{
    return Reference< ::com::sun::star::form::XFormComponent > (m_xMultiServiceFacatory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.form.component.GridControl")), UNO_QUERY);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::addModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    // ... all the grid columns
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns(getControlModel(), UNO_QUERY);
    if (xColumns.is())
    {
        for (sal_uInt16 i=0; i<xColumns->getCount(); ++i)
        {
            Reference< XPropertySet >  xCol = *(Reference< XPropertySet > *)xColumns->getByIndex(i).getValue();
            AddColumnListener(xCol);
        }
    }

    // (we are interested in all columns the grid has (and only in these) so we have to listen to the container, too)
    Reference< ::com::sun::star::container::XContainer >  xColContainer(_xGridControlModel, UNO_QUERY);
    if (xColContainer.is())
        xColContainer->addContainerListener((::com::sun::star::container::XContainerListener*)this);

    Reference< ::com::sun::star::form::XReset >  xReset(_xGridControlModel, UNO_QUERY);
    if (xReset.is())
        xReset->addResetListener((::com::sun::star::form::XResetListener*)this);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::removeModelListeners(const Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel)
{
    // every single column model
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns(getControlModel(), UNO_QUERY);
    if (xColumns.is())
    {
        for (sal_uInt16 i=0; i<xColumns->getCount(); ++i)
        {
            Reference< XPropertySet >  xCol = *(Reference< XPropertySet > *)xColumns->getByIndex(i).getValue();
            RemoveColumnListener(xCol);
        }
    }

    Reference< ::com::sun::star::container::XContainer >  xColContainer(getControlModel(), UNO_QUERY);
    if (xColContainer.is())
        xColContainer->removeContainerListener((::com::sun::star::container::XContainerListener*)this);

    Reference< ::com::sun::star::form::XReset >  xReset(getControlModel(), UNO_QUERY);
    if (xReset.is())
        xReset->removeResetListener((::com::sun::star::form::XResetListener*)this);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::addControlListeners(const Reference< ::com::sun::star::awt::XControl > & _xGridControl)
{
    // to ge the 'modified' for the current cell
    Reference< ::com::sun::star::util::XModifyBroadcaster >  xBroadcaster(m_pContent->getGridControl(), UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->addModifyListener((::com::sun::star::util::XModifyListener*)this);

    // introduce ourself as dispatch provider for the grid
    Reference< ::com::sun::star::frame::XDispatchProviderInterception >  xInterception(m_pContent->getGridControl(), UNO_QUERY);
    if (xInterception.is())
        xInterception->registerDispatchProviderInterceptor((::com::sun::star::frame::XDispatchProviderInterceptor*)this);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::removeControlListeners(const Reference< ::com::sun::star::awt::XControl > & _xGridControl)
{
    Reference< ::com::sun::star::util::XModifyBroadcaster >  xBroadcaster(_xGridControl, UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->removeModifyListener((::com::sun::star::util::XModifyListener*)this);

    Reference< ::com::sun::star::frame::XDispatchProviderInterception >  xInterception(_xGridControl, UNO_QUERY);
    if (xInterception.is())
        xInterception->releaseDispatchProviderInterceptor((::com::sun::star::frame::XDispatchProviderInterceptor*)this);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingGridControl(const ::com::sun::star::lang::EventObject& Source)
{
    removeControlListeners(m_pContent->getGridControl());
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingGridModel(const ::com::sun::star::lang::EventObject& Source)
{
    removeModelListeners(getControlModel());
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingFormModel(const ::com::sun::star::lang::EventObject& Source)
{
    Reference< XPropertySet >  xSourceSet(Source.Source, UNO_QUERY);
    if (xSourceSet.is())
    {
        xSourceSet->removePropertyChangeListener(PROPERTY_ISNEW, (XPropertyChangeListener*)this);
        xSourceSet->removePropertyChangeListener(PROPERTY_ISMODIFIED, (XPropertyChangeListener*)this);
        xSourceSet->removePropertyChangeListener(PROPERTY_ROWCOUNT, (XPropertyChangeListener*)this);
        xSourceSet->removePropertyChangeListener(PROPERTY_ACTIVECOMMAND, (XPropertyChangeListener*)this);
        xSourceSet->removePropertyChangeListener(PROPERTY_ORDER, (XPropertyChangeListener*)this);
        xSourceSet->removePropertyChangeListener(PROPERTY_FILTER, (XPropertyChangeListener*)this);
        xSourceSet->removePropertyChangeListener(PROPERTY_APPLYFILTER, (XPropertyChangeListener*)this);
    }

    Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(Source.Source, UNO_QUERY);
    if (xFormError.is())
        xFormError->removeSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);

    Reference< ::com::sun::star::form::XLoadable >  xLoadable(getRowSet(), UNO_QUERY);
    if (xLoadable.is())
        xLoadable->removeLoadListener(this);

    Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xFormParameter(Source.Source, UNO_QUERY);
    if (xFormParameter.is())
        xFormParameter->removeParameterListener((::com::sun::star::form::XDatabaseParameterListener*)this);
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposingColumnModel(const ::com::sun::star::lang::EventObject& Source)
{
    RemoveColumnListener(Reference< XPropertySet > (Source.Source, UNO_QUERY));
}

// -------------------------------------------------------------------------
void SbaXDataBrowserController::disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException )
{
    // is it the grid control ?
    if (m_pContent)
    {
        Reference< ::com::sun::star::awt::XControl >  xSourceControl(Source.Source, UNO_QUERY);
        if (xSourceControl == m_pContent->getGridControl())
            disposingGridControl(Source);
    }

    // it's model (the container of the columns) ?
    if (getControlModel() == Source.Source)
        disposingGridModel(Source);

    // the form's model ?
    if ((getRowSet() == Source.Source))
        disposingFormModel(Source);

    // from a single column model ?
    Reference< XPropertySet >  xSourceSet(Source.Source, UNO_QUERY);
    if (xSourceSet.is())
    {
        Reference< XPropertySetInfo >  xInfo = xSourceSet->getPropertySetInfo();
        // we assume that columns have a Width property and all other sets we are listening to don't have
        if (xInfo->hasPropertyByName(PROPERTY_WIDTH))
            disposingColumnModel(Source);
    }

    // our frame ?
    Reference< ::com::sun::star::frame::XFrame >  xSourceFrame(Source.Source, UNO_QUERY);
    if (((::com::sun::star::frame::XFrame*)xSourceFrame.get() == (::com::sun::star::frame::XFrame*)m_xCurrentFrame.get()) && m_xCurrentFrame.is())
        m_xCurrentFrame->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::propertyChange(const PropertyChangeEvent& evt)
{
    Reference< XPropertySet >  xSource(evt.Source, UNO_QUERY);
    if (!xSource.is())
        return;

    ::vos::OGuard aGuard(Application::GetSolarMutex());
    // the IsModified changed to sal_False ?
    if  (   (evt.PropertyName.equals(PROPERTY_ISMODIFIED))
        &&  (::comphelper::getBOOL(evt.NewValue) == sal_False)
        )
    {   // -> the current field isn't modified anymore, too
        m_bCurrentlyModified = sal_False;
        InvalidateFeature(::rtl::OUString::createFromAscii(".uno:Save"));
        InvalidateFeature(ID_BROWSER_UNDO);
    }

    // switching to a new record ?
    if  (   (evt.PropertyName.equals(PROPERTY_ISNEW))
        &&  (::comphelper::getBOOL(evt.NewValue) == sal_True)
        )
    {
        if (::comphelper::getINT32(xSource->getPropertyValue(PROPERTY_ROWCOUNT)) == 0)
            // if we're switching to a new record and didn't have any records before we need to invalidate
            // all slots (as the cursor was invalid before the mode change and so the slots were disabled)
            InvalidateAll();
    }

    // the filter or the sort criterias have changed ? -> update our parser
    if (m_xParser.is())
    {
        if (evt.PropertyName.equals(PROPERTY_ACTIVECOMMAND))
        {
            DO_SAFE( m_xParser->setQuery(::comphelper::getString(evt.NewValue)), "SbaXDataBrowserController::propertyChange : could not forward the new query to my parser !" );
        }
        else if (evt.PropertyName.equals(PROPERTY_FILTER))
        {
            if (m_xParser->getFilter() != ::comphelper::getString(evt.NewValue))
            {
                DO_SAFE( m_xParser->setFilter(::comphelper::getString(evt.NewValue)), "SbaXDataBrowserController::propertyChange : could not forward the new filter to my parser !" );
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
        }
        else if (evt.PropertyName.equals(PROPERTY_ORDER))
        {
            if (m_xParser->getOrder() != ::comphelper::getString(evt.NewValue))
            {
                DO_SAFE( m_xParser->setOrder(::comphelper::getString(evt.NewValue)), "SbaXDataBrowserController::propertyChange : could not forward the new order to my parser !" );
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
        }
    }

    // a new record count ? -> may be our search availability has changed
    if (evt.PropertyName.equals(PROPERTY_ROWCOUNT))
        InvalidateFeature(ID_BROWSER_SEARCH);
}

//------------------------------------------------------------------------
void SbaXDataBrowserController::modified(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    m_bCurrentlyModified = sal_True;
    // das muss vom ::com::sun::star::form::GridControl kommen
    InvalidateFeature(::rtl::OUString::createFromAscii(".uno:Save"));
    InvalidateFeature(ID_BROWSER_UNDO);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementInserted(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    Reference< XPropertySet >  xNewColumn(*(Reference< XPropertySet > *)evt.Element.getValue());
    AddColumnListener(xNewColumn);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementRemoved(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    Reference< XPropertySet >  xOldColumn(*(Reference< XPropertySet > *)evt.Element.getValue());
    RemoveColumnListener(xOldColumn);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::elementReplaced(const ::com::sun::star::container::ContainerEvent& evt) throw( RuntimeException )
{
    Reference< XPropertySet >  xOldColumn(*(Reference< XPropertySet > *)evt.ReplacedElement.getValue());
    RemoveColumnListener(xOldColumn);

    Reference< XPropertySet >  xNewColumn(*(Reference< XPropertySet > *)evt.Element.getValue());
    AddColumnListener(xNewColumn);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::attachFrame(const Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( RuntimeException )
{
    if (m_xCurrentFrame.is())
        m_xCurrentFrame->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);

    m_xCurrentFrame = xFrame;
    if (m_xCurrentFrame.is())
    {
        m_xCurrentFrame->addFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);
        m_bFrameUiActive = m_xCurrentFrame->isActive();
    }
}

// -----------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::suspend(sal_Bool bSuspend) throw( RuntimeException )
{
    m_bSuspending = sal_True;

    // have a pending open operation ?
    if (PendingLoad())
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
        if (m_nPendingLoadFinished != 0)
        {   // clean up directly. Otherwise there may be a pending asynchronous call
            // to OnOpenFinishedMainThread, which won't be executed before we leave
            // this method. Sounds like a classic infinite loop.
            Application::RemoveUserEvent(m_nPendingLoadFinished);
            LINK(this, SbaXDataBrowserController, OnOpenFinishedMainThread).Call(NULL);
        }
        else
        {   // set m_bClosingKillOpen to ensure that the our termination handler reacts according
            // it's context
            m_bClosingKillOpen = sal_True;

            // normally we would now just wait for termination of the load thread, but there is a small problem :
            // In the current thread the global solar mutex is locked (that's for sure). If the loading of the
            // form tries to acquire (blocking) the solar mutex, too, and we loop waiting for the other thread
            // we have a classic deadlock. And bet your ass that ANYBODY in the foreign thread tries to lock
            // the solar mutex. Almost all the UNO-capsules around normal C++ classes use the solar mutex for
            // "thread safety" (which doesn't deserve that name anymore ;), e.g. the ::com::sun::star::util::XNumberFormatter-implementation
            // does.
            // So we have to do a fake : we tell the loading thread that we aren't interested in the results anymore
            // and the thread deletes itself (and the data source) as soon as it is done. As it holds the last
            // references to the form (and thus, indirectly, to the grid) they will be cleared as soon as the thread dies.
            // So all is fine. Except the small flaw that the form is still loading in the background while the
            // window that should display it is already dead.
            // If we could release the solar mutex in this thread and block it 'til the loader is finished we could
            // solve it in a less dirty way, but uinfortunatelly we don't know how often this thread acquired the mutex.
            // With high effort we could reach this with releasing the mutex until a third thread - which has to be
            // created - can acquire it.Then we block, the third thread releases the mutex (and dies) and the loader
            // thread would be able to finish. But that sounds difficult and fault-prone, so I think it's not worth it ...
            ((LoadFormThread*)m_pLoadThread)->SetTerminationHdl(Link());
            // and of course we tell the thread to stop ....
            ((LoadFormThread*)m_pLoadThread)->StopIt();
        }

    }
    DBG_ASSERT(m_nPendingLoadFinished == 0, "SbaXDataBrowserController::suspend : there shouldn't be a pending load !");

    m_aAsynClose.CancelCall();
    m_aAsyncGetCellFocus.CancelCall();
    m_aAsyncInvalidateAll.CancelCall();

    sal_Bool bReturn = SaveData(sal_True, sal_False);

    m_bSuspending = sal_False;
    return bReturn;
}
// -----------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::ImplInvalidateTBItem(sal_uInt16 nId, const FeatureState& rState)
{
    ToolBox* pTB = m_pContent ? m_pContent->getToolBox() : NULL;
    if (!pTB || (pTB->GetItemPos(nId) == TOOLBOX_ITEM_NOTFOUND))
        return sal_False;

    pTB->EnableItem(nId, rState.bEnabled);
    switch (rState.aState.getValueTypeClass())
    {
        case TypeClass_BOOLEAN:
            pTB->CheckItem(nId, ::comphelper::getBOOL(rState.aState));
            break;

        case TypeClass_STRING:
            if (pTB->GetItemWindow(nId))
                pTB->GetItemWindow(nId)->SetText(::comphelper::getString(rState.aState));
            break;

        case TypeClass_VOID:
            break;

        default:
            DBG_WARNING("SbaXDataBrowserController::ImplInvalidateTBItem : don't know what to do with the item state !");
    }
    return sal_True;
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::ImplBroadcastFeatureState(const ::rtl::OUString& _rFeature, const Reference< ::com::sun::star::frame::XStatusListener > & xListener, sal_Bool _bIgnoreCache)
{
    sal_Int32 nFeat = m_aSupportedFeatures[_rFeature];
    ::com::sun::star::frame::FeatureStateEvent aEvent;
    FeatureState aFeatState( GetState(nFeat));

    // check if we really need to notify the listeners : this method may be called much more often than needed, so check
    // the cached state of the feature
    sal_Bool bAlreadyCached = (m_aStateCache.find(nFeat) != m_aStateCache.end());
    FeatureState& rCachedState = m_aStateCache[nFeat];  // creates if neccessary
    if (bAlreadyCached && !_bIgnoreCache && (rCachedState.bEnabled == aFeatState.bEnabled))
    {   // the enabled flag hasn't changed, maybe the state ?
        if (rCachedState.aState.getValueTypeClass() == aFeatState.aState.getValueTypeClass())
        {   // at least the type of the state hasn't
            sal_Bool bEqualValue = sal_False;
            switch (rCachedState.aState.getValueTypeClass())
            {
                case TypeClass_VOID:
                    bEqualValue = !aFeatState.aState.hasValue();
                    break;
                case TypeClass_BOOLEAN:
                    bEqualValue = ::comphelper::getBOOL(rCachedState.aState) == ::comphelper::getBOOL(aFeatState.aState);
                    break;
                case TypeClass_SHORT:
                    bEqualValue = ::comphelper::getINT16(rCachedState.aState) == ::comphelper::getINT16(aFeatState.aState);
                    break;
                case TypeClass_LONG:
                    bEqualValue = ::comphelper::getINT32(rCachedState.aState) == ::comphelper::getINT32(aFeatState.aState);
                    break;
                case TypeClass_STRING:
                    bEqualValue = ::comphelper::getString(rCachedState.aState).equals(::comphelper::getString(aFeatState.aState));
                    break;
                default:
                    DBG_ERROR("SbaXDataBrowserController::ImplBroadcastFeatureState : unknown state type (not implemented yet) !");
                    break;
            }
            if (bEqualValue)
                return;
        }
    }
    rCachedState = aFeatState;

    aEvent.FeatureURL.Complete = _rFeature;
    if (m_xUrlTransformer.is())
        m_xUrlTransformer->parseStrict(aEvent.FeatureURL);
    aEvent.Source = (::com::sun::star::frame::XDispatch*)this;
    aEvent.IsEnabled = aFeatState.bEnabled;
    aEvent.Requery = aFeatState.bRequery;
    aEvent.State = aFeatState.aState;

    // a special listener ?
    if (xListener.is())
        xListener->statusChanged(aEvent);
    else
    {   // no -> iterate through all listeners responsible for the ::com::sun::star::util::URL
        DispatchIterator iterSearch = m_arrStatusListener.begin();
        DispatchIterator iterEnd = m_arrStatusListener.end();

        while (iterSearch != iterEnd)
        {
            DispatchTarget& rCurrent = *iterSearch;
            if (rCurrent.aURL.Complete.equals(_rFeature))
                rCurrent.xListener->statusChanged(aEvent);
            ++iterSearch;
        }
    }

    // give the TB a chance
    ImplInvalidateTBItem(nFeat, aFeatState);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::InvalidateFeature(const ::rtl::OUString& rURLPath, const Reference< ::com::sun::star::frame::XStatusListener > & xListener, sal_Bool _bForceBroadcast)
{
    ImplBroadcastFeatureState(rURLPath, xListener, _bForceBroadcast);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::InvalidateFeature(sal_uInt16 _nId, const Reference< ::com::sun::star::frame::XStatusListener > & _xListener, sal_Bool _bForceBroadcast)
{
    sal_Bool bFound = sal_False;
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
    for(;aIter != m_aSupportedFeatures.end();++aIter)
    {
        if(aIter->second == _nId)
        {
            ImplBroadcastFeatureState(aIter->first,_xListener, _bForceBroadcast);
            bFound = sal_True;
        }
    }
    if(!bFound)
        ImplInvalidateTBItem(_nId, GetState(_nId));
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::InvalidateAll(sal_Bool bAsync)
{
    if (bAsync)
    {
        m_aAsyncInvalidateAll.Call();
        return;
    }

    sal_uInt16 i;
    // ---------------------------------
    // invalidate all aupported features
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
    for(;aIter != m_aSupportedFeatures.end();++aIter)
    {
        ImplBroadcastFeatureState(aIter->first, Reference< ::com::sun::star::frame::XStatusListener > (), sal_True);
    }

    // ------------------------------
    // invalidate all slots in the TB (if any)
    ToolBox* pTB = getContent() ? getContent()->getToolBox() : NULL;
    if (pTB)
    {
        for (i=0; i<pTB->GetItemCount(); ++i)
            if (pTB->GetItemId(i))
                InvalidateFeature(pTB->GetItemId(i));
    }
}

// -----------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatch >  SbaXDataBrowserController::queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( RuntimeException )
{
    if (aURL.Complete.equals(URL_CONFIRM_DELETION))
        return (::com::sun::star::frame::XDispatch*)this;

    if (m_aSupportedFeatures.find(aURL.Complete) != m_aSupportedFeatures.end())
        return (::com::sun::star::frame::XDispatch*)this;

    if (m_xSlaveDispatcher.is())
        return m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);

    return Reference< ::com::sun::star::frame::XDispatch > ();
}

// -----------------------------------------------------------------------
Sequence< Reference< ::com::sun::star::frame::XDispatch > > SbaXDataBrowserController::queryDispatches(const Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw( RuntimeException )
{
    Sequence< Reference< ::com::sun::star::frame::XDispatch > > aReturn;
    if (!aDescripts.getLength())
        return aReturn;

    aReturn.realloc(aDescripts.getLength());
    Reference< ::com::sun::star::frame::XDispatch > * pReturn = aReturn.getArray();

    const ::com::sun::star::frame::DispatchDescriptor* pDescripts = aDescripts.getConstArray();
    for (sal_uInt16 i=0; i<aDescripts.getLength(); ++i, ++pDescripts, ++pReturn)
        *pReturn = queryDispatch(pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags);

    return aReturn;
}

// -----------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatchProvider >  SbaXDataBrowserController::getSlaveDispatchProvider(void) throw( RuntimeException )
{
    return m_xSlaveDispatcher;
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::setSlaveDispatchProvider(const Reference< ::com::sun::star::frame::XDispatchProvider > & _xNewProvider) throw( RuntimeException )
{
    m_xSlaveDispatcher = _xNewProvider;
}

// -----------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatchProvider >  SbaXDataBrowserController::getMasterDispatchProvider(void) throw( RuntimeException )
{
    return m_xMasterDispatcher;
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::setMasterDispatchProvider(const Reference< ::com::sun::star::frame::XDispatchProvider > & _xNewProvider) throw( RuntimeException )
{
    m_xMasterDispatcher = _xNewProvider;
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::dispatch(const ::com::sun::star::util::URL& aURL, const Sequence< PropertyValue >& aArgs)
{
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find(aURL.Complete);
    if (aIter != m_aSupportedFeatures.end())
        Execute(aIter->second);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::addStatusListener(const Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL)
{
    // remeber the listener together with the ::com::sun::star::util::URL
    m_arrStatusListener.insert(m_arrStatusListener.end(), DispatchTarget(aURL, aListener));

    // initially broadcast the state
    InvalidateFeature(aURL.Complete, aListener, sal_True);
        // force the new state to be broadcasted to the new listener
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::removeStatusListener(const Reference< ::com::sun::star::frame::XStatusListener > & aListener, const ::com::sun::star::util::URL& aURL)
{
    DispatchIterator iterSearch = m_arrStatusListener.begin();
    DispatchIterator iterEnd = m_arrStatusListener.end();

    sal_Bool bRemoveForAll = (aURL.Complete.getLength() == 0);
    while (iterSearch != iterEnd)
    {
        DispatchTarget& rCurrent = *iterSearch;
        if  (   (rCurrent.xListener == aListener)
            &&  (   bRemoveForAll
                ||  (rCurrent.aURL.Complete.equals(aURL.Complete))
                )
            )
        {
            if (!bRemoveForAll)
            {   // remove the listener only for the given ::com::sun::star::util::URL, so we can exit the loop after deletion
                m_arrStatusListener.erase(iterSearch);
                break;
            }
            else
            {   // we have to remove the listener for all URLs, so a simple erase isn't sufficient (as the iterator may be invalid then)
                sal_Int32 nOffset = iterSearch - m_arrStatusListener.begin();
                m_arrStatusListener.erase(iterSearch);
                iterSearch = m_arrStatusListener.begin() + nOffset;
                iterEnd = m_arrStatusListener.end();
            }
        }
        else
            ++iterSearch;
    }

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find(aURL.Complete);
    if (aIter != m_aSupportedFeatures.end())
    {   // clear the cache for that feature
        StateCacheIterator aCachePos = m_aStateCache.find(aIter->second);
        if (aCachePos != m_aStateCache.end())
            m_aStateCache.erase(aCachePos);
    }
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::dispose()
{
    // say our dispose listeners goodbye
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (XWeak*)(::cppu::OWeakObject*)this;
    m_aDisposeListeners.disposeAndClear(aEvt);

    // our status listeners, too
    while (m_arrStatusListener.size() > 0)
    {
        DispatchIterator iterCurrent = m_arrStatusListener.begin();

        DispatchTarget& rCurrent = *iterCurrent;
        ::com::sun::star::lang::EventObject aDisposeEvent;
        aDisposeEvent.Source = (::com::sun::star::lang::XComponent*)this;

#ifdef DBG_UTIL
        sal_Int32 nSize = m_arrStatusListener.size();
#endif
        rCurrent.xListener->disposing(aDisposeEvent);
        DBG_ASSERT(nSize > m_arrStatusListener.size(), "SbaXDataBrowserController::dispose : the listener did not call removeStatusListener !");
            // in disposing the status listener should remove itself via removeStatusListener, therein we remove it from
            // m_arrStatusListener, so the size should have decreased.
    }

    // check out from all the objects we are listening
    // the frame
    if (m_xCurrentFrame.is())
        m_xCurrentFrame->removeFrameActionListener((::com::sun::star::frame::XFrameActionListener*)this);

    if (!PendingLoad())
    {
        // don't do the removeXxxListener calls if there is a pending load, this may lead to a deadlock :
        // as in this thread the SolarMutex is locked (that's for sure) and removeXxxListener locks
        // the form's mutex. But in the loading thread both mutexes are acquired in reverse order.
        // That's no problem that we don't remove ourself here, as the load thread is responsible for the form
        // at the moment. So if the loading is finished, the form will be disposed (by the load thread), and
        // we get the "disposing" event where we can do the removeXxxListener calls.
        // The alternative for this handling would be that the form has two mutexes : one for handling it's
        // listeners and properties and so on, on for it's pure cursor actions

        // the data source
        Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
        if (xFormSet.is())
        {
            xFormSet->removePropertyChangeListener(PROPERTY_ISNEW, (XPropertyChangeListener*)this);
            xFormSet->removePropertyChangeListener(PROPERTY_ISMODIFIED, (XPropertyChangeListener*)this);
            xFormSet->removePropertyChangeListener(PROPERTY_ROWCOUNT, (XPropertyChangeListener*)this);
            xFormSet->removePropertyChangeListener(PROPERTY_ACTIVECOMMAND, (XPropertyChangeListener*)this);
            xFormSet->removePropertyChangeListener(PROPERTY_ORDER, (XPropertyChangeListener*)this);
            xFormSet->removePropertyChangeListener(PROPERTY_FILTER, (XPropertyChangeListener*)this);
            xFormSet->removePropertyChangeListener(PROPERTY_APPLYFILTER, (XPropertyChangeListener*)this);
        }

        Reference< ::com::sun::star::sdb::XSQLErrorBroadcaster >  xFormError(getRowSet(), UNO_QUERY);
        if (xFormError.is())
            xFormError->removeSQLErrorListener((::com::sun::star::sdb::XSQLErrorListener*)this);

        Reference< ::com::sun::star::form::XLoadable >  xLoadable(getRowSet(), UNO_QUERY);
        if (xLoadable.is())
            xLoadable->removeLoadListener(this);

        Reference< ::com::sun::star::form::XDatabaseParameterBroadcaster >  xFormParameter(getRowSet(), UNO_QUERY);
        if (xFormParameter.is())
            xFormParameter->removeParameterListener((::com::sun::star::form::XDatabaseParameterListener*)this);
    }

    removeModelListeners(getControlModel());

    if (m_pContent)
    {
        removeControlListeners(m_pContent->getGridControl());
        // don't delete explicitly, this is done by the owner (and user) of this controller (me hopes ...)
        m_pContent = NULL;
    }

    if(m_aInvalidateClipboard.IsActive())
        m_aInvalidateClipboard.Stop();

    // dispose the data source
    // if there is a pending load we decided to give the responsibility for the data source to the open thread
    // (see ::suspend)
    if (!PendingLoad())
    {
        Reference< ::com::sun::star::lang::XComponent >  xDataSourceComponent(m_xRowSet, UNO_QUERY);
        if (xDataSourceComponent.is())
            xDataSourceComponent->dispose();
    }
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::addEventListener(const Reference< ::com::sun::star::lang::XEventListener > & aListener)
{
    m_aDisposeListeners.addInterface(aListener);
}

// -----------------------------------------------------------------------
void SbaXDataBrowserController::removeEventListener(const Reference< ::com::sun::star::lang::XEventListener > & aListener)
{
    m_aDisposeListeners.removeInterface(aListener);
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( RuntimeException )
{
    if ((::com::sun::star::frame::XFrame*)aEvent.Frame.get() == (::com::sun::star::frame::XFrame*)m_xCurrentFrame.get())
        switch (aEvent.Action)
        {
            case ::com::sun::star::frame::FrameAction_FRAME_UI_ACTIVATED:
                m_bFrameUiActive = sal_True;
                // ensure that the active cell (if any) has the focus
                m_aAsyncGetCellFocus.Call();
                // start the clipboard timer
                if (m_pContent && m_pContent->getVclControl() && !m_aInvalidateClipboard.IsActive())
                {
                    m_aInvalidateClipboard.SetTimeout(300);
                    m_aInvalidateClipboard.Start();
                }
                break;
            case ::com::sun::star::frame::FrameAction_FRAME_UI_DEACTIVATING:
                m_bFrameUiActive = sal_False;
                // stop the clipboard invalidator
                if (m_pContent && m_pContent->getVclControl() && m_aInvalidateClipboard.IsActive())
                {
                    m_aInvalidateClipboard.Stop();
                    LINK(this, SbaXDataBrowserController, OnInvalidateClipboard).Call(NULL);
                }
                // remove the "get cell focus"-event
                m_aAsyncGetCellFocus.CancelCall();
                break;
        }
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::errorOccured(const ::com::sun::star::sdb::SQLErrorEvent& aEvent) throw( RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if (::comphelper::isAssignableFrom(aEvent.Reason.getValueType(),::getCppuType((const SQLException*)0)))
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
        SvxDBMsgBox aDlg(NULL, *(SQLException*)aEvent.Reason.getValue());
        aDlg.Execute();
    }
    if (m_nFormActionNestingLevel)
        m_bErrorOccured = true;
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::approveParameter(const ::com::sun::star::form::DatabaseParameterEvent& aEvent) throw( RuntimeException )
{
    if (aEvent.Source != getRowSet())
    {
        // not my data source -> allow anything
        DBG_ERROR("SbaXDataBrowserController::approveParameter : invalid event source !");
        return sal_True;
    }

    Reference< ::com::sun::star::container::XIndexAccess >  xParameters = aEvent.Parameters;
    ::vos::OClearableGuard aGuard(Application::GetSolarMutex());
        // this may be executed in a non-main thread and we want to use vcl ...
    Window* pParent = Application::GetDefDialogParent();
        // don't use the content as parent if it isn't visible
    // (and don't use NULL as parent : this may be displayed in the beamer and really shouldn't be task-local)
        // 69297 - FS - 25.10.99
    if (m_pContent && m_pContent->IsVisible())
        pParent = m_pContent;

    // default handling: instantiate an interaction handler and let it handle the parameter request
    try
    {
        // two continuations allowed: OK and Cancel
        OParameterContinuation* pParamValues = new OParameterContinuation;
        OInteractionAbort* pAbort = new OInteractionAbort;
        // the request
        ParametersRequest aRequest;
        aRequest.Parameters = xParameters;
        aRequest.Connection = getConnection(Reference< XRowSet >(aEvent.Source, UNO_QUERY));
        OInteractionRequest* pParamRequest = new OInteractionRequest(makeAny(aRequest));
        Reference< XInteractionRequest > xParamRequest(pParamRequest);
        // some knittings
        pParamRequest->addContinuation(pParamValues);
        pParamRequest->addContinuation(pAbort);

        // create the handler, let it handle the request
        Reference< XInteractionHandler > xHandler(getProcessServiceFactory()->createInstance(SERVICE_SDB_INTERACTION_HANDLER), UNO_QUERY);
        if (xHandler.is())
        {
            ::vos::OGuard aGuard(Application::GetSolarMutex());
            xHandler->handle(xParamRequest);
        }

        if (!pParamValues->wasSelected())
        {   // canceled
            m_bLoadCanceled = sal_True;
            return sal_False;
        }

        // transfer the values into the parameter supplier
        Sequence< PropertyValue > aFinalValues = pParamValues->getValues();
        if (aFinalValues.getLength() != aRequest.Parameters->getCount())
        {
            DBG_ERROR("SbaXDataBrowserController::approveParameter: the InteractionHandler returned nonsense!");
            m_bLoadCanceled = sal_True;
            return sal_False;
        }
        const PropertyValue* pFinalValues = aFinalValues.getConstArray();
        for (sal_Int32 i=0; i<aFinalValues.getLength(); ++i, ++pFinalValues)
        {
            Reference< XPropertySet > xParam;
            ::cppu::extractInterface(xParam, aRequest.Parameters->getByIndex(i));
            DBG_ASSERT(xParam.is(), "SbaXDataBrowserController::approveParameter: one of the parameters is no property set!");
            if (xParam.is())
            {
#ifdef DBG_UTIL
                ::rtl::OUString sName;
                xParam->getPropertyValue(PROPERTY_NAME) >>= sName;
                DBG_ASSERT(sName.equals(pFinalValues->Name), "SbaXDataBrowserController::approveParameter: suspicious value names!");
#endif
                try { xParam->setPropertyValue(PROPERTY_VALUE, pFinalValues->Value); }
                catch(Exception&)
                {
                    DBG_ERROR("SbaXDataBrowserController::approveParameter: setting one of the properties failed!");
                }
            }
        }
    }
    catch(Exception&)
    {
        DBG_ERROR("SbaXDataBrowserController::approveParameter: caught an Exception (tried to let the InteractionHandler handle it)!");
    }

    return sal_True;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::loaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    // not interested in
    // we're loading within an separate thread and have a handling  for it's "finished event"
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    // not interested in
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    InvalidateAll(sal_True);
        // do this asynchron, there are other listeners reacting on this message ...
        // (it's a little hack : the grid columns are listening to this event, too, and their bound field may
        // change as a reaction on that event. as we have no chance to be notified of this change (which is
        // the one we're interested in) we give them time to do what they want to before invalidating our
        // bound-field-dependent slots ....
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::reloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    // not interested in
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    InvalidateAll(sal_True);
        // do this asynchron, there are other listeners reacting on this message ...
        // (it's a little hack : the grid columns are listening to this event, too, and their bound field may
        // change as a reaction on that event. as we have no chance to be notified of this change (which is
        // the one we're interested in) we give them time to do what they want to before invalidating our
        // bound-field-dependent slots ....
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::approveReset(const ::com::sun::star::lang::EventObject& rEvent) throw( RuntimeException )
{
    return sal_True;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::resetted(const ::com::sun::star::lang::EventObject& rEvent) throw( RuntimeException )
{
    DBG_ASSERT(rEvent.Source == getControlModel(), "SbaXDataBrowserController::resetted : where did this come from ?");
    m_bCurrentlyModified = sal_False;
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::confirmDelete(const ::com::sun::star::sdb::RowChangeEvent& aEvent) throw( RuntimeException )
{
    if (QueryBox(m_pContent, ModuleRes(QUERY_BRW_DELETE_ROWS)).Execute() != RET_YES)
        return sal_False;

    return sal_True;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::EmptyWindow()
{
    // dispatch en empty ::com::sun::star::util::URL so we will be cleaned up
    Reference< ::com::sun::star::frame::XDispatchProvider >  xProvider(m_xCurrentFrame, UNO_QUERY);
    if (xProvider.is())
    {
        Reference< ::com::sun::star::frame::XDispatch >  xDispatcher = xProvider->queryDispatch(::com::sun::star::util::URL(), m_xCurrentFrame->getName(), 0);
        if (xDispatcher.is())
            xDispatcher->dispatch(::com::sun::star::util::URL(), Sequence< PropertyValue >());
    }
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::AddSupportedFeatures()
{
    // add all supported features
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Copy")] = ID_BROWSER_COPY;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Cut")] = ID_BROWSER_CUT;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:EditDoc")] = ID_BROWSER_EDITDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:FormSlots/undoRecord")] = ID_BROWSER_UNDORECORD;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Paste")] = ID_BROWSER_PASTE;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Redo")] = ID_BROWSER_REDO;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Save")] = ID_BROWSER_SAVEDOC;
    m_aSupportedFeatures[ ::rtl::OUString::createFromAscii(".uno:Undo")] = ID_BROWSER_UNDO;
}

//------------------------------------------------------------------------------
FeatureState SbaXDataBrowserController::GetState(sal_uInt16 nId)
{
    FeatureState aReturn;
        // (disabled automatically)

    try
    {
        // no chance without a view
        if (!m_pContent || !m_pContent->getVclControl())
            return aReturn;
        // no chance without valid models
        if (isValid() && !isValidCursor())
            return aReturn;
        // no chance while loading the form
        if (PendingLoad())
            return aReturn;

        switch (nId)
        {
            case ID_BROWSER_SEARCH:
            {
                Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                sal_Int32 nCount = ::comphelper::getINT32(xFormSet->getPropertyValue(PROPERTY_ROWCOUNT));
                aReturn.bEnabled = nCount != 0;
            }
            break;

            case ID_BROWSER_COPY:
            case ID_BROWSER_CUT:
            case ID_BROWSER_PASTE:
            {
                DbCellControllerRef xCurrentController = m_pContent->getVclControl()->Controller();
                if (xCurrentController.Is() && xCurrentController->ISA(DbEditCellController))
                {
                    Edit& rEdit = (Edit&)xCurrentController->GetWindow();
                    sal_Bool bHasLen = (rEdit.GetSelection().Len() != 0);
                    sal_Bool bIsReadOnly = rEdit.IsReadOnly();
                    switch (nId)
                    {
                        case ID_BROWSER_CUT : aReturn.bEnabled = m_bFrameUiActive && bHasLen && !bIsReadOnly; break;
                        case SID_COPY   : aReturn.bEnabled = m_bFrameUiActive && bHasLen; break;
                        case ID_BROWSER_PASTE   : aReturn.bEnabled = m_bFrameUiActive && !bIsReadOnly && Clipboard::HasFormat(FORMAT_STRING); break;
                    }
                }
            }
            break;

            case ID_BROWSER_SORTUP:
            case ID_BROWSER_SORTDOWN:
            case ID_BROWSER_AUTOFILTER:
            {
                // a native statement can't be filtered or sorted
                if (!m_xParser.is())
                    break;

                Reference< XPropertySet >  xCurrentField = getBoundField();
                if (!xCurrentField.is())
                    break;

                aReturn.bEnabled = ::comphelper::getBOOL(xCurrentField->getPropertyValue(PROPERTY_ISSEARCHABLE));
            }
            break;

            case ID_BROWSER_ORDERCRIT:
            case ID_BROWSER_FILTERCRIT:
                // we are not in the handle column
                aReturn.bEnabled = m_pContent->getVclControl()->GetCurColumnId() != 0;
                // a native statement can't be filtered or sorted
                aReturn.bEnabled &= m_xParser.is();
                break;

            case ID_BROWSER_REMOVEFILTER:
                if (!m_xParser.is())
                    break;
                // any filter or sort order set ?
                aReturn.bEnabled = m_xParser->getFilter().len() || m_xParser->getOrder().len();
                break;

            case ID_BROWSER_REFRESH:
                aReturn.bEnabled = isValidCursor();
                break;

            case ID_BROWSER_REDO:
                aReturn.bEnabled = sal_False;   // simply forget it ;). no redo possible.
                break;

            case ID_BROWSER_UNDO:
            case ID_BROWSER_SAVEDOC:
            {
                if (!m_bCurrentlyModified)
                {
                    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                    if (xFormSet.is())
                        aReturn.bEnabled = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ISMODIFIED));
                }
                else
                    aReturn.bEnabled = sal_True;

                aReturn.aState <<= ::rtl::OUString((ID_BROWSER_UNDO == nId) ? m_sStateUndoRecord : m_sStateSaveRecord);
            }
            break;
            case ID_BROWSER_EDITDOC:
            {
                // check if it is available
                Reference< XPropertySet >  xDataSourceSet(getRowSet(), UNO_QUERY);
                if (!xDataSourceSet.is())
                    break;  // no datasource -> no edit mode

                sal_Int32 nDataSourcePrivileges = ::comphelper::getINT32(xDataSourceSet->getPropertyValue(PROPERTY_PRIVILEGES));
                sal_Bool bInsertAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::INSERT) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString::createFromAscii("AllowInserts")));
                sal_Bool bUpdateAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::UPDATE) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString::createFromAscii("AllowUpdates")));
                sal_Bool bDeleteAllowedAndPossible = ((nDataSourcePrivileges & ::com::sun::star::sdbcx::Privilege::DELETE) != 0) && ::comphelper::getBOOL(xDataSourceSet->getPropertyValue(::rtl::OUString::createFromAscii("AllowDeletes")));
                if (!bInsertAllowedAndPossible && !bUpdateAllowedAndPossible && !bDeleteAllowedAndPossible)
                    break;  // no insert/update/delete -> no edit mode

                if (!isValidCursor())
                    break;  // no cursor -> no edit mode

                aReturn.bEnabled = sal_True;

                sal_Int16 nGridMode = m_pContent->getVclControl()->GetOptions();
                aReturn.aState = ::comphelper::makeBoolAny(nGridMode > DbGridControl::OPT_READONLY);
            }
            break;
            case ID_BROWSER_FILTERED:
            {
                aReturn.bEnabled = sal_False;
                Reference< XPropertySet >  xActiveSet(getRowSet(), UNO_QUERY);
                ::rtl::OUString aFilter = ::comphelper::getString(xActiveSet->getPropertyValue(PROPERTY_FILTER));
                if (aFilter.len())
                {
                    aReturn.aState = xActiveSet->getPropertyValue(PROPERTY_APPLYFILTER);
                    aReturn.bEnabled = sal_True;
                }
                else
                {
                    aReturn.aState = ::comphelper::makeBoolAny(sal_False);
                    aReturn.bEnabled = sal_False;
                }
            }
            break;
        }
    }
    catch(Exception& e)
    {
#if DBG_UTIL
        String sMessage("SbaXDataBrowserController::GetState(", RTL_TEXTENCODING_ASCII_US);
        sMessage += String::CreateFromInt32(nId);
        sMessage.AppendAscii(") : catched an exception ! message : ");
        sMessage += (const sal_Unicode*)e.Message;
        DBG_ERROR(ByteString(sMessage, gsl_getSystemTextEncoding()).GetBuffer());
#else
        e;  // make compiler happy
#endif
    }

    return aReturn;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::ExecuteFilterSortCrit(sal_Bool bFilter)
{
    if (!SaveModified())
        return;

    // TODO there is no filter dialog yet
    Reference< ::com::sun::star::data::XDatabaseDialogs >  xDlgs(m_xParser, UNO_QUERY);
    if (!xDlgs.is())
        return;

    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);

    // no condition for searching
    if (getRowSet()->isBeforeFirst() || getRowSet()->isAfterLast() ||
        getRowSet()->rowDeleted())
        return;

    Reference< ::com::sun::star::container::XNamed >  xField;
    Reference< XPropertySet >  xColSet = getBoundField();
    xField = Reference< ::com::sun::star::container::XNamed > (xColSet, UNO_QUERY);

    // auslesen der Searchflags
    if (xColSet.is() && !::comphelper::getBOOL(xColSet->getPropertyValue(PROPERTY_ISSEARCHABLE)))
        xField = NULL;

    ::rtl::OUString sOldVal = bFilter ? m_xParser->getFilter() : m_xParser->getOrder();
    try
    {
        if (bFilter)
            xDlgs->executeFilter(xField);
        else
            xDlgs->executeSort(xField);
    }
    catch(...)
    {
        return;
    }
    ;

    ::rtl::OUString sNewVal = bFilter ? m_xParser->getFilter() : m_xParser->getOrder();
    sal_Bool bOldFilterApplied(sal_False);
    if (bFilter)
    {
        try { bOldFilterApplied = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_APPLYFILTER)); } catch(...) { } ;
    }

    if (sOldVal.equals(sNewVal))
        // nothing to be done
        return;

    {
        WaitObject aWO(getContent());
        try
        {
            if (bFilter)
            {
                xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(m_xParser->getFilter()));
                xFormSet->setPropertyValue(PROPERTY_APPLYFILTER, ::comphelper::makeBoolAny(sal_Bool(sal_True)));
            }
            else
                xFormSet->setPropertyValue(PROPERTY_ORDER,makeAny( m_xParser->getOrder()));

            Reference< ::com::sun::star::form::XLoadable >  xReload(xFormSet, UNO_QUERY);
            FormErrorHelper aReportError(this);
            xReload->reload();
        }
        catch(...)
        {
        }
        ;
    }

    if (errorOccured())
    {
        // synchronize the parser with the form
        Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
        if (bFilter)
        {
            xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(sOldVal));
            xFormSet->setPropertyValue(PROPERTY_APPLYFILTER, ::comphelper::makeBoolAny(bOldFilterApplied));
            DO_SAFE( m_xParser->setFilter(sOldVal), "SbaXDataBrowserController::ExecuteFilterSortCrit : could not restore the old filter of my parser !" );
        }
        else
        {
            xFormSet->setPropertyValue(PROPERTY_ORDER, makeAny(sOldVal));
            DO_SAFE( m_xParser->setOrder(sOldVal), "SbaXDataBrowserController::ExecuteFilterSortCrit : could not restore the old order of my parser !" );
        }

        try
        {
            Reference< ::com::sun::star::form::XLoadable >  xReload(xFormSet, UNO_QUERY);
            xReload->reload();
        }
        catch(...)
        {
        }
        ;
    }
    InvalidateFeature(ID_BROWSER_REMOVEFILTER);
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::ExecuteSearch()
{
    // calculate the control source of the active field
    Reference< ::com::sun::star::form::XGrid >  xGrid(m_pContent->getGridControl(), UNO_QUERY);
    DBG_ASSERT(xGrid.is(), "SbaXDataBrowserController::ExecuteSearch : the control should have an ::com::sun::star::form::XGrid interface !");

    Reference< ::com::sun::star::form::XGridPeer >  xGridPeer(m_pContent->getGridControl()->getPeer(), UNO_QUERY);
    Reference< ::com::sun::star::container::XIndexContainer >  xColumns = xGridPeer->getColumns();
    DBG_ASSERT(xGridPeer.is() && xColumns.is(), "SbaXDataBrowserController::ExecuteSearch : invalid peer !");

    sal_Int16 nViewCol = xGrid->getCurrentColumnPosition();
    sal_Int16 nModelCol = m_pContent->View2ModelPos(nViewCol);

    Reference< XPropertySet >  xCurrentCol = *(Reference< XPropertySet > *)xColumns->getByIndex(nModelCol).getValue();
    String sActiveField = ::comphelper::getString(xCurrentCol->getPropertyValue(PROPERTY_CONTROLSOURCE));

    // the text within the current cell
    String sInitialText;
    Reference< ::com::sun::star::container::XIndexAccess >  xColControls(xGridPeer, UNO_QUERY);
    Reference< XInterface >  xCurControl(*(Reference< XInterface > *)xColControls->getByIndex(nViewCol).getValue(), UNO_QUERY);
    ::rtl::OUString aInitialText;
    if (IsSearchableControl(xCurControl, &aInitialText))
        sInitialText = (const sal_Unicode*)aInitialText;

    // prohibit the synchronization of the grid's display with the cursor's position
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    DBG_ASSERT(xModelSet.is(), "SbaXDataBrowserController::ExecuteSearch : no model set ?!");
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), ::comphelper::makeBoolAny(sal_Bool(sal_False)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("AlwaysShowCursor"), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("CursorColor"), makeAny(sal_Int32(COL_LIGHTRED)));

    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xNFS(::dbtools::getNumberFormats(::dbtools::getConnection(m_xRowSet), sal_True,m_xMultiServiceFacatory));

    FmSearchDialog dlg(getContent(), sInitialText, String::CreateFromAscii("Standard"), 0, LINK(this, SbaXDataBrowserController, OnSearchContextRequest),
        FmSearchDialog::SM_ALLOWSCHEDULE);
    dlg.SetActiveField(sActiveField);

    dlg.SetFoundHandler(LINK(this, SbaXDataBrowserController, OnFoundData));
    dlg.SetCanceledNotFoundHdl(LINK(this, SbaXDataBrowserController, OnCanceledNotFound));
    dlg.Execute();

    // restore the grid's normal operating state
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("AlwaysShowCursor"), ::comphelper::makeBoolAny(sal_Bool(sal_False)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("CursorColor"), Any());
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::Execute(sal_uInt16 nId)
{
    sal_Bool bSortUp = sal_True;

    switch (nId)
    {
        case ID_BROWSER_FILTERED:
            if (SaveModified())
            {
                Reference< XPropertySet >  xActiveSet(getRowSet(), UNO_QUERY);
                sal_Bool bApplied = ::comphelper::getBOOL(xActiveSet->getPropertyValue(PROPERTY_APPLYFILTER));
                xActiveSet->setPropertyValue(PROPERTY_APPLYFILTER, ::comphelper::makeBoolAny(sal_Bool(!bApplied)));
                Reference< ::com::sun::star::form::XLoadable >  xReload(xActiveSet, UNO_QUERY);
                {
                    WaitObject aWO(getContent());
                    xReload->reload();
                }
            }
            InvalidateFeature(ID_BROWSER_FILTERED);
            break;
        case ID_BROWSER_EDITDOC:
        {
            sal_Int16 nGridMode = m_pContent->getVclControl()->GetOptions();
            if (nGridMode == DbGridControl::OPT_READONLY)
                m_pContent->getVclControl()->SetOptions(DbGridControl::OPT_UPDATE | DbGridControl::OPT_INSERT | DbGridControl::OPT_DELETE);
                    // the options not supported by the data source will be removed automatically
            else
            {
                if (!SaveData(sal_True, sal_False))
                    // give the user a chance to save the current record (if neccessary)
                    break;

                // maybe the user wanted to reject the modified record ?
                if (GetState(ID_BROWSER_UNDO).bEnabled)
                    Execute(ID_BROWSER_UNDO);

                m_pContent->getVclControl()->SetOptions(DbGridControl::OPT_READONLY);
            }
            InvalidateFeature(ID_BROWSER_EDITDOC);
        }
        break;

        case ID_BROWSER_SEARCH:
            if (SaveData(sal_True, sal_False))
                ExecuteSearch();
            break;

        case ID_BROWSER_COPY:
        case ID_BROWSER_CUT:
        case ID_BROWSER_PASTE:
        {
            DbCellControllerRef xCurrentController = m_pContent->getVclControl()->Controller();
            if (!xCurrentController.Is())
                // should be intercepted by GetState. Normally.
                // Unfortunately ID_BROWSER_PASTE is a 'fast call' slot, which means it may be executed without checking if it is
                // enabled. This would be really deadly herein if the current cell has no controller ...
                // (FS - 13.04.99 - #64694#)
                return;

            Edit& rEdit = (Edit&)xCurrentController->GetWindow();
            switch (nId)
            {
                case ID_BROWSER_CUT : rEdit.Cut(); break;
                case SID_COPY   : rEdit.Copy(); break;
                case ID_BROWSER_PASTE   : rEdit.Paste(); break;
            }
            if (ID_BROWSER_CUT == nId || ID_BROWSER_PASTE == nId)
            {
                xCurrentController->SetModified();
                rEdit.Modify();
            }
        }
        break;

        case ID_BROWSER_SORTDOWN:
            bSortUp = sal_False;
            // DON'T break
        case ID_BROWSER_SORTUP:
        {
            if (!SaveModified())
                break;

            if (!isValidCursor())
                break;

            // only one sort order
            Reference< XPropertySet >  xField(getBoundField(), UNO_QUERY);
            if (!xField.is())
                break;

            ::rtl::OUString sOldSort = m_xParser->getOrder();
            sal_Bool bParserSuccess;
            HANDLE_SQL_ERRORS(
                m_xParser->setOrder(::rtl::OUString()); m_xParser->appendOrderByColumn(xField, bSortUp),
                bParserSuccess,
                UniString(ModuleRes(SBA_BROWSER_SETTING_ORDER)),
                "SbaXDataBrowserController::Execute : catched an exception while composing the new filter !"
            )

            if (bParserSuccess)
            {
                WaitObject aWO(getContent());
                try
                {
                    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                    xFormSet->setPropertyValue(PROPERTY_ORDER, makeAny(m_xParser->getOrder()));

                    Reference< ::com::sun::star::form::XLoadable >  xReload(xFormSet, UNO_QUERY);
                    FormErrorHelper aReportError(this);
                    xReload->reload();
                }
                catch(...)
                {
                }
                ;
            }
            if (errorOccured())
            {
                // synchronize the parser with the form
                Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                xFormSet->setPropertyValue(PROPERTY_ORDER, makeAny(sOldSort));
                m_xParser->setOrder(sOldSort);

                try
                {
                    Reference< ::com::sun::star::form::XLoadable >  xReload(xFormSet, UNO_QUERY);
                    xReload->reload();
                }
                catch(...)
                {
                }
                ;
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
        }
        break;

        case ID_BROWSER_AUTOFILTER:
        {
            if (!SaveModified())
                break;

            if (!isValidCursor())
                break;

            Reference< XPropertySet >  xField(getBoundField(), UNO_QUERY);
            if (!xField.is())
                break;

            ::rtl::OUString sOldFilter = m_xParser->getFilter();

            Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
            sal_Bool bApplied = ::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_APPLYFILTER));
            // do we have a filter but it's not applied ?
            // -> completely overwrite it, else append one
            if (!bApplied)
            {
                DO_SAFE( m_xParser->setFilter(::rtl::OUString()), "SbaXDataBrowserController::Execute : catched an exception while resetting the new filter !" );
            }

            sal_Bool bParserSuccess;
            HANDLE_SQL_ERRORS(
                m_xParser->appendFilterByColumn(xField),
                bParserSuccess,
                UniString(ModuleRes(SBA_BROWSER_SETTING_FILTER)),
                "SbaXDataBrowserController::Execute : catched an exception while composing the new filter !"
            )

            if (bParserSuccess)
            {
                WaitObject aWO(getContent());
                try
                {
                    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                    xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(m_xParser->getFilter()));
                    xFormSet->setPropertyValue(PROPERTY_APPLYFILTER, ::comphelper::makeBoolAny(sal_Bool(sal_True)));

                    Reference< ::com::sun::star::form::XLoadable >  xReload(xFormSet, UNO_QUERY);
                    FormErrorHelper aReportError(this);
                    xReload->reload();
                }
                catch(...)
                {
                }
                ;
            }
            if (errorOccured())
            {
                // synchronize the parser with the form
                Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
                xFormSet->setPropertyValue(PROPERTY_FILTER, makeAny(sOldFilter));
                xFormSet->setPropertyValue(PROPERTY_APPLYFILTER, makeAny(bApplied));
                m_xParser->setFilter(sOldFilter);

                try
                {
                    Reference< ::com::sun::star::form::XLoadable >  xReload(xFormSet, UNO_QUERY);
                    xReload->reload();
                }
                catch(...)
                {
                }
                ;
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
            InvalidateFeature(ID_BROWSER_FILTERED);
        }
        break;

        case ID_BROWSER_ORDERCRIT:
            ExecuteFilterSortCrit(sal_False);
            break;

        case ID_BROWSER_FILTERCRIT:
            ExecuteFilterSortCrit(sal_True);
            InvalidateFeature(ID_BROWSER_FILTERED);
            break;

        case ID_BROWSER_REMOVEFILTER:
        {
            if (!SaveModified())
                break;

            // reset the filter and the sort property simutaneously so only _one_ new statement has to be
            // sent
            Reference< XPropertySet >  xSet(getRowSet(), UNO_QUERY);
            xSet->setPropertyValue(PROPERTY_FILTER,makeAny(::rtl::OUString()));
            xSet->setPropertyValue(PROPERTY_ORDER,makeAny(::rtl::OUString()));
            {
                WaitObject aWO(getContent());
                try
                {
                    Reference< ::com::sun::star::form::XLoadable >  xReload(getRowSet(), UNO_QUERY);
                    xReload->reload();
                }
                catch(...)
                {
                }
                ;
            }
            InvalidateFeature(ID_BROWSER_REMOVEFILTER);
            InvalidateFeature(ID_BROWSER_FILTERED);
        }
        break;

        case ID_BROWSER_REFRESH:
            if (SaveData(sal_True, sal_False))
            {
                Reference< ::com::sun::star::form::XLoadable >  xReload(getRowSet(), UNO_QUERY);
                WaitObject aWO(getContent());
                xReload->reload();
            }
            break;

        case ID_BROWSER_SAVEDOC:
        {
            SaveModified(sal_True);
        }
        break;

        case ID_BROWSER_UNDO:
        {
            // restore the cursor state
            Reference< XResultSetUpdate >  xCursor(getRowSet(), UNO_QUERY);
            Reference< XPropertySet >  xSet(xCursor, UNO_QUERY);
            if (::comphelper::getBOOL(xSet->getPropertyValue(PROPERTY_ISNEW)))
            {
                xCursor->moveToInsertRow();
                // no need to reset the grid model after we moved to the insert row, this is done implicitly by the
                // form
                // (and in some cases it may be deadly to do the reset explicitly after the form did it implicitly,
                // cause the form's reset may be async, and this leads to some nice deadlock scenarios ....)
            }
            else
            {
                xCursor->cancelRowUpdates();

                // restore the grids state
                Reference< ::com::sun::star::form::XReset >  xReset(getControlModel(), UNO_QUERY);
                if (xReset.is())
                    xReset->reset();
            }

            m_bCurrentlyModified = sal_False;
            InvalidateFeature(::rtl::OUString::createFromAscii(".uno:Save"));
            InvalidateFeature(ID_BROWSER_UNDO);
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::SaveModified(sal_Bool bCommit)
{
    if (bCommit && !CommitCurrent())    // das aktuelle Control committen lassen
        return sal_False;

    Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
    sal_Bool bResult = sal_False;
    try
    {
        if (::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ISMODIFIED)))
        {
            Reference< XResultSetUpdate >  xCursor(getRowSet(), UNO_QUERY);
            if (::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_ISNEW)))
                xCursor->insertRow();
            else
                xCursor->updateRow();
        }
        bResult = sal_True;
    }
    catch(SQLException&)
    {
    }
    catch(...)
    {
        DBG_ERROR("SbaXDataBrowserController::SaveModified : could not save the current record !");
        bResult = sal_False;
    }



    InvalidateFeature(::rtl::OUString::createFromAscii(".uno:Save"));
    InvalidateFeature(ID_BROWSER_UNDO);

    return bResult;
}

//------------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::CommitCurrent()
{
    if (!m_pContent)
        return sal_True;

    Reference< ::com::sun::star::awt::XControl >  xActiveControl(m_pContent->getGridControl());
    Reference< ::com::sun::star::form::XBoundControl >  xLockingTest(xActiveControl, UNO_QUERY);
    sal_Bool bControlIsLocked = xLockingTest.is() && xLockingTest->getLock();
    sal_Bool bResult = sal_True;
    if (xActiveControl.is() && !bControlIsLocked)
    {
        // zunaechst das Control fragen ob es das IFace unterstuetzt
        Reference< ::com::sun::star::form::XBoundComponent >  xBoundControl(xActiveControl, UNO_QUERY);
        if (!xBoundControl.is())
            xBoundControl  = Reference< ::com::sun::star::form::XBoundComponent > (xActiveControl->getModel(), UNO_QUERY);
        if (xBoundControl.is() && !xBoundControl->commit())
            return sal_False;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnToolBoxSelected, ToolBox*, pToolBox)
{
    Execute(pToolBox->GetCurItemId());
    return 0L;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::ColumnChanged()
{
    InvalidateFeature(ID_BROWSER_SORTUP);
    InvalidateFeature(ID_BROWSER_SORTDOWN);
    InvalidateFeature(ID_BROWSER_ORDERCRIT);
    InvalidateFeature(ID_BROWSER_FILTERCRIT);
    InvalidateFeature(ID_BROWSER_AUTOFILTER);
    InvalidateFeature(ID_BROWSER_REMOVEFILTER);
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::SelectionChanged()
{
    InvalidateFeature(ID_BROWSER_INSERTCOLUMNS);
    InvalidateFeature(ID_BROWSER_INSERTCONTENT);
    InvalidateFeature(ID_BROWSER_FORMLETTER);
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::CellActivated()
{
    m_aInvalidateClipboard.SetTimeout(300);
    m_aInvalidateClipboard.Start();
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::CellDeactivated()
{
    m_aInvalidateClipboard.Stop();
    LINK(this, SbaXDataBrowserController, OnInvalidateClipboard).Call(NULL);
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnInvalidateClipboard, void*, EMPTYARG)
{
    InvalidateFeature(ID_BROWSER_CUT);
    InvalidateFeature(ID_BROWSER_COPY);
    InvalidateFeature(ID_BROWSER_PASTE);
    return 0L;
}

//------------------------------------------------------------------
sal_uInt16 SbaXDataBrowserController::SaveData(sal_Bool bUI, sal_Bool bForBrowsing)
{
    if (!m_pContent)
        return sal_True;

    if (!isValidCursor())
        return sal_True;

    if (bUI && GetState(ID_BROWSER_SAVEDOC).bEnabled)
    {
        m_pContent->getVclControl()->GrabFocus();

        QueryBox aQry(m_pContent->getVclControl(), ModuleRes(QUERY_BRW_SAVEMODIFIED));
        if (bForBrowsing)
            aQry.AddButton(ResId(RID_STR_NEW_TASK), RET_NEWTASK,
                BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON);

        switch (aQry.Execute())
        {
            case RET_NO:
                Execute(ID_BROWSER_UNDO);
                return sal_True;
            case RET_CANCEL:
                return sal_False;
            case RET_NEWTASK:
                return RET_NEWTASK;
        }
    }

    return (sal_uInt16)SaveModified();
}

// -------------------------------------------------------------------------
Reference< XPropertySet >  SbaXDataBrowserController::getBoundField(sal_uInt16 nViewPos) const
{
    Reference< XPropertySet >  xEmptyReturn;

    // get the current column from the grid
    if (nViewPos == (sal_uInt16)-1)
    {
        Reference< ::com::sun::star::form::XGrid >  xGrid(m_pContent->getGridControl(), UNO_QUERY);
        if (!xGrid.is())
            return xEmptyReturn;
        nViewPos = xGrid->getCurrentColumnPosition();
    }
    sal_uInt16 nCurrentCol = m_pContent->View2ModelPos(nViewPos);
    if (nCurrentCol == (sal_uInt16)-1)
        return xEmptyReturn;

    // get the according column from the model
    Reference< ::com::sun::star::container::XIndexContainer >  xCols(getControlModel(), UNO_QUERY);
    Reference< XPropertySet >  xCurrentCol(*(Reference< XInterface > *)xCols->getByIndex(nCurrentCol).getValue(), UNO_QUERY);
    if (!xCurrentCol.is())
        return xEmptyReturn;

    Any aBoundField = xCurrentCol->getPropertyValue(PROPERTY_BOUNDFIELD);
    if (!aBoundField.hasValue())
        return xEmptyReturn;

    return Reference< XPropertySet > (*(Reference< XInterface > *)aBoundField.getValue(), UNO_QUERY);
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnSearchContextRequest, FmSearchContext*, pContext)
{
    Reference< ::com::sun::star::container::XIndexAccess >  xPeerContainer(getContent()->getGridControl(), UNO_QUERY);

    // check all grid columns for their control source
    Reference< ::com::sun::star::container::XIndexAccess >  xModelColumns(getFormComponent(), UNO_QUERY);
    DBG_ASSERT(xModelColumns.is(), "SbaXDataBrowserController::OnSearchContextRequest : there is a grid control without columns !");
        // the case 'no columns' should be indicated with an empty container, I think ...
    DBG_ASSERT(xModelColumns->getCount() >= xPeerContainer->getCount(), "SbaXDataBrowserController::OnSearchContextRequest : impossible : have more view than model columns !");

    String sFieldList;
    for (sal_Int32 nViewPos=0; nViewPos<xPeerContainer->getCount(); ++nViewPos)
    {
        Reference< XInterface >  xCurrentColumn(*(Reference< XInterface > *)xPeerContainer->getByIndex(nViewPos).getValue(), UNO_QUERY);
        if (!xCurrentColumn.is())
            continue;

        // can we use this column control fo searching ?
        if (!IsSearchableControl(xCurrentColumn))
            continue;

        sal_uInt16 nModelPos = m_pContent->View2ModelPos(nViewPos);
        Reference< XPropertySet >  xCurrentColModel = *(Reference< XPropertySet > *)xModelColumns->getByIndex(nModelPos).getValue();
        ::rtl::OUString aName = ::comphelper::getString(xCurrentColModel->getPropertyValue(PROPERTY_CONTROLSOURCE));

        sFieldList += (const sal_Unicode*)aName;
        sFieldList += ';';

        pContext->arrFields.push_back(xCurrentColumn);
    }
    sFieldList.EraseTrailingChars(';');

    pContext->xCursor = Reference< XResultSet>(getRowSet(),UNO_QUERY);
    pContext->strUsedFields = sFieldList;

    // if the cursor is in a mode other than STANDARD -> reset
    Reference< XPropertySet >  xCursorSet(pContext->xCursor, UNO_QUERY);
    DBG_ASSERT(xCursorSet.is() && !::comphelper::getBOOL(xCursorSet->getPropertyValue(PROPERTY_ISMODIFIED)),
        "SbaXDataBrowserController::OnSearchContextRequest : please do not call for cursors with modified rows !");
    if (xCursorSet.is() && ::comphelper::getBOOL(xCursorSet->getPropertyValue(PROPERTY_ISNEW)))
    {
        Reference< XResultSetUpdate >  xUpdateCursor(pContext->xCursor, UNO_QUERY);
        xUpdateCursor->moveToCurrentRow();
    }
    return pContext->arrFields.size();
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnFoundData, FmFoundRecordInformation*, pInfo)
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xCursor(getRowSet(), UNO_QUERY);
    DBG_ASSERT(xCursor.is(), "SbaXDataBrowserController::OnFoundData : shit happens. sometimes. but this is simply impossible !");

    // move the cursor
    xCursor->moveToBookmark(pInfo->aPosition);

    // let the grid snyc it's display with the cursor
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    DBG_ASSERT(xModelSet.is(), "SbaXDataBrowserController::OnFoundData : no model set ?!");
    Any aOld = xModelSet->getPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), aOld);

    // and move to the field
    Reference< ::com::sun::star::container::XIndexAccess >  aColumnControls(m_pContent->getGridControl()->getPeer(), UNO_QUERY);
    for (sal_uInt16 nViewPos=0; nViewPos<aColumnControls->getCount(); ++nViewPos)
    {
        Reference< XInterface >  xCurrent(*(Reference< XInterface > *)aColumnControls->getByIndex(nViewPos).getValue(), UNO_QUERY);
        if (IsSearchableControl(xCurrent))
            if (pInfo->nFieldPos)
                --pInfo->nFieldPos;
            else
                break;
    }

    Reference< ::com::sun::star::form::XGrid >  xGrid(m_pContent->getGridControl(), UNO_QUERY);
    xGrid->setCurrentColumnPosition(nViewPos);

    return 0;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnCanceledNotFound, FmFoundRecordInformation*, pInfo)
{
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xCursor(getRowSet(), UNO_QUERY);

    DBG_ASSERT(xCursor.is(), "SbaXDataBrowserController::OnCanceledNotFound : shit happens. sometimes. but this is simply impossible !");   // move the cursor
    xCursor->moveToBookmark(pInfo->aPosition);

    // let the grid snyc it's display with the cursor
    Reference< XPropertySet >  xModelSet(getControlModel(), UNO_QUERY);
    DBG_ASSERT(xModelSet.is(), "SbaXDataBrowserController::OnCanceledNotFound : no model set ?!");
    Any aOld = xModelSet->getPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), ::comphelper::makeBoolAny(sal_Bool(sal_True)));
    xModelSet->setPropertyValue(::rtl::OUString::createFromAscii("DisplayIsSynchron"), aOld);

    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnOpenFinishedMainThread, void*, EMPTYARG)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    if (!m_nPendingLoadFinished)
        // it's possible that the direct call of this link from within suspend caused this method to be executed
        // in another thread while we were waiting for the mutex in this thread
        return 0;
    m_nPendingLoadFinished = 0;

    m_bLoadCanceled |= ((LoadFormThread*)m_pLoadThread)->WasCanceled();
    delete m_pLoadThread;
    m_pLoadThread = NULL;

    FormLoaded(sal_False);

    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnOpenFinished, void*, EMPTYARG)
{
    ::osl::MutexGuard aCheckGuard(m_aAsyncLoadSafety);

    if (m_bClosingKillOpen)
    {
        delete m_pLoadThread;
        m_pLoadThread = NULL;
    }
    else
        // all cleaning has to run in the main thread, not here (this is called synchronously from the LoadThread)
        // so we use an user event
        m_nPendingLoadFinished = Application::PostUserEvent(LINK(this, SbaXDataBrowserController, OnOpenFinishedMainThread));

    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnAsyncClose, void*, EMPTYARG)
{
    EmptyWindow();
    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnAsyncGetCellFocus, void*, EMPTYARG)
{
    SbaGridControl* pVclGrid = m_pContent ? m_pContent->getVclControl() : NULL;
    // if we have a controller, but the window for the controller doesn't have the focus, we correct this
    if (!pVclGrid->IsEditing())
        return 0L;

    if (pVclGrid->HasChildPathFocus())
        pVclGrid->Controller()->GetWindow().GrabFocus();

    return 0L;
}

//------------------------------------------------------------------------------
IMPL_LINK(SbaXDataBrowserController, OnAsyncInvalidateAll, void*, EMPTYARG)
{
    InvalidateAll(sal_False);
    return 0L;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::FormLoaded(sal_Bool /*bWasSynch*/)
{
    if (isValid() && !m_bLoadCanceled)
    {
        // --------------------------------
        // switch the control to alive mode
        m_pContent->getGridControl()->setDesignMode(sal_False);

        // ----------------------------------------------
        // create a parser (needed for filtering/sorting)
        try
        {
            Reference< XPropertySet >  xFormSet(getRowSet(), UNO_QUERY);
            if (::comphelper::getBOOL(xFormSet->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING)))
            {   // (only if the statement isn't native)
                // (it is allowed to use the PROPERTY_ISPASSTHROUGH : _after_ loading a form it is valid)
                Reference< ::com::sun::star::sdb::XSQLQueryComposerFactory >  xFactory(::dbtools::getConnection(getRowSet()), UNO_QUERY);
                if (xFactory.is())
                    m_xParser = xFactory->createQueryComposer();
            }

            // initialize the parser with the current sql-statement of the form
            if (m_xParser.is())
            {
                m_xParser->setQuery(::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_ACTIVECOMMAND)));
                m_xParser->setFilter(::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_FILTER)));
                m_xParser->setOrder(::comphelper::getString(xFormSet->getPropertyValue(PROPERTY_ORDER)));
            }
        }
        catch(...)
        {
            DBG_WARNING("SbaXDataBrowserController::Construct : something went wrong while creating the parser !");
            m_xParser = NULL;
            // no further handling, we ignore the error
        }
        ;

        InvalidateAll();
        // -------------------------------
        // start the clipboard invalidator
        m_aInvalidateClipboard.SetTimeoutHdl(LINK(this, SbaXDataBrowserController, OnInvalidateClipboard));

        m_aAsyncGetCellFocus.Call();
    }
    else
    {
        m_aAsynClose.Call();
    }
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::enterFormAction()
{
    if (!m_nFormActionNestingLevel)
        // first action -> reset flag
        m_bErrorOccured = false;

    ++m_nFormActionNestingLevel;
}

//------------------------------------------------------------------------------
void SbaXDataBrowserController::leaveFormAction()
{
    DBG_ASSERT(m_nFormActionNestingLevel > 0, "SbaXDataBrowserController::leaveFormAction : invalid call !");
    --m_nFormActionNestingLevel;
}

// -------------------------------------------------------------------------
URL SbaXDataBrowserController::getURLForId(sal_Int32 _nId) const
{
    URL aReturn;
    for (   ConstSupportedFeaturesIterator aLoop = m_aSupportedFeatures.begin();
            aLoop != m_aSupportedFeatures.end();
            ++aLoop
        )
        if (aLoop->second == _nId)
        {
            aReturn.Complete = aLoop->first;
            break;
        }

    // TODO: maybe let an URLTransformer analyze the URL
    return aReturn;
}

// -------------------------------------------------------------------------
sal_Bool SbaXDataBrowserController::isValidCursor() const
{
    Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(m_xRowSet, UNO_QUERY);
    if (!xSupplyCols.is())
        return sal_False;
    Reference< ::com::sun::star::container::XIndexAccess >  xCols(xSupplyCols->getColumns(), UNO_QUERY);
    if (!xCols.is() || (xCols->getCount() == 0))
        return sal_False;
    return !(m_xRowSet->isBeforeFirst() || m_xRowSet->isAfterLast()) ;
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* SbaXDataBrowserController::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new cppu::OPropertyArrayHelper(aProps);
}

// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & SbaXDataBrowserController::getInfoHelper()
{
    return *const_cast<SbaXDataBrowserController*>(this)->getArrayHelper();
}
//==================================================================
// LoadFormHelper
//==================================================================

class LoadFormHelper :public ::cppu::WeakImplHelper2< ::com::sun::star::form::XLoadListener,
                                                      XRowSetListener>
{
    enum STATE { STARTED, LOADED, POSITIONED, DISPOSED };
    STATE   m_eState;

    Reference< XRowSet >                m_xForm;

    ::osl::Mutex    m_aAccessSafety;

public:
    LoadFormHelper(const Reference< XRowSet > & _rxForm);

    // ::com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException );

    // XRowSetListener
    virtual void SAL_CALL cursorMoved(const ::com::sun::star::lang::EventObject& event) throw( RuntimeException );
    virtual void SAL_CALL rowChanged(const ::com::sun::star::lang::EventObject& event) throw( RuntimeException ){};
    virtual void SAL_CALL rowSetChanged(const ::com::sun::star::lang::EventObject& event) throw( RuntimeException ){};

    // ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException );


    bool WaitUntilReallyLoaded(bool _bOnlyIfLoaded);
        // waits 'til the first positioned event after the loaded event. returns true if successfull,
        // false if the form was disposed or unloaded before or while waiting
        // if _bOnlyIfLoaded is false and the form isn't loaded already loaded, false will be returned
        // (without any wating)

    void cancel();

protected:
    ~LoadFormHelper();

    void implDispose();
};

//------------------------------------------------------------------------------
LoadFormHelper::LoadFormHelper(const Reference< XRowSet > & _rxForm)
    :m_xForm(_rxForm)
    ,m_eState(STARTED)
{
    Reference< ::com::sun::star::form::XLoadable > (m_xForm, UNO_QUERY)->addLoadListener(this);
    m_xForm->addRowSetListener(this);
}

//------------------------------------------------------------------------------
LoadFormHelper::~LoadFormHelper()
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    implDispose();
}

//------------------------------------------------------------------------------
void LoadFormHelper::implDispose()
{
    if (DISPOSED != m_eState)
    {

        Reference< ::com::sun::star::form::XLoadable > (m_xForm, UNO_QUERY)->removeLoadListener(this);
        m_xForm->removeRowSetListener(this);
        m_xForm = NULL;
        m_eState = DISPOSED;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::loaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    DBG_ASSERT(m_eState == STARTED || m_eState == DISPOSED, "LoadFormHelper::loaded : wrong call !");
    if (m_eState == STARTED)
        m_eState = LOADED;
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::unloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    DBG_ERROR("LoadFormHelper::unloaded : shouldn't be called !");
    implDispose();
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::reloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::cursorMoved(const ::com::sun::star::lang::EventObject& event) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    if (m_eState == LOADED)
        m_eState = POSITIONED;
}

//------------------------------------------------------------------------------
void SAL_CALL LoadFormHelper::disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard(m_aAccessSafety);
    implDispose();
}

//------------------------------------------------------------------------------
void LoadFormHelper::cancel()
{
    implDispose();
}

//------------------------------------------------------------------------------
bool LoadFormHelper::WaitUntilReallyLoaded(bool _bOnlyIfLoaded)
{
    ::osl::ClearableMutexGuard aGuard(m_aAccessSafety);
    if (DISPOSED == m_eState)
        return false;

    if (_bOnlyIfLoaded && (STARTED == m_eState))
        // we did't get a "loaded" event ....
        return false;

    sal_Bool bDone = (POSITIONED == m_eState);
    aGuard.clear();

    while (!bDone)
    {
        ::osl::MutexGuard aGuard(m_aAccessSafety);
        bDone = (POSITIONED == m_eState);
    }

    ::osl::MutexGuard aGuard2(m_aAccessSafety);
    implDispose();

    return true;
}

//==================================================================
// LoadFormThread - a thread for asynchronously loading a form
//==================================================================
//------------------------------------------------------------------------------
void LoadFormThread::run()
{
    // On instantiation of a SfxCancellable the application is notified and 'switches on' the red stop button.
    // Unfortunally this is conditioned with the acquirement of the solar mutex, and the application tries
    // only once and ignores the notification if it fails.
    // To prevent that we get the solar mutex and _block_ 'til we got it.
    // As we are in the 'top level execution' of this thread (with a rather small stack and no other mutexes locked)
    // we shouldn't experience problems with deadlocks ...
    ::vos::OClearableGuard aSolarGuard(Application::GetSolarMutex());
    ThreadStopper* pStopper = new ThreadStopper(this, m_sStopperCaption);
    aSolarGuard.clear();

    // we're not canceled yet
    ::osl::ClearableMutexGuard aResetGuard(m_aAccessSafety);
    m_bCanceled = sal_False;
    aResetGuard.clear();

    LoadFormHelper* pHelper = new LoadFormHelper(m_xRowSet);
    pHelper->acquire();

    // start it
    bool bErrorOccured = false;
    try
    {
        Reference< ::com::sun::star::form::XLoadable >  xLoad(m_xRowSet, UNO_QUERY);
        Reference< XRowSet >  xMove(m_xRowSet, UNO_QUERY);
        DBG_ASSERT(xLoad.is() && xMove.is(), "LoadFormThread::run : invalid cursor !");
        xLoad->load();
        // go to the first record if the load was successfull.
        Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(m_xRowSet, UNO_QUERY);
        Reference< ::com::sun::star::container::XNameAccess >  xCols = xSupplyCols.is() ? xSupplyCols->getColumns() : Reference< ::com::sun::star::container::XNameAccess > ();
        if (xCols.is() && xCols->getElementNames().getLength())
            xMove->first();
        else
            bErrorOccured = true;
    }
    catch(...)
    {
        bErrorOccured = true;
    }
    ;

    // check if we were canceled
    ::osl::ClearableMutexGuard aTestGuard(m_aAccessSafety);
    bool bReallyCanceled = (bool)m_bCanceled;
    aTestGuard.clear();

    bReallyCanceled |= bErrorOccured;

    // the load on the form is "slightly asyncronous" (which isn't covered by it's specification, anyway), so wait
    // some time ....
    // (though me thinks that the load of the new api is synchronous, so we won't need this LoadFormHelper anymore ...)
    if (!bReallyCanceled)
        pHelper->WaitUntilReallyLoaded(true);

    pHelper->cancel();
    pHelper->release();

    // yes, we were, but eventually the cancel request didn't reach the data source in time
    Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_xRowSet, UNO_QUERY);
    if (bReallyCanceled && xLoadable.is() && xLoadable->isLoaded())
        xLoadable->unload();

    pStopper->OwnerTerminated();
        // this will cause the stopper to delete itself (in the main thread) so we don't have to take care of the
        // solar mutex
}

//------------------------------------------------------------------------------
void LoadFormThread::onTerminated()
{
    ::osl::ClearableMutexGuard aGuard(m_aAccessSafety);
    if (m_aTerminationHandler.IsSet())
    {
        // within the call of our termination handler we may be deleted, so do anything which is a member
        // access before the call ...
        // FS - #69801# - 02.12.99
        Link aHandler(m_aTerminationHandler);
        aGuard.clear();
        aHandler.Call(this);
    }
    else
    {
        // we are fully responsible for the data source and for ourself, so dispose the former ...
        Reference< ::com::sun::star::lang::XComponent >  xDataSourceComponent(m_xRowSet, UNO_QUERY);
        if (xDataSourceComponent.is())
            xDataSourceComponent->dispose();
        // ... and delete the latter
        aGuard.clear();     // like above - releasing the mutex is a member access ...
        delete this;
    }
}

//------------------------------------------------------------------------------
void LoadFormThread::StopIt()
{
    ::osl::ClearableMutexGuard aResetGuard(m_aAccessSafety);
    m_bCanceled = sal_True;
    aResetGuard.clear();

    Reference< ::com::sun::star::sdbcx::XColumnsSupplier >  xSupplyCols(m_xRowSet, UNO_QUERY);
    if (!xSupplyCols.is())
    {
        DBG_ERROR("LoadFormThread::StopIt : invalid data source !");
        return;
    }
    Reference< ::com::sun::star::container::XIndexAccess >  xCols(xSupplyCols->getColumns(), UNO_QUERY);
    if (!xCols.is() || (xCols->getCount() == 0))
        // the cursor isn't alive, don't need to cancel
        return;

    Reference< ::com::sun::star::util::XCancellable >  xCancel(m_xRowSet, UNO_QUERY);
    if (xCancel.is())
    {
        try  { xCancel->cancel(); } catch(SQLException&) {}
        // with this the cursor returns from it's load call, this terminates our run, this get's our termination handler to
        // be called
        // (the try-catch is just in case the cancel wasn't neccessary anymore)
    }
}

//------------------------------------------------------------------------------
LoadFormThread::ThreadStopper::ThreadStopper(LoadFormThread* pOwner, const String& rTitle)
    :SfxCancellable(SFX_APP()->GetCancelManager(), rTitle)
    ,m_pOwner(pOwner)
{
}

//------------------------------------------------------------------------------
void LoadFormThread::ThreadStopper::Cancel()
{
    if (!m_pOwner)
        return;

    ::osl::MutexGuard aGuard(m_pOwner->m_aAccessSafety);
    if (IsCancelled())
        // we already did pass this to our owner
        return;

    SfxCancellable::Cancel();
    m_pOwner->StopIt();
}

//------------------------------------------------------------------------------
void LoadFormThread::ThreadStopper::OwnerTerminated()
{
    m_pOwner = NULL;
    Application::PostUserEvent(LINK(this, LoadFormThread::ThreadStopper, OnDeleteInMainThread), this);
}

//------------------------------------------------------------------------------
IMPL_LINK(LoadFormThread::ThreadStopper, OnDeleteInMainThread, LoadFormThread::ThreadStopper*, pThis)
{
    delete pThis;
    return 0L;
}

//..................................................................
}   // namespace dbaui
//..................................................................
