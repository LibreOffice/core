/*************************************************************************
 *
 *  $RCSfile: genericunodialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-22 15:51:13 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
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
        break;
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
sal_Int16 SAL_CALL OGenericUnoDialog::execute(  ) throw(RuntimeException)
{
    Dialog* pDialogToExecute = NULL;
    // create the dialog, if neccessary
    {
        ::osl::ClearableMutexGuard aGuard(m_aMutex);

        if (m_bExecuting)
            throw RuntimeException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("already executing the dialog (recursive call)")), THISREF());

        m_bCanceled = sal_False;
        m_bExecuting = sal_True;

        if (!m_pDialog)
        {
            // get the parameters for the dialog from the current settings

            // the parent window
            Window* pParent = NULL;
            VCLXWindow* pImplementation = VCLXWindow::GetImplementation(m_xParent);
            if (pImplementation)
                pParent = pImplementation->GetWindow();

            // the title
            String sTitle = m_sTitle;

            // create the dialog (must be guarded by the solar mutex
            aGuard.clear();
                // this may prevent some deadlocks, as the solar mutex is usually locked very
                // high on the stack, with no mutex locked above, so we try to behave the same way here

            {
                ::vos::OGuard aGuard(Application::GetSolarMutex());
                pDialogToExecute = createDialog(pParent);
                // do some initialisations
                if (!m_bTitleAmbiguous)
                    pDialogToExecute->SetText(sTitle);
            }

            m_pDialog = pDialogToExecute;
                // note that this is not guarded by m_aMutex anymore !
        }
    }

    // start execution
    sal_Int16 nReturn(0);
    {   // again: this has to be guarded with the solar mutex
        ::vos::OGuard aGuard(Application::GetSolarMutex());
        nReturn = pDialogToExecute->Execute();
    }
    {
        ::osl::MutexGuard aExecutionGuard(m_aExecutionMutex);
        if (m_bCanceled)
            nReturn = RET_CANCEL;
    }

    ::osl::MutexGuard aGuard(m_aMutex);

    // get the settings of the dialog
    executedDialog(nReturn);

    m_bExecuting = sal_False;

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
    PropertyValue aProperty;
    if (_rValue >>= aProperty)
    {
        try
        {
            setPropertyValue(aProperty.Name, aProperty.Value);
        }
        catch(Exception&) { }
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

//.........................................................................
}   // namespace svt
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2000/11/24 12:34:35  fs
 *  initial checkin - frame for dialogs usuable as UNO service
 *
 *  Revision 1.4  2000/11/01 16:34:16  fs
 *  migrated from awt::XDialog to ui::XExecutableDialog / removed the star* namespace shortcuts
 *
 *  Revision 1.3  2000/10/31 08:08:38  fs
 *  split up initialize - now using implInitialize, thus enabling derived classes to handle single parameters only
 *
 *  Revision 1.2  2000/10/27 08:08:47  fs
 *  don't include stringconstants.hrc directly anymore
 *
 *  Revision 1.1  2000/10/25 12:56:19  fs
 *  moved herein from ..\dlg
 *
 *  Revision 1.1  2000/10/05 10:06:09  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 21.09.00 09:10:05  fs
 ************************************************************************/

