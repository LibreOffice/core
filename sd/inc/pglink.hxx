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



#ifndef _PGLINK_HXX
#define _PGLINK_HXX

#ifndef SVX_LIGHT

#include <sfx2/lnkbase.hxx>

class SdPage;



class SdPageLink : public ::sfx2::SvBaseLink
{
    SdPage* pPage;

public:
    SdPageLink(SdPage* pPg, const String& rFileName, const String& rBookmarkName);
    virtual ~SdPageLink();

    virtual void Closed();
    virtual void DataChanged( const String& ,
                              const ::com::sun::star::uno::Any & );
    bool Connect() { return 0 != SvBaseLink::GetRealObject(); }
};

#else

class SdPageLink;

#endif

#endif     // _PGLINK_HXX




