/*************************************************************************
 *
 *  $RCSfile: cvtgrf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_CVTGRF_CXX

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

#ifndef REMOTE_APPSERVER

ULONG ImplFilterCallback( void* pInst,
                          ULONG nInFormat, void* pInBuffer, ULONG nInBufSize,
                          ULONG nOutFormat, void** ppOutBuffer )
{
    return( ( (GraphicConverter*) pInst )->ImplConvert( nInFormat,
                                                        pInBuffer, nInBufSize,
                                                        ppOutBuffer, nOutFormat ) );
}

#endif

// --------------------
// - GraphicConverter -
// --------------------

GraphicConverter::GraphicConverter() :
    mpConvertData( NULL )
{
#ifndef REMOTE_APPSERVER
    SetFilterCallback( ImplFilterCallback, this );
#endif
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
