/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _REGISTRY_REFLTYPE_HXX_
#define _REGISTRY_REFLTYPE_HXX_

#include "registry/types.h"
#include <sal/types.h>

/** specifies the type source of a binary type blob.

    Currently only RT_UNO_IDL type is used.
 */
enum RTTypeSource
{
    RT_UNO_IDL,
    RT_CORBA_IDL,
    RT_JAVA
};

/** specifies a helper class for const values.

    This class is used for easy handling of constants or enum values
    as fields in binary type blob.
 */
class RTConstValue
{
public:
    /// stores the type of the constant value.
    RTValueType         m_type;
    /// stores the value of the constant.
    RTConstValueUnion m_value;

    /// Default constructor.
    RTConstValue()
        : m_type(RT_TYPE_NONE)
    {
        m_value.aDouble = 0.0;
    }

    /// Destructor
    ~RTConstValue() {}
};

/** deprecated.

    An earlier version of UNO used an unique identifier for interfaces. In the
    current version of UNO this uik was eliminated and this type is not longer used.
 */
struct RTUik
{
    sal_uInt32 m_Data1;
    sal_uInt16 m_Data2;
    sal_uInt16 m_Data3;
    sal_uInt32 m_Data4;
    sal_uInt32 m_Data5;
};

/// specifies the calling onvention for type reader/wrter api
#define TYPEREG_CALLTYPE    SAL_CALL

#endif
