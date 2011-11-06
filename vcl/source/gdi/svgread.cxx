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
#include <tools/stream.hxx>
#include <vcl/svgread.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metaact.hxx>
#include <vcl/rendergraphicrasterizer.hxx>

// -----------
// - Defines -
// -----------

namespace vcl
{
    // -----------------
    // - SVGReaderImpl -
    // -----------------

    class SVGReaderImpl
    {
    public:

        SVGReaderImpl( SvStream& rStm );
        ~SVGReaderImpl();

        GDIMetaFile& ImplRead( GDIMetaFile& rSVGMtf );

    private:

        SvStream&           mrStm;
    };

    // ------------------------------------------------------------------------------

    SVGReaderImpl::SVGReaderImpl( SvStream& rStm ) :
        mrStm( rStm )
    {
    }

    // ------------------------------------------------------------------------

    SVGReaderImpl::~SVGReaderImpl()
    {
    }

    // ------------------------------------------------------------------------

    GDIMetaFile& SVGReaderImpl::ImplRead( GDIMetaFile& rSVGMtf )
    {
        const sal_uInt32 nStmPos = mrStm.Tell();
        const sal_uInt32 nStmLen = mrStm.Seek( STREAM_SEEK_TO_END ) - nStmPos;

        if( nStmLen )
        {
            const vcl::RenderGraphic aSVGGraphic( ::rtl::OUString::createFromAscii( "image/svg+xml" ), nStmLen );

            mrStm.Seek( nStmPos );
            mrStm.Read( aSVGGraphic.GetGraphicData().get(), nStmLen );

            if( !mrStm.GetError() )
            {
                const vcl::RenderGraphicRasterizer  aRasterizer( aSVGGraphic );
                const Size                          aDefaultSizePixel( aRasterizer.GetDefaultSizePixel() );

                if( aDefaultSizePixel.Width() && aDefaultSizePixel.Height() )
                {
                    const Point aPos;
                    const Size  aPrefSize( aRasterizer.GetPrefSize() );

                    rSVGMtf.SetPrefMapMode( aRasterizer.GetPrefMapMode() );
                    rSVGMtf.SetPrefSize( aPrefSize );
                    rSVGMtf.AddAction( new MetaRenderGraphicAction( aPos, aPrefSize, aSVGGraphic ) );
                    rSVGMtf.WindStart();
                }
            }
        }

        return( rSVGMtf );
    }

    // -------------
    // - SVGReader -
    // -------------

    SVGReader::SVGReader( SvStream& rIStm ) :
        mapImpl( new ::vcl::SVGReaderImpl( rIStm ) )
    {
    }

    // ------------------------------------------------------------------------

    SVGReader::~SVGReader()
    {
    }

    // ------------------------------------------------------------------------

    GDIMetaFile& SVGReader::Read( GDIMetaFile& rSVGMtf )
    {
        rSVGMtf = GDIMetaFile();

        return( mapImpl.get() ? mapImpl->ImplRead( rSVGMtf ) : rSVGMtf );
    }

} // namespace vcl
