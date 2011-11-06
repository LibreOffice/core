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



#ifndef _VCLCANVAS_OUTDEVPROVIDER_HXX
#define _VCLCANVAS_OUTDEVPROVIDER_HXX

#include <sal/types.h>
#include <boost/shared_ptr.hpp>


class OutputDevice;

namespace vclcanvas
{
    /* Definition of OutDevProvider interface */

    /** Implementers of this interface provide the CanvasHelper
        with its OutputDevice.

        This additional level of indirection was necessary, as the
        OutputDevice is not an interface. There had to be a mechanism
        to detect the moment when an OutputDevice is rendered to
        (e.g. for the BitmapBackBuffer).
     */
    class OutDevProvider
    {
    public:
        virtual ~OutDevProvider() {}

        virtual OutputDevice&       getOutDev() = 0;
        virtual const OutputDevice& getOutDev() const = 0;
    };

    typedef ::boost::shared_ptr< OutDevProvider > OutDevProviderSharedPtr;
}

#endif /* _VCLCANVAS_OUTDEVPROVIDER_HXX */
