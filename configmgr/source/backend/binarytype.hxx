#ifndef CONFIGMGR_BINARYTYPE_HXX
#define CONFIGMGR_BINARYTYPE_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace configmgr
{
    namespace binary
    {
        const sal_uInt32 STR_ASCII_MASK = 0x80000000;
        const sal_Int16 CFG_BINARY_MAGIC = 10001;
        const sal_Int16 CFG_BINARY_VERSION = 2;

        namespace ValueFlags
        {
            enum Type
            {
                val_any,        // = 0
                val_string,     // = 1
                val_boolean,    // = 2
                val_int16,      // = 3
                val_int32,      // = 4
                val_int64,      // = 5
                val_double,     // = 6
                val_binary,     // = 7
                val_invalid,    // = 8

                //Sequence Flag
                seq = 0x10,

                //Pairstate Flags
                first_value_NULL = 0x20,
                second_value_NULL = 0x40,

                //Masks
                basictype_mask     = 0x0F,
                valuetype_mask     = 0x1F,
                pairstate_mask     = first_value_NULL | second_value_NULL
            };
            typedef sal_uInt8 Field;
        }

        namespace NodeType
        {
            enum Type
            {
                //Node Type Identifiers
                stop          = 0,
                valuenode     = 0x20,
                groupnode     = 0x40,
                setnode       = 0x80,
                component     = 0x60,
                templates     = 0xA0,
                nodata        = 0xC0,
                invalid       = 0xE0,

                nodetype_mask = 0xE0
            };
            typedef sal_uInt8 Field;
        }
    }
}
#endif
