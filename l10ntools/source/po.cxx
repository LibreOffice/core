/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <regexp/reclass.hxx>
#include <rtl/ustring.hxx>

#include <cstring>
#include <ctime>
#include <cassert>

#include <vector>
#include <string>

#include <boost/crc.hpp>

#include "po.hxx"

#define POESCAPED OString("\\n\\t\\r\\\\\\\"")
#define POUNESCAPED OString("\n\t\r\\\"")

//Class GenPoEntry

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
    virtual bool        getFuzzy() const        { return m_bFuzzy; }
    virtual bool        isNull() const          { return m_bNull; }

    virtual void        setExtractCom(const OString& rExtractCom);
    virtual void        setReference(const OString& rReference);
    virtual void        setMsgCtxt(const OString& rMsgCtxt);
    virtual void        setMsgId(const OString& rMsgId);
    virtual void        setMsgStr(const OString& rMsgStr);
    virtual void        setFuzzy(const bool bFuzzy);

    virtual void        writeToFile(std::ofstream& rOFStream) const;
    virtual void        readFromFile(std::ifstream& rIFStream);
};

namespace
{
    //Escape text
    static OString lcl_EscapeText(const OString& rText,
                           const OString& rUnEscaped= POUNESCAPED,
                           const OString& rEscaped = POESCAPED)
    {
        assert( rEscaped.getLength() == 2*rUnEscaped.getLength() );
        OString sResult = rText;
        int nCount = 0;
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
    static OString lcl_UnEscapeText(const OString& rText,
                             const OString& rEscaped = POESCAPED,
                             const OString& rUnEscaped = POUNESCAPED)
    {
        assert( rEscaped.getLength() == 2*rUnEscaped.getLength() );
        OString sResult = rText;
        int nCount = 0;
        for(sal_Int32 nIndex=0; nIndex<rText.getLength()-1; ++nIndex)
        {
            sal_Int32 nActChar = rEscaped.indexOf(rText.copy(nIndex,2));
            if(nActChar % 2 == 0)
                sResult = sResult.replaceAt((nIndex++)-(nCount++),2,
                                            rUnEscaped.copy(nActChar/2,1));
        }
        return sResult;
    }

    //Convert a normal string to msg/po output string
    static OString lcl_GenMsgString(const OString& rString)
    {
        if ( rString.isEmpty() )
            return "\"\"";

        OString sResult = "\"" + lcl_EscapeText(rString) + "\"";
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
        return lcl_UnEscapeText(rString.copy(1,rString.getLength()-2));
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

//Set class members
void GenPoEntry::setExtractCom(const OString& rExtractCom)
{
    m_sExtractCom = rExtractCom;
}

void GenPoEntry::setReference(const OString& rReference)
{
    m_sReference = rReference;
}

void GenPoEntry::setMsgCtxt(const OString& rMsgCtxt)
{
    m_sMsgCtxt = rMsgCtxt;
}

void GenPoEntry::setMsgId(const OString& rMsgId)
{
    m_sMsgId = rMsgId;
}

void GenPoEntry::setMsgStr(const OString& rMsgStr)
{
    m_sMsgStr = rMsgStr;
}

void GenPoEntry::setFuzzy(const bool bFuzzy)
{
    m_bFuzzy = bFuzzy;
}

//Write to file
void GenPoEntry::writeToFile(std::ofstream& rOFStream) const
{
    if ( rOFStream.tellp() != 0 )
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
                  << lcl_GenMsgString(m_sMsgCtxt).getStr() << std::endl;
    rOFStream << "msgid "
              << lcl_GenMsgString(m_sMsgId).getStr() << std::endl;
    rOFStream << "msgstr "
              << lcl_GenMsgString(m_sMsgStr).getStr() << std::endl;
}

//Read from file
void GenPoEntry::readFromFile(std::ifstream& rIFStream)
{
    *this = GenPoEntry();
    if( rIFStream.eof() )
    {
        m_bNull = true;
        return;
    }
    OString* pLastMsg = 0;
    std::string sTemp;
    getline(rIFStream,sTemp);
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
            *pLastMsg += lcl_GenNormString(sLine);
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
        //Use all readable ASCII charachter exclude xml special tags: ",',&,<,>
        const OString sSymbols = "!#$%()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
        char sKeyId[5];
        for( short nKeyInd = 0; nKeyInd < 4; ++nKeyInd )
        {
            sKeyId[nKeyInd] = sSymbols[(nCRC & 255) % 89];
            nCRC >>= 8;
        }
        sKeyId[4] = '\0';
        return OString(sKeyId);
    }

    //Split string at the delimiter char
    static void lcl_SplitAt(const OString& rSource, const sal_Char nDelimiter,
                     std::vector<OString>& o_vParts)
    {
        o_vParts.resize( 0 );
        sal_Int32 nActIndex = 0;
        sal_Int32 nLastSplit = 0;
        while( nActIndex < rSource.getLength() )
        {
            if ( rSource[nActIndex] == nDelimiter )
            {
                o_vParts.push_back(
                    rSource.copy(nLastSplit,nActIndex-nLastSplit));
                nLastSplit = nActIndex+1;
            }
            ++nActIndex;
        }
        o_vParts.push_back(rSource.copy(nLastSplit));
    }

    //Unescape sdf string
    static OString lcl_UnEscapeSDFText(
        const OString& rText,const bool bHelpText = false )
    {
        if ( bHelpText )
            return lcl_UnEscapeText(rText,"\\<\\>\\\"\\\\","<>\"\\");
        else
            return lcl_UnEscapeText(rText,"\\n\\t\\r","\n\t\r");
    }

    //Miminize the length of the regular expression result
    static void lcl_Minimize(
        const OUString& rText, Regexpr& io_rRegExp, re_registers& io_rRegs )
    {
        re_registers aPrevRegs;
        const sal_Int32 nStart = io_rRegs.start[0];
        do
        {
            const OUString sTemp = rText.copy(0,io_rRegs.end[0]-1);
            memcpy(
                static_cast<void*>(&aPrevRegs),
                static_cast<void*>(&io_rRegs),
                sizeof(re_registers));
            memset(static_cast<void*>(&io_rRegs), 0, sizeof(re_registers));
            io_rRegExp.set_line(sTemp.getStr(),sTemp.getLength());
            io_rRegExp.re_search(&io_rRegs,nStart);
        } while(io_rRegs.num_of_match);

        memcpy(static_cast<void*>(&io_rRegs),static_cast<void*>(&aPrevRegs),
               sizeof(re_registers));
        io_rRegExp.set_line(rText.getStr(),rText.getLength());
    }

    //Find all special tag in a string using a regular expression
    static void lcl_FindAllTag(
        const OString& rText,std::vector<OString>& o_vFoundTags )
    {
        ::com::sun::star::util::SearchOptions aOptions;
        aOptions.algorithmType =
            ::com::sun::star::util::SearchAlgorithms_REGEXP;
        aOptions.searchFlag =
            ::com::sun::star::util::SearchFlags::NORM_WORD_ONLY;
        aOptions.searchString = "<[/]?[a-z_\\-]+(| +[a-z]+=\".*\") *[/]?>";
        ::com::sun::star::uno::Reference<
             ::com::sun::star::i18n::XExtendedTransliteration > xTrans;

        Regexpr aRegExp(aOptions,xTrans);
        const OUString sTemp(OStringToOUString(rText,RTL_TEXTENCODING_UTF8));
        aRegExp.set_line(sTemp.getStr(),sTemp.getLength());

        re_registers aRegs;
        memset(static_cast<void*>(&aRegs), 0, sizeof(re_registers));
        sal_Int32 nStart = 0;
        o_vFoundTags.resize(0);
        aRegExp.re_search(&aRegs,nStart);
        while(aRegs.num_of_match)
        {
            lcl_Minimize(sTemp,aRegExp,aRegs);
            o_vFoundTags.push_back(
                OUStringToOString(
                    sTemp.copy(aRegs.start[0],aRegs.end[0]-aRegs.start[0]),
                    RTL_TEXTENCODING_UTF8));
            nStart = aRegs.end[0];
            memset(static_cast<void*>(&aRegs), 0, sizeof(re_registers));
            aRegExp.re_search(&aRegs,nStart);
        }
    }

    //Escape special tags
    static OString lcl_EscapeTags( const OString& rText )
    {
        typedef std::vector<OString> StrVec_t;
        const OString vInitializer[] = {
            "ahelp", "link", "item", "emph", "defaultinline",
            "switchinline", "caseinline", "variable",
            "bookmark_value", "image", "embedvar", "alt" };
        const StrVec_t vTagsForEscape( vInitializer,
            vInitializer + sizeof(vInitializer) / sizeof(vInitializer[0]) );
        StrVec_t vFoundTags;
        lcl_FindAllTag(rText,vFoundTags);
        OString sResult = rText;
        for(StrVec_t::const_iterator pFound  = vFoundTags.begin();
            pFound != vFoundTags.end(); ++pFound)
        {
            bool bEscapeThis = false;
            for(StrVec_t::const_iterator pEscape = vTagsForEscape.begin();
                pEscape != vTagsForEscape.end(); ++pEscape)
            {
                if (pFound->startsWith("<" + *pEscape) ||
                    *pFound == "</" + *pEscape + ">")
                {
                    bEscapeThis = true;
                    break;
                }
            }
            if( bEscapeThis || *pFound=="<br/>" ||
                *pFound =="<help-id-missing/>")
            {
                OString sToReplace = "\\<" +
                    pFound->copy(1,pFound->getLength()-2).
                        replaceAll("\"","\\\"") + "\\>";
                sResult = sResult.replaceAll(*pFound, sToReplace);
            }
        }
        return sResult;
    }

    //Escape to get sdf/merge string
    static OString lcl_EscapeSDFText(
        const OString& rText,const bool bHelpText = false )
    {
        if ( bHelpText )
            return lcl_EscapeTags(rText.replaceAll("\\","\\\\"));
        else
            return lcl_EscapeText(rText,"\n\t\r","\\n\\t\\r");
    }
}

//Default constructor
PoEntry::PoEntry()
    : m_pGenPo( 0 )
    , m_bIsInitialized( false )
{
}

//Construct PoEntry from sdfline
PoEntry::PoEntry(const OString& rSDFLine, const TYPE eType)
    : m_pGenPo( 0 )
    , m_bIsInitialized( false )
{
    std::vector<OString> vParts;
    lcl_SplitAt(rSDFLine,'\t',vParts);
    if( vParts.size()!=15 ||
        vParts[SOURCEFILE].isEmpty() ||
        vParts[GROUPID].isEmpty() ||
        vParts[RESOURCETYPE].isEmpty() ||
        vParts[eType].isEmpty() ||
        vParts[HELPTEXT].getLength() == 4 )
    {
        throw INVALIDSDFLINE;
    }

    m_pGenPo = new GenPoEntry();
    m_pGenPo->setReference(vParts[SOURCEFILE].
        copy(vParts[SOURCEFILE].lastIndexOf("\\")+1));

    OString sMsgCtxt =
        vParts[GROUPID] + "\n" +
        (vParts[LOCALID].isEmpty() ? "" : vParts[LOCALID] + "\n") +
        vParts[RESOURCETYPE];
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
    m_pGenPo->setExtractCom(
        ( !vParts[HELPTEXT].isEmpty() ?  vParts[HELPTEXT] + "\n" : "" ) +
        lcl_GenKeyId(
            vParts[SOURCEFILE] + sMsgCtxt + vParts[eType] ) );
    m_pGenPo->setMsgCtxt(sMsgCtxt);
    m_pGenPo->setMsgId(
        lcl_UnEscapeSDFText(
            vParts[eType],vParts[SOURCEFILE].endsWith(".xhp")));
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
bool PoEntry::getFuzzy() const
{
    assert( m_bIsInitialized );
    return m_pGenPo->getFuzzy();
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


//Get translation string in sdf/merge format
OString PoEntry::getMsgId() const
{
    assert( m_bIsInitialized );
    return
        lcl_EscapeSDFText(
            m_pGenPo->getMsgId(), getSourceFile().endsWith(".xhp") );
}

//Get translated string in sdf/merge format
OString PoEntry::getMsgStr() const
{
    assert( m_bIsInitialized );
    return
        lcl_EscapeSDFText(
            m_pGenPo->getMsgStr(), getSourceFile().endsWith(".xhp") );

}

//Set translated string when input is in sdf format
void PoEntry::setMsgStr(const OString& rMsgStr)
{
    assert( m_bIsInitialized );
    m_pGenPo->setMsgStr(
                lcl_UnEscapeSDFText(
                    rMsgStr,getSourceFile().endsWith(".xhp")));
}

//Set fuzzy flag
void PoEntry::setFuzzy(const bool bFuzzy)
{
    assert( m_bIsInitialized );
    m_pGenPo->setFuzzy(bFuzzy);
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

    static OString lcl_ReplaceAttribute(
        const OString& rSource, const OString& rOld, const OString& rNew )
    {
        const sal_Int32 nFirstIndex =
            rSource.indexOf( rOld ) + rOld.getLength()+2;
        const sal_Int32 nCount =
            rSource.indexOf( "\n", nFirstIndex ) - nFirstIndex;
        return rSource.replaceFirst( rSource.copy(nFirstIndex, nCount), rNew );
    }
}

//Default Constructor
PoHeader::PoHeader()
    : m_pGenPo( 0 )
    , m_bIsInitialized( false )
{
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
        "X-Genarator: LibreOffice\n"
        "X-Accelerator_Marker: ~\n"));
    m_bIsInitialized = true;
}


//Constructor for old headers to renew po files
PoHeader::PoHeader(  std::ifstream& rOldPo )
    : m_pGenPo( new GenPoEntry() )
    , m_bIsInitialized( false )
{
    assert( rOldPo.is_open() );
    m_pGenPo->readFromFile( rOldPo );

    const OString sExtractCom = m_pGenPo->getExtractCom();
    m_pGenPo->setExtractCom(
        sExtractCom.copy( 0, sExtractCom.getLength() - 3 ) );

    OString sMsgStr = m_pGenPo->getMsgStr();
    sMsgStr =
        lcl_ReplaceAttribute( sMsgStr, "Report-Msgid-Bugs-To",
            "https://bugs.freedesktop.org/enter_bug.cgi?product="
            "LibreOffice&bug_status=UNCONFIRMED&component=UI" );
    sMsgStr =
        lcl_ReplaceAttribute( sMsgStr, "X-Generator", "LibreOffice" );
    sMsgStr =
        lcl_ReplaceAttribute( sMsgStr, "X-Accelerator-Marker", "~" );
    m_pGenPo->setMsgStr( sMsgStr );
    m_bIsInitialized = true;
}

PoHeader::~PoHeader()
{
    delete m_pGenPo;
}

//Get the language of header
OString PoHeader::getLanguage() const
{
    assert( m_bIsInitialized );
    const OString sLang = "Language: ";
    const OString sMsgStr = m_pGenPo->getMsgStr();
    const sal_Int32 nFirstIndex = sMsgStr.indexOf(sLang)+sLang.getLength();
    const sal_Int32 nCount = sMsgStr.indexOf('\n',nFirstIndex)-nFirstIndex;
    if( nFirstIndex == sLang.getLength()-1 || nCount == -nFirstIndex-1 )
    {
        throw NOLANG;
    }
    return sMsgStr.copy( nFirstIndex, nCount );
}

//Class PoOfstream

PoOfstream::PoOfstream()
    : m_aOutPut()
    , m_bIsAfterHeader( false )
{
}

PoOfstream::~PoOfstream()
{
    if( isOpen() )
    {
       close();
    }
}

void PoOfstream::open(const OString& rFileName)
{
    assert( !isOpen() );
    m_aOutPut.open( rFileName.getStr(),
        std::ios_base::out | std::ios_base::trunc );
    m_bIsAfterHeader = false;
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
    , m_bIsAfterHeader( false )
    , m_bEof( false )
{
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
    m_bIsAfterHeader = false;
    m_bEof = false;
}

void PoIfstream::close()
{
    assert( isOpen() );
    m_aInPut.close();
}

void PoIfstream::readHeader( PoHeader& rPoHeader )
{
    assert( isOpen() && !eof() && !m_bIsAfterHeader );
    GenPoEntry aGenPo;
    aGenPo.readFromFile( m_aInPut );
    if( !aGenPo.getExtractCom().isEmpty() &&
        aGenPo.getMsgId().isEmpty() &&
        !aGenPo.getMsgStr().isEmpty() )
    {
        if( rPoHeader.m_pGenPo )
        {
            *(rPoHeader.m_pGenPo) = aGenPo;
        }
        else
        {
            rPoHeader.m_pGenPo = new GenPoEntry( aGenPo );
        }
        rPoHeader.m_bIsInitialized = true;
        m_bIsAfterHeader = true;
    }
    else
    {
        throw INVALIDHEADER;
    }
}

void PoIfstream::readEntry( PoEntry& rPoEntry )
{
    assert( isOpen() && !eof() && m_bIsAfterHeader );
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
            if( rPoEntry.m_pGenPo )
            {
                *(rPoEntry.m_pGenPo) = aGenPo;
            }
            else
            {
                rPoEntry.m_pGenPo = new GenPoEntry( aGenPo );
            }
            const OString sExtractCom = aGenPo.getExtractCom();
            if( sExtractCom.isEmpty() ||
                ( sExtractCom.getLength() != 4 &&
                    sExtractCom.indexOf("\n") == -1 ) )
            {
                aGenPo.setExtractCom(
                    ( !sExtractCom.isEmpty() ? sExtractCom + "\n" : "" ) +
                    lcl_GenKeyId(
                        aGenPo.getReference() + sMsgCtxt +
                        aGenPo.getMsgId() ) );
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
