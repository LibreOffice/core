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


#include <fmservs.hxx>

#include <com/sun/star/form/XFormController.hpp>
#include <com/sun/star/form/runtime/FormController.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/processfactory.hxx>


namespace svxform
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::awt::XControl;
    using ::com::sun::star::awt::XTabControllerModel;
    using ::com::sun::star::awt::XControlContainer;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::form::runtime::FormController;

    using namespace ::com::sun::star;


    //= LegacyFormController

    typedef ::cppu::WeakImplHelper <   form::XFormController
                                    ,   XServiceInfo
                                    >   LegacyFormController_Base;

    namespace {

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
            return *( new LegacyFormController( comphelper::getComponentContext(_rxFactory) ) );
        }

    protected:
        explicit LegacyFormController( const Reference< XComponentContext >& _rxContext )
            :m_xDelegator( FormController::create(_rxContext) )
        {
        }

        // form::XFormController
        virtual Reference< XControl > SAL_CALL getCurrentControl(  ) override;
        virtual void SAL_CALL addActivateListener( const Reference< form::XFormControllerListener >& l ) override;
        virtual void SAL_CALL removeActivateListener( const Reference< form::XFormControllerListener >& l ) override;

        // awt::XTabController
        virtual void SAL_CALL setModel( const Reference< XTabControllerModel >& Model ) override;
        virtual Reference< XTabControllerModel > SAL_CALL getModel(  ) override;
        virtual void SAL_CALL setContainer( const Reference< XControlContainer >& Container ) override;
        virtual Reference< XControlContainer > SAL_CALL getContainer(  ) override;
        virtual Sequence< Reference< XControl > > SAL_CALL getControls(  ) override;
        virtual void SAL_CALL autoTabOrder(  ) override;
        virtual void SAL_CALL activateTabOrder(  ) override;
        virtual void SAL_CALL activateFirst(  ) override;
        virtual void SAL_CALL activateLast(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName(  ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    private:
        const Reference< form::runtime::XFormController >   m_xDelegator;
    };

    }

    Reference< XControl > SAL_CALL LegacyFormController::getCurrentControl(  )
    {
        return m_xDelegator->getCurrentControl();
    }


    void SAL_CALL LegacyFormController::addActivateListener( const Reference< form::XFormControllerListener >& _listener )
    {
        m_xDelegator->addActivateListener( _listener );
    }


    void SAL_CALL LegacyFormController::removeActivateListener( const Reference< form::XFormControllerListener >& _listener )
    {
        m_xDelegator->removeActivateListener( _listener );
    }


    void SAL_CALL LegacyFormController::setModel( const Reference< XTabControllerModel >& _model )
    {
        m_xDelegator->setModel( _model );
    }


    Reference< XTabControllerModel > SAL_CALL LegacyFormController::getModel(  )
    {
        return m_xDelegator->getModel();
    }


    void SAL_CALL LegacyFormController::setContainer( const Reference< XControlContainer >& _container )
    {
        m_xDelegator->setContainer( _container );
    }


    Reference< XControlContainer > SAL_CALL LegacyFormController::getContainer(  )
    {
        return m_xDelegator->getContainer();
    }


    Sequence< Reference< XControl > > SAL_CALL LegacyFormController::getControls(  )
    {
        return m_xDelegator->getControls();
    }


    void SAL_CALL LegacyFormController::autoTabOrder(  )
    {
        m_xDelegator->autoTabOrder();
    }


    void SAL_CALL LegacyFormController::activateTabOrder(  )
    {
        m_xDelegator->activateTabOrder();
    }


    void SAL_CALL LegacyFormController::activateFirst(  )
    {
        m_xDelegator->activateFirst();
    }


    void SAL_CALL LegacyFormController::activateLast(  )
    {
        m_xDelegator->activateLast();
    }


    OUString SAL_CALL LegacyFormController::getImplementationName(  )
    {
        return u"org.openoffice.comp.svx.LegacyFormController"_ustr;
    }

    sal_Bool SAL_CALL LegacyFormController::supportsService( const OUString& _serviceName )
    {
        return cppu::supportsService(this, _serviceName);
    }

    Sequence< OUString > SAL_CALL LegacyFormController::getSupportedServiceNames(  )
    {
        return { u"com.sun.star.form.FormController"_ustr, u"com.sun.star.awt.control.TabController"_ustr };
    }

}

css::uno::Reference< css::uno::XInterface >
    LegacyFormController_NewInstance_Impl( const css::uno::Reference< css::lang::XMultiServiceFactory > & _rxORB )
{
    return ::svxform::LegacyFormController::Create( _rxORB );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
