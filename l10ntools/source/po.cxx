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
#include <boost/crc.hpp>

#define POESCAPED OString("\\n\\t\\r\\\\\\\"")
#define POUNESCAPED OString("\n\t\r\\\"")

//Class GenPoEntry

//Generate KeyId
OString ImplGenKeyId(const OString& rGenerator)
{
    boost::crc_32_type aCRC32;
    aCRC32.process_bytes(rGenerator.getStr(), rGenerator.getLength());
    sal_uInt32 nCRC = aCRC32.checksum();
    OString sKeyId = "";
    while ( sKeyId.getLength() < 4 )
    {
        //Concat a char from the [33,126] interval of ASCII
        sKeyId += OString(char(int(double(nCRC & 255)/255*93)+33));
        nCRC >>= 8;
    }
    return sKeyId;
}

//Escape text
OString ImplEscapeText(const OString& rText,
                       const OString& rUnEscaped= POUNESCAPED,
                       const OString& rEscaped = POESCAPED)
{
    if(rEscaped.getLength()!=2*rUnEscaped.getLength()) throw;
    OString sResult = rText;
    int nCount = 0;
    for(int nIndex=0; nIndex<rText.getLength(); ++nIndex)
    {
        int nActChar = rUnEscaped.indexOf(rText[nIndex]);
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
    int nCount = 0;
    for(int nIndex=0; nIndex<rText.getLength()-1; ++nIndex)
    {
        int nActChar = rEscaped.indexOf(rText.copy(nIndex,2));
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
    int nIndex = 0;
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
    , m_bFuzzy( false )
    , m_sKeyId( OString() )
{
}

//Destructor
GenPoEntry::~GenPoEntry()
{
}

//Set keyid
void GenPoEntry::genKeyId()
{
    m_sKeyId = ImplGenKeyId(m_sReference + m_sContext + m_sUnTransStr);
}

//Write to file
void GenPoEntry::writeToFile(std::ofstream& io_rOFStream)
{
    if ( !m_sWhiteSpace.isEmpty() )
        io_rOFStream << m_sWhiteSpace.getStr();
    if ( !m_sExtractCom.isEmpty() )
        io_rOFStream << "#. " << m_sExtractCom.getStr() << std::endl;
    if ( !m_sKeyId.isEmpty() )
        io_rOFStream << "#. " << m_sKeyId.getStr() << std::endl;
    if ( !m_sReference.isEmpty() )
        io_rOFStream << "#: " << m_sReference.getStr() << std::endl;
    if ( m_bFuzzy )
        io_rOFStream << "#, fuzzy" << std::endl;
    if ( !m_sContext.isEmpty() )
        io_rOFStream << "msgctxt "
                     << ImplGenMsgString(m_sContext).getStr() << std::endl;
    io_rOFStream << "msgid "
                 << ImplGenMsgString(ImplEscapeText(m_sUnTransStr)).getStr()
                 << std::endl;
    io_rOFStream << "msgstr "
                 << ImplGenMsgString(ImplEscapeText(m_sTransStr)).getStr()
                 << std::endl;
}

//Class PoEntry

//Split string at the delimiter char
void ImplSplitAt(const OString& rSource, const char nDelimiter,
                 std::vector<OString>& o_vParts)
{
    o_vParts.resize( 0 );
    int nActIndex( 0 );
    int nLastSplit( 0 );
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



//Construct PoEntry from sdfline
PoEntry::PoEntry(const OString& rSDFLine, const TYPE eType)
    : m_sSourceFile( OString() )
    , m_sGroupId( OString() )
    , m_sLocalId( OString() )
    , m_sResourceType(OString() )
    , m_eType( TTEXT )
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
    setExtractCom(m_sHelpText);

    setUnTransStr(vParts[eType]);
    genKeyId();
}

//Destructor
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
