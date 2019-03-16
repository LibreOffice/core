/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/beans/XPropertySet.hpp>

#include <test/unoapi_property_testers.hxx>
#include <test/text/textdocumentindex.hxx>

namespace apitest
{
TextDocumentIndex::~TextDocumentIndex() {}

void TextDocumentIndex::testDocumentIndexProperties()
{
    css::uno::Reference<css::beans::XPropertySet> xDocumnetIndex(init(), css::uno::UNO_QUERY_THROW);

    testBooleanProperty(xDocumnetIndex, "UseAlphabeticalSeparators");
    testBooleanProperty(xDocumnetIndex, "UseKeyAsEntry");
    testBooleanProperty(xDocumnetIndex, "UseCombinedEntries");
    testBooleanProperty(xDocumnetIndex, "IsCaseSensitive");
    testBooleanProperty(xDocumnetIndex, "UsePP");
    testBooleanProperty(xDocumnetIndex, "UseDash");
    testBooleanProperty(xDocumnetIndex, "UseUpperCase");
    testStringOptionalProperty(xDocumnetIndex, "MainEntryCharacterStyleName");
    //      [readonly, property] sequence <com::sun::star::text::XDocumentIndexMark> DocumentIndexMarks;
    //      [property] com::sun::star::lang::Locale  Locale;
    testStringProperty(xDocumnetIndex, "SortAlgorithm", "Value");
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
