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

#ifndef SVTOOLS_INITGUARD_HXX
#define SVTOOLS_INITGUARD_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/NotInitializedException.hpp>
/** === end UNO includes === **/

#include <comphelper/componentguard.hxx>

//......................................................................................................................
namespace toolkit
{
//......................................................................................................................

    //==================================================================================================================
    //= InitGuard
    //==================================================================================================================
    template < class IMPL >
    class InitGuard : public ::comphelper::ComponentGuard
    {
    public:
        InitGuard( IMPL& i_component, ::cppu::OBroadcastHelper & i_broadcastHelper )
            :comphelper::ComponentGuard( i_component, i_broadcastHelper )
        {
            if ( !i_component.isInitialized() )
                throw ::com::sun::star::lang::NotInitializedException( ::rtl::OUString(), *&i_component );
        }

        ~InitGuard()
        {
        }
    };

//......................................................................................................................
} // namespace toolkit
//......................................................................................................................

#endif // SVTOOLS_INITGUARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */