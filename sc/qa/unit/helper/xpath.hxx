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

#include <libxml/xpathInternals.h>
#include <libxml/parserInternals.h>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

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
     */
    SCQAHELPER_DLLPUBLIC xmlDocPtr parseExport(ScDocShell* pShell, uno::Reference< lang::XMultiServiceFactory> xSFactory,
            const OUString& rFile, sal_Int32 nFormat);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
