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
#include <memory>

class ScDocument;

namespace sc
{
class ColumnSpanSet;

class BulkDataHint final : public SfxHint
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

    BulkDataHint(const BulkDataHint&) = delete;
    BulkDataHint& operator=(const BulkDataHint&) = delete;

public:
    BulkDataHint(ScDocument& rDoc, SfxHintId nHintId);
    virtual ~BulkDataHint() override;

    void setSpans(const ColumnSpanSet* pSpans);
    const ColumnSpanSet* getSpans() const;

    ScDocument& getDoc();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
