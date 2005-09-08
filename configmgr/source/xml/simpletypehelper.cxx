/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simpletypehelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:42:15 $
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

#include "simpletypehelper.hxx"

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

namespace configmgr
{
    namespace uno = com::sun::star::uno;
    namespace SimpleTypeHelper
    {

        uno::Type getBooleanType() { return ::getBooleanCppuType(); }

        uno::Type getByteType()     { return ::getCppuType(static_cast<sal_Int8 const*>(0)); }
        uno::Type getShortType()        { return ::getCppuType(static_cast<sal_Int16 const*>(0)); }
        uno::Type getIntType()      { return ::getCppuType(static_cast<sal_Int32 const*>(0)); }
        uno::Type getLongType()     { return ::getCppuType(static_cast<sal_Int64 const*>(0)); }

        uno::Type getDoubleType()   { return ::getCppuType(static_cast<double const*>(0)); }

        uno::Type getStringType()   { return ::getCppuType(static_cast<rtl::OUString const*>(0)); }

        uno::Type getBinaryType()   { return ::getCppuType(static_cast<uno::Sequence<sal_Int8> const*>(0)); }
        uno::Type getAnyType()      { return ::getCppuType(static_cast<uno::Any const*>(0)); }
    }
}
