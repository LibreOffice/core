/*************************************************************************
 *
 *  $RCSfile: commontypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dg $ $Date: 2000-10-24 11:58:36 $
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

#ifndef _CONFIGMGR_COMMONTYPES_HXX_
#define _CONFIGMGR_COMMONTYPES_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

//..........................................................................
namespace configmgr
{
//..........................................................................

//==========================================================================
//= IInterface
//==========================================================================
/** abstract base class for objects which may be aquired by ORef's
    (in opposite to the IReference, classes deriving from this IInterface can be
    derived from XInterface, too)
*/
class IInterface
{
public:
    virtual void SAL_CALL acquire(  ) throw () = 0;
    virtual void SAL_CALL release(  ) throw () = 0;
};

//==========================================================================
//= SettingsOverride
//==========================================================================
DECLARE_STL_USTRINGACCESS_MAP( ::com::sun::star::uno::Any, SettingsOverride );

typedef ::com::sun::star::uno::RuntimeException CantRenameException_Base;
class CantRenameException : CantRenameException_Base
{
public:
    ::rtl::OUString newName;
    ::rtl::OUString oldName;

    static ::rtl::OUString message(::rtl::OUString const& sNewName, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed> xContext)
    {
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Object cannot be renamed") );
    }

    CantRenameException(::rtl::OUString const& sNewName, ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > xContext)
        : CantRenameException_Base(message(sNewName,xContext), xContext)
        , newName(sNewName)
        , oldName(xContext->getName())
    {
    }
};

//..........................................................................
}   // namespace configmgr
//..........................................................................

#endif // _CONFIGMGR_COMMONTYPES_HXX_


