/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <rtl/crc.h>
#include <sal/log.hxx>

#include <cstring>
#include <ctime>
#include <cassert>

#include <vector>
#include <string>

#include <po.hxx>
#include <helper.hxx>

/** Container of po entry

    Provide all file operations related to LibreOffice specific
    po entry and store it's attributes.
*/
class GenPoEntry
{
private:
    OStringBuffer m_sExtractCom;
    std::vector<OString>    m_sReferences;
    OString    m_sMsgCtxt;
    OString    m_sMsgId;
    OString    m_sMsgIdPlural;
    OString    m_sMsgStr;
    std::vector<OString>    m_sMsgStrPlural;
    bool       m_bFuzzy;
    bool       m_bCFormat;
    bool       m_bNull;

public:
    GenPoEntry();

    const std::vector<OString>& getReference() const    { return m_sReferences; }
    const OString& getMsgCtxt() const      { return m_sMsgCtxt; }
    const OString& getMsgId() const        { return m_sMsgId; }
    const OString& getMsgStr() const       { return m_sMsgStr; }
    bool        isFuzzy() const         { return m_bFuzzy; }
    bool        isNull() const          { return m_bNull; }

    void        setExtractCom(const OString& rExtractCom)
                        {
                            m_sExtractCom = rExtractCom;
                        }
    void        setReference(const OString& rReference)
                        {
                            m_sReferences.push_back(rReference);
                        }
    void        setMsgCtxt(const OString& rMsgCtxt)
                        {
                            m_sMsgCtxt = rMsgCtxt;
                        }
    void        setMsgId(const OString& rMsgId)
                        {
                            m_sMsgId = rMsgId;
                        }
    void        setMsgStr(const OString& rMsgStr)
                        {
                            m_sMsgStr = rMsgStr;
                        }

    void        writeToFile(std::ofstream& rOFStream) const;
    void        readFromFile(std::ifstream& rIFStream);
};

namespace
{
    // Convert a normal string to msg/po output string
    OString lcl_GenMsgString(const OString& rString)
    {
        if ( rString.isEmpty() )
            return "\"\"";

        OString sResult =
            "\"" +
            helper::escapeAll(rString,"\n""\t""\r""\\""\"","\\n""\\t""\\r""\\\\""\\\"") +
            "\"";
        sal_Int32 nIndex = 0;
        while((nIndex=sResult.indexOf("\\n",nIndex))!=-1)
        {
            if( !sResult.match("\\\\n", nIndex-1) &&
                nIndex!=sResult.getLength()-3)
            {
               sResult = sResult.replaceAt(nIndex,2,"\\n\"\n\"");
            }
            ++nIndex;
        }

        if ( sResult.indexOf('\n') != -1 )
            return "\"\"\n" +  sResult;

        return sResult;
    }

    // Convert msg string to normal form
    OString lcl_GenNormString(const OString& rString)
    {
        return
            helper::unEscapeAll(
                rString.copy(1,rString.getLength()-2),
                "\\n""\\t""\\r""\\\\""\\\"",
                "\n""\t""\r""\\""\"");
    }
}

GenPoEntry::GenPoEntry()
    : m_sExtractCom( OString() )
    , m_sReferences( std::vector<OString>() )
    , m_sMsgCtxt( OString() )
    , m_sMsgId( OString() )
    , m_sMsgIdPlural( OString() )
    , m_sMsgStr( OString() )
    , m_sMsgStrPlural( std::vector<OString>() )
    , m_bFuzzy( false )
    , m_bCFormat( false )
    , m_bNull( false )
{
}

void GenPoEntry::writeToFile(std::ofstream& rOFStream) const
{
    if ( rOFStream.tellp() != std::ofstream::pos_type( 0 ))
        rOFStream << std::endl;
    if ( !m_sExtractCom.isEmpty() )
        rOFStream
            << "#. "
            << m_sExtractCom.toString().replaceAll("\n","\n#. ") << std::endl;
    for(const auto& rReference : m_sReferences)
        rOFStream << "#: " << rReference << std::endl;
    if ( m_bFuzzy )
        rOFStream << "#, fuzzy" << std::endl;
    if ( m_bCFormat )
        rOFStream << "#, c-format" << std::endl;
    if ( !m_sMsgCtxt.isEmpty() )
        rOFStream << "msgctxt "
                  << lcl_GenMsgString(m_sMsgCtxt)
                  << std::endl;
    rOFStream << "msgid "
              << lcl_GenMsgString(m_sMsgId) << std::endl;
    if ( !m_sMsgIdPlural.isEmpty() )
        rOFStream << "msgid_plural "
                  << lcl_GenMsgString(m_sMsgIdPlural)
                  << std::endl;
    if ( !m_sMsgStrPlural.empty() )
        for(auto & line : m_sMsgStrPlural)
            rOFStream << line.copy(0,10) << lcl_GenMsgString(line.copy(10)) << std::endl;
    else
        rOFStream << "msgstr "
                  << lcl_GenMsgString(m_sMsgStr) << std::endl;
}

void GenPoEntry::readFromFile(std::ifstream& rIFStream)
{
    *this = GenPoEntry();
    OString* pLastMsg = nullptr;
    std::string sTemp;
    getline(rIFStream,sTemp);
    if( rIFStream.eof() || sTemp.empty() )
    {
        m_bNull = true;
        return;
    }
    while(!rIFStream.eof())
    {
        OString sLine = OString(sTemp.data(),sTemp.length());
        if (sLine.startsWith("#. "))
        {
            if( !m_sExtractCom.isEmpty() )
            {
                m_sExtractCom.append("\n");
            }
            m_sExtractCom.append(sLine.copy(3));
        }
        else if (sLine.startsWith("#: "))
        {
            m_sReferences.push_back(sLine.copy(3));
        }
        else if (sLine.startsWith("#, fuzzy"))
        {
            m_bFuzzy = true;
        }
        else if (sLine.startsWith("#, c-format"))
        {
            m_bCFormat = true;
        }
        else if (sLine.startsWith("msgctxt "))
        {
            m_sMsgCtxt = lcl_GenNormString(sLine.copy(8));
            pLastMsg = &m_sMsgCtxt;
        }
        else if (sLine.startsWith("msgid "))
        {
            m_sMsgId = lcl_GenNormString(sLine.copy(6));
            pLastMsg = &m_sMsgId;
        }
        else if (sLine.startsWith("msgid_plural "))
        {
            m_sMsgIdPlural = lcl_GenNormString(sLine.copy(13));
            pLastMsg = &m_sMsgIdPlural;
        }
        else if (sLine.startsWith("msgstr "))
        {
            m_sMsgStr = lcl_GenNormString(sLine.copy(7));
            pLastMsg = &m_sMsgStr;
        }
        else if (sLine.startsWith("msgstr["))
        {
            // assume there are no more than 10 plural forms...
            // and that plural strings are never split to multi-line in po
            m_sMsgStrPlural.push_back(sLine.copy(0,10) + lcl_GenNormString(sLine.copy(10)));
        }
        else if (sLine.startsWith("\"") && pLastMsg)
        {
            OString sReference;
            if (!m_sReferences.empty())
            {
                sReference = m_sReferences.front();
            }
            if (pLastMsg != &m_sMsgCtxt || sLine != "\"" + sReference + "\\n\"")
            {
                *pLastMsg += lcl_GenNormString(sLine);
            }
        }
        else
            break;
        getline(rIFStream,sTemp);
    }
 }

PoEntry::PoEntry()
    : m_bIsInitialized( false )
{
}

PoEntry::PoEntry(
    const OString& rSourceFile, const OString& rResType, const OString& rGroupId,
    const OString& rLocalId, const OString& rHelpText,
    const OString& rText, const TYPE eType )
    : m_bIsInitialized( false )
{
    if( rSourceFile.isEmpty() )
        throw NOSOURCFILE;
    else if ( rResType.isEmpty() )
        throw NORESTYPE;
    else if ( rGroupId.isEmpty() )
        throw NOGROUPID;
    else if ( rText.isEmpty() )
        throw NOSTRING;
    else if ( rHelpText.getLength() == 5 )
        throw WRONGHELPTEXT;

    m_pGenPo.reset( new GenPoEntry() );
    OString sReference = rSourceFile.copy(rSourceFile.lastIndexOf('/')+1);
    m_pGenPo->setReference(sReference);

    OString sMsgCtxt =
        sReference + "\n" +
        rGroupId + "\n" +
        (rLocalId.isEmpty() ? OString() : rLocalId + "\n") +
        rResType;
    switch(eType){
    case TTEXT:
        sMsgCtxt += ".text"; break;
    case TQUICKHELPTEXT:
        sMsgCtxt += ".quickhelptext"; break;
    case TTITLE:
        sMsgCtxt += ".title"; break;
    // Default case is unneeded because the type of eType has only three element
    }
    m_pGenPo->setMsgCtxt(sMsgCtxt);
    m_pGenPo->setMsgId(rText);
    m_pGenPo->setExtractCom(
        ( !rHelpText.isEmpty() ?  rHelpText + "\n" : OString()) +
        genKeyId( m_pGenPo->getReference().front() + rGroupId + rLocalId + rResType + rText ) );
    m_bIsInitialized = true;
}

PoEntry::~PoEntry()
{
}

PoEntry::PoEntry( const PoEntry& rPo )
    : m_pGenPo( rPo.m_pGenPo ? new GenPoEntry( *(rPo.m_pGenPo) ) : nullptr )
    , m_bIsInitialized( rPo.m_bIsInitialized )
{
}

PoEntry& PoEntry::operator=(const PoEntry& rPo)
{
    if( this == &rPo )
    {
        return *this;
    }
    if( rPo.m_pGenPo )
    {
        if( m_pGenPo )
        {
            *m_pGenPo = *(rPo.m_pGenPo);
        }
        else
        {
            m_pGenPo.reset( new GenPoEntry( *(rPo.m_pGenPo) ) );
        }
    }
    else
    {
        m_pGenPo.reset();
    }
    m_bIsInitialized = rPo.m_bIsInitialized;
    return *this;
}

PoEntry& PoEntry::operator=(PoEntry&& rPo)
{
    m_pGenPo = std::move(rPo.m_pGenPo);
    m_bIsInitialized = std::move(rPo.m_bIsInitialized);
    return *this;
}

OString const & PoEntry::getSourceFile() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getReference().front();
}

OString PoEntry::getGroupId() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getMsgCtxt().getToken(0,'\n');
}

OString PoEntry::getLocalId() const
{
    assert( m_bIsInitialized );
    const OString sMsgCtxt = m_pGenPo->getMsgCtxt();
    if (sMsgCtxt.indexOf('\n')==sMsgCtxt.lastIndexOf('\n'))
        return OString();
    else
        return sMsgCtxt.getToken(1,'\n');
}

OString PoEntry::getResourceType() const
{
    assert( m_bIsInitialized );
    const OString sMsgCtxt = m_pGenPo->getMsgCtxt();
    if (sMsgCtxt.indexOf('\n')==sMsgCtxt.lastIndexOf('\n'))
        return sMsgCtxt.getToken(1,'\n').getToken(0,'.');
    else
        return sMsgCtxt.getToken(2,'\n').getToken(0,'.');
}

PoEntry::TYPE PoEntry::getType() const
{
    assert( m_bIsInitialized );
    const OString sMsgCtxt = m_pGenPo->getMsgCtxt();
    const OString sType = sMsgCtxt.copy( sMsgCtxt.lastIndexOf('.') + 1 );
    assert(
        (sType == "text" || sType == "quickhelptext" || sType == "title") );
    if ( sType == "text" )
        return TTEXT;
    else if ( sType == "quickhelptext" )
        return TQUICKHELPTEXT;
    else
        return TTITLE;
}

bool PoEntry::isFuzzy() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->isFuzzy();
}

// Get message context
const OString& PoEntry::getMsgCtxt() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getMsgCtxt();

}

// Get translation string in merge format
OString const & PoEntry::getMsgId() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getMsgId();
}

// Get translated string in merge format
const OString& PoEntry::getMsgStr() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getMsgStr();

}

bool PoEntry::IsInSameComp(const PoEntry& rPo1,const PoEntry& rPo2)
{
    assert( rPo1.m_bIsInitialized && rPo2.m_bIsInitialized );
    return ( rPo1.getSourceFile() == rPo2.getSourceFile() &&
             rPo1.getGroupId() == rPo2.getGroupId() &&
             rPo1.getLocalId() == rPo2.getLocalId() &&
             rPo1.getResourceType() == rPo2.getResourceType() );
}

OString PoEntry::genKeyId(const OString& rGenerator)
{
    sal_uInt32 nCRC = rtl_crc32(0, rGenerator.getStr(), rGenerator.getLength());
    // Use simple ASCII characters, exclude I, l, 1 and O, 0 to avoid confusing IDs
    static const char sSymbols[] =
        "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
    char sKeyId[6];
    for( short nKeyInd = 0; nKeyInd < 5; ++nKeyInd )
    {
        sKeyId[nKeyInd] = sSymbols[(nCRC & 63) % strlen(sSymbols)];
        nCRC >>= 6;
    }
    sKeyId[5] = '\0';
    return OString(sKeyId);
}

namespace
{
    // Get actual time in "YEAR-MO-DA HO:MI+ZONE" form
    OString lcl_GetTime()
    {
        time_t aNow = time(nullptr);
        struct tm* pNow = localtime(&aNow);
        char pBuff[50];
        strftime( pBuff, sizeof pBuff, "%Y-%m-%d %H:%M%z", pNow );
        return OString(pBuff);
    }
}

// when updating existing files (pocheck), reuse provided po-header
PoHeader::PoHeader( const OString& rExtSrc, const OString& rPoHeaderMsgStr )
    : m_pGenPo( new GenPoEntry() )
    , m_bIsInitialized( false )
{
    m_pGenPo->setExtractCom("extracted from " + rExtSrc);
    m_pGenPo->setMsgStr(rPoHeaderMsgStr);
    m_bIsInitialized = true;
}

PoHeader::PoHeader( const OString& rExtSrc )
    : m_pGenPo( new GenPoEntry() )
    , m_bIsInitialized( false )
{
    m_pGenPo->setExtractCom("extracted from " + rExtSrc);
    m_pGenPo->setMsgStr(
        OString("Project-Id-Version: PACKAGE VERSION\n"
        "Report-Msgid-Bugs-To: https://bugs.libreoffice.org/enter_bug.cgi?"
        "product=LibreOffice&bug_status=UNCONFIRMED&component=UI\n"
        "POT-Creation-Date: ") + lcl_GetTime() +
        OString("\nPO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n"
        "Last-Translator: FULL NAME <EMAIL@ADDRESS>\n"
        "Language-Team: LANGUAGE <LL@li.org>\n"
        "MIME-Version: 1.0\n"
        "Content-Type: text/plain; charset=UTF-8\n"
        "Content-Transfer-Encoding: 8bit\n"
        "X-Accelerator-Marker: ~\n"
        "X-Generator: LibreOffice\n"));
    m_bIsInitialized = true;
}

PoHeader::~PoHeader()
{
}

PoOfstream::PoOfstream()
    : m_aOutPut()
    , m_bIsAfterHeader( false )
{
}

PoOfstream::PoOfstream(const OString& rFileName, OpenMode aMode )
    : m_aOutPut()
    , m_bIsAfterHeader( false )
{
    open( rFileName, aMode );
}

PoOfstream::~PoOfstream()
{
    if( isOpen() )
    {
       close();
    }
}

void PoOfstream::open(const OString& rFileName, OpenMode aMode )
{
    assert( !isOpen() );
    if( aMode == TRUNC )
    {
        m_aOutPut.open( rFileName.getStr(),
            std::ios_base::out | std::ios_base::trunc );
        m_bIsAfterHeader = false;
    }
    else if( aMode == APP )
    {
        m_aOutPut.open( rFileName.getStr(),
            std::ios_base::out | std::ios_base::app );
        m_bIsAfterHeader = m_aOutPut.tellp() != std::ofstream::pos_type( 0 );
    }
}

void PoOfstream::close()
{
    assert( isOpen() );
    m_aOutPut.close();
}

void PoOfstream::writeHeader(const PoHeader& rPoHeader)
{
    assert( isOpen() && !m_bIsAfterHeader && rPoHeader.m_bIsInitialized );
    rPoHeader.m_pGenPo->writeToFile( m_aOutPut );
    m_bIsAfterHeader = true;
}

void PoOfstream::writeEntry( const PoEntry& rPoEntry )
{
    assert( isOpen() && m_bIsAfterHeader && rPoEntry.m_bIsInitialized );
    rPoEntry.m_pGenPo->writeToFile( m_aOutPut );
}

namespace
{

// Check the validity of read entry
bool lcl_CheckInputEntry(const GenPoEntry& rEntry)
{
    return !rEntry.getReference().empty() &&
           !rEntry.getMsgCtxt().isEmpty() &&
           !rEntry.getMsgId().isEmpty();
}

}

PoIfstream::PoIfstream()
    : m_aInPut()
    , m_bEof( false )
{
}

PoIfstream::PoIfstream(const OString& rFileName)
    : m_aInPut()
    , m_bEof( false )
{
    open( rFileName );
}

PoIfstream::~PoIfstream()
{
    if( isOpen() )
    {
       close();
    }
}

void PoIfstream::open( const OString& rFileName, OString& rPoHeader )
{
    assert( !isOpen() );
    m_aInPut.open( rFileName.getStr(), std::ios_base::in );

    // capture header, updating timestamp and generator
    std::string sTemp;
    std::getline(m_aInPut,sTemp);
    while( !sTemp.empty() && !m_aInPut.eof() )
    {
        std::getline(m_aInPut,sTemp);
        OString sLine = OString(sTemp.data(),sTemp.length());
        if (sLine.startsWith("\"PO-Revision-Date"))
            rPoHeader += "PO-Revision-Date: " + lcl_GetTime() + "\n";
        else if (sLine.startsWith("\"X-Generator"))
            rPoHeader += "X-Generator: LibreOffice\n";
        else if (sLine.startsWith("\""))
            rPoHeader += lcl_GenNormString(sLine);
    }
    m_bEof = false;
}

void PoIfstream::open( const OString& rFileName )
{
    assert( !isOpen() );
    m_aInPut.open( rFileName.getStr(), std::ios_base::in );

    // Skip header
    std::string sTemp;
    std::getline(m_aInPut,sTemp);
    while( !sTemp.empty() && !m_aInPut.eof() )
    {
        std::getline(m_aInPut,sTemp);
    }
    m_bEof = false;
}

void PoIfstream::close()
{
    assert( isOpen() );
    m_aInPut.close();
}

void PoIfstream::readEntry( PoEntry& rPoEntry )
{
    assert( isOpen() && !eof() );
    GenPoEntry aGenPo;
    aGenPo.readFromFile( m_aInPut );
    if( aGenPo.isNull() )
    {
        m_bEof = true;
        rPoEntry = PoEntry();
    }
    else
    {
        if( lcl_CheckInputEntry(aGenPo) )
        {
            if( rPoEntry.m_pGenPo )
            {
                *(rPoEntry.m_pGenPo) = aGenPo;
            }
            else
            {
                rPoEntry.m_pGenPo.reset( new GenPoEntry( aGenPo ) );
            }
            rPoEntry.m_bIsInitialized = true;
        }
        else
        {
            SAL_WARN("l10ntools", "Parse problem with entry: " << aGenPo.getMsgStr());
            throw PoIfstream::Exception();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
