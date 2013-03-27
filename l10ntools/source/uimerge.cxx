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

rtl::OString sInputFileName;
rtl::OString sOutputFile;

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
                    vIDs.push_back(helper::xmlStrToOString(content));
                    xmlFree(content);
                }
                OString sText = helper::xmlStrToOString(xmlNodeGetContent(nodeLevel2));
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

    MergeDataFile aMergeDataFile( rPOFile, rSourceFile, sal_False );
    std::vector<rtl::OString> aLanguages;
    if( rLanguage.equalsIgnoreAsciiCase("ALL") )
        aLanguages = aMergeDataFile.GetLanguages();
    else
        aLanguages.push_back(rLanguage);

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
        if (aArgs.m_sLanguage != "en-US")
        {
            fprintf(stderr, "only en-US can exist in source .ui files\n");
            nRetValue = 1;
        }
        else
            nRetValue = extractTranslations();
    }
    else
    {
        Merge(aArgs.m_sMergeSrc, sInputFileName, sOutputFile, aArgs.m_sLanguage);
    }

    return nRetValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
