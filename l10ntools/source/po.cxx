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

#include "po.hxx"
#include <ctime>
#include <vector>

#define POESCAPED OString("\\n\\t\\r\\\\\\\"")
#define POUNESCAPED OString("\n\t\r\\\"")

//Class GenPoEntry

//Escape text
OString ImplEscapeText(const OString& rText,
                       const OString& rUnEscaped= POUNESCAPED,
                       const OString& rEscaped = POESCAPED)
{
    if(rEscaped.getLength()!=2*rUnEscaped.getLength()) throw;
    OString sResult = rText;
    sal_Int32 nCount = 0;
    for(sal_Int32 nIndex=0; nIndex<rText.getLength(); ++nIndex)
    {
        sal_Int32 nActChar = rUnEscaped.indexOf(rText[nIndex]);
        if(nActChar!=-1)
            sResult = sResult.replaceAt((nIndex)+(nCount++),1,
                                        rEscaped.copy(2*nActChar,2));
    }
    return sResult;
}

//Unescape text
OString ImplUnEscapeText(const OString& rText,
                         const OString& rEscaped = POESCAPED,
                         const OString& rUnEscaped = POUNESCAPED)
{
    if(rEscaped.getLength()!=2*rUnEscaped.getLength()) throw;
    OString sResult = rText;
    sal_Int32 nCount = 0;
    for(sal_Int32 nIndex=0; nIndex<rText.getLength()-1; ++nIndex)
    {
        sal_Int32 nActChar = rEscaped.indexOf(rText.copy(nIndex,2));
        if(nActChar % 2 == 0)
            sResult = sResult.replaceAt((nIndex++)-(nCount++),2,
                                        rUnEscaped.copy(nActChar/2,1));
    }
    return sResult;
}

//Generate msgctxt, msgid and msgstr strings
OString ImplGenMsgString(const OString& rSource)
{
    if ( rSource.isEmpty() )
        return "\"\"";

    OString sResult = "\"" + rSource + "\"";
    sal_Int32 nIndex = 0;
    while((nIndex=sResult.indexOf("\\n",nIndex))!=-1)
    {
        if( sResult.copy(nIndex-1,3) != "\\\\n" )
           sResult = sResult.replaceAt(nIndex,2,"\\n\"\n\"");
        ++nIndex;
    }

    if ( sResult.indexOf('\n') != -1 )
        return "\"\"\n" +  sResult;

    return sResult;
}

//Default constructor
GenPoEntry::GenPoEntry()
    : m_sWhiteSpace( OString() )
    , m_sExtractCom( OString() )
    , m_sReference( OString() )
    , m_sContext( OString() )
    , m_sUnTransStr( OString() )
    , m_sTransStr( OString() )
{
}

//Destructor
GenPoEntry::~GenPoEntry()
{
}

//Write to file
void GenPoEntry::writeToFile(std::ofstream& io_rOFStream)
{
    if ( !m_sWhiteSpace.isEmpty() )
        io_rOFStream << m_sWhiteSpace.getStr();
    if ( !m_sExtractCom.isEmpty() )
        io_rOFStream << "#. "
                     << m_sExtractCom.replaceAll("\n","\n#. ").getStr()
                     << std::endl;
    if ( !m_sReference.isEmpty() )
        io_rOFStream << "#: " << m_sReference.getStr() << std::endl;
    if ( !m_sContext.isEmpty() )
        io_rOFStream << "msgctxt "
                     << ImplGenMsgString(m_sContext).getStr() << std::endl;
        io_rOFStream << "msgid "
                     << ImplGenMsgString(
                            ImplEscapeText(m_sUnTransStr)).getStr()
                     << std::endl;
        io_rOFStream << "msgstr "
                     << ImplGenMsgString(
                            ImplEscapeText(m_sTransStr)).getStr()
                     << std::endl;
}

//Class PoEntry

//Split string at the delimiter char
void ImplSplitAt(const OString& rSource, const sal_Int32 nDelimiter,
                 std::vector<OString>& o_vParts)
{
    o_vParts.resize( 0 );
    sal_Int32 nActIndex( 0 );
    sal_Int32 nLastSplit( 0 );
    while( nActIndex < rSource.getLength() )
    {
        if ( rSource[nActIndex] == nDelimiter )
        {
            o_vParts.push_back(rSource.copy(nLastSplit,nActIndex-nLastSplit));
            nLastSplit = nActIndex+1;
        }
        ++nActIndex;
    }
    o_vParts.push_back(rSource.copy(nLastSplit));
}

//Generate crc24
sal_Int32 ImplGenCRC24(const OString& rKey)
{
    const sal_Int32 CRC24_INIT = 0x00b704ce;
    const sal_Int32 CRC24_POLY = 0x00864cfb;
    sal_Int32 nCRC = CRC24_INIT;
    sal_Int32 nPosition = 0;

    while ( nPosition < rKey.getLength() )
    {
        nCRC ^= sal_Int32(rKey[nPosition]) << 16;
        for (sal_uInt16 i = 0; i < 8; ++i)
        {
            nCRC <<= 1;
            if (nCRC & 0x01000000)
                nCRC ^= CRC24_POLY;
        }
        ++nPosition;
    }
    return nCRC & 0x00ffffff;
}

//Generate KeyId
OString ImplGenKeyId(const OString& rSourcePath, const OString& rContext)
{
    std::vector<OString> vPathParts;
    ImplSplitAt(rSourcePath,'\\',vPathParts);
    if ( vPathParts.size()<3 ) throw;

    sal_Int32 nCRC = ImplGenCRC24(  vPathParts[vPathParts.size()-3] + "_" +
                                    vPathParts[vPathParts.size()-2] + "_" +
                                    vPathParts[vPathParts.size()-1] + "_" +
                                    rContext);
    const OString sSymbols = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#_";
    OString sKeyId = "";
    while ( sKeyId.getLength() < 4 )
    {
        sKeyId += sSymbols.copy(nCRC & 63, 1);
        nCRC >>= 6;
    }
    return sKeyId;
}

//Construct PoEntry from sdfline
PoEntry::PoEntry(const OString& rSDFLine, const TYPE eType)
    : m_sSourceFile( OString() )
    , m_sGroupId( OString() )
    , m_sLocalId( OString() )
    , m_sResourceType(OString() )
    , m_eType( TTEXT )
    , m_sKeyId( OString() )
{
    setWhiteSpace("\n");
    std::vector<OString> vParts;
    ImplSplitAt(rSDFLine,'\t',vParts);
    if(vParts.size()!=15) throw;

    m_sSourceFile = vParts[SOURCEFILE].
                        copy(vParts[SOURCEFILE].lastIndexOf("\\")+1);
    setReference(m_sSourceFile);
    m_sGroupId = vParts[GROUPID];
    m_sLocalId = vParts[LOCALID];
    m_sResourceType = vParts[RESOURCETYPE];
    m_eType = eType;
    m_sHelpText = vParts[HELPTEXT];

    OString sContext = m_sGroupId + "\\n" +
                       (m_sLocalId.isEmpty() ? "" : m_sLocalId + "\\n") +
                       m_sResourceType;
    switch(eType){
    case TTEXT:
        sContext += ".text"; break;
    case TQUICKHELPTEXT:
        sContext += ".quickhelptext"; break;
    case TTITLE:
        sContext += ".title"; break;
    default:
        throw; break;
    }
    setContext(sContext);
    m_sKeyId = ImplGenKeyId(vParts[PROJECT] + "\\" + vParts[SOURCEFILE],
                            sContext);
    setExtractCom((m_sHelpText.isEmpty() ? "" : m_sHelpText + "\n") + m_sKeyId);

    setUnTransStr(vParts[eType]);
}

PoEntry::~PoEntry()
{
}

//Unescape sdf text
OString ImplUnEscapeSDFText(const OString& rText,const bool bHelpText = false)
{
    if ( bHelpText )
        return ImplUnEscapeText(rText,"\\<\\>\\\"\\\\","<>\"\\");
    else
        return ImplUnEscapeText(rText,"\\n\\t\\r","\n\t\r");
}

//Set translation text when input is in sdf format
void PoEntry::setUnTransStr(const OString& rUnTransStr)
{
    GenPoEntry::setUnTransStr(
                    ImplUnEscapeSDFText(
                        rUnTransStr,m_sSourceFile.endsWith(".xhp")));
}

//Set translated text when input is in sdf format
void PoEntry::setTransStr(const OString& rTransStr)
{
    GenPoEntry::setTransStr(
                    ImplUnEscapeSDFText(
                        rTransStr,m_sSourceFile.endsWith(".xhp")));
}

//Class PoHeader

//Get actual time in "YEAR-MO-DA HO:MI+ZONE" form
OString ImplGetTime()
{
    time_t aNow(time(NULL));
    struct tm* pNow(localtime(&aNow));
    char pBuff[50];
    strftime( pBuff, sizeof pBuff, "%Y-%m-%d %H:%M%z", pNow );
    return pBuff;
}

//Constructor
PoHeader::PoHeader( const OString& rExtSrc )
    : m_sExtractionSource( rExtSrc )
    , m_sProjectIdVersion( "PACKAGE VERSION" )
    , m_sReportMsgidBugsTo( OString("https://bugs.freedesktop.org/") +
                            "enter_bug.cgi?product=LibreOffice&" +
                            "bug_status=UNCONFIRMED&component=UI" )
    , m_sPotCreationDate( ImplGetTime() )
    , m_sPoRevisionDate( "YEAR-MO-DA HO:MI+ZONE" )
    , m_sLastTranslator( "FULL NAME <EMAIL@ADDRESS>" )
    , m_sLanguageTeam( "LANGUAGE <LL@li.org>" )
    , m_sMimeVersion( "1.0" )
    , m_sContentType( "text/plain" )
    , m_sCharset( "UTF-8" )
    , m_sEncoding( "8bit" )
    , m_sXGenerator( "LibreOffice" )
    , m_sXAcceleratorMarker( "~" )
{
    setExtractCom("extracted from " + rExtSrc);
    setTransStr("Project-Id-Version: " + m_sProjectIdVersion + "\n" +
                "Report-Msgid-Bugs-To: " + m_sReportMsgidBugsTo + "\n" +
                "POT-Creation-Date: " + m_sPotCreationDate + "\n" +
                "PO-Revision-Date: " + m_sPoRevisionDate + "\n" +
                "Last-Translator: " + m_sLastTranslator + "\n" +
                "Language-Team: " + m_sLanguageTeam + "\n" +
                "MIME-Version: " + m_sMimeVersion + "\n" +
                "Content-Type: " + m_sContentType + "; " +
                "charset=" + m_sCharset + "\n" +
                "Content-Transfer-Encoding: " + m_sEncoding + "\n" +
                "X-Genarator: " + m_sXGenerator + "\n" +
                "X-Accelerator_Marker: " + m_sXAcceleratorMarker + "\n");
}

PoHeader::~PoHeader()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
