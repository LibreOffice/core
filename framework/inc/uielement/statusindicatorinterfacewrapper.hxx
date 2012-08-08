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

#ifndef __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>

#include <vector>

namespace framework
{

class StatusIndicatorInterfaceWrapper :   public ::cppu::WeakImplHelper1< ::com::sun::star::task::XStatusIndicator>
{
    public:
        StatusIndicatorInterfaceWrapper( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rStatusIndicatorImpl );
        virtual ~StatusIndicatorInterfaceWrapper();

        //---------------------------------------------------------------------------------------------------------
        //  XStatusIndicator
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL start   ( const ::rtl::OUString& sText  ,
                                              sal_Int32        nRange ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL end     (                               ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL reset   (                               ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL setText ( const ::rtl::OUString& sText  ) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL setValue(       sal_Int32        nValue ) throw( ::com::sun::star::uno::RuntimeException );

    private:
        StatusIndicatorInterfaceWrapper();

        ::com::sun::star::uno::WeakReference< ::com::sun::star::lang::XComponent > m_xStatusIndicatorImpl;
};

}

#endif // __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
