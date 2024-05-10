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
#include <jobdata.hxx>
#include <printerinfomanager.hxx>
#include <tools/stream.hxx>
#include <o3tl/string_view.hxx>

#include <rtl/strbuf.hxx>
#include <memory>

using namespace psp;

JobData& JobData::operator=(const JobData& rRight)
{
    if(this == &rRight)
        return *this;

    m_nCopies               = rRight.m_nCopies;
    m_bCollate              = rRight.m_bCollate;
    m_nLeftMarginAdjust     = rRight.m_nLeftMarginAdjust;
    m_nRightMarginAdjust    = rRight.m_nRightMarginAdjust;
    m_nTopMarginAdjust      = rRight.m_nTopMarginAdjust;
    m_nBottomMarginAdjust   = rRight.m_nBottomMarginAdjust;
    m_nColorDepth           = rRight.m_nColorDepth;
    m_eOrientation          = rRight.m_eOrientation;
    m_aPrinterName          = rRight.m_aPrinterName;
    m_bPapersizeFromSetup   = rRight.m_bPapersizeFromSetup;
    m_pParser               = rRight.m_pParser;
    m_aContext              = rRight.m_aContext;
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
    m_bCollate = bCollate;
    return;
}

void JobData::setPaper( int i_nWidth, int i_nHeight )
{
    if( m_pParser )
    {
        OUString aPaper( m_pParser->matchPaper( i_nWidth, i_nHeight ) );

        const PPDKey*   pKey = m_pParser->getKey( u"PageSize"_ustr );
        const PPDValue* pValue = pKey ? pKey->getValueCaseInsensitive( aPaper ) : nullptr;

        if (pKey && pValue)
            m_aContext.setValue( pKey, pValue );
    }
}

void JobData::setPaperBin( int i_nPaperBin )
{
    if( m_pParser )
    {
        const PPDKey*   pKey = m_pParser->getKey( u"InputSlot"_ustr );
        const PPDValue* pValue = pKey ? pKey->getValue( i_nPaperBin ) : nullptr;

        if (pKey && pValue)
            m_aContext.setValue( pKey, pValue );
    }
}

bool JobData::getStreamBuffer( std::unique_ptr<sal_uInt8[]>& pData, sal_uInt32& bytes )
{
    // consistency checks
    if( ! m_pParser )
        m_pParser = m_aContext.getParser();
    if( m_pParser != m_aContext.getParser() ||
        ! m_pParser )
        return false;

    SvMemoryStream aStream;

    // write header job data
    aStream.WriteLine("JobData 1");

    OStringBuffer aLine("printer="
        + OUStringToOString(m_aPrinterName, RTL_TEXTENCODING_UTF8));
    aStream.WriteLine(aLine);
    aLine.setLength(0);

    aLine.append("orientation=");
    if (m_eOrientation == orientation::Landscape)
        aLine.append("Landscape");
    else
        aLine.append("Portrait");
    aStream.WriteLine(aLine);
    aLine.setLength(0);

    aStream.WriteLine(Concat2View("copies=" + OString::number(static_cast<sal_Int32>(m_nCopies))));
    aStream.WriteLine(Concat2View("collate=" + OString::boolean(m_bCollate)));

    aStream.WriteLine(Concat2View(
        "marginadjustment="
        + OString::number(static_cast<sal_Int32>(m_nLeftMarginAdjust))
        + ","
        + OString::number(static_cast<sal_Int32>(m_nRightMarginAdjust))
        + ",'"
        + OString::number(static_cast<sal_Int32>(m_nTopMarginAdjust))
        + ","
        + OString::number(static_cast<sal_Int32>(m_nBottomMarginAdjust))));

    aStream.WriteLine(Concat2View("colordepth=" + OString::number(static_cast<sal_Int32>(m_nColorDepth))));

    aStream.WriteLine(Concat2View("colordevice=" + OString::number(static_cast<sal_Int32>(m_nColorDevice))));

    // now append the PPDContext stream buffer
    aStream.WriteLine( "PPDContextData" );
    sal_uLong nBytes;
    std::unique_ptr<char[]> pContextBuffer(m_aContext.getStreamableBuffer( nBytes ));
    if( nBytes )
        aStream.WriteBytes( pContextBuffer.get(), nBytes );
    pContextBuffer.reset();

    // success
    bytes = static_cast<sal_uInt32>(aStream.Tell());
    pData = std::make_unique<sal_uInt8[]>( bytes );
    memcpy( pData.get(), aStream.GetData(), bytes );
    return true;
}

bool JobData::constructFromStreamBuffer( const void* pData, sal_uInt32 bytes, JobData& rJobData )
{
    SvMemoryStream aStream( const_cast<void*>(pData), bytes, StreamMode::READ );
    OString aLine;
    bool bVersion       = false;
    bool bPrinter       = false;
    bool bOrientation   = false;
    bool bCopies        = false;
    bool bContext       = false;
    bool bMargin        = false;
    bool bColorDepth    = false;
    bool bColorDevice   = false;

    const char printerEquals[] = "printer=";
    const char orientatationEquals[] = "orientation=";
    const char copiesEquals[] = "copies=";
    const char collateEquals[] = "collate=";
    const char marginadjustmentEquals[] = "marginadjustment=";
    const char colordepthEquals[] = "colordepth=";
    const char colordeviceEquals[] = "colordevice=";

    while( ! aStream.eof() )
    {
        aStream.ReadLine( aLine );
        if (aLine.startsWith("JobData"))
            bVersion = true;
        else if (aLine.startsWith(printerEquals))
        {
            bPrinter = true;
            rJobData.m_aPrinterName = OStringToOUString(aLine.subView(RTL_CONSTASCII_LENGTH(printerEquals)), RTL_TEXTENCODING_UTF8);
        }
        else if (aLine.startsWith(orientatationEquals))
        {
            bOrientation = true;
            rJobData.m_eOrientation = o3tl::equalsIgnoreAsciiCase(aLine.subView(RTL_CONSTASCII_LENGTH(orientatationEquals)), "landscape") ? orientation::Landscape : orientation::Portrait;
        }
        else if (aLine.startsWith(copiesEquals))
        {
            bCopies = true;
            rJobData.m_nCopies = o3tl::toInt32(aLine.subView(RTL_CONSTASCII_LENGTH(copiesEquals)));
        }
        else if (aLine.startsWith(collateEquals))
        {
            rJobData.m_bCollate = aLine.copy(RTL_CONSTASCII_LENGTH(collateEquals)).toBoolean();
        }
        else if (aLine.startsWith(marginadjustmentEquals))
        {
            bMargin = true;
            sal_Int32 nIdx {RTL_CONSTASCII_LENGTH(marginadjustmentEquals)};
            rJobData.m_nLeftMarginAdjust = o3tl::toInt32(o3tl::getToken(aLine, 0, ',', nIdx));
            rJobData.m_nRightMarginAdjust = o3tl::toInt32(o3tl::getToken(aLine, 0, ',', nIdx));
            rJobData.m_nTopMarginAdjust = o3tl::toInt32(o3tl::getToken(aLine, 0, ',', nIdx));
            rJobData.m_nBottomMarginAdjust = o3tl::toInt32(o3tl::getToken(aLine, 0, ',', nIdx));
        }
        else if (aLine.startsWith(colordepthEquals))
        {
            bColorDepth = true;
            rJobData.m_nColorDepth = o3tl::toInt32(aLine.subView(RTL_CONSTASCII_LENGTH(colordepthEquals)));
        }
        else if (aLine.startsWith(colordeviceEquals))
        {
            bColorDevice = true;
            rJobData.m_nColorDevice = o3tl::toInt32(aLine.subView(RTL_CONSTASCII_LENGTH(colordeviceEquals)));
        }
        else if (aLine == "PPDContextData" && bPrinter)
        {
            PrinterInfoManager& rManager = PrinterInfoManager::get();
            const PrinterInfo& rInfo = rManager.getPrinterInfo( rJobData.m_aPrinterName );
            rJobData.m_pParser = PPDParser::getParser( rInfo.m_aDriverName );
            if( rJobData.m_pParser )
            {
                rJobData.m_aContext.setParser( rJobData.m_pParser );
                sal_uInt64 nBytes = bytes - aStream.Tell();
                std::vector<char> aRemain(nBytes+1);
                nBytes = aStream.ReadBytes(aRemain.data(), nBytes);
                if (nBytes)
                {
                    aRemain.resize(nBytes+1);
                    aRemain[nBytes] = 0;
                    rJobData.m_aContext.rebuildFromStreamBuffer(aRemain);
                    bContext = true;
                }
            }
        }
    }

    return bVersion && bPrinter && bOrientation && bCopies && bContext && bMargin && bColorDevice && bColorDepth;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
