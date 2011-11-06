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

#include <implfont.hxx>
#include <canvas/canvastools.hxx>


using namespace ::com::sun::star;

/* Definition of Font class */

namespace cppcanvas
{
    namespace internal
    {

        ImplFont::ImplFont( const uno::Reference< rendering::XCanvas >& rCanvas,
                            const ::rtl::OUString& rFontName,
                            const double& rCellSize ) :
            mxCanvas( rCanvas ),
            mxFont( NULL )
        {
            OSL_ENSURE( mxCanvas.is(), "ImplFont::ImplFont(): Invalid Canvas" );

            rendering::FontRequest aFontRequest;
            aFontRequest.FontDescription.FamilyName = rFontName;
            aFontRequest.CellSize = rCellSize;

            geometry::Matrix2D aFontMatrix;
            ::canvas::tools::setIdentityMatrix2D( aFontMatrix );

            mxFont = mxCanvas->createFont( aFontRequest,
                                           uno::Sequence< beans::PropertyValue >(),
                                           aFontMatrix );
        }


        ImplFont::~ImplFont()
        {
        }

        ::rtl::OUString ImplFont::getName() const
        {
            OSL_ENSURE( mxFont.is(), "ImplFont::getName(): Invalid Font" );

            return mxFont->getFontRequest().FontDescription.FamilyName;
        }

        double ImplFont::getCellSize() const
        {
            OSL_ENSURE( mxFont.is(), "ImplFont::getCellSize(): Invalid Font" );

            return mxFont->getFontRequest().CellSize;
        }

        uno::Reference< rendering::XCanvasFont > ImplFont::getUNOFont() const
        {
            OSL_ENSURE( mxFont.is(), "ImplFont::getUNOFont(): Invalid Font" );

            return mxFont;
        }

    }
}
