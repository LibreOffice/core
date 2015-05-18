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

#include <sal/config.h>

#include <com/sun/star/beans/NamedValue.hpp>

#include "pcrservices.hxx"
#include "pcrunodialogs.hxx"
#include "formstrings.hxx"
#include "pcrstrings.hxx"
#include "taborder.hxx"
#include "pcrcommon.hxx"

extern "C" void SAL_CALL createRegistryInfo_OTabOrderDialog()
{
    ::pcr::OAutoRegistration< ::pcr::OTabOrderDialog > aAutoRegistration;
}


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;


    //= OTabOrderDialog


    OTabOrderDialog::OTabOrderDialog( const Reference< XComponentContext >& _rxContext )
        :OGenericUnoDialog( _rxContext )
    {
        registerProperty( PROPERTY_CONTROLCONTEXT, OWN_PROPERTY_ID_CONTROLCONTEXT,
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xControlContext, cppu::UnoType<decltype(m_xControlContext)>::get() );

        registerProperty( PROPERTY_TABBINGMODEL, OWN_PROPERTY_ID_TABBINGMODEL,
            PropertyAttribute::BOUND | PropertyAttribute::TRANSIENT,
            &m_xTabbingModel, cppu::UnoType<decltype(m_xTabbingModel)>::get() );
    }


    OTabOrderDialog::~OTabOrderDialog()
    {
        if ( m_pDialog )
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if ( m_pDialog )
                destroyDialog();
        }
    }


    Sequence<sal_Int8> SAL_CALL OTabOrderDialog::getImplementationId(  ) throw(RuntimeException, std::exception)
    {
        return css::uno::Sequence<sal_Int8>();
    }


    Reference< XInterface > SAL_CALL OTabOrderDialog::Create( const Reference< XComponentContext >& _rxContext )
    {
        return *( new OTabOrderDialog( _rxContext ) );
    }


    OUString SAL_CALL OTabOrderDialog::getImplementationName() throw(RuntimeException, std::exception)
    {
        return getImplementationName_static();
    }


    OUString OTabOrderDialog::getImplementationName_static() throw(RuntimeException)
    {
        return OUString("org.openoffice.comp.form.ui.OTabOrderDialog");
    }


    ::comphelper::StringSequence SAL_CALL OTabOrderDialog::getSupportedServiceNames() throw(RuntimeException, std::exception)
    {
        return getSupportedServiceNames_static();
    }


    ::comphelper::StringSequence OTabOrderDialog::getSupportedServiceNames_static() throw(RuntimeException)
    {
        ::comphelper::StringSequence aSupported(2);
        aSupported.getArray()[0] = "com.sun.star.form.ui.TabOrderDialog";
        aSupported.getArray()[1] = "com.sun.star.form.TabOrderDialog";
        return aSupported;
    }


    Reference<XPropertySetInfo>  SAL_CALL OTabOrderDialog::getPropertySetInfo() throw(RuntimeException, std::exception)
    {
        Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
        return xInfo;
    }


    ::cppu::IPropertyArrayHelper& OTabOrderDialog::getInfoHelper()
    {
        return *getArrayHelper();
    }


    ::cppu::IPropertyArrayHelper* OTabOrderDialog::createArrayHelper( ) const
    {
        Sequence< Property > aProps;
        describeProperties( aProps );
        return new ::cppu::OPropertyArrayHelper( aProps );
    }


    VclPtr<Dialog> OTabOrderDialog::createDialog( vcl::Window* _pParent )
    {
        return VclPtr<TabOrderDialog>::Create( _pParent, m_xTabbingModel, m_xControlContext, m_aContext );
    }

    void OTabOrderDialog::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException, std::exception)
    {
        Reference<css::awt::XTabControllerModel> xTabbingModel;
        Reference<css::awt::XControlContainer> xControlContext;
        Reference<css::awt::XWindow> xParentWindow;
        if (aArguments.getLength() == 3 && (aArguments[0] >>= xTabbingModel) && (aArguments[1] >>= xControlContext) && (aArguments[2] >>= xParentWindow))
        {
            Sequence< Any > aNewArguments( 3 );
            aNewArguments[0] <<= NamedValue(
                OUString( "TabbingModel" ),
                makeAny( xTabbingModel )
            );
            aNewArguments[1] <<= NamedValue(
                OUString( "ControlContext" ),
                makeAny( xControlContext )
            );
            aNewArguments[2] <<= NamedValue(
                OUString( "ParentWindow"  ),
                makeAny( xParentWindow )
            );
            OTabOrderDialog_DBase::initialize(aNewArguments);
        }
        else
            OTabOrderDialog_DBase::initialize(aArguments);
    }




}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
