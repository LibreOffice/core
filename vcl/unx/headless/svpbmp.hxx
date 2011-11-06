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



#ifndef SVP_SVBMP_HXX
#define SVP_SVBMP_HXX

#include <salbmp.hxx>
#include "svpelement.hxx"

class SvpSalBitmap : public SalBitmap, public SvpElement
{
    basebmp::BitmapDeviceSharedPtr     m_aBitmap;
public:
    SvpSalBitmap() {}
    virtual ~SvpSalBitmap();

    const basebmp::BitmapDeviceSharedPtr& getBitmap() const { return m_aBitmap; }
    void setBitmap( const basebmp::BitmapDeviceSharedPtr& rSrc ) { m_aBitmap = rSrc; }

    // SvpElement
    virtual const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aBitmap; }

    // SalBitmap
    virtual bool            Create( const Size& rSize,
                                    sal_uInt16 nBitCount,
                                    const BitmapPalette& rPal );
    virtual bool            Create( const SalBitmap& rSalBmp );
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    SalGraphics* pGraphics );
    virtual bool            Create( const SalBitmap& rSalBmp,
                                    sal_uInt16 nNewBitCount );
    virtual void            Destroy();
    virtual Size            GetSize() const;
    virtual sal_uInt16              GetBitCount() const;

    virtual BitmapBuffer*   AcquireBuffer( bool bReadOnly );
    virtual void            ReleaseBuffer( BitmapBuffer* pBuffer, bool bReadOnly );
    virtual bool            GetSystemData( BitmapSystemData& rData );

};

#endif
