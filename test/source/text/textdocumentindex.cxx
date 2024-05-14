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
    css::uno::Reference<css::beans::XPropertySet> xDocumentIndex(init(), css::uno::UNO_QUERY_THROW);

    testBooleanProperty(xDocumentIndex, u"UseAlphabeticalSeparators"_ustr);
    testBooleanProperty(xDocumentIndex, u"UseKeyAsEntry"_ustr);
    testBooleanProperty(xDocumentIndex, u"UseCombinedEntries"_ustr);
    testBooleanProperty(xDocumentIndex, u"IsCaseSensitive"_ustr);
    testBooleanProperty(xDocumentIndex, u"UsePP"_ustr);
    testBooleanProperty(xDocumentIndex, u"UseDash"_ustr);
    testBooleanProperty(xDocumentIndex, u"UseUpperCase"_ustr);
    testStringOptionalProperty(xDocumentIndex, u"MainEntryCharacterStyleName"_ustr);
    //      [readonly, property] sequence <com::sun::star::text::XDocumentIndexMark> DocumentIndexMarks;
    //      [property] com::sun::star::lang::Locale  Locale;
    testStringProperty(xDocumentIndex, u"SortAlgorithm"_ustr, u"Value"_ustr);
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
