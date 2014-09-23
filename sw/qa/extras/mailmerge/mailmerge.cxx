/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#if !defined(MACOSX) && !defined(WNT)

#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/text/XTextTable.hpp>
#include <com/sun/star/text/MailMergeType.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <swdtflvr.hxx>
#include <view.hxx>
#include <edtwin.hxx>
#include <olmenu.hxx>
#include <cmdid.h>

class MMTest : public SwModelTestBase
{
    public:
        MMTest() : SwModelTestBase("/sw/qa/extras/mailmerge/data/", "writer8") {}
};

#define DECLARE_DFLT_MAILMERGE_TEST(TestName, filename, datasource, tablename) \
    DECLARE_MAILMERGE_TEST(TestName, filename, datasource, tablename, MMTest)

DECLARE_DFLT_MAILMERGE_TEST(testMultiPageAnchoredDraws, "multiple-page-anchored-draws.odt", "4_v01.ods", "Tabelle1")
{
    executeMailMerge();

    SwXTextDocument* pTxtDoc = dynamic_cast<SwXTextDocument *>(mxMMComponent.get());
    CPPUNIT_ASSERT(pTxtDoc);
    sal_uInt16 nPhysPages = pTxtDoc->GetDocShell()->GetWrtShell()->GetPhyPageNum();
    CPPUNIT_ASSERT_EQUAL(sal_uInt16(8), nPhysPages);

    uno::Reference<drawing::XDrawPageSupplier> xDrawPageSupplier(mxMMComponent, uno::UNO_QUERY);
    uno::Reference<container::XIndexAccess> xDraws(xDrawPageSupplier->getDrawPage(), uno::UNO_QUERY);
    CPPUNIT_ASSERT_EQUAL(sal_Int32(8), xDraws->getCount());

    // bitset of all page numbers
    char nPageSum = 0xFF;
    uno::Reference<beans::XPropertySet> xPropertySet;

    for (sal_Int32 i = 0; i < xDraws->getCount(); i++)
    {
        text::TextContentAnchorType nAnchorType;
        sal_uInt16 nAnchorPageNo;
        xPropertySet.set(xDraws->getByIndex(i), uno::UNO_QUERY);

        xPropertySet->getPropertyValue( UNO_NAME_ANCHOR_TYPE ) >>= nAnchorType;
        CPPUNIT_ASSERT_EQUAL( text::TextContentAnchorType_AT_PAGE, nAnchorType );

        xPropertySet->getPropertyValue( UNO_NAME_ANCHOR_PAGE_NO ) >>= nAnchorPageNo;
        nPageSum &= !nAnchorPageNo;
    }

    // are all shapes are on different page numbers?
    CPPUNIT_ASSERT_EQUAL(char(0), nPageSum);
}

#endif

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
