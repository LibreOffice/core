/*************************************************************************
 *
 *  $RCSfile: codemaker.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-03-01 11:57:01 $
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

#include "codemaker/codemaker.hxx"

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

namespace {

void checkNoTypeArguments(std::vector< rtl::OString > const & arguments) {
    if (!arguments.empty()) {
        throw CannotDumpException(
            rtl::OString(RTL_CONSTASCII_STRINGPARAM("Bad type information")));
            //TODO
    }
}

}

namespace codemaker {

rtl::OString convertString(rtl::OUString const & string) {
    rtl::OString s;
    if (!string.convertToString(
            &s, RTL_TEXTENCODING_UTF8,
            (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
             | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
    {
        throw CannotDumpException(
            rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "Failure converting string from UTF-16 to UTF-8")));
    }
    return s;
}

codemaker::UnoType::Sort decomposeAndResolve(
    TypeManager const & manager, rtl::OString const & type,
    bool resolveTypedefs, bool allowVoid, bool allowExtraEntities,
    RTTypeClass * typeClass, rtl::OString * name, sal_Int32 * rank,
    std::vector< rtl::OString > * arguments)
{
    OSL_ASSERT(typeClass != 0 && name != 0 && rank != 0 && arguments != 0);
    *rank = 0;
    for (rtl::OString t(type);;) {
        sal_Int32 n = 0;
        *name = codemaker::UnoType::decompose(t, &n, arguments);
        if (n > SAL_MAX_INT32 - *rank) {
            throw CannotDumpException(
                rtl::OString(
                    RTL_CONSTASCII_STRINGPARAM("Bad type information"))); //TODO
        }
        *rank += n;
        if (n > 0) {
            allowVoid = false;
            allowExtraEntities = false;
        }
        codemaker::UnoType::Sort sort = codemaker::UnoType::getSort(*name);
        switch (sort) {
        case codemaker::UnoType::SORT_VOID:
            if (!allowVoid) {
                throw CannotDumpException(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information")));
                    //TODO
            }
        default:
            checkNoTypeArguments(*arguments);
            *typeClass = RT_TYPE_INVALID;
            return sort;

        case codemaker::UnoType::SORT_COMPLEX:
            typereg::Reader reader(manager.getTypeReader(*name));
            *typeClass = reader.getTypeClass();
            switch (*typeClass) {
            case RT_TYPE_ENUM:
            case RT_TYPE_INTERFACE:
                checkNoTypeArguments(*arguments);
                return sort;

            case RT_TYPE_STRUCT:
                if (!(allowExtraEntities && arguments->empty())
                    && (arguments->size() > SAL_MAX_UINT16
                        || (static_cast< sal_uInt16 >(arguments->size())
                            != reader.getReferenceCount())))
                {
                    throw CannotDumpException(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Bad type information"))); //TODO
                }
                return sort;

            case RT_TYPE_MODULE:
            case RT_TYPE_EXCEPTION:
            case RT_TYPE_SERVICE:
            case RT_TYPE_SINGLETON:
            case RT_TYPE_CONSTANTS:
                if (!allowExtraEntities) {
                    throw CannotDumpException(
                        rtl::OString(
                            RTL_CONSTASCII_STRINGPARAM(
                                "Bad type information"))); //TODO
                }
                checkNoTypeArguments(*arguments);
                //TODO: check reader for consistency
                return sort;

            case RT_TYPE_TYPEDEF:
                checkNoTypeArguments(*arguments);
                if (reader.getSuperTypeCount() == 1
                    && reader.getFieldCount() == 0
                    && reader.getMethodCount() == 0
                    && reader.getReferenceCount() == 0)
                {
                    if (resolveTypedefs) {
                        t = convertString(reader.getSuperTypeName(0));
                        continue;
                    } else {
                        return sort;
                    }
                }
            default:
                throw CannotDumpException(
                    rtl::OString(
                        RTL_CONSTASCII_STRINGPARAM("Bad type information")));
                    //TODO
            }
        }
    }
}

}
