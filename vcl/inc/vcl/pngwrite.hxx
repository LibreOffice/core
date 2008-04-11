/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pngwrite.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_PNGWRITE_HXX
#define _SV_PNGWRITE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/dllapi.h>
#include <vcl/bitmapex.hxx>
#include <vector>

// -------------
// - PNGWriter -
// -------------

namespace vcl
{
    class PNGWriterImpl;

    class VCL_DLLPUBLIC PNGWriter
    {
        PNGWriterImpl*          mpImpl;

    public:

        PNGWriter( const BitmapEx& BmpEx,
            const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData = NULL );
        ~PNGWriter();

        sal_Bool Write( SvStream& rStm );

        // additional method to be able to modify all chunk before they are stored
        struct ChunkData
        {
            sal_uInt32                  nType;
            std::vector< sal_uInt8 >    aData;
        };
        std::vector< vcl::PNGWriter::ChunkData >& GetChunks();
    };
}

#endif // _SV_PNGWRITE_HXX
