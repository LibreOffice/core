/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unosqlmessage.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:31:29 $
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
#include "precompiled_dbaccess.hxx"

#ifndef _DBAUI_SQLMESSAGE_HXX_
#include "sqlmessage.hxx"
#endif
#ifndef _DBAUI_UNOSQLMESSAGE_HXX_
#include "unosqlmessage.hxx"
#endif
#ifndef _DBU_REGHELPER_HXX_
#include "dbu_reghelper.hxx"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

using namespace dbaui;
using namespace dbtools;

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;

extern "C" void SAL_CALL createRegistryInfo_OSQLMessageDialog()
{
    static OMultiInstanceAutoRegistration< OSQLMessageDialog > aAutoRegistration;
}

//.........................................................................
namespace dbaui
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

//=========================================================================
//-------------------------------------------------------------------------
OSQLMessageDialog::OSQLMessageDialog(const Reference< XMultiServiceFactory >& _rxORB)
    :OSQLMessageDialogBase(_rxORB)
{
    registerMayBeVoidProperty(PROPERTY_SQLEXCEPTION, PROPERTY_ID_SQLEXCEPTION, PropertyAttribute::TRANSIENT | PropertyAttribute::MAYBEVOID,
        &m_aException, ::getCppuType(static_cast<SQLException*>(NULL)));
    registerProperty( PROPERTY_HELP_URL, PROPERTY_ID_HELP_URL, PropertyAttribute::TRANSIENT,
        &m_sHelpURL, ::getCppuType( &m_sHelpURL ) );
}

//-------------------------------------------------------------------------
Sequence<sal_Int8> SAL_CALL OSQLMessageDialog::getImplementationId(  ) throw(RuntimeException)
{
    static ::cppu::OImplementationId aId;
    return aId.getImplementationId();
}

//-------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OSQLMessageDialog::Create(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return *(new OSQLMessageDialog(_rxFactory));
}

//-------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSQLMessageDialog::getImplementationName() throw(RuntimeException)
{
    return getImplementationName_Static();
}

//-------------------------------------------------------------------------
::rtl::OUString OSQLMessageDialog::getImplementationName_Static() throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbu.OSQLMessageDialog");
}

//-------------------------------------------------------------------------
::comphelper::StringSequence SAL_CALL OSQLMessageDialog::getSupportedServiceNames() throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//-------------------------------------------------------------------------
::comphelper::StringSequence OSQLMessageDialog::getSupportedServiceNames_Static() throw(RuntimeException)
{
    ::comphelper::StringSequence aSupported(1);
    aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdb.ErrorMessageDialog");
    return aSupported;
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OSQLMessageDialog::convertFastPropertyValue( Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue) throw(IllegalArgumentException)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_SQLEXCEPTION:
        {
            SQLExceptionInfo aInfo(_rValue);
            if (!aInfo.isValid())
                throw IllegalArgumentException();

            _rOldValue = m_aException;
            _rConvertedValue = aInfo.get();

            return sal_True;
                // always assume "modified", don't bother with with comparing the two values
        }
        default:
            return OSQLMessageDialogBase::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
    }
}

//-------------------------------------------------------------------------
Reference<XPropertySetInfo>  SAL_CALL OSQLMessageDialog::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OSQLMessageDialog::getInfoHelper()
{
    return *const_cast<OSQLMessageDialog*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OSQLMessageDialog::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

//------------------------------------------------------------------------------
Dialog* OSQLMessageDialog::createDialog(Window* _pParent)
{
    if ( m_aException.hasValue() )
        return new OSQLMessageBox( _pParent, SQLExceptionInfo( m_aException ), WB_OK | WB_DEF_OK, m_sHelpURL );

    OSL_ENSURE(sal_False, "OSQLMessageDialog::createDialog : You should use the SQLException property to specify the error to display!");
    return new OSQLMessageBox(_pParent, SQLException());
}

//.........................................................................
}   // namespace dbaui
//.........................................................................

