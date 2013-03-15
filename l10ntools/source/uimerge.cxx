/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/main.h>

#include <osl/file.hxx>

#include <rtl/strbuf.hxx>

#include <libexslt/exslt.h>
#include <libxslt/transform.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltutils.h>

#include <stdio.h>

#include "common.hxx"
#include "export.hxx"
#include "tokens.h"
#include "po.hxx"
#include <iostream>
#include <fstream>
#include <vector>

rtl::OString sPrj;
rtl::OString sPrjRoot;
rtl::OString sInputFileName;
rtl::OString sOutputFile;

namespace
{
    //Convert xmlChar* to OString
    static OString lcl_xmlStrToOString( const xmlChar* pString )
    {
        xmlChar* pTemp = xmlStrdup( pString );
        OString sResult =
            static_cast<OString>(reinterpret_cast<sal_Char*>( pTemp ));
        xmlFree( pTemp );
        return sResult;
    }
}

int extractTranslations()
{
    PoOfstream aPOStream( sOutputFile, PoOfstream::APP);
    if (!aPOStream.isOpen())
    {
        fprintf(stderr, "cannot open %s\n", sOutputFile.getStr());
        return 1;
    }

    exsltRegisterAll();

    rtl::OString sStyleSheet = rtl::OString(getenv("SRC_ROOT"))  + rtl::OString("/solenv/bin/uilangfilter.xslt");

    xsltStylesheetPtr stylesheet = xsltParseStylesheetFile ((const xmlChar *)sStyleSheet.getStr());

    xmlDocPtr doc = xmlParseFile(sInputFileName.getStr());

    xmlDocPtr res = xsltApplyStylesheet(stylesheet, doc, NULL);

    for( xmlNodePtr nodeLevel1 = res->children; nodeLevel1 != NULL; nodeLevel1 = nodeLevel1->next)
    {
        for( xmlNodePtr nodeLevel2 = nodeLevel1->children; nodeLevel2 != NULL; nodeLevel2 = nodeLevel2->next)
        {
            if (nodeLevel2->type == XML_ELEMENT_NODE)
            {
                std::vector<OString> vIDs;
                for(xmlAttrPtr attribute = nodeLevel2->properties; attribute != NULL; attribute = attribute->next)
                {
                    xmlChar *content = xmlNodeListGetString(res, attribute->children, 1);
                    vIDs.push_back(lcl_xmlStrToOString(content));
                    xmlFree(content);
                }
                OString sText = lcl_xmlStrToOString(xmlNodeGetContent(nodeLevel2));
                Export::writePoEntry(
                    "Uiex", aPOStream, sInputFileName, vIDs[0],
                    (vIDs.size()>=2) ? vIDs[1] : OString(),
                    (vIDs.size()>=3) ? vIDs[2] : OString(),
                    OString(), sText);
            }
        }
    }

    xmlFreeDoc(res);

    xmlFreeDoc(doc);

    xsltFreeStylesheet(stylesheet);

    aPOStream.close();

    return 0;
}

namespace
{
    rtl::OString QuotHTML(const rtl::OString &rString)
    {
        rtl::OStringBuffer sReturn;
        for (sal_Int32 i = 0; i < rString.getLength(); ++i) {
            switch (rString[i]) {
            case '\\':
                if (i < rString.getLength()) {
                    switch (rString[i + 1]) {
                    case '"':
                    case '<':
                    case '>':
                    case '\\':
                        ++i;
                        break;
                    }
                }
                // fall through
            default:
                sReturn.append(rString[i]);
                break;

            case '<':
                sReturn.append("&lt;");
                break;

            case '>':
                sReturn.append("&gt;");
                break;

            case '"':
                sReturn.append("&quot;");
                break;

            case '&':
                if (rString.matchL(RTL_CONSTASCII_STRINGPARAM("&amp;"), i))
                    sReturn.append('&');
                else
                    sReturn.append(RTL_CONSTASCII_STRINGPARAM("&amp;"));
                break;
            }
        }
        return sReturn.makeStringAndClear();
    }

    bool lcl_MergeLang(
            const MergeDataHashMap &rMap,
            const rtl::OString &rLanguage,
            const rtl::OString &rDestinationFile)
    {
        std::ofstream aDestination(
            rDestinationFile.getStr(), std::ios_base::out | std::ios_base::trunc);
        if (!aDestination.is_open()) {
            return false;
        }

        aDestination << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        aDestination << "<t>\n";

        for (MergeDataHashMap::const_iterator aI = rMap.begin(), aEnd = rMap.end(); aI != aEnd; ++aI)
        {
            if (aI->second->sGID.isEmpty())
                continue;

            PFormEntrys* pFoo = aI->second->GetPFormEntries();
            rtl::OString sOut;
            pFoo->GetText( sOut, STRING_TYP_TEXT, rLanguage );

            if (sOut.isEmpty())
                continue;

            aDestination << " <e "
                << "g=\"" << aI->second->sGID.getStr() << "\" "
                << "i=\"" << aI->second->sLID.getStr() << "\">"
                << QuotHTML(sOut).getStr() << "</e>\n";
        }

        aDestination << "</t>";
        aDestination.close();

        return true;
    }

}

bool Merge(
    const rtl::OString &rSDFFile,
    const rtl::OString &rSourceFile,
    const rtl::OString &rDestinationDir)
{
    {
        bool bDestinationIsDir(false);

        const rtl::OUString aDestDir(rtl::OStringToOUString(rDestinationDir, RTL_TEXTENCODING_UTF8));
        rtl::OUString aDestDirUrl;
        if (osl::FileBase::E_None == osl::FileBase::getFileURLFromSystemPath(aDestDir, aDestDirUrl))
        {
            osl::DirectoryItem aTmp;
            if (osl::DirectoryItem::E_None == osl::DirectoryItem::get(aDestDirUrl, aTmp))
            {
                osl::FileStatus aDestinationStatus(osl_FileStatus_Mask_Type);
                if (osl::DirectoryItem::E_None == aTmp.getFileStatus(aDestinationStatus))
                    bDestinationIsDir = aDestinationStatus.isDirectory();
            }
        }

        if (!bDestinationIsDir)
        {
            fprintf(stderr, "%s must be a directory\n", rDestinationDir.getStr());
            return false;
        }
    }

    Export::InitLanguages( true );

    MergeDataFile aMergeDataFile( rSDFFile, rSourceFile, sal_False );
    rtl::OString sTmp( Export::sLanguages );
    if( sTmp.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("ALL")) )
        Export::SetLanguages( aMergeDataFile.GetLanguages() );

    std::vector<rtl::OString> aLanguages = Export::GetLanguages();

    const MergeDataHashMap& rMap = aMergeDataFile.getMap();
    const rtl::OString aDestinationDir(rDestinationDir + "/");

    bool bResult = true;
    for(size_t n = 0; n < aLanguages.size(); ++n)
    {
        rtl::OString sCur = aLanguages[ n ];
        if (sCur.isEmpty() || sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")))
            continue;
        const rtl::OString aDestinationFile(aDestinationDir + sCur + ".ui");
        if (!lcl_MergeLang(rMap, sCur, aDestinationFile))
            bResult = false;
    }

    return bResult;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    int nRetValue = 0;

    HandledArgs aArgs;
    if ( !Export::handleArguments(argc, argv, aArgs) )
    {
        Export::writeUsage("uiex","ui");
        return 1;
    }

    sPrj = aArgs.m_sPrj;
    sPrjRoot = aArgs.m_sPrjRoot;
    sInputFileName = aArgs.m_sInputFile;
    sOutputFile = aArgs.m_sOutputFile;

    if (!aArgs.m_bMergeMode)
    {
        if (Export::sLanguages != "en-US")
        {
            fprintf(stderr, "only en-US can exist in source .ui files\n");
            nRetValue = 1;
        }
        else
            nRetValue = extractTranslations();
    }
    else
    {
        Merge(aArgs.m_sMergeSrc, sInputFileName, sOutputFile);
    }

    return nRetValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
