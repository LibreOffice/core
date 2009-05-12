/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: flags.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_DATA_FLAGS_HXX
#define INCLUDED_DATA_FLAGS_HXX

#include <sal/types.h>

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace data
    {
    //-----------------------------------------------------------------------------
    // node type + value type selector - fits into a byte (bit 5 currently unassigned)
        namespace Type
        {
            enum Type
            {
            // Primitive Data Type is stored in bits 0-4 (bit 3 currently unused)
                // base data types
                value_any,       // = 0
                value_string,    // = 1
                value_boolean,   // = 2
                value_short,     // = 3
                value_int,       // = 4
                value_long,      // = 5
                value_double,    // = 6
                value_binary,    // = 7

                mask_basetype = 0x0F,

                // sequence data types
                flag_sequence = 0x10,

                value_invalid        = value_any | flag_sequence,

                value_string_list    = value_string   | flag_sequence,
                value_boolean_list   = value_boolean  | flag_sequence,
                value_short_list     = value_short    | flag_sequence,
                value_int_list       = value_int      | flag_sequence,
                value_long_list      = value_long     | flag_sequence,
                value_double_list    = value_double   | flag_sequence,
                value_binary_list    = value_binary   | flag_sequence,

                mask_valuetype = mask_basetype | flag_sequence, // 0x1F

            // Node Type is stored in bits 6+7
                nodetype_shift_ = 6,
                mask_nodetype  = 0x03 << nodetype_shift_, // 0xC0

                // maybe we should use 0 for node type 'value'
                // to make the complete type be the valuetype ?
                nodetype_invalid= 0x00,

                nodetype_value  = 0x01 << nodetype_shift_, // 0x40
                nodetype_group  = 0x02 << nodetype_shift_, // 0x80
                nodetype_set    = 0x03 << nodetype_shift_, // 0xC0

                flag_innernode = 0x02 << nodetype_shift_, // 0x80
                flag_setnode   = 0x01 << nodetype_shift_  // 0x40
            };
        }

    //-----------------------------------------------------------------------------
    // node attributes
        namespace Flags
        {
            enum Type
            {
                readonly    = 0x01,
                finalized   = 0x02,

                nullable    = 0x04,
                localized   = 0x08,

                valueAvailable   = 0x10, // only used for value nodes
                defaultAvailable = 0x20, // only used for value nodes

                defaulted   = 0x40, // somewhat redundant with State
                defaultable = 0x80  // redundant with State (merged || defaulted)
            };
        }
    //-----------------------------------------------------------------------------
        // set element or tree state
        namespace State
        {
            enum Type
            {
                merged,     // = 0, - result of merging defaults + changes
                defaulted,  // = 1, - result of copying defaults unchanged
                replaced,   // = 2, - result of copying new layer unchanged
                added,      // = 3, - same as 'replaced', but it is known,
                                  //        that there is no corresponding default

                mask_state = 0x0F, // leaves bits 3+4 reserved for states

                flag_mandatory      = 0x10, // marks this element as non-removable/replacable
                flag_readonly       = 0x20, // marks this element as read-only
                flag_default_avail  = 0x40, // marks this element as containing default values
                flag_removable      = 0x80  // marks this element as removable
            };
        }
    //-----------------------------------------------------------------------------
    }
//-----------------------------------------------------------------------------
}

#endif // INCLUDED_DATA_FLAGS_HXX
