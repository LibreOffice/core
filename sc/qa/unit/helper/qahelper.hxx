/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_QA_UNIT_HELPER_QAHELPER_HXX
#define INCLUDED_SC_QA_UNIT_HELPER_QAHELPER_HXX

#include <docsh.hxx>
#include <address.hxx>

#include <cppunit/SourceLine.h>

#include <test/bootstrapfixture.hxx>
#include <comphelper/documentconstants.hxx>

#include <comphelper/fileformat.h>
#include <formula/grammar.hxx>

#include <string>
#include <sstream>

#include <sal/types.h>

#include <memory>

namespace utl { class TempFile; }

#if defined(SCQAHELPER_DLLIMPLEMENTATION)
#define SCQAHELPER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define SCQAHELPER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#define ODS_FORMAT_TYPE      (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::TEMPLATE | SfxFilterFlags::OWN | SfxFilterFlags::DEFAULT | SfxFilterFlags::ENCRYPTION | SfxFilterFlags::PASSWORDTOMODIFY)
#define XLS_FORMAT_TYPE      (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::ENCRYPTION | SfxFilterFlags::PASSWORDTOMODIFY | SfxFilterFlags::PREFERED)
#define XLSX_FORMAT_TYPE     (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::STARONEFILTER | SfxFilterFlags::PREFERED)
#define LOTUS123_FORMAT_TYPE (SfxFilterFlags::IMPORT |                          SfxFilterFlags::ALIEN | SfxFilterFlags::PREFERED)
#define CSV_FORMAT_TYPE      (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN )
#define HTML_FORMAT_TYPE     (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN )
#define DIF_FORMAT_TYPE      (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::ALIEN )
#define XLS_XML_FORMAT_TYPE  (SfxFilterFlags::IMPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::PREFERED)
#define XLSB_XML_FORMAT_TYPE (SfxFilterFlags::IMPORT |                          SfxFilterFlags::ALIEN | SfxFilterFlags::STARONEFILTER | SfxFilterFlags::PREFERED)
#define FODS_FORMAT_TYPE     (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::OWN | SfxFilterFlags::STARONEFILTER )
#define GNUMERIC_FORMAT_TYPE (SfxFilterFlags::IMPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::PREFERED )
#define XLTX_FORMAT_TYPE     (SfxFilterFlags::IMPORT | SfxFilterFlags::EXPORT | SfxFilterFlags::TEMPLATE |SfxFilterFlags::ALIEN | SfxFilterFlags::STARONEFILTER | SfxFilterFlags::PREFERED)

#define FORMAT_ODS      0
#define FORMAT_XLS      1
#define FORMAT_XLSX     2
#define FORMAT_XLSM     3
#define FORMAT_CSV      4
#define FORMAT_HTML     5
#define FORMAT_LOTUS123 6
#define FORMAT_DIF      7
#define FORMAT_XLS_XML  8
#define FORMAT_XLSB     9
#define FORMAT_FODS     10
#define FORMAT_GNUMERIC 11
#define FORMAT_XLTX     12

enum class StringType { PureString, StringValue };

SCQAHELPER_DLLPUBLIC bool testEqualsWithTolerance( long nVal1, long nVal2, long nTol );

#define CHECK_OPTIMAL 0x1

class SdrOle2Obj;
class ScRangeList;
class ScTokenArray;

// data format for row height tests
struct TestParam
{
    struct RowData
    {
        SCROW const nStartRow;
        SCROW const nEndRow;
        SCTAB const nTab;
        int const nExpectedHeight; // -1 for default height
        int const nCheck; // currently only CHECK_OPTIMAL ( we could add CHECK_MANUAL etc.)
        bool const bOptimal;
    };
    const char* sTestDoc;
    int nImportType;
    int nExportType; // -1 for import test, otherwise this is an export test
    int nRowData;
    RowData const * pData;
};

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; SfxFilterFlags nFormatType;
};

// Printers for the calc data structures. Needed for the EQUAL assertion
// macros from CPPUNIT.

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const ScAddress& rAddr);

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const ScRange& rRange);

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const ScRangeList& rList);

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const Color& rColor);

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const OpCode& rCode);

// Why is this here and not in osl, and using the already existing file
// handling APIs? Do we really want to add arbitrary new file handling
// wrappers here and there (and then having to handle the Android (and
// eventually perhaps iOS) special cases here, too)?  Please move this to osl,
// it sure looks generally useful. Or am I missing something?

SCQAHELPER_DLLPUBLIC void loadFile(const OUString& aFileName, std::string& aContent);

SCQAHELPER_DLLPUBLIC void testFile(const OUString& aFileName, ScDocument& rDoc, SCTAB nTab, StringType aStringFormat = StringType::StringValue);

//need own handler because conditional formatting strings must be generated
SCQAHELPER_DLLPUBLIC void testCondFile(const OUString& aFileName, ScDocument* pDoc, SCTAB nTab);

SCQAHELPER_DLLPUBLIC const SdrOle2Obj* getSingleChartObject(ScDocument& rDoc, sal_uInt16 nPage);

SCQAHELPER_DLLPUBLIC std::vector<OUString> getChartRangeRepresentations(const SdrOle2Obj& rChartObj);

SCQAHELPER_DLLPUBLIC ScRangeList getChartRanges(ScDocument& rDoc, const SdrOle2Obj& rChartObj);

SCQAHELPER_DLLPUBLIC bool checkFormula(ScDocument& rDoc, const ScAddress& rPos, const char* pExpected);

SCQAHELPER_DLLPUBLIC bool checkFormulaPosition(ScDocument& rDoc, const ScAddress& rPos);
SCQAHELPER_DLLPUBLIC bool checkFormulaPositions(
    ScDocument& rDoc, SCTAB nTab, SCCOL nCol, const SCROW* pRows, size_t nRowCount);

SCQAHELPER_DLLPUBLIC std::unique_ptr<ScTokenArray> compileFormula(
    ScDocument* pDoc, const OUString& rFormula,
    formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_NATIVE );

SCQAHELPER_DLLPUBLIC bool checkOutput(
    const ScDocument* pDoc, const ScRange& aOutRange,
    const std::vector<std::vector<const char*>>& aCheck, const char* pCaption );

SCQAHELPER_DLLPUBLIC void clearFormulaCellChangedFlag( ScDocument& rDoc, const ScRange& rRange );

/**
 * Check if the cell at specified position is a formula cell that doesn't
 * have an error value.
 */
SCQAHELPER_DLLPUBLIC bool isFormulaWithoutError(ScDocument& rDoc, const ScAddress& rPos);

/**
 * Convert formula token array to a formula string.
 */
SCQAHELPER_DLLPUBLIC OUString toString(
    ScDocument& rDoc, const ScAddress& rPos, ScTokenArray& rArray,
    formula::FormulaGrammar::Grammar eGram);

inline std::string print(const ScAddress& rAddr)
{
    std::ostringstream str;
    str << "Col: " << rAddr.Col();
    str << " Row: " << rAddr.Row();
    str << " Tab: " << rAddr.Tab();
    return str.str();
}

class SCQAHELPER_DLLPUBLIC ScBootstrapFixture : public test::BootstrapFixture
{
    static const FileFormat aFileFormats[];
protected:
    OUString const m_aBaseString;

    ScDocShellRef load(
        bool bReadWrite, const OUString& rURL, const OUString& rFilter, const OUString &rUserData,
        const OUString& rTypeName, SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
        sal_uIntPtr nFilterVersion = SOFFICE_FILEFORMAT_CURRENT, const OUString* pPassword = nullptr );

    ScDocShellRef load(
        const OUString& rURL, const OUString& rFilter, const OUString &rUserData,
        const OUString& rTypeName, SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
        sal_uIntPtr nFilterVersion = SOFFICE_FILEFORMAT_CURRENT, const OUString* pPassword = nullptr );

    ScDocShellRef loadDoc(const OUString& rFileName, sal_Int32 nFormat, bool bReadWrite = false );

public:
    static const FileFormat* getFileFormats() { return aFileFormats; }

    explicit ScBootstrapFixture( const OUString& rsBaseString );
    virtual ~ScBootstrapFixture() override;

    void createFileURL(const OUString& aFileBase, const OUString& aFileExtension, OUString& rFilePath);

    void createCSVPath(const OUString& aFileBase, OUString& rCSVPath);

    ScDocShellRef saveAndReload(ScDocShell* pShell, const OUString &rFilter,
    const OUString &rUserData, const OUString& rTypeName, SfxFilterFlags nFormatType);

    ScDocShellRef saveAndReload( ScDocShell* pShell, sal_Int32 nFormat );

    std::shared_ptr<utl::TempFile> saveAs(ScDocShell* pShell, sal_Int32 nFormat);
    std::shared_ptr<utl::TempFile> exportTo(ScDocShell* pShell, sal_Int32 nFormat);

    void miscRowHeightsTest( TestParam const * aTestValues, unsigned int numElems );
};

#define ASSERT_DOUBLES_EQUAL( expected, result )    \
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (expected), (result), 1e-14 )

#define ASSERT_DOUBLES_EQUAL_MESSAGE( message, expected, result )   \
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( (message), (expected), (result), 1e-14 )

SCQAHELPER_DLLPUBLIC void checkFormula(ScDocument& rDoc, const ScAddress& rPos,
        const char* expected, const char* msg, CppUnit::SourceLine const & sourceLine);

#define ASSERT_FORMULA_EQUAL(doc, pos, expected, msg) \
    checkFormula(doc, pos, expected, msg, CPPUNIT_SOURCELINE())

SCQAHELPER_DLLPUBLIC void testFormats(ScBootstrapFixture* pTest, ScDocument* pDoc, sal_Int32 nFormat);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
