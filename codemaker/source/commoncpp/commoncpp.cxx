/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commoncpp.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2005-10-27 17:11:08 $
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
#include "sal/config.h"

#include "codemaker/commoncpp.hxx"

#include "codemaker/options.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "registry/reader.hxx"
#include "registry/types.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <vector>

namespace codemaker { namespace cpp {

rtl::OString scopedCppName(rtl::OString const & type, bool bNoNameSpace,
                           bool shortname)
{
    char c('/');
    sal_Int32 nPos = type.lastIndexOf( c );
    if (nPos == -1) {
        nPos = type.lastIndexOf( '.' );
        if (nPos == -1)
            return type;
        c = '.';
    }
    if (bNoNameSpace)
        return type.copy(nPos+1);

    rtl::OStringBuffer tmpBuf(type.getLength()*2);
    nPos = 0;
    do
    {
        tmpBuf.append("::");
        tmpBuf.append(type.getToken(0, c, nPos));
    } while( nPos != -1 );

    if (shortname) {
        rtl::OString s(tmpBuf.makeStringAndClear());
        if (s.indexOf("::com::sun::star") == 0)
            return s.replaceAt(0, 16, "css");
    }

    return tmpBuf.makeStringAndClear();
}

rtl::OString translateUnoToCppType(
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    rtl::OString const & nucleus)
{
    rtl::OStringBuffer buf;
    if (sort == codemaker::UnoType::SORT_COMPLEX) {
        if (typeClass == RT_TYPE_INTERFACE
            && nucleus == rtl::OString("com/sun/star/uno/XInterface"))
        {
            buf.append(RTL_CONSTASCII_STRINGPARAM("::com::sun::star::uno::XInterface"));
        } else {
            //TODO: check that nucleus is a valid (UTF-8) identifier
            buf.append(nucleus);
        }
    } else {
        rtl::OString const cppTypes[codemaker::UnoType::SORT_ANY + 1] = {
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("void")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_Bool")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_Int8")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_Int16")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_uInt16")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_Int32")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_uInt32")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_Int64")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_uInt64")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("float")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("double")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::sal_Unicode")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::rtl::OUString")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::com::sun::star::uno::Type")),
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("::com::sun::star::uno::Any")) };
        buf.append(cppTypes[sort]);
    }
    return buf.makeStringAndClear();
}

rtl::OString translateUnoToCppIdentifier(
    rtl::OString const & identifier, rtl::OString const & prefix)
{
    if (identifier == "and"
        || identifier == "and_eq"
        || identifier == "asm"
        || identifier == "auto"
        || identifier == "bitand"
        || identifier == "bitor"
        || identifier == "bool"
        || identifier == "break"
        || identifier == "case"
        || identifier == "catch"
        || identifier == "char"
        || identifier == "class"
        || identifier == "compl"
        || identifier == "const"
        || identifier == "const_cast"
        || identifier == "continue"
        || identifier == "default"
        || identifier == "delete"
        || identifier == "do"
        || identifier == "double"
        || identifier == "dynamic_cast"
        || identifier == "else"
        || identifier == "enum"
        || identifier == "explicit"
        || identifier == "export"
        || identifier == "extern"
        || identifier == "false"
        || identifier == "float"
        || identifier == "for"
        || identifier == "friend"
        || identifier == "goto"
        || identifier == "if"
        || identifier == "inline"
        || identifier == "int"
        || identifier == "long"
        || identifier == "mutable"
        || identifier == "namespace"
        || identifier == "new"
        || identifier == "not"
        || identifier == "not_eq"
        || identifier == "operator"
        || identifier == "or"
        || identifier == "or_eq"
        || identifier == "private"
        || identifier == "protected"
        || identifier == "public"
        || identifier == "register"
        || identifier == "reinterpret_cast"
        || identifier == "return"
        || identifier == "short"
        || identifier == "signed"
        || identifier == "sizeof"
        || identifier == "static"
        || identifier == "static_cast"
        || identifier == "struct"
        || identifier == "switch"
        || identifier == "template"
        || identifier == "this"
        || identifier == "throw"
        || identifier == "true"
        || identifier == "try"
        || identifier == "typedef"
        || identifier == "typeid"
        || identifier == "typename"
        || identifier == "union"
        || identifier == "unsigned"
        || identifier == "using"
        || identifier == "virtual"
        || identifier == "void"
        || identifier == "volatile"
        || identifier == "wchar_t"
        || identifier == "while"
        || identifier == "xor"
        || identifier == "xor_eq")
    {
        rtl::OStringBuffer buf(prefix);
        buf.append('_');
        buf.append(identifier);
        return buf.makeStringAndClear();
    } else {
        return identifier;
    }
}

} }
