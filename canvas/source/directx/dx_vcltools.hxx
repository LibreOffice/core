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



#ifndef _DXCANVAS_VCLTOOLS_HXX
#define _DXCANVAS_VCLTOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/util/TriState.hpp>

#include <boost/shared_ptr.hpp>


namespace com { namespace sun { namespace star { namespace lang
{
    class XUnoTunnel;
} } } }

namespace Gdiplus { class Graphics; }

namespace dxcanvas
{
    namespace tools
    {
        /** Raw RGBA bitmap data,
            contiguous in memory
        */
        struct RawRGBABitmap
        {
            sal_Int32                           mnWidth;
            sal_Int32                           mnHeight;
            ::boost::shared_ptr< sal_uInt8 >    mpBitmapData;
        };

        bool drawVCLBitmapFromXBitmap( const ::boost::shared_ptr< Gdiplus::Graphics >& rGraphics,
                                       const ::com::sun::star::uno::Reference<
                                             ::com::sun::star::rendering::XBitmap >&   xBitmap );
    }
}

#endif /* _DXCANVAS_VCLTOOLS_HXX */
