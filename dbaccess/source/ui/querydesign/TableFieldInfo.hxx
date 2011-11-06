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


#ifndef DBAUI_TABLEFIELDINFO_HXX
#define DBAUI_TABLEFIELDINFO_HXX

#ifndef DBAUI_ENUMTYPES_HXX
#include "QEnumTypes.hxx"
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

namespace dbaui
{
    class OTableFieldInfo
    {
    private:
        ETableFieldType m_eFieldType;
        sal_Int32       m_eDataType;

    public:
        OTableFieldInfo();
        ~OTableFieldInfo();

        inline ETableFieldType  GetKeyType() const { return m_eFieldType; }
        inline void             SetKey(ETableFieldType bKey=TAB_NORMAL_FIELD) { m_eFieldType = bKey; }
        inline sal_Int32        GetDataType() const { return m_eDataType; }
        inline void             SetDataType(sal_Int32 eTyp) { m_eDataType = eTyp; }
    };
}
#endif // DBAUI_TABLEFIELDINFO_HXX


