/*************************************************************************
 *
 *  $RCSfile: exsrcbrw.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-23 13:24:16 $
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

#ifndef _SBA_EXTCTRLR_HXX
#include "exsrcbrw.hxx"
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XGRIDCOLUMNFACTORY_HPP_
#include <com/sun/star/form/XGridColumnFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORMCONTROLLER_HPP_
#include <com/sun/star/form/XFormController.hpp>
#endif
#ifndef _SBA_FORMADAPTER_HXX
#include "formadapter.hxx"
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbaui;

//==============================================================================
//= SbaExternalSourceBrowser
//==============================================================================
extern "C" void SAL_CALL createRegistryInfo_OFormGridView()
{
    static OMultiInstanceAutoRegistration< SbaExternalSourceBrowser > aAutoRegistration;
}
//------------------------------------------------------------------------------
Any SAL_CALL SbaExternalSourceBrowser::queryInterface(const Type& _rType) throw (RuntimeException)
{
    Any aRet = SbaXDataBrowserController::queryInterface(_rType);
    if(!aRet.hasValue())
    {
        aRet = ::cppu::queryInterface(_rType,
                                (::com::sun::star::util::XModifyBroadcaster*)this,
                                (::com::sun::star::form::XLoadListener*)this);
        if(!aRet.hasValue())
            aRet = m_xFormControllerImpl->queryAggregation(_rType);
    }

    return aRet;
}
//------------------------------------------------------------------------------
SbaExternalSourceBrowser::SbaExternalSourceBrowser(const Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_pDataSourceImpl(NULL)
    ,m_aModifyListeners(m_aPropertyMutex)
    ,m_bInQueryDispatch(NULL)
{
    // create the aggregated object
    ::comphelper::increment(m_refCount);
    {
        m_pFormControllerImpl = new FormControllerImpl(this);
        m_xFormControllerImpl = m_pFormControllerImpl;
        m_xFormControllerImpl->setDelegator(*this);
    }
    ::comphelper::decrement(m_refCount);
}

//------------------------------------------------------------------------------
SbaExternalSourceBrowser::~SbaExternalSourceBrowser()
{
    // Freigeben der Aggregation
    if (m_xFormControllerImpl.is())
    {
        Reference< XInterface >  xEmpty;
        m_xFormControllerImpl->setDelegator(xEmpty);
    }
}
//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL SbaExternalSourceBrowser::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
::rtl::OUString SbaExternalSourceBrowser::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.OFormGridView");
}
//-------------------------------------------------------------------------
::comphelper::StringSequence SbaExternalSourceBrowser::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.FormGridView");
    return aSupported;
}
//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL SbaExternalSourceBrowser::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new SbaExternalSourceBrowser(_rxFactory));
}
//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL SbaExternalSourceBrowser::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}
//------------------------------------------------------------------------------
Reference< XRowSet >  SbaExternalSourceBrowser::CreateForm()
{
    m_pDataSourceImpl = new SbaXFormAdapter();
    return m_pDataSourceImpl;
}

//------------------------------------------------------------------------------
sal_Bool SbaExternalSourceBrowser::InitializeForm(const Reference< XRowSet > & xForm)
{
    return sal_True;
}

//------------------------------------------------------------------
sal_Bool SbaExternalSourceBrowser::InitializeGridModel(const Reference< ::com::sun::star::form::XFormComponent > & xGrid)
{
    return sal_True;
}

//------------------------------------------------------------------
sal_Bool SbaExternalSourceBrowser::LoadForm()
{
    // as we don't have a main form (yet), we have nothing to do
    // we don't call FormLoaded, because this expects a working data source
    return sal_True;
}


//------------------------------------------------------------------
void SbaExternalSourceBrowser::modified(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    SbaXDataBrowserController::modified(aEvent);

    // multiplex this event to all my listeners
    ::com::sun::star::lang::EventObject aEvt(*this);
    ::cppu::OInterfaceIteratorHelper aIt(m_aModifyListeners);
    while (aIt.hasMoreElements())
        ((::com::sun::star::util::XModifyListener*)aIt.next())->modified(aEvt);
}

// -----------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::attachFrame(const Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( RuntimeException )
{
    Reference< ::com::sun::star::frame::XFrameActionListener >  xAggListener;
    if (m_xFormControllerImpl.is())
        m_xFormControllerImpl->queryAggregation(::getCppuType((const Reference< ::com::sun::star::frame::XFrameActionListener>*)0)) >>= xAggListener;

    // log off my aggregated object
    if (m_xCurrentFrame.is() && xAggListener.is())
        m_xCurrentFrame->removeFrameActionListener(xAggListener);

    SbaXDataBrowserController::attachFrame(xFrame);

    // and log on to the new frame
    if (m_xCurrentFrame.is() && xAggListener.is())
        m_xCurrentFrame->addFrameActionListener(xAggListener);
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::dispatch(const ::com::sun::star::util::URL& aURL, const Sequence< ::com::sun::star::beans::PropertyValue>& aArgs)
{
    const ::com::sun::star::beans::PropertyValue* pArguments = aArgs.getConstArray();
    if (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/AddGridColumn")))
    {
        // search the argument describing the column to create
        ::rtl::OUString sControlType;
        sal_Int32 nControlPos = -1;
        Sequence< ::com::sun::star::beans::PropertyValue> aControlProps;
        for (sal_uInt16 i=0; i<aArgs.getLength(); ++i, ++pArguments)
        {
            if (pArguments->Name.equals(::rtl::OUString::createFromAscii("ColumnType")))
            {
                sal_Bool bCorrectType = pArguments->Value.getValueType().equals(::getCppuType((const ::rtl::OUString*)0));
                OSL_ENSURE(bCorrectType, "invalid type for argument \"ColumnType\" !");
                if (bCorrectType)
                    sControlType = ::comphelper::getString(pArguments->Value);
            }
            else if (pArguments->Name.equals(::rtl::OUString::createFromAscii("ColumnPosition")))
            {
                sal_Bool bCorrectType = pArguments->Value.getValueType().equals(::getCppuType((const sal_Int16*)0));
                OSL_ENSURE(bCorrectType, "invalid type for argument \"ColumnPosition\" !");
                if (bCorrectType)
                    nControlPos = ::comphelper::getINT16(pArguments->Value);
            }
            else if (pArguments->Name.equals(::rtl::OUString::createFromAscii("ColumnProperties")))
            {
                sal_Bool bCorrectType = pArguments->Value.getValueType().equals(::getCppuType((const Sequence< ::com::sun::star::beans::PropertyValue>*)0));
                OSL_ENSURE(bCorrectType, "invalid type for argument \"ColumnProperties\" !");
                if (bCorrectType)
                    aControlProps = *(Sequence< ::com::sun::star::beans::PropertyValue>*)pArguments->Value.getValue();
            }
            else
                OSL_ASSERT(((ByteString("SbaExternalSourceBrowser::dispatch(AddGridColumn) : unknown argument (") += ByteString((const sal_Unicode*)pArguments->Name, gsl_getSystemTextEncoding()).GetBuffer()) += ") !").GetBuffer());
        }
        if (!sControlType.getLength())
        {
            OSL_ASSERT("SbaExternalSourceBrowser::dispatch(AddGridColumn) : missing argument (ColumnType) !");
            sControlType = ::rtl::OUString::createFromAscii("TextField");
        }
        OSL_ENSURE(aControlProps.getLength(), "SbaExternalSourceBrowser::dispatch(AddGridColumn) : missing argument (ColumnProperties) !");

        // create the col
        Reference< ::com::sun::star::form::XGridColumnFactory >  xColFactory(getControlModel(), UNO_QUERY);
        Reference< ::com::sun::star::beans::XPropertySet >  xNewCol = xColFactory->createColumn(sControlType);
        // set it's properties
        const ::com::sun::star::beans::PropertyValue* pControlProps = aControlProps.getConstArray();
        for (i=0; i<aControlProps.getLength(); ++i, ++pControlProps)
        {
            try
            {
                xNewCol->setPropertyValue(pControlProps->Name, pControlProps->Value);
            }
            catch(Exception&)
            {
                OSL_ASSERT("SbaExternalSourceBrowser::dispatch : could not set a column property (maybe you forgot a READONLY attribute for a dispatch-argument ?) !");
            }
            ;
        }

        // correct the position
        Reference< ::com::sun::star::container::XIndexContainer >  xColContainer(getControlModel(), UNO_QUERY);

        if (nControlPos > xColContainer->getCount())
            nControlPos = xColContainer->getCount();
        if (nControlPos < 0)
            nControlPos = 0;

        // append the column
        xColContainer->insertByIndex(nControlPos, makeAny(xNewCol));
    }
    else if (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/ClearView")))
    {
        ClearView();
    }
    else if (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/AttachToForm")))
    {
        if (!m_pDataSourceImpl)
            return;

        Reference< XRowSet >  xMasterForm;
        // search the arguments for he master form
        for (sal_uInt16 i=0; i<aArgs.getLength(); ++i, ++pArguments)
        {
            if ((pArguments->Name.equals(::rtl::OUString::createFromAscii("MasterForm"))) && (pArguments->Value.getValueTypeClass() == TypeClass_INTERFACE))
            {
                xMasterForm = Reference< XRowSet > (*(Reference< XInterface > *)pArguments->Value.getValue(), UNO_QUERY);
                break;
            }
        }
        if (!xMasterForm.is())
        {
            OSL_ASSERT("SbaExternalSourceBrowser::dispatch(FormSlots/AttachToForm) : please specify a form to attach to as argument !");
            return;
        }

        Attach(xMasterForm);
    }
    else
        SbaXDataBrowserController::dispatch(aURL, aArgs);
}

//------------------------------------------------------------------
Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL SbaExternalSourceBrowser::queryDispatch(const ::com::sun::star::util::URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( RuntimeException )
{
    Reference< ::com::sun::star::frame::XDispatch >  xReturn;
    if (m_bInQueryDispatch)
        return xReturn;

    m_bInQueryDispatch = sal_True;

    if  (   (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/AttachToForm")))
            // attach a new external form
        ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/AddGridColumn")))
            // add a column to the grid
        ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/ClearView")))
            // clear the grid
        )
        xReturn = (::com::sun::star::frame::XDispatch*)this;

    if  (   !xReturn.is()
        &&  (   (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/moveToFirst")))
            ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/moveToPrev")))
            ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/moveToNext")))
            ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/moveToLast")))
            ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/moveToNew")))
            ||  (aURL.Complete.equals(::rtl::OUString::createFromAscii(".uno:FormSlots/undoRecord")))
            )
        )
    {
        OSL_ENSURE(aURL.Mark.len() == 0, "SbaExternalSourceBrowser::queryDispatch : the ::com::sun::star::util::URL shouldn't have a mark !");
        ::com::sun::star::util::URL aNewUrl = aURL;

        // split the ::com::sun::star::util::URL
        Reference< ::com::sun::star::util::XURLTransformer >  xTransformer(::comphelper::getProcessServiceFactory()->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")), UNO_QUERY);
        OSL_ENSURE(xTransformer.is(), "SbaExternalSourceBrowser::queryDispatch : could not create an URLTransformer !");
        if (xTransformer.is())
            xTransformer->parseStrict(aNewUrl);

        // set a new mark
        aNewUrl.Mark = ::rtl::OUString::createFromAscii("DB/FormGridView");
            // this controller is instantiated when somebody dispatches the ".component:DB/FormGridView" in any
            // frame, so we use "FormGridView" as mark that a dispatch request came from this view

        if (xTransformer.is())
            xTransformer->assemble(aNewUrl);

        Reference< ::com::sun::star::frame::XDispatchProvider >  xFrameDispatcher(m_xCurrentFrame, UNO_QUERY);
        if (xFrameDispatcher.is())
            xReturn = xFrameDispatcher->queryDispatch(aNewUrl, aTargetFrameName, nSearchFlags);

#if SUPD<=548 && !PRIV_DEBUG
        xReturn = NULL;
        // in version 548 the dispatch interception and the task-local beamer will work ... me hopes
#endif
    }

    if (!xReturn.is())
        xReturn = SbaXDataBrowserController::queryDispatch(aURL, aTargetFrameName, nSearchFlags);

    m_bInQueryDispatch = sal_False;
    return xReturn;
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::disposing()
{
    // say our modify listeners goodbye
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (XWeak*) this;
    m_aModifyListeners.disposeAndClear(aEvt);

    stopListening();

    if (m_xFormControllerImpl.is())
    {
        Reference< ::com::sun::star::lang::XComponent >  xAggComp;
        m_xFormControllerImpl->queryAggregation(::getCppuType((const Reference< ::com::sun::star::lang::XComponent>*)0)) >>= xAggComp;
        if (xAggComp.is())
            xAggComp->dispose();
    }

    // our aggregated object doesn't handle its frame action listening itself, so we have to log it off
    Reference< ::com::sun::star::frame::XFrameActionListener >  xAggListener;
    if (m_xFormControllerImpl.is())
    {
        m_xFormControllerImpl->queryAggregation( ::getCppuType((const Reference< ::com::sun::star::frame::XFrameActionListener >*)0)) >>= xAggListener;
        m_xCurrentFrame->removeFrameActionListener(xAggListener);
    }

    SbaXDataBrowserController::disposing();
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::addModifyListener(const Reference< ::com::sun::star::util::XModifyListener > & aListener) throw( RuntimeException )
{
    m_aModifyListeners.addInterface(aListener);
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::removeModifyListener(const Reference< ::com::sun::star::util::XModifyListener > & aListener) throw( RuntimeException )
{
    m_aModifyListeners.removeInterface(aListener);
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    if (m_pDataSourceImpl && (m_pDataSourceImpl->getAttachedForm() == aEvent.Source))
    {
        ClearView();
    }

    SbaXDataBrowserController::unloading(aEvent);
}

//------------------------------------------------------------------
void SbaExternalSourceBrowser::Attach(const Reference< XRowSet > & xMaster)
{
    // switch the control to design mode
    if (getBrowserView() && getBrowserView()->getGridControl().is())
        getBrowserView()->getGridControl()->setDesignMode(sal_True);

    // the grid will move the form's cursor to the first record, but we want the form to remain unchanged
    Any aOldPos;
    // restore the old position
    sal_Bool bBeforeFirst   = sal_True;
    sal_Bool bAfterLast     = sal_True;
    Reference< ::com::sun::star::sdbc::XResultSet >  xResultSet(xMaster, UNO_QUERY);
    Reference< ::com::sun::star::sdbcx::XRowLocate >  xCursor(xMaster, UNO_QUERY);
    if (xCursor.is() && xResultSet.is())
    {
        bBeforeFirst = xResultSet->isBeforeFirst();
        bAfterLast   = xResultSet->isBeforeFirst();
        if(!bBeforeFirst && !bAfterLast)
            aOldPos = xCursor->getBookmark();
    }

    sal_Bool bWasInsertRow = sal_False;
    Reference< ::com::sun::star::beans::XPropertySet >  xMasterProps(xMaster, UNO_QUERY);
    if (xMasterProps.is())
    {
        try { bWasInsertRow = ::comphelper::getBOOL(xMasterProps->getPropertyValue(PROPERTY_ISNEW)); }
        catch(Exception&)
        { }
    }

    stopListening();
    m_pDataSourceImpl->AttachForm(xMaster);
    startListening();

    if (xMaster.is())
    {
        // at this point we have to reset the formatter for the new form
        initFormatter();
        // assume that the master form is already loaded, we have no chance to check this
        m_bLoadCanceled = sal_False;
        FormLoaded(sal_True);

        Reference< XResultSetUpdate >  xUpdate(xMaster, UNO_QUERY);
        try
        {
            if (bWasInsertRow && xUpdate.is())
                xUpdate->moveToInsertRow();
            else if (xCursor.is() && aOldPos.hasValue())
                xCursor->moveToBookmark(aOldPos);
            else if(bBeforeFirst && xResultSet.is())
                xResultSet->beforeFirst();
            else if(bAfterLast && xResultSet.is())
                xResultSet->afterLast();
        }
        catch(Exception&)
        {
            OSL_ASSERT("SbaExternalSourceBrowser::Attach : couldn't restore the cursor position !");
        }

    }
}

//------------------------------------------------------------------
void SbaExternalSourceBrowser::ClearView()
{
    // set a new (empty) datasource
    Attach(Reference< XRowSet > ());


    // clear all cols in the grid
    Reference< ::com::sun::star::container::XIndexContainer >  xColContainer(getControlModel(), UNO_QUERY);
    while (xColContainer->getCount() > 0)
        xColContainer->removeByIndex(0);
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException )
{
    if (m_pDataSourceImpl && (m_pDataSourceImpl->getAttachedForm() == Source.Source))
    {
        ClearView();
    }

    if (m_xFormControllerImpl != Source.Source)
    {   // forward the event to my aggregate (if it isn't the source itself)
        Reference< ::com::sun::star::lang::XEventListener >  xAggListener;
        m_xFormControllerImpl->queryAggregation(::getCppuType((const Reference< ::com::sun::star::lang::XEventListener >*)0)) >>= xAggListener;
        if (xAggListener.is())
            xAggListener->disposing(Source);
    }

    // the frame
    if (Source.Source == m_xCurrentFrame)
    {
        // our aggregated object doesn't handle its frame action listening itself, so we have to log it off
        Reference< ::com::sun::star::frame::XFrameActionListener >  xAggListener;
        if (m_xFormControllerImpl.is())
        {
            m_xFormControllerImpl->queryAggregation(::getCppuType((const Reference< ::com::sun::star::frame::XFrameActionListener >*)0)) >>= xAggListener;
            m_xCurrentFrame->removeFrameActionListener(xAggListener);
        }
    }
    SbaXDataBrowserController::disposing(Source);
}
//------------------------------------------------------------------
void SbaExternalSourceBrowser::addControlListeners(const Reference< ::com::sun::star::awt::XControl > & _xGridControl)
{
    SbaXDataBrowserController::addControlListeners(_xGridControl);

    Reference< ::com::sun::star::awt::XWindow >  xWindow(_xGridControl, UNO_QUERY);
    if (xWindow.is())
        xWindow->addFocusListener(this);
}

//------------------------------------------------------------------
void SbaExternalSourceBrowser::removeControlListeners(const Reference< ::com::sun::star::awt::XControl > & _xGridControl)
{
    SbaXDataBrowserController::removeControlListeners(_xGridControl);

    Reference< ::com::sun::star::awt::XWindow >  xWindow(_xGridControl, UNO_QUERY);
    if (xWindow.is())
        xWindow->removeFocusListener(this);
}

//------------------------------------------------------------------
void SbaExternalSourceBrowser::startListening()
{
    if (m_pDataSourceImpl && m_pDataSourceImpl->getAttachedForm().is())
    {
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_pDataSourceImpl->getAttachedForm(), UNO_QUERY);
        xLoadable->addLoadListener((::com::sun::star::form::XLoadListener*)this);
    }
}

//------------------------------------------------------------------
void SbaExternalSourceBrowser::stopListening()
{
    if (m_pDataSourceImpl && m_pDataSourceImpl->getAttachedForm().is())
    {
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_pDataSourceImpl->getAttachedForm(), UNO_QUERY);
        xLoadable->removeLoadListener((::com::sun::star::form::XLoadListener*)this);
    }
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::focusGained(const ::com::sun::star::awt::FocusEvent& e) throw( RuntimeException )
{
    ::com::sun::star::lang::EventObject aEvt(*this);
    ::cppu::OInterfaceIteratorHelper aIter(m_pFormControllerImpl->m_aActivateListeners);
    while (aIter.hasMoreElements())
        ((::com::sun::star::form::XFormControllerListener*)aIter.next())->formActivated(aEvt);
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::focusLost(const ::com::sun::star::awt::FocusEvent& e) throw( RuntimeException )
{
    if (!getBrowserView() || !getBrowserView()->getGridControl().is())
        return;
    Reference< ::com::sun::star::awt::XVclWindowPeer >  xMyGridPeer(getBrowserView()->getGridControl()->getPeer(), UNO_QUERY);
    if (!xMyGridPeer.is())
        return;
    Reference< ::com::sun::star::awt::XWindowPeer >  xNextControlPeer(e.NextFocus, UNO_QUERY);
    if (!xNextControlPeer.is())
        return;

    if (xMyGridPeer->isChild(xNextControlPeer))
        return;

    if (xMyGridPeer == xNextControlPeer)
        return;

    ::com::sun::star::lang::EventObject aEvt(*this);
    ::cppu::OInterfaceIteratorHelper aIter(m_pFormControllerImpl->m_aActivateListeners);
    while (aIter.hasMoreElements())
        ((::com::sun::star::form::XFormControllerListener*)aIter.next())->formDeactivated(aEvt);

    // commit the changes of the grid control (as we're deactivated)
    Reference< ::com::sun::star::form::XBoundComponent >  xCommitable(getBrowserView()->getGridControl(), UNO_QUERY);
    if (xCommitable.is())
        xCommitable->commit();
    else
        OSL_ASSERT("SbaExternalSourceBrowser::focusLost : why is my control not commitable ?");
}

//------------------------------------------------------------------
sal_uInt16 SbaExternalSourceBrowser::SaveData(sal_Bool bUI, sal_Bool bForBrowsing)
{
    if (m_bSuspending)
        // don't ask the user if we're beeing suspended currently
        // this is the responsibility of our owner, as we are only an external view to an existing form
        // 73384 - 22.02.00 - FS
        return (sal_uInt16)sal_True;

    return SbaXDataBrowserController::SaveData(bUI, bForBrowsing);
}

//==================================================================
//==================================================================

//------------------------------------------------------------------
SbaExternalSourceBrowser::FormControllerImpl::FormControllerImpl(SbaExternalSourceBrowser* m_pOwner)
    :m_pOwner(m_pOwner)
    ,m_bActive(sal_False)
    ,m_aActivateListeners(m_pOwner->m_aPropertyMutex)
{
    OSL_ENSURE(m_pOwner, "SbaExternalSourceBrowser::FormControllerImpl::FormControllerImpl : invalid Owner !");
}

//------------------------------------------------------------------
SbaExternalSourceBrowser::FormControllerImpl::~FormControllerImpl()
{
}

//------------------------------------------------------------------
Reference< ::com::sun::star::awt::XControl >  SbaExternalSourceBrowser::FormControllerImpl::getCurrentControl(void) throw( RuntimeException )
{
    return m_pOwner->getBrowserView() ? m_pOwner->getBrowserView()->getGridControl() : Reference< ::com::sun::star::awt::XControl > ();
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::addActivateListener(const Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( RuntimeException )
{
    m_aActivateListeners.addInterface(l);
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::removeActivateListener(const Reference< ::com::sun::star::form::XFormControllerListener > & l) throw( RuntimeException )
{
    m_aActivateListeners.removeInterface(l);
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::setModel(const Reference< ::com::sun::star::awt::XTabControllerModel > & Model) throw( RuntimeException )
{
    OSL_ASSERT("SbaExternalSourceBrowser::FormControllerImpl::setModel : invalid call, can't change my model !");
}

//------------------------------------------------------------------
Reference< ::com::sun::star::awt::XTabControllerModel >  SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::getModel(void) throw( RuntimeException )
{
    OSL_ASSERT("SbaExternalSourceBrowser::FormControllerImpl::getModel : don't (and won't) have a model !");
    if (m_pOwner->m_pDataSourceImpl)
        return Reference< ::com::sun::star::awt::XTabControllerModel > ((Reference< XInterface > )*m_pOwner->m_pDataSourceImpl, UNO_QUERY);

    return Reference< ::com::sun::star::awt::XTabControllerModel > ();
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::setContainer(const Reference< ::com::sun::star::awt::XControlContainer > & _Container) throw( RuntimeException )
{
    OSL_ASSERT("SbaExternalSourceBrowser::FormControllerImpl::setContainer : invalid call, can't change my container !");
}

//------------------------------------------------------------------
Reference< ::com::sun::star::awt::XControlContainer >  SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::getContainer(void) throw( RuntimeException )
{
    if (m_pOwner->getBrowserView())
        return m_pOwner->getBrowserView()->getContainer();
    return Reference< ::com::sun::star::awt::XControlContainer > ();
}

//------------------------------------------------------------------
Sequence< Reference< ::com::sun::star::awt::XControl > > SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::getControls(void) throw( RuntimeException )
{
    if (m_pOwner->getBrowserView())
    {
        Reference< ::com::sun::star::awt::XControl >  xGrid = m_pOwner->getBrowserView()->getGridControl();
        return Sequence< Reference< ::com::sun::star::awt::XControl > >(&xGrid, 1);
    }
    return Sequence< Reference< ::com::sun::star::awt::XControl > >();
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::autoTabOrder(void) throw( RuntimeException )
{
    OSL_ASSERT("SbaExternalSourceBrowser::FormControllerImpl::autoTabOrder : nothing to do (always have only one control) !");
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::activateTabOrder(void) throw( RuntimeException )
{
    OSL_ASSERT("SbaExternalSourceBrowser::FormControllerImpl::activateTabOrder : nothing to do (always have only one control) !");
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::activateFirst(void) throw( RuntimeException )
{
    if (m_pOwner->getBrowserView())
        m_pOwner->getBrowserView()->getVclControl()->ActivateCell();
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::activateLast(void) throw( RuntimeException )
{
    if (m_pOwner->getBrowserView())
        m_pOwner->getBrowserView()->getVclControl()->ActivateCell();
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::frameAction(const ::com::sun::star::frame::FrameActionEvent& aEvent) throw( RuntimeException )
{
    OSL_ENSURE(aEvent.Source == m_pOwner->m_xCurrentFrame, "SbaExternalSourceBrowser::FormControllerImpl::frameAction : where did this come frome ?");

    ::com::sun::star::lang::EventObject aEvt(*m_pOwner);
    ::cppu::OInterfaceIteratorHelper aIter(m_aActivateListeners);
    switch (aEvent.Action)
    {
        case ::com::sun::star::frame::FrameAction_FRAME_ACTIVATED:
            // as the frame sends more ACTIVATED than DEACTIVATING events we check this with our own flag, so the listeners
            // will be notified only when the first activation occurs
//          if (!m_bActive)
                // TODO : when de DEACTIVATED-event is implemented (MBA) reinsert this line
//          {
//              while (aIter.hasMoreElements())
//                  ((::com::sun::star::form::XFormControllerListener*)aIter.next())->formActivated(aEvt);
//          }
//          m_bActive = sal_True;
            break;

        case ::com::sun::star::frame::FrameAction_FRAME_DEACTIVATING:
//          while (aIter.hasMoreElements())
//              ((::com::sun::star::form::XFormControllerListener*)aIter.next())->formDeactivated(aEvt);
//          m_bActive = sal_False;
            break;
    }
}

//------------------------------------------------------------------
void SAL_CALL SbaExternalSourceBrowser::FormControllerImpl::disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException )
{
    // nothing to do
    // we don't add ourself as listener to any broadcasters, so we are not resposible for removing us
}

