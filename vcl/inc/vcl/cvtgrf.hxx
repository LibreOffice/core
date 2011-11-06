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



#ifndef _SV_CVTGRF_HXX
#define _SV_CVTGRF_HXX

#include <vcl/dllapi.h>
#include <tools/link.hxx>
#include <vcl/salctype.hxx>

// --------------------
// - GraphicConverter -
// --------------------

struct  ConvertData;
class   Graphic;

class VCL_DLLPUBLIC GraphicConverter
{
private:

    Link                maFilterHdl;
    ConvertData*        mpConvertData;

//#if 0 // _SOLAR__PRIVATE
public:
    SAL_DLLPRIVATE sal_uLong    ImplConvert( sal_uLong nInFormat, void* pInBuffer, sal_uLong nInBufSize,
                                     void** ppOutBuffer, sal_uLong nOutFormat );
//#endif // __PRIVATE

public:

                        GraphicConverter();
                        ~GraphicConverter();

    static sal_uLong        Import( SvStream& rIStm, Graphic& rGraphic, sal_uLong nFormat = CVT_UNKNOWN );
    static sal_uLong        Export( SvStream& rOStm, const Graphic& rGraphic, sal_uLong nFormat );

    ConvertData*        GetConvertData() { return mpConvertData; }

    void                SetFilterHdl( const Link& rLink ) { maFilterHdl = rLink; }
    const Link&         GetFilterHdl() const { return maFilterHdl; }
};

#endif // _SV_CVTGRF_HXX

