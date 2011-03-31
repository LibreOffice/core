/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SV_SVGREAD_HXX
#define _SV_SVGREAD_HXX

#include <vcl/dllapi.h>
#include <memory>

// -------------
// - SVGReader -
// -------------

class SvStream;
class GDIMetaFile;

namespace vcl
{
    class SVGReaderImpl;

    class VCL_DLLPUBLIC SVGReader
    {
    public:

        SVGReader( SvStream& rStm );
        ~SVGReader();

        /* rSVGMtf is an output parameter, that is also returned
          by the method as well.

          In case of a failure, the GDIMetaFile will not contain
          any actions at all and thus GDIMetaFile::GetActionCount
          will return 0.

         In case of success, the GDIMetaFile will contain a
         META_RENDERGRAPHIC_ACTION, containing the SVG raw data. The
         data can be rendered by using the standard ways to output a
         GDIMetaFile, e.g. calling GDIMetaFile::Play

        */
        GDIMetaFile& Read( GDIMetaFile& rSVGMtf );

    private:

        SVGReader();
        SVGReader( const SVGReader& rReader );

        inline SVGReader& operator=( const SVGReader& /* rReader */ ) { return( *this ); }

    private:

        std::auto_ptr< SVGReaderImpl > mapImpl;
    };
}

#endif // _SV_SVGREAD_HXX
