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
#include <vector>
#include <rtl/strbuf.hxx>
#ifdef WNT
#include <windows.h>
#undef CopyFile
#include <direct.h>
#endif

#include "export.hxx"
#include "common.hxx"
#include "helper.hxx"
#include "po.hxx"

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
bool HelpParser::CreatePO(
/*****************************************************************************/
    const OString &rPOFile_in, const OString &sHelpFile, const OString &rLanguage,
    XMLFile *pXmlFile, const OString &rGsi1){
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

    PoOfstream aPoOutput( rPOFile_in, PoOfstream::APP );

    if (!aPoOutput.isOpen()) {
        fprintf(stdout,"Can't open file %s\n",rPOFile_in.getStr());
        return false;
    }

    XMLHashMap*  aXMLStrHM   = file->GetStrings();
    LangHashMap* pElem;
    XMLElement*  pXMLElement  = NULL;

    std::vector<rtl::OString> aLanguages;
    aLanguages.push_back( rLanguage );

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
                    OUStringToOString( pXMLElement->ToOUString(), RTL_TEXTENCODING_UTF8 ).
                        replaceAll("\n",OString()).
                        replaceAll("\t",OString()).trim());

                common::writePoEntry(
                    "Helpex", aPoOutput, sHelpFile, rGsi1,
                    posm->first, pXMLElement->GetOldref(), OString(), data);

                pXMLElement=NULL;
            }
            else
            {
                fprintf(stdout,"\nDBG: NullPointer in HelpParser::CreatePO, Language %s, File %s\n", sCur.getStr(), sHelpFile.getStr());
            }
        }
    }
    aPoOutput.close();

    return sal_True;
}

bool HelpParser::Merge( const rtl::OString &rPOFile, const rtl::OString &rDestinationFile,
    const rtl::OString& rLanguage , MergeDataFile& aMergeDataFile )
{

    (void) rPOFile;

    SimpleXMLParser aParser;

    rtl::OUString sXmlFile(
        rtl::OStringToOUString(sHelpFile, RTL_TEXTENCODING_ASCII_US));
    //TODO: explicit BOM handling?

    XMLFile* xmlfile = ( aParser.Execute( sXmlFile, new XMLFile( rtl::OUString('0') ) ) );
    bool hasNoError = MergeSingleFile( xmlfile , aMergeDataFile , rLanguage , rDestinationFile );
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

    rtl::OString sLId;

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
                rtl::OUString sSourceText(
                    pXMLElement->ToOUString().
                    replaceAll(
                        rtl::OUString("\n"),
                        rtl::OUString()).
                    replaceAll(
                        rtl::OUString("\t"),
                        rtl::OUString()));
                // re-add spaces to the beginning of translated string,
                // important for indentation of Basic code examples
                sal_Int32 nPreSpaces = 0;
                sal_Int32 nLen = sSourceText.getLength();
                while ( (nPreSpaces < nLen) && (*(sSourceText.getStr()+nPreSpaces) == ' ') )
                    nPreSpaces++;
                pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur , true );
                OUString sNewdata;
                if (helper::isWellFormedXML(helper::QuotHTML(sNewText)))
                {
                    sNewdata = sSourceText.copy(0,nPreSpaces) +
                        rtl::OStringToOUString(sNewText, RTL_TEXTENCODING_UTF8);
                }
                else
                {
                    sNewdata = sSourceText;
                }
                if (!sNewdata.isEmpty())
                {
                    if( pXMLElement != NULL )
                    {
                        XMLData *data = new XMLData( sNewdata , NULL , true ); // Add new one
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
