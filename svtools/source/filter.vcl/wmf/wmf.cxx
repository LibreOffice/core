/*************************************************************************
 *
 *  $RCSfile: wmf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:59:00 $
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

#include "winmtf.hxx"
#include "emfwr.hxx"
#include "wmfwr.hxx"
#include "wmf.hxx"

// -----------------------------------------------------------------------------

BOOL ConvertWMFToGDIMetaFile( SvStream & rStreamWMF, GDIMetaFile & rGDIMetaFile,
                              PFilterCallback pCallback, void * pCallerData)
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
        if ( EnhWMFReader( rStreamWMF, rGDIMetaFile, pCallback, pCallerData ).ReadEnhWMF() == FALSE )
            rStreamWMF.SetError( SVSTREAM_FILEFORMAT_ERROR );
    }
    else
    {
        WMFReader( rStreamWMF, rGDIMetaFile, pCallback, pCallerData ).ReadWMF();
    }
    rStreamWMF.SetNumberFormatInt( nOrigNumberFormat );
    return !rStreamWMF.GetError();
}

// -----------------------------------------------------------------------------

BOOL ReadWindowMetafile( SvStream& rStream, GDIMetaFile& rMTF )
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
        if ( EnhWMFReader( rStream, rMTF, NULL, NULL ).ReadEnhWMF() == FALSE )
            rStream.SetError( SVSTREAM_FILEFORMAT_ERROR );
    }
    else
    {
        WMFReader( rStream, rMTF, NULL, NULL ).ReadWMF();
    }
    rStream.SetNumberFormatInt( nOrigNumberFormat );
    return !rStream.GetError();
}

// -----------------------------------------------------------------------------

BOOL ConvertGDIMetaFileToWMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              PFilterCallback pCallback, void * pCallerData,
                              BOOL bPlaceable)
{
    WMFWriter aWMFWriter;
    return aWMFWriter.WriteWMF(rMTF,rTargetStream,pCallback,pCallerData,bPlaceable);
}

// -----------------------------------------------------------------------------

BOOL ConvertGDIMetaFileToEMF( const GDIMetaFile & rMTF, SvStream & rTargetStream,
                              PFilterCallback pCallback, void * pCallerData )
{
    EMFWriter aEMFWriter;
    return aEMFWriter.WriteEMF( rMTF, rTargetStream, pCallback, pCallerData );
}

// -----------------------------------------------------------------------------

BOOL WriteWindowMetafile( SvStream& rStream, const GDIMetaFile& rMTF )
{
    return WMFWriter().WriteWMF( rMTF, rStream, NULL, NULL );
}

// -----------------------------------------------------------------------------

BOOL WriteWindowMetafileBits( SvStream& rStream, const GDIMetaFile& rMTF )
{
    return WMFWriter().WriteWMF( rMTF, rStream, NULL, NULL, FALSE );
}
