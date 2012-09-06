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

#include "sal/config.h"

#include <fstream>
#include <functional>

#include <osl/file.hxx>
#include <sal/log.hxx>

#include <stdio.h>
#include <stdlib.h>
#include "helpmerge.hxx"
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <rtl/strbuf.hxx>
#ifdef WNT
#include <windows.h>
#undef CopyFile
#include <direct.h>
#endif

#include "common.hxx"
#include "helper.hxx"
#include "gsicheck.hxx"

#if OSL_DEBUG_LEVEL > 2
void HelpParser::Dump(XMLHashMap* rElem_in)
{
    for(XMLHashMap::iterator pos = rElem_in->begin();pos != rElem_in->end(); ++pos)
    {
        Dump(pos->second,pos->first);
    }
}

void HelpParser::Dump(LangHashMap* rElem_in,const rtl::OString & sKey_in)
{
    rtl::OString x;
    OString y;
    fprintf(stdout,"+------------%s-----------+\n",sKey_in.getStr() );
    for(LangHashMap::iterator posn=rElem_in->begin();posn!=rElem_in->end();++posn)
    {
        x=posn->first;
        y=posn->second->ToOString();
        fprintf(stdout,"key=%s value=%s\n",x.getStr(),y.getStr());
    }
    fprintf(stdout,"+--------------------------+\n");
}
#endif

HelpParser::HelpParser( const rtl::OString &rHelpFile )
        : sHelpFile( rHelpFile )
          {};

/*****************************************************************************/
bool HelpParser::CreateSDF(
/*****************************************************************************/
    const rtl::OString &rSDFFile_in, const rtl::OString &rPrj_in,const rtl::OString &rRoot_in,
    const rtl::OString &sHelpFile, XMLFile *pXmlFile, const rtl::OString &rGsi1){
    SimpleXMLParser aParser;
    rtl::OUString sXmlFile(
        rtl::OStringToOUString(sHelpFile, RTL_TEXTENCODING_ASCII_US));
    //TODO: explicit BOM handling?

    std::auto_ptr <XMLFile> file ( aParser.Execute( sXmlFile, pXmlFile ) );

    if(file.get() == NULL)
    {
        printf(
            "%s: %s\n",
            sHelpFile.getStr(),
            (rtl::OUStringToOString(
                aParser.GetError().sMessage, RTL_TEXTENCODING_ASCII_US).
             getStr()));
        exit(-1);
    }
    file->Extract();
    if( !file->CheckExportStatus() ){
        return true;
    }
    std::ofstream aSDFStream(
        rSDFFile_in.getStr(), std::ios_base::out | std::ios_base::trunc);

    if (!aSDFStream.is_open()) {
        fprintf(stdout,"Can't open file %s\n",rSDFFile_in.getStr());
        return false;
    }

    rtl::OString sActFileName(
        common::pathnameToken(sHelpFile.getStr(), rRoot_in.getStr()));

    XMLHashMap*  aXMLStrHM   = file->GetStrings();
    LangHashMap* pElem;
    XMLElement*  pXMLElement  = NULL;

    OUStringBuffer sBuffer;
    const OUString sOUPrj( rPrj_in.getStr() , rPrj_in.getLength() , RTL_TEXTENCODING_ASCII_US );
    const OUString sOUActFileName(sActFileName.getStr() , sActFileName.getLength() , RTL_TEXTENCODING_ASCII_US );
    const OUString sOUGsi1( rGsi1.getStr() , rGsi1.getLength() , RTL_TEXTENCODING_ASCII_US );

    Export::InitLanguages( false );
    std::vector<rtl::OString> aLanguages = Export::GetLanguages();

    std::vector<rtl::OString> order = file->getOrder();
    std::vector<rtl::OString>::iterator pos;
    XMLHashMap::iterator posm;

    for( pos = order.begin(); pos != order.end() ; ++pos )
    {
        posm = aXMLStrHM->find( *pos );
        pElem = posm->second;
        rtl::OString sCur;

        for( unsigned int n = 0; n < aLanguages.size(); n++ )
        {
            sCur = aLanguages[ n ];
            pXMLElement = (*pElem)[ sCur ];

            if( pXMLElement != NULL )
            {
                OUString data(
                    pXMLElement->ToOUString().
                    replaceAll(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n")),
                        rtl::OUString()).
                    replaceAll(
                        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t")),
                        rtl::OUString()));
                sBuffer.append( sOUPrj );
                sBuffer.append('\t');
                if ( !rRoot_in.isEmpty())
                    sBuffer.append( sOUActFileName );
                   sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("\t0\t"));
                   sBuffer.append( sOUGsi1 );               //"help";
                   sBuffer.append('\t');
                   rtl::OString sID = posm->first;           // ID
                   sBuffer.append( rtl::OStringToOUString( sID, RTL_TEXTENCODING_UTF8 ) );
                   sBuffer.append('\t');
                rtl::OString sOldRef = pXMLElement->GetOldref(); // oldref
                sBuffer.append( rtl::OStringToOUString(sOldRef, RTL_TEXTENCODING_UTF8 ) );
                sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("\t\t\t0\t"));
                   sBuffer.append( rtl::OStringToOUString( sCur, RTL_TEXTENCODING_UTF8 ) );
                   sBuffer.append('\t');
                sBuffer.append( data );
                sBuffer.appendAscii(RTL_CONSTASCII_STRINGPARAM("\t\t\t\t"));
                rtl::OString sOut(rtl::OUStringToOString(sBuffer.makeStringAndClear().getStr() , RTL_TEXTENCODING_UTF8));
                if( !data.isEmpty() )
                    aSDFStream << sOut.getStr() << '\n';
                pXMLElement=NULL;
            }
            else
            {
                fprintf(stdout,"\nDBG: NullPointer in HelpParser::CreateSDF, Language %s, File %s\n", sCur.getStr(), sHelpFile.getStr());
            }
        }
    }
    aSDFStream.close();

    return sal_True;
}

bool HelpParser::Merge( const rtl::OString &rSDFFile, const rtl::OString &rDestinationFile  ,
    const rtl::OString& rLanguage , MergeDataFile& aMergeDataFile )
{

    (void) rSDFFile;
    bool hasNoError = true;

    SimpleXMLParser aParser;

    rtl::OUString sXmlFile(
        rtl::OStringToOUString(sHelpFile, RTL_TEXTENCODING_ASCII_US));
    //TODO: explicit BOM handling?

    XMLFile* xmlfile = ( aParser.Execute( sXmlFile, new XMLFile( rtl::OUString('0') ) ) );
    hasNoError = MergeSingleFile( xmlfile , aMergeDataFile , rLanguage , rDestinationFile );
    delete xmlfile;
    return hasNoError;
}

void HelpParser::parse_languages( std::vector<rtl::OString>& aLanguages , MergeDataFile& aMergeDataFile ){
    std::vector<rtl::OString> aTmp;

    Export::InitLanguages( false );

    if (Export::sLanguages.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("ALL")))
    {
        aLanguages = aMergeDataFile.GetLanguages();
        aLanguages.push_back(rtl::OString(RTL_CONSTASCII_STRINGPARAM("en-US")));

        if( !Export::sForcedLanguages.isEmpty() )
        {
            std::vector<rtl::OString> aFL = Export::GetForcedLanguages();
            std::copy( aFL.begin() ,
                       aFL.end() ,
                       back_inserter( aLanguages )
                     );
            std::sort(   aLanguages.begin() , aLanguages.end() , std::less< rtl::OString >() );
            std::vector<rtl::OString>::iterator unique_iter =  std::unique( aLanguages.begin() , aLanguages.end() , std::equal_to< rtl::OString >() );
            std::copy( aLanguages.begin() , unique_iter , back_inserter( aTmp ) );
            aLanguages = aTmp;
        }
    }
    else{
        aLanguages = Export::GetLanguages();
    }

}

bool HelpParser::Merge(
    const rtl::OString &rSDFFile, const rtl::OString &rPathX , const rtl::OString &rPathY , bool bISO ,
    const std::vector<rtl::OString>& aLanguages , MergeDataFile& aMergeDataFile , bool bCreateDir )
{


    (void) rSDFFile ;
    bool hasNoError = true;
    SimpleXMLParser aParser;
    rtl::OUString sXmlFile(
        rtl::OStringToOUString(sHelpFile, RTL_TEXTENCODING_ASCII_US));
    //TODO: explicit BOM handling?

    XMLFile* xmlfile = aParser.Execute( sXmlFile, new XMLFile( rtl::OUString('0') ) );

    if( xmlfile == NULL)
    {
        printf("%s\n", rtl::OUStringToOString(aParser.GetError().sMessage, RTL_TEXTENCODING_UTF8).getStr());
        exit(-1);
    }

    xmlfile->Extract();

    rtl::OString sCur;
    for( unsigned int n = 0; n < aLanguages.size(); n++ ){
        sCur = aLanguages[ n ];

        rtl::OString sFilepath;
        if( bISO )  sFilepath = GetOutpath( rPathX , sCur , rPathY );
        else        sFilepath = rPathX;
        if( bCreateDir )
            MakeDir(sFilepath);

        XMLFile* file = new XMLFile( *xmlfile );
        sFilepath += sHelpFile;
        hasNoError = MergeSingleFile( file , aMergeDataFile , sCur , sFilepath );
        delete file;

        if( !hasNoError ) return false;         // Stop on error
     }

    delete xmlfile;
    return hasNoError;
}

bool HelpParser::MergeSingleFile( XMLFile* file , MergeDataFile& aMergeDataFile , const rtl::OString& sLanguage ,
                                  rtl::OString const & sPath )
{
    file->Extract();

       XMLHashMap*   aXMLStrHM     = file->GetStrings();
    LangHashMap*  aLangHM;
    static  ResData pResData( "","","");
    pResData.sResTyp   = "help";

    for(XMLHashMap::iterator pos=aXMLStrHM->begin();pos!=aXMLStrHM->end();++pos)    // Merge every l10n related string
    {

        aLangHM             = pos->second;
#if OSL_DEBUG_LEVEL > 2
        printf("*********************DUMPING HASHMAP***************************************");
        Dump(aXMLStrHM);
        printf("DBG: sHelpFile = %s\n",sHelpFile.getStr() );
#endif

        pResData.sGId      =  pos->first;
        pResData.sFilename  =  sHelpFile;

        ProcessHelp( aLangHM , sLanguage, &pResData , aMergeDataFile );
     }

    file->Write(sPath);
    return true;
}

rtl::OString HelpParser::GetOutpath( const rtl::OString& rPathX , const rtl::OString& sCur , const rtl::OString& rPathY )
{
    rtl::OString testpath = rPathX;
    if (!testpath.endsWithL(RTL_CONSTASCII_STRINGPARAM("/"))) {
        testpath += "/";
    }
    testpath += sCur;
    testpath += "/";
    rtl::OString sRelativePath( rPathY );
    if (sRelativePath.matchL(RTL_CONSTASCII_STRINGPARAM("/"))) {
        sRelativePath = sRelativePath.copy(1);
    }
    testpath += sRelativePath;
    testpath += "/";
    return testpath;
}

void HelpParser::MakeDir(const rtl::OString& rPath)
{
    rtl::OString sTPath(rPath.replaceAll("\\", "/"));
    sal_Int32 cnt = helper::countOccurrences(sTPath, '/');
    rtl::OStringBuffer sCreateDir;
    for (sal_uInt16 i = 0; i <= cnt; ++i)
    {
        sCreateDir.append(sTPath.getToken(i , '/'));
        sCreateDir.append('/');
#ifdef WNT
        _mkdir( sCreateDir.getStr() );
#else
        mkdir( sCreateDir.getStr() , S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
#endif
    }
}


/* ProcessHelp Methode: search for en-US entry and replace it with the current language*/
void HelpParser::ProcessHelp( LangHashMap* aLangHM , const rtl::OString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile ){

    XMLElement*   pXMLElement = NULL;
       PFormEntrys   *pEntrys    = NULL;
    XMLData       *data       = NULL;

    rtl::OString sLId;
    rtl::OString sGId;

    pEntrys = NULL;

    if( !sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")) ){
        pXMLElement = (*aLangHM)[ "en-US" ];
        if( pXMLElement == NULL )
        {
            printf("Error: Can't find en-US entry\n");
        }
        if( pXMLElement != NULL )
        {
            sLId    = pXMLElement->GetOldref();
            pResData->sId     =  sLId;

            pEntrys = aMergeDataFile.GetPFormEntrys( pResData );
            if( pEntrys != NULL)
            {
                rtl::OString sNewText;
                pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur , true );
                rtl::OUString sNewdata(
                    rtl::OStringToOUString(sNewText, RTL_TEXTENCODING_UTF8));
                if (!sNewdata.isEmpty())
                {
                    if( pXMLElement != NULL )
                    {
                        if (check(pEntrys->GetSDF(sCur),pEntrys->GetNLine(sCur)))
                        {
                            data   = new XMLData( sNewdata , NULL , true ); // Add new one
                            pXMLElement->RemoveAndDeleteAllChildren();
                            pXMLElement->AddChild( data );
                        }
                        aLangHM->erase( sCur );
                    }
                }
            }
            else if( pResData == NULL )
            {
                fprintf(stdout,"Can't find GID=%s LID=%s TYP=%s\n",
                    pResData->sGId.getStr(), pResData->sId.getStr(),
                    pResData->sResTyp.getStr());
            }
            pXMLElement->ChangeLanguageTag(
                rtl::OStringToOUString(sCur, RTL_TEXTENCODING_ASCII_US));
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
