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



#ifndef _SD_PPTWRP_HXX
#define _SD_PPTWRP_HXX

#include "sdfilter.hxx"

// ---------------
// - SdPPTFilter -
// ---------------

class SdPPTFilter : public SdFilter
{
public:
    SdPPTFilter (
        SfxMedium& rMedium,
        ::sd::DrawDocShell& rDocShell,
        sal_Bool bShowProgress);
    virtual ~SdPPTFilter (void);

    sal_Bool        Import();
    sal_Bool        Export();

    // these methods are necassary for the export to PowerPoint
    void                    PreSaveBasic();         // restaures the original basic storage

private:
    SvMemoryStream*         pBas;
};

#endif // _SD_PPTWRP_HXX
