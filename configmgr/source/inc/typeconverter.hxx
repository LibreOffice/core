/*************************************************************************
 *
 *  $RCSfile: typeconverter.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jb $ $Date: 2001-04-05 09:19:18 $
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
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#define CONFIGMGR_TYPECONVERTER_HXX

#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace configmgr
{
    namespace uno = ::com::sun::star::uno;
    namespace script = ::com::sun::star::script;

    // UNO Type handling
    uno::Type getSequenceElementType(uno::Type const& rSequenceType);

    uno::Type getBasicType(uno::Type const& rType, bool& bSequence);
    inline
    uno::Type getBasicType(uno::Type const& rType)
    { bool dummy; return getBasicType(rType,dummy); }

    // Any Conversion
    uno::Any toAny(const uno::Reference< script::XTypeConverter >& xTypeConverter, const ::rtl::OUString& _rValue,const uno::TypeClass& _rTypeClass) throw( script::CannotConvertException );
    rtl::OUString toString(const uno::Reference< script::XTypeConverter >& xTypeConverter, const uno::Any& rValue) throw( script::CannotConvertException );

    // Type conversion
    uno::TypeClass toTypeClass(const ::rtl::OUString& _rType);
    ::rtl::OUString toTypeName(const uno::TypeClass& _rTypeClass);

    uno::Type toType(const ::rtl::OUString& _rsType);
    uno::Type toListType(const ::rtl::OUString& _rsElementType);
    ::rtl::OUString toTypeName(const uno::Type& _rType);

    inline
    uno::Type toType(const ::rtl::OUString& _rsSimpleType, bool isList)
    {
        return isList ? toListType(_rsSimpleType) : toType(_rsSimpleType);
    }

    inline uno::Type getBinaryType()    { return ::getCppuType(static_cast<uno::Sequence<sal_Int8> const*>(0)); }
    inline uno::Type getAnyType()       { return ::getCppuType(static_cast<uno::Any const*>(0)); }

    // template names
    ::rtl::OUString toTemplateName(const uno::Type& _rType);
    ::rtl::OUString toTemplateName(const uno::TypeClass& _rBasicType, bool bList = false);
    ::rtl::OUString toTemplateName(const ::rtl::OUString& _rBasicTypeName, bool bList = false);

     uno::Type parseTemplateName(::rtl::OUString const& sTypeName);
     bool parseTemplateName(::rtl::OUString const& sTypeName, uno::TypeClass& _rType, bool& bList);
     bool parseTemplateName(::rtl::OUString const& sTypeName, ::rtl::OUString& _rBasicName, bool& bList);

} // namespace configmgr

#endif /* CONFIGMGR_TYPECONVERTER_HXX */
