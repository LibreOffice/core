/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: backendstratalistener.hxx,v $
 * $Revision: 1.4 $
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
#ifndef CONFIGMGR_BACKEND_BACKENDSTRATALISTENER_HXX
#define CONFIGMGR_BACKEND_BACKENDSTRATALISTENER_HXX

#include <com/sun/star/configuration/backend/XBackendChangesListener.hpp>
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#include "backendnotifier.hxx"
#include <com/sun/star/configuration/backend/XBackend.hpp>
#include "multistratumbackend.hxx"


#include <cppuhelper/implbase1.hxx>

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace configmgr
{
    // -----------------------------------------------------------------------------
    namespace backend
    {
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

        namespace backenduno = ::com::sun::star::configuration::backend;
        // --------------------------------------------------------------------------
        class BackendStrataListener: public cppu::WeakImplHelper1<backenduno::XBackendChangesListener>
        {
        public:
            BackendStrataListener(const MultiStratumBackend& aBackend);
            ~BackendStrataListener();
            virtual void SAL_CALL componentDataChanged(const backenduno::ComponentChangeEvent& aEvent)
            throw (::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL disposing( lang::EventObject const & rSource )
            throw (uno::RuntimeException);
        private:
             const MultiStratumBackend& mBackend ;

        };



    }
    // -----------------------------------------------------------------------------
}
// -----------------------------------------------------------------------------
#endif
