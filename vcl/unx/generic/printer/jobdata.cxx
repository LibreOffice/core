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


#include <officecfg/Office/Common.hxx>
#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"
#include "tools/stream.hxx"

#include <sal/alloca.h>
#include <rtl/strbuf.hxx>

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
    m_nPDFDevice            = rRight.m_nPDFDevice;
    m_nColorDevice          = rRight.m_nColorDevice;

    if( !m_pParser && !m_aPrinterName.isEmpty() )
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
        const PPDKey* pKey = pParser->getKey( OUString( "Collate" ) );
        if( pKey )
        {
            const PPDValue* pVal = NULL;
            if( bCollate )
                pVal = pKey->getValue( OUString( "True" ) );
            else
            {
                pVal = pKey->getValue( OUString( "False" ) );
                if( ! pVal )
                    pVal = pKey->getValue( OUString( "None" ) );
            }
            m_aContext.setValue( pKey, pVal );
        }
    }
}

bool JobData::setPaper( int i_nWidth, int i_nHeight )
{
    bool bSuccess = false;
    if( m_pParser )
    {
        OUString aPaper( m_pParser->matchPaper( i_nWidth, i_nHeight ) );

        const PPDKey*   pKey = m_pParser->getKey( OUString( "PageSize" ) );
        const PPDValue* pValue = pKey ? pKey->getValueCaseInsensitive( aPaper ) : NULL;

        bSuccess = pKey && pValue && m_aContext.setValue( pKey, pValue, false );
    }
    return bSuccess;
}

bool JobData::setPaperBin( int i_nPaperBin )
{
    bool bSuccess = false;
    if( m_pParser )
    {
        const PPDKey*   pKey = m_pParser->getKey( OUString( "InputSlot" ) );
        const PPDValue* pValue = pKey ? pKey->getValue( i_nPaperBin ) : NULL;

        bSuccess = pKey && pValue && m_aContext.setValue( pKey, pValue, false );
    }
    return bSuccess;
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

    // write header job data
    aStream.WriteLine(OString(RTL_CONSTASCII_STRINGPARAM("JobData 1")));

    OStringBuffer aLine;

    aLine.append(RTL_CONSTASCII_STRINGPARAM("printer="));
    aLine.append(OUStringToOString(m_aPrinterName, RTL_TEXTENCODING_UTF8));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("orientation="));
    if (m_eOrientation == orientation::Landscape)
        aLine.append(RTL_CONSTASCII_STRINGPARAM("Landscape"));
    else
        aLine.append(RTL_CONSTASCII_STRINGPARAM("Portrait"));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("copies="));
    aLine.append(static_cast<sal_Int32>(m_nCopies));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("margindajustment="));
    aLine.append(static_cast<sal_Int32>(m_nLeftMarginAdjust));
    aLine.append(',');
    aLine.append(static_cast<sal_Int32>(m_nRightMarginAdjust));
    aLine.append(',');
    aLine.append(static_cast<sal_Int32>(m_nTopMarginAdjust));
    aLine.append(',');
    aLine.append(static_cast<sal_Int32>(m_nBottomMarginAdjust));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("colordepth="));
    aLine.append(static_cast<sal_Int32>(m_nColorDepth));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("pslevel="));
    aLine.append(static_cast<sal_Int32>(m_nPSLevel));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("pdfdevice="));
    aLine.append(static_cast<sal_Int32>(m_nPDFDevice));
    aStream.WriteLine(aLine.makeStringAndClear());

    aLine.append(RTL_CONSTASCII_STRINGPARAM("colordevice="));
    aLine.append(static_cast<sal_Int32>(m_nColorDevice));
    aStream.WriteLine(aLine.makeStringAndClear());

    // now append the PPDContext stream buffer
    aStream.WriteLine( "PPDContexData" );
    sal_uLong nBytes;
    char* pContextBuffer = m_aContext.getStreamableBuffer( nBytes );
    if( nBytes )
        aStream.Write( pContextBuffer, nBytes );
    delete [] pContextBuffer;

    // success
    pData = rtl_allocateMemory( bytes = aStream.Tell() );
    memcpy( pData, aStream.GetData(), bytes );
    return true;
}

bool JobData::constructFromStreamBuffer( void* pData, int bytes, JobData& rJobData )
{
    SvMemoryStream aStream( pData, bytes, STREAM_READ );
    OString aLine;
    bool bVersion       = false;
    bool bPrinter       = false;
    bool bOrientation   = false;
    bool bCopies        = false;
    bool bContext       = false;
    bool bMargin        = false;
    bool bColorDepth    = false;
    bool bColorDevice   = false;
    bool bPSLevel       = false;
    bool bPDFDevice     = false;

    const char printerEquals[] = "printer=";
    const char orientatationEquals[] = "orientation=";
    const char copiesEquals[] = "copies=";
    const char margindajustmentEquals[] = "margindajustment=";
    const char colordepthEquals[] = "colordepth=";
    const char colordeviceEquals[] = "colordevice=";
    const char pslevelEquals[] = "pslevel=";
    const char pdfdeviceEquals[] = "pdfdevice=";

    while( ! aStream.IsEof() )
    {
        aStream.ReadLine( aLine );
        if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM("JobData")))
            bVersion = true;
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM(printerEquals)))
        {
            bPrinter = true;
            rJobData.m_aPrinterName = OStringToOUString(aLine.copy(RTL_CONSTASCII_LENGTH(printerEquals)), RTL_TEXTENCODING_UTF8);
        }
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM(orientatationEquals)))
        {
            bOrientation = true;
            rJobData.m_eOrientation = aLine.copy(RTL_CONSTASCII_LENGTH(orientatationEquals)).equalsIgnoreAsciiCase("landscape") ? orientation::Landscape : orientation::Portrait;
        }
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM(copiesEquals)))
        {
            bCopies = true;
            rJobData.m_nCopies = aLine.copy(RTL_CONSTASCII_LENGTH(copiesEquals)).toInt32();
        }
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM(margindajustmentEquals)))
        {
            bMargin = true;
            OString aValues(aLine.copy(RTL_CONSTASCII_LENGTH(margindajustmentEquals)));
            rJobData.m_nLeftMarginAdjust = aValues.getToken(0, ',').toInt32();
            rJobData.m_nRightMarginAdjust = aValues.getToken(1, ',').toInt32();
            rJobData.m_nTopMarginAdjust = aValues.getToken(2, ',').toInt32();
            rJobData.m_nBottomMarginAdjust = aValues.getToken(3, ',').toInt32();
        }
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM(colordepthEquals)))
        {
            bColorDepth = true;
            rJobData.m_nColorDepth = aLine.copy(RTL_CONSTASCII_LENGTH(colordepthEquals)).toInt32();
        }
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM(colordeviceEquals)))
        {
            bColorDevice = true;
            rJobData.m_nColorDevice = aLine.copy(RTL_CONSTASCII_LENGTH(colordeviceEquals)).toInt32();
        }
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM(pslevelEquals)))
        {
            bPSLevel = true;
            rJobData.m_nPSLevel = aLine.copy(RTL_CONSTASCII_LENGTH(pslevelEquals)).toInt32();
        }
        else if (aLine.matchL(RTL_CONSTASCII_STRINGPARAM(pdfdeviceEquals)))
        {
            bPDFDevice = true;
            rJobData.m_nPDFDevice = aLine.copy(RTL_CONSTASCII_LENGTH(pdfdeviceEquals)).toInt32();
        }
        else if (aLine.equalsL(RTL_CONSTASCII_STRINGPARAM("PPDContexData")))
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
                    char* pRemain = (char*)alloca( bytes - aStream.Tell() );
                    aStream.Read( pRemain, nBytes );
                    rJobData.m_aContext.rebuildFromStreamBuffer( pRemain, nBytes );
                    bContext = true;
                }
            }
        }
    }

    return bVersion && bPrinter && bOrientation && bCopies && bContext && bMargin && bPSLevel && bPDFDevice && bColorDevice && bColorDepth;
}

void JobData::resolveDefaultBackend()
{
    if (m_nPSLevel == 0 && m_nPDFDevice == 0)
        setDefaultBackend(officecfg::Office::Common::Print::Option::Printer::PDFAsStandardPrintJobFormat::get());
}

void JobData::setDefaultBackend(bool bUsePDF)
{
    if (bUsePDF && m_nPSLevel == 0 && m_nPDFDevice == 0)
        m_nPDFDevice = 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
