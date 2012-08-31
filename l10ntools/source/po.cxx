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

#include <string>
#include <cstring>
#include <ctime>
#include <vector>
#include <boost/crc.hpp>

#include "po.hxx"

#define POESCAPED OString("\\n\\t\\r\\\\\\\"")
#define POUNESCAPED OString("\n\t\r\\\"")

//Class GenPoEntry

//Generate KeyId
OString ImplGenKeyId(const OString& rGenerator)
{
    boost::crc_32_type aCRC32;
    aCRC32.process_bytes(rGenerator.getStr(), rGenerator.getLength());
    sal_uInt32 nCRC = aCRC32.checksum();
    char sKeyId[5];
    for(int nIndex = 0; nIndex < 4; ++nIndex)
    {
        //Get a char from the [33,126] interval of ASCII
        sKeyId[nIndex] = static_cast<char>((nCRC & 255) % 93 + 33);
        nCRC >>= 8;
    }
    sKeyId[4] = 0;
    return OString(sKeyId);
}

//Escape text
OString ImplEscapeText(const OString& rText,
                       const OString& rUnEscaped= POUNESCAPED,
                       const OString& rEscaped = POESCAPED)
{
    if(rEscaped.getLength()!=2*rUnEscaped.getLength()) throw;
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
OString ImplUnEscapeText(const OString& rText,
                         const OString& rEscaped = POESCAPED,
                         const OString& rUnEscaped = POUNESCAPED)
{
    if(rEscaped.getLength()!=2*rUnEscaped.getLength()) throw;
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
OString ImplGenMsgString(const OString& rString)
{
    if ( rString.isEmpty() )
        return "\"\"";

    OString sResult = "\"" + ImplEscapeText(rString) + "\"";
    sal_Int32 nIndex = 0;
    while((nIndex=sResult.indexOf("\\n",nIndex))!=-1)
    {
        if(sResult.copy(nIndex-1,3)!="\\\\n" && nIndex!=sResult.getLength()-3)
           sResult = sResult.replaceAt(nIndex,2,"\\n\"\n\"");
        ++nIndex;
    }

    if ( sResult.indexOf('\n') != -1 )
        return "\"\"\n" +  sResult;

    return sResult;
}

//Convert msg string to normal form
OString ImplGenNormString(const OString& rString)
{
    return ImplUnEscapeText(rString.copy(1,rString.getLength()-2));
}

//Decide whether a string starts with an other string
bool ImplStartsWith(const OString& rString,const OString& rStart)
{
    return rString.copy(0,rStart.getLength())==rStart;
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
void GenPoEntry::writeToFile(std::ofstream& rOFStream)
{
    if ( !m_sWhiteSpace.isEmpty() )
        rOFStream << m_sWhiteSpace.getStr();
    if ( !m_sExtractCom.isEmpty() )
        rOFStream << "#. " << m_sExtractCom.getStr() << std::endl;
    if ( !m_sKeyId.isEmpty() )
        rOFStream << "#. " << m_sKeyId.getStr() << std::endl;
    if ( !m_sReference.isEmpty() )
        rOFStream << "#: " << m_sReference.getStr() << std::endl;
    if ( m_bFuzzy )
        rOFStream << "#, fuzzy" << std::endl;
    if ( !m_sContext.isEmpty() )
        rOFStream << "msgctxt "
                  << ImplGenMsgString(m_sContext).getStr() << std::endl;
    rOFStream << "msgid "
              << ImplGenMsgString(m_sUnTransStr).getStr() << std::endl;
    rOFStream << "msgstr "
              << ImplGenMsgString(m_sTransStr).getStr() << std::endl;
}

//Read from file
void GenPoEntry::readFromFile(std::ifstream& rIFStream)
{
    m_sWhiteSpace = "\n";
    OString* pLastMsg = 0;
    std::string sTemp;
    getline(rIFStream,sTemp);
    while(!rIFStream.eof())
    {
        OString sLine = OString(sTemp.data(),sTemp.length());
        if (ImplStartsWith(sLine,"#. "))
        {
            if (sLine.getLength()==7)
                m_sKeyId = sLine.copy(3);
            else
                m_sExtractCom = sLine.copy(3);
        }
        else if (ImplStartsWith(sLine,"#: "))
        {
            m_sReference = sLine.copy(3);
        }
        else if (ImplStartsWith(sLine,"#, fuzzy"))
        {
            m_bFuzzy = true;
        }
        else if (ImplStartsWith(sLine,"msgctxt "))
        {
            m_sContext = ImplGenNormString(sLine.copy(8));
            pLastMsg = &m_sContext;
        }
        else if (ImplStartsWith(sLine,"msgid "))
        {
            m_sUnTransStr = ImplGenNormString(sLine.copy(6));
            pLastMsg = &m_sUnTransStr;
        }
        else if (ImplStartsWith(sLine,"msgstr "))
        {
            m_sTransStr = ImplGenNormString(sLine.copy(7));
            pLastMsg = &m_sTransStr;
        }
        else if (ImplStartsWith(sLine,"\"") && pLastMsg)
        {
            *pLastMsg += ImplGenNormString(sLine);
        }
        else
            break;
        getline(rIFStream,sTemp);
    }
    if (m_sKeyId.isEmpty())
        genKeyId();
 }

//Class PoEntry

//Split string at the delimiter char
void ImplSplitAt(const OString& rSource, const sal_Char nDelimiter,
                 std::vector<OString>& o_vParts)
{
    o_vParts.resize( 0 );
    sal_Int32 nActIndex = 0;
    sal_Int32 nLastSplit = 0;
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

//Unescape sdf string
OString ImplUnEscapeSDFText(const OString& rText,const bool bHelpText = false)
{
    if ( bHelpText )
        return ImplUnEscapeText(rText,"\\<\\>\\\"\\\\","<>\"\\");
    else
        return ImplUnEscapeText(rText,"\\n\\t\\r","\n\t\r");
}

//Miminize the length of the regular expression result
void ImplMinimize(const OUString& rText, Regexpr& io_rRegExp, re_registers& io_rRegs)
{
    re_registers aPrevRegs;
    const sal_Int32 nStart = io_rRegs.start[0];
    do
    {
        const OUString sTemp = rText.copy(0,io_rRegs.end[0]-1);
        memcpy(static_cast<void*>(&aPrevRegs), static_cast<void*>(&io_rRegs),
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
void ImplFindAllTag(const OString& rText,std::vector<OString>& o_vFoundTags)
{
    ::com::sun::star::util::SearchOptions aOptions;
    aOptions.algorithmType = ::com::sun::star::util::SearchAlgorithms_REGEXP;
    aOptions.searchFlag = ::com::sun::star::util::SearchFlags::NORM_WORD_ONLY;
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
        ImplMinimize(sTemp,aRegExp,aRegs);
        o_vFoundTags.push_back(
                rText.copy(aRegs.start[0],aRegs.end[0]-aRegs.start[0]));
        nStart = aRegs.end[0];
        memset(static_cast<void*>(&aRegs), 0, sizeof(re_registers));
        aRegExp.re_search(&aRegs,nStart);
    }
}

//Escape special tags
OString ImplEscapeTags(const OString& rText)
{
    typedef std::vector<OString> StrVec;
    const StrVec vTagsForEscape =
                  { "ahelp", "link", "item", "emph", "defaultinline",
                    "switchinline", "caseinline", "variable",
                    "bookmark_value", "image", "embedvar", "alt" };
    StrVec vFoundTags;
    ImplFindAllTag(rText,vFoundTags);
    OString sResult = rText;
    for(StrVec::const_iterator pFound  = vFoundTags.begin();
        pFound != vFoundTags.end(); ++pFound)
    {
        bool bEscapeThis = false;
        for(StrVec::const_iterator pEscape = vTagsForEscape.begin();
            pEscape != vTagsForEscape.end(); ++pEscape)
        {
            if (ImplStartsWith(*pFound,"<" + *pEscape) ||
                *pFound == "</" + *pEscape + ">")
            {
                bEscapeThis = true;
                break;
            }
        }
        if (bEscapeThis || *pFound=="<br/>" || *pFound =="<help-id-missing/>")
        {
            OString sToReplace = "\\<" + pFound->copy(1,pFound->getLength()-2).
                                           replaceAll("\"","\\\"") + "\\>";
            sResult = sResult.replaceAll(*pFound, sToReplace);
        }
    }
    return sResult;
}

//Escape to get sdf/merge string
OString ImplEscapeSDFText(const OString& rText,const bool bHelpText = false)
{
    if ( bHelpText )
        return ImplEscapeTags(rText.replaceAll("\\","\\\\"));
    else
        return ImplEscapeText(rText,"\n\t\r","\\n\\t\\r");
}


//Default constructor
PoEntry::PoEntry()
    : GenPoEntry()
    , m_sSourceFile( OString() )
    , m_sGroupId( OString() )
    , m_sLocalId( OString() )
    , m_sResourceType( OString() )
    , m_eType( TTEXT )
    , m_sHelpText( OString() )
{
}

//Construct PoEntry from sdfline
PoEntry::PoEntry(const OString& rSDFLine, const TYPE eType)
    : m_sSourceFile( OString() )
    , m_sGroupId( OString() )
    , m_sLocalId( OString() )
    , m_sResourceType(OString() )
    , m_eType( TTEXT )
    , m_sHelpText( OString() )
{
    std::vector<OString> vParts;
    ImplSplitAt(rSDFLine,'\t',vParts);
    if(vParts.size()!=15) throw;

    m_sSourceFile = vParts[SOURCEFILE].
                        copy(vParts[SOURCEFILE].lastIndexOf("\\")+1);
    m_sResourceType = vParts[RESOURCETYPE];
    m_sGroupId = vParts[GROUPID];
    m_sLocalId = vParts[LOCALID];
    m_eType = eType;
    m_sHelpText = vParts[HELPTEXT];
    setUnTransStr(vParts[eType]);
}

//Destructor
PoEntry::~PoEntry()
{
}

//Get translation string in sdf/merge format
OString PoEntry::getUnTransStr() const
{
    return ImplEscapeSDFText(GenPoEntry::getUnTransStr(),
                             m_sSourceFile.endsWith(".xhp"));
}

//Get translated string in sdf/merge format
OString PoEntry::getTransStr() const
{
    return ImplEscapeSDFText(GenPoEntry::getTransStr(),
                             m_sSourceFile.endsWith(".xhp"));

}

//Set translation string when input is in sdf format
void PoEntry::setUnTransStr(const OString& rUnTransStr)
{
    GenPoEntry::setUnTransStr(
                    ImplUnEscapeSDFText(
                        rUnTransStr,m_sSourceFile.endsWith(".xhp")));
}

//Set translated string when input is in sdf format
void PoEntry::setTransStr(const OString& rTransStr)
{
    GenPoEntry::setTransStr(
                    ImplUnEscapeSDFText(
                        rTransStr,m_sSourceFile.endsWith(".xhp")));
}

//Write to file
void PoEntry::writeToFile(std::ofstream& rOFStream)
{
    setWhiteSpace("\n");
    setExtractCom(m_sHelpText);
    setReference(m_sSourceFile);

    OString sContext = m_sGroupId + "\n" +
                       (m_sLocalId.isEmpty() ? "" : m_sLocalId + "\n") +
                       m_sResourceType;
    switch(m_eType){
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
    genKeyId();
    GenPoEntry::writeToFile(rOFStream);
}


//Read from file
void PoEntry::readFromFile(std::ifstream& rIFStream)
{
    GenPoEntry::readFromFile(rIFStream);
    m_sSourceFile = getReference();

    OString sContext =  getContext();
    m_sGroupId = sContext.getToken(0,'\n');

    if (sContext.indexOf('\n')==sContext.lastIndexOf('\n'))
        m_sResourceType = sContext.getToken(1,'\n').getToken(0,'.');
    else
    {
        m_sLocalId = sContext.getToken(1,'\n');
        m_sResourceType = sContext.getToken(2,'\n').getToken(0,'.');
    }
    if (sContext.endsWith(".text"))
        m_eType = TTEXT;
    else if (sContext.endsWith(".quickhelptext"))
        m_eType = TQUICKHELPTEXT;
    else if (sContext.endsWith(".title"))
        m_eType = TTITLE;
    else
        throw;

    m_sHelpText = getExtractCom();
}

//Class PoHeader

//Get actual time in "YEAR-MO-DA HO:MI+ZONE" form
OString ImplGetTime()
{
    time_t aNow = time(NULL);
    struct tm* pNow = localtime(&aNow);
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
