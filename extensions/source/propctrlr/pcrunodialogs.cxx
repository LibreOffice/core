/*************************************************************************
 *
 *  $RCSfile: pcrunodialogs.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:09:48 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PCRUNODIALOGS_HXX
#include "pcrunodialogs.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_PCRSTRINGS_HXX_
#include "pcrstrings.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_TABORDER_HXX
#include "taborder.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#include "pcrcommon.hxx"
#endif

extern "C" void SAL_CALL createRegistryInfo_OTabOrderDialog()
{
    static ::pcr::OMultiInstanceAutoRegistration< ::pcr::OTabOrderDialog > aAutoRegistration;
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
    OTabOrderDialog::OTabOrderDialog( const Reference< XMultiServiceFactory >& _rxORB )
        :OGenericUnoDialog( _rxORB )
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
    Reference< XInterface > SAL_CALL OTabOrderDialog::Create( const Reference< XMultiServiceFactory >& _rxFactory )
    {
        return *( new OTabOrderDialog( _rxFactory ) );
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OTabOrderDialog::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OTabOrderDialog::getImplementationName_Static() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii( "org.openoffice.comp.form.ui.OTabOrderDialog" );
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL OTabOrderDialog::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence OTabOrderDialog::getSupportedServiceNames_Static() throw(RuntimeException)
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

