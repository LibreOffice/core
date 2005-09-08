/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: backendfactory.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:42:49 $
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

#ifndef CONFIGMGR_BACKENDFACTORY_HXX_
#define CONFIGMGR_BACKENDFACTORY_HXX_

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_BACKEND_XBACKEND_HPP_
#include <com/sun/star/configuration/backend/XBackend.hpp>
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace backend
    {
//-----------------------------------------------------------------------------

        struct IMergedDataProvider;
//-----------------------------------------------------------------------------
        class BackendFactory
        {
        public:
            typedef com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
                CreationContext;

            typedef com::sun::star::uno::Reference< com::sun::star::configuration::backend::XBackend >
                UnoBackend;

            rtl::Reference<IMergedDataProvider> createBackend();

            UnoBackend getUnoBackend();

            static BackendFactory instance(CreationContext const & _xCtx);

        private:
            explicit
            BackendFactory(CreationContext const & _xCtx)
            : m_xCtx(_xCtx)
            {}

            CreationContext m_xCtx;
        };
//-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}


#endif

