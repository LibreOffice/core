/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unomain.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:38:15 $
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

#ifndef INCLUDED_UNOMAIN_HXX
#define INCLUDED_UNOMAIN_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

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
