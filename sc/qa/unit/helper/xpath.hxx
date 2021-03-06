/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <libxml/parser.h>

#include <rtl/ustring.hxx>
#include <sal/types.h>
#include "scqahelperdllapi.h"
#include <test/xmltesttools.hxx>

#include <memory>

namespace com::sun::star::uno { template <class interface_type> class Reference; }
namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace utl { class TempFile; }
class ScDocShell;
class SvStream;

using namespace com::sun::star;

class ScBootstrapFixture;

namespace XPathHelper
{
    /**
     * Given that some problem doesn't affect the result in the importer, we
     * test the resulting file directly, by opening the zip file, parsing an
     * xml stream, and asserting an XPath expression. This method returns the
     * xml stream, so that you can do the asserting.
     *
     * Warning: This method saves the document and does not export it! If you need to
     *      test several files in the same exported xml file you need to export the file manually
     *      and call the parseExport method that takes a TempFile
     */
    SCQAHELPER_DLLPUBLIC xmlDocUniquePtr parseExport2(ScBootstrapFixture &, ScDocShell& rShell, uno::Reference< lang::XMultiServiceFactory> const & xSFactory,
            const OUString& rFile, sal_Int32 nFormat);

    /**
     * Tries to parse the specified file in the temp file zip container as a binary file.
     */
    SCQAHELPER_DLLPUBLIC std::unique_ptr<SvStream> parseExportStream(std::shared_ptr<utl::TempFile> const & pTempFile,
            uno::Reference<lang::XMultiServiceFactory> const & xSFactory, const OUString& rFile);

    /**
     * Tries to parse the specified file in the temp file zip container as an xml file.
     *
     * Should be used when the same exported file is used for testing different files in
     * the same zip file.
     */
    SCQAHELPER_DLLPUBLIC xmlDocUniquePtr parseExport(std::shared_ptr<utl::TempFile> const & pTempFile, uno::Reference< lang::XMultiServiceFactory> const & xSFactory,
            const OUString& rFile);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
