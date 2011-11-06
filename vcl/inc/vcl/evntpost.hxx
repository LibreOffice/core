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


#ifndef _VCL_EVNTPOST_HXX
#define _VCL_EVNTPOST_HXX

#include <tools/link.hxx>
#include <vcl/dllapi.h>

//===================================================================

namespace vcl
{
    struct UserEvent
    {
        sal_uLong           m_nWhich;
        void*           m_pData;
    };

    class VCL_DLLPUBLIC EventPoster
    {
        sal_uLong           m_nId;
        Link            m_aLink;

//#if 0 // _SOLAR__PRIVATE
        DECL_DLLPRIVATE_LINK( DoEvent_Impl, UserEvent* );
//#endif

    public:
                        EventPoster( const Link& rLink );
                        ~EventPoster();
        void            Post( UserEvent* pEvent );
    };
}

#endif
