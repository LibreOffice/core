/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef HTML_TEST_TOOLS_H
#define HTML_TEST_TOOLS_H

#include <sal/config.h>
#include <test/testdllapi.hxx>

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

#include <unotools/tempfile.hxx>
#include <boost/scoped_array.hpp>

class OOO_DLLPUBLIC_TEST HtmlTestTools
{
protected:
    htmlDocPtr parseHtml(utl::TempFile& aTempFile);
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
