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
#include "helper.hxx"
#include "export.hxx"
#include "tokens.h"
#include "po.hxx"
#include <iostream>
#include <fstream>
#include <vector>

OString sInputFileName;
OString sOutputFile;

int extractTranslations()
{
    PoOfstream aPOStream( sOutputFile, PoOfstream::APP);
    if (!aPOStream.isOpen())
    {
        fprintf(stderr, "cannot open %s\n", sOutputFile.getStr());
        return 1;
    }

    exsltRegisterAll();

    OString sStyleSheet = OString(getenv("SRC_ROOT"))  + OString("/solenv/bin/uilangfilter.xslt");

    xsltStylesheetPtr stylesheet = xsltParseStylesheetFile (reinterpret_cast<const xmlChar *>(sStyleSheet.getStr()));

    xmlDocPtr doc = xmlParseFile(sInputFileName.getStr());

    xmlDocPtr res = xsltApplyStylesheet(stylesheet, doc, nullptr);

    for( xmlNodePtr nodeLevel1 = res->children; nodeLevel1 != nullptr; nodeLevel1 = nodeLevel1->next)
    {
        for( xmlNodePtr nodeLevel2 = nodeLevel1->children; nodeLevel2 != nullptr; nodeLevel2 = nodeLevel2->next)
        {
            if (nodeLevel2->type == XML_ELEMENT_NODE)
            {
                std::vector<OString> vIDs;
                for(xmlAttrPtr attribute = nodeLevel2->properties; attribute != nullptr; attribute = attribute->next)
                {
                    xmlChar *content = xmlNodeListGetString(res, attribute->children, 1);
                    vIDs.push_back(helper::xmlStrToOString(content));
                    xmlFree(content);
                }
                OString sText = helper::UnQuotHTML(helper::xmlStrToOString(xmlNodeGetContent(nodeLevel2)));
                common::writePoEntry(
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
    bool lcl_MergeLang(
            const MergeDataHashMap &rMap,
            const OString &rLanguage,
            const OString &rDestinationFile)
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

            MergeEntrys* pEntrys = aI->second->GetMergeEntries();
            OString sOut;
            pEntrys->GetText( sOut, STRING_TYP_TEXT, rLanguage );

            if (sOut.isEmpty())
                continue;

            aDestination << " <e g=\"" << aI->second->sGID.getStr() << "\" i=\""
                << aI->second->sLID.getStr() << "\">"
                << helper::QuotHTML(sOut).getStr() << "</e>\n";
        }

        aDestination << "</t>";
        aDestination.close();

        return true;
    }

}

bool Merge(
    const OString &rPOFile,
    const OString &rSourceFile,
    const OString &rDestinationDir,
    const OString &rLanguage )
{
    {
        bool bDestinationIsDir(false);

        const OUString aDestDir(OStringToOUString(rDestinationDir, RTL_TEXTENCODING_UTF8));
        OUString aDestDirUrl;
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

    MergeDataFile aMergeDataFile( rPOFile, rSourceFile, false );
    std::vector<OString> aLanguages;
    if( rLanguage.equalsIgnoreAsciiCase("ALL") )
        aLanguages = aMergeDataFile.GetLanguages();
    else
        aLanguages.push_back(rLanguage);

    const MergeDataHashMap& rMap = aMergeDataFile.getMap();
    const OString aDestinationDir(rDestinationDir + "/");

    bool bResult = true;
    for(size_t n = 0; n < aLanguages.size(); ++n)
    {
        OString sCur = aLanguages[ n ];
        if (sCur.isEmpty() || sCur.equalsIgnoreAsciiCase("en-US"))
            continue;
        const OString aDestinationFile(aDestinationDir + sCur + ".ui");
        if (!lcl_MergeLang(rMap, sCur, aDestinationFile))
            bResult = false;
    }

    return bResult;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    int nRetValue = 0;
    try
    {

        common::HandledArgs aArgs;
        if ( !common::handleArguments(argc, argv, aArgs) )
        {
            common::writeUsage("uiex","*.ui");
            return 1;
        }

        sInputFileName = aArgs.m_sInputFile;
        sOutputFile = aArgs.m_sOutputFile;

        if (!aArgs.m_bMergeMode)
        {
            nRetValue = extractTranslations();
        }
        else
        {
            Merge(aArgs.m_sMergeSrc, sInputFileName, sOutputFile, aArgs.m_sLanguage);
        }
    }
    catch (std::exception& e)
    {
        fprintf(stderr, "exception: %s\n", e.what());
        return 1;
    }
    return nRetValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
