/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pcrunodialogs.cxx,v $
 * $Revision: 1.7 $
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
#include "precompiled_extensions.hxx"
#include "pcrunodialogs.hxx"
#include "formstrings.hxx"
#include "pcrstrings.hxx"
#include "taborder.hxx"
#include "pcrcommon.hxx"

extern "C" void SAL_CALL createRegistryInfo_OTabOrderDialog()
{
    ::pcr::OAutoRegistration< ::pcr::OTabOrderDialog > aAutoRegistration;
}

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;

    //====================================================================
    //= OTabOrderDialog
    //====================================================================
    //---------------------------------------------------------------------
    OTabOrderDialog::OTabOrderDialog( const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
    {
        registerProperty( PROPERTY_CONTROLCONTEXT, OWN_PROPERTY_ID_CONTROLCONTEXT,
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xControlContext, ::getCppuType( &m_xControlContext ) );

        registerProperty( PROPERTY_TABBINGMODEL, OWN_PROPERTY_ID_TABBINGMODEL,
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xTabbingModel, ::getCppuType( &m_xTabbingModel ) );
    }

    //---------------------------------------------------------------------
    OTabOrderDialog::~OTabOrderDialog()
    {
        if ( m_pDialog )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pDialog )
                destroyDialog();
        }
    }

    //---------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OTabOrderDialog::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId aId;
        return aId.getImplementationId();
    }

    //---------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OTabOrderDialog::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new OTabOrderDialog( _rxContext ) );
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OTabOrderDialog::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_static();
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OTabOrderDialog::getImplementationName_static() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( "org.openoffice.comp.form.ui.OTabOrderDialog" );
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL OTabOrderDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_static();
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence OTabOrderDialog::getSupportedServiceNames_static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.ui.TabOrderDialog" ) );
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OTabOrderDialog::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //---------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OTabOrderDialog::getInfoHelper()
    {
        return *const_cast< OTabOrderDialog* >( this )->getArrayHelper();
    }

    //--------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OTabOrderDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }

    //--------------------------------------------------------------------------
    Dialog* OTabOrderDialog::createDialog( Window* _pParent )
    {
        return new TabOrderDialog( _pParent, m_xTabbingModel, m_xControlContext, m_xORB );
    }

//........................................................................
}   // namespace pcr
//........................................................................

