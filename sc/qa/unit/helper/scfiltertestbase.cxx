/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "scfiltertestbase.hxx"

#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/intitem.hxx>

#include <com/sun/star/document/MacroExecMode.hpp>

using namespace com::sun::star;
using namespace ::com::sun::star::uno;

ScDocShellRef ScFilterTestBase::loadDoc(const OUString& rURL, const OUString& rFilter,
                                        const OUString& rUserData, const OUString& rTypeName,
                                        SfxFilterFlags nFilterFlags,
                                        SotClipboardFormatId nClipboardID, sal_Int32 nFilterVersion)
{
    auto pFilter
        = std::make_shared<SfxFilter>(rFilter, OUString(), nFilterFlags, nClipboardID, rTypeName,
                                      OUString(), rUserData, "private:factory/scalc");
    pFilter->SetVersion(nFilterVersion);

    ScDocShellRef xDocShRef = new ScDocShell;
    xDocShRef->GetDocument().EnableUserInteraction(false);
    SfxMedium* pSrcMed = new SfxMedium(rURL, StreamMode::STD_READ);
    pSrcMed->SetFilter(pFilter);
    pSrcMed->UseInteractionHandler(false);
    pSrcMed->GetItemSet().Put(
        SfxUInt16Item(SID_MACROEXECMODE, css::document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN));
    SAL_INFO("sc.qa", "about to load " << rURL);
    if (!xDocShRef->DoLoad(pSrcMed))
    {
        xDocShRef->DoClose();
        // load failed.
        xDocShRef.clear();
    }

    return xDocShRef;
}

void ScFilterTestBase::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent = getMultiServiceFactory()->createInstance(
        u"com.sun.star.comp.Calc.SpreadsheetDocument"_ustr);
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void ScFilterTestBase::tearDown()
{
    uno::Reference<lang::XComponent>(m_xCalcComponent, UNO_QUERY_THROW)->dispose();
    test::BootstrapFixture::tearDown();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
