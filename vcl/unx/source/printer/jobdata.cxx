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
#include "precompiled_vcl.hxx"

#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"

#include "tools/stream.hxx"

#include "sal/alloca.h"

using namespace psp;

JobData& JobData::operator=(const JobData& rRight)
{
    m_nCopies               = rRight.m_nCopies;
    m_nLeftMarginAdjust     = rRight.m_nLeftMarginAdjust;
    m_nRightMarginAdjust    = rRight.m_nRightMarginAdjust;
    m_nTopMarginAdjust      = rRight.m_nTopMarginAdjust;
    m_nBottomMarginAdjust   = rRight.m_nBottomMarginAdjust;
    m_nColorDepth           = rRight.m_nColorDepth;
    m_eOrientation          = rRight.m_eOrientation;
    m_aPrinterName          = rRight.m_aPrinterName;
    m_pParser               = rRight.m_pParser;
    m_aContext              = rRight.m_aContext;
    m_nPSLevel              = rRight.m_nPSLevel;
    m_nColorDevice          = rRight.m_nColorDevice;

    if( ! m_pParser && m_aPrinterName.getLength() )
    {
        PrinterInfoManager& rMgr = PrinterInfoManager::get();
        rMgr.setupJobContextData( *this );
    }
    return *this;
}

void JobData::setCollate( bool bCollate )
{
    const PPDParser* pParser = m_aContext.getParser();
    if( pParser )
    {
        const PPDKey* pKey = pParser->getKey( String( RTL_CONSTASCII_USTRINGPARAM( "Collate" ) ) );
        if( pKey )
        {
            const PPDValue* pVal = NULL;
            if( bCollate )
                pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "True" ) ) );
            else
            {
                pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "False" ) ) );
                if( ! pVal )
                    pVal = pKey->getValue( String( RTL_CONSTASCII_USTRINGPARAM( "None" ) ) );
            }
            m_aContext.setValue( pKey, pVal );
        }
    }
}

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
    pData = rtl_allocateMemory( bytes = aStream.Tell() );
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

    return bVersion && bPrinter && bOrientation && bCopies && bContext && bMargin && bPSLevel && bColorDevice && bColorDepth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
