/*************************************************************************
 *
 *  $RCSfile: unodialogabp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: fs $ $Date: 2001-09-14 09:56:05 $
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

#ifndef EXTENSIONS_ABP_UNODIALOG_HXX
#include "unodialogabp.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef EXTENSIONS_ABSPILOT_HXX
#include "abspilot.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

extern "C" void SAL_CALL createRegistryInfo_OABSPilotUno()
{
    static ::abp::OMultiInstanceAutoRegistration< ::abp::OABSPilotUno > aAutoRegistration;
}

//.........................................................................
namespace abp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::ui::dialogs;

    //=====================================================================
    //= OABSPilotUno
    //=====================================================================
    //---------------------------------------------------------------------
    OABSPilotUno::OABSPilotUno(const Reference< XMultiServiceFactory >& _rxORB)
        :OGenericUnoDialog(_rxORB)
    {
    }

    //--------------------------------------------------------------------------
    Any SAL_CALL OABSPilotUno::queryInterface( const Type& aType ) throw (RuntimeException)
    {
        Any aReturn = OABSPilotUno_DBase::queryInterface( aType );
        return aReturn.hasValue() ? aReturn : OABSPilotUno_JBase::queryInterface( aType );
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OABSPilotUno::acquire(  ) throw ()
    {
        OABSPilotUno_DBase::acquire();
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OABSPilotUno::release(  ) throw ()
    {
        OABSPilotUno_DBase::release();
    }

    //---------------------------------------------------------------------
    Sequence< Type > SAL_CALL OABSPilotUno::getTypes(  ) throw (RuntimeException)
    {
        return ::comphelper::concatSequences(
            OABSPilotUno_DBase::getTypes(),
            OABSPilotUno_JBase::getTypes()
        );
    }

    //---------------------------------------------------------------------
    Sequence<sal_Int8> SAL_CALL OABSPilotUno::getImplementationId(  ) throw(RuntimeException)
    {
        static ::cppu::OImplementationId* s_pId;
        if ( !s_pId )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( !s_pId )
            {
                static ::cppu::OImplementationId s_aId;
                s_pId = &s_aId;
            }
        }
        return s_pId->getImplementationId();
    }

    //---------------------------------------------------------------------
    Reference< XInterface > SAL_CALL OABSPilotUno::Create(const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return *(new OABSPilotUno(_rxFactory));
    }

    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL OABSPilotUno::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }

    //---------------------------------------------------------------------
    ::rtl::OUString OABSPilotUno::getImplementationName_Static() throw(RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.abp.OAddressBookSourcePilot");
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence SAL_CALL OABSPilotUno::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }

    //---------------------------------------------------------------------
    ::comphelper::StringSequence OABSPilotUno::getSupportedServiceNames_Static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.ui.dialogs.AddressBookSourcePilot");
        return aSupported;
    }

    //---------------------------------------------------------------------
    Reference<XPropertySetInfo>  SAL_CALL OABSPilotUno::getPropertySetInfo() throw(RuntimeException)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }

    //---------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper& OABSPilotUno::getInfoHelper()
    {
        return *const_cast<OABSPilotUno*>(this)->getArrayHelper();
    }

    //--------------------------------------------------------------------------
    ::cppu::IPropertyArrayHelper* OABSPilotUno::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    //--------------------------------------------------------------------------
    Dialog* OABSPilotUno::createDialog(Window* _pParent)
    {
        return new OAddessBookSourcePilot(_pParent, m_xORB);
    }

    //--------------------------------------------------------------------------
    void SAL_CALL OABSPilotUno::execute( const Reference< XInterface >& xContext, const Sequence< NamedValue >& aArgs ) throw (IllegalArgumentException, RuntimeException)
    {
        // not interested in the context, not interested in the args
        // -> call the execute method of the XExecutableDialog

        static_cast< XExecutableDialog* >( this )->execute();
    }

//.........................................................................
}   // namespace abp
//.........................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.1  2001/08/01 11:10:07  fs
 *  initial checkin - address book auto pilot - uno wrapper for the pilot
 *
 *  Revision 1.1  2001/02/12 07:16:13  fs
 *  initial checkin - importing StarOffice 5.2 database files
 *
 *
 *  Revision 1.0 30.01.01 15:33:22  fs
 ************************************************************************/

