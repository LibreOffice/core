/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"
#include <test/unoapi_test.hxx>
#include <tools/color.hxx>
#include <tools/stream.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/numitem.hxx>
#include <docmodel/uno/UnoGradientTools.hxx>
#include <officecfg/Office/Common.hxx>
#include <test/commontesttools.hxx>

#include <svx/xlineit0.hxx>
#include <svx/xlndsit.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdotable.hxx>
#include <svx/unoapi.hxx>
#include <unotools/tempfile.hxx>
#include <vcl/filter/PngImageReader.hxx>
#include <vcl/settings.hxx>
#include <vcl/themecolors.hxx>
#include <xmloff/autolayout.hxx>

#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <sdpage.hxx>
#include <SlideSectionManager.hxx>
#include <unomodel.hxx>

using namespace css;
using namespace css::uno;

class SdOOXMLExportTest6 : public SdModelTestBase
{
public:
    SdOOXMLExportTest6()
        : SdModelTestBase(u"/sd/qa/unit/data/"_ustr)
    {
    }
};

// A presentation written under a dark appearance keeps automatic text readable: the saved colour
// is decided by the page background, not by the colour the application paints behind the page.
CPPUNIT_TEST_FIXTURE(SdOOXMLExportTest6, testAutomaticTextColorFollowsPageBackground)
{
    // The view takes the document background colour when it is created, so the dark appearance
    // has to be in place before the document is loaded.
    const AppearanceMode eOldMode = MiscSettings::GetAppColorMode();
    MiscSettings::SetAppColorMode(AppearanceMode::DARK);
    comphelper::ScopeGuard aResetMode([eOldMode] { MiscSettings::SetAppColorMode(eOldMode); });

    createSdImpressDoc("odp/automatic-text-color.fodp");
    save(TestFilter::PPTX);

    // The page carries no fill of its own, so its background is light and the text stays black.
    xmlDocUniquePtr pXmlDoc = parseExport(u"ppt/slides/slide1.xml"_ustr);
    assertXPath(pXmlDoc, "/p:sld/p:cSld/p:spTree/p:sp/p:txBody/a:p/a:r/a:rPr/a:solidFill/a:srgbClr",
                "val", u"000000");

    // Text that takes its colour from the master stays readable as well: the default run
    // properties the master carries hold no white text colour.
    xmlDocUniquePtr pXmlMaster = parseExport(u"ppt/slideMasters/slideMaster1.xml"_ustr);
    assertXPath(pXmlMaster, "//a:defRPr/a:solidFill/a:srgbClr[@val='FFFFFF']", 0);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
