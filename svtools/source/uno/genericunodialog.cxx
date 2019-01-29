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


#include <svtools/genericunodialog.hxx>

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>

#include <toolkit/awt/vclxwindow.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace css::uno;
using namespace css::lang;
using namespace css::beans;
using namespace css::ucb;


namespace svt
{


OGenericUnoDialog::OGenericUnoDialog(const Reference< XComponentContext >& _rxContext)
        :OPropertyContainer(GetBroadcastHelper())
        ,m_bExecuting(false)
        ,m_bTitleAmbiguous(true)
        ,m_bInitialized( false )
        ,m_bNeedInitialization( false )
        ,m_aContext(_rxContext)
{
    registerProperty(UNODIALOG_PROPERTY_TITLE, UNODIALOG_PROPERTY_ID_TITLE, PropertyAttribute::TRANSIENT,
        &m_sTitle, cppu::UnoType<decltype(m_sTitle)>::get());
    registerProperty(UNODIALOG_PROPERTY_PARENT, UNODIALOG_PROPERTY_ID_PARENT, PropertyAttribute::TRANSIENT,
        &m_xParent, cppu::UnoType<decltype(m_xParent)>::get());
}


OGenericUnoDialog::~OGenericUnoDialog()
{
    if (m_aDialog)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        if (m_aDialog)
            destroyDialog();
    }
}


Any SAL_CALL OGenericUnoDialog::queryInterface(const Type& _rType)
{
    Any aReturn = OGenericUnoDialogBase::queryInterface(_rType);

    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<XPropertySet*>(this)
            ,static_cast<XMultiPropertySet*>(this)
            ,static_cast<XFastPropertySet*>(this)
        );

    return aReturn;
}


Sequence<Type> SAL_CALL OGenericUnoDialog::getTypes(  )
{
    return ::comphelper::concatSequences(
        OGenericUnoDialogBase::getTypes(),
        getBaseTypes()
    );
}

sal_Bool SAL_CALL OGenericUnoDialog::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}


void OGenericUnoDialog::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue )
{
    // TODO: need some handling if we're currently executing ...

    OPropertyContainer::setFastPropertyValue_NoBroadcast(nHandle, rValue);

    if (UNODIALOG_PROPERTY_ID_TITLE == nHandle)
    {
        // from now on m_sTitle is valid
        m_bTitleAmbiguous = false;

        if (m_aDialog)
            m_aDialog.set_title(m_sTitle);
    }
}


sal_Bool OGenericUnoDialog::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
{
    switch (nHandle)
    {
        case UNODIALOG_PROPERTY_ID_PARENT:
        {
            Reference<css::awt::XWindow> xNew(rValue, css::uno::UNO_QUERY);
            if (xNew != m_xParent)
            {
                rConvertedValue <<= xNew;
                rOldValue <<= m_xParent;
                return true;
            }
            return false;
        }
    }
    return OPropertyContainer::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
}


void SAL_CALL OGenericUnoDialog::setTitle( const OUString& _rTitle )
{
    UnoDialogEntryGuard aGuard( *this );

    try
    {
        setPropertyValue(UNODIALOG_PROPERTY_TITLE, makeAny(_rTitle));
    }
    catch(RuntimeException&)
    {
        // allowed to pass
        throw;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("svtools.uno");
        // not allowed to pass
    }
}


bool OGenericUnoDialog::impl_ensureDialog_lck()
{
    if (m_aDialog)
        return true;

    // get the parameters for the dialog from the current settings

    // the title
    OUString sTitle = m_sTitle;

    OGenericUnoDialog::Dialog aDialog(createDialog(m_xParent));
    OSL_ENSURE(aDialog, "OGenericUnoDialog::impl_ensureDialog_lck: createDialog returned nonsense!");
    if (!aDialog)
        return false;

    // do some initialisations
    if (!m_bTitleAmbiguous)
        aDialog.set_title(sTitle);

    if (aDialog.m_xVclDialog)
    {
        // be notified when the dialog is killed by somebody else #i65958#
        aDialog.m_xVclDialog->AddEventListener(LINK(this, OGenericUnoDialog, OnDialogDying));
    }

    m_aDialog = std::move(aDialog);

    return true;
}

sal_Int16 SAL_CALL OGenericUnoDialog::execute()
{
    // both creation and execution of the dialog must be guarded with the SolarMutex, so be generous here
    SolarMutexGuard aSolarGuard;

    ::Dialog* pVclDialogToExecute = nullptr;
    // create the dialog, if necessary
    {
        UnoDialogEntryGuard aGuard( *this );

        if (m_bExecuting)
            throw RuntimeException(
                    "already executing the dialog (recursive call)",
                    *this
                  );

        m_bExecuting = true;

        if ( !impl_ensureDialog_lck() )
            return 0;

        pVclDialogToExecute = m_aDialog.m_xVclDialog;
    }

    // start execution
    sal_Int16 nReturn(0);
    if (pVclDialogToExecute)
        nReturn = pVclDialogToExecute->Execute();
    else if (m_aDialog.m_xWeldDialog)
        nReturn = m_aDialog.m_xWeldDialog->run();

    {
        ::osl::MutexGuard aGuard(m_aMutex);

        // get the settings of the dialog
        executedDialog( nReturn );

        m_bExecuting = false;
    }

    // outta here
    return nReturn;
}

void OGenericUnoDialog::implInitialize(const Any& _rValue)
{
    try
    {
        PropertyValue aProperty;
        NamedValue aValue;
        if ( _rValue >>= aProperty )
        {
            setPropertyValue( aProperty.Name, aProperty.Value );
        }
        else if ( _rValue >>= aValue )
        {
            setPropertyValue( aValue.Name, aValue.Value );
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("svtools.uno");
    }
}


void SAL_CALL OGenericUnoDialog::initialize( const Sequence< Any >& aArguments )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw AlreadyInitializedException( OUString(), *this );

    const Any* pArguments = aArguments.getConstArray();
    for (sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
        implInitialize(*pArguments);

    m_bInitialized = true;
}


void OGenericUnoDialog::destroyDialog()
{
    SolarMutexGuard aSolarGuard;
    m_aDialog.m_xVclDialog.disposeAndClear();
    m_aDialog.m_xWeldDialog.reset();
}


IMPL_LINK( OGenericUnoDialog, OnDialogDying, VclWindowEvent&, _rEvent, void )
{
    OSL_ENSURE( _rEvent.GetWindow() == m_aDialog.m_xVclDialog, "OGenericUnoDialog::OnDialogDying: where does this come from?" );
    if ( _rEvent.GetId() == VclEventId::ObjectDying )
        m_aDialog.m_xVclDialog = nullptr;
}


}   // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
