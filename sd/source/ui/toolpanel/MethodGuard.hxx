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

#ifndef SD_METHODGUARD_HXX
#define SD_METHODGUARD_HXX

#include <osl/mutex.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    //==================================================================================================================
    //= MethodGuard
    //==================================================================================================================
    template < class COMPONENT >
    class MethodGuard
    {
    public:
        MethodGuard( COMPONENT& i_rComponent )
            :m_aGuard( i_rComponent.getMutex() )
        {
            i_rComponent.checkDisposed();
        }

        ~MethodGuard()
        {
        }

        inline void clear()
        {
            m_aGuard.clear();
        }

    private:
        ::osl::ClearableMutexGuard  m_aGuard;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_METHODGUARD_HXX
