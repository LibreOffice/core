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

#include <exsrcbrw.hxx>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/form/XLoadable.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <formadapter.hxx>
#include <strings.hxx>
#include <o3tl/any.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::frame;
using namespace dbaui;

// SbaExternalSourceBrowser
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
org_openoffice_comp_dbu_OFormGridView_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new SbaExternalSourceBrowser(context));
}

Any SAL_CALL SbaExternalSourceBrowser::queryInterface(const Type& _rType)
{
    Any aRet = SbaXDataBrowserController::queryInterface(_rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(_rType,
                                static_cast<css::util::XModifyBroadcaster*>(this),
                                static_cast<css::form::XLoadListener*>(this));

    return aRet;
}

SbaExternalSourceBrowser::SbaExternalSourceBrowser(const Reference< css::uno::XComponentContext >& _rM)
    :SbaXDataBrowserController(_rM)
    ,m_aModifyListeners(getMutex())
    ,m_bInQueryDispatch( false )
{

}

SbaExternalSourceBrowser::~SbaExternalSourceBrowser()
{
}

css::uno::Sequence<OUString> SAL_CALL SbaExternalSourceBrowser::getSupportedServiceNames()
{
    return { u"com.sun.star.sdb.FormGridView"_ustr };
}

OUString SAL_CALL SbaExternalSourceBrowser::getImplementationName()
{
    return u"org.openoffice.comp.dbu.OFormGridView"_ustr;
}

Reference< XRowSet >  SbaExternalSourceBrowser::CreateForm()
{
    m_pDataSourceImpl = new SbaXFormAdapter();
    return m_pDataSourceImpl;
}

bool SbaExternalSourceBrowser::InitializeForm(const Reference< XPropertySet > & /*i_formProperties*/)
{
    return true;
}

bool SbaExternalSourceBrowser::LoadForm()
{
    // as we don't have a main form (yet), we have nothing to do
    // we don't call FormLoaded, because this expects a working data source
    return true;
}

void SbaExternalSourceBrowser::modified(const css::lang::EventObject& aEvent)
{
    SbaXDataBrowserController::modified(aEvent);

    // multiplex this event to all my listeners
    css::lang::EventObject aEvt(*this);
    m_aModifyListeners.notifyEach( &css::util::XModifyListener::modified, aEvt );
}

void SAL_CALL SbaExternalSourceBrowser::dispatch(const css::util::URL& aURL, const Sequence< css::beans::PropertyValue>& aArgs)
{
    if ( aURL.Complete == ".uno:FormSlots/AddGridColumn" )
    {
        // search the argument describing the column to create
        OUString sControlType;
        sal_Int32 nControlPos = -1;
        Sequence< css::beans::PropertyValue> aControlProps;
        for ( const css::beans::PropertyValue& rArgument : aArgs )
        {
            if ( rArgument.Name == "ColumnType" )
            {
                auto s = o3tl::tryAccess<OUString>(rArgument.Value);
                OSL_ENSURE(s, "invalid type for argument \"ColumnType\" !");
                if (s)
                    sControlType = *s;
            }
            else if ( rArgument.Name == "ColumnPosition" )
            {
                auto n = o3tl::tryAccess<sal_Int16>(rArgument.Value);
                OSL_ENSURE(n, "invalid type for argument \"ColumnPosition\" !");
                if (n)
                    nControlPos = *n;
            }
            else if ( rArgument.Name == "ColumnProperties" )
            {
                auto s = o3tl::tryAccess<Sequence<css::beans::PropertyValue>>(
                    rArgument.Value);
                OSL_ENSURE(s, "invalid type for argument \"ColumnProperties\" !");
                if (s)
                    aControlProps = *s;
            }
            else
                SAL_WARN("dbaccess.ui", "SbaExternalSourceBrowser::dispatch(AddGridColumn) : unknown argument (" << rArgument.Name << ") !");
        }
        if (sControlType.isEmpty())
        {
            SAL_WARN("dbaccess.ui", "SbaExternalSourceBrowser::dispatch(AddGridColumn) : missing argument (ColumnType) !");
            sControlType = "TextField";
        }
        OSL_ENSURE(aControlProps.hasElements(), "SbaExternalSourceBrowser::dispatch(AddGridColumn) : missing argument (ColumnProperties) !");

        // create the col
        Reference< css::form::XGridColumnFactory >  xColFactory(getControlModel(), UNO_QUERY);
        Reference< css::beans::XPropertySet >  xNewCol = xColFactory->createColumn(sControlType);
        Reference< XPropertySetInfo > xNewColProperties;
        if (xNewCol.is())
            xNewColProperties = xNewCol->getPropertySetInfo();
        // set its properties
        if (xNewColProperties.is())
        {
            for (const css::beans::PropertyValue& rControlProp : aControlProps)
            {
                try
                {
                    if (xNewColProperties->hasPropertyByName(rControlProp.Name))
                        xNewCol->setPropertyValue(rControlProp.Name, rControlProp.Value);
                }
                catch (const Exception&)
                {
                    SAL_WARN("dbaccess.ui", "SbaExternalSourceBrowser::dispatch : could not set a column property (" << rControlProp.Name << ")!");
                }
            }
        }

        // correct the position
        Reference< css::container::XIndexContainer >  xColContainer(getControlModel(), UNO_QUERY);

        if (nControlPos > xColContainer->getCount())
            nControlPos = xColContainer->getCount();
        if (nControlPos < 0)
            nControlPos = 0;

        // append the column
        xColContainer->insertByIndex(nControlPos, Any(xNewCol));
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
        for (const css::beans::PropertyValue& rArgument : aArgs)
        {
            if ( (rArgument.Name == "MasterForm") && (rArgument.Value.getValueTypeClass() == TypeClass_INTERFACE) )
            {
                xMasterForm.set(rArgument.Value, UNO_QUERY);
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

Reference< css::frame::XDispatch >  SAL_CALL SbaExternalSourceBrowser::queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags)
{
    Reference< css::frame::XDispatch >  xReturn;
    if (m_bInQueryDispatch)
        return xReturn;

    m_bInQueryDispatch = true;

    if  (   ( aURL.Complete == ".uno:FormSlots/AttachToForm" )
            // attach a new external form
        ||  ( aURL.Complete == ".uno:FormSlots/AddGridColumn" )
            // add a column to the grid
        ||  ( aURL.Complete == ".uno:FormSlots/ClearView" )
            // clear the grid
        )
        xReturn = static_cast<css::frame::XDispatch*>(this);

    if  (   !xReturn.is()
        &&  (   (aURL.Complete == ".uno:FormSlots/moveToFirst" ) ||  (aURL.Complete == ".uno:FormSlots/moveToPrev" )
            ||  (aURL.Complete == ".uno:FormSlots/moveToNext" ) ||  (aURL.Complete == ".uno:FormSlots/moveToLast" )
            ||  (aURL.Complete == ".uno:FormSlots/moveToNew" ) ||  (aURL.Complete == ".uno:FormSlots/undoRecord" )
            )
        )
    {
        OSL_ENSURE(aURL.Mark.isEmpty(), "SbaExternalSourceBrowser::queryDispatch : the css::util::URL shouldn't have a mark !");
        css::util::URL aNewUrl = aURL;

        // split the css::util::URL
        OSL_ENSURE( m_xUrlTransformer.is(), "SbaExternalSourceBrowser::queryDispatch : could not create a URLTransformer !" );
        if ( m_xUrlTransformer.is() )
            m_xUrlTransformer->parseStrict( aNewUrl );

        // set a new mark
        aNewUrl.Mark = "DB/FormGridView";
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

    m_bInQueryDispatch = false;
    return xReturn;
}

void SAL_CALL SbaExternalSourceBrowser::disposing()
{
    // say our modify listeners goodbye
    css::lang::EventObject aEvt;
    aEvt.Source = static_cast<XWeak*>(this);
    m_aModifyListeners.disposeAndClear(aEvt);

    stopListening();

    SbaXDataBrowserController::disposing();
}

void SAL_CALL SbaExternalSourceBrowser::addModifyListener(const Reference< css::util::XModifyListener > & aListener)
{
    m_aModifyListeners.addInterface(aListener);
}

void SAL_CALL SbaExternalSourceBrowser::removeModifyListener(const Reference< css::util::XModifyListener > & aListener)
{
    m_aModifyListeners.removeInterface(aListener);
}

void SAL_CALL SbaExternalSourceBrowser::unloading(const css::lang::EventObject& aEvent)
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
    bool bWasInsertRow = false;
    bool bBeforeFirst   = true;
    bool bAfterLast     = true;
    Reference< XRowLocate > xCursor(xMaster, UNO_QUERY);
    Reference< XPropertySet > xMasterProps(xMaster, UNO_QUERY);

    try
    {
        // switch the control to design mode
        if (getBrowserView() && getBrowserView()->getGridControl().is())
            getBrowserView()->getGridControl()->setDesignMode(true);

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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    onStartLoading( Reference< XLoadable >( xMaster, UNO_QUERY ) );

    stopListening();
    m_pDataSourceImpl->AttachForm(xMaster);
    startListening();

    if (!xMaster.is())
        return;

    // at this point we have to reset the formatter for the new form
    initFormatter();
    // assume that the master form is already loaded
#if OSL_DEBUG_LEVEL > 0
    {
        Reference< XLoadable > xLoadable( xMaster, UNO_QUERY );
        OSL_ENSURE( xLoadable.is() && xLoadable->isLoaded(), "SbaExternalSourceBrowser::Attach: master is not loaded!" );
    }
#endif

    LoadFinished(true);

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

void SbaExternalSourceBrowser::ClearView()
{
    // set a new (empty) datasource
    Attach(Reference< XRowSet > ());

    // clear all cols in the grid
    Reference< css::container::XIndexContainer >  xColContainer(getControlModel(), UNO_QUERY);
    while (xColContainer->getCount() > 0)
        xColContainer->removeByIndex(0);
}

void SAL_CALL SbaExternalSourceBrowser::disposing(const css::lang::EventObject& Source)
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
        Reference< css::form::XLoadable >  xLoadable(m_pDataSourceImpl->getAttachedForm(), UNO_QUERY);
        xLoadable->addLoadListener(static_cast<css::form::XLoadListener*>(this));
    }
}

void SbaExternalSourceBrowser::stopListening()
{
    if (m_pDataSourceImpl && m_pDataSourceImpl->getAttachedForm().is())
    {
        Reference< css::form::XLoadable >  xLoadable(m_pDataSourceImpl->getAttachedForm(), UNO_QUERY);
        xLoadable->removeLoadListener(static_cast<css::form::XLoadListener*>(this));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
