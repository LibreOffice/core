/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: genericunodialog.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:23:00 $
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
#include "precompiled_svtools.hxx"

#ifndef _SVT_GENERICUNODIALOG_HXX_
#include "genericunodialog.hxx"
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

// --- needed because of the solar mutex
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
// ---

#define THISREF()   static_cast< XServiceInfo* >(this)

using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

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
        ,m_xORB(_rxORB)
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
        ,m_xORB( _rxContext->getServiceManager(), UNO_QUERY_THROW )
        ,m_xContext(_rxContext)
{
    registerProperty(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_TITLE), UNODIALOG_PROPERTY_ID_TITLE, PropertyAttribute::TRANSIENT,
        &m_sTitle, getCppuType(&m_sTitle));
    registerProperty(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_PARENT), UNODIALOG_PROPERTY_ID_PARENT, PropertyAttribute::TRANSIENT,
        &m_xParent, getCppuType(&m_xParent));
}

//-------------------------------------------------------------------------
OGenericUnoDialog::~OGenericUnoDialog()
{
    if (m_pDialog)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (m_pDialog)
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
    Sequence<Type> aTypes = OGenericUnoDialogBase::getTypes();

    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 3);
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast<Reference<XPropertySet>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast<Reference<XFastPropertySet>*>(NULL));
    aTypes.getArray()[nLen++] = ::getCppuType(static_cast<Reference<XMultiPropertySet>*>(NULL));

    return aTypes;
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

    // TODO : need to be a dispose listener on the interface ...
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
    try
    {
        setPropertyValue(::rtl::OUString::createFromAscii(UNODIALOG_PROPERTY_TITLE), makeAny(_rTitle));
    }
    catch(RuntimeException&)
    {
        // allowed to pass
        throw;
    }
    catch(Exception&)
    {
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
        ::osl::MutexGuard aGuard(m_aMutex);

        if (m_bExecuting)
            throw RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("already executing the dialog (recursive call)")), THISREF());

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
    ::osl::MutexGuard aGuard(m_aMutex);
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
    ::osl::MutexGuard aGuard(m_aMutex);
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
    const Any* pArguments = aArguments.getConstArray();
    for (sal_Int32 i=0; i<aArguments.getLength(); ++i, ++pArguments)
        implInitialize(*pArguments);
}

//-------------------------------------------------------------------------
void OGenericUnoDialog::destroyDialog()
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
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

