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
#include "precompiled_cppcanvas.hxx"

#include <implcolor.hxx>
#include <tools.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {

        ImplColor::ImplColor( const uno::Reference< rendering::XGraphicDevice >& rDevice ) :
            mxDevice( rDevice )
        {
            OSL_ENSURE( mxDevice.is(), "ImplColor::ImplColor(): Invalid graphic device" );
        }

        ImplColor::~ImplColor()
        {
        }

        Color::IntSRGBA ImplColor::getIntSRGBA( uno::Sequence< double >& rDeviceColor ) const
        {
            OSL_ENSURE( mxDevice.is(), "ImplColor::getIntSRGBA(): Invalid graphic device" );

            // TODO(F1): Color space handling
            return tools::doubleSequenceToIntSRGBA( mxDevice, rDeviceColor );
        }

        uno::Sequence< double > ImplColor::getDeviceColor( Color::IntSRGBA aSRGBA ) const
        {
            OSL_ENSURE( mxDevice.is(), "ImplColor::getDeviceColor(): Invalid graphic device" );

            // TODO(F1): Color space handling
            return tools::intSRGBAToDoubleSequence( mxDevice, aSRGBA );
        }

    }
}
