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

#ifndef __FRAMEWORK_UIELEMENT_ADDONSTOOLBARWRAPPER_HXX_
#define __FRAMEWORK_UIELEMENT_ADDONSTOOLBARWRAPPER_HXX_

#include <helper/uielementwrapperbase.hxx>

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace framework
{

class AddonsToolBarManager;
class AddonsToolBarWrapper : public UIElementWrapperBase
{
    public:
        AddonsToolBarWrapper( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~AddonsToolBarWrapper();

        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException);

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------
    private:
        com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >                             m_xServiceManager;
        com::sun::star::uno::Reference< com::sun::star::lang::XComponent >                                       m_xToolBarManager;
        com::sun::star::uno::Reference< com::sun::star::awt::XWindow >                                           m_xToolBarWindow;
        com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > > m_aConfigData;
};

}

#endif // __FRAMEWORK_UIELEMENT_ADDONSTOOLBARWRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
