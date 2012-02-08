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

#include <tools/fsys.hxx>
#include <osl/file.hxx>
// local includes
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
#include <comphelper/string.hxx>
#ifdef WNT
#include <windows.h>
#undef CopyFile
#include <direct.h>
#endif

#include "helper.hxx"

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
        fprintf(stdout,"key=%s value=%s\n",x.GetBuffer(),y.getStr());
    }
    fprintf(stdout,"+--------------------------+\n");
}
#endif

HelpParser::HelpParser( const rtl::OString &rHelpFile, bool rUTF8 , bool rHasInputList  )
        : sHelpFile( rHelpFile ),
          bUTF8    ( rUTF8     ),
          bHasInputList( rHasInputList )
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

    rtl::OString fullFilePath = rPrj_in;
    fullFilePath += "\\";
    fullFilePath += makeAbsolutePath( sHelpFile , rRoot_in );
    fullFilePath = fullFilePath.replace('\\', '/');

    rtl::OUString strFullPath(
        rtl::OStringToOUString(fullFilePath, RTL_TEXTENCODING_ASCII_US));

    //printf( "%s\n", fullFilePath.GetBuffer() );
    std::auto_ptr <XMLFile> file ( aParser.Execute( strFullPath , sXmlFile, pXmlFile ) );

    if(file.get() == NULL)
    {
        printf("%s\n", rtl::OUStringToOString(aParser.GetError().sMessage, RTL_TEXTENCODING_ASCII_US).getStr());
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

    rtl::OString sActFileName = makeAbsolutePath( sHelpFile , rRoot_in );

    XMLHashMap*  aXMLStrHM   = file->GetStrings();
    LangHashMap* pElem;
    XMLElement*  pXMLElement  = NULL;

    OUString sOUTimeStamp(
        rtl::OStringToOUString(
            Export::GetTimeStamp(), RTL_TEXTENCODING_ASCII_US));

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
                OUString data = pXMLElement->ToOUString();
                helper::searchAndReplaceAll(
                    &data, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n")),
                    rtl::OUString()); // remove \n
                helper::searchAndReplaceAll(
                    &data, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t")),
                    rtl::OUString()); // remove \t
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
                sBuffer.append( sOUTimeStamp );
                rtl::OString sOut(rtl::OUStringToOString(sBuffer.makeStringAndClear().getStr() , RTL_TEXTENCODING_UTF8));
                if( !data.isEmpty() )
                    aSDFStream << sOut.getStr() << '\n';
                pXMLElement=NULL;
            }else fprintf(stdout,"\nDBG: NullPointer in HelpParser::CreateSDF , Language %s\n",sCur.getStr() );
        }

    }
    aSDFStream.close();

    return sal_True;
}

rtl::OString HelpParser::makeAbsolutePath(const rtl::OString& sHelpFile, const rtl::OString& rRoot_in)
{
    DirEntry aEntry(rtl::OStringToOUString(sHelpFile, RTL_TEXTENCODING_ASCII_US));
    aEntry.ToAbs();
    rtl::OUString sFullEntry(aEntry.GetFull());
    aEntry += DirEntry(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("..")));
    aEntry += DirEntry( rRoot_in );
    rtl::OString sPrjEntry(rtl::OUStringToOString(aEntry.GetFull(), osl_getThreadTextEncoding()));
    rtl::OString sActFileName(rtl::OUStringToOString(
        sFullEntry.copy(sPrjEntry.getLength() + 1),
        osl_getThreadTextEncoding()));

    return sActFileName.replace('/', '\\');
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

    OUString sOUHelpFile( sXmlFile );
    DirEntry aFile( sXmlFile );

    XMLFile* xmlfile = ( aParser.Execute( aFile.GetFull() , sOUHelpFile, new XMLFile( rtl::OUString('0') ) ) );
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

    OUString sOUHelpFile( sXmlFile );
    DirEntry aFile( sXmlFile );

    XMLFile* xmlfile = ( aParser.Execute( aFile.GetFull() , sOUHelpFile, new XMLFile( rtl::OUString('0') ) ) );
    xmlfile->Extract();

    if( xmlfile == NULL)
    {
        printf("%s\n", rtl::OUStringToOString(aParser.GetError().sMessage, RTL_TEXTENCODING_UTF8).getStr());
        exit(-1);
    }


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
        printf("DBG: sHelpFile = %s\n",sHelpFile.GetBuffer() );
#endif

        pResData.sGId      =  pos->first;
        pResData.sFilename  =  sHelpFile;

        ProcessHelp( aLangHM , sLanguage, &pResData , aMergeDataFile );
     }


    // Init temp and target file
    rtl::OString sTempFile;
    rtl::OString sTargetFile( sPath );
    rtl::OString sTempFileCopy;

    static const rtl::OString INPATH = Export::GetEnv("INPATH");
    sTempFile = Export::getRandomName(sPath, INPATH);
    sTempFileCopy = Export::getRandomName(sPath, INPATH);
    // Write in the temp file
    bool hasNoError = file->Write ( sTempFile );
    if( !hasNoError )
    {
        cerr << "ERROR: file->Write failed\n";
        return false;
    }

    DirEntry aTmp( sTempFile );
    DirEntry aTmp2( sTempFileCopy );
    DirEntry aTar( sTargetFile );

    if( !Export::CopyFile( sTempFile , sTempFileCopy ) )
    {
#if defined(UNX)
        sleep( 3 );
#else
        Sleep( 3 );
#endif
        if( !Export::CopyFile( sTempFile , sTempFileCopy ) )
        {
            cerr << "ERROR: Can not copy file from " << sTempFile.getStr() << " to " << sTempFileCopy.getStr() << "\n";
            return false;
        }
    }

    FileStat aFSTest( aTar );
    if( aFSTest.GetSize() < 1 )
    {
        remove( sTargetFile.getStr() );
    }
    int rc;
#if defined(UNX)
    rc = rename( sTempFile.getStr() , sTargetFile.getStr() );
#else
    rc = MoveFileEx( sTempFile.getStr() , sTargetFile.getStr(), MOVEFILE_REPLACE_EXISTING );
#endif
    FileStat aFS( aTar );

    //cout << "mv " << sTempFile.GetBuffer() << " " << sTargetFile.GetBuffer() << "\n";
    //cout << "rc -> " << rc << " filesize -> " << aFS.GetSize() << "\n";
// Windows rename returns -1 if the file already exits
//#ifdef UNX
    if( rc < 0 || aFS.GetSize() < 1 )
//#else
//    if( aFS.GetSize() < 1 )
//#endif
    {
#if defined(UNX)
        sleep( 3 );
#else
        Sleep( 3 );
#endif
        aFSTest.Update( aTar );
        if( aFSTest.GetSize() < 1 )
        {
            remove( sTargetFile.getStr() );
        }
#if defined(UNX)
        rc = rename( sTempFileCopy.getStr() , sTargetFile.getStr() );
#else
        rc = MoveFileEx( sTempFileCopy.getStr() , sTargetFile.getStr() , MOVEFILE_REPLACE_EXISTING );
#endif
        aFS.Update( aTar );

        //cout << "mv2 " << sTempFileCopy.GetBuffer() << " " << sTargetFile.GetBuffer() << "\n";
        //cout << "rc -> " << rc << " filesize -> " << aFS.GetSize() << "\n";

// Windows rename returns -1 if the file already exits
//#ifdef WNT
//        if( aFS.GetSize() < 1 )
//#else
        if( rc < 0 || aFS.GetSize() < 1 )
//#endif
        {
            cerr << "ERROR: helpex Can't rename file " << sTempFileCopy.getStr() << " to " << sTargetFile.getStr() << " rename rc=" << rc << " filesize=" << aFS.GetSize() << "\n";
            aTmp.Kill();
            aTmp2.Kill();
            if( aFS.GetSize() < 1 )
                aTar.Kill();
            return false;
        }
    }
    aTmp.Kill();
    aTmp2.Kill();

    return true;
}

rtl::OString HelpParser::GetOutpath( const rtl::OString& rPathX , const rtl::OString& sCur , const rtl::OString& rPathY )
{
    rtl::OString testpath = rPathX;
    static const rtl::OString sDelimiter(rtl::OUStringToOString(DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US));
    testpath = comphelper::string::stripEnd(testpath, '/');
    testpath = comphelper::string::stripEnd(testpath, '\\');
    testpath += sDelimiter;
    testpath += sCur;
    testpath += sDelimiter;
    rtl::OString sRelativePath( rPathY );
    sRelativePath = comphelper::string::stripStart(sRelativePath, '/');
    sRelativePath = comphelper::string::stripStart(sRelativePath, '\\');
    testpath += sRelativePath;
    testpath += sDelimiter;
    return testpath;
}

void HelpParser::MakeDir(const rtl::OString& rPath)
{
    rtl::OString sDelimiter(rtl::OUStringToOString(DirEntry::GetAccessDelimiter(),
        RTL_TEXTENCODING_ASCII_US));
    rtl::OString sTPath(comphelper::string::replace(rPath, sDelimiter, rtl::OString('/')));
    sal_uInt16 cnt = comphelper::string::getTokenCount(sTPath, '/');
    rtl::OStringBuffer sCreateDir;
    for (sal_uInt16 i = 0; i < cnt; ++i)
    {
        sCreateDir.append(comphelper::string::getToken(sTPath, i , '/'));
        sCreateDir.append(sDelimiter);
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
                        data   = new XMLData( sNewdata , NULL , true ); // Add new one
                        pXMLElement->RemoveAndDeleteAllChildren();
                        pXMLElement->AddChild( data );
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
