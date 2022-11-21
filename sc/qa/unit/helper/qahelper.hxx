/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <docsh.hxx>
#include <address.hxx>

#include <cppunit/SourceLine.h>

#include <test/unoapixml_test.hxx>
#include <test/bootstrapfixture.hxx>
#include <comphelper/documentconstants.hxx>

#include <comphelper/fileformat.h>
#include <formula/grammar.hxx>
#include "scqahelperdllapi.h"

#include <string>
#include <string_view>
#include <sstream>
#include <undoblk.hxx>

#include <sal/types.h>

#include <memory>
#include <tuple>

namespace utl { class TempFileNamed; }

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

SCQAHELPER_DLLPUBLIC bool testEqualsWithTolerance( tools::Long nVal1, tools::Long nVal2, tools::Long nTol );

#define CHECK_OPTIMAL 0x1

class SdrOle2Obj;
class ScRangeList;
class ScTokenArray;

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
    const std::u16string_view sTestDoc;
    const OUString sExportType; // empty for import test, otherwise this is an export test
    int nRowData;
    RowData const * pData;
};

struct RangeNameDef
{
    const char* mpName;
    const char* mpExpr;
    sal_uInt16 mnIndex;
};

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; SfxFilterFlags nFormatType;
};

// Printers for the calc data structures. Needed for the EQUAL assertion
// macros from CPPUNIT.

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const ScAddress& rAddr);

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const ScRange& rRange);

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const ScRangeList& rList);

SCQAHELPER_DLLPUBLIC std::ostream& operator<<(std::ostream& rStrm, const OpCode& rCode);

SCQAHELPER_DLLPUBLIC bool checkOutput(
    const ScDocument* pDoc, const ScRange& aOutRange,
    const std::vector<std::vector<const char*>>& aCheck, const char* pCaption );

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

/**
 * Temporarily set formula grammar.
 */
class FormulaGrammarSwitch
{
    ScDocument* mpDoc;
    formula::FormulaGrammar::Grammar meOldGrammar;

public:
    FormulaGrammarSwitch(ScDocument* pDoc, formula::FormulaGrammar::Grammar eGrammar);
    ~FormulaGrammarSwitch();
};

class SCQAHELPER_DLLPUBLIC ScBootstrapFixture : public test::BootstrapFixture
{
    static const FileFormat aFileFormats[];
protected:
    OUString m_aBaseString;

    ScDocShellRef load(
        const OUString& rURL, const OUString& rFilter, const OUString &rUserData,
        const OUString& rTypeName, SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
        sal_Int32 nFilterVersion = SOFFICE_FILEFORMAT_CURRENT, const OUString* pPassword = nullptr );

    ScDocShellRef loadDoc(
        std::u16string_view rFileName, sal_Int32 nFormat, bool bCheckErrorCode = true );

private:
    // reference to document interface that we are testing
    css::uno::Reference<css::uno::XInterface> m_xCalcComponent;
public:
    explicit ScBootstrapFixture( const OUString& rsBaseString );
    virtual ~ScBootstrapFixture() override;

    void createFileURL(std::u16string_view aFileBase, std::u16string_view aFileExtension, OUString& rFilePath);

    virtual void setUp() override;
    virtual void tearDown() override;
};

class SCQAHELPER_DLLPUBLIC ScSimpleBootstrapFixture : public test::BootstrapFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    ScRange insertRangeData(ScDocument* pDoc, const ScAddress& rPos,
                                       const std::vector<std::vector<const char*>>& rData);
    void copyToClip(ScDocument* pSrcDoc, const ScRange& rRange, ScDocument* pClipDoc);
    void pasteFromClip(ScDocument* pDestDoc, const ScRange& rDestRange,
                                        ScDocument* pClipDoc);
    ScUndoPaste* createUndoPaste(ScDocShell& rDocSh, const ScRange& rRange,
                                        ScDocumentUniquePtr pUndoDoc);
    void pasteOneCellFromClip(ScDocument* pDestDoc, const ScRange& rDestRange,
                                         ScDocument* pClipDoc,
                                         InsertDeleteFlags eFlags = InsertDeleteFlags::ALL);
    void setCalcAsShown(ScDocument* pDoc, bool bCalcAsShown);
    ScDocShell* findLoadedDocShellByName(std::u16string_view rName);
    ScUndoCut* cutToClip(ScDocShell& rDocSh, const ScRange& rRange, ScDocument* pClipDoc,
                                        bool bCreateUndo);
    bool insertRangeNames(ScDocument* pDoc, ScRangeName* pNames, const RangeNameDef* p,
                                       const RangeNameDef* pEnd);
    OUString getRangeByName(ScDocument* pDoc, const OUString& aRangeName);
    void printFormula(ScDocument* pDoc, SCCOL nCol, SCROW nRow, SCTAB nTab,
                                           const char* pCaption = nullptr);
    void printRange(ScDocument* pDoc, const ScRange& rRange, const char* pCaption,
                                         const bool printFormula = false);
    void printRange(ScDocument* pDoc, const ScRange& rRange,
                                         const OString& rCaption, const bool printFormula = false);
    void clearRange(ScDocument* pDoc, const ScRange& rRange);
    void clearSheet(ScDocument* pDoc, SCTAB nTab);
    bool checkFormulaPosition(ScDocument& rDoc, const ScAddress& rPos);
    bool checkFormulaPositions(
        ScDocument& rDoc, SCTAB nTab, SCCOL nCol, const SCROW* pRows, size_t nRowCount);
    std::unique_ptr<ScTokenArray> compileFormula(
        ScDocument* pDoc, const OUString& rFormula,
        formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_NATIVE );
    void clearFormulaCellChangedFlag( ScDocument& rDoc, const ScRange& rRange );

protected:
    ScDocShellRef m_xDocShell;
    ScDocument* m_pDoc;
};

class SCQAHELPER_DLLPUBLIC ScModelTestBase : public UnoApiXmlTest
{
public:
    ScModelTestBase(OUString path)
        : UnoApiXmlTest(path)
    {
    }

    void createScDoc(const char* pName = nullptr, const char* pPassword = nullptr);
    ScDocument* getScDoc();
    ScDocShell* getScDocShell();
    ScTabViewShell* getViewShell();
    void miscRowHeightsTest( TestParam const * aTestValues, unsigned int numElems);

    void testFile(const OUString& aFileName, ScDocument& rDoc, SCTAB nTab, StringType aStringFormat = StringType::StringValue);

    //need own handler because conditional formatting strings must be generated
    void testCondFile(const OUString& aFileName, ScDocument* pDoc, SCTAB nTab);

    const SdrOle2Obj* getSingleOleObject(ScDocument& rDoc, sal_uInt16 nPage);

    const SdrOle2Obj* getSingleChartObject(ScDocument& rDoc, sal_uInt16 nPage);

    ScRangeList getChartRanges(ScDocument& rDoc, const SdrOle2Obj& rChartObj);

    void testFormats(ScDocument* pDoc,std::u16string_view sFormat);

private:
    // Why is this here and not in osl, and using the already existing file
    // handling APIs? Do we really want to add arbitrary new file handling
    // wrappers here and there (and then having to handle the Android (and
    // eventually perhaps iOS) special cases here, too)?  Please move this to osl,
    // it sure looks generally useful. Or am I missing something?

    void loadFile(const OUString& aFileName, std::string& aContent);
};

#define ASSERT_DOUBLES_EQUAL( expected, result )    \
    CPPUNIT_ASSERT_DOUBLES_EQUAL( (expected), (result), 1e-14 )

#define ASSERT_DOUBLES_EQUAL_MESSAGE( message, expected, result )   \
    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE( (message), (expected), (result), 1e-14 )

SCQAHELPER_DLLPUBLIC ScTokenArray* getTokens(ScDocument& rDoc, const ScAddress& rPos);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
