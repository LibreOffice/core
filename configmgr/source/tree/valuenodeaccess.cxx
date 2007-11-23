/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: valuenodeaccess.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:34:23 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "valuenodeaccess.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

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


