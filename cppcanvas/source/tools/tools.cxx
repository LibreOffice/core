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

#include <tools.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace tools
    {
        uno::Sequence< double > intSRGBAToDoubleSequence( const uno::Reference< rendering::XGraphicDevice >&,
                                                          Color::IntSRGBA                                   aColor  )
        {
            uno::Sequence< double > aRes( 4 );

            aRes[0] = getRed(aColor) / 255.0;
            aRes[1] = getGreen(aColor) / 255.0;
            aRes[2] = getBlue(aColor) / 255.0;
            aRes[3] = getAlpha(aColor) / 255.0;

            return aRes;
        }

        Color::IntSRGBA doubleSequenceToIntSRGBA( const uno::Reference< rendering::XGraphicDevice >&,
                                                  const uno::Sequence< double >&                    rColor  )
        {
            return makeColor( static_cast<sal_uInt8>( 255*rColor[0] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[1] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[2] + .5 ),
                              static_cast<sal_uInt8>( 255*rColor[3] + .5 ) );
        }
    }
}
