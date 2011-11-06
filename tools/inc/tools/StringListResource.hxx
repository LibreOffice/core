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


#ifndef TOOLS_STRINGLISTRESOURCE_HXX
#define TOOLS_STRINGLISTRESOURCE_HXX

#include <vector>
#include <tools/resid.hxx>
#include <tools/rcid.h>
#include <tools/rc.hxx>

namespace tools
{
    class StringListResource : public Resource
    {
    public:
        StringListResource(const ResId& _aResId,::std::vector< ::rtl::OUString>& _rToFill ) : Resource(_aResId)
        {
            sal_uInt16 i = 1;
            while( IsAvailableRes(ResId(i,*m_pResMgr).SetRT(RSC_STRING)) )
            {
                String sStr = String(ResId(i,*m_pResMgr));
                _rToFill.push_back(sStr);
                ++i;
            }
        }
        ~StringListResource()
        {
            FreeResource();
        }
    };
} // namespace tools
#endif // TOOLS_STRINGLISTRESOURCE_HXX
