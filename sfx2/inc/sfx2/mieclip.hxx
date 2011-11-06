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



#ifndef _MIECLIP_HXX
#define _MIECLIP_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <tools/solar.h>
#include <tools/string.hxx>

class SvStream;
class SotDataObject;
class SvData;

class SFX2_DLLPUBLIC MSE40HTMLClipFormatObj
{
    SvStream* pStrm;
    String sBaseURL;

public:
    MSE40HTMLClipFormatObj() : pStrm(0)         {}
    ~MSE40HTMLClipFormatObj();

//JP 31.01.2001: old interfaces
    SAL_DLLPRIVATE sal_Bool GetData( SotDataObject& );
    SAL_DLLPRIVATE sal_Bool GetData( SvData& );
//JP 31.01.2001: the new one
    SvStream* IsValid( SvStream& );

    const SvStream* GetStream() const       { return pStrm; }
          SvStream* GetStream()             { return pStrm; }
    const String& GetBaseURL() const        { return sBaseURL; }
};


#endif //_MIECLIP_HXX

