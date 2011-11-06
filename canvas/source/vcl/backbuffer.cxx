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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_canvas.hxx"

#include "backbuffer.hxx"


namespace vclcanvas
{
    BackBuffer::BackBuffer( const OutputDevice& rRefDevice,
                            bool                bMonochromeBuffer ) :
        maVDev( new VirtualDevice( rRefDevice,
                                   bMonochromeBuffer ) )
    {
        if( !bMonochromeBuffer )
        {
            // #i95645#
#if defined( QUARTZ )
            // use AA on VCLCanvas for Mac
            maVDev->SetAntialiasing( ANTIALIASING_ENABLE_B2DDRAW | maVDev->GetAntialiasing() );
#else
            // switch off AA for WIN32 and UNIX, the VCLCanvas does not look good with it and
            // is not required to do AA. It would need to be adapted to use it correctly
            // (especially gradient painting). This will need extra work.
            maVDev->SetAntialiasing( maVDev->GetAntialiasing() & ~ANTIALIASING_ENABLE_B2DDRAW);
#endif
        }
    }

    OutputDevice& BackBuffer::getOutDev()
    {
        return maVDev.get();
    }

    const OutputDevice& BackBuffer::getOutDev() const
    {
        return maVDev.get();
    }

    void BackBuffer::setSize( const ::Size& rNewSize )
    {
        maVDev->SetOutputSizePixel( rNewSize );
    }

}
