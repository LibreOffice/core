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



#ifndef _SD_SDIOCMPT_HXX
#define _SD_SDIOCMPT_HXX

#include <tools/stream.hxx>

//////////////////////////////////////////////////////////////////////////////
class SvStream;

class old_SdrDownCompat
{
protected:
    SvStream&                   rStream;
    sal_uInt32                      nSubRecSiz;
    sal_uInt32                      nSubRecPos;
    sal_uInt16                      nMode;
    bool                        bOpen;

protected:
    void Read();
    void Write();

public:
    old_SdrDownCompat(SvStream& rNewStream, sal_uInt16 nNewMode);
    ~old_SdrDownCompat();
    void  OpenSubRecord();
    void  CloseSubRecord();
};
//////////////////////////////////////////////////////////////////////////////
#include "sddllapi.h"

#define SDIOCOMPAT_VERSIONDONTKNOW (sal_uInt16)0xffff

class SD_DLLPUBLIC SdIOCompat : public old_SdrDownCompat
{
private:
    sal_uInt16 nVersion;

public:
                // nNewMode: STREAM_READ oder STREAM_WRITE
                // nVer:     nur beim Schreiben angeben
            SdIOCompat(SvStream& rNewStream, sal_uInt16 nNewMode,
                       sal_uInt16 nVer = SDIOCOMPAT_VERSIONDONTKNOW);
            ~SdIOCompat();
    sal_uInt16  GetVersion() const { return nVersion; }
};

#endif      // _SD_SDIOCMPT_HXX


