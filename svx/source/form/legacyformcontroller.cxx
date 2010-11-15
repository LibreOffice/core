/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
#include "precompiled_svx.hxx"

#include "fmservs.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/form/XFormController.hpp>
#include <com/sun/star/form/runtime/XFormController.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
/** === end UNO includes === **/

#include <cppuhelper/implbase2.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::awt::XTabControllerModel;
    using ::com::sun::star::awt::XControlContainer;
    using ::com::sun::star::lang::XServiceInfo;
    /** === end UNO using === **/

    using namespace ::com::sun::star;

    //====================================================================
    //= LegacyFormController
    //====================================================================
    typedef ::cppu::WeakImplHelper2 <   form::XFormController
                                    ,   XServiceInfo
                                    >   LegacyFormController_Base;
    /** is an implementation of the legacy form controller service, namely css.form.FormController, supporting the
        css.form.XFormController interface.

        This legacy API is superseded by css.form.runtime.(X)FormController, and though we migrated all OOo-internal
        usage of this old API, their might be clients external to OOo still using it (though this is rather unlikely).
    */
    class LegacyFormController : public LegacyFormController_Base
    {
    public:
        static Reference< XInterface > Create( const Reference< XMultiServiceFactory >& _rxFactory )
        {
            return *( new LegacyFormController( _rxFactory ) );
        }

    protected:
        LegacyFormController( const Reference< XMultiServiceFactory >& _rxFactory )
            :m_xDelegator( _rxFactory->createInstance( FM_FORM_CONTROLLER ), UNO_QUERY_THROW )
        {
        }

        // form::XFormController
        virtual Reference< XControl > SAL_CALL getCurrentControl(  ) throw (RuntimeException);
        virtual void SAL_CALL addActivateListener( const Reference< form::XFormControllerListener >& l ) throw (RuntimeException);
        virtual void SAL_CALL removeActivateListener( const Reference< form::XFormControllerListener >& l ) throw (RuntimeException);

        // awt::XTabController
        virtual void SAL_CALL setModel( const Reference< XTabControllerModel >& Model ) throw (RuntimeException);
        virtual Reference< XTabControllerModel > SAL_CALL getModel(  ) throw (RuntimeException);
        virtual void SAL_CALL setContainer( const Reference< XControlContainer >& Container ) throw (RuntimeException);
        virtual Reference< XControlContainer > SAL_CALL getContainer(  ) throw (RuntimeException);
        virtual Sequence< Reference< XControl > > SAL_CALL getControls(  ) throw (RuntimeException);
        virtual void SAL_CALL autoTabOrder(  ) throw (RuntimeException);
        virtual void SAL_CALL activateTabOrder(  ) throw (RuntimeException);
        virtual void SAL_CALL activateFirst(  ) throw (RuntimeException);
        virtual void SAL_CALL activateLast(  ) throw (RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (RuntimeException);
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (RuntimeException);
        virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (RuntimeException);

    private:
        const Reference< form::runtime::XFormController >   m_xDelegator;
    };

    //--------------------------------------------------------------------
    Reference< XControl > SAL_CALL LegacyFormController::getCurrentControl(  ) throw (RuntimeException)
    {
        return m_xDelegator->getCurrentControl();
    }

    //--------------------------------------------------------------------
    void SAL_CALL LegacyFormController::addActivateListener( const Reference< form::XFormControllerListener >& _listener ) throw (RuntimeException)
    {
        m_xDelegator->addActivateListener( _listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL LegacyFormController::removeActivateListener( const Reference< form::XFormControllerListener >& _listener ) throw (RuntimeException)
    {
        m_xDelegator->removeActivateListener( _listener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL LegacyFormController::setModel( const Reference< XTabControllerModel >& _model ) throw (RuntimeException)
    {
        m_xDelegator->setModel( _model );
    }

    //--------------------------------------------------------------------
    Reference< XTabControllerModel > SAL_CALL LegacyFormController::getModel(  ) throw (RuntimeException)
    {
        return m_xDelegator->getModel();
    }

    //--------------------------------------------------------------------
    void SAL_CALL LegacyFormController::setContainer( const Reference< XControlContainer >& _container ) throw (RuntimeException)
    {
        m_xDelegator->setContainer( _container );
    }

    //--------------------------------------------------------------------
    Reference< XControlContainer > SAL_CALL LegacyFormController::getContainer(  ) throw (RuntimeException)
    {
        return m_xDelegator->getContainer();
    }

    //--------------------------------------------------------------------
    Sequence< Reference< XControl > > SAL_CALL LegacyFormController::getControls(  ) throw (RuntimeException)
    {
        return m_xDelegator->getControls();
    }

    //--------------------------------------------------------------------
    void SAL_CALL LegacyFormController::autoTabOrder(  ) throw (RuntimeException)
    {
        m_xDelegator->autoTabOrder();
    }

    //--------------------------------------------------------------------
    void SAL_CALL LegacyFormController::activateTabOrder(  ) throw (RuntimeException)
    {
        m_xDelegator->activateTabOrder();
    }

    //--------------------------------------------------------------------
    void SAL_CALL LegacyFormController::activateFirst(  ) throw (RuntimeException)
    {
        m_xDelegator->activateFirst();
    }

    //--------------------------------------------------------------------
    void SAL_CALL LegacyFormController::activateLast(  ) throw (RuntimeException)
    {
        m_xDelegator->activateLast();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL LegacyFormController::getImplementationName(  ) throw (RuntimeException)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.comp.svx.LegacyFormController" ) );
    }

    //--------------------------------------------------------------------
    ::sal_Bool SAL_CALL LegacyFormController::supportsService( const ::rtl::OUString& _serviceName ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices( getSupportedServiceNames() );
        const ::rtl::OUString* pServices = aServices.getConstArray();
        for ( sal_Int32 i = 0; i < aServices.getLength(); ++i, ++pServices )
            if( pServices->equals( _serviceName ) )
                return sal_True;
        return sal_False;
    }

    //--------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL LegacyFormController::getSupportedServiceNames(  ) throw (RuntimeException)
    {
        Sequence< ::rtl::OUString > aServices(2);
        aServices.getArray()[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.form.FormController" ) );
        aServices.getArray()[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.control.TabController") );
        return aServices;
    }

//........................................................................
} // namespace svxform
//........................................................................

//------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    LegacyFormController_NewInstance_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & _rxORB )
{
    return ::svxform::LegacyFormController::Create( _rxORB );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
