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



#ifndef INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBUFFERDEVICE_HXX
#define INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBUFFERDEVICE_HXX

#include <vcl/virdev.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx { class B2DRange; }

//////////////////////////////////////////////////////////////////////////////
// support methods for vcl direct gradient renderering

namespace drawinglayer
{
    class impBufferDevice
    {
        OutputDevice&                       mrOutDev;
        VirtualDevice                       maContent;
        VirtualDevice*                      mpMask;
        VirtualDevice*                      mpAlpha;
        Rectangle                           maDestPixel;

    public:
        impBufferDevice(
            OutputDevice& rOutDev,
            const basegfx::B2DRange& rRange,
            bool bAddOffsetToMapping);
        ~impBufferDevice();

        void paint(double fTrans = 0.0);
        bool isVisible() const { return !maDestPixel.IsEmpty(); }
        VirtualDevice& getContent() { return maContent; }
        VirtualDevice& getMask();
        VirtualDevice& getTransparence();
    };
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PROCESSOR2D_VCLHELPERBUFFERDEVICE_HXX

// eof
