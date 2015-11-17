/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <bulkdatahint.hxx>

namespace sc {

struct BulkDataHint::Impl
{
    ScDocument& mrDoc;
    const ColumnSpanSet* mpSpans;

    Impl( ScDocument& rDoc, const ColumnSpanSet* pSpans ) :
        mrDoc(rDoc),
        mpSpans(pSpans) {}
};

BulkDataHint::BulkDataHint( ScDocument& rDoc, const ColumnSpanSet* pSpans ) :
    SfxSimpleHint(SC_HINT_BULK_DATACHANGED), mpImpl(new Impl(rDoc, pSpans)) {}

BulkDataHint::~BulkDataHint()
{
}

void BulkDataHint::setSpans( const ColumnSpanSet* pSpans )
{
    mpImpl->mpSpans = pSpans;
}

const ColumnSpanSet* BulkDataHint::getSpans() const
{
    return mpImpl->mpSpans;
}

ScDocument& BulkDataHint::getDoc()
{
    return mpImpl->mrDoc;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
