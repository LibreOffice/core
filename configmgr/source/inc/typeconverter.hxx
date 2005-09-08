/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: typeconverter.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:00:54 $
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
#ifndef CONFIGMGR_TYPECONVERTER_HXX
#define CONFIGMGR_TYPECONVERTER_HXX

#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

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

    // Any Conversion - uses TypeConverter
    uno::Any toAny( const uno::Reference< script::XTypeConverter >& xTypeConverter,
                    const ::rtl::OUString& _rValue,
                    const uno::TypeClass& _rTypeClass)
                CFG_UNO_THROW1( script::CannotConvertException );

    rtl::OUString toString(const uno::Reference< script::XTypeConverter >& xTypeConverter, const uno::Any& rValue)
                CFG_UNO_THROW1( script::CannotConvertException );

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

    // template names
    ::rtl::OUString toTemplateName(const uno::Type& _rType);
    ::rtl::OUString toTemplateName(const uno::TypeClass& _rBasicType, bool bList = false);
    ::rtl::OUString toTemplateName(const ::rtl::OUString& _rBasicTypeName, bool bList = false);

     uno::Type parseTemplateName(::rtl::OUString const& sTypeName);
     bool parseTemplateName(::rtl::OUString const& sTypeName, uno::TypeClass& _rType, bool& bList);
     bool parseTemplateName(::rtl::OUString const& sTypeName, ::rtl::OUString& _rBasicName, bool& bList);

} // namespace configmgr

#endif /* CONFIGMGR_TYPECONVERTER_HXX */
