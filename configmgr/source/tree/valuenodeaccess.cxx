/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: valuenodeaccess.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "valuenodeaccess.hxx"
#include <osl/diagnose.h>

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {

void ValueNodeAccess::setValue(ValueNodeAddress _aValueNode,
                               uno::Any const& _aValue)
{
    sharable::ValueNode * node = _aValueNode;

    using namespace sharable;
    AnyData::TypeCode aType = AnyData::TypeCode( node->info.type & Type::mask_valuetype );

    // release old data
    if (node->info.flags & Flags::valueAvailable)
    {
        OSL_ASSERT(aType != Type::value_any);

        freeData(aType,node->value);
        node = _aValueNode;

        node->value.data = 0;
        node->info.flags ^= Flags::valueAvailable;
    }

    // set new value
    if (_aValue.hasValue())
    {
        AnyData::TypeCode aNewType = getTypeCode(_aValue.getValueType());
        OSL_ASSERT(aNewType != Type::value_any);

        // set new type, if appropriate
        if (aType == Type::value_any)
        {
            OSL_ASSERT(Type::value_any == 0);
            OSL_ASSERT(aNewType != Type::value_any);

            aType = AnyData::TypeCode( aNewType & Type::mask_valuetype );

            node->info.type |= aType;
        }

        OSL_ENSURE(aType == aNewType, "ERROR: setValue() - Value type does not match");
        if (aType == aNewType)
        {
            // store the data
            sharable::AnyData aNewData = allocData(aType,_aValue);
            node = _aValueNode;

            node->value = aNewData;
            node->info.flags |= Flags::valueAvailable;
        }
    }
    node->info.flags &= ~Flags::defaulted;
}
//-----------------------------------------------------------------------------

void ValueNodeAccess::setToDefault(ValueNodeAddress _aValueNode)
{
    sharable::ValueNode * node = _aValueNode;

    using namespace sharable;
    OSL_ENSURE(node->hasUsableDefault(), "ERROR: setToDefault() - Value does not have a default");

    // release value data
    if (node->info.flags & Flags::valueAvailable)
    {
        AnyData::TypeCode aType = AnyData::TypeCode( node->info.type & Type::mask_valuetype );
        OSL_ASSERT(aType != Type::value_any);

        freeData(aType,node->value);
        node = _aValueNode;

        node->value.data = 0;
        node->info.flags ^= Flags::valueAvailable;
    }

    node->info.flags |= Flags::defaulted;
}
//-----------------------------------------------------------------------------

void ValueNodeAccess::changeDefault(ValueNodeAddress _aValueNode,
                                    uno::Any const& _aValue)
{
    sharable::ValueNode * node = _aValueNode;

    using namespace sharable;
    AnyData::TypeCode aType = AnyData::TypeCode( node->info.type & Type::mask_valuetype );

    // release old data
    if (node->info.flags & Flags::defaultAvailable)
    {
        OSL_ASSERT(aType != Type::value_any);

        freeData(aType,node->defaultValue);
        node = _aValueNode;

        node->defaultValue.data = 0;
        node->info.flags ^= Flags::defaultAvailable;
    }

    // set new value
    if (_aValue.hasValue())
    {
        AnyData::TypeCode aNewType = getTypeCode(_aValue.getValueType());
        OSL_ASSERT(aNewType != Type::value_any);

        // set new type, if appropriate
        if (aType == Type::value_any)
        {
            OSL_ASSERT(Type::value_any == 0);
            OSL_ASSERT(aNewType != Type::value_any);

            aType = AnyData::TypeCode( aNewType & Type::mask_valuetype );

            node->info.type |= aType;
        }

        OSL_ENSURE(aType == aNewType, "ERROR: changeDefault() - Value type does not match");
        if (aType == aNewType)
        {
            // store the data
            sharable::AnyData aNewData = allocData(aType,_aValue);
            node = _aValueNode;

            node->defaultValue = aNewData;
            node->info.flags |= Flags::defaultAvailable;
        }
    }
}

    } // namespace data
} // namespace configmgr


