/*************************************************************************
 *
 *  $RCSfile: valuenodeaccess.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 13:38:51 $
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

#include "valuenodeaccess.hxx"

#ifndef CONFIGMGR_UPDATEACCESSOR_HXX
#include "updateaccessor.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

// -----------------------------------------------------------------------------

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace data
    {
    // -------------------------------------------------------------------------
        using memory::Pointer;
    // -------------------------------------------------------------------------
        static
        ValueNodeAddress::DataType* accessValue(Pointer const& _p, memory::UpdateAccessor& _rUpdateAccess)
        {
            return static_cast<ValueNodeAddress::DataType*>( _rUpdateAccess.validate(_p) );
        }
    // -------------------------------------------------------------------------
    // -------------------------------------------------------------------------

        Pointer ValueNodeAccess::check(NodeAccessRef const& _aNode)
        {
            if (sharable::Node const* pNode = _aNode.getDataPtr())
                return _aNode.accessor().address(pNode->valueData());

            else
                return Pointer();
        }
    // -------------------------------------------------------------------------

void ValueNodeAccess::setValue(memory::UpdateAccessor & _aUpdater, NodeAddressType _aValueNode, uno::Any const& _aValue)
{
    sharable::ValueNode * node = accessValue(_aValueNode.m_pData,_aUpdater);

    using namespace sharable;
    AnyData::TypeCode aType = node->info.type & Type::mask_valuetype;

    // release old data
    if (node->info.flags & Flags::valueAvailable)
    {
        OSL_ASSERT(aType != Type::value_any);

        freeData(_aUpdater.allocator(),aType,node->value);
        node = accessValue(_aValueNode.m_pData,_aUpdater);

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

            aType = aNewType & Type::mask_valuetype;

            node->info.type |= aType;
        }

        OSL_ENSURE(aType == aNewType, "ERROR: setValue() - Value type does not match");
        if (aType == aNewType)
        {
            // store the data
            sharable::AnyData aNewData = allocData(_aUpdater.allocator(),aType,_aValue);
            node = accessValue(_aValueNode.m_pData,_aUpdater);

            node->value = aNewData;
            node->info.flags |= Flags::valueAvailable;
        }
    }
    node->info.flags &= ~Flags::defaulted;
}
//-----------------------------------------------------------------------------

void ValueNodeAccess::setToDefault(memory::UpdateAccessor & _aUpdater, NodeAddressType _aValueNode)
{
    sharable::ValueNode * node = accessValue(_aValueNode.m_pData,_aUpdater);

    using namespace sharable;
    OSL_ENSURE(node->hasUsableDefault(), "ERROR: setToDefault() - Value does not have a default");

    // release value data
    if (node->info.flags & Flags::valueAvailable)
    {
        AnyData::TypeCode aType = node->info.type & Type::mask_valuetype;
        OSL_ASSERT(aType != Type::value_any);

        freeData(_aUpdater.allocator(),aType,node->value);
        node = accessValue(_aValueNode.m_pData,_aUpdater);

        node->value.data = 0;
        node->info.flags ^= Flags::valueAvailable;
    }

    node->info.flags |= Flags::defaulted;
}
//-----------------------------------------------------------------------------

void ValueNodeAccess::changeDefault(memory::UpdateAccessor & _aUpdater, NodeAddressType _aValueNode, uno::Any const& _aValue)
{
    sharable::ValueNode * node = accessValue(_aValueNode.m_pData,_aUpdater);

    using namespace sharable;
    AnyData::TypeCode aType = node->info.type & Type::mask_valuetype;

    // release old data
    if (node->info.flags & Flags::defaultAvailable)
    {
        OSL_ASSERT(aType != Type::value_any);

        freeData(_aUpdater.allocator(),aType,node->defaultValue);
        node = accessValue(_aValueNode.m_pData,_aUpdater);

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

            aType = aNewType & Type::mask_valuetype;

            node->info.type |= aType;
        }

        OSL_ENSURE(aType == aNewType, "ERROR: changeDefault() - Value type does not match");
        if (aType == aNewType)
        {
            // store the data
            sharable::AnyData aNewData = allocData(_aUpdater.allocator(),aType,_aValue);
            node = accessValue(_aValueNode.m_pData,_aUpdater);

            node->defaultValue = aNewData;
            node->info.flags |= Flags::defaultAvailable;
        }
    }
}

//-----------------------------------------------------------------------------
/*
void ValueNode::clearData(data::Allocator const & _aAlloc)
{
    AnyData::TypeCode aType = info.type & Type::mask_valuetype;

    if (info.flags & Flags::valueAvailable)
    {
        OSL_ASSERT(aType != Type::value_any);
        freeData(_aAlloc,aType,this->value);
        info.flags ^= Flags::valueAvailable;
        this->value.data = 0;
    }

    if (info.flags & Flags::defaultAvailable)
    {
        OSL_ASSERT(aType != Type::value_any);
        freeData(_aAlloc,aType,this->defaultValue);
        info.flags ^= Flags::defaultAvailable;
        this->defaultValue.data = 0;
    }
}
*/
//-----------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr


