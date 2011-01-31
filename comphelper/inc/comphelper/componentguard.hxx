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

#ifndef COMPHELPER_COMPONENTGUARD_HXX
#define COMPHELPER_COMPONENTGUARD_HXX

/** === begin UNO includes === **/
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <cppuhelper/weak.hxx>
#include <cppuhelper/interfacecontainer.hxx>

//......................................................................................................................
namespace comphelper
{
//......................................................................................................................

    //==================================================================================================================
    //= ComponentGuard
    //==================================================================================================================
    class ComponentGuard
    {
    public:
        ComponentGuard( ::cppu::OWeakObject& i_component, ::cppu::OBroadcastHelper & i_broadcastHelper )
            :m_aGuard( i_broadcastHelper.rMutex )
        {
            if ( i_broadcastHelper.bDisposed )
                throw ::com::sun::star::lang::DisposedException( ::rtl::OUString(), &i_component );
        }

        ~ComponentGuard()
        {
        }

        void clear()    { m_aGuard.clear(); }
        void reset()    { m_aGuard.reset(); }

    private:
        ::osl::ResettableMutexGuard m_aGuard;
    };

//......................................................................................................................
} // namespace comphelper
//......................................................................................................................

#endif // COMPHELPER_COMPONENTGUARD_HXX
