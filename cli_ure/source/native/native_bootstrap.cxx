/*************************************************************************
 *
 *  $RCSfile: native_bootstrap.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2003-08-20 12:53:21 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "native_share.h"

#include "rtl/bootstrap.hxx"
#include "cppuhelper/bootstrap.hxx"


[assembly:System::Reflection::AssemblyProduct( "CLI-UNO Language Binding" )];
// xxx todo AssemblyCompany, AssemblyCopyright, AssemblyTrademark
[assembly:System::Reflection::AssemblyDescription( "CLI-UNO Helper Library" )];
[assembly:System::Reflection::AssemblyVersion( "3.1.0.0" )];

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace uno
{
namespace util
{

/** Bootstrapping native UNO.
*/
public __sealed __gc class Bootstrap
{
    inline Bootstrap() {}

public:

    /** Bootstraps the initial component context from a native UNO installation.

        @see cppuhelper/bootstrap.hxx:defaultBootstrap_InitialComponentContext()
    */
    static ::unoidl::com::sun::star::uno::XComponentContext *
        defaultBootstrap_InitialComponentContext();

    /** Bootstraps the initial component context from a native UNO installation.

        @param ini_file
               ini_file (may be null: uno.ini/unorc besides cppuhelper lib)
        @param bootstrap_parameters
               bootstrap parameters (maybe null)

        @see cppuhelper/bootstrap.hxx:defaultBootstrap_InitialComponentContext()
    */
    static ::unoidl::com::sun::star::uno::XComponentContext *
        defaultBootstrap_InitialComponentContext(
            ::System::String * ini_file,
            ::System::Collections::IDictionaryEnumerator *
              bootstrap_parameters );
};

//______________________________________________________________________________
::unoidl::com::sun::star::uno::XComponentContext *
Bootstrap::defaultBootstrap_InitialComponentContext(
    ::System::String * ini_file,
    ::System::Collections::IDictionaryEnumerator * bootstrap_parameters )
{
    if (0 != bootstrap_parameters)
    {
        bootstrap_parameters->Reset();
        while (bootstrap_parameters->MoveNext())
        {
            OUString key(
                String_to_ustring( __try_cast< ::System::String * >(
                                       bootstrap_parameters->get_Key() ) ) );
            OUString value(
                String_to_ustring( __try_cast< ::System::String * >(
                                       bootstrap_parameters->get_Value() ) ) );

            ::rtl::Bootstrap::set( key, value );
        }
    }

    // bootstrap native uno
    Reference< XComponentContext > xContext;
    if (0 == ini_file)
    {
        xContext = ::cppu::defaultBootstrap_InitialComponentContext();
    }
    else
    {
        xContext = ::cppu::defaultBootstrap_InitialComponentContext(
            String_to_ustring( __try_cast< ::System::String * >( ini_file ) ) );
    }

    return __try_cast< ::unoidl::com::sun::star::uno::XComponentContext * >(
        to_cli( xContext ) );
}

//______________________________________________________________________________
::unoidl::com::sun::star::uno::XComponentContext *
Bootstrap::defaultBootstrap_InitialComponentContext()
{
    return defaultBootstrap_InitialComponentContext( 0, 0 );
}

}
}
