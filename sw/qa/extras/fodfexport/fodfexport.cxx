/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <swmodeltestbase.hxx>
#include <config_features.h>

#include <initializer_list>

#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/XGraphicExportFilter.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/table/XCellRange.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <officecfg/Office/Common.hxx>
#include <com/sun/star/document/XEmbeddedObjectSupplier.hpp>
#include <com/sun/star/text/XTextEmbeddedObjectsSupplier.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <comphelper/storagehelper.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/propertysequence.hxx>
#include <unotools/streamwrap.hxx>

class Test : public SwModelTestBase
{
public:
    Test() : SwModelTestBase("/sw/qa/extras/fodfexport/data/", "OpenDocument Text Flat XML") {}

    /**
     * Blacklist handling
     */
    bool mustTestImportOf(const char* filename) const override {
        // Only test import of .fodt document
        return OString(filename).endsWith(".odt") || OString(filename).endsWith(".fodt");
    }

    bool mustValidate(const char* filename) const override
    {
        std::vector<const char*> aBlacklist = {
            // These are known problems, they should be fixed one by one.
        };

        return std::find(aBlacklist.begin(), aBlacklist.end(), filename) == aBlacklist.end();
    }

    virtual std::unique_ptr<Resetter> preTest(const char* /* pFilename */) override
    {
        return nullptr;
    }
};

DECLARE_FODFEXPORT_TEST(testTdf113696, "tdf113696.odt")
{
    // Test that the image is written as svg, not as internal svm format
    if (xmlDocPtr pXmlDoc = parseExportedFile())
    {
        assertXPath(pXmlDoc, "/office:document-content/office:body/office:text/text:p/draw:frame/draw:image[@loext:mime-type='image/svg+xml']");
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
