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

#include "exsrcbrw.hxx"
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include "formadapter.hxx"
#include <comphelper/processfactory.hxx>
#include "dbustrings.hrc"
#include "dbu_reghelper.hxx"
#include <tools/diagnose_ex.h>
#include <rtl/strbuf.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace dbaui;

// SbaExternalSourceBrowser
extern "C" void SAL_CALL createRegistryInfo_OFormGridView()
{
    static OMultiInstanceAutoRegistration< SbaExternalSourceBrowser > aAutoRegistration;
}

Any SAL_CALL SbaExternalSourceBrowser::queryInterface(const Type& _rType) throw (RuntimeException)
{
    Any aRet = SbaXDataBrowserController::queryInterface(_rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(_rType,
                                (::com::sun::star::util::XModifyBroadcaster*)this,
                                (::com::sun::star::form::XLoadListener*)this);

    return aRet;
}

DBG_NAME(SbaExternalSourceBrowser)
SbaExternalSourceBrowser::SbaExternalSourceBrowser(const Reference< ::com::sun::star::uno::XComponentContext >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_aModifyListeners(getMutex())
    ,m_pDataSourceImpl(NULL)
    ,m_bInQueryDispatch( sal_False )
{
    DBG_CTOR(SbaExternalSourceBrowser,NULL);

}

SbaExternalSourceBrowser::~SbaExternalSourceBrowser()
{

    DBG_DTOR(SbaExternalSourceBrowser,NULL);
}

::comphelper::StringSequence SAL_CALL SbaExternalSourceBrowser::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

OUString SbaExternalSourceBrowser::getImplementationName_Static() throw(RuntimeException)
{
    return OUString("org.openoffice.comp.dbu.OFormGridView");
}

::comphelper::StringSequence SbaExternalSourceBrowser::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = OUString("com.sun.star.sdb.FormGridView");
    return aSupported;
}

Reference< XInterface > SAL_CALL SbaExternalSourceBrowser::Create(const Reference<XMultiServiceFactory >& _rxFactory)
{
    return *(new SbaExternalSourceBrowser( comphelper::getComponentContext(_rxFactory)));
}

OUString SAL_CALL SbaExternalSourceBrowser::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

Reference< XRowSet >  SbaExternalSourceBrowser::CreateForm()
{
    m_pDataSourceImpl = new SbaXFormAdapter();
    return m_pDataSourceImpl;
}

sal_Bool SbaExternalSourceBrowser::InitializeForm(const Reference< XPropertySet > & /*i_formProperties*/)
{
    return sal_True;
}

sal_Bool SbaExternalSourceBrowser::LoadForm()
{
    // as we don't have a main form (yet), we have nothing to do
    // we don't call FormLoaded, because this expects a working data source
    return sal_True;
}

void SbaExternalSourceBrowser::modified(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    SbaXDataBrowserController::modified(aEvent);

    // multiplex this event to all my listeners
    ::com::sun::star::lang::EventObject aEvt(*this);
    ::cppu::OInterfaceIteratorHelper aIt(m_aModifyListeners);
    while (aIt.hasMoreElements())
        ((::com::sun::star::util::XModifyListener*)aIt.next())->modified(aEvt);
}

void SAL_CALL SbaExternalSourceBrowser::dispatch(const ::com::sun::star::util::URL& aURL, const Sequence< ::com::sun::star::beans::PropertyValue>& aArgs) throw(::com::sun::star::uno::RuntimeException)
{
    const ::com::sun::star::beans::PropertyValue* pArguments = aArgs.getConstArray();
    if ( aURL.Complete == ".uno:FormSlots/AddGridColumn" )
    {
        // search the argument describing the column to create
        OUString sControlType;
        sal_Int32 nControlPos = -1;
        Sequence< ::com::sun::star::beans::PropertyValue> aControlProps;
        sal_uInt16 i;
        for ( i = 0; i < aArgs.getLength(); ++i, ++pArguments )
        {
            if ( pArguments->Name == "ColumnType" )
            {
                sal_Bool bCorrectType = pArguments->Value.getValueType().equals(::getCppuType((const OUString*)0));
                OSL_ENSURE(bCorrectType, "invalid type for argument \"ColumnType\" !");
                if (bCorrectType)
                    sControlType = ::comphelper::getString(pArguments->Value);
            }
            else if ( pArguments->Name == "ColumnPosition" )
            {
                sal_Bool bCorrectType = pArguments->Value.getValueType().equals(::getCppuType((const sal_Int16*)0));
                OSL_ENSURE(bCorrectType, "invalid type for argument \"ColumnPosition\" !");
                if (bCorrectType)
                    nControlPos = ::comphelper::getINT16(pArguments->Value);
            }
            else if ( pArguments->Name == "ColumnProperties" )
            {
                sal_Bool bCorrectType = pArguments->Value.getValueType().equals(::getCppuType((const Sequence< ::com::sun::star::beans::PropertyValue>*)0));
                OSL_ENSURE(bCorrectType, "invalid type for argument \"ColumnProperties\" !");
                if (bCorrectType)
                    aControlProps = *(Sequence< ::com::sun::star::beans::PropertyValue>*)pArguments->Value.getValue();
            }
            else
                SAL_WARN("dbaccess.ui", "SbaExternalSourceBrowser::dispatch(AddGridColumn) : unknown argument (" << pArguments->Name << ") !");
        }
        if (sControlType.isEmpty())
        {
            SAL_WARN("dbaccess.ui", "SbaExternalSourceBrowser::dispatch(AddGridColumn) : missing argument (ColumnType) !");
            sControlType = OUString("TextField");
        }
        OSL_ENSURE(aControlProps.getLength(), "SbaExternalSourceBrowser::dispatch(AddGridColumn) : missing argument (ColumnProperties) !");

        // create the col
        Reference< ::com::sun::star::form::XGridColumnFactory >  xColFactory(getControlModel(), UNO_QUERY);
        Reference< ::com::sun::star::beans::XPropertySet >  xNewCol = xColFactory->createColumn(sControlType);
        Reference< XPropertySetInfo > xNewColProperties;
        if (xNewCol.is())
            xNewColProperties = xNewCol->getPropertySetInfo();
        // set its properties
        if (xNewColProperties.is())
        {
            const ::com::sun::star::beans::PropertyValue* pControlProps = aControlProps.getConstArray();
            for (i=0; i<aControlProps.getLength(); ++i, ++pControlProps)
            {
                try
                {
                    if (xNewColProperties->hasPropertyByName(pControlProps->Name))
                        xNewCol->setPropertyValue(pControlProps->Name, pControlProps->Value);
                }
                catch (const Exception&)
                {
                    SAL_WARN("dbaccess.ui", "SbaExternalSourceBrowser::dispatch : could not set a column property (" << pControlProps->Name << ")!");
                }
            }
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
    else if ( aURL.Complete == ".uno:FormSlots/ClearView" )
    {
        ClearView();
    }
    else if ( aURL.Complete == ".uno:FormSlots/AttachToForm" )
    {
        if (!m_pDataSourceImpl)
            return;

        Reference< XRowSet >  xMasterForm;
        // search the arguments for the master form
        for (sal_uInt16 i=0; i<aArgs.getLength(); ++i, ++pArguments)
        {
            if ( (pArguments->Name == "MasterForm") && (pArguments->Value.getValueTypeClass() == TypeClass_INTERFACE) )
            {
                xMasterForm = Reference< XRowSet > (*(Reference< XInterface > *)pArguments->Value.getValue(), UNO_QUERY);
                break;
            }
        }
        if (!xMasterForm.is())
        {
            SAL_WARN("dbaccess.ui", "SbaExternalSourceBrowser::dispatch(FormSlots/AttachToForm) : please specify a form to attach to as argument !");
            return;
        }

        Attach(xMasterForm);
    }
    else
        SbaXDataBrowserController::dispatch(aURL, aArgs);
}

Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL SbaExternalSourceBrowser::queryDispatch(const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( RuntimeException )
{
    Reference< ::com::sun::star::frame::XDispatch >  xReturn;
    if (m_bInQueryDispatch)
        return xReturn;

    m_bInQueryDispatch = sal_True;

    if  (   ( aURL.Complete == ".uno:FormSlots/AttachToForm" )
            // attach a new external form
        ||  ( aURL.Complete == ".uno:FormSlots/AddGridColumn" )
            // add a column to the grid
        ||  ( aURL.Complete == ".uno:FormSlots/ClearView" )
            // clear the grid
        )
        xReturn = (::com::sun::star::frame::XDispatch*)this;

    if  (   !xReturn.is()
        &&  (   (aURL.Complete == ".uno:FormSlots/moveToFirst" ) ||  (aURL.Complete == ".uno:FormSlots/moveToPrev" )
            ||  (aURL.Complete == ".uno:FormSlots/moveToNext" ) ||  (aURL.Complete == ".uno:FormSlots/moveToLast" )
            ||  (aURL.Complete == ".uno:FormSlots/moveToNew" ) ||  (aURL.Complete == ".uno:FormSlots/undoRecord" )
            )
        )
    {
        OSL_ENSURE(aURL.Mark.isEmpty(), "SbaExternalSourceBrowser::queryDispatch : the ::com::sun::star::util::URL shouldn't have a mark !");
        ::com::sun::star::util::URL aNewUrl = aURL;

        // split the ::com::sun::star::util::URL
        OSL_ENSURE( m_xUrlTransformer.is(), "SbaExternalSourceBrowser::queryDispatch : could not create an URLTransformer !" );
        if ( m_xUrlTransformer.is() )
            m_xUrlTransformer->parseStrict( aNewUrl );

        // set a new mark
        aNewUrl.Mark = OUString("DB/FormGridView");
            // this controller is instantiated when somebody dispatches the ".component:DB/FormGridView" in any
            // frame, so we use "FormGridView" as mark that a dispatch request came from this view

        if (m_xUrlTransformer.is())
            m_xUrlTransformer->assemble(aNewUrl);

        Reference< XDispatchProvider >  xFrameDispatcher( getFrame(), UNO_QUERY );
        if (xFrameDispatcher.is())
            xReturn = xFrameDispatcher->queryDispatch(aNewUrl, aTargetFrameName, FrameSearchFlag::PARENT);

    }

    if (!xReturn.is())
        xReturn = SbaXDataBrowserController::queryDispatch(aURL, aTargetFrameName, nSearchFlags);

    m_bInQueryDispatch = sal_False;
    return xReturn;
}

void SAL_CALL SbaExternalSourceBrowser::disposing()
{
    // say our modify listeners goodbye
    ::com::sun::star::lang::EventObject aEvt;
    aEvt.Source = (XWeak*) this;
    m_aModifyListeners.disposeAndClear(aEvt);

    stopListening();

    SbaXDataBrowserController::disposing();
}

void SAL_CALL SbaExternalSourceBrowser::addModifyListener(const Reference< ::com::sun::star::util::XModifyListener > & aListener) throw( RuntimeException )
{
    m_aModifyListeners.addInterface(aListener);
}

void SAL_CALL SbaExternalSourceBrowser::removeModifyListener(const Reference< ::com::sun::star::util::XModifyListener > & aListener) throw( RuntimeException )
{
    m_aModifyListeners.removeInterface(aListener);
}

void SAL_CALL SbaExternalSourceBrowser::unloading(const ::com::sun::star::lang::EventObject& aEvent) throw( RuntimeException )
{
    if (m_pDataSourceImpl && (m_pDataSourceImpl->getAttachedForm() == aEvent.Source))
    {
        ClearView();
    }

    SbaXDataBrowserController::unloading(aEvent);
}

void SbaExternalSourceBrowser::Attach(const Reference< XRowSet > & xMaster)
{
    Any aOldPos;
    sal_Bool bWasInsertRow = sal_False;
    sal_Bool bBeforeFirst   = sal_True;
    sal_Bool bAfterLast     = sal_True;
    Reference< XRowLocate > xCursor(xMaster, UNO_QUERY);
    Reference< XPropertySet > xMasterProps(xMaster, UNO_QUERY);

    try
    {
        // switch the control to design mode
        if (getBrowserView() && getBrowserView()->getGridControl().is())
            getBrowserView()->getGridControl()->setDesignMode(sal_True);

        // the grid will move the form's cursor to the first record, but we want the form to remain unchanged
        // restore the old position
        if (xCursor.is() && xMaster.is())
        {
            bBeforeFirst = xMaster->isBeforeFirst();
            bAfterLast   = xMaster->isAfterLast();
            if(!bBeforeFirst && !bAfterLast)
                aOldPos = xCursor->getBookmark();
        }

        if (xMasterProps.is())
            xMasterProps->getPropertyValue(PROPERTY_ISNEW) >>= bWasInsertRow;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    onStartLoading( Reference< XLoadable >( xMaster, UNO_QUERY ) );

    stopListening();
    m_pDataSourceImpl->AttachForm(xMaster);
    startListening();

    if (xMaster.is())
    {
        // at this point we have to reset the formatter for the new form
        initFormatter();
        // assume that the master form is already loaded
#if OSL_DEBUG_LEVEL > 0
        {
            Reference< XLoadable > xLoadable( xMaster, UNO_QUERY );
            OSL_ENSURE( xLoadable.is() && xLoadable->isLoaded(), "SbaExternalSourceBrowser::Attach: master is not loaded!" );
        }
#endif

        LoadFinished(sal_True);

        Reference< XResultSetUpdate >  xUpdate(xMaster, UNO_QUERY);
        try
        {
            if (bWasInsertRow && xUpdate.is())
                xUpdate->moveToInsertRow();
            else if (xCursor.is() && aOldPos.hasValue())
                xCursor->moveToBookmark(aOldPos);
            else if(bBeforeFirst && xMaster.is())
                xMaster->beforeFirst();
            else if(bAfterLast && xMaster.is())
                xMaster->afterLast();
        }
        catch(Exception&)
        {
            SAL_WARN("dbaccess.ui", "SbaExternalSourceBrowser::Attach : couldn't restore the cursor position !");
        }

    }
}

void SbaExternalSourceBrowser::ClearView()
{
    // set a new (empty) datasource
    Attach(Reference< XRowSet > ());

    // clear all cols in the grid
    Reference< ::com::sun::star::container::XIndexContainer >  xColContainer(getControlModel(), UNO_QUERY);
    while (xColContainer->getCount() > 0)
        xColContainer->removeByIndex(0);
}

void SAL_CALL SbaExternalSourceBrowser::disposing(const ::com::sun::star::lang::EventObject& Source) throw( RuntimeException )
{
    if (m_pDataSourceImpl && (m_pDataSourceImpl->getAttachedForm() == Source.Source))
    {
        ClearView();
    }

    SbaXDataBrowserController::disposing(Source);
}

void SbaExternalSourceBrowser::startListening()
{
    if (m_pDataSourceImpl && m_pDataSourceImpl->getAttachedForm().is())
    {
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_pDataSourceImpl->getAttachedForm(), UNO_QUERY);
        xLoadable->addLoadListener((::com::sun::star::form::XLoadListener*)this);
    }
}

void SbaExternalSourceBrowser::stopListening()
{
    if (m_pDataSourceImpl && m_pDataSourceImpl->getAttachedForm().is())
    {
        Reference< ::com::sun::star::form::XLoadable >  xLoadable(m_pDataSourceImpl->getAttachedForm(), UNO_QUERY);
        xLoadable->removeLoadListener((::com::sun::star::form::XLoadListener*)this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
