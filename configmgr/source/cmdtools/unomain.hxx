/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unomain.hxx,v $
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

#ifndef INCLUDED_UNOMAIN_HXX
#define INCLUDED_UNOMAIN_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace unoapp
{
//-----------------------------------------------------------------------------
    namespace uno = ::com::sun::star::uno;

    /// helper: retrieves the full file URL of the application
    extern ::rtl::OUString SAL_CALL getAppCommandURL();
    /// helper: retrieves the full system path of the application
    extern ::rtl::OUString SAL_CALL getAppCommandPath();
    /// helper: retrieves the name of the application
    extern ::rtl::OUString SAL_CALL getAppCommandName();
    /// helper: retrieves all the commandline arguments of the application (skipping -env: arguments)
    extern uno::Sequence< ::rtl::OUString > SAL_CALL getAppCommandArgs();

    /// global variable that allows to suppress all error output from the unoapp launcher, if set to <TRUE/>
    extern bool g_bQuiet;

    /// entrypoint that must be implemented by the application
    extern int SAL_CALL uno_main( uno::Reference< uno::XComponentContext > const & xContext)
        SAL_THROW( (uno::Exception) );
//-----------------------------------------------------------------------------
}


#endif //  INCLUDED_UNOMAIN_HXX
