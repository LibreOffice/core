/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

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
#include <vos/mutex.hxx>
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

//=========================================================================
//-------------------------------------------------------------------------
OGenericUnoDialog::OGenericUnoDialog(const Reference< XMultiServiceFactory >& _rxORB)
        :OPropertyContainer(GetBroadcastHelper())
        ,m_pDialog(NULL)
        ,m_bExecuting(sal_False)
        ,m_bCanceled(sal_False)
        ,m_bTitleAmbiguous(sal_True)
        ,m_bInitialized( false )
        ,m_bNeedInitialization( false )
        ,m_aContext( _rxORB )
{
    registerProperty(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_TITLE), UNODIALOG_PROPERTY_ID_TITLE, PropertyAttribute::TRANSIENT,
        &m_sTitle, getCppuType(&m_sTitle));
    registerProperty(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_PARENT), UNODIALOG_PROPERTY_ID_PARENT, PropertyAttribute::TRANSIENT,
        &m_xParent, getCppuType(&m_xParent));
}

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
    registerProperty(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_TITLE), UNODIALOG_PROPERTY_ID_TITLE, PropertyAttribute::TRANSIENT,
        &m_sTitle, getCppuType(&m_sTitle));
    registerProperty(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_PARENT), UNODIALOG_PROPERTY_ID_PARENT, PropertyAttribute::TRANSIENT,
        &m_xParent, getCppuType(&m_xParent));
}

//-------------------------------------------------------------------------
OGenericUnoDialog::~OGenericUnoDialog()
{
    if ( m_pDialog )
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
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
Sequence<sal_Int8> SAL_CALL OGenericUnoDialog::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OGenericUnoDialog::supportsService(const ::rtl::OUString& ServiceName) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pArray = aSupported.getConstArray();
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
            m_pDialog->SetText(String(m_sTitle));
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
void SAL_CALL OGenericUnoDialog::setTitle( const ::rtl::OUString& _rTitle ) throw(RuntimeException)
{
    UnoDialogEntryGuard aGuard( *this );

    try
    {
        setPropertyValue(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_TITLE), makeAny(_rTitle));
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
    String sTitle = m_sTitle;

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
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    Dialog* pDialogToExecute = NULL;
    // create the dialog, if neccessary
    {
        UnoDialogEntryGuard aGuard( *this );

        if (m_bExecuting)
            throw RuntimeException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "already executing the dialog (recursive call)" ) ),
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
        throw AlreadyInitializedException( ::rtl::OUString(), *this );

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

