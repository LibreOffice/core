/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cvtgrf.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 11:59:14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_CVTGRF_HXX
#include <cvtgrf.hxx>
#endif

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

ULONG GraphicConverter::ImplConvert( ULONG nInFormat, void* pInBuffer, ULONG nInBufSize,
                                     void** ppOutBuffer, ULONG nOutFormat )
{
    ULONG nRetBufSize = 0UL;

    if( ( nInFormat != nOutFormat ) && pInBuffer )
    {
        if( ( nInFormat == CVT_SVM ) || ( nInFormat == CVT_BMP ) )
        {
            SvMemoryStream  aIStm;
            Graphic         aGraphic;

            aIStm.SetBuffer( (char*) pInBuffer, nInBufSize, FALSE, nInBufSize );
            aIStm >> aGraphic;

            if( !aIStm.GetError() )
            {
                SvMemoryStream aOStm( 64535, 64535 );

                mpConvertData = new ConvertData( aGraphic, aOStm, nOutFormat );

                if( maFilterHdl.IsSet() && maFilterHdl.Call( mpConvertData ) )
                {
                    nRetBufSize = aOStm.Seek( STREAM_SEEK_TO_END );
                    *ppOutBuffer = (void*) aOStm.GetData();
                    aOStm.ObjectOwnsMemory( FALSE );
                }

                delete mpConvertData;
                mpConvertData = NULL;
            }
        }
        else if( ( nOutFormat == CVT_SVM ) || ( nOutFormat == CVT_BMP ) )
        {
            SvMemoryStream  aIStm;

            aIStm.SetBuffer( (char*) pInBuffer, nInBufSize, FALSE, nInBufSize );
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
                    aOStm.ObjectOwnsMemory( FALSE );
                }
            }

            delete mpConvertData;
            mpConvertData = NULL;
        }
    }

    return nRetBufSize;
}

// ------------------------------------------------------------------------

ULONG GraphicConverter::Import( SvStream& rIStm, Graphic& rGraphic, ULONG nFormat )
{
    GraphicConverter*   pCvt = ImplGetSVData()->maGDIData.mpGrfConverter;
    ULONG               nRet = ERRCODE_IO_GENERAL;

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

ULONG GraphicConverter::Export( SvStream& rOStm, const Graphic& rGraphic, ULONG nFormat )
{
    GraphicConverter*   pCvt = ImplGetSVData()->maGDIData.mpGrfConverter;
    ULONG               nRet = ERRCODE_IO_GENERAL;

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
