/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "helper/qahelper.hxx"
#include "helper/shared_test_impl.hxx"

#include <userdat.hxx>
#include <tokenstringcontext.hxx>
#include <chgtrack.hxx>
#include <scmod.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdomeas.hxx>
#include <svl/zformat.hxx>
#include <svl/numformat.hxx>
#include <tabprotection.hxx>
#include <editeng/borderline.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/useroptions.hxx>
#include <sfx2/docfile.hxx>
#include <tools/datetime.hxx>

#include <com/sun/star/drawing/XDrawPageSupplier.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::std::cerr;
using ::std::endl;

class ScExportTest3 : public ScModelTestBase
{
public:
    ScExportTest3()
        : ScModelTestBase("sc/qa/unit/data")
    {
    }

protected:
    void testCeilingFloor(const OUString& sFormatType);
    void testFunctionsExcel2010(const OUString& sFormatType);
};

CPPUNIT_TEST_FIXTURE(ScExportTest3, testBordersExchangeXLSX)
{
    // Document: sc/qa/unit/data/README.cellborders

    // short name for the table
    const SvxBorderLineStyle None = SvxBorderLineStyle::NONE;
    const SvxBorderLineStyle Solid = SvxBorderLineStyle::SOLID;
    const SvxBorderLineStyle Dotted = SvxBorderLineStyle::DOTTED;
    const SvxBorderLineStyle Dashed = SvxBorderLineStyle::DASHED;
    const SvxBorderLineStyle FineDash = SvxBorderLineStyle::FINE_DASHED;
    const SvxBorderLineStyle DashDot = SvxBorderLineStyle::DASH_DOT;
    const SvxBorderLineStyle DashDoDo = SvxBorderLineStyle::DASH_DOT_DOT;
    const SvxBorderLineStyle DoubThin = SvxBorderLineStyle::DOUBLE_THIN;

    const size_t nMaxCol = 18;
    const size_t nMaxRow = 7;

    static struct
    {
        SvxBorderLineStyle BorderStyleTop, BorderStyleBottom;
        tools::Long WidthTop, WidthBottom;
    } aCheckBorderWidth[nMaxCol][nMaxRow]
        = { /* Width */
            /* 0,05 */ { { Solid, Solid, 1, 1 }, // SOLID
                         { Dotted, Dotted, 15, 15 }, // DOTTED
                         { Dotted, Dotted, 15, 15 }, // DASHED
                         { FineDash, FineDash, 15, 15 }, // FINE_DASHED
                         { FineDash, FineDash, 15, 15 }, // DASH_DOT
                         { FineDash, FineDash, 15, 15 }, // DASH_DOT_DOT
                         { None, None, 0, 0 } }, // DOUBLE_THIN
            /* 0,25 */
            { { Solid, Solid, 1, 1 },
              { Dotted, Dotted, 15, 15 },
              { Dotted, Dotted, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { None, None, 0, 0 } },
            /* 0,50 */
            { { Solid, Solid, 1, 1 },
              { Dotted, Dotted, 15, 15 },
              { Dotted, Dotted, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { None, None, 0, 0 } },
            /* 0,75 */
            { { Solid, Solid, 15, 15 },
              { Dotted, Dotted, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { DashDot, DashDot, 15, 15 },
              { DashDoDo, DashDoDo, 15, 15 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 1,00 */
            { { Solid, Solid, 15, 15 },
              { Dotted, Dotted, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { DashDot, DashDot, 15, 15 },
              { DashDoDo, DashDoDo, 15, 15 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 1,25 */
            { { Solid, Solid, 15, 15 },
              { Dotted, Dotted, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { DashDot, DashDot, 15, 15 },
              { DashDoDo, DashDoDo, 15, 15 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 1,50 */
            { { Solid, Solid, 15, 15 },
              { Dotted, Dotted, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { FineDash, FineDash, 15, 15 },
              { DashDot, DashDot, 15, 15 },
              { DashDoDo, DashDoDo, 15, 15 },
              { DoubThin, DoubThin, 35, 35 } },

            /* 1,75 */
            { { Solid, Solid, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 2,00 */
            { { Solid, Solid, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 2,25 */
            { { Solid, Solid, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },

            /* 2,50 */
            { { Solid, Solid, 50, 50 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 2,75 */
            { { Solid, Solid, 50, 50 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 3,00 */
            { { Solid, Solid, 50, 50 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 3,50 */
            { { Solid, Solid, 50, 50 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 4,00 */
            { { Solid, Solid, 50, 50 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 5,00 */
            { { Solid, Solid, 50, 50 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 7,00 */
            { { Solid, Solid, 50, 50 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } },
            /* 9,00 */
            { { Solid, Solid, 50, 50 },
              { FineDash, FineDash, 35, 35 },
              { Dashed, Dashed, 35, 35 },
              { FineDash, FineDash, 35, 35 },
              { DashDot, DashDot, 35, 35 },
              { DashDoDo, DashDoDo, 35, 35 },
              { DoubThin, DoubThin, 35, 35 } }
          };

    createScDoc("ods/test_borders_export.ods");

    saveAndReload("Calc Office Open XML");
    ScDocument* pDoc = getScDoc();

    for (size_t nCol = 0; nCol < nMaxCol; ++nCol)
    {
        for (size_t nRow = 0; nRow < nMaxRow; ++nRow)
        {
            const editeng::SvxBorderLine* pLineTop = nullptr;
            const editeng::SvxBorderLine* pLineBottom = nullptr;
            pDoc->GetBorderLines(nCol + 2, (nRow * 2) + 8, 0, nullptr, &pLineTop, nullptr,
                                 &pLineBottom);
            if ((nCol < 3) && (nRow == 6))
            { // in this range no lines since minimum size to create a double is 0.5
                CPPUNIT_ASSERT(!pLineTop);
                CPPUNIT_ASSERT(!pLineBottom);
                continue;
            }
            else
            {
                CPPUNIT_ASSERT(pLineTop);
                CPPUNIT_ASSERT(pLineBottom);
            }

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Top Border-Line-Style wrong",
                                         aCheckBorderWidth[nCol][nRow].BorderStyleTop,
                                         pLineTop->GetBorderLineStyle());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom Border-Line-Style wrong",
                                         aCheckBorderWidth[nCol][nRow].BorderStyleBottom,
                                         pLineBottom->GetBorderLineStyle());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Top Width-Line wrong",
                                         aCheckBorderWidth[nCol][nRow].WidthTop,
                                         pLineTop->GetWidth());
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Bottom Width-Line wrong",
                                         aCheckBorderWidth[nCol][nRow].WidthBottom,
                                         pLineBottom->GetWidth());
        }
    }
}

static OUString toString(const ScBigRange& rRange)
{
    return "(columns:" + OUString::number(rRange.aStart.Col()) + "-"
           + OUString::number(rRange.aEnd.Col()) + ";rows:" + OUString::number(rRange.aStart.Row())
           + "-" + OUString::number(rRange.aEnd.Row())
           + ";sheets:" + OUString::number(rRange.aStart.Tab()) + "-"
           + OUString::number(rRange.aEnd.Tab()) + ")";
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTrackChangesSimpleXLSX)
{
    struct CheckItem
    {
        sal_uLong mnActionId;
        ScChangeActionType meType;

        sal_Int32 mnStartCol;
        sal_Int32 mnStartRow;
        sal_Int32 mnStartTab;
        sal_Int32 mnEndCol;
        sal_Int32 mnEndRow;
        sal_Int32 mnEndTab;

        bool mbRowInsertedAtBottom;
    };

    struct
    {
        bool checkRange(ScChangeActionType eType, const ScBigRange& rExpected,
                        const ScBigRange& rActual)
        {
            ScBigRange aExpected(rExpected), aActual(rActual);

            switch (eType)
            {
                case SC_CAT_INSERT_ROWS:
                {
                    // Ignore columns.
                    aExpected.aStart.SetCol(0);
                    aExpected.aEnd.SetCol(0);
                    aActual.aStart.SetCol(0);
                    aActual.aEnd.SetCol(0);
                }
                break;
                default:;
            }

            return aExpected == aActual;
        }

        bool check(ScDocument& rDoc)
        {
            static const CheckItem aChecks[] = {
                { 1, SC_CAT_CONTENT, 1, 1, 0, 1, 1, 0, false },
                { 2, SC_CAT_INSERT_ROWS, 0, 2, 0, 0, 2, 0, true },
                { 3, SC_CAT_CONTENT, 1, 2, 0, 1, 2, 0, false },
                { 4, SC_CAT_INSERT_ROWS, 0, 3, 0, 0, 3, 0, true },
                { 5, SC_CAT_CONTENT, 1, 3, 0, 1, 3, 0, false },
                { 6, SC_CAT_INSERT_ROWS, 0, 4, 0, 0, 4, 0, true },
                { 7, SC_CAT_CONTENT, 1, 4, 0, 1, 4, 0, false },
                { 8, SC_CAT_INSERT_ROWS, 0, 5, 0, 0, 5, 0, true },
                { 9, SC_CAT_CONTENT, 1, 5, 0, 1, 5, 0, false },
                { 10, SC_CAT_INSERT_ROWS, 0, 6, 0, 0, 6, 0, true },
                { 11, SC_CAT_CONTENT, 1, 6, 0, 1, 6, 0, false },
                { 12, SC_CAT_INSERT_ROWS, 0, 7, 0, 0, 7, 0, true },
                { 13, SC_CAT_CONTENT, 1, 7, 0, 1, 7, 0, false },
            };

            ScChangeTrack* pCT = rDoc.GetChangeTrack();
            if (!pCT)
            {
                cerr << "Change track instance doesn't exist." << endl;
                return false;
            }

            sal_uLong nActionMax = pCT->GetActionMax();
            if (nActionMax != 13)
            {
                cerr << "Unexpected highest action ID value." << endl;
                return false;
            }

            for (size_t i = 0; i < SAL_N_ELEMENTS(aChecks); ++i)
            {
                sal_uInt16 nActId = aChecks[i].mnActionId;
                const ScChangeAction* pAction = pCT->GetAction(nActId);
                if (!pAction)
                {
                    cerr << "No action for action number " << nActId << " found." << endl;
                    return false;
                }

                if (pAction->GetType() != aChecks[i].meType)
                {
                    cerr << "Unexpected action type for action number " << nActId << "." << endl;
                    return false;
                }

                const ScBigRange& rRange = pAction->GetBigRange();
                ScBigRange aCheck(aChecks[i].mnStartCol, aChecks[i].mnStartRow,
                                  aChecks[i].mnStartTab, aChecks[i].mnEndCol, aChecks[i].mnEndRow,
                                  aChecks[i].mnEndTab);

                if (!checkRange(pAction->GetType(), aCheck, rRange))
                {
                    cerr << "Unexpected range for action number " << nActId
                         << ": expected=" << toString(aCheck) << " actual=" << toString(rRange)
                         << endl;
                    return false;
                }

                switch (pAction->GetType())
                {
                    case SC_CAT_INSERT_ROWS:
                    {
                        const ScChangeActionIns* p = static_cast<const ScChangeActionIns*>(pAction);
                        if (p->IsEndOfList() != aChecks[i].mbRowInsertedAtBottom)
                        {
                            cerr << "Unexpected end-of-list flag for action number " << nActId
                                 << "." << endl;
                            return false;
                        }
                    }
                    break;
                    default:;
                }
            }

            return true;
        }

        bool checkRevisionUserAndTime(ScDocument& rDoc, std::u16string_view rOwnerName)
        {
            ScChangeTrack* pCT = rDoc.GetChangeTrack();
            if (!pCT)
            {
                cerr << "Change track instance doesn't exist." << endl;
                return false;
            }

            ScChangeAction* pAction = pCT->GetLast();
            if (pAction->GetUser() != "Kohei Yoshida")
            {
                cerr << "Wrong user name." << endl;
                return false;
            }

            DateTime aDT = pAction->GetDateTime();
            if (aDT.GetYear() != 2014 || aDT.GetMonth() != 7 || aDT.GetDay() != 11)
            {
                cerr << "Wrong time stamp." << endl;
                return false;
            }

            // Insert a new record to make sure the user and date-time are correct.
            rDoc.SetString(ScAddress(1, 8, 0), "New String");
            ScCellValue aEmpty;
            pCT->AppendContent(ScAddress(1, 8, 0), aEmpty);
            pAction = pCT->GetLast();
            if (!pAction)
            {
                cerr << "Failed to retrieve last revision." << endl;
                return false;
            }

            if (rOwnerName != pAction->GetUser())
            {
                cerr << "Wrong user name." << endl;
                return false;
            }

            DateTime aDTNew = pAction->GetDateTime();
            if (aDTNew <= aDT)
            {
                cerr << "Time stamp of the new revision should be more recent than that of the "
                        "last revision."
                     << endl;
                return false;
            }

            return true;
        }

    } aTest;

    SvtUserOptions& rUserOpt = SC_MOD()->GetUserOptions();
    rUserOpt.SetToken(UserOptToken::FirstName, "Export");
    rUserOpt.SetToken(UserOptToken::LastName, "Test");

    OUString aOwnerName = rUserOpt.GetFirstName() + " " + rUserOpt.GetLastName();

    // First, test the xls variant.

    createScDoc("xls/track-changes/simple-cell-changes.xls");
    ScDocument* pDoc = getScDoc();
    bool bGood = aTest.check(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Initial check failed (xls).", bGood);

    saveAndReload("MS Excel 97");
    pDoc = getScDoc();
    bGood = aTest.check(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Check after reload failed (xls).", bGood);

    // fdo#81445 : Check the blank value string to make sure it's "<empty>".
    ScChangeTrack* pCT = pDoc->GetChangeTrack();
    CPPUNIT_ASSERT(pCT);
    ScChangeAction* pAction = pCT->GetAction(1);
    CPPUNIT_ASSERT(pAction);
    OUString aDesc = pAction->GetDescription(*pDoc);
    CPPUNIT_ASSERT_EQUAL(OUString("Cell B2 changed from '<empty>' to '1'"), aDesc);

    pDoc = getScDoc();
    bGood = aTest.checkRevisionUserAndTime(*pDoc, aOwnerName);
    CPPUNIT_ASSERT_MESSAGE("Check revision and time failed after reload (xls).", bGood);

    // Now, test the xlsx variant the same way.

    createScDoc("xlsx/track-changes/simple-cell-changes.xlsx");
    pDoc = getScDoc();
    aTest.check(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Initial check failed (xlsx).", bGood);

    saveAndReload("Calc Office Open XML");
    pDoc = getScDoc();
    bGood = aTest.check(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Check after reload failed (xlsx).", bGood);

    bGood = aTest.checkRevisionUserAndTime(*pDoc, aOwnerName);
    CPPUNIT_ASSERT_MESSAGE("Check revision and time failed after reload (xlsx).", bGood);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSheetTabColorsXLSX)
{
    struct
    {
        bool checkContent(ScDocument& rDoc)
        {
            std::vector<OUString> aTabNames = rDoc.GetAllTableNames();

            // green, red, blue, yellow (from left to right).
            if (aTabNames.size() != 4)
            {
                cerr << "There should be exactly 4 sheets." << endl;
                return false;
            }

            const char* pNames[] = { "Green", "Red", "Blue", "Yellow" };
            for (size_t i = 0; i < SAL_N_ELEMENTS(pNames); ++i)
            {
                OUString aExpected = OUString::createFromAscii(pNames[i]);
                if (aExpected != aTabNames[i])
                {
                    cerr << "incorrect sheet name: expected='" << aExpected << "', actual='"
                         << aTabNames[i] << "'" << endl;
                    return false;
                }
            }

            static const Color aXclColors[] = {
                0x0000B050, // green
                0x00FF0000, // red
                0x000070C0, // blue
                0x00FFFF00, // yellow
            };

            for (size_t i = 0; i < SAL_N_ELEMENTS(aXclColors); ++i)
            {
                if (aXclColors[i] != rDoc.GetTabBgColor(i))
                {
                    cerr << "wrong sheet color for sheet " << i << endl;
                    return false;
                }
            }

            return true;
        }

    } aTest;

    createScDoc("xlsx/sheet-tab-color.xlsx");
    {
        ScDocument* pDoc = getScDoc();
        bool bRes = aTest.checkContent(*pDoc);
        CPPUNIT_ASSERT_MESSAGE("Failed on the initial content check.", bRes);
    }

    saveAndReload("Calc Office Open XML");
    ScDocument* pDoc = getScDoc();
    bool bRes = aTest.checkContent(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Failed on the content check after reload.", bRes);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTdf133487)
{
    createScDoc("fods/shapes_foreground_background.fods");

    save("calc8");
    xmlDocUniquePtr pXmlDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pXmlDoc);

    // shape in background has lowest index
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:table-row[1]/table:table-cell[1]/draw:custom-shape"_ostr,
                "z-index"_ostr, "0");
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:table-row[1]/table:table-cell[1]/draw:custom-shape"
                "/attribute::table:table-background"_ostr,
                1);
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:table-row[1]/table:table-cell[1]/draw:custom-shape"_ostr,
                "table-background"_ostr, "true");
    // shape in foreground, previously index 1
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:table-row[1]/table:table-cell[2]/draw:custom-shape"_ostr,
                "z-index"_ostr, "2");
    // attribute is only written for value "true"
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:table-row[1]/table:table-cell[2]/draw:custom-shape"
                "/attribute::table:table-background"_ostr,
                0);
    // shape in foreground, previously index 0
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:table-row[3]/table:table-cell[1]/draw:custom-shape"_ostr,
                "z-index"_ostr, "1");
    // attribute is only written for value "true"
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:table-row[3]/table:table-cell[1]/draw:custom-shape"
                "/attribute::table:table-background"_ostr,
                0);
    // shape in foreground, previously index 4
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:shapes/draw:custom-shape"_ostr,
                "z-index"_ostr, "3");
    // attribute is only written for value "true"
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:shapes/draw:custom-shape"
                "/attribute::table:table-background"_ostr,
                0);
    // form control, previously index 3
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:shapes/draw:control"_ostr,
                "z-index"_ostr, "4");
    // attribute is only written for value "true"
    assertXPath(pXmlDoc,
                "/office:document-content/office:body/office:spreadsheet/table:table[1]/"
                "table:shapes/draw:control"
                "/attribute::table:table-background"_ostr,
                0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSharedFormulaExportXLS)
{
    struct
    {
        bool checkContent(ScDocument& rDoc)
        {
            formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_ENGLISH_XL_R1C1;
            FormulaGrammarSwitch aFGSwitch(&rDoc, eGram);
            sc::TokenStringContext aCxt(rDoc, eGram);

            // Check the title row.

            OUString aActual = rDoc.GetString(0, 1, 0);
            OUString aExpected = "Response";
            if (aActual != aExpected)
            {
                cerr << "Wrong content in A2: expected='" << aExpected << "', actual='" << aActual
                     << "'" << endl;
                return false;
            }

            aActual = rDoc.GetString(1, 1, 0);
            aExpected = "Response";
            if (aActual != aExpected)
            {
                cerr << "Wrong content in B2: expected='" << aExpected << "', actual='" << aActual
                     << "'" << endl;
                return false;
            }

            // A3:A12 and B3:B12 are numbers from 1 to 10.
            for (SCROW i = 0; i <= 9; ++i)
            {
                double fExpected = i + 1.0;
                ScAddress aPos(0, i + 2, 0);
                double fActual = rDoc.GetValue(aPos);
                if (fExpected != fActual)
                {
                    cerr << "Wrong value in A" << (i + 2) << ": expected=" << fExpected
                         << ", actual=" << fActual << endl;
                    return false;
                }

                aPos.IncCol();
                ScFormulaCell* pFC = rDoc.GetFormulaCell(aPos);
                if (!pFC)
                {
                    cerr << "B" << (i + 2) << " should be a formula cell." << endl;
                    return false;
                }

                OUString aFormula = pFC->GetCode()->CreateString(aCxt, aPos);
                aExpected = "Coefficients!RC[-1]";
                if (aFormula != aExpected)
                {
                    cerr << "Wrong formula in B" << (i + 2) << ": expected='" << aExpected
                         << "', actual='" << aFormula << "'" << endl;
                    return false;
                }

                fActual = rDoc.GetValue(aPos);
                if (fExpected != fActual)
                {
                    cerr << "Wrong value in B" << (i + 2) << ": expected=" << fExpected
                         << ", actual=" << fActual << endl;
                    return false;
                }
            }

            return true;
        }

    } aTest;

    createScDoc("ods/shared-formula/3d-reference.ods");
    {
        // Check the content of the original.
        ScDocument* pDoc = getScDoc();
        bool bRes = aTest.checkContent(*pDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the original document failed.", bRes);
    }

    saveAndReload("MS Excel 97");

    // Check the content of the reloaded. This should be identical.
    ScDocument* pDoc = getScDoc();
    bool bRes = aTest.checkContent(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Content check on the reloaded document failed.", bRes);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSharedFormulaExportXLSX)
{
    struct
    {
        bool checkContent(ScDocument& rDoc)
        {
            SCTAB nTabCount = rDoc.GetTableCount();
            if (nTabCount != 2)
            {
                cerr << "Document should have exactly 2 sheets.  " << nTabCount << " found."
                     << endl;
                return false;
            }

            // Make sure the sheet tab colors are not set.
            for (SCROW i = 0; i <= 1; ++i)
            {
                Color aTabBgColor = rDoc.GetTabBgColor(i);
                if (aTabBgColor != COL_AUTO)
                {
                    cerr << "The tab color of Sheet " << (i + 1) << " should not be explicitly set."
                         << endl;
                    return false;
                }
            }

            // B2:B7 should show 1,2,3,4,5,6.
            double fExpected = 1.0;
            for (SCROW i = 1; i <= 6; ++i, ++fExpected)
            {
                ScAddress aPos(1, i, 0);
                double fVal = rDoc.GetValue(aPos);
                if (fVal != fExpected)
                {
                    cerr << "Wrong value in B" << (i + 1) << ": expected=" << fExpected
                         << ", actual=" << fVal << endl;
                    return false;
                }
            }

            // C2:C7 should show 10,20,...,60.
            fExpected = 10.0;
            for (SCROW i = 1; i <= 6; ++i, fExpected += 10.0)
            {
                ScAddress aPos(2, i, 0);
                double fVal = rDoc.GetValue(aPos);
                if (fVal != fExpected)
                {
                    cerr << "Wrong value in C" << (i + 1) << ": expected=" << fExpected
                         << ", actual=" << fVal << endl;
                    return false;
                }
            }

            // D2:D7 should show 1,2,...,6.
            fExpected = 1.0;
            for (SCROW i = 1; i <= 6; ++i, ++fExpected)
            {
                ScAddress aPos(3, i, 0);
                double fVal = rDoc.GetValue(aPos);
                if (fVal != fExpected)
                {
                    cerr << "Wrong value in D" << (i + 1) << ": expected=" << fExpected
                         << ", actual=" << fVal << endl;
                    return false;
                }
            }

            return true;
        }

    } aTest;

    createScDoc("xlsx/shared-formula/3d-reference.xlsx");
    {
        ScDocument* pDoc = getScDoc();
        bool bRes = aTest.checkContent(*pDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the initial document failed.", bRes);

        pDoc->CalcAll(); // Recalculate to flush all cached results.
        bRes = aTest.checkContent(*pDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the initial recalculated document failed.", bRes);
    }

    // Save and reload, and check the content again.
    saveAndReload("Calc Office Open XML");

    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll(); // Recalculate to flush all cached results.

    bool bRes = aTest.checkContent(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Content check on the reloaded document failed.", bRes);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSharedFormulaStringResultExportXLSX)
{
    struct
    {
        bool checkContent(ScDocument& rDoc)
        {
            {
                // B2:B7 should show A,B,...,F.
                const char* const expected[] = { "A", "B", "C", "D", "E", "F" };
                for (SCROW i = 0; i <= 5; ++i)
                {
                    ScAddress aPos(1, i + 1, 0);
                    OUString aStr = rDoc.GetString(aPos);
                    OUString aExpected = OUString::createFromAscii(expected[i]);
                    if (aStr != aExpected)
                    {
                        cerr << "Wrong value in B" << (i + 2) << ": expected='" << aExpected
                             << "', actual='" << aStr << "'" << endl;
                        return false;
                    }
                }
            }

            {
                // C2:C7 should show AA,BB,...,FF.
                const char* const expected[] = { "AA", "BB", "CC", "DD", "EE", "FF" };
                for (SCROW i = 0; i <= 5; ++i)
                {
                    ScAddress aPos(2, i + 1, 0);
                    OUString aStr = rDoc.GetString(aPos);
                    OUString aExpected = OUString::createFromAscii(expected[i]);
                    if (aStr != aExpected)
                    {
                        cerr << "Wrong value in C" << (i + 2) << ": expected='" << aExpected
                             << "', actual='" << aStr << "'" << endl;
                        return false;
                    }
                }
            }

            return true;
        }

    } aTest;

    createScDoc("xlsx/shared-formula/text-results.xlsx");
    {
        ScDocument* pDoc = getScDoc();

        // Check content without re-calculation, to test cached formula results.
        bool bRes = aTest.checkContent(*pDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the initial document failed.", bRes);

        // Now, re-calculate and check the results.
        pDoc->CalcAll();
        bRes = aTest.checkContent(*pDoc);
        CPPUNIT_ASSERT_MESSAGE("Content check on the initial recalculated document failed.", bRes);
    }
    // Reload and check again.
    saveAndReload("Calc Office Open XML");
    ScDocument* pDoc = getScDoc();

    bool bRes = aTest.checkContent(*pDoc);
    CPPUNIT_ASSERT_MESSAGE("Content check on the reloaded document failed.", bRes);
}

void ScExportTest3::testFunctionsExcel2010(const OUString& sFormatType)
{
    createScDoc("xlsx/functions-excel-2010.xlsx");

    saveAndReload(sFormatType);
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll(); // perform hard re-calculation.

    testFunctionsExcel2010_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testFunctionsExcel2010XLSX)
{
    testFunctionsExcel2010("Calc Office Open XML");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testFunctionsExcel2010XLS)
{
    testFunctionsExcel2010("MS Excel 97");
}

void ScExportTest3::testCeilingFloor(const OUString& sFormatType)
{
    createScDoc("xlsx/ceiling-floor.xlsx");

    saveAndReload(sFormatType);
    ScDocument* pDoc = getScDoc();
    pDoc->CalcAll(); // perform hard re-calculation.

    testCeilingFloor_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testCeilingFloorXLSX)
{
    testCeilingFloor("Calc Office Open XML");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testCeilingFloorODSToXLSX)
{
    // tdf#100011 - Cannot open sheet containing FLOOR/CEILING functions by MS Excel, after export to .xlsx
    createScDoc("ods/ceiling-floor.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/workbook.xml");
    CPPUNIT_ASSERT(pSheet);

    // there shouldn't be any defined names during export of FLOOR and CEILING functions to .xlsx
    assertXPath(pSheet, "/x:workbook/x:definedNames"_ostr, 0);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testCeilingFloorXLS) { testCeilingFloor("MS Excel 97"); }

CPPUNIT_TEST_FIXTURE(ScExportTest3, testCeilingFloorODS) { testCeilingFloor("calc8"); }

CPPUNIT_TEST_FIXTURE(ScExportTest3, testCustomXml)
{
    // Load document and export it to a temporary file
    createScDoc("xlsx/customxml.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pXmlDoc = parseExport("customXml/item1.xml");
    CPPUNIT_ASSERT(pXmlDoc);
    xmlDocUniquePtr pRelsDoc = parseExport("customXml/_rels/item1.xml.rels");
    CPPUNIT_ASSERT(pRelsDoc);

    // Check there is a relation to itemProps1.xml.
    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship"_ostr, 1);
    assertXPath(pRelsDoc, "/rels:Relationships/rels:Relationship[@Id='rId1']"_ostr, "Target"_ostr,
                "itemProps1.xml");

    std::unique_ptr<SvStream> pStream = parseExportStream(maTempFile.GetURL(), "ddp/ddpfile.xen");
    CPPUNIT_ASSERT(pStream);
}

#ifdef _WIN32
static sal_Unicode lcl_getWindowsDrive(const OUString& aURL)
{
    static const sal_Int32 nMinLen = strlen("file:///X:/");
    if (aURL.getLength() <= nMinLen)
        return 0;
    const OUString aUrlStart = aURL.copy(0, nMinLen);
    return (aUrlStart.startsWith("file:///") && aUrlStart.endsWith(":/")) ? aUrlStart[8] : 0;
}
#endif

CPPUNIT_TEST_FIXTURE(ScExportTest3, testRelativePathsODS)
{
    createScDoc("ods/fdo79305.ods");

    save("calc8");
    xmlDocUniquePtr pDoc = parseExport("content.xml");
    CPPUNIT_ASSERT(pDoc);
    OUString aURL = getXPath(pDoc,
                             "/office:document-content/office:body/office:spreadsheet/table:table/"
                             "table:table-row[2]/table:table-cell[2]/text:p/text:a"_ostr,
                             "href"_ostr);
#ifdef _WIN32
    // if the exported document is not on the same drive then the linked document,
    // there is no way to get a relative URL for the link, because ../X:/ is undefined.
    if (!aURL.startsWith(".."))
    {
        sal_Unicode aDocDrive = lcl_getWindowsDrive(maTempFile.GetURL());
        sal_Unicode aLinkDrive = lcl_getWindowsDrive(aURL);
        CPPUNIT_ASSERT_MESSAGE("document on the same drive but no relative link!", aDocDrive != 0);
        CPPUNIT_ASSERT_MESSAGE("document on the same drive but no relative link!", aLinkDrive != 0);
        CPPUNIT_ASSERT_MESSAGE("document on the same drive but no relative link!",
                               aDocDrive != aLinkDrive);
        return;
    }
#endif
    // make sure that the URL is relative
    CPPUNIT_ASSERT(aURL.startsWith(".."));
}

namespace
{
void testSheetProtection_Impl(ScDocument& rDoc)
{
    CPPUNIT_ASSERT(rDoc.IsTabProtected(0));
    const ScTableProtection* pTabProtection = rDoc.GetTabProtection(0);
    CPPUNIT_ASSERT(pTabProtection);
    CPPUNIT_ASSERT(pTabProtection->isOptionEnabled(ScTableProtection::SELECT_UNLOCKED_CELLS));
    CPPUNIT_ASSERT(!pTabProtection->isOptionEnabled(ScTableProtection::SELECT_LOCKED_CELLS));
}
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSheetProtectionODS)
{
    createScDoc("ods/sheet-protection.ods");

    ScDocument* pDoc = getScDoc();

    testSheetProtection_Impl(*pDoc);

    saveAndReload("calc8");

    pDoc = getScDoc();

    testSheetProtection_Impl(*pDoc);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testFunctionsExcel2010ODS)
{
    //testFunctionsExcel2010("calc8");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSwappedOutImageExport)
{
    std::vector<OUString> aFilterNames{ "calc8", "MS Excel 97", "Calc Office Open XML" };

    for (size_t i = 0; i < aFilterNames.size(); ++i)
    {
        // Check whether the export code swaps in the image which was swapped out before.
        createScDoc("ods/document_with_two_images.ods");

        const OString sFailedMessage
            = OString::Concat("Failed on filter: ") + aFilterNames[i].toUtf8();

        // Export the document and import again for a check
        saveAndReload(aFilterNames[i]);

        // Check whether graphic exported well after it was swapped out
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                     UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                       UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2),
                                     xDraws->getCount());

        uno::Reference<drawing::XShape> xImage(xDraws->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> XPropSet(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381),
                                         xBitmap->getSize().Height);
        }
        // Second Image
        xImage.set(xDraws->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600),
                                         xBitmap->getSize().Height);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSupBookVirtualPathXLS)
{
    createScDoc("xls/external-ref.xls");

    saveAndReload("MS Excel 97");

    ScDocument* pDoc = getScDoc();

    OUString aFormula = pDoc->GetFormula(0, 0, 0);
#ifdef _WIN32
    aFormula = OUString::Concat(aFormula.subView(0, 9)) + aFormula.subView(12);
    // strip drive letter, e.g. 'C:/'
#endif
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
        "Wrong SupBook VirtualPath URL",
        OUString("='file:///home/timar/Documents/external.xls'#$Sheet1.A1"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testLinkedGraphicRT)
{
    // Problem was with linked images
    std::vector<OUString> aFilterNames{ "calc8", "MS Excel 97", "Calc Office Open XML" };

    for (size_t i = 0; i < aFilterNames.size(); ++i)
    {
        // Load the original file with one image
        createScDoc("ods/document_with_linked_graphic.ods");
        const OString sFailedMessage
            = OString::Concat("Failed on filter: ") + aFilterNames[i].toUtf8();

        // Export the document and import again for a check
        saveAndReload(aFilterNames[i]);

        // Check whether graphic imported well after export
        ScDocument* pDoc = getScDoc();
        ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pDrawLayer != nullptr);
        const SdrPage* pPage = pDrawLayer->GetPage(0);
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pPage != nullptr);
        SdrGrafObj* pObject = dynamic_cast<SdrGrafObj*>(pPage->GetObj(0));
        CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pObject != nullptr);
        if (aFilterNames[i] == "Calc Office Open XML")
        {
            // FIXME: tdf#152036
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), !pObject->IsLinkedGraphic());
        }
        else
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), pObject->IsLinkedGraphic());

        const GraphicObject& rGraphicObj = pObject->GetGraphicObject(true);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), int(GraphicType::Bitmap),
                                     int(rGraphicObj.GetGraphic().GetType()));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), sal_uLong(864900),
                                     rGraphicObj.GetGraphic().GetSizeBytes());
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testImageWithSpecialID)
{
    std::vector<OUString> aFilterNames{ "calc8", "MS Excel 97", "Calc Office Open XML" };

    for (size_t i = 0; i < aFilterNames.size(); ++i)
    {
        createScDoc("ods/images_with_special_IDs.ods");

        const OString sFailedMessage
            = OString::Concat("Failed on filter: ") + aFilterNames[i].toUtf8();

        // Export the document and import again for a check
        saveAndReload(aFilterNames[i]);

        // Check whether graphic was exported well
        uno::Reference<sheet::XSpreadsheetDocument> xDoc(mxComponent, UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xIA(xDoc->getSheets(), UNO_QUERY_THROW);
        uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(xIA->getByIndex(0),
                                                                     UNO_QUERY_THROW);
        uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(),
                                                       UNO_QUERY_THROW);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(2),
                                     xDraws->getCount());

        uno::Reference<drawing::XShape> xImage(xDraws->getByIndex(0), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> XPropSet(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(610),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(381),
                                         xBitmap->getSize().Height);
        }
        // Second Image
        xImage.set(xDraws->getByIndex(1), uno::UNO_QUERY);
        XPropSet.set(xImage, uno::UNO_QUERY_THROW);

        // Check Graphic, Size
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            XPropSet->getPropertyValue("Graphic") >>= xGraphic;
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xGraphic.is());
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(),
                                   xGraphic->getType() != graphic::GraphicType::EMPTY);
            uno::Reference<awt::XBitmap> xBitmap(xGraphic, uno::UNO_QUERY);
            CPPUNIT_ASSERT_MESSAGE(sFailedMessage.getStr(), xBitmap.is());
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(900),
                                         xBitmap->getSize().Width);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(sFailedMessage.getStr(), static_cast<sal_Int32>(600),
                                         xBitmap->getSize().Height);
        }
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testAbsNamedRangeHTML)
{
    setImportFilterName("calc_HTML_WebQuery");
    createScDoc("html/numberformat.html");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    //reset import filter
    setImportFilterName("calc8");
    saveAndReload("calc8");
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    ScRangeData* pRangeData = pDoc->GetRangeName()->findByUpperName(OUString("HTML_1"));
    ScSingleRefData* pRef = pRangeData->GetCode()->FirstToken()->GetSingleRef();
    // see tdf#119141 for the reason why this isn't Sheet1.HTML_1
    CPPUNIT_ASSERT_MESSAGE("HTML_1 is an absolute reference", !pRef->IsTabRel());
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTdf80149)
{
    createScDoc("csv/tdf80149.csv");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    saveAndReload("Calc Office Open XML");
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT_EQUAL(OUString("row 1"), pDoc->GetString(0, 0, 0));

    // Without the fix in place, this test would have failed with
    // - Expected: Character 0x16 is here ->><<--
    // - Actual  :
    CPPUNIT_ASSERT_EQUAL(OUString("Character 0x16 is here ->><<--"), pDoc->GetString(1, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("File opens in libre office, but can't be saved as xlsx"),
                         pDoc->GetString(2, 0, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("row 2"), pDoc->GetString(0, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Subsequent rows get truncated"), pDoc->GetString(1, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("This cell goes missing"), pDoc->GetString(2, 1, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("row 3"), pDoc->GetString(0, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("Subsequent rows get truncated"), pDoc->GetString(1, 2, 0));
    CPPUNIT_ASSERT_EQUAL(OUString("This cell goes missing"), pDoc->GetString(2, 2, 0));
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSheetLocalRangeNameXLS)
{
    createScDoc("xls/named-ranges-local.xls");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    saveAndReload("MS Excel 97");
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    ScRangeName* pRangeName = pDoc->GetRangeName(0);
    CPPUNIT_ASSERT(pRangeName);
    CPPUNIT_ASSERT_EQUAL(size_t(2), pRangeName->size());

    OUString aFormula = pDoc->GetFormula(3, 11, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(local_name2)"), aFormula);
    ASSERT_DOUBLES_EQUAL(14.0, pDoc->GetValue(3, 11, 0));

    aFormula = pDoc->GetFormula(6, 4, 0);
    CPPUNIT_ASSERT_EQUAL(OUString("=local_name1"), aFormula);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testRelativeNamedExpressionsXLS)
{
    createScDoc("ods/tdf113991_relativeNamedRanges.ods");
    ScDocShell* pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();
    saveAndReload("MS Excel 97");
    pDocSh = getScDocShell();
    pDocSh->DoHardRecalc();

    ScDocument* pDoc = getScDoc();
    // Sheet1:G3
    ScAddress aPos(6, 2, 0);
    CPPUNIT_ASSERT_EQUAL(1.0, pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("=single_cell_A3"),
                         pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    // Sheet2:F6
    aPos = ScAddress(5, 5, 1);
    CPPUNIT_ASSERT_EQUAL(18.0, pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(test_conflict)"),
                         pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    // Sheet2:H3
    aPos = ScAddress(7, 2, 1);
    CPPUNIT_ASSERT_EQUAL(10.0, pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("=single_global_A3"),
                         pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
    // Sheet2:H6
    aPos = ScAddress(7, 5, 1);
    CPPUNIT_ASSERT_EQUAL(75.0, pDoc->GetValue(aPos));
    CPPUNIT_ASSERT_EQUAL(OUString("=SUM(A6:F6)"),
                         pDoc->GetFormula(aPos.Col(), aPos.Row(), aPos.Tab()));
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSheetTextBoxHyperlinkXLSX)
{
    createScDoc("xlsx/textbox-hyperlink.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(
        pDoc,
        "/xdr:wsDr[1]/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:nvSpPr[1]/xdr:cNvPr[1]/a:hlinkClick[1]"_ostr,
        1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testFontSizeXLSX)
{
    createScDoc("xlsx/fontSize.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    OUString fontSize = getXPath(
        pDoc, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr"_ostr,
        "sz"_ostr);
    // make sure that the font size is 18
    CPPUNIT_ASSERT_EQUAL(OUString("1800"), fontSize);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSheetCharacterKerningSpaceXLSX)
{
    createScDoc("xlsx/textbox-CharKerningSpace.xlsx");

    saveAndReload("Calc Office Open XML");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);

    OUString CharKerningSpace = getXPath(
        pDoc,
        "/xdr:wsDr[1]/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody[1]/a:p[1]/a:r[1]/a:rPr[1]"_ostr,
        "spc"_ostr);

    // make sure that the CharKerning is 2001 (2000 in original document).
    CPPUNIT_ASSERT_EQUAL(OUString("2001"), CharKerningSpace);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSheetCondensedCharacterSpaceXLSX)
{
    createScDoc("xlsx/textbox-CondensedCharacterSpace.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);

    OUString CondensedCharSpace = getXPath(
        pDoc,
        "/xdr:wsDr[1]/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody[1]/a:p[1]/a:r[1]/a:rPr[1]"_ostr,
        "spc"_ostr);

    // make sure that the CondensedCharSpace is -1001 (-1000 in original document).
    CPPUNIT_ASSERT_EQUAL(OUString("-1001"), CondensedCharSpace);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTextUnderlineColorXLSX)
{
    createScDoc("xlsx/underlineColor.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);
    // Make sure the underline type is double line
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr"_ostr,
                "u"_ostr, "dbl");

    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr"_ostr,
                "b"_ostr, "1");
    // Make sure that the underline color is RED
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr/a:uFill/"
                "a:solidFill/a:srgbClr"_ostr,
                "val"_ostr, "ff0000");

    // Make sure the underline type is drawn with heavy line
    assertXPath(pDoc,
                "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr"_ostr,
                "u"_ostr, "heavy");
    // tdf#104219 Make sure that uFill is not existing and uFillTx is set.
    // It mean that color is automatic, should be the same color as the text.
    assertXPath(
        pDoc,
        "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr/a:uFill"_ostr, 0);
    assertXPath(
        pDoc,
        "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:sp[1]/xdr:txBody/a:p[1]/a:r[1]/a:rPr/a:uFillTx"_ostr,
        1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testSheetRunParagraphPropertyXLSX)
{
    createScDoc("xlsx/TextColor.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/sharedStrings.xml");
    CPPUNIT_ASSERT(pDoc);

    OUString aColor = getXPath(pDoc, "/x:sst/x:si/x:r[1]/x:rPr[1]/x:color"_ostr, "rgb"_ostr);
    CPPUNIT_ASSERT_EQUAL(OUString("FFFF0000"), aColor);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testPreserveTextWhitespaceXLSX)
{
    createScDoc("xlsx/preserve-whitespace.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/sharedStrings.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:sst/x:si/x:t"_ostr, "space"_ostr, "preserve");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testPreserveTextWhitespace2XLSX)
{
    createScDoc("xlsx/preserve_space.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/sharedStrings.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/x:sst/x:si[1]/x:t"_ostr, "space"_ostr, "preserve");
    assertXPath(pDoc, "/x:sst/x:si[2]/x:t"_ostr, "space"_ostr, "preserve");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testHiddenShapeXLS)
{
    createScDoc("xls/hiddenShape.xls");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc->GetTableCount() > 0);
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    CPPUNIT_ASSERT_MESSAGE("Drawing object should not be visible.", !pObj->IsVisible());
    CPPUNIT_ASSERT_MESSAGE("Drawing object should not be printable.", !pObj->IsPrintable());
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testHiddenShapeXLSX)
{
    createScDoc("xlsx/hiddenShape.xlsx");

    ScDocument* pDoc = getScDoc();
    CPPUNIT_ASSERT(pDoc->GetTableCount() > 0);
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT(pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT(pObj);
    CPPUNIT_ASSERT_MESSAGE("Drawing object should not be visible.", !pObj->IsVisible());
    CPPUNIT_ASSERT_MESSAGE("Drawing object should not be printable.", !pObj->IsPrintable());

    save("Calc Office Open XML");
    xmlDocUniquePtr pDocXml = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDocXml);
    assertXPath(pDocXml, "/xdr:wsDr/xdr:twoCellAnchor/xdr:sp[1]/xdr:nvSpPr/xdr:cNvPr"_ostr,
                "hidden"_ostr, "1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testShapeAutofitXLSX)
{
    createScDoc("xlsx/testShapeAutofit.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/drawing1.xml");
    CPPUNIT_ASSERT(pDoc);

    // TextAutoGrowHeight --> "Fit height to text" / "Resize shape to fit text" --> true
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[1]/xdr:sp/xdr:txBody/a:bodyPr/a:spAutoFit"_ostr,
                1);
    // TextAutoGrowHeight --> "Fit height to text" / "Resize shape to fit text" --> false
    assertXPath(pDoc, "/xdr:wsDr/xdr:twoCellAnchor[2]/xdr:sp/xdr:txBody/a:bodyPr/a:noAutofit"_ostr,
                1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testHyperlinkXLSX)
{
    createScDoc("xlsx/hyperlink.xlsx");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/drawings/_rels/drawing1.xml.rels");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "/rels:Relationships/rels:Relationship"_ostr, "Target"_ostr, "#Sheet2!A1");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testMoveCellAnchoredShapesODS)
{
    createScDoc("ods/move-cell-anchored-shapes.ods");

    // There are two cell-anchored objects on the first sheet.
    ScDocument* pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

    // Check cell anchor state
    ScAnchorType oldType = ScDrawLayer::GetAnchorType(*pObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to get anchor type", SCA_CELL_RESIZE, oldType);

    // Get anchor data
    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pData->getShapeRect().IsEmpty());

    ScAddress aDataStart = pData->maStart;
    ScAddress aDataEnd = pData->maEnd;

    // Get non rotated anchor data
    ScDrawObjData* pNData = ScDrawLayer::GetNonRotatedObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pNData->getShapeRect().IsEmpty());

    ScAddress aNDataStart = pNData->maStart;
    ScAddress aNDataEnd = pNData->maEnd;
    CPPUNIT_ASSERT_EQUAL(aDataStart, aNDataStart);
    CPPUNIT_ASSERT_EQUAL(aDataEnd, aNDataEnd);

    // Insert 2 rows.
    pDoc->InsertRow(ScRange(0, aDataStart.Row() - 1, 0, pDoc->MaxCol(), aDataStart.Row(), 0));

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pData->getShapeRect().IsEmpty());

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pNData->getShapeRect().IsEmpty());

    // Check if data has moved to new rows
    CPPUNIT_ASSERT_EQUAL(pData->maStart.Row(), aDataStart.Row() + 2);
    CPPUNIT_ASSERT_EQUAL(pData->maEnd.Row(), aDataEnd.Row() + 2);

    CPPUNIT_ASSERT_EQUAL(pNData->maStart.Row(), aNDataStart.Row() + 2);
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd.Row(), aNDataEnd.Row() + 2);

    // Save the anchor data
    aDataStart = pData->maStart;
    aDataEnd = pData->maEnd;
    aNDataStart = pNData->maStart;
    aNDataEnd = pNData->maEnd;

    // Save the document and load again to check anchor persist
    saveAndReload("calc8");

    // There are two cell-anchored objects on the first sheet.
    pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    pDrawLayer = pDoc->GetDrawLayer();
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

    // Check cell anchor state
    oldType = ScDrawLayer::GetAnchorType(*pObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to get anchor type", SCA_CELL_RESIZE, oldType);

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pData->getShapeRect().IsEmpty());

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pNData->getShapeRect().IsEmpty());

    // Check if data after save it
    CPPUNIT_ASSERT_EQUAL(pData->maStart, aDataStart);
    CPPUNIT_ASSERT_EQUAL(pData->maEnd, aDataEnd);

    CPPUNIT_ASSERT_EQUAL(pNData->maStart, aNDataStart);
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd, aNDataEnd);

    // Insert a column.
    pDoc->InsertCol(ScRange(aDataStart.Col(), 0, 0, aDataStart.Col(), pDoc->MaxRow(), 0));

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pData->getShapeRect().IsEmpty());

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pNData->getShapeRect().IsEmpty());

    // Check if data has moved to new rows
    CPPUNIT_ASSERT_EQUAL(pData->maStart.Col(), SCCOL(aDataStart.Col() + 1));
    CPPUNIT_ASSERT_EQUAL(pData->maEnd.Col(), SCCOL(aDataEnd.Col() + 1));

    CPPUNIT_ASSERT_EQUAL(pNData->maStart.Col(), SCCOL(aNDataStart.Col() + 1));
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd.Col(), SCCOL(aNDataEnd.Col() + 1));

    // Save the anchor data
    aDataStart = pData->maStart;
    aDataEnd = pData->maEnd;
    aNDataStart = pNData->maStart;
    aNDataEnd = pNData->maEnd;

    // Save the document and load again to check anchor persist
    saveAndReload("calc8");

    // There are two cell-anchored objects on the first sheet.
    pDoc = getScDoc();

    CPPUNIT_ASSERT_MESSAGE("There should be at least one sheet.", pDoc->GetTableCount() > 0);

    pDrawLayer = pDoc->GetDrawLayer();
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("draw page for sheet 1 should exist.", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Failed to get drawing object.", pObj);

    // Check cell anchor state
    oldType = ScDrawLayer::GetAnchorType(*pObj);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failed to get anchor type", SCA_CELL_RESIZE, oldType);

    // Get anchor data
    pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve user data for this object.", pData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pData->getShapeRect().IsEmpty());

    // Get non rotated anchor data
    pNData = ScDrawLayer::GetNonRotatedObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("Failed to retrieve non rotated user data for this object.", pNData);
    CPPUNIT_ASSERT_MESSAGE("Bounding rectangle should have been calculated upon import.",
                           !pNData->getShapeRect().IsEmpty());

    // Check if data after save it
    CPPUNIT_ASSERT_EQUAL(pData->maStart, aDataStart);
    CPPUNIT_ASSERT_EQUAL(pData->maEnd, aDataEnd);

    CPPUNIT_ASSERT_EQUAL(pNData->maStart, aNDataStart);
    CPPUNIT_ASSERT_EQUAL(pNData->maEnd, aNDataEnd);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testConditionalFormatRangeListXLSX)
{
    createScDoc("ods/conditionalformat_rangelist.ods");
    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPath(pDoc, "//x:conditionalFormatting"_ostr, "sqref"_ostr, "F4 F10");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testConditionalFormatContainsTextXLSX)
{
    createScDoc("ods/conditionalformat_containstext.ods");
    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    assertXPathContent(pDoc, "//x:conditionalFormatting/x:cfRule/x:formula"_ostr,
                       "NOT(ISERROR(SEARCH(\"test\",A1)))");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testConditionalFormatPriorityCheckXLSX)
{
    createScDoc("xlsx/conditional_fmt_checkpriority.xlsx");
    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    constexpr bool bHighPriorityExtensionA1
        = true; // Should A1's extension cfRule has higher priority than normal cfRule ?
    constexpr bool bHighPriorityExtensionA3
        = false; // Should A3's extension cfRule has higher priority than normal cfRule ?
    size_t nA1NormalPriority = 0;
    size_t nA1ExtPriority = 0;
    size_t nA3NormalPriority = 0;
    size_t nA3ExtPriority = 0;
    for (size_t nIdx = 1; nIdx <= 2; ++nIdx)
    {
        OString aIdx = OString::number(nIdx);
        OUString aCellAddr
            = getXPath(pDoc, "//x:conditionalFormatting[" + aIdx + "]", "sqref"_ostr);
        OUString aPriority
            = getXPath(pDoc, "//x:conditionalFormatting[" + aIdx + "]/x:cfRule", "priority"_ostr);
        CPPUNIT_ASSERT_MESSAGE("conditionalFormatting sqref must be either A1 or A3",
                               aCellAddr == "A1" || aCellAddr == "A3");
        if (aCellAddr == "A1")
            nA1NormalPriority = aPriority.toUInt32();
        else
            nA3NormalPriority = aPriority.toUInt32();
        aCellAddr = getXPathContent(
            pDoc, "//x:extLst/x:ext[1]/x14:conditionalFormattings/x14:conditionalFormatting[" + aIdx
                      + "]/xm:sqref");
        aPriority
            = getXPath(pDoc,
                       "//x:extLst/x:ext[1]/x14:conditionalFormattings/x14:conditionalFormatting["
                           + aIdx + "]/x14:cfRule",
                       "priority"_ostr);
        CPPUNIT_ASSERT_MESSAGE("x14:conditionalFormatting sqref must be either A1 or A3",
                               aCellAddr == "A1" || aCellAddr == "A3");
        if (aCellAddr == "A1")
            nA1ExtPriority = aPriority.toUInt32();
        else
            nA3ExtPriority = aPriority.toUInt32();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong priorities for A1", bHighPriorityExtensionA1,
                                 nA1ExtPriority < nA1NormalPriority);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong priorities for A3", bHighPriorityExtensionA3,
                                 nA3ExtPriority < nA3NormalPriority);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testConditionalFormatOriginXLSX)
{
    createScDoc("xlsx/conditional_fmt_origin.xlsx");
    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);
    // tdf#124953 : The range-list is B3:C6 F1:G2, origin address in the formula should be B1, not B3.
    OUString aFormula = getXPathContent(pDoc, "//x:conditionalFormatting/x:cfRule/x:formula"_ostr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong origin address in formula",
                                 OUString("NOT(ISERROR(SEARCH(\"BAC\",B1)))"), aFormula);
}

// FILESAVE: XLSX export with long sheet names (length > 31 characters)
CPPUNIT_TEST_FIXTURE(ScExportTest3, testTdf79998)
{
    // check: original document has tab name > 31 characters
    createScDoc("ods/tdf79998.ods");
    ScDocument* pDoc = getScDoc();
    const std::vector<OUString> aTabNames1 = pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_EQUAL(OUString("Utilities (FX Kurse, Kreditkarten etc)"), aTabNames1[1]);

    // check: saved XLSX document has truncated tab name
    saveAndReload("Calc Office Open XML");
    pDoc = getScDoc();
    const std::vector<OUString> aTabNames2 = pDoc->GetAllTableNames();
    CPPUNIT_ASSERT_EQUAL(OUString("Utilities (FX Kurse, Kreditkart"), aTabNames2[1]);
}

static void impl_testLegacyCellAnchoredRotatedShape(ScDocument& rDoc, const tools::Rectangle& aRect,
                                                    const ScDrawObjData& aAnchor,
                                                    tools::Long TOLERANCE = 30 /* 30 hmm */)
{
    ScDrawLayer* pDrawLayer = rDoc.GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("No drawing layer.", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("No page instance for the 1st sheet.", pPage);
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pPage->GetObjCount());

    SdrObject* pObj = pPage->GetObj(0);
    const tools::Rectangle& aSnap = pObj->GetSnapRect();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aRect.GetHeight(), aSnap.GetHeight(), TOLERANCE);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aRect.GetWidth(), aSnap.GetWidth(), TOLERANCE);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aRect.Left(), aSnap.Left(), TOLERANCE);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(aRect.Top(), aSnap.Top(), TOLERANCE);

    ScDrawObjData* pData = ScDrawLayer::GetObjData(pObj);
    CPPUNIT_ASSERT_MESSAGE("expected object meta data", pData);
    CPPUNIT_ASSERT_EQUAL(aAnchor.maStart.Row(), pData->maStart.Row());
    CPPUNIT_ASSERT_EQUAL(aAnchor.maStart.Col(), pData->maStart.Col());
    CPPUNIT_ASSERT_EQUAL(aAnchor.maEnd.Row(), pData->maEnd.Row());
    CPPUNIT_ASSERT_EQUAL(aAnchor.maEnd.Col(), pData->maEnd.Col());
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testLegacyCellAnchoredRotatedShape)
{
    {
        // This example doc contains cell anchored shape that is rotated, the
        // rotated shape is in fact clipped by the sheet boundaries (and thus
        // is a good edge case test to see if we import it still correctly)
        createScDoc("ods/legacycellanchoredrotatedclippedshape.ods");

        ScDocument* pDoc = getScDoc();
        // ensure the imported legacy rotated shape is in the expected position
        tools::Rectangle aRect(6000, -2000, 8000, 4000);
        // ensure the imported ( and converted ) anchor ( note we internally now store the anchor in
        // terms of the rotated shape ) is more or less contains the correct info
        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow(0);
        aAnchor.maStart.SetCol(5);
        aAnchor.maEnd.SetRow(3);
        aAnchor.maEnd.SetCol(7);
        impl_testLegacyCellAnchoredRotatedShape(*pDoc, aRect, aAnchor);
        // test save and reload
        // for some reason having this test in subsequent_export-test.cxx causes
        // a core dump in editeng ( so moved to here )
        saveAndReload("calc8");
        pDoc = getScDoc();
        impl_testLegacyCellAnchoredRotatedShape(*pDoc, aRect, aAnchor);
    }
    {
        // This example doc contains cell anchored shape that is rotated, the
        // rotated shape is in fact clipped by the sheet boundaries, additionally
        // the shape is completely hidden because the rows the shape occupies
        // are hidden
        createScDoc("ods/legacycellanchoredrotatedhiddenshape.ods");
        ScDocument* pDoc = getScDoc();
        // ensure the imported legacy rotated shape is in the expected position
        tools::Rectangle aRect(6000, -2000, 8000, 4000);

        // ensure the imported (and converted) anchor (note we internally now store the anchor in
        // terms of the rotated shape) is more or less contains the correct info
        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow(0);
        aAnchor.maStart.SetCol(5);
        aAnchor.maEnd.SetRow(3);
        aAnchor.maEnd.SetCol(7);
        pDoc->ShowRows(0, 9, 0, true); // show relevant rows
        pDoc->SetDrawPageSize(0); // trigger recalcpos
        impl_testLegacyCellAnchoredRotatedShape(*pDoc, aRect, aAnchor);
        // test save and reload
        saveAndReload("calc8");
        pDoc = getScDoc();
        impl_testLegacyCellAnchoredRotatedShape(*pDoc, aRect, aAnchor);
    }
    {
        // This example doc contains cell anchored shape that is rotated
        createScDoc("ods/legacycellanchoredrotatedshape.ods");

        ScDocument* pDoc = getScDoc();
        // ensure the imported legacy rotated shape is in the expected position
        tools::Rectangle aRect(6000, 3000, 8000, 9000);
        // ensure the imported (and converted) anchor (note we internally now store the anchor in
        // terms of the rotated shape) more or less contains the correct info

        ScDrawObjData aAnchor;
        aAnchor.maStart.SetRow(3);
        aAnchor.maStart.SetCol(6);
        aAnchor.maEnd.SetRow(9);
        aAnchor.maEnd.SetCol(8);
        // test import
        impl_testLegacyCellAnchoredRotatedShape(*pDoc, aRect, aAnchor);
        // test save and reload
        saveAndReload("calc8");
        pDoc = getScDoc();
        impl_testLegacyCellAnchoredRotatedShape(*pDoc, aRect, aAnchor);
    }
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTdf113646)
{
    createScDoc("ods/tdf113646.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pSheet = parseExport("xl/styles.xml");
    CPPUNIT_ASSERT(pSheet);

    assertXPath(pSheet, "/x:styleSheet/x:dxfs/x:dxf/x:font/x:sz"_ostr, "val"_ostr, "36");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testDateStandardfilterXLSX)
{
    // XLSX Roundtripping standard filter with date
    createScDoc("ods/tdf142607.ods");

    save("Calc Office Open XML");
    xmlDocUniquePtr pDoc = parseExport("xl/worksheets/sheet1.xml");
    CPPUNIT_ASSERT(pDoc);

    assertXPath(pDoc, "//x:autoFilter"_ostr, "ref"_ostr, "A1:B6");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]"_ostr, "day"_ostr,
                "03");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]"_ostr,
                "month"_ostr, "12");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]"_ostr,
                "year"_ostr, "2011");
    assertXPath(pDoc, "//x:autoFilter/x:filterColumn/x:filters/x:dateGroupItem[1]"_ostr,
                "dateTimeGrouping"_ostr, "day");
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testNumberFormatODS)
{
    createScDoc("ods/testNumberFormats.ods");
    saveAndReload("calc8");
    ScDocument* pDoc = getScDoc();
    sal_uInt32 nNumberFormat;
    const sal_Int32 nCountFormats = 18;
    const OUString aExpectedFormatStr[nCountFormats]
        = { "\"format=\"000000",        "\"format=\"??????",        "\"format=\"??0000",
            "\"format=\"000,000",       "\"format=\"???,???",       "\"format=\"??0,000",
            "\"format=\"000\" \"?/?",   "\"format=\"???\" \"?/?",   "\"format=\"?00\" \"?/?",
            "\"format=\"0,000\" \"?/?", "\"format=\"?,???\" \"?/?", "\"format=\"?,?00\" \"?/?",
            "\"format=\"0.000E+00",     "\"format=\"?.###E+00",     "\"format=\"?.0##E+00",
            "\"format=\"000E+00",       "\"format=\"???E+00",       "\"format=\"?00E+00" };
    for (sal_Int32 i = 0; i < nCountFormats; i++)
    {
        nNumberFormat = pDoc->GetNumberFormat(i + 1, 2, 0);
        const SvNumberformat* pNumberFormat = pDoc->GetFormatTable()->GetEntry(nNumberFormat);
        const OUString& rFormatStr = pNumberFormat->GetFormatstring();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Number format modified during export/import",
                                     aExpectedFormatStr[i], rFormatStr);
    }
    OUString aCSVPath = createFilePath(u"contentCSV/testNumberFormats.csv");
    testCondFile(aCSVPath, &*pDoc, 0,
                 false); // comma is thousand separator and cannot be used as delimiter
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTdf137576_Measureline)
{
    // The document contains a vertical measure line, anchored "To Cell (resize with cell)" with
    // length 37mm. Save and reload had resulted in a line of 0mm length.

    // Get document
    createScDoc("ods/tdf137576_Measureline.ods");
    ScDocument* pDoc = getScDoc();

    // Get shape
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrMeasureObj* pObj = static_cast<SdrMeasureObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("Load: No measure object", pObj);

    // Check start and end point of measureline
    const Point aStart = pObj->GetPoint(0);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(4800, 1500), aStart, 1);
    const Point aEnd = pObj->GetPoint(1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(4800, 5200), aEnd, 1);

    // Save and reload
    saveAndReload("calc8");
    pDoc = getScDoc();

    // Get shape
    pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = static_cast<SdrMeasureObj*>(pPage->GetObj(0));
    CPPUNIT_ASSERT_MESSAGE("Reload: No measure object", pObj);

    // Check start and end point of measureline, should be unchanged
    const Point aStart2 = pObj->GetPoint(0);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(4800, 1500), aStart2, 1);
    const Point aEnd2 = pObj->GetPoint(1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(4800, 5200), aEnd2, 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTdf137044_CoverHiddenRows)
{
    // The document contains a shape anchored "To Cell (resize with cell)" with start in cell A4 and
    // end in cell A7. Row height is 30mm. Hiding rows 5 and 6, then saving and reload had resulted
    // in a wrong end cell offset and thus a wrong height of the shape.

    // Get document
    createScDoc("ods/tdf137044_CoverHiddenRows.ods");
    ScDocument* pDoc = getScDoc();

    // Get shape
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No object", pObj);

    // Get original object values
    tools::Rectangle aSnapRectOrig = pObj->GetSnapRect();
    Point aOriginalEndOffset = ScDrawLayer::GetObjData(pObj)->maEndOffset;
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(
        tools::Rectangle(Point(500, 3500), Size(1501, 11001)), aSnapRectOrig, 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(2000, 2499), aOriginalEndOffset, 1);

    // Hide rows 5 and 6 in UI = row index 4 to 5.
    pDoc->SetRowHidden(4, 5, 0, true);

    // Save and reload
    saveAndReload("calc8");
    pDoc = getScDoc();

    // Get shape
    pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No object", pObj);

    // Get new values and compare. End offset should be the same, height should be 6000 smaller.
    tools::Rectangle aSnapRectReload = pObj->GetSnapRect();
    Point aReloadEndOffset = ScDrawLayer::GetObjData(pObj)->maEndOffset;
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(
        tools::Rectangle(Point(500, 3500), Size(1501, 5001)), aSnapRectReload, 1);
    CPPUNIT_ASSERT_POINT_EQUAL_WITH_TOLERANCE(Point(2000, 2499), aReloadEndOffset, 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTdf137020_FlipVertical)
{
    // Get document
    createScDoc("ods/tdf137020_FlipVertical.ods");
    ScDocument* pDoc = getScDoc();

    // Get shape
    ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Load: No ScDrawLayer", pDrawLayer);
    SdrPage* pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No draw page", pPage);
    SdrObject* pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Load: No object", pObj);

    const tools::Rectangle aSnapRectOrig = pObj->GetSnapRect();

    // Vertical mirror on center should not change the snap rect.
    pObj->Mirror(aSnapRectOrig.LeftCenter(), aSnapRectOrig.RightCenter());
    const tools::Rectangle aSnapRectFlip = pObj->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectOrig, aSnapRectFlip, 1);

    // Save and reload
    saveAndReload("calc8");
    pDoc = getScDoc();

    // Get shape
    pDrawLayer = pDoc->GetDrawLayer();
    CPPUNIT_ASSERT_MESSAGE("Reload: No ScDrawLayer", pDrawLayer);
    pPage = pDrawLayer->GetPage(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No draw page", pPage);
    pObj = pPage->GetObj(0);
    CPPUNIT_ASSERT_MESSAGE("Reload: No object", pObj);

    // Check pos and size of shape again, should be unchanged
    const tools::Rectangle aSnapRectReload = pObj->GetSnapRect();
    CPPUNIT_ASSERT_RECTANGLE_EQUAL_WITH_TOLERANCE(aSnapRectOrig, aSnapRectReload, 1);
}

CPPUNIT_TEST_FIXTURE(ScExportTest3, testTdf82254_csv_bom)
{
    setImportFilterName(SC_TEXT_CSV_FILTER_NAME);
    createScDoc("csv/testTdf82254-csv-bom.csv");
    saveAndReload(SC_TEXT_CSV_FILTER_NAME);
    ScDocShell* pDocSh = getScDocShell();
    SvStream* pStream = pDocSh->GetMedium()->GetInStream();

    pStream->Seek(0);
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(0), pStream->Tell());
    pStream->StartReadingUnicodeText(RTL_TEXTENCODING_UTF8);
    // Without the fix in place, this test would have failed with
    // - Expected: 3
    // - Actual  : 0 (no byte order mark was read)
    CPPUNIT_ASSERT_EQUAL(sal_uInt64(3), pStream->Tell());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
