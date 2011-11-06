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



#ifndef _SVP_SVPVD_HXX
#define _SVP_SVPVD_HXX

#include <salvd.hxx>
#include "svpelement.hxx"

#include <list>

class SvpSalGraphics;

class SvpSalVirtualDevice : public SalVirtualDevice, public SvpElement
{
    sal_uInt16                          m_nBitCount;
    basebmp::BitmapDeviceSharedPtr      m_aDevice;
    std::list< SvpSalGraphics* >        m_aGraphics;

public:
    SvpSalVirtualDevice( sal_uInt16 nBitCount ) : SvpElement(), m_nBitCount(nBitCount) {}
    virtual ~SvpSalVirtualDevice();

    // SvpElement
    virtual const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aDevice; }

    // SalVirtualDevice
    virtual SalGraphics*    GetGraphics();
    virtual void            ReleaseGraphics( SalGraphics* pGraphics );

    virtual sal_Bool            SetSize( long nNewDX, long nNewDY );
    virtual void            GetSize( long& rWidth, long& rHeight );
};

#endif // _SVP_SVPVD_HXX
