/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backendstratalistener.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:24:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONFIGMGR_BACKEND_BACKENDSTRATALISTENER_HXX
#define CONFIGMGR_BACKEND_BACKENDSTRATALISTENER_HXX

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKENDCHANGESLISTENER_HPP_
#include <com/sun/star/configuration/backend/XBackendChangesListener.hpp>
#endif

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_COMPONENTCHANGEEVENT_HPP_
#include <com/sun/star/configuration/backend/ComponentChangeEvent.hpp>
#endif

#ifndef CONFIGMGR_BACKEND_BACKENDNOTIFIER_HXX
#include "backendnotifier.hxx"
#endif // CONFIGMGR_BACKEND_BACKENDNOTIFIER_HXX

#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <com/sun/star/configuration/backend/XBackend.hpp>
#endif // _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_

#ifndef CONFIGMGR_BACKEND_MULTISTRATUMBACKEND_HXX_
#include "multistratumbackend.hxx"
#endif


#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

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
        typedef ::cppu::WeakImplHelper1<backenduno::XBackendChangesListener> BackendStrataListener_Base;

        class BackendStrataListener: public BackendStrataListener_Base
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
