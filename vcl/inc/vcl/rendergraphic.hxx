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

#ifndef _SV_RENDERGRAPHIC_HXX
#define _SV_RENDERGRAPHIC_HXX

#include <vcl/dllapi.h>
#include <rtl/ustring.hxx>
#include <boost/shared_array.hpp>
#include <memory>

class SvStream;
class MapMode;
class Size;
class BitmapEx;

namespace vcl
{
    // -----------------
    // - RenderGraphic -
    // -----------------
    class VCL_DLLPUBLIC RenderGraphic
    {
    public:

        typedef boost::shared_array< sal_uInt8 > GraphicData;

        friend VCL_DLLPUBLIC ::SvStream& operator>>( ::SvStream& rIStm, RenderGraphic& rRenderGraphic );
        friend VCL_DLLPUBLIC ::SvStream& operator<<( ::SvStream& rOStm, const RenderGraphic& rRenderGraphic );

    public:

        RenderGraphic( );

        RenderGraphic( const RenderGraphic& rRenderGraphic );

        RenderGraphic( const GraphicData& rGraphicData,
                       sal_uInt32 nGraphicDataLength,
                       const rtl::OUString& rGraphicDataMimeType );

        RenderGraphic( const rtl::OUString& rGraphicDataMimeType,
                       sal_uInt32 nGraphicDataLength,
                       const sal_uInt8* pGraphicData = NULL );

        virtual ~RenderGraphic();

        RenderGraphic& operator=(const RenderGraphic& rRenderGraphic );

        bool operator==(const RenderGraphic& rRenderGraphic ) const;

        inline bool operator!=(const RenderGraphic& rRenderGraphic ) const
        {
            return( !( rRenderGraphic == *this ) );
        }

        bool IsEqual( const RenderGraphic& rRenderGraphic ) const;

        bool operator!() const;

        inline bool IsEmpty( ) const
        {
            return( !( *this ) );
        }

        inline const GraphicData& GetGraphicData( ) const
        {
            return( maGraphicData );
        }

        inline sal_uInt32 GetGraphicDataLength( ) const
        {
            return( mnGraphicDataLength );
        }

        inline const rtl::OUString& GetGraphicDataMimeType( ) const
        {
            return( maGraphicDataMimeType );
        }

        const MapMode& GetPrefMapMode() const;
        const Size& GetPrefSize() const;

        BitmapEx GetReplacement() const;

    private:

        void ImplCheckData();
        void ImplGetDefaults() const;

    private:

        GraphicData maGraphicData;
        sal_uInt32 mnGraphicDataLength;
        rtl::OUString maGraphicDataMimeType;
        mutable ::std::auto_ptr< MapMode > mapPrefMapMode;
        mutable ::std::auto_ptr< Size > mapPrefSize;
    };
}

#endif // _SV_RENDERHRAPHIC_HXX
