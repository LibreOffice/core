/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/GraphicType.hpp>
#include <vcl/GraphicNativeTransform.hxx>
#include <sfx2/linkmgr.hxx>

#include <docsh.hxx>
#include <editsh.hxx>
#include <ndgrf.hxx>

class HtmlImportTest : public SwModelTestBase
{
    public:
        HtmlImportTest() : SwModelTestBase("sw/qa/extras/htmlimport/data/", "HTML (StarWriter)") {}
};

#define DECLARE_HTMLIMPORT_TEST(TestName, filename) DECLARE_SW_IMPORT_TEST(TestName, filename, HtmlImportTest)

DECLARE_HTMLIMPORT_TEST(testPictureImport, "picture.html")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // The document contains two pictures stored as a link.
    sfx2::LinkManager& rLinkManager = pTextDoc->GetDocShell()->GetDoc()->GetEditShell()->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(2), rLinkManager.GetLinks().size());
    rLinkManager.Remove(0,2);
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    // TODO: Get the data into clipboard in html format and paste

    // But when pasting we don't want images to be linked.
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());
}

DECLARE_HTMLIMPORT_TEST(testInlinedImage, "inlined_image.html")
{
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument *>(mxComponent.get());
    CPPUNIT_ASSERT(pTextDoc);
    // The document contains only one embedded picture inlined in img's src attribute.

    SwDoc* pDoc = pTextDoc->GetDocShell()->GetDoc();
    SwEditShell* pEditShell = pDoc->GetEditShell();
    CPPUNIT_ASSERT(pEditShell);

    // This was 1 before 3914a711060341345f15b83656457f90095f32d6
    const sfx2::LinkManager& rLinkManager = pEditShell->GetLinkManager();
    CPPUNIT_ASSERT_EQUAL(size_t(0), rLinkManager.GetLinks().size());

    uno::Reference<drawing::XShape> xShape = getShape(1);
    uno::Reference<container::XNamed> const xNamed(xShape, uno::UNO_QUERY_THROW);
    CPPUNIT_ASSERT_EQUAL(OUString("Image1"), xNamed->getName());

    uno::Reference<graphic::XGraphic> xGraphic =
        getProperty< uno::Reference<graphic::XGraphic> >(xShape, "Graphic");
    CPPUNIT_ASSERT(xGraphic.is());
    CPPUNIT_ASSERT(xGraphic->getType() != graphic::GraphicType::EMPTY);

    OUString sGraphicURL = getProperty< OUString >(xShape, "GraphicURL");
    // Before it was "data:image/png;base64,<data>"
    CPPUNIT_ASSERT(sGraphicURL.startsWith("vnd.sun.star.GraphicObject:"));

    for (int n = 0; ; n++)
    {
        SwNode* pNode = pDoc->GetNodes()[ n ];
        if (SwGrfNode *pGrfNode = pNode->GetGrfNode())
        {
            // FIXME? For some reason without the fix in 72703173066a2db5c977d422ace
            // I was getting GRAPHIC_NONE from SwEditShell::GetGraphicType() when
            // running LibreOffice but cannot reproduce that in a unit test here. :-(
            // So, this does not really test anything.
            CPPUNIT_ASSERT(pGrfNode->GetGrfObj().GetType() != GRAPHIC_NONE);
            break;
        }
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
