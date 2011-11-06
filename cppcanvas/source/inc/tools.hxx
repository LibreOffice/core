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



#ifndef _CPPCANVAS_TOOLS_HXX
#define _CPPCANVAS_TOOLS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <cppcanvas/color.hxx>

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XGraphicDevice;
} } } }


namespace cppcanvas
{
    namespace tools
    {
        ::com::sun::star::uno::Sequence< double >
            intSRGBAToDoubleSequence( const ::com::sun::star::uno::Reference<
                                              ::com::sun::star::rendering::XGraphicDevice >&,
                                      Color::IntSRGBA );

        Color::IntSRGBA doubleSequenceToIntSRGBA( const ::com::sun::star::uno::Reference<
                                                          ::com::sun::star::rendering::XGraphicDevice >&    rDevice,
                                                  const ::com::sun::star::uno::Sequence< double >&              rColor  );
    }
}

#endif /* _CPPCANVAS_TOOLS_HXX */
