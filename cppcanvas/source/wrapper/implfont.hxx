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



#ifndef _CPPCANVAS_IMPLFONT_HXX
#define _CPPCANVAS_IMPLFONT_HXX

#include <com/sun/star/uno/Reference.hxx>

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _COM_SUN_STAR_RENDERING_XCANVAS_HPP__
#include <com/sun/star/rendering/XCanvas.hpp>
#endif
#include <cppcanvas/font.hxx>

namespace rtl
{
    class OUString;
}

namespace com { namespace sun { namespace star { namespace rendering
{
    class  XCanvasFont;
} } } }

/* Definition of Font class */

namespace cppcanvas
{

    namespace internal
    {

        class ImplFont : public Font
        {
        public:
            ImplFont( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::rendering::XCanvas >& rCanvas,
                      const ::rtl::OUString& rFontName,
                      const double& rCellSize );

            virtual ~ImplFont();

            virtual ::rtl::OUString getName() const;
            virtual double          getCellSize() const;

            virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvasFont > getUNOFont() const;

        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >        mxCanvas;
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvasFont >    mxFont;
        };
    }
}

#endif /* _CPPCANVAS_IMPLFONT_HXX */
