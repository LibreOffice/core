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

    explicit Impl( ScDocument& rDoc ) :
        mrDoc(rDoc),
        mpSpans(nullptr) {}
};

BulkDataHint::BulkDataHint( ScDocument& rDoc, SfxHintId nHintId ) :
    SfxHint( nHintId ), mpImpl(new Impl(rDoc)) {}

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
