/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implfont.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:28:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
