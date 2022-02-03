/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svl/hint.hxx>

class ScDocument;

namespace sc
{
class ColumnSpanSet;

class BulkDataHint final : public SfxHint
{
    ScDocument& mrDoc;
    const ColumnSpanSet* mpSpans;

public:
    BulkDataHint(ScDocument& rDoc, SfxHintId nHintId)
        : SfxHint(nHintId)
        , mrDoc(rDoc)
        , mpSpans(nullptr)
    {
    }

    void setSpans(const ColumnSpanSet* pSpans) { mpSpans = pSpans; }
    const ColumnSpanSet* getSpans() const { return mpSpans; }

    ScDocument& getDoc() { return mrDoc; }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
