/*************************************************************************
 *
 *  $RCSfile: exc_hlp.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2004-03-25 15:01:32 $
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

/** Use this function to get the dynamic type of a caught C++-UNO exception;
    completes the above function throwing exceptions generically.

    try
    {
        ...
    }
    catch (::com::sun::star::uno::RuntimeException &)
    {
        // you ought not handle RuntimeExceptions:
        throw;
    }
    catch (::com::sun::star::uno::Exception &)
    {
        ::com::sun::star::uno::Any caught( ::cppu::getCaughtException() );
        ...
    }

    Restrictions:
    - use only for caught C++-UNO exceptions (UNOIDL defined)
    - only as first statement in a catch block!
    - don't do a C++ rethrow (throw;) after you have called this function
    - call getCaughtException() just once in your catch block!
      (function internally uses a C++ rethrow)

    @return
              caught UNO exception

    @attention Caution!
              This function is limited to the same C++ compiler runtime library.
              E.g. for MSVC, this means that the catch handler code (the one
              that calls getCaughtException()) needs to use the very same
              C++ runtime library, e.g. msvcrt.dll as cppuhelper, e.g.
              cppuhelper3MSC.dll and the bridge library, e.g. msci_uno.dll.
              This is the case if all of them are compiled with the same
              compiler version.
              Background: The msci_uno.dll gets a rethrown exception out
              of the internal msvcrt.dll thread local storage (tls).
              Thus you _must_ not use this function if your code needs to run
              in newer UDK versions without being recompiled, because those
              newer UDK (-> OOo versions) potentially use newer C++ runtime
              libraries which most often become incompatible!

              But this function ought to be usable for most OOo internal C++-UNO
              development, because the whole OOo code base is compiled using the
              same C++ compiler (and linking against one runtime library).
*/
::com::sun::star::uno::Any SAL_CALL getCaughtException();

}

#endif

