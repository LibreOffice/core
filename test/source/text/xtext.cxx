/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test/text/xtext.hxx"

using namespace css;
using namespace css::uno;

namespace apitest {

XText::~XText() {}

void XText::testInsertRemoveTextContent()
{
    uno::Reference<text::XText> xText(init(), UNO_QUERY_THROW);
    uno::Reference<text::XTextRange> xCursor(xText->createTextCursor(), UNO_QUERY_THROW);

    xText->insertTextContent(xCursor, getTextContent(), sal_False);
    xText->removeTextContent(getTextContent());
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
