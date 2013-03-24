/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2012 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef SC_QA_HELPER_HXX
#define SC_QA_HELPER_HXX

#include <test/bootstrapfixture.hxx>
#include "helper/csv_handler.hxx"
#include "helper/debughelper.hxx"
#include "orcus/csv_parser.hpp"
#include <fstream>
#include <string>
#include <sstream>

#include <comphelper/documentconstants.hxx>

#include <osl/detail/android-bootstrap.h>

#include <unotools/tempfile.hxx>
#include <comphelper/storagehelper.hxx>
#include <sfx2/docfilt.hxx>

#define ODS_FORMAT_TYPE 50331943
#define XLS_FORMAT_TYPE 318767171
#define XLSX_FORMAT_TYPE 268959811
#define LOTUS123_FORMAT_TYPE 268435649
#define CSV_FORMAT_TYPE  (SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_ALIEN | SFX_FILTER_USESOPTIONS)
#define HTML_FORMAT_TYPE (SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_ALIEN | SFX_FILTER_USESOPTIONS)

#define ODS      0
#define XLS      1
#define XLSX     2
#define CSV      3
#define HTML     4
#define LOTUS123 5

bool testEqualsWithTolerance( long nVal1, long nVal2, long nTol )
{
    return ( labs( nVal1 - nVal2 ) <= nTol );
}

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; unsigned int nFormatType;
};

#define CHECK_OPTIMAL 0x1

// data format for row height tests
struct TestParam
{
    struct RowData
    {
        SCROW nStartRow;
        SCROW nEndRow;
        SCTAB nTab;
        int nExpectedHeight; // -1 for default height
        int nCheck; // currently only CHECK_OPTIMAL ( we could add CHECK_MANUAL etc.)
        bool bOptimal;
    };
    const char* sTestDoc;
    int nImportType;
    int nExportType; // -1 for import test, otherwise this is an export test
    int nRowData;
    RowData* pData;
};

FileFormat aFileFormats[] = {
    { "ods" , "calc8", "", ODS_FORMAT_TYPE },
    { "xls" , "MS Excel 97", "calc_MS_EXCEL_97", XLS_FORMAT_TYPE },
    { "xlsx", "Calc MS Excel 2007 XML" , "MS Excel 2007 XML", XLSX_FORMAT_TYPE },
    { "csv" , "Text - txt - csv (StarCalc)", "generic_Text", CSV_FORMAT_TYPE },
    { "html" , "calc_HTML_WebQuery", "generic_HTML", HTML_FORMAT_TYPE },
    { "123" , "Lotus", "calc_Lotus", LOTUS123_FORMAT_TYPE },
};

// Why is this here and not in osl, and using the already existing file
// handling APIs? Do we really want to add arbitrary new file handling
// wrappers here and there (and then having to handle the Android (and
// eventually perhaps iOS) special cases here, too)?  Please move this to osl,
// it sure looks gemerally useful. Or am I missing something?

void loadFile(const OUString& aFileName, std::string& aContent)
{
    OString aOFileName = OUStringToOString(aFileName, RTL_TEXTENCODING_UTF8);

#ifdef ANDROID
    size_t size;
    if (strncmp(aOFileName.getStr(), "/assets/", sizeof("/assets/")-1) == 0) {
        const char *contents = (const char *) lo_apkentry(aOFileName.getStr(), &size);
        if (contents != 0) {
            aContent = std::string(contents, size);
            return;
        }
    }
#endif

    std::ifstream aFile(aOFileName.getStr());

    OStringBuffer aErrorMsg("Could not open csv file: ");
    aErrorMsg.append(aOFileName);
    CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), aFile);
    std::ostringstream aOStream;
    aOStream << aFile.rdbuf();
    aFile.close();
    aContent = aOStream.str();
}

class ScBootstrapFixture : public test::BootstrapFixture
{
protected:
    OUString m_aBaseString;
    ScDocShellRef load( bool bReadWrite,
        const OUString& rURL, const OUString& rFilter, const OUString &rUserData,
        const OUString& rTypeName, unsigned int nFilterFlags, unsigned int nClipboardID,  sal_uIntPtr nFilterVersion = SOFFICE_FILEFORMAT_CURRENT, const OUString* pPassword = NULL )
    {
        SfxFilter* pFilter = new SfxFilter(
            rFilter,
            OUString(), nFilterFlags, nClipboardID, rTypeName, 0, OUString(),
            rUserData, OUString("private:factory/scalc*"));
        pFilter->SetVersion(nFilterVersion);

        ScDocShellRef xDocShRef = new ScDocShell;
        xDocShRef->GetDocument()->EnableUserInteraction(false);
        SfxMedium* pSrcMed = new SfxMedium(rURL, bReadWrite ? STREAM_STD_READWRITE : STREAM_STD_READ );
        pSrcMed->SetFilter(pFilter);
        pSrcMed->UseInteractionHandler(false);
        if (pPassword)
        {
            SfxItemSet* pSet = pSrcMed->GetItemSet();
            pSet->Put(SfxStringItem(SID_PASSWORD, *pPassword));
        }
        printf("about to load %s\n", rtl::OUStringToOString( rURL, RTL_TEXTENCODING_UTF8 ).getStr() );
        if (!xDocShRef->DoLoad(pSrcMed))
        {
            xDocShRef->DoClose();
            // load failed.
            xDocShRef.Clear();
        }

        return xDocShRef;
    }

    ScDocShellRef load(
        const OUString& rURL, const OUString& rFilter, const OUString &rUserData,
        const OUString& rTypeName, unsigned int nFilterFlags, unsigned int nClipboardID,  sal_uIntPtr nFilterVersion = SOFFICE_FILEFORMAT_CURRENT, const OUString* pPassword = NULL )
    {
        return load( false, rURL, rFilter, rUserData, rTypeName, nFilterFlags, nClipboardID,  nFilterVersion, pPassword );
    }

    ScDocShellRef loadDoc(const OUString& rFileName, sal_Int32 nFormat, bool bReadWrite = false )
    {
        OUString aFileExtension(aFileFormats[nFormat].pName, strlen(aFileFormats[nFormat].pName), RTL_TEXTENCODING_UTF8 );
        OUString aFilterName(aFileFormats[nFormat].pFilterName, strlen(aFileFormats[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
        OUString aFileName;
        createFileURL( rFileName, aFileExtension, aFileName );
        OUString aFilterType(aFileFormats[nFormat].pTypeName, strlen(aFileFormats[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
        unsigned int nFormatType = aFileFormats[nFormat].nFormatType;
        unsigned int nClipboardId = nFormatType ? SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS : 0;

        return load(bReadWrite, aFileName, aFilterName, OUString(), aFilterType, nFormatType, nClipboardId, nFormatType);
    }


public:
    ScBootstrapFixture( const OUString& rsBaseString ) : m_aBaseString( rsBaseString ) {}
    void createFileURL(const OUString& aFileBase, const OUString& aFileExtension, OUString& rFilePath)
    {
        OUString aSep("/");
        OUStringBuffer aBuffer( getSrcRootURL() );
        aBuffer.append(m_aBaseString).append(aSep).append(aFileExtension);
        aBuffer.append(aSep).append(aFileBase).append(aFileExtension);
        rFilePath = aBuffer.makeStringAndClear();
    }

    void createCSVPath(const OUString& aFileBase, OUString& rCSVPath)
    {
        OUStringBuffer aBuffer( getSrcRootPath());
        aBuffer.append(m_aBaseString).append(OUString("/contentCSV/"));
        aBuffer.append(aFileBase).append(OUString("csv"));
        rCSVPath = aBuffer.makeStringAndClear();
    }

    ScDocShellRef saveAndReload(ScDocShell* pShell, const OUString &rFilter,
    const OUString &rUserData, const OUString& rTypeName, sal_uLong nFormatType)
    {

        utl::TempFile aTempFile;
        aTempFile.EnableKillingFile();
        SfxMedium aStoreMedium( aTempFile.GetURL(), STREAM_STD_WRITE );
        sal_uInt32 nExportFormat = 0;
        if (nFormatType == ODS_FORMAT_TYPE)
            nExportFormat = SFX_FILTER_EXPORT | SFX_FILTER_USESOPTIONS;
        SfxFilter* pExportFilter = new SfxFilter(
            rFilter,
            OUString(), nFormatType, nExportFormat, rTypeName, 0, OUString(),
            rUserData, OUString("private:factory/scalc*") );
        pExportFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);
        aStoreMedium.SetFilter(pExportFilter);
        pShell->DoSaveAs( aStoreMedium );
        pShell->DoClose();

        //std::cout << "File: " << aTempFile.GetURL() << std::endl;

        sal_uInt32 nFormat = 0;
        if (nFormatType == ODS_FORMAT_TYPE)
            nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;

        return load(aTempFile.GetURL(), rFilter, rUserData, rTypeName, nFormatType, nFormat );
    }
    ScDocShellRef saveAndReload( ScDocShell* pShell, sal_Int32 nFormat )
    {
        OUString aFilterName(aFileFormats[nFormat].pFilterName, strlen(aFileFormats[nFormat].pFilterName), RTL_TEXTENCODING_UTF8) ;
        OUString aFilterType(aFileFormats[nFormat].pTypeName, strlen(aFileFormats[nFormat].pTypeName), RTL_TEXTENCODING_UTF8);
        ScDocShellRef xDocSh = saveAndReload(pShell, aFilterName, OUString(), aFilterType, aFileFormats[nFormat].nFormatType);

        CPPUNIT_ASSERT(xDocSh.Is());
        return xDocSh;
    }

    void miscRowHeightsTest( TestParam* aTestValues, unsigned int numElems )
    {
        for ( unsigned int index=0; index<numElems; ++index )
        {
            OUString sFileName = OUString::createFromAscii( aTestValues[ index ].sTestDoc );
            printf("aTestValues[%u] %s\n", index, OUStringToOString( sFileName, RTL_TEXTENCODING_UTF8 ).getStr() );
            int nImportType =  aTestValues[ index ].nImportType;
            int nExportType =  aTestValues[ index ].nExportType;
            ScDocShellRef xShell = loadDoc( sFileName, nImportType );
            CPPUNIT_ASSERT(xShell.Is());

            if ( nExportType != -1 )
                xShell = saveAndReload(&(*xShell), nExportType );

            CPPUNIT_ASSERT(xShell.Is());

            ScDocument* pDoc = xShell->GetDocument();

            for (int i=0; i<aTestValues[ index ].nRowData; ++i)
            {
                SCROW nRow = aTestValues[ index ].pData[ i].nStartRow;
                SCROW nEndRow = aTestValues[ index ].pData[ i ].nEndRow;
                SCTAB nTab = aTestValues[ index ].pData[ i ].nTab;
                int nExpectedHeight = aTestValues[ index ].pData[ i ].nExpectedHeight;
                if ( nExpectedHeight == -1 )
                    nExpectedHeight =  sc::TwipsToHMM( ScGlobal::nStdRowHeight );
                bool bCheckOpt = ( ( aTestValues[ index ].pData[ i ].nCheck & CHECK_OPTIMAL ) == CHECK_OPTIMAL );
                for ( ; nRow <= nEndRow; ++nRow )
                {
                    printf("\t checking row %" SAL_PRIdINT32 " for height %d\n", nRow, nExpectedHeight );
                    int nHeight = sc::TwipsToHMM( pDoc->GetRowHeight(nRow, nTab, false) );
                    if ( bCheckOpt )
                    {
                        bool bOpt = !(pDoc->GetRowFlags( nRow, nTab ) & CR_MANUALSIZE);
                        CPPUNIT_ASSERT_EQUAL(aTestValues[ index ].pData[ i ].bOptimal, bOpt);
                    }
                    CPPUNIT_ASSERT_EQUAL(nExpectedHeight, nHeight);
                }
            }
            xShell->DoClose();
        }
    }
};

void testFile(OUString& aFileName, ScDocument* pDoc, SCTAB nTab, StringType aStringFormat = StringValue)
{
    csv_handler aHandler(pDoc, nTab, aStringFormat);
    orcus::csv_parser_config aConfig;
    aConfig.delimiters.push_back(',');
    aConfig.delimiters.push_back(';');
    aConfig.text_qualifier = '"';
    aConfig.trim_cell_value = false;


    std::string aContent;
    loadFile(aFileName, aContent);
    orcus::csv_parser<csv_handler> parser ( &aContent[0], aContent.size() , aHandler, aConfig);
    try
    {
        parser.parse();
    }
    catch (const orcus::csv_parse_error& e)
    {
        std::cout << "reading csv content file failed: " << e.what() << std::endl;
        OStringBuffer aErrorMsg("csv parser error: ");
        aErrorMsg.append(e.what());
        CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), false);
    }
}

//need own handler because conditional formatting strings must be generated
void testCondFile(OUString& aFileName, ScDocument* pDoc, SCTAB nTab)
{
    conditional_format_handler aHandler(pDoc, nTab);
    orcus::csv_parser_config aConfig;
    aConfig.delimiters.push_back(',');
    aConfig.delimiters.push_back(';');
    aConfig.text_qualifier = '"';
    std::string aContent;
    loadFile(aFileName, aContent);
    orcus::csv_parser<conditional_format_handler> parser ( &aContent[0], aContent.size() , aHandler, aConfig);
    try
    {
        parser.parse();
    }
    catch (const orcus::csv_parse_error& e)
    {
        std::cout << "reading csv content file failed: " << e.what() << std::endl;
        OStringBuffer aErrorMsg("csv parser error: ");
        aErrorMsg.append(e.what());
        CPPUNIT_ASSERT_MESSAGE(aErrorMsg.getStr(), false);
    }

}

#define ASSERT_DOUBLES_EQUAL( expected, result )    \
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (expected), (result), 1e-14 )

#define ASSERT_DOUBLES_EQUAL_MESSAGE( message, expected, result )   \
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( (message), (expected), (result), 1e-14 )

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
