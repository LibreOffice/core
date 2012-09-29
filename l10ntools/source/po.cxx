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
    return rString.match(rStart);
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
    , m_bNull( false )
    , m_sKeyId( OString() )
{
}

//Destructor
GenPoEntry::~GenPoEntry()
{
}

//Set class members
void GenPoEntry::setWhiteSpace(const OString& rWhiteSpace)
{
    m_sWhiteSpace = rWhiteSpace;
}

void GenPoEntry::setExtractCom(const OString& rExtractCom)
{
    m_sExtractCom = rExtractCom;
}

void GenPoEntry::setReference(const OString& rReference)
{
    m_sReference = rReference;
}

void GenPoEntry::setContext(const OString& rContext)
{
    m_sContext = rContext;
}

void GenPoEntry::setUnTransStr(const OString& rUnTransStr)
{
    m_sUnTransStr = rUnTransStr;
}

void GenPoEntry::setTransStr(const OString& rTransStr)
{
    m_sTransStr = rTransStr;
}

void GenPoEntry::setFuzzy(const bool bFuzzy)
{
    m_bFuzzy = bFuzzy;
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
    *this = GenPoEntry();
    if( rIFStream.eof() )
    {
        m_bNull = true;
        return;
    }
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
    const OString vInitializer[] = {
        "ahelp", "link", "item", "emph", "defaultinline",
        "switchinline", "caseinline", "variable",
        "bookmark_value", "image", "embedvar", "alt" };
    const StrVec vTagsForEscape( vInitializer,
        vInitializer + sizeof(vInitializer) / sizeof(vInitializer[0]) );
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
    : m_aGenPo( GenPoEntry() )
{
}

//Construct PoEntry from sdfline
PoEntry::PoEntry(const OString& rSDFLine, const TYPE eType)
    : m_aGenPo( GenPoEntry() )
{
    std::vector<OString> vParts;
    ImplSplitAt(rSDFLine,'\t',vParts);
    if(vParts.size()!=15) throw;
    m_aGenPo.setWhiteSpace("\n");

    m_aGenPo.setReference(vParts[SOURCEFILE].
        copy(vParts[SOURCEFILE].lastIndexOf("\\")+1));

    m_aGenPo.setExtractCom(vParts[HELPTEXT]);
    OString sContext = vParts[GROUPID] + "\n" +
        (vParts[LOCALID].isEmpty() ? "" : vParts[LOCALID] + "\n") +
        vParts[RESOURCETYPE];
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
    m_aGenPo.setContext(sContext);
    setUnTransStr(vParts[eType]);
    m_aGenPo.genKeyId();
}

//Destructor
PoEntry::~PoEntry()
{
}

//Get name of file from which entry is extracted
OString PoEntry::getSourceFile() const
{
    return m_aGenPo.getReference();
}

//Get groupid
OString PoEntry::getGroupId() const
{
    return m_aGenPo.getContext().getToken(0,'\n');
}

//Get localid
OString PoEntry::getLocalId() const
{
    const OString sContext = m_aGenPo.getContext();
    if (sContext.indexOf('\n')==sContext.lastIndexOf('\n'))
        return OString();
    else
        return sContext.getToken(1,'\n');
}

//Get the type of component from which entry is extracted
OString PoEntry::getResourceType() const
{
    const OString sContext = m_aGenPo.getContext();
    if (sContext.indexOf('\n')==sContext.lastIndexOf('\n'))
        return sContext.getToken(1,'\n').getToken(0,'.');
    else
        return sContext.getToken(2,'\n').getToken(0,'.');
}

//Get the type of entry
PoEntry::TYPE PoEntry::getType() const
{
    const OString sContext = m_aGenPo.getContext();
    if (sContext.endsWith(".text"))
        return TTEXT;
    else if (sContext.endsWith(".quickhelptext"))
        return TQUICKHELPTEXT;
    else if (sContext.endsWith(".title"))
        return TTITLE;
    else throw;
}

//Check wheather entry is fuzzy
bool PoEntry::getFuzzy() const
{
    return m_aGenPo.getFuzzy();
}

//Check wheather entry is null
bool PoEntry::isNull() const
{
    return m_aGenPo.isNull();
}

//Get keyid
OString PoEntry::getKeyId() const
{
    return m_aGenPo.getKeyId();
}


//Get translation string in sdf/merge format
OString PoEntry::getUnTransStr() const
{
    return ImplEscapeSDFText(m_aGenPo.getUnTransStr(),
                             getSourceFile().endsWith(".xhp"));
}

//Get translated string in sdf/merge format
OString PoEntry::getTransStr() const
{
    return ImplEscapeSDFText(m_aGenPo.getTransStr(),
                             getSourceFile().endsWith(".xhp"));

}

//Set translation string when input is in sdf format
void PoEntry::setUnTransStr(const OString& rUnTransStr)
{
    m_aGenPo.setUnTransStr(
                ImplUnEscapeSDFText(
                    rUnTransStr,getSourceFile().endsWith(".xhp")));
}

//Set translated string when input is in sdf format
void PoEntry::setTransStr(const OString& rTransStr)
{
    m_aGenPo.setTransStr(
                ImplUnEscapeSDFText(
                    rTransStr,getSourceFile().endsWith(".xhp")));
}

//Set fuzzy flag
void PoEntry::setFuzzy(const bool bFuzzy)
{
    m_aGenPo.setFuzzy(bFuzzy);
}

//Write to file
void PoEntry::writeToFile(std::ofstream& rOFStream)
{
    m_aGenPo.writeToFile(rOFStream);
}


//Read from file
void PoEntry::readFromFile(std::ifstream& rIFStream)
{
    *this = PoEntry();
    m_aGenPo.readFromFile(rIFStream);
}

//Check whether po-s belong to the same localization component
bool PoEntry::IsInSameComp(const PoEntry& rPo1,const PoEntry& rPo2)
{
    return ( rPo1.getSourceFile() == rPo2.getSourceFile() &&
             rPo1.getGroupId() == rPo2.getGroupId() &&
             rPo1.getLocalId() == rPo2.getLocalId() &&
             rPo1.getResourceType() == rPo2.getResourceType() );
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

//Default Constructor
PoHeader::PoHeader()
    : m_aGenPo( GenPoEntry() )
{
}

//Template Constructor
PoHeader::PoHeader( const OString& rExtSrc )
    : m_aGenPo( GenPoEntry() )
{
    m_aGenPo.setExtractCom("extracted from " + rExtSrc);
    m_aGenPo.setTransStr(
        OString("Project-Id-Version: PACKAGE VERSION\n"
        "Report-Msgid-Bugs-To: https://bugs.freedesktop.org/enter_bug.cgi?"
        "product=LibreOffice&bug_status=UNCONFIRMED&component=UI\n"
        "POT-Creation-Date: ") + ImplGetTime() +
        OString("\nPO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\n"
        "Last-Translator: FULL NAME <EMAIL@ADDRESS>\n"
        "Language-Team: LANGUAGE <LL@li.org>\n"
        "MIME-Version: 1.0\n"
        "Content-Type: text/plain; charset=UTF-8\n"
        "Content-Transfer-Encoding: 8bit\n"
        "X-Genarator: LibreOffice\n"
        "X-Accelerator_Marker: ~\n"));
}

PoHeader::~PoHeader()
{
}

//Get the language of header
OString PoHeader::getLanguage() const
{
    const OString sLang = "Language: ";
    const OString sTransStr = m_aGenPo.getTransStr();
    const sal_Int32 nFirstIndex = sTransStr.indexOf(sLang)+sLang.getLength();
    const sal_Int32 nCount = sTransStr.indexOf('\n',nFirstIndex)-nFirstIndex;
    return sTransStr.copy(nFirstIndex,nCount);
}

//Write out to file
void PoHeader::writeToFile(std::ofstream& rOFStream)
{
    m_aGenPo.writeToFile(rOFStream);
}

//Read from file
void PoHeader::readFromFile(std::ifstream& rIFStream)
{
    *this = PoHeader();
    m_aGenPo.readFromFile(rIFStream);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
