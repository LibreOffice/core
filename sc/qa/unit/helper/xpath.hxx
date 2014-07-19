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
#include "scdllapi.h"

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

    /**
     * Helper method to return nodes represented by rXPath.
     */
    SCQAHELPER_DLLPUBLIC xmlNodeSetPtr getXPathNode(xmlDocPtr pXmlDoc, const OString& rXPath);

    /**
     * Assert that rXPath exists, and returns exactly one node.
     * In case rAttribute is provided, the rXPath's attribute's value must
     * equal to the rExpected value.
     */
    SCQAHELPER_DLLPUBLIC void assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute = OString(), const OUString& rExpectedValue = OUString());

    /**
     * Assert that rXPath exists, and returns exactly nNumberOfNodes nodes.
     * Useful for checking that we do _not_ export some node (nNumberOfNodes == 0).
     */
    SCQAHELPER_DLLPUBLIC void assertXPath(xmlDocPtr pXmlDoc, const OString& rXPath, int nNumberOfNodes);

    /**
     * Same as the assertXPath(), but don't assert: return the string instead.
     */
    SCQAHELPER_DLLPUBLIC OUString getXPath(xmlDocPtr pXmlDoc, const OString& rXPath, const OString& rAttribute);
    /**
    Assert that rXPath exists, and its content equals rContent.
    */
    SCQAHELPER_DLLPUBLIC void assertXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath, const OUString& rContent);
    /**
    Same as the assertXPathContent(), but don't assert: return the string instead.
    */
    SCQAHELPER_DLLPUBLIC OUString getXPathContent(xmlDocPtr pXmlDoc, const OString& rXPath);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
