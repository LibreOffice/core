/*************************************************************************
 *
 *  $RCSfile: exc_hlp.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-10-06 12:50:24 $
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

#ifndef _CPPUHELPER_EXC_HLP_HXX_
#define _CPPUHELPER_EXC_HLP_HXX_

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

namespace cppu
{

/** This function throws the exception given by rExc.  The given value has to
    be of typeclass EXCEPTION and must be dervived from or of
    type com.sun.star.uno.Exception.

    @param rExc
           exception to be thrown.
*/
void SAL_CALL throwException( const ::com::sun::star::uno::Any & rExc )
    SAL_THROW( (::com::sun::star::uno::Exception) );

/** @internal
                Not for public use!

    Currently under investigation if this function runs with all known
    C++-UNO bridges.  Until now tested for:

    - CC5, Solaris SPARC
    - MSVC .NET 2002/2003, Windows
    - gcc 3.2.2, Linux Intel


    Use this function to get the dynamic type of a caught C++-UNO exception,
    e.g.

    try
    {
        ...
    }
    catch (::com::sun::star::uno::Exception &)
    {
        ::com::sun::star::uno::Any caught( ::cppu::getCaughtException() );
        ...
    }

    Restrictions:

    - only use for caught UNO exceptions
      (C++ exceptions derived from com::sun::star::uno::Exception)
    - only as first statement in a catch block!
    - never do a C++ rethrow (throw;) again after you have called this function
      and call getCaughtException() just once!
      (function internally uses a C++ rethrow)

    @return
              caught UNO exception

    @attention
              This function is limited to the same C++ compiler runtime library.
              E.g. for MSVC, this means that the catch handler code (the one
              that calls getCaughtException()) needs to use the very same
              msvcrt.dll as cppuhelper3MSC.dll and the bridge msci_uno.dll,
              e.g. all them are compiled with the same compiler version.
              This is because the msci_uno.dll gets a rethrown exception out
              of the internal msvcrt.dll thread local data (tls).
              Thus you must not use this function if your code needs to run
              in newer UDK versions without being recompiled, because those
              newer UDK (-> OOo versions) potentially use a newer MSVC
              (which potentially uses an incompatible msvcrt.dll).
*/
::com::sun::star::uno::Any SAL_CALL getCaughtException();

}

#endif

