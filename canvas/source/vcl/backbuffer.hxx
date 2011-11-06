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



#ifndef _VCLCANVAS_BACKBUFFER_HXX_
#define _VCLCANVAS_BACKBUFFER_HXX_

#include <vcl/virdev.hxx>

#include <canvas/vclwrapper.hxx>
#include "outdevprovider.hxx"

#include <boost/shared_ptr.hpp>


namespace vclcanvas
{
    /// Background buffer abstraction
    class BackBuffer : public OutDevProvider
    {
    public:
        /** Create a backbuffer for given reference device

            @param bMonochromeBuffer
            When false, default depth of reference device is
            chosen. When true, the buffer will be monochrome, i.e. one
            bit deep.
         */
        BackBuffer( const OutputDevice& rRefDevice,
                    bool                bMonochromeBuffer=false );

        virtual OutputDevice&       getOutDev();
        virtual const OutputDevice& getOutDev() const;

        void setSize( const ::Size& rNewSize );

    private:
        ::canvas::vcltools::VCLObject<VirtualDevice>    maVDev;
    };

    typedef ::boost::shared_ptr< BackBuffer > BackBufferSharedPtr;
}

#endif /* #ifndef _VCLCANVAS_BACKBUFFER_HXX_ */
