/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>
#include <test/testdllapi.hxx>

#include <libxml/HTMLparser.h>
#include <libxml/HTMLtree.h>

#include <unotools/tempfile.hxx>

class OOO_DLLPUBLIC_TEST HtmlTestTools
{
protected:
    static htmlDocPtr parseHtml(utl::TempFile const & aTempFile);
    static htmlDocPtr parseHtmlStream(SvStream* pStream);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
