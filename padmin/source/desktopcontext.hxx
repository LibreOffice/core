/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: desktopcontext.hxx,v $
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

#ifndef _PADMIN_DESKTOPCONTEXT_HXX_
#define _PADMIN_DESKTOPCONTEXT_HXX_

#include <cppuhelper/implbase1.hxx>
#include <uno/current_context.hxx>

#define DESKTOP_ENVIRONMENT_NAME "system.desktop-environment"

namespace padmin
{
    class DesktopContext: public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >
    {
    public:
    DesktopContext( const com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > & ctx);

        // XCurrentContext
        virtual com::sun::star::uno::Any SAL_CALL getValueByName( const rtl::OUString& Name )
            throw (com::sun::star::uno::RuntimeException);

    private:
            com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > m_xNextContext;
    };
}

#endif // _PADMIN_DESKTOPCONTEXT_HXX_
