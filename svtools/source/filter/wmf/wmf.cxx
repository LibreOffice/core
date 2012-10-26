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
#include "precompiled_svtools.hxx"

#include "winmtf.hxx"
#include "emfwr.hxx"
#include "wmfwr.hxx"
#include <svtools/wmf.hxx>
#include <vcl/gdimetafiletools.hxx>

// -----------------------------------------------------------------------------

sal_Bool ConvertWMFToGDIMetaFile( SvStream & rStreamWMF, GDIMetaFile & rGDIMetaFile, FilterConfigItem* pConfigItem )
{
    sal_uInt32 nMetaType;
    sal_uInt32 nOrgPos = rStreamWMF.Tell();
    sal_uInt16 nOrigNumberFormat = rStreamWMF.GetNumberFormatInt();
    rStreamWMF.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStreamWMF.Seek( 0x28 );
    rStreamWMF >> nMetaType;
    rStreamWMF.Seek( nOrgPos );
    if ( nMetaType == 0x464d4520 )
    {
        if ( EnhWMFReader( rStreamWMF, rGDIMetaFile, pConfigItem ).ReadEnhWMF() == sal_False )
            rStreamWMF.SetError( SVSTREAM_FILEFORMAT_ERROR );
    }
    else
    {
        WMFReader( rStreamWMF, rGDIMetaFile, pConfigItem ).ReadWMF();
    }
    rStreamWMF.SetNumberFormatInt( nOrigNumberFormat );
    return !rStreamWMF.GetError();
}

// -----------------------------------------------------------------------------

sal_Bool ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF, FilterConfigItem* pFilterConfigItem )
{
    sal_uInt32 nMetaType;
    sal_uInt32 nOrgPos = rStream.Tell();
    sal_uInt16 nOrigNumberFormat = rStream.GetNumberFormatInt();
    rStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStream.Seek( 0x28 );
    rStream >> nMetaType;
    rStream.Seek( nOrgPos );
    if ( nMetaType == 0x464d4520 )
    {
        if ( EnhWMFReader( rStream, rMTF, NULL ).ReadEnhWMF() == sal_False )
            rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
    }
    else
    {
        WMFReader( rStream, rMTF, pFilterConfigItem ).ReadWMF();
    }
    rStream.SetNumberFormatInt( nOrigNumberFormat );
    return !rStream.GetError();
}

// -----------------------------------------------------------------------------

sal_Bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              FilterConfigItem* pConfigItem, sal_Bool bPlaceable)
{
    WMFWriter aWMFWriter;
    GDIMetaFile aGdiMetaFile(rMTF);

    if(usesClipActions(aGdiMetaFile))
    {
        // #121267# It is necessary to prepare the metafile since the export does *not* support
        // clip regions. This tooling method clips the geometry content of the metafile internally
        // against it's own clip regions, so that the export is safe to ignore clip regions
        clipMetafileContentAgainstOwnRegions(aGdiMetaFile);
    }

    return aWMFWriter.WriteWMF( aGdiMetaFile, rTargetStream, pConfigItem, bPlaceable );
}

// -----------------------------------------------------------------------------

sal_Bool ConvertGDIMetaFileToEMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              FilterConfigItem* pConfigItem )
{
    EMFWriter aEMFWriter;
    GDIMetaFile aGdiMetaFile(rMTF);

    if(usesClipActions(aGdiMetaFile))
    {
        // #121267# It is necessary to prepare the metafile since the export does *not* support
        // clip regions. This tooling method clips the geometry content of the metafile internally
        // against it's own clip regions, so that the export is safe to ignore clip regions
        clipMetafileContentAgainstOwnRegions(aGdiMetaFile);
    }

    return aEMFWriter.WriteEMF( aGdiMetaFile, rTargetStream, pConfigItem );
}

// -----------------------------------------------------------------------------

sal_Bool WriteWindowMetafile( SvStream& rStream, const GDIMetaFile& rMTF )
{
    return WMFWriter().WriteWMF( rMTF, rStream, NULL );
}

// -----------------------------------------------------------------------------

sal_Bool WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF )
{
    return WMFWriter().WriteWMF( rMTF, rStream, NULL, sal_False );
}
