/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "winmtf.hxx"
#include "emfwr.hxx"
#include "wmfwr.hxx"
#include <vcl/wmf.hxx>
#include <vcl/gdimetafiletools.hxx>
#include <comphelper/scopeguard.hxx>

#include <boost/bind.hpp>

bool ConvertWMFToGDIMetaFile( SvStream & rStreamWMF, GDIMetaFile & rGDIMetaFile, FilterConfigItem* pConfigItem, WMF_EXTERNALHEADER *pExtHeader )
{
    sal_uInt32 nMetaType;
    sal_uInt32 nOrgPos = rStreamWMF.Tell();
    SvStreamEndian nOrigNumberFormat = rStreamWMF.GetEndian();
    rStreamWMF.SetEndian( SvStreamEndian::LITTLE );
    rStreamWMF.Seek( 0x28 );
    rStreamWMF.ReadUInt32( nMetaType );
    rStreamWMF.Seek( nOrgPos );
    if ( nMetaType == 0x464d4520 )
    {
        if ( !EnhWMFReader( rStreamWMF, rGDIMetaFile, pConfigItem ).ReadEnhWMF() )
            rStreamWMF.SetError( SVSTREAM_FILEFORMAT_ERROR );
    }
    else
    {
        WMFReader( rStreamWMF, rGDIMetaFile, pConfigItem, pExtHeader ).ReadWMF( );
    }

#ifdef DBG_UTIL
    // #i123216# allow a look at CheckSum and ByteSize for debugging
    SAL_INFO("vcl.emf", "\t\t\tchecksum: 0x" << std::hex << rGDIMetaFile.GetChecksum() << std::dec);
    SAL_INFO("vcl.emf", "\t\t\tsize: " << rGDIMetaFile.GetSizeBytes());
#endif

    rStreamWMF.SetEndian( nOrigNumberFormat );
    return !rStreamWMF.GetError();
}

bool ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF, FilterConfigItem* pFilterConfigItem )
{
    sal_uInt32 nMetaType(0);
    sal_uInt32 nOrgPos = rStream.Tell();

    SvStreamEndian nOrigNumberFormat = rStream.GetEndian();
    rStream.SetEndian( SvStreamEndian::LITTLE );
    //exception-safe reset nOrigNumberFormat at end of scope
    const ::comphelper::ScopeGuard aScopeGuard(
        boost::bind(&SvStream::SetEndian, ::boost::ref(rStream),
          nOrigNumberFormat));

    rStream.Seek( 0x28 );
    rStream.ReadUInt32( nMetaType );
    rStream.Seek( nOrgPos );

    if (!rStream.good())
        return false;

    if ( nMetaType == 0x464d4520 )
    {
        if ( !EnhWMFReader( rStream, rMTF, nullptr ).ReadEnhWMF() )
            rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
    }
    else
    {
        WMFReader( rStream, rMTF, pFilterConfigItem ).ReadWMF();
    }

    return rStream.good();
}

bool ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              FilterConfigItem* pConfigItem, bool bPlaceable)
{
    WMFWriter aWMFWriter;
    GDIMetaFile aGdiMetaFile(rMTF);

    if(usesClipActions(aGdiMetaFile))
    {
        // #i121267# It is necessary to prepare the metafile since the export does *not* support
        // clip regions. This tooling method clips the geometry content of the metafile internally
        // against it's own clip regions, so that the export is safe to ignore clip regions
        clipMetafileContentAgainstOwnRegions(aGdiMetaFile);
    }

    return aWMFWriter.WriteWMF( aGdiMetaFile, rTargetStream, pConfigItem, bPlaceable );
}

bool ConvertGDIMetaFileToEMF(const GDIMetaFile & rMTF, SvStream & rTargetStream)
{
    EMFWriter aEMFWriter(rTargetStream);
    GDIMetaFile aGdiMetaFile(rMTF);

    if(usesClipActions(aGdiMetaFile))
    {
        // #i121267# It is necessary to prepare the metafile since the export does *not* support
        // clip regions. This tooling method clips the geometry content of the metafile internally
        // against it's own clip regions, so that the export is safe to ignore clip regions
        clipMetafileContentAgainstOwnRegions(aGdiMetaFile);
    }

    return aEMFWriter.WriteEMF(aGdiMetaFile);
}

bool WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF )
{
    return WMFWriter().WriteWMF( rMTF, rStream, nullptr, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
