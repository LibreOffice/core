/*************************************************************************
 *
 *  $RCSfile: commonjava.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-01 11:57:31 $
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

#include "sal/config.h"

#include "codemaker/commonjava.hxx"

#include "codemaker/options.hxx"
#include "codemaker/typemanager.hxx"
#include "codemaker/unotype.hxx"

#include "osl/diagnose.h"
#include "registry/reader.hxx"
#include "registry/types.h"
#include "rtl/strbuf.h"
#include "rtl/string.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include <vector>

namespace codemaker { namespace java {

rtl::OString translateUnoToJavaType(
    codemaker::UnoType::Sort sort, RTTypeClass typeClass,
    rtl::OString const & nucleus, bool referenceType)
{
    rtl::OStringBuffer buf;
    if (sort == codemaker::UnoType::SORT_COMPLEX) {
        if (typeClass == RT_TYPE_INTERFACE
            && nucleus == rtl::OString("com/sun/star/uno/XInterface"))
        {
            buf.append(RTL_CONSTASCII_STRINGPARAM("java/lang/Object"));
        } else {
            //TODO: check that nucleus is a valid (Java-modified UTF-8)
            // identifier
            buf.append(nucleus);
        }
    } else {
        rtl::OString const javaTypes[codemaker::UnoType::SORT_ANY + 1][2] = {
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("void")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Void")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("boolean")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Boolean")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("byte")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Byte")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("short")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Short")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("short")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Short")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("int")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Integer")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("int")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Integer")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("long")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Long")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("long")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Long")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("float")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Float")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("double")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Double")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("char")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Character")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/String")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/String")) },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("com/sun/star/uno/Type"))
            },
            { rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")),
              rtl::OString(RTL_CONSTASCII_STRINGPARAM("java/lang/Object")) } };
        buf.append(javaTypes[sort][referenceType]);
    }
    return buf.makeStringAndClear();
}

rtl::OString translateUnoToJavaIdentifier(
    rtl::OString const & identifier, rtl::OString const & prefix)
{
    if (identifier == "abstract"
        || identifier == "assert" // since Java 1.4
        || identifier == "boolean"
        || identifier == "break"
        || identifier == "byte"
        || identifier == "case"
        || identifier == "catch"
        || identifier == "char"
        || identifier == "class"
        || identifier == "const"
        || identifier == "continue"
        || identifier == "default"
        || identifier == "do"
        || identifier == "double"
        || identifier == "else"
        || identifier == "enum" // probable addition in Java 1.5
        || identifier == "extends"
        || identifier == "final"
        || identifier == "finally"
        || identifier == "float"
        || identifier == "for"
        || identifier == "goto"
        || identifier == "if"
        || identifier == "implements"
        || identifier == "import"
        || identifier == "instanceof"
        || identifier == "int"
        || identifier == "interface"
        || identifier == "long"
        || identifier == "native"
        || identifier == "new"
        || identifier == "package"
        || identifier == "private"
        || identifier == "protected"
        || identifier == "public"
        || identifier == "return"
        || identifier == "short"
        || identifier == "static"
        || identifier == "strictfp"
        || identifier == "super"
        || identifier == "switch"
        || identifier == "synchronized"
        || identifier == "this"
        || identifier == "throw"
        || identifier == "throws"
        || identifier == "transient"
        || identifier == "try"
        || identifier == "void"
        || identifier == "volatile"
        || identifier == "while")
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
