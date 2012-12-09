/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    //TODO: explicit BOM handling?

    std::auto_ptr <XMLFile> file ( aParser.Execute( sHelpFile, pXmlFile ) );

    if(file.get() == NULL)
    {
        printf(
            "%s: %s\n",
            sHelpFile.getStr(),
            aParser.GetError().sMessage.getStr());
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

    OStringBuffer sBuffer;

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
                OString data(
                    pXMLElement->ToOString().
                    replaceAll(
                        rtl::OString("\n"),
                        rtl::OString()).
                    replaceAll(
                        rtl::OString("\t"),
                        rtl::OString()).trim());
                if( !data.isEmpty() )
                {
                    sBuffer.append( rPrj_in );
                    sBuffer.append("\t");
                    if ( !rRoot_in.isEmpty())
                        sBuffer.append( sActFileName );
                    sBuffer.append( "\t0\t");
                    sBuffer.append( rGsi1 );               //"help";
                    sBuffer.append( "\t");
                    sBuffer.append( posm->first );
                    sBuffer.append( "\t");
                    sBuffer.append( pXMLElement->GetOldref());
                    sBuffer.append( "\t\t\t0\t");
                    sBuffer.append( sCur);
                    sBuffer.append('\t');
                    sBuffer.append( data );
                    sBuffer.append( "\t\t\t\t");
                    aSDFStream << sBuffer.makeStringAndClear().getStr() << '\n';
                }
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

    //TODO: explicit BOM handling?

    XMLFile* xmlfile = ( aParser.Execute( sHelpFile, new XMLFile( rtl::OString('0') ) ) );
    hasNoError = MergeSingleFile( xmlfile , aMergeDataFile , rLanguage , rDestinationFile );
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
                rtl::OString sSourceText(
                    pXMLElement->ToOString().
                    replaceAll(
                        rtl::OString("\n"),
                        rtl::OString()).
                    replaceAll(
                        rtl::OString("\t"),
                        rtl::OString()));
                // re-add spaces to the beginning of translated string,
                // important for indentation of Basic code examples
                sal_Int32 nPreSpaces = 0;
                sal_Int32 nLen = sSourceText.getLength();
                while ( (nPreSpaces < nLen) && (*(sSourceText.getStr()+nPreSpaces) == ' ') )
                    nPreSpaces++;
                pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur , true );
                OString sNewdata;
                if (helper::isWellFormedXML(helper::QuotHTML(sNewText)))
                {
                    sNewdata = sSourceText.copy(0,nPreSpaces) +
                        sNewText;
                }
                else
                {
                    sNewdata = sSourceText;
                }
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
            pXMLElement->ChangeLanguageTag(sCur);
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
