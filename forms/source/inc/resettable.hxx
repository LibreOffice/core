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

#ifndef FORMS_RESETTABLE_HXX
#define FORMS_RESETTABLE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/form/XResetListener.hpp>
/** === end UNO includes === **/

#include <cppuhelper/interfacecontainer.hxx>

namespace cppu
{
    class OWeakObject;
}

//........................................................................
namespace frm
{
//........................................................................

    //====================================================================
    //= ResetHelper
    //====================================================================
    class ResetHelper
    {
    public:
        ResetHelper( ::cppu::OWeakObject& _parent, ::osl::Mutex& _mutex )
            :m_rParent( _parent )
            ,m_aResetListeners( _mutex )
        {
        }

        // XReset equivalents
        void addResetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& _listener );
        void removeResetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& _listener );

        // calling listeners
        bool approveReset();
        void notifyResetted();
        void disposing();

    private:
        ::cppu::OWeakObject&                m_rParent;
        ::cppu::OInterfaceContainerHelper   m_aResetListeners;
    };

//........................................................................
} // namespace frm
//........................................................................

#endif // FORMS_RESETTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
