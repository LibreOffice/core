/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unodialogabp.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:39:56 $
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
#include "precompiled_extensions.hxx"

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
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

extern "C" void SAL_CALL createRegistryInfo_OABSPilotUno()
{
    static ::abp::OMultiInstanceAutoRegistration< ::abp::OABSPilotUno > aAutoRegistration;
}

#define PROPERTY_ID_DATASOURCENAME  3
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
        registerProperty( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataSourceName")), PROPERTY_ID_DATASOURCENAME, PropertyAttribute::READONLY ,
            &m_sDataSourceName, ::getCppuType( &m_sDataSourceName ) );
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
    Any SAL_CALL OABSPilotUno::execute( const Sequence< NamedValue >& /*lArgs*/ ) throw (IllegalArgumentException, Exception, RuntimeException)
    {
        // not interested in the context, not interested in the args
        // -> call the execute method of the XExecutableDialog
        static_cast< XExecutableDialog* >( this )->execute();

        // result interest not really ...
        // We show this dialog one times only!
        // User has one chance to accept it or not.
        // (or he can start it again by using wizard-menu!)
        // So we should deregister it on our general job execution service by using right protocol parameters.
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > lProtocol(1);
        lProtocol[0].Name    = ::rtl::OUString::createFromAscii("Deactivate");
        lProtocol[0].Value <<= sal_True;
        return makeAny( lProtocol );
    }
    // -----------------------------------------------------------------------------
    void OABSPilotUno::executedDialog(sal_Int16 _nExecutionResult)
    {
        if ( _nExecutionResult == RET_OK )
        {
            const AddressSettings& aSettings = static_cast<OAddessBookSourcePilot*>(m_pDialog)->getSettings();
            m_sDataSourceName = aSettings.bRegisterDataSource ? aSettings.sRegisteredDataSourceName : aSettings.sDataSourceName;
        }
    }

//.........................................................................
}   // namespace abp
//.........................................................................

