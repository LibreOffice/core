/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove


#include "winmtf.hxx"
#include "emfwr.hxx"
#include "wmfwr.hxx"
#include <bf_svtools/wmf.hxx>

namespace binfilter
{

// -----------------------------------------------------------------------------

BOOL ConvertWMFToGDIMetaFile( SvStream & rStreamWMF, GDIMetaFile & rGDIMetaFile, FilterConfigItem* pConfigItem )
{
    UINT32 nMetaType;
    UINT32 nOrgPos = rStreamWMF.Tell();
    UINT16 nOrigNumberFormat = rStreamWMF.GetNumberFormatInt();
    rStreamWMF.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStreamWMF.Seek( 0x28 );
    rStreamWMF >> nMetaType;
    rStreamWMF.Seek( nOrgPos );
    if ( nMetaType == 0x464d4520 )
    {
        if ( EnhWMFReader( rStreamWMF, rGDIMetaFile, pConfigItem ).ReadEnhWMF() == FALSE )
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

BOOL ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF, FilterConfigItem* pFilterConfigItem )
{
    UINT32 nMetaType;
    UINT32 nOrgPos = rStream.Tell();
    UINT16 nOrigNumberFormat = rStream.GetNumberFormatInt();
    rStream.SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
    rStream.Seek( 0x28 );
    rStream >> nMetaType;
    rStream.Seek( nOrgPos );
    if ( nMetaType == 0x464d4520 )
    {
        if ( EnhWMFReader( rStream, rMTF, NULL ).ReadEnhWMF() == FALSE )
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

BOOL ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              FilterConfigItem* pConfigItem, BOOL bPlaceable)
{
    WMFWriter aWMFWriter;
    return aWMFWriter.WriteWMF( rMTF, rTargetStream, pConfigItem, bPlaceable );
}

// -----------------------------------------------------------------------------

BOOL ConvertGDIMetaFileToEMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              FilterConfigItem* pConfigItem )
{
    EMFWriter aEMFWriter;
    return aEMFWriter.WriteEMF( rMTF, rTargetStream, pConfigItem );
}

// -----------------------------------------------------------------------------

BOOL WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF )
{
    return WMFWriter().WriteWMF( rMTF, rStream, NULL, FALSE );
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
