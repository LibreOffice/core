/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <vcl/transfer.hxx>
#include <svtools/stringtransfer.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>

#include <doc.hxx>
#include <docsh.hxx>
#include <flyenum.hxx>
#include <swdtflvr.hxx>
#include <wrtsh.hxx>
#include <view.hxx>
#include <fmtanchr.hxx>
#include <fmtinfmt.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>

using namespace css;
using namespace css::uno;

/// Covers sw/source/uibase/dochdl/ fixes.
class SwUibaseDochdlTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testSelectPasteFormat)
{
    // Create a new document and cut a character.
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert2(u"x"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    rtl::Reference<SwTransferable> pTransfer = new SwTransferable(*pWrtShell);
    pTransfer->Cut();

    // Decide what format to use when doing a Writer->Writer paste and both RTF and ODF is an
    // available format.
    TransferableDataHelper aHelper(pTransfer);
    sal_uInt8 nAction = EXCHG_OUT_ACTION_INSERT_STRING;
    SotClipboardFormatId nFormat = SotClipboardFormatId::RICHTEXT;
    SwTransferable::SelectPasteFormat(aHelper, nAction, nFormat);

    CPPUNIT_ASSERT_EQUAL(EXCHG_OUT_ACTION_INSERT_OLE, nAction);
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 85 (EMBED_SOURCE)
    // - Actual  : 145 (RICHTEXT)
    // i.e. RTF was selected for Writer->Writer out of process copying, which is worse than ODF.
    CPPUNIT_ASSERT_EQUAL(SotClipboardFormatId::EMBED_SOURCE, nFormat);
}

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testComplexSelection)
{
    // Given a document where a text node has hints, but no as-char images.
    createSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    pWrtShell->Insert2(u"abc"_ustr);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 1, /*bBasicCall=*/false);
    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/true, 1, /*bBasicCall=*/false);
    SfxItemSet aSet(pWrtShell->GetView().GetPool(),
                    svl::Items<RES_CHRATR_BEGIN, RES_CHRATR_END - 1>);
    // Bold, italic, underline.
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, RES_CHRATR_WEIGHT));
    aSet.Put(SvxPostureItem(ITALIC_NORMAL, RES_CHRATR_POSTURE));
    aSet.Put(SvxUnderlineItem(LINESTYLE_SINGLE, RES_CHRATR_UNDERLINE));
    pWrtShell->SetAttrSet(aSet);
    uno::Reference<datatransfer::XTransferable2> xTransfer = new SwTransferable(*pWrtShell);

    // When checking if the selection is complex, then there should be no crash.
    // Without the accompanying fix in place, this test would have crashed, because we read past the
    // end of the hints array.
    CPPUNIT_ASSERT(!xTransfer->isComplex());
}

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testComplexSelectionAtChar)
{
    // Given a document with an at-char anchored image:
    createSwDoc();
    SwDoc* pDoc = getSwDoc();
    SwDocShell* pDocShell = getSwDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    SfxItemSet aFrameSet(pDoc->GetAttrPool(), svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>);
    SwFormatAnchor aAnchor(RndStdIds::FLY_AT_CHAR);
    aFrameSet.Put(aAnchor);
    Graphic aGrf;
    pWrtShell->SwFEShell::Insert(OUString(), OUString(), &aGrf, &aFrameSet);
    pWrtShell->UnSelectFrame();

    // When checking if the selection is simple or complex:
    pWrtShell->SelAll();
    uno::Reference<datatransfer::XTransferable2> xTransfer = new SwTransferable(*pWrtShell);
    bool bComplex = xTransfer->isComplex();

    // Then make sure it's complex:
    // Without the accompanying fix in place, this test would have failed, a selection containing an
    // image was considered simple.
    CPPUNIT_ASSERT(bComplex);
}

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testPasteURLOverSelection)
{
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    pWrtShell->Insert2(u"Click here for details"_ustr);

    pWrtShell->Left(SwCursorSkipMode::Chars, /*bSelect=*/false, 16, /*bBasicCall=*/false);
    pWrtShell->Right(SwCursorSkipMode::Chars, /*bSelect=*/true, 4, /*bBasicCall=*/false);

    OUString aURL(u"https://www.example.com"_ustr);
    rtl::Reference<svt::OStringTransferable> xTransferable(new svt::OStringTransferable(aURL));
    TransferableDataHelper aHelper(xTransferable);
    SwTransferable::Paste(*pWrtShell, aHelper);

    SwTextNode* pTextNode = pWrtShell->GetCursor()->GetPointNode().GetTextNode();
    CPPUNIT_ASSERT(pTextNode);
    CPPUNIT_ASSERT_EQUAL(u"Click here for details"_ustr, pTextNode->GetText());

    SwTextAttr* pAttr = pTextNode->GetTextAttrAt(6, RES_TXTATR_INETFMT);
    CPPUNIT_ASSERT(pAttr);

    auto* pINetFormat = static_cast<const SwFormatINetFormat*>(&pAttr->GetAttr());
    CPPUNIT_ASSERT_EQUAL(aURL, pINetFormat->GetValue());
}

namespace
{
/// Minimal XTransferable that exposes exactly one flavor: SVG.
class SvgTransferable : public cppu::WeakImplHelper<datatransfer::XTransferable>
{
    OString m_aSvg;
    OUString m_aMime{ u"image/svg+xml;windows_formatname=\"image/svg+xml\""_ustr };

public:
    SvgTransferable(OString aSvg)
        : m_aSvg(std::move(aSvg))
    {
    }

    cpo::uno::Any getTransferData(const datatransfer::DataFlavor& rFlavor) override
    {
        if (rFlavor.MimeType != m_aMime)
            throw datatransfer::UnsupportedFlavorException();
        cpo::uno::Sequence<sal_Int8> aData(
            reinterpret_cast<const sal_Int8*>(m_aSvg.getStr()), m_aSvg.getLength());
        return cpo::uno::Any(aData);
    }

    cpo::uno::Sequence<datatransfer::DataFlavor> getTransferDataFlavors() override
    {
        datatransfer::DataFlavor aFlavor;
        aFlavor.MimeType = m_aMime;
        aFlavor.HumanPresentableName = u"SVG"_ustr;
        aFlavor.DataType = cppu::UnoType<cpo::uno::Sequence<sal_Int8>>::get();
        return { aFlavor };
    }

    bool isDataFlavorSupported(const datatransfer::DataFlavor& rFlavor) override
    {
        return rFlavor.MimeType == m_aMime;
    }
};
}

CPPUNIT_TEST_FIXTURE(SwUibaseDochdlTest, testPasteSvg)
{
    // Given a Writer document and a transferable that exposes only SVG:
    createSwDoc();
    SwWrtShell* pWrtShell = getSwDocShell()->GetWrtShell();
    OString aSvg(
        R"(<svg xmlns="http://www.w3.org/2000/svg" width="10" height="10">)"
        R"(<rect width="10" height="10" fill="red"/></svg>)"_ostr);
    rtl::Reference<SvgTransferable> xTransferable(new SvgTransferable(aSvg));
    TransferableDataHelper aHelper(xTransferable);

    // When pasting:
    SwTransferable::Paste(*pWrtShell, aHelper);

    // Then a graphic must land in the document:
    SwDoc* pDoc = pWrtShell->GetDoc();
    // Without the accompanying fix in place, this test would have failed with:
    // - Expected: 1
    // - Actual  : 0
    // i.e. the SVG was not pasted as an image.
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), pDoc->GetFlyCount(FLYCNTTYPE_GRF));
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
