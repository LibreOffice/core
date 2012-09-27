/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/main.h>

#include <rtl/strbuf.hxx>

#include <libexslt/exslt.h>
#include <libxslt/transform.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltutils.h>

#include <stdio.h>

#include "common.hxx"
#include "export.hxx"
#include "xrmmerge.hxx"
#include "tokens.h"
#include <iostream>
#include <fstream>
#include <vector>

#define STATE_NON       0x0001
#define STATE_INPUT     0x0002
#define STATE_OUTPUT    0x0003
#define STATE_PRJ       0x0004
#define STATE_ROOT      0x0005
#define STATE_MERGESRC  0x0006
#define STATE_ERRORLOG  0x0007
#define STATE_LANGUAGES 0x000C

sal_Bool bMergeMode;
sal_Bool bErrorLog;
sal_Bool bUTF8;
sal_Bool bDisplayName;
sal_Bool bExtensionDescription;
rtl::OString sPrj;
rtl::OString sPrjRoot;
rtl::OString sInputFileName;
rtl::OString sOutputFile;
rtl::OString sMergeSrc;
rtl::OString sLangAttribute;
rtl::OString sResourceType;
XRMResParser *pParser = NULL;

void GetOutputFile( int argc, char* argv[])
{
    bMergeMode = sal_False;
    bErrorLog = sal_True;
    bUTF8 = sal_True;
    bDisplayName = sal_False;
    bExtensionDescription = sal_False;
    sPrj = "";
    sPrjRoot = "";
    sInputFileName = "";
    Export::sLanguages = "";
    sal_uInt16 nState = STATE_NON;

    // parse command line
    for( int i = 1; i < argc; i++ ) {
        if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-I" ) {
            nState = STATE_INPUT; // next token specifies source file
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-O" ) {
            nState = STATE_OUTPUT; // next token specifies the dest file
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-P" ) {
            nState = STATE_PRJ; // next token specifies the cur. project
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-R" ) {
            nState = STATE_ROOT; // next token specifies path to project root
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-M" ) {
            nState = STATE_MERGESRC; // next token specifies the merge database
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-E" ) {
            nState = STATE_ERRORLOG;
            bErrorLog = sal_False;
        }
        else if ( rtl::OString( argv[ i ] ).toAsciiUpperCase() == "-L" ) {
            nState = STATE_LANGUAGES;
        }
        else {
            switch ( nState ) {
                case STATE_NON: {
                    return;    // no valid command line
                }
                case STATE_INPUT: {
                    sInputFileName = argv[ i ];
                }
                break;
                case STATE_OUTPUT: {
                    sOutputFile = argv[ i ]; // the dest. file
                }
                break;
                case STATE_PRJ: {
                    sPrj = rtl::OString( argv[ i ]);
                }
                break;
                case STATE_ROOT: {
                    sPrjRoot = rtl::OString( argv[ i ]); // path to project root
                }
                break;
                case STATE_MERGESRC: {
                    sMergeSrc = rtl::OString( argv[ i ]);
                    bMergeMode = sal_True; // activate merge mode, cause merge database found
                }
                break;
                case STATE_LANGUAGES: {
                    Export::sLanguages = rtl::OString( argv[ i ]);
                }
                break;
            }
        }
    }
}

int extractTranslations()
{
    FILE *pOutFile = fopen(sOutputFile.getStr(), "w");
    if (!pOutFile)
    {
        fprintf(stderr, "cannot open %s\n", sOutputFile.getStr());
        return 1;
    }

    exsltRegisterAll();

    rtl::OString sActFileName = common::pathnameToken(sInputFileName.getStr(), sPrjRoot.getStr());

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
                fprintf(pOutFile, "%s\t%s\t0\t",sPrj.getStr(), sActFileName.getStr());
                for(xmlAttrPtr attribute = nodeLevel2->properties; attribute != NULL; attribute = attribute->next)
                {
                    xmlChar *content = xmlNodeListGetString(res, attribute->children, 1);
                    fprintf(pOutFile, "%s\t", content);
                    xmlFree(content);
                }
                fprintf(pOutFile, "\t\t0\ten-US\t%s\t\t\t\t\n", xmlNodeGetContent(nodeLevel2));
            }
        }
    }

    xmlFreeDoc(res);

    xmlFreeDoc(doc);

    xsltFreeStylesheet(stylesheet);

    fclose(pOutFile);

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
}

bool Merge(
    const rtl::OString &rSDFFile,
    const rtl::OString &rSourceFile,
    const rtl::OString &rDestinationFile)
{
    Export::InitLanguages( true );
    std::ofstream aDestination(
        rDestinationFile.getStr(), std::ios_base::out | std::ios_base::trunc);
    if (!aDestination.is_open()) {
        return false;
    }

    aDestination << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    aDestination << "<t>\n";

    MergeDataFile aMergeDataFile( rSDFFile, rSourceFile, sal_False );
    rtl::OString sTmp( Export::sLanguages );
    if( sTmp.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("ALL")) )
        Export::SetLanguages( aMergeDataFile.GetLanguages() );

    std::vector<rtl::OString> aLanguages = Export::GetLanguages();

    const MergeDataHashMap& rMap = aMergeDataFile.getMap();

    for(size_t n = 0; n < aLanguages.size(); ++n)
    {
        rtl::OString sCur = aLanguages[ n ];
        if (sCur.isEmpty() || sCur.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("en-US")))
            continue;
        for (MergeDataHashMap::const_iterator aI = rMap.begin(), aEnd = rMap.end(); aI != aEnd; ++aI)
        {
            if (aI->second->sGID.isEmpty())
                continue;

            PFormEntrys* pFoo = aI->second->GetPFormEntries();
            rtl::OString sOut;
            pFoo->GetText( sOut, STRING_TYP_TEXT, sCur);

            if (sOut.isEmpty())
                continue;

            aDestination << " <e "
                << "g=\"" << aI->second->sGID.getStr() << "\" "
                << "i=\"" << aI->second->sLID.getStr() << "\">"
                << QuotHTML(sOut).getStr() << "</e>\n";
        }
    }

    aDestination << "</t>";
    aDestination.close();
    return sal_True;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    int nRetValue = 0;

    GetOutputFile( argc, argv );

    if (sOutputFile.isEmpty())
    {
        fprintf( stdout, "Syntax: UIEX[-p Prj][-r PrjRoot]-i FileIn [-o FileOut][-m DataBase][-e][-L l1,l2,...]\n" );
        fprintf( stdout, " Prj:      Project\n" );
        fprintf( stdout, " PrjRoot:  Path to project root (..\\.. etc.)\n" );
        fprintf( stdout, " FileIn:   Source files (*.src)\n" );
        fprintf( stdout, " FileOut:  Destination file (*.*)\n" );
        fprintf( stdout, " DataBase: Mergedata (*.sdf)\n" );
        fprintf( stdout, " -e: Disable writing errorlog\n" );
        fprintf( stdout, " -L: Restrict the handled languages. l1,l2,... are elements of (de,en-US,es...)\n" );
        return 1;
    }

    if (!bMergeMode)
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
        Merge(sMergeSrc, sInputFileName, sOutputFile);
    }

    return nRetValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
