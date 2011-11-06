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



#ifndef INCLUDED_BASEBMP_GENERICCOLORIMAGEACCESSOR_HXX
#define INCLUDED_BASEBMP_GENERICCOLORIMAGEACCESSOR_HXX

#include <basebmp/color.hxx>
#include <basebmp/bitmapdevice.hxx>

namespace basebmp
{
    /** Access a BitmapDevice generically

        This accessor deals with an opaque BitmapDevice generically,
        via getPixel()/setPixel() at the published interface.
     */
    class GenericColorImageAccessor
    {
        BitmapDeviceSharedPtr mpDevice;
        DrawMode              meDrawMode;

    public:
        typedef Color value_type;

        explicit GenericColorImageAccessor( BitmapDeviceSharedPtr const& rTarget ) :
            mpDevice(rTarget),
            meDrawMode(DrawMode_PAINT)
        {}

        GenericColorImageAccessor( BitmapDeviceSharedPtr const& rTarget,
                                   DrawMode                     eDrawMode ) :
            mpDevice(rTarget),
            meDrawMode(eDrawMode)
        {}

        template< typename Iterator >
        Color operator()( Iterator const& i ) const
        { return mpDevice->getPixel( basegfx::B2IPoint( i->x,i->y ) ); }

        template< typename Iterator, typename Difference >
        Color operator()( Iterator const& i, Difference const& diff) const
        { return mpDevice->getPixel( basegfx::B2IPoint( i[diff]->x,
                                                        i[diff]->y ) ); }

        template< typename Iterator >
        void set(Color const& value, Iterator const& i) const
        { return mpDevice->setPixel( basegfx::B2IPoint( i->x,i->y ),
                                     value, meDrawMode ); }

        template< class Iterator, class Difference >
        void set(value_type const& value, Iterator const& i, Difference const& diff) const
        { return mpDevice->setPixel( basegfx::B2IPoint( i[diff]->x,
                                                        i[diff]->y ),
                                     value, meDrawMode ); }
    };
}

#endif /* INCLUDED_BASEBMP_GENERICCOLORIMAGEACCESSOR_HXX */
