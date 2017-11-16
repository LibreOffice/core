/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/XFootnote.hpp>
#include <com/sun/star/text/XPageCursor.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/XTextFrame.hpp>
#include <com/sun/star/text/XTextFramesSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>


#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <svx/xfillit0.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text") {}

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }
};

DECLARE_OOXMLEXPORT_TEST(testTdf92524_autoColor, "tdf92524_autoColor.doc")
{
    CPPUNIT_ASSERT_EQUAL(drawing::FillStyle_NONE, getProperty<drawing::FillStyle>(getParagraph(1), "FillStyle"));
    CPPUNIT_ASSERT_EQUAL(COL_AUTO, getProperty<sal_uInt32>(getParagraph(1), "ParaBackColor"));
}

DECLARE_OOXMLEXPORT_TEST(testTdf46938_clearTabStop, "tdf46938_clearTabStop.docx")
{
    // Number of tabstops should be zero, overriding the one in the style
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), getProperty< uno::Sequence<style::TabStop> >(getParagraph(1), "ParaTabStops").getLength());
}

DECLARE_OOXMLEXPORT_TEST(testTdf82065_Ind_start_strict, "tdf82065_Ind_start_strict.docx")
{
    uno::Reference<beans::XPropertySet> xPropertySet(getStyles("NumberingStyles")->getByName("WWNum1"), uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xLevels(xPropertySet->getPropertyValue("NumberingRules"), uno::UNO_QUERY);
    uno::Sequence<beans::PropertyValue> aProps;
    xLevels->getByIndex(0) >>= aProps; // 1st level
    bool bFoundIndentAt = false;
    for (int i = 0; i < aProps.getLength(); ++i)
    {
        const beans::PropertyValue& rProp = aProps[i];

        if (rProp.Name == "IndentAt")
        {
            CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("IndentAt", double(6001), rProp.Value.get<double>(), 10 );
            bFoundIndentAt = true;
        }
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("IndentAt defined", true, bFoundIndentAt);
}

DECLARE_OOXMLEXPORT_TEST(testTdf67207_MERGEFIELD, "mailmerge.docx")
{
    uno::Reference<beans::XPropertySet> xTextField = getProperty< uno::Reference<beans::XPropertySet> >(getRun(getParagraph(1), 2), "TextField");
    CPPUNIT_ASSERT(xTextField.is());
    uno::Reference<lang::XServiceInfo> xServiceInfo(xTextField, uno::UNO_QUERY_THROW);
    uno::Reference<text::XDependentTextField> xDependent(xTextField, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xServiceInfo->supportsService("com.sun.star.text.TextField.Database"));
    OUString sValue;
    xTextField->getPropertyValue("Content") >>= sValue;
    CPPUNIT_ASSERT_EQUAL(OUString::fromUtf8(u8"«Name»"), sValue);

    uno::Reference<beans::XPropertySet> xFiledMaster = xDependent->getTextFieldMaster();
    uno::Reference<lang::XServiceInfo> xFiledMasterServiceInfo(xFiledMaster, uno::UNO_QUERY_THROW);

    CPPUNIT_ASSERT(xFiledMasterServiceInfo->supportsService("com.sun.star.text.fieldmaster.Database"));

    // Defined properties: DataBaseName, Name, DataTableName, DataColumnName, DependentTextFields, DataCommandType, InstanceName, DataBaseURL
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("Name") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("DataColumnName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("Name"), sValue);
    CPPUNIT_ASSERT(xFiledMaster->getPropertyValue("InstanceName") >>= sValue);
    CPPUNIT_ASSERT_EQUAL(OUString("com.sun.star.text.fieldmaster.DataBase.Name"), sValue);
}

DECLARE_OOXMLEXPORT_TEST(testTdf44832_testSectionWithDifferentHeader, "tdf44832_section_new_header.odt")
{
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");

    if(!pXmlDoc)
        return;

    assertXPath(pXmlDoc, "/w:document/w:body/w:sectPr/w:headerReference", 1);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
