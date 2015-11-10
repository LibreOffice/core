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
#include <memory>

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

void HelpParser::Dump(LangHashMap* rElem_in,const OString & sKey_in)
{
    OString x;
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

HelpParser::HelpParser( const OString &rHelpFile )
        : sHelpFile( rHelpFile )
          {};

/*****************************************************************************/
bool HelpParser::CreatePO(
/*****************************************************************************/
    const OString &rPOFile_in, const OString &sHelpFile,
    XMLFile *pXmlFile, const OString &rGsi1){
    SimpleXMLParser aParser;
    //TODO: explicit BOM handling?

    std::unique_ptr <XMLFile> file ( aParser.Execute( sHelpFile, pXmlFile ) );

    if(file.get() == nullptr)
    {
        printf(
            "%s: %s\n",
            sHelpFile.getStr(),
            aParser.GetError().m_sMessage.getStr());
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

    XMLHashMap* aXMLStrHM = file->GetStrings();

    std::vector<OString> order = file->getOrder();
    std::vector<OString>::iterator pos;
    XMLHashMap::iterator posm;

    for( pos = order.begin(); pos != order.end() ; ++pos )
    {
        posm = aXMLStrHM->find( *pos );
        LangHashMap* pElem = posm->second;

        XMLElement* pXMLElement = (*pElem)[ "en-US" ];

        if( pXMLElement != nullptr )
        {
            OString data(
                pXMLElement->ToOString().
                    replaceAll("\n",OString()).
                    replaceAll("\t",OString()).trim());

            common::writePoEntry(
                "Helpex", aPoOutput, sHelpFile, rGsi1,
                posm->first, pXMLElement->GetOldref(), OString(), data);

            pXMLElement=nullptr;
        }
        else
        {
            fprintf(stdout,"\nDBG: NullPointer in HelpParser::CreatePO, File %s\n", sHelpFile.getStr());
        }
    }
    aPoOutput.close();

    return true;
}

bool HelpParser::Merge( const OString &rPOFile, const OString &rDestinationFile,
    const OString& rLanguage , MergeDataFile* pMergeDataFile )
{

    (void) rPOFile;

    SimpleXMLParser aParser;

    //TODO: explicit BOM handling?

    XMLFile* xmlfile = ( aParser.Execute( sHelpFile, new XMLFile( OString('0') ) ) );
    if (!xmlfile)
    {
        SAL_WARN("l10ntools", "could not parse " << sHelpFile);
        return false;
    }
    bool hasNoError = MergeSingleFile( xmlfile , pMergeDataFile , rLanguage , rDestinationFile );
    delete xmlfile;
    return hasNoError;
}

bool HelpParser::MergeSingleFile( XMLFile* file , MergeDataFile* pMergeDataFile , const OString& sLanguage ,
                                  OString const & sPath )
{
    file->Extract();

    XMLHashMap* aXMLStrHM = file->GetStrings();
    static  ResData pResData("","");
    pResData.sResTyp   = "help";

    std::vector<OString> order = file->getOrder();
    std::vector<OString>::iterator pos;
    XMLHashMap::iterator posm;

    for( pos = order.begin(); pos != order.end() ; ++pos ) // Merge every l10n related string in the same order as export
    {
        posm = aXMLStrHM->find( *pos );
        LangHashMap*  aLangHM = posm->second;
#if OSL_DEBUG_LEVEL > 2
        printf("*********************DUMPING HASHMAP***************************************");
        Dump(aXMLStrHM);
        printf("DBG: sHelpFile = %s\n",sHelpFile.getStr() );
#endif

        pResData.sGId      =  posm->first;
        pResData.sFilename  =  sHelpFile;

        ProcessHelp( aLangHM , sLanguage, &pResData , pMergeDataFile );
     }

    file->Write(sPath);
    return true;
}

/* ProcessHelp Methode: search for en-US entry and replace it with the current language*/
void HelpParser::ProcessHelp( LangHashMap* aLangHM , const OString& sCur , ResData *pResData , MergeDataFile* pMergeDataFile ){

    XMLElement*   pXMLElement = nullptr;
    MergeEntrys   *pEntrys    = nullptr;

    OString sLId;

    pEntrys = nullptr;

    if( !sCur.equalsIgnoreAsciiCase("en-US") ){
        pXMLElement = (*aLangHM)[ "en-US" ];
        if( pXMLElement == nullptr )
        {
            printf("Error: Can't find en-US entry\n");
        }
        if( pXMLElement != nullptr )
        {
            sLId    = pXMLElement->GetOldref();
            pResData->sId     =  sLId;

            OString sNewText;
            OString sNewdata;
            OString sSourceText(
            pXMLElement->ToOString().
                replaceAll(
                    OString("\n"),
                    OString()).
                replaceAll(
                    OString("\t"),
                    OString()));
            // re-add spaces to the beginning of translated string,
            // important for indentation of Basic code examples
            sal_Int32 nPreSpaces = 0;
            sal_Int32 nLen = sSourceText.getLength();
            while ( (nPreSpaces < nLen) && (*(sSourceText.getStr()+nPreSpaces) == ' ') )
                nPreSpaces++;
            if( sCur == "qtz" )
            {
                sNewText = MergeEntrys::GetQTZText(*pResData, sSourceText);
                sNewdata = sNewText;
            }
            else if( pMergeDataFile )
            {
                pEntrys = pMergeDataFile->GetMergeEntrys( pResData );
                if( pEntrys != nullptr)
                {
                    pEntrys->GetText( sNewText, STRING_TYP_TEXT, sCur, true );
                    if (helper::isWellFormedXML(XMLUtil::QuotHTML(sNewText)))
                    {
                        sNewdata = sSourceText.copy(0,nPreSpaces) + sNewText;
                    }
                }
            }
            if (!sNewdata.isEmpty())
            {
                if( pXMLElement != nullptr )
                {
                    XMLData *data = new XMLData( sNewdata , nullptr ); // Add new one
                    pXMLElement->RemoveAndDeleteAllChildren();
                    pXMLElement->AddChild( data );
                    aLangHM->erase( sCur );
                }
            }
            else
            {
                SAL_WARN(
                    "l10ntools",
                    "Can't find GID=" << pResData->sGId.getStr() << " LID="
                        << pResData->sId.getStr() << " TYP=" << pResData->sResTyp.getStr());
            }
            pXMLElement->ChangeLanguageTag(sCur);
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
