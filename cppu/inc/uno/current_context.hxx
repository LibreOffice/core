/*************************************************************************
 *
 *  $RCSfile: current_context.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dbo $ $Date: 2001-05-07 15:06:56 $
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
#ifndef _UNO_CURRENT_CONTEXT_HXX_
#define _UNO_CURRENT_CONTEXT_HXX_

#include <uno/current_context.h>
#include <cppu/macros.hxx>

#include <com/sun/star/uno/XCurrentContext.hpp>


/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{

/** Getting the current context.

    @return current context or null ref, if none is set
*/
inline Reference< XCurrentContext > SAL_CALL getCurrentContext()
    SAL_THROW( () )
{
    Reference< XCurrentContext > xRet;
    ::rtl::OUString aEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    ::uno_getCurrentContext( (void **)&xRet, aEnvTypeName.pData, 0 );
    return xRet;
}
/** Setting the current context.

    @param xContext current context to be set
    @return true, if context has been successfully set
*/
inline bool SAL_CALL setCurrentContext(
    Reference< XCurrentContext > const & xContext )
    SAL_THROW( () )
{
    ::rtl::OUString aEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(CPPU_CURRENT_LANGUAGE_BINDING_NAME) );
    return (::uno_setCurrentContext( xContext.get(), aEnvTypeName.pData, 0 ) != sal_False);
}

//  /** Permission check.
//  */
//  inline void SAL_CALL checkPermission(
//      ::com::sun::star::security::Permission const & rPerm )
//      SAL_THROW( (::com::sun::star::security::AccessControlException) )
//  {
//      Reference< XCurrentContext > xContext( getCurrentContext() );
//      if (xContext.is())
//      {
//          Reference< ::com::sun::star::security::XAccessController > xAC;
//          if (xContext->getValueByName( OUString(
//              RTL_CONSTASCII_USTRINGPARAM("com.sun.star.security.AccessController") ) ) >>= xAC)
//          {
//              xAC->checkPermission( rPerm );
//          }
//      }
//  }

}
}
}
}

#endif
