/*************************************************************************
 *
 *  $RCSfile: jobdata.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pl $ $Date: 2002-06-19 10:53:34 $
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

#include <psprint/jobdata.hxx>
#include <psprint/printerinfomanager.hxx>
#include <tools/stream.hxx>

#ifdef SOLARIS
#include <alloca.h>
#else
#include <stdlib.h>
#endif

using namespace psp;
using namespace rtl;

bool JobData::getStreamBuffer( void*& pData, int& bytes )
{
    // consistency checks
    if( ! m_pParser )
        m_pParser = m_aContext.getParser();
    if( m_pParser != m_aContext.getParser() ||
        ! m_pParser )
        return false;

    SvMemoryStream aStream;
    ByteString aLine;

    // write header job data
    aStream.WriteLine( "JobData 1" );

    aLine = "printer=";
    aLine += ByteString( String( m_aPrinterName ), RTL_TEXTENCODING_UTF8 );
    aStream.WriteLine( aLine );

    aLine = "orientation=";
    aLine += m_eOrientation == orientation::Landscape ? "Landscape" : "Portrait";
    aStream.WriteLine( aLine );

    aLine = "copies=";
    aLine += ByteString::CreateFromInt32( m_nCopies );
    aStream.WriteLine( aLine );

    aLine = "scale=";
    aLine += ByteString::CreateFromInt32( m_nScale );
    aStream.WriteLine( aLine );

    aLine = "margindajustment=";
    aLine += ByteString::CreateFromInt32( m_nLeftMarginAdjust );
    aLine += ',';
    aLine += ByteString::CreateFromInt32( m_nRightMarginAdjust );
    aLine += ',';
    aLine += ByteString::CreateFromInt32( m_nTopMarginAdjust );
    aLine += ',';
    aLine += ByteString::CreateFromInt32( m_nBottomMarginAdjust );
    aStream.WriteLine( aLine );

    aLine = "colordepth=";
    aLine += ByteString::CreateFromInt32( m_nColorDepth );
    aStream.WriteLine( aLine );

    aLine = "pslevel=";
    aLine += ByteString::CreateFromInt32( m_nPSLevel );
    aStream.WriteLine( aLine );

    aLine = "colordevice=";
    aLine += ByteString::CreateFromInt32( m_nColorDevice );
    aStream.WriteLine( aLine );

    // now append the PPDContext stream buffer
    aStream.WriteLine( "PPDContexData" );
    ULONG nBytes;
    void* pContextBuffer = m_aContext.getStreamableBuffer( nBytes );
    if( nBytes )
        aStream.Write( pContextBuffer, nBytes );

    // success
    pData = rtl_allocateMemory( bytes = aStream.GetSize() );
    memcpy( pData, aStream.GetData(), bytes );
    return true;
}

bool JobData::constructFromStreamBuffer( void* pData, int bytes, JobData& rJobData )
{
    SvMemoryStream aStream( pData, bytes, STREAM_READ );
    ByteString aLine;
    bool bVersion       = false;
    bool bPrinter       = false;
    bool bOrientation   = false;
    bool bCopies        = false;
    bool bScale         = false;
    bool bContext       = false;
    bool bMargin        = false;
    bool bColorDepth    = false;
    bool bColorDevice   = false;
    bool bPSLevel       = false;
    while( ! aStream.IsEof() )
    {
        aStream.ReadLine( aLine );
        if( aLine.CompareTo( "JobData", 7 ) == COMPARE_EQUAL )
            bVersion = true;
        else if( aLine.CompareTo( "printer=", 8 ) == COMPARE_EQUAL )
        {
            bPrinter = true;
            rJobData.m_aPrinterName = String( aLine.Copy( 8 ), RTL_TEXTENCODING_UTF8 );
        }
        else if( aLine.CompareTo( "orientation=", 12 ) == COMPARE_EQUAL )
        {
            bOrientation = true;
            rJobData.m_eOrientation = aLine.Copy( 12 ).EqualsIgnoreCaseAscii( "landscape" ) ? orientation::Landscape : orientation::Portrait;
        }
        else if( aLine.CompareTo( "copies=", 7 ) == COMPARE_EQUAL )
        {
            bCopies = true;
            rJobData.m_nCopies = aLine.Copy( 7 ).ToInt32();
        }
        else if( aLine.CompareTo( "scale=", 6 ) == COMPARE_EQUAL )
        {
            bScale = true;
            rJobData.m_nScale = aLine.Copy( 6 ).ToInt32();
        }
        else if( aLine.CompareTo( "margindajustment=",17 ) == COMPARE_EQUAL )
        {
            bMargin = true;
            ByteString aValues( aLine.Copy( 17 ) );
            rJobData.m_nLeftMarginAdjust = aValues.GetToken( 0, ',' ).ToInt32();
            rJobData.m_nRightMarginAdjust = aValues.GetToken( 1, ',' ).ToInt32();
            rJobData.m_nTopMarginAdjust = aValues.GetToken( 2, ',' ).ToInt32();
            rJobData.m_nBottomMarginAdjust = aValues.GetToken( 3, ',' ).ToInt32();
        }
        else if( aLine.CompareTo( "colordepth=", 11 ) == COMPARE_EQUAL )
        {
            bColorDepth = true;
            rJobData.m_nColorDepth = aLine.Copy( 11 ).ToInt32();
        }
        else if( aLine.CompareTo( "colordevice=", 12 ) == COMPARE_EQUAL )
        {
            bColorDevice = true;
            rJobData.m_nColorDevice = aLine.Copy( 12 ).ToInt32();
        }
        else if( aLine.CompareTo( "pslevel=", 8 ) == COMPARE_EQUAL )
        {
            bPSLevel = true;
            rJobData.m_nPSLevel = aLine.Copy( 8 ).ToInt32();
        }
        else if( aLine.Equals( "PPDContexData" ) )
        {
            if( bPrinter )
            {
                PrinterInfoManager& rManager = PrinterInfoManager::get();
                const PrinterInfo& rInfo = rManager.getPrinterInfo( rJobData.m_aPrinterName );
                rJobData.m_pParser = PPDParser::getParser( rInfo.m_aDriverName );
                if( rJobData.m_pParser )
                {
                    rJobData.m_aContext.setParser( rJobData.m_pParser );
                    int nBytes = bytes - aStream.Tell();
                    void* pRemain = alloca( bytes - aStream.Tell() );
                    aStream.Read( pRemain, nBytes );
                    rJobData.m_aContext.rebuildFromStreamBuffer( pRemain, nBytes );
                    bContext = true;
                }
            }
        }
    }

    return bVersion && bPrinter && bOrientation && bCopies && bScale && bContext && bMargin && bPSLevel && bColorDevice && bColorDepth;
}
