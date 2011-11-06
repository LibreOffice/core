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



#ifndef _SDR_CONTACT_OBJECTCONTACTTOOLS_HXX
#define _SDR_CONTACT_OBJECTCONTACTTOOLS_HXX

#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////
// predeclarations

class OutputDevice;

namespace drawinglayer { namespace processor2d {
    class BaseProcessor2D;
}}

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace contact
    {
        // create a mating VCL-Provessor for given OutputDevice. This includes
        // looking for MetaFile-recording. The returned renderer changes owner,
        // deletion is duty of the caller
        drawinglayer::processor2d::BaseProcessor2D* createBaseProcessor2DFromOutputDevice(
            OutputDevice& rTargetOutDev,
            const drawinglayer::geometry::ViewInformation2D& rViewInformation2D);

    } // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_CONTACT_OBJECTCONTACTTOOLS_HXX

// eof
