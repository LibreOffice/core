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



#ifndef _SV_SALBMP_HXX
#define _SV_SALBMP_HXX

#ifndef _TL_GEN_HXX
#include <tools/gen.hxx>
#endif
#include <vcl/dllapi.h>

struct BitmapBuffer;
class SalGraphics;
class BitmapPalette;
struct BitmapSystemData;

class VCL_PLUGIN_PUBLIC SalBitmap
{
public:
    SalBitmap() {}
    virtual ~SalBitmap();

    virtual bool            Create( const Size& rSize,
                                    sal_uInt16 nBitCount,
                                    const BitmapPalette& rPal ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics ) = 0;
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    sal_uInt16 nNewBitCount ) = 0;
    virtual void            Destroy() = 0;
    virtual Size            GetSize() const = 0;
    virtual sal_uInt16          GetBitCount() const = 0;

    virtual BitmapBuffer*   AcquireBuffer( bool bReadOnly ) = 0;
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly ) = 0;
    virtual bool            GetSystemData( BitmapSystemData& rData ) = 0;

};

#endif
