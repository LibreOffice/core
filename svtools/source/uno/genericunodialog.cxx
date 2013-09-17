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


#include "svtools/genericunodialog.hxx"

#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>

#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/extract.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/property.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ucb;

//.........................................................................
namespace svt
{
//.........................................................................

//-------------------------------------------------------------------------
OGenericUnoDialog::OGenericUnoDialog(const Reference< XComponentContext >& _rxContext)
        :OPropertyContainer(GetBroadcastHelper())
        ,m_pDialog(NULL)
        ,m_bExecuting(sal_False)
        ,m_bCanceled(sal_False)
        ,m_bTitleAmbiguous(sal_True)
        ,m_bInitialized( false )
        ,m_bNeedInitialization( false )
        ,m_aContext(_rxContext)
{
    registerProperty(OUString(UNODIALOG_PROPERTY_TITLE), UNODIALOG_PROPERTY_ID_TITLE, PropertyAttribute::TRANSIENT,
        &m_sTitle, getCppuType(&m_sTitle));
    registerProperty(OUString(UNODIALOG_PROPERTY_PARENT), UNODIALOG_PROPERTY_ID_PARENT, PropertyAttribute::TRANSIENT,
        &m_xParent, getCppuType(&m_xParent));
}

//-------------------------------------------------------------------------
OGenericUnoDialog::~OGenericUnoDialog()
{
    if ( m_pDialog )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( m_pDialog )
            destroyDialog();
    }
}

//-------------------------------------------------------------------------
Any SAL_CALL OGenericUnoDialog::queryInterface(const Type& _rType) throw (RuntimeException)
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

//-------------------------------------------------------------------------
Sequence<Type> SAL_CALL OGenericUnoDialog::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(
        OGenericUnoDialogBase::getTypes(),
        ::comphelper::OPropertyContainer::getTypes()
    );
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OGenericUnoDialog::supportsService(const OUString& ServiceName) throw(RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pArray = aSupported.getConstArray();
    for (sal_Int32 i = 0; i < aSupported.getLength(); ++i, ++pArray)
        if (pArray->equals(ServiceName))
            return sal_True;
    return sal_False;
}

//-------------------------------------------------------------------------
void OGenericUnoDialog::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const Any& rValue ) throw(Exception)
{
    // TODO : need some handling if we're currently executing ...

    OPropertyContainer::setFastPropertyValue_NoBroadcast(nHandle, rValue);

    if (UNODIALOG_PROPERTY_ID_TITLE == nHandle)
    {
        // from now on m_sTitle is valid
        m_bTitleAmbiguous = sal_False;

        if (m_pDialog)
            m_pDialog->SetText(m_sTitle);
    }
}

//-------------------------------------------------------------------------
sal_Bool OGenericUnoDialog::convertFastPropertyValue( Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue) throw(IllegalArgumentException)
{
    switch (nHandle)
    {
        case UNODIALOG_PROPERTY_ID_PARENT:
        {
            Reference<starawt::XWindow> xNew;
            ::cppu::extractInterface(xNew, rValue);
            if (xNew != m_xParent)
            {
                rConvertedValue <<= xNew;
                rOldValue <<= m_xParent;
                return sal_True;
            }
            return sal_False;
        }
    }
    return OPropertyContainer::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
}

//-------------------------------------------------------------------------
void SAL_CALL OGenericUnoDialog::setTitle( const OUString& _rTitle ) throw(RuntimeException)
{
    UnoDialogEntryGuard aGuard( *this );

    try
    {
        setPropertyValue(OUString(UNODIALOG_PROPERTY_TITLE), makeAny(_rTitle));
    }
    catch(RuntimeException&)
    {
        // allowed to pass
        throw;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        // not allowed to pass
    }
}

//-------------------------------------------------------------------------
bool OGenericUnoDialog::impl_ensureDialog_lck()
{
    if ( m_pDialog )
        return true;

    // get the parameters for the dialog from the current settings

    // the parent window
    Window* pParent = NULL;
    VCLXWindow* pImplementation = VCLXWindow::GetImplementation(m_xParent);
    if (pImplementation)
        pParent = pImplementation->GetWindow();

    // the title
    OUString sTitle = m_sTitle;

    Dialog* pDialog = createDialog( pParent );
    OSL_ENSURE( pDialog, "OGenericUnoDialog::impl_ensureDialog_lck: createDialog returned nonsense!" );
    if ( !pDialog )
        return false;

    // do some initialisations
    if ( !m_bTitleAmbiguous )
        pDialog->SetText( sTitle );

    // be notified when the dialog is killed by somebody else
    // #i65958# / 2006-07-07 / frank.schoenheit@sun.com
    pDialog->AddEventListener( LINK( this, OGenericUnoDialog, OnDialogDying ) );

    m_pDialog = pDialog;

    return true;
}

//-------------------------------------------------------------------------
sal_Int16 SAL_CALL OGenericUnoDialog::execute(  ) throw(RuntimeException)
{
    // both creation and execution of the dialog must be guarded with the SolarMutex, so be generous here
    SolarMutexGuard aSolarGuard;

    Dialog* pDialogToExecute = NULL;
    // create the dialog, if necessary
    {
        UnoDialogEntryGuard aGuard( *this );

        if (m_bExecuting)
            throw RuntimeException(
                    OUString( "already executing the dialog (recursive call)" ),
                    *this
                  );

        m_bCanceled = sal_False;
        m_bExecuting = sal_True;

        if ( !impl_ensureDialog_lck() )
            return 0;

        pDialogToExecute = m_pDialog;
    }

    // start execution
    sal_Int16 nReturn(0);
    if ( pDialogToExecute )
        nReturn = pDialogToExecute->Execute();

    {
        ::osl::MutexGuard aExecutionGuard(m_aExecutionMutex);
        if (m_bCanceled)
            nReturn = RET_CANCEL;
    }

    {
        ::osl::MutexGuard aGuard(m_aMutex);

        // get the settings of the dialog
        executedDialog( nReturn );

        m_bExecuting = sal_False;
    }

    // outta here
    return nReturn;
}

#ifdef AWT_DIALOG
//-------------------------------------------------------------------------
void SAL_CALL OGenericUnoDialog::endExecute(  ) throw(RuntimeException)
{
    UnoDialogEntryGuard aGuard( *this );
    if (!m_bExecuting)
        throw RuntimeException();

    {
        ::osl::MutexGuard aExecutionGuard(m_aExecutionMutex);
        OSL_ENSURE(m_pDialog, "OGenericUnoDialog::endExecute : executing which dialog ?");
            // m_bExecuting is true but we have no dialog ?
        if (!m_pDialog)
            throw RuntimeException();

        if (!m_pDialog->IsInExecute())
            // we tighly missed it ... another thread finished the execution of the dialog,
            // but did not manage it to reset m_bExecuting, it currently tries to acquire
            // m_aMutex or m_aExecutionMutex
            // => nothing to do
            return;

        m_pDialog->EndDialog(RET_CANCEL);
        m_bCanceled = sal_True;
    }
}
#endif

//-------------------------------------------------------------------------
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
        DBG_UNHANDLED_EXCEPTION();
    }
}

//-------------------------------------------------------------------------
void SAL_CALL OGenericUnoDialog::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw AlreadyInitializedException( OUString(), *this );

    const Any* pArguments = aArguments.getConstArray();
    for (sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
        implInitialize(*pArguments);

    m_bInitialized = true;
}

//-------------------------------------------------------------------------
void OGenericUnoDialog::destroyDialog()
{
    delete m_pDialog;
    m_pDialog = NULL;
}

//-------------------------------------------------------------------------
IMPL_LINK( OGenericUnoDialog, OnDialogDying, VclWindowEvent*, _pEvent )
{
    OSL_ENSURE( _pEvent->GetWindow() == m_pDialog, "OGenericUnoDialog::OnDialogDying: where does this come from?" );
    if ( _pEvent->GetId() == VCLEVENT_OBJECT_DYING )
        m_pDialog = NULL;
    return 0L;
}

//.........................................................................
}   // namespace svt
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
