/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_HTMLTESTTOOLS_HXX
#define INCLUDED_TEST_HTMLTESTTOOLS_HXX

#include <sal/config.h>
#include <test/testdllapi.hxx>
#include <test/xmltesttools.hxx>

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

#include <unotools/tempfile.hxx>

using htmlDocUniquePtr = xmlDocUniquePtr;

class OOO_DLLPUBLIC_TEST HtmlTestTools
{
protected:
    static htmlDocUniquePtr parseHtml(utl::TempFile const& aTempFile);
    static htmlDocUniquePtr parseHtmlStream(SvStream* pStream);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
