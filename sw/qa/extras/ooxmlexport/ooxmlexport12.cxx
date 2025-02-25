/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextTablesSupplier.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>
#include <o3tl/string_view.hxx>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase(u"/sw/qa/extras/ooxmlexport/data/"_ustr, u"Office Open XML Text"_ustr)
    {
    }
};

CPPUNIT_TEST_FIXTURE(Test, testTableCrossReference)
{
    loadAndReload("table_cross_reference.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#42346: Cross references to tables were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_number_only"_ustr));

    // Check bookmark text ranges
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table 1: Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table 1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }

    // Check reference fields
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        switch (nIndex)
        {
            // Full reference to table caption
            case 0:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Table 1: Table caption"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_full"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Page style reference / exported as simple page reference
            case 1:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_full"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to table number
            case 2:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_number_only"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to caption only
            case 3:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_caption_only"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to category and number
            case 4:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Table 1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_label_and_number"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to page of the table
            case 5:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_full"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Above / below reference
            case 6:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"above"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Table0_full"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SequenceNumber"_ustr) >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            default:
                break;
        }
        ++nIndex;
    }

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), nIndex);
}

CPPUNIT_TEST_FIXTURE(Test, testTableCrossReferenceCustomFormat)
{
    loadAndReload("table_cross_reference_custom_format.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#42346: Cross references to tables were not saved
    // Check also captions with custom formatting

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(16), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table0_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table1_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table1_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table1_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table1_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table2_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table2_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table2_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table2_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table3_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table3_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table3_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Table3_number_only"_ustr));

    // Check bookmark text ranges
    // First table's caption
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1. Table: Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1. Table"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }
    // Second table's caption
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table1_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"2. TableTable caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table1_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"2. Table"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table1_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table1_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"2"_ustr, xRange->getString());
    }
    // Third table's caption
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table2_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"3) Table Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table2_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"3) Table"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table2_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table2_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"3"_ustr, xRange->getString());
    }
    // Fourth table's caption
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table3_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table 4- Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table3_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table 4"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table3_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Table caption"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Table3_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"4"_ustr, xRange->getString());
    }
}

CPPUNIT_TEST_FIXTURE(Test, testObjectCrossReference)
{
    loadAndReload("object_cross_reference.odt");
    CPPUNIT_ASSERT_EQUAL(10, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // tdf#42346: Cross references to objects were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(15), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing0_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Drawing1_full"_ustr));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration0_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Illustration1_caption_only"_ustr));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text0_full"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text0_label_and_number"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text0_caption_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text0_number_only"_ustr));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName(u"Ref_Text1_label_and_number"_ustr));

    // Check bookmark text ranges
    // Cross references to shapes
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Drawing0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Drawing 1: A rectangle"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Drawing0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Drawing 1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Drawing0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"A rectangle"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Drawing0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Drawing1_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Drawing 2: a circle"_ustr, xRange->getString());
    }

    // Cross references to pictures
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Illustration0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Illustration 1: A picture"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Illustration0_label_and_number"_ustr),
            uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Illustration 1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Illustration0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"A picture"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Illustration0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Illustration1_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"another image"_ustr, xRange->getString());
    }

    // Cross references to text frames
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Text0_full"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Text 1: A frame"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Text0_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Text 1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Text0_caption_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"A frame"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Text0_number_only"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"1"_ustr, xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName(u"Ref_Text1_label_and_number"_ustr), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(u"Text 2"_ustr, xRange->getString());
    }

    // Check reference fields
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());

    sal_uInt16 nIndex = 0;
    while (xFields->hasMoreElements())
    {
        uno::Reference<lang::XServiceInfo> xServiceInfo(xFields->nextElement(), uno::UNO_QUERY);
        uno::Reference<beans::XPropertySet> xPropertySet(xServiceInfo, uno::UNO_QUERY);
        switch (nIndex)
        {
            // Reference to image number
            case 0:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Illustration0_number_only"_ustr, sValue);
                break;
            }
            // Full reference to the circle shape
            case 1:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Drawing 2: a circle"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Drawing1_full"_ustr, sValue);
                break;
            }
            // Caption only reference to the second picture
            case 2:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"another image"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Illustration1_caption_only"_ustr, sValue);
                break;
            }
            // Category and number reference to second text frame
            case 3:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Text 2"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Text1_label_and_number"_ustr, sValue);
                break;
            }
            // Full reference to rectangle shape
            case 4:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Drawing 1: A rectangle"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Drawing0_full"_ustr, sValue);
                break;
            }
            // Caption only reference to rectangle shape
            case 5:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"A rectangle"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Drawing0_caption_only"_ustr, sValue);
                break;
            }
            // Category and number reference to rectangle shape
            case 6:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Drawing 1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Drawing0_label_and_number"_ustr, sValue);
                break;
            }
            // Reference to rectangle shape's number
            case 7:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Drawing0_number_only"_ustr, sValue);
                break;
            }
            // Full reference to first text frame
            case 8:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Text 1: A frame"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Text0_full"_ustr, sValue);
                break;
            }
            // Caption only reference to first text frame
            case 9:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"A frame"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Text0_caption_only"_ustr, sValue);
                break;
            }
            // Category and number reference to first text frame
            case 10:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Text 1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Text0_label_and_number"_ustr, sValue);
                break;
            }
            // Number only reference to first text frame
            case 11:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Text0_number_only"_ustr, sValue);
                break;
            }
            // Full reference to first picture
            case 12:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Illustration 1: A picture"_ustr,
                                     sValue.trim()); // fails on MAC without trim
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Illustration0_full"_ustr, sValue);
                break;
            }
            // Reference to first picture' caption
            case 13:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"A picture"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Illustration0_caption_only"_ustr, sValue);
                break;
            }
            // Category and number reference to first picture
            case 14:
            {
                CPPUNIT_ASSERT(xServiceInfo->supportsService(
                    u"com.sun.star.text.TextField.GetReference"_ustr));
                OUString sValue;
                xPropertySet->getPropertyValue(u"CurrentPresentation"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Illustration 1"_ustr, sValue);
                xPropertySet->getPropertyValue(u"SourceName"_ustr) >>= sValue;
                CPPUNIT_ASSERT_EQUAL(u"Ref_Illustration0_label_and_number"_ustr, sValue);
                break;
            }
            default:
                break;
        }
        ++nIndex;
    }

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(21), nIndex);
}

CPPUNIT_TEST_FIXTURE(Test, testTd112202)
{
    auto verify = [this](bool bIsExport = false) {
        xmlDocUniquePtr pXmlDoc = parseLayoutDump();

        // page 1 header: 1 paragraph, 2 flys, 1 draw object
        assertXPath(pXmlDoc, "/root/page[1]/header/txt", 1);
        assertXPath(pXmlDoc, "/root/page[1]/header/txt/anchored/fly", 2);
        if (bIsExport) // somehow there's an additional shape on re-import?
            assertXPath(pXmlDoc, "/root/page[1]/header/txt/anchored/SwAnchoredDrawObject", 2);
        else
            assertXPath(pXmlDoc, "/root/page[1]/header/txt/anchored/SwAnchoredDrawObject", 1);

        // page 2 header: 3 paragraphs, 1 table, 1 fly on last paragraph
        assertXPath(pXmlDoc, "/root/page[2]/header/txt", 3);
        assertXPath(pXmlDoc, "/root/page[2]/header/tab", 1);
        assertXPath(pXmlDoc, "/root/page[2]/header/txt/anchored/fly", 1);

        // page 3 header: 1 table, 1 paragraph, no text
        assertXPath(pXmlDoc, "/root/page[3]/header/txt", 1);
        assertXPath(pXmlDoc, "/root/page[3]/header/tab", 1);
        assertXPath(pXmlDoc,
                    "/root/page[3]/header/tab/row/cell/txt/SwParaPortion/SwLineLayout/child::*", 0);
        assertXPath(pXmlDoc, "/root/page[3]/header//anchored", 0);
        // tdf#149313: ensure 3rd page does not have extra empty paragraph at top
        assertXPathContent(pXmlDoc, "/root/page[3]/body//txt", u"AUFGABENSTELLUNG");

        // page 4 header: 1 table, 1 paragraph, with text
        assertXPath(pXmlDoc, "/root/page[4]/header/txt", 1);
        assertXPath(pXmlDoc, "/root/page[4]/header/tab", 1);
        assertXPath(pXmlDoc,
                    "/root/page[4]/header/tab/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion",
                    "portion", u"Titel der studentischen Arbeit");
        assertXPath(pXmlDoc, "/root/page[4]/header//anchored", 0);

        // page 5: same as page 4
        assertXPath(pXmlDoc, "/root/page[5]/header/txt", 1);
        assertXPath(pXmlDoc, "/root/page[5]/header/tab", 1);
        assertXPath(pXmlDoc,
                    "/root/page[5]/header/tab/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion",
                    "portion", u"Titel der studentischen Arbeit");
        assertXPath(pXmlDoc, "/root/page[5]/header//anchored", 0);

        // page 6: same as page 4
        assertXPath(pXmlDoc, "/root/page[6]/header/txt", 1);
        assertXPath(pXmlDoc, "/root/page[6]/header/tab", 1);
        assertXPath(pXmlDoc,
                    "/root/page[6]/header/tab/row[1]/cell[1]/txt[1]/SwParaPortion/SwLineLayout/"
                    "SwParaPortion",
                    "portion", u"Titel der studentischen Arbeit");
        assertXPath(pXmlDoc, "/root/page[6]/header//anchored", 0);
    };

    createSwDoc("090716_Studentische_Arbeit_VWS.docx");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf79435_legacyInputFields)
{
    loadAndReload("tdf79435_legacyInputFields.doc");
    //using .doc input file to verify cross-format compatibility.
    uno::Reference<text::XFormField> xFormField = getProperty<uno::Reference<text::XFormField>>(
        getRun(getParagraph(5), 3), u"Bookmark"_ustr);
    uno::Reference<container::XNameContainer> xParameters(xFormField->getParameters());

    OUString sTmp;
    // Doc import problems, so disabling tests
    //xParameters->getByName("EntryMacro") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("test"), sTmp);
    //xParameters->getByName("Help") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("F1Help"), sTmp);
    //xParameters->getByName("ExitMacro") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("test"), sTmp);
    xParameters->getByName(u"Hint"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"StatusHelp"_ustr, sTmp);
    //xParameters->getByName("Content") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("Camelcase"), sTmp);
    //xParameters->getByName("Format") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("TITLE CASE"), sTmp);

    sal_uInt16 nMaxLength = 0;
    xParameters->getByName(u"MaxLength"_ustr) >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Max Length", sal_uInt16(10), nMaxLength);

    // too bad this is based on character runs - just found try trial and error.
    xFormField = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(6), 2),
                                                               u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"calculated"_ustr, sTmp);

    xFormField = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(7), 2),
                                                               u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"currentDate"_ustr, sTmp);

    xFormField = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(7), 7),
                                                               u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"currentTime"_ustr, sTmp);

    xFormField = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(8), 2),
                                                               u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"number"_ustr, sTmp);

    xFormField = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(8), 7),
                                                               u"Bookmark"_ustr);
    xParameters.set(xFormField->getParameters());
    xParameters->getByName(u"Type"_ustr) >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(u"date"_ustr, sTmp);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/settings.xml"_ustr);
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "name", u"compatibilityMode");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "uri",
                u"http://schemas.microsoft.com/office/word");
    assertXPath(pXmlDoc, "/w:settings/w:compat/w:compatSetting[1]", "val", u"11");
}

DECLARE_OOXMLEXPORT_TEST(testTdf120224_textControlCrossRef, "tdf120224_textControlCrossRef.docx")
{
    uno::Reference<text::XTextDocument> xTextDocument(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xParaEnumAccess(xTextDocument->getText(),
                                                                  uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xParaEnum = xParaEnumAccess->createEnumeration();
    uno::Reference<container::XEnumerationAccess> xRunEnumAccess(xParaEnum->nextElement(),
                                                                 uno::UNO_QUERY);
    uno::Reference<container::XEnumeration> xRunEnum = xRunEnumAccess->createEnumeration();
    xRunEnum->nextElement(); //Text
    uno::Reference<beans::XPropertySet> xPropertySet(xRunEnum->nextElement(), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(u"Bookmark"_ustr,
                         getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));

    xPropertySet.set(xRunEnum->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"TextFieldStart"_ustr,
                         getProperty<OUString>(xPropertySet, u"TextPortionType"_ustr));
    uno::Reference<container::XNamed> xBookmark(
        getProperty<uno::Reference<beans::XPropertySet>>(xPropertySet, u"Bookmark"_ustr),
        uno::UNO_QUERY);

    // Critical test: does TextField's bookmark name match cross-reference?
    const OUString sTextFieldName(xBookmark->getName());
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sTextFieldName, getProperty<OUString>(xPropertySet, u"SourceName"_ustr));

    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    // TextFields should not be turned into real bookmarks.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(2), xBookmarksByIdx->getCount());

    // The actual name isn't critical, but if it fails, it is worth asking why.
    CPPUNIT_ASSERT_EQUAL(u"Text1"_ustr, sTextFieldName);
}

DECLARE_OOXMLEXPORT_TEST(testTdf117504_numberingIndent, "tdf117504_numberingIndent.docx")
{
    OUString sName = getProperty<OUString>(getParagraph(1), u"NumberingStyleName"_ustr);
    CPPUNIT_ASSERT_MESSAGE("Paragraph has numbering style", !sName.isEmpty());

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"Revision"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353),
                         getProperty<sal_Int32>(xPropertySet, u"ParaBottomMargin"_ustr));
    xPropertySet.set(getStyles(u"ParagraphStyles"_ustr)->getByName(u"Body Note"_ustr),
                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                         getProperty<sal_Int32>(xPropertySet, u"ParaBottomMargin"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testWatermark, "watermark.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);

    sal_Int32 nHeight = xShape->getSize().Height;

    // Rounding errors
    sal_Int32 nDifference = 5150 - nHeight;
    std::stringstream ss;
    ss << "Difference: " << nDifference << " TotalHeight: " << nHeight;
    CPPUNIT_ASSERT_MESSAGE(ss.str(), nDifference <= 4);
    CPPUNIT_ASSERT_MESSAGE(ss.str(), nDifference >= -4);
}

DECLARE_OOXMLEXPORT_TEST(testWatermarkTrim, "tdf114308.docx")
{
    uno::Reference<drawing::XShape> xShape = getShape(1);

    // Rounding errors
    sal_Int32 nHeight = xShape->getSize().Height;
    sal_Int32 nDifference = 8729 - nHeight;
    std::stringstream ss;
    ss << "Difference: " << nDifference << " TotalHeight: " << nHeight;
    CPPUNIT_ASSERT_MESSAGE(ss.str(), nDifference <= 4);
    CPPUNIT_ASSERT_MESSAGE(ss.str(), nDifference >= -4);
}

CPPUNIT_TEST_FIXTURE(Test, testVMLShapetypeId)
{
    loadAndSave("controlshape.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // must be _x0000_t<NR>
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:r/mc:AlternateContent/"
                "mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wps:wsp/wps:txbx/"
                "w:txbxContent/w:p/w:r/w:object/v:shapetype",
                "id", u"_x0000_t75");
    assertXPath(pXmlDoc,
                "/w:document/w:body/w:tbl[1]/w:tr[1]/w:tc[1]/w:p[1]/w:r/mc:AlternateContent/"
                "mc:Choice/w:drawing/wp:inline/a:graphic/a:graphicData/wps:wsp/wps:txbx/"
                "w:txbxContent/w:p/w:r/w:object/v:shape",
                "type", u"#_x0000_t75");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf73547)
{
    loadAndSave("tdf73547-dash.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    double nD = getXPath(pXmlDoc, "//a:custDash/a:ds[1]", "d").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(105000.0, nD, 5000.0); // was 100000
    double nSp = getXPath(pXmlDoc, "//a:custDash/a:ds[1]", "sp").toDouble();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(35000.0, nSp, 5000.0); // was 100000
}

DECLARE_OOXMLEXPORT_TEST(testTdf119143, "tdf119143.docx")
{
    // The runs inside <w:dir> were ignored
    const OUString sParaText = getParagraph(1)->getString();
    CPPUNIT_ASSERT_EQUAL(
        u"عندما يريد العالم أن يتكلّم ‬ ، فهو يتحدّث "
        u"بلغة "
        u"يونيكود. تسجّل الآن لحضور المؤتمر الدولي العاشر "
        u"ليونيكود (Unicode Conference)، الذي سيعقد في 10-12 "
        u"آذار 1997 بمدينة مَايِنْتْس، ألمانيا. و سيجمع المؤتمر "
        u"بين خبراء من كافة قطاعات الصناعة على الشبكة "
        u"العالمية انترنيت ويونيكود، حيث ستتم، على الصعيدين "
        u"الدولي والمحلي على حد سواء مناقشة سبل استخدام "
        u"يونكود في النظم القائمة وفيما يخص التطبيقات "
        u"الحاسوبية، الخطوط، تصميم النصوص والحوسبة متعددة "
        u"اللغات."_ustr,
        sParaText);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105444)
{
    loadAndSave("tdf105444.docx");
    xmlDocUniquePtr pXmlComm = parseExport(u"word/comments.xml"_ustr);
    // there is no extra paragraph on Win32, only a single one.
    assertXPath(pXmlComm, "/w:comments/w:comment/w:p", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf117137, "tdf117137.docx")
{
    // Paragraphs were not part of a numbering anymore after roundtrip.
    uno::Reference<beans::XPropertySet> xPara1(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara1.is());
    CPPUNIT_ASSERT(xPara1->getPropertyValue(u"NumberingRules"_ustr).hasValue());

    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara2.is());
    CPPUNIT_ASSERT(xPara2->getPropertyValue(u"NumberingRules"_ustr).hasValue());

    uno::Reference<beans::XPropertySet> xPara3(getParagraph(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara3.is());
    CPPUNIT_ASSERT(xPara3->getPropertyValue(u"NumberingRules"_ustr).hasValue());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138780)
{
    loadAndReload("tdf138780.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // Paragraphs were not part of a numbering anymore after roundtrip.
    uno::Reference<beans::XPropertySet> xPara1(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara1.is());
    CPPUNIT_ASSERT(xPara1->getPropertyValue(u"NumberingRules"_ustr).hasValue());

    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara2.is());
    CPPUNIT_ASSERT(xPara2->getPropertyValue(u"NumberingRules"_ustr).hasValue());

    uno::Reference<beans::XPropertySet> xPara3(getParagraph(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara3.is());
    CPPUNIT_ASSERT(xPara3->getPropertyValue(u"NumberingRules"_ustr).hasValue());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf134618)
{
    loadAndSave("tdf134618.doc");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    //Without the fix it in place, it would have failed with
    //- Expected: 1
    //- Actual  : 9
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r", 1);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf99631)
{
    loadAndSave("tdf99631.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "//w:object", 2);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object", 2);
    // first XSLX OLE object (1:1 scale)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:object[1]", "dxaOrig", u"2561");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:object[1]", "dyaOrig", u"513");
    // second XLSX OLE object (same content + 1 row, but zoomed)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:object[1]", "dxaOrig", u"2561");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:object[1]", "dyaOrig", u"769");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf138899)
{
    loadAndSave("tdf138899.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    // This was 6, not removed empty temporary paragraph at the end of the section
    assertXPath(pXmlDocument, "/w:document/w:body/w:p", 5);

    //tdf#134385: Paragraph property to "add space between paragraphs of the same style" was lost
    assertXPath(pXmlDocument, "//w:p[1]/w:pPr/w:contextualSpacing", "val", u"false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf122563)
{
    loadAndSave("tdf122563.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object", 1);
    // Size of the embedded OLE spreadsheet was the bad "width:28.35pt;height:28.35pt"
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:object/v:shape", "style",
                u"width:255.75pt;height:63.75pt;mso-wrap-distance-right:0pt");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf94628)
{
    loadAndReload("tdf94628.docx");
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles(u"NumberingStyles"_ustr)->getByName(u"WWNum1"_ustr), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue(u"NumberingRules"_ustr), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString sBulletChar = std::find_if(std::cbegin(aProps), std::cend(aProps),
                                        [](const beans::PropertyValue& rValue) {
                                            return rValue.Name == "BulletChar";
                                        })
                               ->Value.get<OUString>();
    // Actually for 'BLACK UPPER RIGHT TRIANGLE' is \u25E5
    // But we use Wingdings 3 font here, so code is different
    CPPUNIT_ASSERT_EQUAL(u"\uF07B"_ustr, sBulletChar);
}

DECLARE_OOXMLEXPORT_TEST(testTdf122594, "tdf122594.docx")
{
    // test import/export of ActiveTable (visible sheet) of embedded XLSX OLE objects
    uno::Reference<text::XTextEmbeddedObjectsSupplier> xEmbeddedObjectsSupplier(mxComponent,
                                                                                uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xObjects(xEmbeddedObjectsSupplier->getEmbeddedObjects(),
                                                     uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xObjects->getCount());

    uno::Reference<beans::XPropertySet> xSheets;
    xObjects->getByIndex(0) >>= xSheets;

    uno::Reference<frame::XModel> xModel;
    xSheets->getPropertyValue(u"Model"_ustr) >>= xModel;
    uno::Reference<document::XViewDataSupplier> xViewDataSupplier(xModel, uno::UNO_QUERY);

    uno::Reference<container::XIndexAccess> xIndexAccess(xViewDataSupplier->getViewData());
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xIndexAccess->getCount());

    uno::Sequence<beans::PropertyValue> aSeq;
    sal_Int32 nCheck = 0;
    if (xIndexAccess->getByIndex(0) >>= aSeq)
    {
        sal_Int32 nCount(aSeq.getLength());
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            OUString sName(aSeq[i].Name);
            if (sName == "ActiveTable")
            {
                OUString sTabName;
                if (aSeq[i].Value >>= sTabName)
                {
                    // Sheet2, not Sheet1
                    CPPUNIT_ASSERT_EQUAL(u"Munka2"_ustr, sTabName);
                    nCheck++;
                }
            }
            // tdf#122624 column and row viewarea positions
            else if (sName == "PositionLeft")
            {
                sal_Int32 nPosLeft;
                aSeq[i].Value >>= nPosLeft;
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nPosLeft);
                nCheck++;
            }
            else if (sName == "PositionTop")
            {
                sal_Int32 nPosTop;
                aSeq[i].Value >>= nPosTop;
                CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), nPosTop);
                nCheck++;
            }
        }
    }

    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(3), nCheck);
}

CPPUNIT_TEST_FIXTURE(Test, testLanguageInGroupShape)
{
    loadAndSave("tdf131922_LanguageInGroupShape.docx");
    // tdf#131922: Check if good language is used in shape group texts
    xmlDocUniquePtr pXml = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXml,
                "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wpg:wgp/"
                "wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:lang",
                "val", u"de-DE");
}

DECLARE_OOXMLEXPORT_TEST(testTdf116883, "tdf116883.docx")
{
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>1>"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>2>"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>2>1>1>"_ustr,
                             getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>2>2>"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>2>3>"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>1)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(7), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>2)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(8), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>2>1<1)"_ustr,
                             getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(9), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>2.2)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(10), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(u"1>2.3)"_ustr, getProperty<OUString>(xPara, u"ListLabelString"_ustr));
    }
}

CPPUNIT_TEST_FIXTURE(Test, testTdf131420)
{
    loadAndSave("tdf131420.docx");
    xmlDocUniquePtr pXmlDocument = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:pPr/w:pBdr/w:top");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf80526_word_wrap)
{
    // tdf#80526: check whether the "wrap" property has been set
    createSwDoc("tdf80526_word_wrap.docx");
    // TODO: fix export too
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xShape, u"TextWordWrap"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf118521_marginsLR, "tdf118521_marginsLR.docx")
{
    // tdf#118521 paragraphs with direct formatting of only some of left, right, or first margins have
    // lost the other unset margins coming from paragraph style, getting a bad margin from the default style instead

    uno::Reference<beans::XPropertySet> xMyStyle(
        getStyles(u"ParagraphStyles"_ustr)->getByName(u"MyStyle"_ustr), uno::UNO_QUERY);
    // from paragraph style - this is what direct formatting should equal
    sal_Int32 nMargin = getProperty<sal_Int32>(xMyStyle, u"ParaLeftMargin"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nMargin);
    // from direct formatting
    CPPUNIT_ASSERT_EQUAL(nMargin, getProperty<sal_Int32>(getParagraph(1), u"ParaLeftMargin"_ustr));

    nMargin = getProperty<sal_Int32>(xMyStyle, u"ParaRightMargin"_ustr);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1900), nMargin);
    CPPUNIT_ASSERT_EQUAL(nMargin, getProperty<sal_Int32>(getParagraph(2), u"ParaRightMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(882),
                         getProperty<sal_Int32>(getParagraph(2), u"ParaFirstLineIndent"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf104797, "tdf104797.docx")
{
    // check moveFrom and moveTo
    CPPUNIT_ASSERT_EQUAL(u"Will this sentence be duplicated?"_ustr, getParagraph(1)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(1), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 3), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Delete"_ustr,
                         getProperty<OUString>(getRun(getParagraph(1), 3), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getRun(getParagraph(1), 3), u"IsStart"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        u"This is a filler sentence. Will this sentence be duplicated ADDED STUFF?"_ustr,
        getParagraph(2)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(2), 1)->getString());
    CPPUNIT_ASSERT_EQUAL(u"This is a filler sentence."_ustr,
                         getRun(getParagraph(2), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(2), 3)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 3), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Insert"_ustr,
                         getProperty<OUString>(getRun(getParagraph(2), 3), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getRun(getParagraph(2), 3), u"IsStart"_ustr));

    CPPUNIT_ASSERT_EQUAL(u" "_ustr, getRun(getParagraph(2), 4)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(2), 5)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 6), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Insert"_ustr,
                         getProperty<OUString>(getRun(getParagraph(2), 6), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(2), 7)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(2), 7), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(getRun(getParagraph(2), 7), u"IsStart"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"Will this sentence be duplicated"_ustr,
                         getRun(getParagraph(2), 8)->getString());
    CPPUNIT_ASSERT_EQUAL(u" ADDED STUFF"_ustr, getRun(getParagraph(2), 11)->getString());
    CPPUNIT_ASSERT_EQUAL(u"?"_ustr, getRun(getParagraph(2), 14)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf145720)
{
    // check moveFromRangeStart/End and moveToRangeStart/End (to keep tracked text moving)
    loadAndSave("tdf104797.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // These were 0 (missing move*FromRange* elements)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFrom/w:moveFromRangeStart", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFromRangeEnd", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveTo/w:moveToRangeStart", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveToRangeEnd", 1);

    // paired names
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFrom/w:moveFromRangeStart", "name",
                u"move471382752");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveTo/w:moveToRangeStart", "name",
                u"move471382752");

    // mandatory authors and dates
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFrom/w:moveFromRangeStart", "author",
                u"Tekijä");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveTo/w:moveToRangeStart", "author",
                u"Tekijä");
    // no date (anonymized change)
    // This failed, date was exported as w:date="0-00-00T00:00:00Z", and later "1970-01-01T00:00:00Z"
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p[1]/w:moveFrom/w:moveFromRangeStart",
                           "date");
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveTo/w:moveToRangeStart",
                           "date");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150166)
{
    // check moveFromRangeStart/End and moveToRangeStart/End (to keep tracked text moving)
    loadAndSave("tdf150166.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "//w:moveFromRangeStart", 0);
    // This was 2 (missing RangeStart elements, but bad unpaired RangeEnds)
    assertXPath(pXmlDoc, "//w:moveFromRangeEnd", 0);

    // These were 0 (moveFrom, moveTo and t)
    assertXPath(pXmlDoc, "//w:del", 11);
    assertXPath(pXmlDoc, "//w:ins", 12);
    assertXPath(pXmlDoc, "//w:delText", 7);

    // no more moveFrom/moveTo to avoid of problems with ToC
    assertXPath(pXmlDoc, "//w:moveFrom", 0);
    assertXPath(pXmlDoc, "//w:moveTo", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143510)
{
    // check moveFromRangeStart/End and moveToRangeStart/End for tracked table move by drag & drop
    loadAndSave("TC-table-DnD-move.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 0 (missing tracked table row deletion/insertion)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:trPr/w:del", 2);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr/w:trPr/w:ins", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143510_table_from_row)
{
    // check moveFromRangeStart/End and moveToRangeStart/End for tracked table move by drag & drop
    loadAndSave("TC-table-Separate-Move.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 0 (missing tracked table row deletion/insertion)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr/w:trPr/w:del", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[1]/w:tr[3]/w:trPr/w:del", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl[2]/w:tr/w:trPr/w:ins", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143510_within_table)
{
    // check moveFromRangeStart/End and moveToRangeStart/End for tracked table row move by DnD
    loadAndSave("TC-table-rowDND.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 0 (missing tracked table row deletion/insertion)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:trPr/w:del", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:trPr/w:ins", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143510_within_table2)
{
    // check moveFromRangeStart/End and moveToRangeStart/End for tracked table row move by DnD
    loadAndSave("TC-table-rowDND-front.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 0 (missing tracked table row deletion/insertion)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:trPr/w:ins", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:trPr/w:del", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf150824)
{
    // check tracked table row insertion (stored in a single redline)
    loadAndSave("tdf150824.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // This was 0 (missing tracked table row deletion/insertion)
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[1]/w:trPr/w:ins", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:trPr/w:ins", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:trPr/w:ins", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf157011)
{
    // check tracked table column insertions and deletions with empty cells
    loadAndSave("tdf157011_ins_del_empty_cols.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // This was 1 (missing tracked table cell insertions)
    assertXPath(pXmlDoc, "//w:ins", 3);

    // This was 4 (missing tracked table cell deletions)
    assertXPath(pXmlDoc, "//w:del", 6);

    // tdf#157187 This was false (dummy w:tc/w:p/w:sdt/w:sdtContent content box)
    assertXPath(pXmlDoc, "//w:tc/w:p/w:del", 6);
    assertXPath(pXmlDoc, "//w:tc/w:p/w:ins", 3);
}

DECLARE_OOXMLEXPORT_TEST(testTdf150824_regression, "ooo30436-1-minimized.sxw")
{
    // There should be no crash during loading of the document
    // so, let's check just how much pages we have
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

DECLARE_OOXMLEXPORT_TEST(testTdf113608_runAwayNumbering, "tdf113608_runAwayNumbering.docx")
{
    // check that an incorrect numbering style is not applied
    // after removing a w:r-less paragraph
    CPPUNIT_ASSERT_EQUAL(OUString(),
                         getProperty<OUString>(getParagraph(2), u"NumberingStyleName"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf119188_list_margin_in_cell, "tdf119188_list_margin_in_cell.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);

    // lists with auto margins in cells: top margin of the first paragraph is zero,
    // but not the bottom margin of the last paragraph, also other list items have got
    // zero margins.

    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(1, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(2, xCell->getText()), u"ParaBottomMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(0),
        getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaTopMargin"_ustr));
    CPPUNIT_ASSERT_EQUAL(
        static_cast<sal_Int32>(494),
        getProperty<sal_Int32>(getParagraphOfText(3, xCell->getText()), u"ParaBottomMargin"_ustr));
}

CPPUNIT_TEST_FIXTURE(Test, testChart_BorderLine_Style)
{
    loadAndSave("Chart_BorderLine_Style.docx");
    /* DOCX containing Chart with BorderLine Style as Dash Type should get preserved
     * inside an XML tag <a:prstDash> with value "dash", "sysDot, "lgDot", etc.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[1]/c:spPr/a:ln/a:prstDash",
                "val", u"sysDot");
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[2]/c:spPr/a:ln/a:prstDash",
                "val", u"sysDash");
    assertXPath(pXmlDoc,
                "/c:chartSpace/c:chart/c:plotArea/c:barChart/c:ser[3]/c:spPr/a:ln/a:prstDash",
                "val", u"dash");
}

CPPUNIT_TEST_FIXTURE(Test, testChart_Plot_BorderLine_Style)
{
    loadAndSave("Chart_Plot_BorderLine_Style.docx");
    /* DOCX containing Chart wall (plot area) and Chart Page with BorderLine Style as Dash Type
     * should get preserved inside an XML tag <a:prstDash> with value "dash", "sysDot, "lgDot", etc.
     */
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/charts/chart1.xml"_ustr);
    assertXPath(pXmlDoc, "/c:chartSpace/c:chart/c:plotArea/c:spPr/a:ln/a:prstDash", "val",
                u"lgDashDot");
    assertXPath(pXmlDoc, "/c:chartSpace/c:spPr/a:ln/a:prstDash", "val", u"sysDash");
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesDeletedEmptyParagraph)
{
    loadAndSave("testTrackChangesDeletedEmptyParagraph.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:rPr/w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTrackChangesEmptyParagraphsInADeletion)
{
    loadAndSave("testTrackChangesEmptyParagraphsInADeletion.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    for (int i = 1; i < 12; ++i)
        assertXPath(pXmlDoc,
                    "/w:document/w:body/w:p[" + OString::number(i) + "]/w:pPr/w:rPr/w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149708)
{
    loadAndSave("tdf149708.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // keep tracked insertion of a list item
    // This was 0 (missing tracked insertion of the paragraph mark)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:ins");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149707)
{
    loadAndSave("tdf149711.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:moveFrom");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:moveTo");
    // These were missing
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:moveFrom");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:rPr/w:moveTo");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf70234)
{
    loadAndSave("tdf70234.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import field with tracked deletion
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:del/w:r[1]/w:fldChar");

    // export multiple runs of a field with tracked deletion
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:del/w:r", 6);

    // export w:delInstrText
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:del/w:r/w:delInstrText");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf115212)
{
    loadAndSave("tdf115212.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // export field with tracked deletion
    assertXPath(pXmlDoc, "//w:p[2]/w:del[1]/w:r[1]/w:fldChar");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126243)
{
    loadAndSave("tdf120338.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // export change tracking rejection data for tracked paragraph style change
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:pPrChange/w:pPr/w:pStyle", "val",
                u"Heading3");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf126245)
{
    loadAndSave("tdf126245.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // export change tracking rejection data for tracked numbering change
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:pPrChange/w:pPr/w:numPr/w:numId", "val",
                u"1");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124491)
{
    loadAndSave("tdf124491.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import format change of empty lines, FIXME: change w:r with w:pPr in export
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/*/w:rPr/w:rPrChange");
    // empty line without format change
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/*/w:rPrChange", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/*/*/w:rPrChange", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf143911)
{
    loadAndSave("tdf126206.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // export format change of text portions
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w:rPrChange");
    // This was without tracked bold formatting
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[2]/w:rPr/w:rPrChange/w:rPr/w:b");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105485)
{
    loadAndSave("tdf105485.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking of deleted comments
    assertXPath(pXmlDoc, "//w:del/w:r/w:commentReference");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125894)
{
    loadAndSave("tdf125894.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in frames
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149388)
{
    // see also testTdf132371
    loadAndSave("tdf132271.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    // (don't recognize tracked text moving during the import,
    // because the text is too short and it's only a single word)
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins", 2);
    assertXPath(pXmlDoc, "//w:moveFrom", 0);
    assertXPath(pXmlDoc, "//w:moveTo", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf132271)
{
    // see also testTdf149388
    loadAndSave("tdf149388.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del", 1);
    assertXPath(pXmlDoc, "//w:ins", 1);
    // tracked text moving recognized during the import
    assertXPath(pXmlDoc, "//w:moveFrom", 1);
    assertXPath(pXmlDoc, "//w:moveTo", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf149388_fly)
{
    // see also testTdf136667
    loadAndSave("tdf136667.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del", 2);
    assertXPath(pXmlDoc, "//w:ins", 4);
    assertXPath(pXmlDoc, "//w:moveFrom", 0);
    assertXPath(pXmlDoc, "//w:moveTo", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136667)
{
    // see also testTdf149388_fly
    loadAndSave("tdf149388_fly.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del", 1);
    assertXPath(pXmlDoc, "//w:ins", 3);
    // tracked text moving recognized during the import
    assertXPath(pXmlDoc, "//w:moveFrom", 1);
    assertXPath(pXmlDoc, "//w:moveTo", 1);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136850)
{
    loadAndSave("tdf136850.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // import change tracking in floating tables
    assertXPath(pXmlDoc, "//w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128156)
{
    loadAndSave("tdf128156.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // keep tracked insertion of a paragraph
    // This was 0 before 350972a8bffc1a74b531e0336954bf54b1356025,
    // and 1 later (missing tracked insertion of the paragraph mark)
    assertXPath(pXmlDoc, "//w:ins", 2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf165330)
{
    loadAndSave("CT-with-frame.docx");

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // check that *both* tracked changes were round tripped
    assertXPathContent(pXmlDoc, "//w:p[1]/w:ins/w:r/w:t", u"world! ");
    assertXPathContent(pXmlDoc, "//w:p[3]/w:ins/w:r/w:t", u"hello ");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf125546)
{
    loadAndSave("tdf125546.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // compress redlines (it was 15)
    assertXPath(pXmlDoc, "//w:rPrChange", 3);
}

CPPUNIT_TEST_FIXTURE(Test, testLabelWidthAndPosition_Left_FirstLineIndent)
{
    loadAndSave("Hau_min_list2.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // list is LABEL_WIDTH_AND_POSITION with SvxAdjust::Left
    // I) LTR
    // a) all LTR cases with no number text look good in Word
    // 1) negative first line indent on paragraph:
    // no list width/indent: this one was 0 previously; this looks good
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "end", u"0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "hanging", u"966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:ind", "end", u"0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:pPr/w:ind", "end", u"0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "start", u"1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "hanging", u"966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:ind", "end", u"0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:ind", "end", u"0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "hanging", u"147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[6]/w:pPr/w:ind", "end", u"0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "end", u"0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "start", u"1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "hanging", u"147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[8]/w:pPr/w:ind", "end", u"0");
    // b) all LTR cases with number text: the indent looks good but some tabs are wrong
    // 1) negative first line indent on paragraph:
    // no list width/indent: this one was 0 previously; this looks good
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[9]/w:pPr/w:ind", "end", u"0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "hanging", u"966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:ind", "end", u"0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[11]/w:pPr/w:ind", "end", u"0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "start", u"1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "hanging", u"966");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[12]/w:pPr/w:ind", "end", u"0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[13]/w:pPr/w:ind", "end", u"0");
    // list width:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "hanging", u"147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[14]/w:pPr/w:ind", "end", u"0");
    // list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "start", u"567");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[15]/w:pPr/w:ind", "end", u"0");
    // list width + list indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "start", u"1134");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "hanging", u"147");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[16]/w:pPr/w:ind", "end", u"0");
    // (w:p[17] is empty)
    // I) RTL
    // a) only RTL cases with no number text and no width/indent look good in Word
    // 1) negative first line indent on paragraph:
    // no list width/indent
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[18]/w:pPr/w:ind", "end", u"0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[22]/w:pPr/w:ind", "end", u"0");
    // b) RTL cases with number text: the indent looks good but some tabs are wrong
    // 1) negative first line indent on paragraph:
    // no list width/indent
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "hanging", u"399");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[26]/w:pPr/w:ind", "end", u"0");
    // 2) positive first line indent on paragraph:
    // no list width/indent:
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "start", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "firstLine", u"420");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[30]/w:pPr/w:ind", "end", u"0");
    // TODO: other cases
}

CPPUNIT_TEST_FIXTURE(Test, testTdf124604)
{
    loadAndSave("tdf124604.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // If the numbering comes from a base style, indentation of the base style has also priority.
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:ind", "start", u"0");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf95374)
{
    loadAndSave("tdf95374.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // Numbering disabled by non-existent numId=0, disabling also inheritance of indentation of parent styles
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "hanging", u"0");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:ind", "start", u"1136");
}

DECLARE_OOXMLEXPORT_TEST(testTdf108493, "tdf108493.docx")
{
    uno::Reference<beans::XPropertySet> xPara7(getParagraph(7), uno::UNO_QUERY);
    // set in the paragraph (709 twips)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1251), getProperty<sal_Int32>(xPara7, u"ParaLeftMargin"_ustr));
    // set in the numbering style (this was 0)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1251),
                         getProperty<sal_Int32>(xPara7, u"ParaFirstLineIndent"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf118691, "tdf118691.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    // Text "Before" stays in the first cell, not removed before the table because of
    // bad handling of <w:cr>
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);
    uno::Reference<text::XTextRange> xCell(xTable->getCellByName(u"A1"_ustr), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(u"Before\nAfter"_ustr, xCell->getString());
}

DECLARE_OOXMLEXPORT_TEST(testTdf64264, "tdf64264.docx")
{
    // DOCX table rows with tblHeader setting mustn't modify the count of the
    // repeated table header rows, when there is rows before them without tblHeader settings.
    xmlDocUniquePtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // table starts on page 1 and finished on page 2
    // and it has got only a single repeating header line
    assertXPath(pDump, "/root/page[2]/body/tab", 1);
    assertXPath(pDump, "/root/page[2]/body/tab/row", 47);
    assertXPathContent(pDump, "/root/page[2]/body/tab/row[1]/cell[1]/txt/text()",
                       u"Repeating Table Header");
    assertXPathContent(pDump, "/root/page[2]/body/tab/row[2]/cell[1]/txt/text()", u"Text");
}

DECLARE_OOXMLEXPORT_TEST(testTdf58944RepeatingTableHeader, "tdf58944-repeating-table-header.docx")
{
    // DOCX tables with more than 10 repeating header lines imported without repeating header lines
    // as a workaround for MSO's limitation of header line repetition
    xmlDocUniquePtr pDump = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL(2, getPages());

    // table starts on page 1 and finished on page 2
    // instead of showing only a part of it on page 2
    assertXPath(pDump, "/root/page[1]/body/tab", 1);
    assertXPath(pDump, "/root/page[1]/body/tab/row", 11);
    assertXPathContent(pDump, "/root/page[2]/body/tab/row[1]/cell[1]/txt/text()", u"Test1");
    assertXPathContent(pDump, "/root/page[2]/body/tab/row[2]/cell[1]/txt/text()", u"Test2");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf81100)
{
    auto verify = [this](bool bIsExport = false) {
        xmlDocUniquePtr pDump = parseLayoutDump();
        CPPUNIT_ASSERT_EQUAL(3, getPages());

        // table starts on page 1 and finished on page 2
        // and it has got only a single repeating header line
        assertXPath(pDump, "/root/page[2]/body/tab[1]", 1);
        assertXPath(pDump, "/root/page[2]/body/tab[1]/row", 2);
        assertXPath(pDump, "/root/page[3]/body/tab", 1);
        if (!bIsExport) // TODO export tblHeader=false
            assertXPath(pDump, "/root/page[3]/body/tab/row", 1);
    };
    createSwDoc("tdf81100.docx");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/styles.xml"_ustr);
    CPPUNIT_ASSERT(pXmlDoc);
    // keep "repeat table header" setting of table styles
    assertXPath(pXmlDoc, "/w:styles/w:style/w:tblStylePr/w:trPr/w:tblHeader", 4);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf88496)
{
    loadAndReload("tdf88496.docx");
    // Switch off repeating header, there is no place for it.
    // Now there are only 3 pages with complete table content
    // instead of a 51-page long table only with header.
    CPPUNIT_ASSERT_EQUAL(3, getPages());
    // (this appears to have the correct result now?)
    // FIXME: this actually has 3 pages but SwWrtShell::SttPg() puts the cursor
    // into the single SwTextFrame in the follow-flow-row at the top of the
    // table but that SwTextFrame 1105 should not exist and the cursor ends up
    // at the end of its master frame 848 instead; the problem is somewhere in
    // SwTextFrame::FormatAdjust() which first determines nNew = 1 but then
    // grows this frame anyway so that the follow is empty, but nothing
    // invalidates 1105 again.
}

CPPUNIT_TEST_FIXTURE(Test, testTdf77417)
{
    loadAndReload("tdf77417.docx");
    // MSO 2010 compatibility mode: terminating white spaces are ignored in tables.
    // This was 3 pages with the first invisible blank page.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130494)
{
    loadAndSave("tdf130494.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:highlight", "val",
                u"yellow");
    // keep direct formatting of table cell paragraph with removed highlighting
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r/w:rPr/w:highlight", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf130690)
{
    loadAndSave("tdf130690.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:pPr/w:rPr/w:highlight", "val",
                u"yellow");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[1]/w:rPr/w:highlight", 1);
    // keep direct formatting of table cell paragraph with removed highlighting
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc[1]/w:p/w:r[2]/w:rPr/w:highlight", 0);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf105215)
{
    loadAndSave("tdf105215.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:pPr/w:rPr/w:rFonts", "ascii",
                u"Linux Libertine G");

    // These were "Linux Libertine G"
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r/w:rPr", 5);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[2]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[3]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[4]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[5]/w:rPr/w:rFonts", "ascii",
                u"Lohit Devanagari");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf135187)
{
    loadAndSave("tdf135187.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:pPr/w:rPr/w:b", 1);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b",
                           "val");
    // This was 0
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[3]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b",
                           "val");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[4]/w:tc[1]/w:p/w:r[1]/w:rPr/w:b", "val",
                u"false");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf136617)
{
    loadAndSave("tdf136617.docx");

    // This was 2
    CPPUNIT_ASSERT_EQUAL(1, getPages());

    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr[2]/w:tc[2]/w:p[2]/w:pPr/w:rPr/w:sz", "val",
                u"16");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121597TrackedDeletionOfMultipleParagraphs)
{
    loadAndSave("tdf121597.odt");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    // check paragraphs with removed paragraph mark
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[4]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[5]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[7]/w:pPr/w:rPr/w:del");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[10]/w:pPr/w:rPr/w:del");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf141660)
{
    loadAndSave("tdf141660.docx");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:r[2]/w:footnoteReference", "id", u"2");
    // w:del is imported correctly with its footnote
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:del[2]/w:r/w:footnoteReference", "id", u"3");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[3]/w:r/w:footnoteReference", "id", u"4");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf133643)
{
    loadAndSave("tdf133643.doc");
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar", "fldCharType",
                u"begin");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar/w:ffData/w:ddList/w:listEntry[1]",
        "val", u"Bourgoin-Jallieu, ");
    assertXPath(
        pXmlDoc,
        "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[1]/w:fldChar/w:ffData/w:ddList/w:listEntry[2]",
        "val", u"Fontaine, ");

    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[2]/w:instrText",
                       u" FORMDROPDOWN ");

    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[3]/w:fldChar", "fldCharType",
                u"separate");
    assertXPath(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[5]/w:fldChar", "fldCharType",
                u"end");

    // Without the fix in place, this w:r wouldn't exist
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:tbl/w:tr/w:tc/w:p/w:r[6]/w:t",
                       u"le 22 fevrier 2013");
}

DECLARE_OOXMLEXPORT_TEST(testTdf123189_tableBackground, "table-black_fill.docx")
{
    uno::Reference<text::XTextTablesSupplier> xTextTablesSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xTables(xTextTablesSupplier->getTextTables(),
                                                    uno::UNO_QUERY);
    uno::Reference<text::XTextTable> xTable(xTables->getByIndex(0), uno::UNO_QUERY);

    uno::Reference<table::XCell> xCell = xTable->getCellByName(u"A1"_ustr);
    CPPUNIT_ASSERT_EQUAL(COL_TRANSPARENT, getProperty<Color>(xCell, u"BackColor"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf116084, "tdf116084.docx")
{
    // tracked line is not a single text portion: w:del is recognized within w:ins
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(1), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 1), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"There "_ustr, getRun(getParagraph(1), 2)->getString());
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(1), 4)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 4), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"must"_ustr, getRun(getParagraph(1), 5)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf116084_anonymized)
{
    loadAndSave("tdf116084.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:del in w:ins is exported correctly
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del/w:r/w:delText", u"must");

    // no date (anonymized changes)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins[@date]", 0);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del[@w:date]", 0);

    // w:ins and w:del have w:author attributes, and the same
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del[@w:author]", 1);
    OUString sAuthor = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins[2]", "author");
    OUString sAuthor2 = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del", "author");
    CPPUNIT_ASSERT_EQUAL(sAuthor, sAuthor2);
}

DECLARE_OOXMLEXPORT_TEST(testTdf121176, "tdf121176.docx")
{
    // w:del is imported correctly when it is in a same size w:ins
    CPPUNIT_ASSERT_EQUAL(u""_ustr, getRun(getParagraph(1), 1)->getString());
    CPPUNIT_ASSERT(hasProperty(getRun(getParagraph(1), 1), u"RedlineType"_ustr));
    CPPUNIT_ASSERT_EQUAL(u"must"_ustr, getRun(getParagraph(1), 2)->getString());
}

CPPUNIT_TEST_FIXTURE(Test, testTdf121176_anonymized)
{
    loadAndSave("tdf121176.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:del in w:ins is exported correctly
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del/w:r/w:delText", u"must");

    // no date (anonymized changes)
    assertXPathNoAttribute(pXmlDoc, "/w:document/w:body/w:p/w:ins", "date");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del[@w:date]", 0);

    // w:ins and w:del have w:author attributes, and the same
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del[@w:author]", 1);
    OUString sAuthor = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins", "author");
    OUString sAuthor2 = getXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del", "author");
    CPPUNIT_ASSERT_EQUAL(sAuthor, sAuthor2);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf128913)
{
    loadAndSave("tdf128913.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:ins and w:del are imported correctly, if they contain only inline images
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:r/w:drawing/wp:inline/a:graphic");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:del/w:r/w:drawing/wp:inline/a:graphic");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142700)
{
    loadAndSave("tdf142700.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:ins and w:del are imported correctly, if they contain only images anchored to character
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:r/w:drawing/wp:anchor/a:graphic");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:del/w:r/w:drawing/wp:anchor/a:graphic");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf142387)
{
    loadAndSave("tdf142387.docx");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:del in w:ins is exported correctly (only w:del was exported)
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:ins/w:del/w:r/w:delText", u"inserts ");
}

CPPUNIT_TEST_FIXTURE(Test, testTdf147892)
{
    loadAndSave("tdf147892.fodt");
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);
    // w:del in w:ins is exported correctly
    // (both w:del and w:ins were exported for para marker)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:del", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[1]/w:pPr/w:rPr/w:ins", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf123054, "tdf123054.docx")
{
    CPPUNIT_ASSERT_EQUAL(u"No Spacing"_ustr,
                         getProperty<OUString>(getParagraph(20), u"ParaStyleName"_ustr));
}

DECLARE_OOXMLEXPORT_TEST(testTdf67207_MERGEFIELD_DATABASE, "tdf67207.docx")
{
    // database fields use the database "database" and its table "Sheet1"
    uno::Reference<beans::XPropertySet> xTextField
        = getProperty<uno::Reference<beans::XPropertySet>>(getRun(getParagraph(2), 2),
                                                           u"TextField"_ustr);
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService(u"com.sun.star.text.TextField.Database"_ustr));
    OUString sValue;
    xTextField->getPropertyValue(u"Content"_ustr) >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8("<c1>"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(
        xFiledMasterServiceInfo->supportsService(u"com.sun.star.text.fieldmaster.Database"_ustr));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataBaseName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"database"_ustr, sValue);
    sal_Int32 nCommandType;
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataCommandType"_ustr) >>= nCommandType);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), nCommandType); // css::sdb::CommandType::TABLE
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataTableName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"Sheet1"_ustr, sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"DataColumnName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"c1"_ustr, sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue(u"InstanceName"_ustr) >>= sValue);
    CPPUNIT_ASSERT_EQUAL(u"com.sun.star.text.fieldmaster.DataBase.database.Sheet1.c1"_ustr, sValue);
}

CPPUNIT_TEST_FIXTURE(Test, testTdf101122_noFillForCustomShape)
{
    loadAndSave("tdf101122_noFillForCustomShape.odt");
    CPPUNIT_ASSERT_EQUAL(2, getShapes());
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#101122 check whether the "F" (noFill) option has been exported to docx
    xmlDocUniquePtr pXmlDoc = parseExport(u"word/document.xml"_ustr);

    assertXPath(pXmlDoc,
                "/w:document/w:body/w:p/w:r/mc:AlternateContent[1]/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wps:wsp/wps:spPr/a:custGeom/a:pathLst/a:path",
                "fill", u"none");
    assertXPathNoAttribute(
        pXmlDoc,
        "/w:document/w:body/w:p/w:r/mc:AlternateContent[2]/mc:Choice/w:drawing/wp:anchor/a:graphic/"
        "a:graphicData/wps:wsp/wps:spPr/a:custGeom/a:pathLst/a:path",
        "fill");
}
// The (tdf124678_no_leading_paragraph.odt, tdf124678_with_leading_paragraph.odt) documents are the same,
// except:
// - tdf124678_no_leading_paragraph.odt doesn't contain leading empty paragraph
//   before the first section
//
CPPUNIT_TEST_FIXTURE(Test, testTdf124678_case1)
{
    loadAndReload("tdf124678_no_leading_paragraph.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First page header text", u""_ustr,
                                 getXPathContent(pXmlDoc, "/root/page[1]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second page header text", u"HEADER"_ustr,
                                 getXPathContent(pXmlDoc, "/root/page[2]/header/txt"));
}

// The (tdf124678_no_leading_paragraph.odt, tdf124678_with_leading_paragraph.odt) documents are the same,
// except:
// - tdf124678_no_leading_paragraph.odt doesn't contain leading empty paragraph
//   before the first section
//
CPPUNIT_TEST_FIXTURE(Test, testTdf124678_case2)
{
    loadAndReload("tdf124678_with_leading_paragraph.odt");
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First page header text", u""_ustr,
                                 getXPathContent(pXmlDoc, "/root/page[1]/header/txt"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Second page header text", u"HEADER"_ustr,
                                 getXPathContent(pXmlDoc, "/root/page[2]/header/txt"));
}

static bool lcl_nearEqual(const sal_Int32 nNumber1, const sal_Int32 nNumber2,
                          sal_Int32 nMaxDiff = 5)
{
    return std::abs(nNumber1 - nNumber2) < nMaxDiff;
}

CPPUNIT_TEST_FIXTURE(Test, testTdf119952_negativeMargins)
{
    auto verify = [this](bool bIsExport = false) {
        // With negative margins (in MS Word) one can set up header (or footer) that overlaps with the body.
        // LibreOffice unable to display that, so when importing negative margins,
        // the header (or footer) converted to a flyframe, anchored to the header..
        // that can overlap with the body, and will appear like in Word.
        // This conversion modifies the document [i.e. replacing header text with a textbox...]
        // but its DOCX export looks the same, as the original document in Word, too.
        xmlDocUniquePtr pDump = parseLayoutDump();

        //Check layout positions / sizes
        sal_Int32 nLeftHead = getXPath(pDump, "//page[1]/header/infos/bounds", "left").toInt32();
        sal_Int32 nLeftBody = getXPath(pDump, "//page[1]/body/infos/bounds", "left").toInt32();
        sal_Int32 nLeftFoot = getXPath(pDump, "//page[1]/footer/infos/bounds", "left").toInt32();
        sal_Int32 nLeftHFly
            = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "left").toInt32();
        sal_Int32 nLeftFFly
            = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "left").toInt32();

        sal_Int32 nTopHead = getXPath(pDump, "//page[1]/header/infos/bounds", "top").toInt32();
        sal_Int32 nTopBody = getXPath(pDump, "//page[1]/body/infos/bounds", "top").toInt32();
        sal_Int32 nTopFoot = getXPath(pDump, "//page[1]/footer/infos/bounds", "top").toInt32();
        sal_Int32 nTopHFly
            = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "top").toInt32();
        sal_Int32 nTopFFly
            = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "top").toInt32();

        sal_Int32 nHeightHead
            = getXPath(pDump, "//page[1]/header/infos/bounds", "height").toInt32();
        sal_Int32 nHeightBody = getXPath(pDump, "//page[1]/body/infos/bounds", "height").toInt32();
        sal_Int32 nHeightFoot
            = getXPath(pDump, "//page[1]/footer/infos/bounds", "height").toInt32();
        sal_Int32 nHeightHFly
            = getXPath(pDump, "//page[1]/header/txt/anchored/fly/infos/bounds", "height").toInt32();
        sal_Int32 nHeightFFly
            = getXPath(pDump, "//page[1]/footer/txt/anchored/fly/infos/bounds", "height").toInt32();
        sal_Int32 nHeightHFlyBound
            = getXPath(pDump, "//page[1]/header/infos/prtBounds", "height").toInt32();
        sal_Int32 nHeightFFlyBound
            = getXPath(pDump, "//page[1]/footer/infos/prtBounds", "height").toInt32();

        CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftBody));
        CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftFoot));
        CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftHFly));
        CPPUNIT_ASSERT(lcl_nearEqual(nLeftHead, nLeftFFly));

        CPPUNIT_ASSERT(lcl_nearEqual(nTopHead, 851));
        CPPUNIT_ASSERT(lcl_nearEqual(nTopBody, 1418));
        CPPUNIT_ASSERT(lcl_nearEqual(nTopFoot, 15875));
        CPPUNIT_ASSERT(lcl_nearEqual(nTopHFly, 851));

        // this seems to be an import bug
        if (!bIsExport)
            CPPUNIT_ASSERT(lcl_nearEqual(nTopFFly, 14403));

        CPPUNIT_ASSERT(lcl_nearEqual(nHeightHead, 567));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightBody, 14457));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightFoot, 680));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightHFly, 2152));
        CPPUNIT_ASSERT(lcl_nearEqual(nHeightFFly, 2152));

        // after export these heights increase to like 567.
        // not sure if it is another import, or export bug... or just the result of the modified document
        if (!bIsExport)
        {
            CPPUNIT_ASSERT(lcl_nearEqual(nHeightHFlyBound, 57));
            CPPUNIT_ASSERT(lcl_nearEqual(nHeightFFlyBound, 57));
        }

        //Check text of header/ footer
        assertXPath(
            pDump,
            "//page[1]/header/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"f1");
        assertXPath(
            pDump,
            "//page[1]/header/txt/anchored/fly/txt[8]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"                f8");
        assertXPath(
            pDump,
            "//page[1]/footer/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"                f8");
        assertXPath(
            pDump,
            "//page[1]/footer/txt/anchored/fly/txt[8]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"f1");

        assertXPath(
            pDump,
            "//page[2]/header/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"p1");
        assertXPath(
            pDump,
            "//page[2]/footer/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"p1");

        assertXPath(
            pDump,
            "//page[3]/header/txt/anchored/fly/txt[1]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"  aaaa");
        assertXPath(
            pDump,
            "//page[3]/header/txt/anchored/fly/txt[5]/SwParaPortion/SwLineLayout/SwParaPortion",
            "portion", u"      eeee");

        assertXPathContent(pDump, "/root/page[1]/header/txt/anchored/fly",
                           u"f1    f2      f3        f4          f5            f6            "
                           "  f7                f8");
        assertXPathContent(pDump, "/root/page[1]/footer/txt/anchored/fly",
                           u"                f8              f7            f6          f5    "
                           "    f4      f3    f2f1");
        assertXPathContent(pDump, "/root/page[2]/header/txt/anchored/fly", u"p1");
        assertXPathContent(pDump, "/root/page[2]/footer/txt/anchored/fly", u"p1");
        assertXPathContent(pDump, "/root/page[3]/header/txt/anchored/fly",
                           u"  aaaa   bbbb    cccc     dddd      eeee");
    };

    createSwDoc("tdf119952_negativeMargins.docx");
    verify();
    saveAndReload(mpFilter);
    verify(/*bIsExport*/ true);
}

DECLARE_OOXMLEXPORT_TEST(testTdf143384_tableInFoot_negativeMargins,
                         "tdf143384_tableInFoot_negativeMargins.docx")
{
    // There should be no crash during loading of the document
    // so, let's check just how much pages we have
    // Ideally this would be 1, matching Word.
    CPPUNIT_ASSERT_EQUAL(2, getPages());
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
