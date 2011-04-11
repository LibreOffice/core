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

#ifndef __FRAMEWORK_HELPER_DOCKINGAREADEFAULTACCEPTOR_HXX_
#define __FRAMEWORK_HELPER_DOCKINGAREADEFAULTACCEPTOR_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <classes/framecontainer.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/ui/XDockingAreaAcceptor.hpp>
#include <com/sun/star/frame/XFrame.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakref.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________


class DockingAreaDefaultAcceptor    :   private ThreadHelpBase                                      ,
                                        public ::cppu::WeakImplHelper1< ::com::sun::star::ui::XDockingAreaAcceptor >
{
    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      constructor to initialize this instance
            @descr      A docking area acceptor
                        But we need a instance to create more then one enumerations to the same tasklist!

            @seealso    class Desktop
            @seealso    class OTasksEnumeration

            @param      "xOwner" is a reference to ouer owner and must be the desktop!
            @param      "pTasks" is a pointer to the taskcontainer of the desktop. We need it to create a new enumeration.
            @return     -

            @onerror    Do nothing and reset this object to default with an empty list.
        *//*-*****************************************************************************************************/

         DockingAreaDefaultAcceptor(    const css::uno::Reference< css::frame::XFrame >& xOwner );
        virtual ~DockingAreaDefaultAcceptor();

        //---------------------------------------------------------------------------------------------------------
        //  XDockingAreaAcceptor
        //---------------------------------------------------------------------------------------------------------

        virtual css::uno::Reference< css::awt::XWindow > SAL_CALL getContainerWindow() throw (css::uno::RuntimeException);
        virtual sal_Bool SAL_CALL requestDockingAreaSpace( const css::awt::Rectangle& RequestedSpace ) throw (css::uno::RuntimeException);
        virtual void SAL_CALL setDockingAreaSpace( const css::awt::Rectangle& BorderSpace ) throw (css::uno::RuntimeException);


    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        css::uno::WeakReference< css::frame::XFrame >       m_xOwner            ;   /// weak reference to our frame object!
};

} // namespace framework

#endif // __FRAMEWORK_HELPER_DOCKINGAREADEFAULTACCEPTOR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
