/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

/**
 * implements the {@link DocumentTest} interface on top of a spreadsheet document
 */
class CalcDocumentTest : public DocumentTestBase
{
public:
    CalcDocumentTest(const Reference<XComponent>& rxDoc)
        : DocumentTestBase(rxDoc)
    {
    }

    virtual OUString getDocumentDescription() override { return u"spreadsheet document"_ustr; }

    virtual void initializeDocument() override
    {
        Reference<XCell> cellA1 = getCellA1();
        cellA1->setValue(INIT_VALUE);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("initializing the cell value didn't work", INIT_VALUE,
                                     cellA1->getValue());

        Reference<XCellRange> range(getSheet(0));

        for (int i = 0; i < 12; ++i)
        {
            Reference<XCell> cell = range->getCellByPosition(1, i);
            cell->setFormula(u""_ustr);
        }
    }

    virtual void doSingleModification() override
    {
        Reference<XCell> cellA1 = getCellA1();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("initial cell value not as expected", INIT_VALUE,
                                     cellA1->getValue());
        cellA1->setValue(MODIFIED_VALUE);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("modified cell value not as expected", MODIFIED_VALUE,
                                     cellA1->getValue());
    }

    virtual void verifyInitialDocumentState() override
    {
        Reference<XCell> cellA1 = getCellA1();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cell A1 doesn't have its initial value", INIT_VALUE,
                                     cellA1->getValue());

        Reference<XCellRange> range = getSheet(0);
        for (int i = 0; i < 12; ++i)
        {
            Reference<XCell> cell = range->getCellByPosition(1, i);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Cell B" + std::to_string(i + 1)
                                             + " not having its initial value (an empty string)",
                                         u""_ustr, cell->getFormula());
        }
    }

    virtual void verifySingleModificationDocumentState() override
    {
        Reference<XCell> cellA1 = getCellA1();
        CPPUNIT_ASSERT_EQUAL_MESSAGE("cell A1 doesn't have the value which we gave it",
                                     MODIFIED_VALUE, cellA1->getValue());
    }

    virtual int doMultipleModifications() override
    {
        Reference<XCellRange> range(getSheet(0));

        OUString months[]{
            u"January"_ustr,   u"February"_ustr, u"March"_ustr,    u"April"_ustr,
            u"May"_ustr,       u"June"_ustr,     u"July"_ustr,     u"August"_ustr,
            u"September"_ustr, u"October"_ustr,  u"November"_ustr, u"December"_ustr
        };
        for (int i = 0; i < 12; ++i)
        {
            Reference<XCell> cell = range->getCellByPosition(1, i);
            cell->setFormula(months[i]);
        }
        return 12;
    }

    Reference<XCellRange> getSheet(int index)
    {
        Reference<XIndexAccess> sheets(getSheets(), UNO_QUERY);
        return Reference<XCellRange>(sheets->getByIndex(index), UNO_QUERY);
    }
    Reference<XSpreadsheets> getSheets()
    {
        Reference<XSpreadsheetDocument> spreadsheetDoc(m_document, UNO_QUERY);
        return spreadsheetDoc->getSheets();
    }

private:
    Reference<XCell> getCellA1()
    {
        Reference<XCellRange> range(getSheet(0));
        return range->getCellByPosition(0, 0);
    }

    static constexpr double INIT_VALUE = 100.0;
    static constexpr double MODIFIED_VALUE = 200.0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
