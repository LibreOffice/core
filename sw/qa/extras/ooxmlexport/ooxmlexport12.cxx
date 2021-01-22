/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sstream>

#include <swmodeltestbase.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/XBookmarksSupplier.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/document/XViewDataSupplier.hpp>

class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Denylist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTableCrossReference, "table_cross_reference.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#42346: Cross references to tables were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(4), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_number_only"));

    // Check bookmark text ranges
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table 1: Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
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
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table 1: Table caption"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Page style reference / exported as simple page reference
            case 1:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to table number
            case 2:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_number_only"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to caption only
            case 3:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_caption_only"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to category and number
            case 4:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Table 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_label_and_number"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Reference to page of the table
            case 5:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
                CPPUNIT_ASSERT_EQUAL(sal_Int16(0), nValue);
                break;
            }
            // Above / below reference
            case 6:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                sal_Int16 nValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("above"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Table0_full"), sValue);
                xPropertySet->getPropertyValue("SequenceNumber") >>= nValue;
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

DECLARE_OOXMLEXPORT_TEST(testTableCrossReferenceCustomFormat,
                         "table_cross_reference_custom_format.odt")
{
    CPPUNIT_ASSERT_EQUAL(1, getPages());
    // tdf#42346: Cross references to tables were not saved
    // Check also captions with custom formatting
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(16), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table1_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table2_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Table3_number_only"));

    // Check bookmark text ranges
    // First table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table0_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("1. Table: Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("1. Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    // Second table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table1_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("2. TableTable caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table1_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("2. Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table1_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table1_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("2"), xRange->getString());
    }
    // Third table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table2_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("3) Table Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table2_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("3) Table"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table2_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table2_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("3"), xRange->getString());
    }
    // Fourth table's caption
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Table3_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table 4- Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table3_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table 4"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table3_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Table caption"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Table3_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("4"), xRange->getString());
    }
}

DECLARE_OOXMLEXPORT_TEST(testObjectCrossReference, "object_cross_reference.odt")
{
    CPPUNIT_ASSERT_EQUAL(10, getShapes());
    CPPUNIT_ASSERT_EQUAL(2, getPages());
    // tdf#42346: Cross references to objects were not saved
    // MSO uses simple bookmarks for referencing table caption, so we do the same by export
    if (!mbExported)
        return;

    // Check whether we have all the necessary bookmarks exported and imported back
    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(15), xBookmarksByIdx->getCount());
    uno::Reference<container::XNameAccess> xBookmarksByName = xBookmarksSupplier->getBookmarks();
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Drawing1_full"));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Illustration1_caption_only"));

    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_full"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_label_and_number"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_caption_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text0_number_only"));
    CPPUNIT_ASSERT(xBookmarksByName->hasByName("Ref_Text1_label_and_number"));

    // Check bookmark text ranges
    // Cross references to shapes
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1: A rectangle"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("A rectangle"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Drawing1_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Drawing 2: a circle"), xRange->getString());
    }

    // Cross references to pictures
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration0_full"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1: A picture"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("A picture"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Illustration1_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("an other image"), xRange->getString());
    }

    // Cross references to text frames
    {
        uno::Reference<text::XTextContent> xContent(xBookmarksByName->getByName("Ref_Text0_full"),
                                                    uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Text 1: A frame"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Text0_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Text 1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Text0_caption_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("A frame"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Text0_number_only"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("1"), xRange->getString());
    }
    {
        uno::Reference<text::XTextContent> xContent(
            xBookmarksByName->getByName("Ref_Text1_label_and_number"), uno::UNO_QUERY);
        uno::Reference<text::XTextRange> xRange = xContent->getAnchor();
        CPPUNIT_ASSERT_EQUAL(OUString("Text 2"), xRange->getString());
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
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration0_number_only"), sValue);
                break;
            }
            // Full reference to the circle shape
            case 1:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Drawing 2: a circle"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing1_full"), sValue);
                break;
            }
            // Caption only reference to the second picture
            case 2:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("an other image"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration1_caption_only"), sValue);
                break;
            }
            // Category and number reference to second text frame
            case 3:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Text 2"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text1_label_and_number"), sValue);
                break;
            }
            // Full reference to rectangle shape
            case 4:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1: A rectangle"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing0_full"), sValue);
                break;
            }
            // Caption only reference to rectangle shape
            case 5:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("A rectangle"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing0_caption_only"), sValue);
                break;
            }
            // Category and number reference to rectangle shape
            case 6:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Drawing 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing0_label_and_number"), sValue);
                break;
            }
            // Reference to rectangle shape's number
            case 7:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Drawing0_number_only"), sValue);
                break;
            }
            // Full reference to first text frame
            case 8:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Text 1: A frame"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text0_full"), sValue);
                break;
            }
            // Caption only reference to first text frame
            case 9:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("A frame"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text0_caption_only"), sValue);
                break;
            }
            // Category and number reference to first text frame
            case 10:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Text 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text0_label_and_number"), sValue);
                break;
            }
            // Number only reference to first text frame
            case 11:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Text0_number_only"), sValue);
                break;
            }
            // Full reference to first picture
            case 12:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1: A picture"),
                                     sValue.trim()); // fails on MAC without trim
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration0_full"), sValue);
                break;
            }
            // Reference to first picture' caption
            case 13:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("A picture"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration0_caption_only"), sValue);
                break;
            }
            // Category and number reference to first picture
            case 14:
            {
                CPPUNIT_ASSERT(
                    xServiceInfo->supportsService("com.sun.star.text.TextField.GetReference"));
                OUString sValue;
                xPropertySet->getPropertyValue("CurrentPresentation") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Illustration 1"), sValue);
                xPropertySet->getPropertyValue("SourceName") >>= sValue;
                CPPUNIT_ASSERT_EQUAL(OUString("Ref_Illustration0_label_and_number"), sValue);
                break;
            }
            default:
                break;
        }
        ++nIndex;
    }

    CPPUNIT_ASSERT_EQUAL(sal_uInt16(21), nIndex);
}

DECLARE_OOXMLEXPORT_TEST(testTdf112202, "090716_Studentische_Arbeit_VWS.docx")
{
    xmlDocUniquePtr pXmlDoc = parseLayoutDump();

    // page 1 header: 1 paragraph, 2 flys, 1 draw object
    assertXPath(pXmlDoc, "/root/page[1]/header/txt", 1);
    assertXPath(pXmlDoc, "/root/page[1]/header/txt/anchored/fly", 2);
    if (mbExported) // somehow there's an additional shape on re-import?
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
    assertXPath(pXmlDoc, "/root/page[3]/header/tab/row/cell/txt/Text", 0);
    assertXPath(pXmlDoc, "/root/page[3]/header//anchored", 0);

    // page 4 header: 1 table, 1 paragraph, with text
    assertXPath(pXmlDoc, "/root/page[4]/header/txt", 1);
    assertXPath(pXmlDoc, "/root/page[4]/header/tab", 1);
    assertXPath(pXmlDoc, "/root/page[4]/header/tab/row[1]/cell[1]/txt[1]/Text", "Portion",
                "Titel der studentischen Arbeit");
    assertXPath(pXmlDoc, "/root/page[4]/header//anchored", 0);

    // page 5: same as page 4
    assertXPath(pXmlDoc, "/root/page[5]/header/txt", 1);
    assertXPath(pXmlDoc, "/root/page[5]/header/tab", 1);
    assertXPath(pXmlDoc, "/root/page[5]/header/tab/row[1]/cell[1]/txt[1]/Text", "Portion",
                "Titel der studentischen Arbeit");
    assertXPath(pXmlDoc, "/root/page[5]/header//anchored", 0);

    // page 6: same as page 4
    assertXPath(pXmlDoc, "/root/page[6]/header/txt", 1);
    assertXPath(pXmlDoc, "/root/page[6]/header/tab", 1);
    assertXPath(pXmlDoc, "/root/page[6]/header/tab/row[1]/cell[1]/txt[1]/Text", "Portion",
                "Titel der studentischen Arbeit");
    assertXPath(pXmlDoc, "/root/page[6]/header//anchored", 0);
}

DECLARE_OOXMLEXPORT_TEST(testTdf79435_legacyInputFields, "tdf79435_legacyInputFields.doc")
{
    //using .doc input file to verify cross-format compatibility.
    uno::Reference<text::XFormField> xFormField
        = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(5), 3), "Bookmark");
    uno::Reference<container::XNameContainer> xParameters(xFormField->getParameters());

    OUString sTmp;
    // Doc import problems, so disabling tests
    //xParameters->getByName("EntryMacro") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("test"), sTmp);
    //xParameters->getByName("Help") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("F1Help"), sTmp);
    //xParameters->getByName("ExitMacro") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("test"), sTmp);
    xParameters->getByName("Hint") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("StatusHelp"), sTmp);
    //xParameters->getByName("Content") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("Camelcase"), sTmp);
    //xParameters->getByName("Format") >>= sTmp;
    //CPPUNIT_ASSERT_EQUAL(OUString("TITLE CASE"), sTmp);

    sal_uInt16 nMaxLength = 0;
    xParameters->getByName("MaxLength") >>= nMaxLength;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Max Length", sal_uInt16(10), nMaxLength);

    // too bad this is based on character runs - just found try trial and error.
    xFormField
        = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(6), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("calculated"), sTmp);

    xFormField
        = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(7), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("currentDate"), sTmp);

    xFormField
        = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(7), 7), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("currentTime"), sTmp);

    xFormField
        = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(8), 2), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("number"), sTmp);

    xFormField
        = getProperty<uno::Reference<text::XFormField>>(getRun(getParagraph(8), 7), "Bookmark");
    xParameters.set(xFormField->getParameters());
    xParameters->getByName("Type") >>= sTmp;
    CPPUNIT_ASSERT_EQUAL(OUString("date"), sTmp);
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

    CPPUNIT_ASSERT_EQUAL(OUString("TextFieldStart"),
                         getProperty<OUString>(xPropertySet, "TextPortionType"));
    uno::Reference<container::XNamed> xBookmark(
        getProperty<uno::Reference<beans::XPropertySet>>(xPropertySet, "Bookmark"), uno::UNO_QUERY);

    // Critical test: does TextField's bookmark name match cross-reference?
    const OUString& sTextFieldName(xBookmark->getName());
    uno::Reference<text::XTextFieldsSupplier> xTextFieldsSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XEnumerationAccess> xFieldsAccess(
        xTextFieldsSupplier->getTextFields());
    uno::Reference<container::XEnumeration> xFields(xFieldsAccess->createEnumeration());
    CPPUNIT_ASSERT(xFields->hasMoreElements());
    xPropertySet.set(xFields->nextElement(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sTextFieldName, getProperty<OUString>(xPropertySet, "SourceName"));

    uno::Reference<text::XBookmarksSupplier> xBookmarksSupplier(mxComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xBookmarksByIdx(xBookmarksSupplier->getBookmarks(),
                                                            uno::UNO_QUERY);
    // TextFields should not be turned into real bookmarks.
    CPPUNIT_ASSERT_EQUAL(static_cast<sal_Int32>(1), xBookmarksByIdx->getCount());

    // The actual name isn't critical, but if it fails, it is worth asking why.
    CPPUNIT_ASSERT_EQUAL(OUString("Text1"), sTextFieldName);
}

DECLARE_OOXMLEXPORT_TEST(testTdf117504_numberingIndent, "tdf117504_numberingIndent.docx")
{
    OUString sName = getProperty<OUString>(getParagraph(1), "NumberingStyleName");
    CPPUNIT_ASSERT_MESSAGE("Paragraph has numbering style", !sName.isEmpty());

    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("ParagraphStyles")->getByName("Revision"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(353), getProperty<sal_Int32>(xPropertySet, "ParaBottomMargin"));
    xPropertySet.set(getStyles("ParagraphStyles")->getByName("Body Note"), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty<sal_Int32>(xPropertySet, "ParaBottomMargin"));
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf73547, "tdf73547-dash.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");
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
        OUString::fromUtf8(
            u8"عندما يريد العالم أن يتكلّم ‬ ، فهو يتحدّث "
            u8"بلغة "
            u8"يونيكود. تسجّل الآن لحضور المؤتمر الدولي العاشر "
            u8"ليونيكود (Unicode Conference)، الذي سيعقد في 10-12 "
            u8"آذار 1997 بمدينة مَايِنْتْس، ألمانيا. و سيجمع المؤتمر "
            u8"بين خبراء من كافة قطاعات الصناعة على الشبكة "
            u8"العالمية انترنيت ويونيكود، حيث ستتم، على الصعيدين "
            u8"الدولي والمحلي على حد سواء مناقشة سبل استخدام "
            u8"يونكود في النظم القائمة وفيما يخص التطبيقات "
            u8"الحاسوبية، الخطوط، تصميم النصوص والحوسبة متعددة "
            u8"اللغات."),
        sParaText);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf105444, "tdf105444.docx")
{
    xmlDocUniquePtr pXmlComm = parseExport("word/comments.xml");
    // there is no extra paragraph on Win32, only a single one.
    assertXPath(pXmlComm, "/w:comments/w:comment/w:p", 1);
}

DECLARE_OOXMLEXPORT_TEST(testTdf117137, "tdf117137.docx")
{
    // Paragraphs were not part of a numbering anymore after roundtrip.
    uno::Reference<beans::XPropertySet> xPara1(getParagraph(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara1.is());
    CPPUNIT_ASSERT(xPara1->getPropertyValue("NumberingRules").hasValue());

    uno::Reference<beans::XPropertySet> xPara2(getParagraph(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara2.is());
    CPPUNIT_ASSERT(xPara2->getPropertyValue("NumberingRules").hasValue());

    uno::Reference<beans::XPropertySet> xPara3(getParagraph(3), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xPara3.is());
    CPPUNIT_ASSERT(xPara3->getPropertyValue("NumberingRules").hasValue());
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf134618, "tdf134618.doc")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    //Without the fix it in place, it would have failed with
    //- Expected: 1
    //- Actual  : 9
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r", 1);

    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/mc:AlternateContent", 2);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf99631, "tdf99631.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "//w:object", 2);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object", 2);
    // first XSLX OLE object (1:1 scale)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:object[1]", "dxaOrig", "2560");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:object[1]", "dyaOrig", "513");
    // second XLSX OLE object (same content + 1 row, but zoomed)
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:object[1]", "dxaOrig", "2560");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[2]/w:object[1]", "dyaOrig", "768");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf138899, "tdf138899.docx")
{
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");
    // This was 6, not removed empty temporary paragraph at the end of the section
    assertXPath(pXmlDocument, "/w:document/w:body/w:p", 5);
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf122563, "tdf122563.docx")
{
    xmlDocUniquePtr pXmlDoc = parseExport("word/document.xml");

    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r/w:object", 1);
    // Size of the embedded OLE spreadsheet was the bad "width:28.35pt;height:28.35pt"
    assertXPath(pXmlDoc, "/w:document/w:body/w:p[2]/w:r[1]/w:object/v:shape", "style",
                "width:255.75pt;height:63.75pt;mso-wrap-distance-right:0pt");
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf94628, "tdf94628.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(
        getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(
        xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level

    OUString sBulletChar = std::find_if(aProps.begin(), aProps.end(),
                                        [](const beans::PropertyValue& rValue) {
                                            return rValue.Name == "BulletChar";
                                        })
                               ->Value.get<OUString>();
    // Actually for 'BLACK UPPER RIGHT TRIANGLE' is \u25E5
    // But we use Wingdings 3 font here, so code is different
    CPPUNIT_ASSERT_EQUAL(OUString(u"\uF07B"), sBulletChar);
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
    xSheets->getPropertyValue("Model") >>= xModel;
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
                    CPPUNIT_ASSERT_EQUAL(OUString("Munka2"), sTabName);
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

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testLanguageInGroupShape, "tdf131922_LanguageInGroupShape.docx")
{
    // tdf#131922: Check if good language is used in shape group texts
    xmlDocUniquePtr pXml = parseExport("word/document.xml");
    assertXPath(pXml,
                "/w:document/w:body/w:p[7]/w:r/mc:AlternateContent/mc:Choice/w:drawing/wp:anchor/"
                "a:graphic/a:graphicData/wpg:wgp/"
                "wps:wsp[1]/wps:txbx/w:txbxContent/w:p/w:r/w:rPr/w:lang",
                "val", "de-DE");
}

DECLARE_OOXMLEXPORT_TEST(testTdf116883, "tdf116883.docx")
{
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(1), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>1>"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(2), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>2>"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(3), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>2>1>1>"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(4), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>2>2>"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(5), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>2>3>"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(6), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>1)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(7), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>2)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(8), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>2>1<1)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(9), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>2.2)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
    {
        uno::Reference<beans::XPropertySet> xPara(getParagraph(10), uno::UNO_QUERY);
        CPPUNIT_ASSERT_EQUAL(OUString("1>2.3)"), getProperty<OUString>(xPara, "ListLabelString"));
    }
}

DECLARE_OOXMLEXPORT_EXPORTONLY_TEST(testTdf131420, "tdf131420.docx")
{
    xmlDocUniquePtr pXmlDocument = parseExport("word/document.xml");
    assertXPath(pXmlDocument, "/w:document/w:body/w:p/w:pPr/w:pBdr[2]");
}

DECLARE_OOXMLEXPORT_TEST(testTdf80526_word_wrap, "tdf80526_word_wrap.docx")
{
    // tdf#80526: check whether the "wrap" property has been set
    // TODO: fix export too
    if (mbExported)
        return;
    uno::Reference<drawing::XShape> xShape = getShape(1);
    CPPUNIT_ASSERT_EQUAL(false, getProperty<bool>(xShape, "TextWordWrap"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
