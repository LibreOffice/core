/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: typeconverter.hxx,v $
 * $Revision: 1.10 $
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
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#define CONFIGMGR_TYPECONVERTER_HXX

#include "utility.hxx"
#include <com/sun/star/script/XTypeConverter.hpp>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>

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

    // Any Conversion - uses TypeConverter
    uno::Any toAny( const uno::Reference< script::XTypeConverter >& xTypeConverter,
                    const ::rtl::OUString& _rValue,
                    const uno::TypeClass& _rTypeClass)
                SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException));

    rtl::OUString toString(const uno::Reference< script::XTypeConverter >& xTypeConverter, const uno::Any& rValue)
                SAL_THROW((script::CannotConvertException , com::sun::star::uno::RuntimeException));

    // Type conversion
    ::rtl::OUString toTypeName(const uno::TypeClass& _rTypeClass);

    uno::Type toType(const ::rtl::OUString& _rsType);
    uno::Type toListType(const ::rtl::OUString& _rsElementType);
    ::rtl::OUString toTypeName(const uno::Type& _rType);

    inline
    uno::Type toType(const ::rtl::OUString& _rsSimpleType, bool isList)
    {
        return isList ? toListType(_rsSimpleType) : toType(_rsSimpleType);
    }

    // template names
    ::rtl::OUString toTemplateName(const uno::Type& _rType);
    ::rtl::OUString toTemplateName(const uno::TypeClass& _rBasicType, bool bList = false);
    ::rtl::OUString toTemplateName(const ::rtl::OUString& _rBasicTypeName, bool bList = false);

     uno::Type parseTemplateName(::rtl::OUString const& sTypeName);
     bool parseTemplateName(::rtl::OUString const& sTypeName, ::rtl::OUString& _rBasicName, bool& bList);

} // namespace configmgr

#endif /* CONFIGMGR_TYPECONVERTER_HXX */
