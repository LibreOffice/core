/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/text/WritingMode2.hpp>
#include <com/sun/star/text/TableColumnSeparator.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <oox/drawingml/drawingmltypes.hxx>
#include <config_features.h>
#include <string>
class Test : public SwModelTestBase
{
public:
    Test()
        : SwModelTestBase("/sw/qa/extras/ooxmlexport/data/", "Office Open XML Text")
    {
    }

protected:
    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override
    {
        // If the testcase is stored in some other format, it's pointless to test.
        return OString(filename).endsWith(".docx");
    }

protected:
};

DECLARE_OOXMLEXPORT_TEST(testDateControl, "date-control.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date", "fullDate",
                "2014-03-05T00:00:00Z");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date/w:dateFormat", "val",
                "dddd, dd' de 'MMMM' de 'yyyy");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:date/w:lid", "val", "es-ES");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t",
                       u"mi\u00E9rcoles, 05 de marzo de 2014");

    // check imported control
    uno::Reference<drawing::XControlShape> xControl(getShape(1), uno::UNO_QUERY);
    util::Date aDate = getProperty<util::Date>(xControl->getControl(), "Date");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5), sal_Int32(aDate.Day));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(3), sal_Int32(aDate.Month));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2014), sal_Int32(aDate.Year));
}

DECLARE_OOXMLEXPORT_TEST(testComboBoxControl, "combobox-control.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dropDownList/w:listItem[1]",
                "value", "manolo");
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtPr/w:dropDownList/w:listItem[2]",
                "value", "pepito");
    assertXPathContent(pXmlDoc, "/w:document/w:body/w:p/w:sdt/w:sdtContent/w:r/w:t", "Manolo");

    // check imported control
    uno::Reference<drawing::XControlShape> xControl(getShape(1), uno::UNO_QUERY);

    CPPUNIT_ASSERT_EQUAL(OUString("Manolo"), getProperty<OUString>(xControl->getControl(), "Text"));

    uno::Sequence<OUString> aItems
        = getProperty<uno::Sequence<OUString>>(xControl->getControl(), "StringItemList");
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), aItems.getLength());
    CPPUNIT_ASSERT_EQUAL(OUString("manolo"), aItems[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("pepito"), aItems[1]);
}

DECLARE_OOXMLEXPORT_TEST(testCheckBoxControl, "checkbox-control.docx")
{
    // check XML
    xmlDocPtr pXmlDoc = parseExport("word/document.xml");
    if (!pXmlDoc)
        return;
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:checked", "val", "1");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:checkedState", "val",
                "2612");
    assertXPath(pXmlDoc, "/w:document/w:body/w:sdt/w:sdtPr/w14:checkbox/w14:uncheckedState", "val",
                "2610");

    // TODO: import control and add a check here
}

DECLARE_OOXMLEXPORT_TEST(testN779630, "n779630.docx")
{
    // First shape: date picker
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.DateField")));
    CPPUNIT_ASSERT_EQUAL(OUString("date default text"),
                         getProperty<OUString>(xPropertySet, "HelpText"));
    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xPropertySet, "DateFormat"));
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPropertySet, "Dropdown"));

    // Second shape: combo box
    xControlShape.set(getShape(2), uno::UNO_QUERY);
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.ComboBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("dropdown default text"),
                         getProperty<OUString>(xPropertySet, "DefaultText"));
    CPPUNIT_ASSERT_EQUAL(
        sal_Int32(2),
        getProperty<uno::Sequence<OUString>>(xPropertySet, "StringItemList").getLength());
    CPPUNIT_ASSERT_EQUAL(true, getProperty<bool>(xPropertySet, "Dropdown"));
}

DECLARE_OOXMLEXPORT_TEST(testN820509, "n820509.docx")
{
    // Design mode was enabled.
    uno::Reference<frame::XModel> xModel(mxComponent, uno::UNO_QUERY);
    uno::Reference<view::XFormLayerAccess> xFormLayerAccess(xModel->getCurrentController(),
                                                            uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(false, bool(xFormLayerAccess->isFormDesignMode()));

    // M.d.yyyy date format was unhandled.
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int16(8), getProperty<sal_Int16>(xPropertySet, "DateFormat"));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXCheckbox, "activex_checkbox.docx")
{
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check control type
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));

    // Check custom label
    CPPUNIT_ASSERT_EQUAL(OUString("Custom Caption"), getProperty<OUString>(xPropertySet, "Label"));

    // Check background color (highlight system color)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0x316AC5),
                         getProperty<sal_Int32>(xPropertySet, "BackgroundColor"));

    // Check Text color (active border system color)
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0xD4D0C8), getProperty<sal_Int32>(xPropertySet, "TextColor"));

    // Check state of the checkbox
    CPPUNIT_ASSERT_EQUAL(sal_Int16(1), getProperty<sal_Int16>(xPropertySet, "State"));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xPropertySet2, "AnchorType"));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXControlAlign, "activex_control_align.odt")
{
    // First check box aligned as a floating object
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Floating Check Box"),
                         getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xPropertySet2, "AnchorType"));

    // Also check position and size
    uno::Reference<drawing::XShape> xShape(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4470), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1427), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(5126), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2341), xShape->getPosition().Y);

    // Second check box aligned inline / as character
    xControlShape.set(getShape(2), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Inline Check Box"),
                         getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xPropertySet2, "AnchorType"));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(text::VertOrientation::TOP),
                         getProperty<sal_Int32>(xPropertySet2, "VertOrient"));

    // Also check position and size
    xShape.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT(xShape.is());
    CPPUNIT_ASSERT_EQUAL(sal_Int32(4410), xShape->getSize().Width);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(1083), xShape->getSize().Height);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), xShape->getPosition().X);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-1085), xShape->getPosition().Y);

    // Also check the specific OOXML elements
    xmlDocPtr pXmlDoc = parseExport();
    CPPUNIT_ASSERT(pXmlDoc);
    // For inline controls use w:object as parent element and pictureFrame shapetype
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object/v:shapetype", "spt", "75");
    // For floating controls use w:pict as parent element and hostControl shapetype
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict", 1);
    assertXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict/v:shapetype", "spt", "201");

    // Have different shape ids
    CPPUNIT_ASSERT(getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r/w:object/v:shape", "id")
                   != getXPath(pXmlDoc, "/w:document/w:body/w:p/w:r[1]/w:pict/v:shape", "id"));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXControlAtRunEnd, "activex_control_at_run_end.odt")
{
    // Two issues were here:
    //  1) second shape was not export (it is anchored to the end of the run)
    //  2) inline property was inherited to the second shape by mistake

    // First checkbox is the inlined one
    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());

    // Check whether we have the right control
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    uno::Reference<lang::XServiceInfo> xServiceInfo(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Inline Checkbox"), getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    uno::Reference<beans::XPropertySet> xPropertySet2(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AS_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xPropertySet2, "AnchorType"));

    // Second check box anchored to character
    xControlShape.set(getShape(2), uno::UNO_QUERY);

    // Check whether we have the right control
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    xServiceInfo.set(xPropertySet, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(
        true, bool(xServiceInfo->supportsService("com.sun.star.form.component.CheckBox")));
    CPPUNIT_ASSERT_EQUAL(OUString("Floating Checkbox"),
                         getProperty<OUString>(xPropertySet, "Label"));

    // Check anchor type
    xPropertySet2.set(xControlShape, uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(text::TextContentAnchorType_AT_CHARACTER,
                         getProperty<text::TextContentAnchorType>(xPropertySet2, "AnchorType"));
}

DECLARE_OOXMLEXPORT_TEST(testActiveXOptionButtonGroup, "activex_option_button_group.docx")
{
    // Optionbutton groups were not handled
    // The two optionbutton should have the same group name
    const OUString sGroupName = "GroupX";

    uno::Reference<drawing::XControlShape> xControlShape(getShape(1), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());
    uno::Reference<beans::XPropertySet> xPropertySet(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sGroupName, getProperty<OUString>(xPropertySet, "GroupName"));

    xControlShape.set(getShape(2), uno::UNO_QUERY);
    CPPUNIT_ASSERT(xControlShape.is());
    xPropertySet.set(xControlShape->getControl(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sGroupName, getProperty<OUString>(xPropertySet, "GroupName"));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
