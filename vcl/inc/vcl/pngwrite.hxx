/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pngwrite.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 18:03:05 $
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

#ifndef _SV_PNGWRITE_HXX
#define _SV_PNGWRITE_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif
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
