/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: helpmerge.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 14:30:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#include <tools/fsys.hxx>

// local includes
#include "helpmerge.hxx"
#include "utf8conv.hxx"
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>

/*****************************************************************************/
void HelpParser::FillInFallbacks( LangHashMap& rElem_out, //int nLangIdx_in ){
                                                            ByteString sLangIdx_in ){
/*****************************************************************************/
    const ByteString ENGLISH_LANGUAGECODE( "en-US" );
    const ByteString GERMAN_LANGUAGECODE ( "de"    );
    ByteString sCur;
    XMLElement* pTmp     = NULL;
    XMLElement* pTmp2    = NULL;
    //int nReallang        = Export::LangId[nLangIdx_in];   // Mapping index <-> numeric iso code

    XMLUtil& rXMLUtil = XMLUtil::Instance();   // Get Singleton

    //USHORT nFallback = Export::GetFallbackLanguage( nReallang );
    sCur = sLangIdx_in;
    ByteString sFallback( sCur );
    GetIsoFallback( sFallback );
    if( (rElem_out.find( sFallback ) != rElem_out.end()) && rElem_out[ sFallback ] != NULL ){
        pTmp2 = rElem_out[ sFallback ];
        pTmp = new XMLElement( *pTmp2 )  ; // Copy
        pTmp->SetPos( pTmp2->GetPos()+1 );
        //pTmp->ChangeLanguageTag( String( rXMLUtil.GetIsoLangByIndex( sLangIdx_in ) ,
        pTmp->ChangeLanguageTag( String( sLangIdx_in  ,
                                         RTL_TEXTENCODING_ASCII_US) );
          //rElem_out[ nReallang ] = pTmp;
        rElem_out[ sLangIdx_in ] = pTmp;
        pTmp2 = NULL;
    }
    else if( (rElem_out.find( ENGLISH_LANGUAGECODE ) != rElem_out.end()) && rElem_out[ ENGLISH_LANGUAGECODE ] != NULL ){// No English
        pTmp2 = rElem_out[ ENGLISH_LANGUAGECODE ];
        pTmp = new XMLElement( *pTmp2 )  ; // Copy
        pTmp->SetPos( pTmp2->GetPos()+1 );
        //pTmp->ChangeLanguageTag( String( rXMLUtil.GetIsoLangByIndex( sLangIdx_in ) ,
        pTmp->ChangeLanguageTag( String( sLangIdx_in  ,
                                         RTL_TEXTENCODING_ASCII_US) );
          //rElem_out[ nReallang ] = pTmp;
        rElem_out[ sCur ] = pTmp;
        pTmp2 = NULL;
    }
    else if( (rElem_out.find( GERMAN_LANGUAGECODE ) != rElem_out.end() ) && rElem_out[ GERMAN_LANGUAGECODE ] != NULL ){// No English
        pTmp2 = rElem_out[ GERMAN_LANGUAGECODE ];
        pTmp = new XMLElement( *pTmp2 ); // Copy
        pTmp->SetPos( pTmp2->GetPos()+1 );
        //pTmp->ChangeLanguageTag( String( rXMLUtil.GetIsoLangByIndex( sLangIdx_in ),
        pTmp->ChangeLanguageTag( String( sLangIdx_in ,
                                 RTL_TEXTENCODING_ASCII_US ) );
    //    rElem_out[ nReallang ] = pTmp;
        rElem_out[ sCur ] = pTmp;
        pTmp2 = NULL;
    }else{
        //fprintf(stdout,"ERROR: No Fallback found for language %d:\n",nReallang);
        fprintf(stdout,"ERROR: No Fallback found for language %d:\n",sCur.GetBuffer());
        //rElem_out[ nReallang ]=new XMLElement(); // Use dummy element
        rElem_out[ sCur ]=new XMLElement(); // Use dummy element
    }
}

/*****************************************************************************/
void HelpParser::Dump(XMLHashMap* rElem_in) {
/*****************************************************************************/
    for(XMLHashMap::iterator pos = rElem_in->begin();pos != rElem_in->end(); ++pos){
        Dump(pos->second,pos->first);
    }
}
/*****************************************************************************/
void HelpParser::Dump(LangHashMap* rElem_in,const ByteString sKey_in) {
/*****************************************************************************/
    //int x;
    ByteString x;
    OString y;
    fprintf(stdout,"+------------%s-----------+\n",sKey_in.GetBuffer() );
    for(LangHashMap::iterator posn=rElem_in->begin();posn!=rElem_in->end();++posn){
        x=posn->first;
        y=posn->second->ToOString();
        fprintf(stdout,"key=%s value=%s\n",x.GetBuffer(),y.getStr());
    }
    fprintf(stdout,"+--------------------------+\n");
}

HelpParser::HelpParser( const ByteString &rHelpFile, bool bUTF8 )
        : sHelpFile( rHelpFile ),
          bUTF8    ( bUTF8     ) {};

/*****************************************************************************/
bool HelpParser::CreateSDF(
/*****************************************************************************/
    const ByteString &rSDFFile_in, const ByteString &rPrj_in,const ByteString &rRoot_in ){
    // GSI File constants
    static const String GSI_SEQUENCE1( String::CreateFromAscii("\t0\thelp\t")   );
    static const String GSI_SEQUENCE2( String::CreateFromAscii("\t\t\t0\t")     );
    static const String GSI_TAB      ( String::CreateFromAscii("\t")            );
    static const String GSI_SEQUENCE4( String::CreateFromAscii("\t\t\t\t")      );
    static const String ret          ( String::CreateFromAscii("\n")            );
    static const String ret_char     ( String::CreateFromAscii("")              );
    static const String tab          ( String::CreateFromAscii("\t")            );
    static const String tab_char     ( String::CreateFromAscii("")              );

    SimpleXMLParser aParser;
    String sUsedTempFile;
    String sXmlFile;

    //String x(sHelpFile.GetBuffer(),RTL_TEXTENCODING_UTF8 , sHelpFile.Len());
    if( Export::fileHasUTF8ByteOrderMarker( sHelpFile ) ){
        DirEntry aTempFile = Export::GetTempFile();
        DirEntry aSourceFile( String( sHelpFile , RTL_TEXTENCODING_ASCII_US ) );
        aSourceFile.CopyTo( aTempFile , FSYS_ACTION_COPYFILE );
        String sTempFile = aTempFile.GetFull();
        Export::RemoveUTF8ByteOrderMarkerFromFile( ByteString( sTempFile , RTL_TEXTENCODING_ASCII_US ) );
        sUsedTempFile = sTempFile;
        sXmlFile = sTempFile;
    }else{
        sUsedTempFile = String::CreateFromAscii("");
        sXmlFile = String( sHelpFile , RTL_TEXTENCODING_ASCII_US );
    }


    //String x(sHelpFile,RTL_TEXTENCODING_ASCII_US );
    //std::auto_ptr <XMLFile> file ( aParser.Execute(x) );
    std::auto_ptr <XMLFile> file ( aParser.Execute( sXmlFile ) );

    if(file.get() == NULL){
        //printf("%s\n",ByteString(aParser.GetError().sMessage,RTL_TEXTENCODING_UTF8).GetBuffer());
        printf("%s\n",ByteString(aParser.GetError().sMessage,RTL_TEXTENCODING_ASCII_US).GetBuffer());
        exit(-1);
        return false;
    }
    file->Extract();
    if( !file->CheckExportStatus() ){
        return true;
    }
    SvFileStream aSDFStream( String( rSDFFile_in, RTL_TEXTENCODING_ASCII_US ),
        STREAM_STD_WRITE | STREAM_TRUNC );
    //aSDFStream.SetLineDelimiter( LINEEND_LF );
    if ( !aSDFStream.IsOpen()) {
        fprintf(stdout,"Can't open file %s\n",rSDFFile_in.GetBuffer());
        return false;
    }

      DirEntry aEntry( String( sHelpFile, RTL_TEXTENCODING_ASCII_US ));
    aEntry.ToAbs();
    String sFullEntry = aEntry.GetFull();
    aEntry += DirEntry( String( "..", RTL_TEXTENCODING_ASCII_US ));
    aEntry += DirEntry( rRoot_in );
    ByteString sPrjEntry( aEntry.GetFull(), gsl_getSystemTextEncoding());
    ByteString sActFileName(
    sFullEntry.Copy( sPrjEntry.Len() + 1 ), gsl_getSystemTextEncoding());
//  sActFileName.ToLowerAscii();

    sActFileName.SearchAndReplaceAll( "/", "\\" );

    XMLHashMap*  aXMLStrHM   = file->GetStrings();
    LangHashMap* pElem;
    XMLElement*  pXMLElement  = NULL;

    //Dump(aXMLStrHM);

    ByteString sTimeStamp( Export::GetTimeStamp() );
    OUString sOUTimeStamp( sTimeStamp.GetBuffer() , sTimeStamp.Len() , RTL_TEXTENCODING_ASCII_US );

    short nCurLang=0;
    OUStringBuffer sBuffer;
    XMLUtil& xmlutil=XMLUtil::Instance();
    const OUString sOUPrj( rPrj_in.GetBuffer() , rPrj_in.Len() , RTL_TEXTENCODING_ASCII_US );
    const OUString sOUActFileName(sActFileName.GetBuffer() , sActFileName.Len() , RTL_TEXTENCODING_ASCII_US );



    Export::InitLanguages( false );
    std::vector<ByteString> aLanguages = Export::GetLanguages();

    //for(XMLHashMap::iterator pos=aXMLStrHM->begin();pos!=aXMLStrHM->end();++pos){
    std::vector<ByteString> order = file->getOrder();
    std::vector<ByteString>::iterator pos;
    XMLHashMap::iterator posm;

    for( pos = order.begin(); pos != order.end() ; ++pos ){
        //ByteString sKey = *pos;
        posm = aXMLStrHM->find( *pos );
        pElem = posm->second;
        //pElem = aXMLStrHM[ sKey ];
        //pElem=pos->second;
        ByteString sCur;
        for( long int n = 0; n < aLanguages.size(); n++ ){
                sCur = aLanguages[ n ];

                if(pElem->find( sCur )==pElem->end()){

                    FillInFallbacks( *pElem , sCur );
                }

                pXMLElement = (*pElem)[ sCur ];
                  if( pXMLElement != NULL ){
                    OUString data = pXMLElement->ToOUString();
                       String sTmp = String(data.getStr());
                    sTmp.SearchAndReplaceAll(ret,ret_char);    // Remove \n
                    sTmp.SearchAndReplaceAll(tab,tab_char);    // Remove \t

                    data = OUString( sTmp );
                    sBuffer.append( sOUPrj );
                    sBuffer.append( GSI_TAB );              //"\t";
                    if ( rRoot_in.Len())
                        sBuffer.append( sOUActFileName );
                       sBuffer.append( GSI_SEQUENCE1 );     //"\t0\thelp\t";
                    ByteString sID = posm->first;           // ID
                    sBuffer.append( OUString( sID.GetBuffer() , sID.Len() , RTL_TEXTENCODING_UTF8 ) );
                    sBuffer.append( GSI_TAB ); //"\t";
                       ByteString sOldRef = pXMLElement->GetOldref(); // oldref
                    sBuffer.append( OUString(sOldRef.GetBuffer() , sOldRef.Len() , RTL_TEXTENCODING_UTF8 ) );
                       sBuffer.append( GSI_SEQUENCE2 );     //"\t\t\t0\t";
                    //sBuffer.append( String::CreateFromInt64( Export::LangId[ i ] ) );
                    sBuffer.append( OUString( sCur.GetBuffer() , sCur.Len() , RTL_TEXTENCODING_UTF8 ) );
                       sBuffer.append( GSI_TAB );               //"\t";
                    sBuffer.append( data );
                      sBuffer.append( GSI_SEQUENCE4 );      //"\t\t\t\t";
                    sBuffer.append( sOUTimeStamp );
                    ByteString sOut( sBuffer.makeStringAndClear().getStr() , RTL_TEXTENCODING_UTF8 );
                    if( !sCur.EqualsIgnoreCaseAscii("de") ||( sCur.EqualsIgnoreCaseAscii("de") && !Export::isMergingGermanAllowed( rPrj_in ) ) ){
                        if( data.getLength() > 0 ) aSDFStream.WriteLine( sOut );
                    }
                    pXMLElement=NULL;
                }else fprintf(stdout,"\nDBG: NullPointer in HelpParser::CreateSDF , Language %s\n",sCur.GetBuffer() );
            }
    //  }
    }
    //Dump(aXMLStrHM);
    aSDFStream.Close();
    //fprintf(stdout,"Closing stream ...");

    if( !sUsedTempFile.EqualsIgnoreCaseAscii( "" ) ){
        DirEntry aTempFile( sUsedTempFile );
        aTempFile.Kill();
    }
    return TRUE;
}
bool HelpParser::Merge( const ByteString &rSDFFile, const ByteString &rDestinationFile )
{
    Merge( rSDFFile , rDestinationFile , ByteString("") , false );
    return true;
}

bool ByteStringEqual( const ByteString& rKey1, const ByteString& rKey2 )  {
    return rKey1.CompareTo( rKey2 )==COMPARE_EQUAL;
};
bool ByteStringLess( const ByteString& rKey1, const ByteString& rKey2 )  {
     return rKey1.CompareTo( rKey2 )==COMPARE_LESS;
}

/*****************************************************************************/
bool HelpParser::Merge(
    const ByteString &rSDFFile, const ByteString &rPathX , const ByteString &rPathY , bool bISO )
/*****************************************************************************/
{

    SimpleXMLParser aParser;

    String sUsedTempFile;
    String sXmlFile;

    //String x(sHelpFile.GetBuffer(),RTL_TEXTENCODING_UTF8 , sHelpFile.Len());
    if( Export::fileHasUTF8ByteOrderMarker( sHelpFile ) ){
        DirEntry aTempFile = Export::GetTempFile();
        DirEntry aSourceFile( String( sHelpFile , RTL_TEXTENCODING_ASCII_US ) );
        aSourceFile.CopyTo( aTempFile , FSYS_ACTION_COPYFILE );
        String sTempFile = aTempFile.GetFull();
        Export::RemoveUTF8ByteOrderMarkerFromFile( ByteString( sTempFile , RTL_TEXTENCODING_ASCII_US ) );
        sUsedTempFile = sTempFile;
        sXmlFile = sTempFile;
    }else{
        sUsedTempFile = String::CreateFromAscii("");
        sXmlFile = String( sHelpFile , RTL_TEXTENCODING_ASCII_US );
    }


    //OUString sOUHelpFile( sHelpFile.GetBuffer(),sHelpFile.Len(),RTL_TEXTENCODING_UTF8);
    OUString sOUHelpFile( sXmlFile );

    std::auto_ptr <XMLFile> xmlfile ( aParser.Execute( sOUHelpFile ) );

    if( xmlfile.get() == NULL){
        printf("%s\n",ByteString(aParser.GetError().sMessage,RTL_TEXTENCODING_UTF8).GetBuffer());
        exit(-1);
        return false;
    }
    std::vector<ByteString> aLanguages , aTmp;

    MergeDataFile aMergeDataFile( rSDFFile, sHelpFile , FALSE, RTL_TEXTENCODING_MS_1252, false );
    Export::InitLanguages( false );
    if( Export::sLanguages.EqualsIgnoreCaseAscii( "ALL" ) ){
            aLanguages = aMergeDataFile.GetLanguages();
            aLanguages.push_back( ByteString("de") );
            aLanguages.push_back( ByteString("en-US") );
            if( !Export::sForcedLanguages.Equals("") ){

                std::vector<ByteString> aFL = Export::GetForcedLanguages();
                std::copy( aFL.begin() ,
                           aFL.end() ,
                           back_inserter( aLanguages )
                          );
                std::sort(   aLanguages.begin() , aLanguages.end() , ByteStringLess );
                std::vector<ByteString>::iterator unique_iter =  std::unique( aLanguages.begin() , aLanguages.end() , ByteStringEqual );
                std::copy( aLanguages.begin() , unique_iter , back_inserter( aTmp ) );
                aLanguages = aTmp;
            }
    }
    else{
        aLanguages = Export::GetLanguages();
    }
        ByteString sCur;
        for( long int n = 0; n < aLanguages.size(); n++ ){
            sCur = aLanguages[ n ];

            ByteString testpath;
            if( bISO ){
                testpath = GetOutpath( rPathX , sCur , rPathY );
            //    MakeDir( testpath );
            }
            else       testpath = rPathX;
            // Test
            MakeDir( testpath );
            // Test

            XMLFile* pFile = new XMLFile( *xmlfile );// copy new()
            std::auto_ptr <XMLFile> file ( pFile );
            file->Extract();
            //Dump( file->GetStrings() );

            XMLHashMap*   aXMLStrHM   = file->GetStrings();
            LangHashMap*  aLangHM;
            ResData       *pResData   = NULL;
            ByteString sTmp = Export::sLanguages;
            sTmp.EraseLeadingAndTrailingChars();
            for(XMLHashMap::iterator pos=aXMLStrHM->begin();pos!=aXMLStrHM->end();++pos){
                aLangHM = pos->second;

                pResData = new ResData( "", pos->first );
                pResData->sResTyp = "help";
                pResData->sGId    =  pos->first;
                ProcessHelp( aLangHM , sCur , pResData , aMergeDataFile );
            }

            String test( testpath , RTL_TEXTENCODING_ASCII_US ); // check and remove '\\'
            file->Write(test); // Always write!
        }
    if( !sUsedTempFile.EqualsIgnoreCaseAscii( "" ) ){
        DirEntry aTempFile( sUsedTempFile );
        aTempFile.Kill();
    }

    return true;
}

ByteString HelpParser::GetOutpath( const ByteString& rPathX , const ByteString& sCur , const ByteString& rPathY ){
    ByteString testpath = rPathX;
    ByteString sDelimiter( DirEntry::GetAccessDelimiter(), RTL_TEXTENCODING_ASCII_US );
    testpath.EraseTrailingChars( '/' );
    testpath.EraseTrailingChars( '\\' );
    testpath += sDelimiter;
    testpath += sCur;
    testpath += sDelimiter;
    ByteString sRelativePath( rPathY );
    sRelativePath.EraseLeadingChars( '/' );
    sRelativePath.EraseLeadingChars( '\\' );
    testpath += sRelativePath;
    return testpath;
}
void HelpParser::MakeDir( const ByteString& sPath ){
    String sPathtmp( sPath , RTL_TEXTENCODING_ASCII_US );
    String sDir( sPathtmp.Copy( 0 , sPathtmp.SearchCharBackward( DirEntry::GetAccessDelimiter().GetBuffer() ) ) );
    DirEntry aDirEntry( sDir );

    ByteString sTDir( sDir , sDir.Len() , RTL_TEXTENCODING_ASCII_US );
    if( aDirEntry.MakeDir() ){
    //    printf("ERROR: Could NOT create Directory %s\n",sTDir.GetBuffer() );
    //    exit( -1 );
    }

}

/*****************************************************************************/
//bool HelpParser::Merge(
//  const ByteString &rSDFFile, const ByteString &rDestinationFile )
/*****************************************************************************/
/*{

    SimpleXMLParser aParser;
    OUString sOUHelpFile( sHelpFile.GetBuffer(),sHelpFile.Len(),RTL_TEXTENCODING_UTF8);
    std::auto_ptr <XMLFile> file ( aParser.Execute( sOUHelpFile ) );

    if(file.get() == NULL){
        printf("%s\n",ByteString(aParser.GetError().sMessage,RTL_TEXTENCODING_UTF8).GetBuffer());
        exit(-1);
        return false;
    }

    file->Extract();

    MergeDataFile aMergeDataFile( rSDFFile, FALSE, RTL_TEXTENCODING_MS_1252, false );

    XMLHashMap*   aXMLStrHM   = file->GetStrings();
    LangHashMap*  aLangHM;
    ResData       *pResData   = NULL;
    ByteString sTmp = Export::sLanguages;
    sTmp.EraseLeadingAndTrailingChars();
    bool bAll = sTmp.ToUpperAscii().Equals("ALL");

    for(XMLHashMap::iterator pos=aXMLStrHM->begin();pos!=aXMLStrHM->end();++pos){
        aLangHM = pos->second;
        ByteString sCur;

        if( bAll ){
            int nSize = aMergeDataFile.LanguagesCnt();
            for( ByteStringSet::const_iterator posl = aMergeDataFile.LanguagesBeginIter();
                 posl != aMergeDataFile.LanguagesEndIter(); ++posl ){
                sCur = *posl;
                pResData = new ResData( "", pos->first );
                pResData->sResTyp = "help";
                pResData->sGId    =  pos->first;
                Process( aLangHM , sCur , pResData , aMergeDataFile );
            }
        }
        else{
            for( ByteStringBoolHashMap::const_iterator posl = Export::AllLanguagesBeginIter() ;
                 posl != Export::AllLanguagesEndIter(); ++posl){
                sCur = posl->first;
                pResData = new ResData( "", pos->first );
                pResData->sResTyp = "help";
                pResData->sGId    =  pos->first;
                Process( aLangHM , sCur , pResData , aMergeDataFile );
            }
        }
    }

    String test(rDestinationFile.GetBuffer(),rDestinationFile.Len(),RTL_TEXTENCODING_UTF8);
    //test.Append( String::CreateFromAscii(".tmp") );
    file->Write(test); // Always write!
    //Dump(aXMLStrHM);
    return true;
}   */
/* ProcessHelp Methode: search for en-US entry and replace it with the current language*/
void HelpParser::ProcessHelp( LangHashMap* aLangHM , ByteString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile ){

    XMLElement*   pXMLElement = NULL;
       PFormEntrys   *pEntrys    = NULL;
    XMLData       *data       = NULL;
    XMLParentNode *parent     = NULL;
    XMLDefault    *xmldefault = NULL;

    short         curLang     = 0;
    String        sNewdata;
    bool          isFallback  = false;
    ULONG nPos = 0;
    ByteString sLId;
    ByteString sGId;

    pEntrys = NULL;

#ifdef MERGE_SOURCE_LANGUAGES
    if( true ){                  // Merge en-US!
#else
    if( !sCur.EqualsIgnoreCaseAscii("en-US") ){
#endif
        pXMLElement = (*aLangHM)[ "en-US" ];
        if( pXMLElement == NULL ){
            printf("Error: Can't find en-US entry");
        }
        if( pXMLElement != NULL ){
            parent  = pXMLElement->GetParent();
            sLId    = pXMLElement->GetOldref();
            pResData->sId     =  sLId;

            pEntrys = aMergeDataFile.GetPFormEntrys( pResData );
            if( pEntrys != NULL) {
                ByteString sNewText;
                pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur , true );
                sNewdata = String(  sNewText , RTL_TEXTENCODING_UTF8 );
                if ( sNewdata.Len()) {
                    if( pXMLElement != NULL ){
                        data   = new XMLData( sNewdata , NULL , true ); // Add new one
                        pXMLElement->RemoveAndDeleteAllChilds();
                        pXMLElement->AddChild( data );
              //          pXMLElement->ChangeLanguageTag( String( sCur , RTL_TEXTENCODING_ASCII_US) );
                        aLangHM->erase( sCur );
                    }
                } else
            //      pXMLElement->ChangeLanguageTag( String( sCur , RTL_TEXTENCODING_ASCII_US) );

                delete pResData;
            }else if( pResData == NULL ){fprintf(stdout,"Can't find GID=%s LID=%s TYP=%s\n",pResData->sGId.GetBuffer(),pResData->sId.GetBuffer(),pResData->sResTyp.GetBuffer());}
            pXMLElement->ChangeLanguageTag( String( sCur , RTL_TEXTENCODING_ASCII_US) );

        }

    }
}
/* Process() Method merges */
void HelpParser::Process( LangHashMap* aLangHM , ByteString& sCur , ResData *pResData , MergeDataFile& aMergeDataFile ){

    XMLElement*   pXMLElement = NULL;
       PFormEntrys   *pEntrys    = NULL;
    XMLData       *data       = NULL;
    XMLParentNode *parent     = NULL;
    XMLDefault    *xmldefault = NULL;

    short         curLang     = 0;
    String        sNewdata;
    bool          isFallback  = false;
    ULONG nPos = 0;
    ByteString sLId;
    ByteString sGId;

    pEntrys = NULL;

#ifdef MERGE_SOURCE_LANGUAGES
    if( true ){                  // Merge en-US!
#else
    if( !sCur.EqualsIgnoreCaseAscii("en-US") ){
#endif
        pXMLElement = (*aLangHM)[ sCur ];
        if( pXMLElement == NULL ){
            FillInFallbacks( *aLangHM , sCur );
            pXMLElement =   ( *aLangHM )[ sCur ];
            isFallback = true;
        }
        if( pXMLElement != NULL ){
            parent  = pXMLElement->GetParent();
            sLId    = pXMLElement->GetOldref();
            pResData->sId     =  sLId;

            pEntrys = aMergeDataFile.GetPFormEntrys( pResData );
            if( pEntrys != NULL) {
                ByteString sNewText;
                pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur , true );
                sNewdata = String(  sNewText , RTL_TEXTENCODING_UTF8 );
                if ( sNewdata.Len()) {
                    printf("Entries found\n");
                    if( pXMLElement != NULL ){
                        data   = new XMLData( sNewdata , NULL , true ); // Add new one
                        if( pXMLElement->ToOUString().compareTo( OUString(data->GetData()) ) != 0 ){
                            pXMLElement->RemoveAndDeleteAllChilds();
                            pXMLElement->AddChild( data );
                        }
                        if( isFallback ){
                            xmldefault = new XMLDefault( String::CreateFromAscii("\n") , NULL );
                            int pos = parent->GetPos( pXMLElement->GetId() );
                            if( pos != -1 ){
                                parent->AddChild(xmldefault , pos+1 );
                                parent->AddChild(pXMLElement , pos+2 );
                            }else fprintf(stdout,"ERROR: Can't find reference Element of id %s language %d\n",pXMLElement->GetId().GetBuffer(),curLang);
                        }

                        aLangHM->erase( sCur );
                    }
                }
                delete pResData;
            }else if( pResData == NULL ){fprintf(stdout,"Can't find GID=%s LID=%s TYP=%s\n",pResData->sGId.GetBuffer(),pResData->sId.GetBuffer(),pResData->sResTyp.GetBuffer());}
        }

    }
}
