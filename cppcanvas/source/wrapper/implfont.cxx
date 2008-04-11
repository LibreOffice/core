/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: implfont.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
