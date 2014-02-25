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

#include "unodialogabp.hxx"
#include <cppuhelper/typeprovider.hxx>
#include "abspilot.hxx"
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/msgbox.hxx>

extern "C" void SAL_CALL createRegistryInfo_OABSPilotUno()
{
    static ::abp::OMultiInstanceAutoRegistration< ::abp::OABSPilotUno > aAutoRegistration;
}

#define PROPERTY_ID_DATASOURCENAME  3
//.........................................................................
namespace abp
{
//.........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::ui::dialogs;

    //=====================================================================
    //= OABSPilotUno
    //=====================================================================

    OABSPilotUno::OABSPilotUno(const Reference< XComponentContext >& _rxORB)
        :OGenericUnoDialog(_rxORB)
    {
        registerProperty( OUString("DataSourceName"), PROPERTY_ID_DATASOURCENAME, PropertyAttribute::READONLY ,
            &m_sDataSourceName, ::getCppuType( &m_sDataSourceName ) );
    }


    Any SAL_CALL OABSPilotUno::queryInterface( const Type& aType ) throw (RuntimeException, std::exception)
    {
        Any aReturn = OABSPilotUno_DBase::queryInterface( aType );
        return aReturn.hasValue() ? aReturn : OABSPilotUno_JBase::queryInterface( aType );
    }


    void SAL_CALL OABSPilotUno::acquire(  ) throw ()
    {
        OABSPilotUno_DBase::acquire();
    }


    void SAL_CALL OABSPilotUno::release(  ) throw ()
    {
        OABSPilotUno_DBase::release();
    }


    Sequence< Type > SAL_CALL OABSPilotUno::getTypes(  ) throw (RuntimeException, std::exception)
    {
        return ::comphelper::concatSequences(
            OABSPilotUno_DBase::getTypes(),
            OABSPilotUno_JBase::getTypes()
        );
    }


    Sequence<sal_Int8> SAL_CALL OABSPilotUno::getImplementationId(  ) throw(RuntimeException, std::exception)
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


    Reference< XInterface > SAL_CALL OABSPilotUno::Create(const Reference< XMultiServiceFactory >& _rxFactory)
    {
        return *(new OABSPilotUno( comphelper::getComponentContext(_rxFactory) ));
    }


    OUString SAL_CALL OABSPilotUno::getImplementationName() throw(RuntimeException, std::exception)
    {
        return getImplementationName_Static();
    }


    OUString OABSPilotUno::getImplementationName_Static() throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.abp.OAddressBookSourcePilot");
    }


    ::comphelper::StringSequence SAL_CALL OABSPilotUno::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_Static();
    }


    ::comphelper::StringSequence OABSPilotUno::getSupportedServiceNames_Static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(1);
        aSupported[0] = "com.sun.star.ui.dialogs.AddressBookSourcePilot";
        return aSupported;
    }


    Reference<XPropertySetInfo>  SAL_CALL OABSPilotUno::getPropertySetInfo() throw(RuntimeException, std::exception)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    ::cppu::IPropertyArrayHelper& OABSPilotUno::getInfoHelper()
    {
        return *const_cast<OABSPilotUno*>(this)->getArrayHelper();
    }


    ::cppu::IPropertyArrayHelper* OABSPilotUno::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
    }

    void SAL_CALL OABSPilotUno::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException, std::exception)
    {
        Reference<awt::XWindow> xParentWindow;
        if (aArguments.getLength() == 1 && (aArguments[0] >>= xParentWindow) ) {
            Sequence< Any > aNewArgs(1);
            aNewArgs[0] <<= PropertyValue( OUString("ParentWindow"), 0, makeAny(xParentWindow), PropertyState_DIRECT_VALUE );
            OGenericUnoDialog::initialize(aNewArgs);
        } else {
            OGenericUnoDialog::initialize(aArguments);
        }
    }


    Dialog* OABSPilotUno::createDialog(Window* _pParent)
    {
        return new OAddessBookSourcePilot(_pParent, m_aContext );
    }


    Any SAL_CALL OABSPilotUno::execute( const Sequence< NamedValue >& /*lArgs*/ ) throw (IllegalArgumentException, Exception, RuntimeException, std::exception)
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
        lProtocol[0].Name    = "Deactivate";
        lProtocol[0].Value <<= sal_True;
        return makeAny( lProtocol );
    }

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
