/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elementinfo.hxx,v $
 * $Revision: 1.9 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#ifndef CONFIGMGR_XML_ELEMENTINFO_HXX
#define CONFIGMGR_XML_ELEMENTINFO_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        namespace ElementType
        {
            enum Enum
            {
                unknown,

                schema,
                layer,

                component,
                templates,

                property,
                node,
                group,
                set,

                import,
                instance,
                item_type,
                value,
                uses,

                other
            };
        }
// -----------------------------------------------------------------------------
        namespace Operation
        {
            enum Enum
            {
                none,

                modify,
                clear,

                replace,
                fuse,
                remove,

                unknown
            };
        }
// -----------------------------------------------------------------------------
        struct ElementInfo
        {
            explicit
            ElementInfo(ElementType::Enum _type = ElementType::unknown)
            : name()
            , type(_type)
            , op(Operation::none)
            , flags()
            {}

            explicit
            ElementInfo(rtl::OUString const & _name, ElementType::Enum _type = ElementType::unknown)
            : name(_name)
            , type(_type)
            , op(Operation::none)
            , flags()
            {}


            rtl::OUString         name;
            ElementType::Enum   type;
            Operation::Enum     op;
            sal_Int16           flags;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif

