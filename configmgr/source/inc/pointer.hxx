/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pointer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2005-12-28 17:30:54 $
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

#ifndef CONFIGMGR_POINTER_HXX
#define CONFIGMGR_POINTER_HXX

#ifndef CONFIGMGR_MEMORYMODEL_HXX
#include "memorymodel.hxx"
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace memory
    {
    // -------------------------------------------------------------------------
        class Accessor;
        class UpdateAccessor;
    // -------------------------------------------------------------------------
        /// class mediating read-only access to a memory::Segment
        class Pointer
        {
            friend class Accessor;
            friend class UpdateAccessor;

            typedef memory::Address AddressType;

            AddressType m_value;

            struct Opaque_;
        public:
            typedef AddressType RawAddress;

            Pointer() : m_value(0) {}
            explicit Pointer(AddressType p) : m_value(p) {}

            RawAddress value() const { return m_value; }

            bool isNull()   const { return m_value == 0; }
            bool is()       const { return m_value != 0; }

            operator Opaque_ const * () const { return reinterpret_cast<Opaque_ const *>(m_value); }

            friend bool operator == (Pointer lhs, Pointer rhs)
            { return lhs.value() == rhs.value(); }

            friend bool operator != (Pointer lhs, Pointer rhs)
            { return lhs.value() != rhs.value(); }
        };

    // -------------------------------------------------------------------------
    }
// -----------------------------------------------------------------------------
} // namespace configmgr
// -----------------------------------------------------------------------------

#endif // CONFIGMGR_POINTER_HXX

