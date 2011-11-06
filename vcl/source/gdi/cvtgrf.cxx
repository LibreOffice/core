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
#include "precompiled_vcl.hxx"

#include <vcl/metaact.hxx>
#include <vcl/cvtgrf.hxx>

#include <salinst.hxx>
#include <svdata.hxx>

// --------------
// - Callback   -
// --------------

// --------------------
// - GraphicConverter -
// --------------------

GraphicConverter::GraphicConverter() :
    mpConvertData( NULL )
{
}

// ------------------------------------------------------------------------

GraphicConverter::~GraphicConverter()
{
}

// ------------------------------------------------------------------------

sal_uLong GraphicConverter::ImplConvert( sal_uLong nInFormat, void* pInBuffer, sal_uLong nInBufSize,
                                     void** ppOutBuffer, sal_uLong nOutFormat )
{
    sal_uLong nRetBufSize = 0UL;

    if( ( nInFormat != nOutFormat ) && pInBuffer )
    {
        if( ( nInFormat == CVT_SVM ) || ( nInFormat == CVT_BMP ) )
        {
            SvMemoryStream  aIStm;
            Graphic         aGraphic;

            aIStm.SetBuffer( (char*) pInBuffer, nInBufSize, sal_False, nInBufSize );
            aIStm >> aGraphic;

            if( !aIStm.GetError() )
            {
                SvMemoryStream aOStm( 64535, 64535 );

                mpConvertData = new ConvertData( aGraphic, aOStm, nOutFormat );

                if( maFilterHdl.IsSet() && maFilterHdl.Call( mpConvertData ) )
                {
                    nRetBufSize = aOStm.Seek( STREAM_SEEK_TO_END );
                    *ppOutBuffer = (void*) aOStm.GetData();
                    aOStm.ObjectOwnsMemory( sal_False );
                }

                delete mpConvertData;
                mpConvertData = NULL;
            }
        }
        else if( ( nOutFormat == CVT_SVM ) || ( nOutFormat == CVT_BMP ) )
        {
            SvMemoryStream  aIStm;

            aIStm.SetBuffer( (char*) pInBuffer, nInBufSize, sal_False, nInBufSize );
            mpConvertData = new ConvertData( Graphic(), aIStm, nInFormat );

            if( maFilterHdl.IsSet() && maFilterHdl.Call( mpConvertData ) )
            {
                SvMemoryStream  aOStm( 645535, 64535 );
                Graphic&        rGraphic = mpConvertData->maGraphic;

                if( ( rGraphic.GetType() == GRAPHIC_BITMAP ) && ( CVT_SVM == nOutFormat ) )
                {
                    GDIMetaFile aMtf;

                    aMtf.SetPrefSize( rGraphic.GetPrefSize() );
                    aMtf.SetPrefMapMode( rGraphic.GetPrefMapMode() );
                    aMtf.AddAction( new MetaBmpExScaleAction( Point(), aMtf.GetPrefSize(), rGraphic.GetBitmapEx() ) );
                    rGraphic = aMtf;
                }
                else if( ( rGraphic.GetType() == GRAPHIC_GDIMETAFILE ) && ( CVT_BMP == nOutFormat ) )
                    rGraphic = rGraphic.GetBitmapEx();

                aOStm << rGraphic;

                if( !aOStm.GetError() )
                {
                    nRetBufSize = aOStm.Seek( STREAM_SEEK_TO_END );
                    *ppOutBuffer = (void*) aOStm.GetData();
                    aOStm.ObjectOwnsMemory( sal_False );
                }
            }

            delete mpConvertData;
            mpConvertData = NULL;
        }
    }

    return nRetBufSize;
}

// ------------------------------------------------------------------------

sal_uLong GraphicConverter::Import( SvStream& rIStm, Graphic& rGraphic, sal_uLong nFormat )
{
    GraphicConverter*   pCvt = ImplGetSVData()->maGDIData.mpGrfConverter;
    sal_uLong               nRet = ERRCODE_IO_GENERAL;

    if( pCvt && pCvt->GetFilterHdl().IsSet() )
    {
        ConvertData aData( rGraphic, rIStm, nFormat );

        if( pCvt->GetFilterHdl().Call( &aData ) )
        {
            rGraphic = aData.maGraphic;
            nRet = ERRCODE_NONE;
        }
        else if( rIStm.GetError() )
            nRet = rIStm.GetError();
    }

    return nRet;
}

// ------------------------------------------------------------------------

sal_uLong GraphicConverter::Export( SvStream& rOStm, const Graphic& rGraphic, sal_uLong nFormat )
{
    GraphicConverter*   pCvt = ImplGetSVData()->maGDIData.mpGrfConverter;
    sal_uLong               nRet = ERRCODE_IO_GENERAL;

    if( pCvt && pCvt->GetFilterHdl().IsSet() )
    {
        ConvertData aData( rGraphic, rOStm, nFormat );

        if( pCvt->GetFilterHdl().Call( &aData ) )
            nRet = ERRCODE_NONE;
        else if( rOStm.GetError() )
            nRet = rOStm.GetError();
    }

    return nRet;
}
