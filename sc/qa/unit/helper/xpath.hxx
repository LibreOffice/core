/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_QA_UNIT_HELPER_XPATH_HXX
#define INCLUDED_SC_QA_UNIT_HELPER_XPATH_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <unotools/tempfile.hxx>

#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#include <memory>

#if defined(SCQAHELPER_DLLIMPLEMENTATION)
#define SCQAHELPER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define SCQAHELPER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#include "docsh.hxx"

using namespace com::sun::star;

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
    SCQAHELPER_DLLPUBLIC xmlDocPtr parseExport(ScDocShell* pShell, uno::Reference< lang::XMultiServiceFactory> xSFactory,
            const OUString& rFile, sal_Int32 nFormat);

    /**
     * Tries to parse the specified file in the temp file zip container as an xml file.
     *
     * Should be used when the same exported file is used for testing different files in
     * the same zip file.
     */
    SCQAHELPER_DLLPUBLIC xmlDocPtr parseExport(std::shared_ptr<utl::TempFile> const & pTempFile, uno::Reference< lang::XMultiServiceFactory> const & xSFactory,
            const OUString& rFile);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
