/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_
#define __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/XComponent.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
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
