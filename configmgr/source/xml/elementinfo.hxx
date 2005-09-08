/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: elementinfo.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:38:52 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#ifndef CONFIGMGR_XML_ELEMENTINFO_HXX
#define CONFIGMGR_XML_ELEMENTINFO_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace xml
    {
// -----------------------------------------------------------------------------
        typedef rtl::OUString ElementName;
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
                remove,

                unknown
            };
        }
// -----------------------------------------------------------------------------
        struct ElementInfo
        {
            typedef sal_Int16 FlagsType;

            explicit
            ElementInfo(ElementType::Enum _type = ElementType::unknown)
            : name()
            , type(_type)
            , op(Operation::none)
            , flags()
            {}

            explicit
            ElementInfo(ElementName const & _name, ElementType::Enum _type = ElementType::unknown)
            : name(_name)
            , type(_type)
            , op(Operation::none)
            , flags()
            {}


            ElementName         name;
            ElementType::Enum   type;
            Operation::Enum     op;
            FlagsType           flags;
        };
// -----------------------------------------------------------------------------
    } // namespace xml
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif

