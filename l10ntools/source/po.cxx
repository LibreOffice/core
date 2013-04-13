/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

#include <cstring>
#include <ctime>
#include <cassert>

#include <vector>
#include <string>

#include <boost/crc.hpp>

#include "po.hxx"
#include "helper.hxx"

/** Container of po entry

    Provide all file operations related to LibreOffice specific
    po entry and store it's attributes.
*/
class GenPoEntry
{
private:

    OString    m_sExtractCom;
    OString    m_sReference;
    OString    m_sMsgCtxt;
    OString    m_sMsgId;
    OString    m_sMsgStr;
    bool       m_bFuzzy;
    bool       m_bNull;

public:

                        GenPoEntry();
    virtual             ~GenPoEntry();
                        //Default copy constructor and copy operator work well

    virtual OString     getExtractCom() const   { return m_sExtractCom; }
    virtual OString     getReference() const    { return m_sReference; }
    virtual OString     getMsgCtxt() const      { return m_sMsgCtxt; }
    virtual OString     getMsgId() const        { return m_sMsgId; }
    virtual OString     getMsgStr() const       { return m_sMsgStr; }
    virtual bool        isFuzzy() const         { return m_bFuzzy; }
    virtual bool        isNull() const          { return m_bNull; }

    virtual void        setExtractCom(const OString& rExtractCom)
                        {
                            m_sExtractCom = rExtractCom;
                        }
    virtual void        setReference(const OString& rReference)
                        {
                            m_sReference = rReference;
                        }
    virtual void        setMsgCtxt(const OString& rMsgCtxt)
                        {
                            m_sMsgCtxt = rMsgCtxt;
                        }
    virtual void        setMsgId(const OString& rMsgId)
                        {
                            m_sMsgId = rMsgId;
                        }
    virtual void        setMsgStr(const OString& rMsgStr)
                        {
                            m_sMsgStr = rMsgStr;
                        }
    virtual void        setFuzzy(const bool bFuzzy)
                        {
                            m_bFuzzy = bFuzzy;
                        }

    virtual void        writeToFile(std::ofstream& rOFStream) const;
    virtual void        readFromFile(std::ifstream& rIFStream);
};

namespace
{
    //Convert a normal string to msg/po output string
    static OString lcl_GenMsgString(const OString& rString)
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
            if( sResult.copy(nIndex-1,3)!="\\\\n" &&
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

    //Convert msg string to normal form
    static OString lcl_GenNormString(const OString& rString)
    {
        return
            helper::unEscapeAll(
                rString.copy(1,rString.getLength()-2),
                "\\n""\\t""\\r""\\\\""\\\"",
                "\n""\t""\r""\\""\"");
    }
}

//Default constructor
GenPoEntry::GenPoEntry()
    : m_sExtractCom( OString() )
    , m_sReference( OString() )
    , m_sMsgCtxt( OString() )
    , m_sMsgId( OString() )
    , m_sMsgStr( OString() )
    , m_bFuzzy( false )
    , m_bNull( false )
{
}

//Destructor
GenPoEntry::~GenPoEntry()
{
}

//Write to file
void GenPoEntry::writeToFile(std::ofstream& rOFStream) const
{
    if ( rOFStream.tellp() != std::ofstream::pos_type( 0 ))
        rOFStream << std::endl;
    if ( !m_sExtractCom.isEmpty() )
        rOFStream
            << "#. "
            << m_sExtractCom.replaceAll("\n","\n#. ").getStr() << std::endl;
    if ( !m_sReference.isEmpty() )
        rOFStream << "#: " << m_sReference.getStr() << std::endl;
    if ( m_bFuzzy )
        rOFStream << "#, fuzzy" << std::endl;
    if ( !m_sMsgCtxt.isEmpty() )
        rOFStream << "msgctxt "
                  << lcl_GenMsgString(m_sReference+"\n"+m_sMsgCtxt).getStr()
                  << std::endl;
    rOFStream << "msgid "
              << lcl_GenMsgString(m_sMsgId).getStr() << std::endl;
    rOFStream << "msgstr "
              << lcl_GenMsgString(m_sMsgStr).getStr() << std::endl;
}

//Read from file
void GenPoEntry::readFromFile(std::ifstream& rIFStream)
{
    *this = GenPoEntry();
    OString* pLastMsg = 0;
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
                m_sExtractCom += "\n";
            }
            m_sExtractCom += sLine.copy(3);
        }
        else if (sLine.startsWith("#: "))
        {
            m_sReference = sLine.copy(3);
        }
        else if (sLine.startsWith("#, fuzzy"))
        {
            m_bFuzzy = true;
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
        else if (sLine.startsWith("msgstr "))
        {
            m_sMsgStr = lcl_GenNormString(sLine.copy(7));
            pLastMsg = &m_sMsgStr;
        }
        else if (sLine.startsWith("\"") && pLastMsg)
        {
            if (pLastMsg != &m_sMsgCtxt || sLine != "\"" + m_sReference + "\\n\"")
            {
                *pLastMsg += lcl_GenNormString(sLine);
            }
        }
        else
            break;
        getline(rIFStream,sTemp);
    }
 }

//Class PoEntry

namespace
{
    //Generate KeyId
    static OString lcl_GenKeyId(const OString& rGenerator)
    {
        boost::crc_32_type aCRC32;
        aCRC32.process_bytes(rGenerator.getStr(), rGenerator.getLength());
        sal_uInt32 nCRC = aCRC32.checksum();
        ///Use simple ASCII characters, exclude I, l, 1 and O, 0 to avoid confusing IDs
        static const OString sSymbols =
            "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789";
        char sKeyId[6];
        for( short nKeyInd = 0; nKeyInd < 5; ++nKeyInd )
        {
            sKeyId[nKeyInd] = sSymbols[(nCRC & 63) % sSymbols.getLength()];
            nCRC >>= 6;
        }
        sKeyId[5] = '\0';
        return OString(sKeyId);
    }
}

//Default constructor
PoEntry::PoEntry()
    : m_pGenPo( 0 )
    , m_bIsInitialized( false )
{
}

PoEntry::PoEntry(
    const OString& rSourceFile, const OString& rResType, const OString& rGroupId,
    const OString& rLocalId, const OString& rHelpText,
    const OString& rText, const TYPE eType )
    : m_pGenPo( 0 )
    , m_bIsInitialized( false )
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

    m_pGenPo = new GenPoEntry();
    m_pGenPo->setReference(rSourceFile.copy(rSourceFile.lastIndexOf("/")+1));

    OString sMsgCtxt =
        rGroupId + "\n" +
        (rLocalId.isEmpty() ? OString( "" ) : rLocalId + "\n") +
        rResType;
    switch(eType){
    case TTEXT:
        sMsgCtxt += ".text"; break;
    case TQUICKHELPTEXT:
        sMsgCtxt += ".quickhelptext"; break;
    case TTITLE:
        sMsgCtxt += ".title"; break;
    /*Default case is unneeded because the type of eType has
      only three element*/
    }
    m_pGenPo->setMsgCtxt(sMsgCtxt);
    m_pGenPo->setMsgId(rText);
    m_pGenPo->setExtractCom(
        ( !rHelpText.isEmpty() ?  rHelpText + "\n" : OString( "" )) +
        lcl_GenKeyId(
            m_pGenPo->getReference() + sMsgCtxt + m_pGenPo->getMsgId() ) );
    m_bIsInitialized = true;
}

//Destructor
PoEntry::~PoEntry()
{
    delete m_pGenPo;
}

//Copy constructor
PoEntry::PoEntry( const PoEntry& rPo )
    : m_pGenPo( rPo.m_pGenPo ? new GenPoEntry( *(rPo.m_pGenPo) ) : 0 )
    , m_bIsInitialized( rPo.m_bIsInitialized )
{
}

//Copy operator
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
            m_pGenPo = new GenPoEntry( *(rPo.m_pGenPo) );
        }
    }
    else
    {
        delete m_pGenPo;
        m_pGenPo = 0;
    }
    m_bIsInitialized = rPo.m_bIsInitialized;
    return *this;
}

//Get name of file from which entry is extracted
OString PoEntry::getSourceFile() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getReference();
}

//Get groupid
OString PoEntry::getGroupId() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getMsgCtxt().getToken(0,'\n');
}

//Get localid
OString PoEntry::getLocalId() const
{
    assert( m_bIsInitialized );
    const OString sMsgCtxt = m_pGenPo->getMsgCtxt();
    if (sMsgCtxt.indexOf('\n')==sMsgCtxt.lastIndexOf('\n'))
        return OString();
    else
        return sMsgCtxt.getToken(1,'\n');
}

//Get the type of component from which entry is extracted
OString PoEntry::getResourceType() const
{
    assert( m_bIsInitialized );
    const OString sMsgCtxt = m_pGenPo->getMsgCtxt();
    if (sMsgCtxt.indexOf('\n')==sMsgCtxt.lastIndexOf('\n'))
        return sMsgCtxt.getToken(1,'\n').getToken(0,'.');
    else
        return sMsgCtxt.getToken(2,'\n').getToken(0,'.');
}

//Get the type of entry
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

//Check wheather entry is fuzzy
bool PoEntry::isFuzzy() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->isFuzzy();
}

//Get keyid
OString PoEntry::getKeyId() const
{
    assert( m_bIsInitialized );
    const OString sExtractCom = m_pGenPo->getExtractCom();
    if( sExtractCom.indexOf("\n") == -1 )
    {
        return sExtractCom;
    }
    else
    {
        return sExtractCom.getToken(1,'\n');
    }
}


//Get translation string in merge format
OString PoEntry::getMsgId() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getMsgId();
}

//Get translated string in merge format
OString PoEntry::getMsgStr() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getMsgStr();

}

//Check whether po-s belong to the same localization component
bool PoEntry::IsInSameComp(const PoEntry& rPo1,const PoEntry& rPo2)
{
    assert( rPo1.m_bIsInitialized && rPo2.m_bIsInitialized );
    return ( rPo1.getSourceFile() == rPo2.getSourceFile() &&
             rPo1.getGroupId() == rPo2.getGroupId() &&
             rPo1.getLocalId() == rPo2.getLocalId() &&
             rPo1.getResourceType() == rPo2.getResourceType() );
}

//Class PoHeader

namespace
{
    //Get actual time in "YEAR-MO-DA HO:MI+ZONE" form
    static OString lcl_GetTime()
    {
        time_t aNow = time(NULL);
        struct tm* pNow = localtime(&aNow);
        char pBuff[50];
        strftime( pBuff, sizeof pBuff, "%Y-%m-%d %H:%M%z", pNow );
        return pBuff;
    }
}

//Template Constructor
PoHeader::PoHeader( const OString& rExtSrc )
    : m_pGenPo( new GenPoEntry() )
    , m_bIsInitialized( false )
{
    m_pGenPo->setExtractCom("extracted from " + rExtSrc);
    m_pGenPo->setMsgStr(
        OString("Project-Id-Version: PACKAGE VERSION\n"
        "Report-Msgid-Bugs-To: https://bugs.freedesktop.org/enter_bug.cgi?"
        "product=LibreOffice&bug_status=UNCONFIRMED&component=UI\n"
        "POT-Creation-Date: ") + lcl_GetTime() +
        OString("\nPO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n"
        "Last-Translator: FULL NAME <EMAIL@ADDRESS>\n"
        "Language-Team: LANGUAGE <LL@li.org>\n"
        "MIME-Version: 1.0\n"
        "Content-Type: text/plain; charset=UTF-8\n"
        "Content-Transfer-Encoding: 8bit\n"
        "X-Generator: LibreOffice\n"
        "X-Accelerator-Marker: ~\n"));
    m_bIsInitialized = true;
}

PoHeader::~PoHeader()
{
    delete m_pGenPo;
}

//Class PoOfstream

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

//Class PoIfstream

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

void PoIfstream::open( const OString& rFileName )
{
    assert( !isOpen() );
    m_aInPut.open( rFileName.getStr(), std::ios_base::in );

    //Skip header
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
        const OString sMsgCtxt = aGenPo.getMsgCtxt();
        const sal_Int32 nFirstEndLine = sMsgCtxt.indexOf('\n');
        const sal_Int32 nLastEndLine = sMsgCtxt.lastIndexOf('\n');
        const sal_Int32 nLastDot = sMsgCtxt.lastIndexOf('.');
        const OString sType = sMsgCtxt.copy( nLastDot + 1 );
        if( !aGenPo.getReference().isEmpty() &&
            nFirstEndLine > 0 &&
            (nLastEndLine == nFirstEndLine ||
                nLastEndLine == sMsgCtxt.indexOf('\n',nFirstEndLine+1)) &&
            nLastDot - nLastEndLine > 1 &&
            (sType == "text" || sType == "quickhelptext" || sType == "title")&&
            !aGenPo.getMsgId().isEmpty() )
        {
            //Generate keyid if po file not includes it
            const OString sExtractCom = aGenPo.getExtractCom();
            if( sExtractCom.isEmpty() ||
                ( sExtractCom.getLength() != 4 &&
                    sExtractCom.indexOf("\n") == -1 ) )
            {
                aGenPo.setExtractCom(
                    ( !sExtractCom.isEmpty() ? sExtractCom + "\n" : OString( "" )) +
                    lcl_GenKeyId(
                        aGenPo.getReference() + sMsgCtxt +
                        aGenPo.getMsgId() ) );
            }
            if( rPoEntry.m_pGenPo )
            {
                *(rPoEntry.m_pGenPo) = aGenPo;
            }
            else
            {
                rPoEntry.m_pGenPo = new GenPoEntry( aGenPo );
            }
            rPoEntry.m_bIsInitialized = true;
        }
        else
        {
            throw INVALIDENTRY;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
