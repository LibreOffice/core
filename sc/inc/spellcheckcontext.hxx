/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <i18nlangtag/lang.h>
#include <editeng/misspellrange.hxx>
#include "types.hxx"

#include <memory>
#include <vector>

class ScDocument;
class ScTabEditEngine;

namespace sc
{
/**
 * Class shared between grid windows to cache
 * spelling results.
 */
class SpellCheckContext
{
    class SpellCheckCache;
    struct SpellCheckStatus;
    struct SpellCheckResult;

    std::unique_ptr<SpellCheckCache> mpCache;
    std::unique_ptr<SpellCheckResult> mpResult;
    ScDocument* pDoc;
    std::unique_ptr<ScTabEditEngine> mpEngine;
    std::unique_ptr<SpellCheckStatus> mpStatus;
    SCTAB mnTab;
    LanguageType meLanguage;

public:
    SpellCheckContext(ScDocument* pDocument, SCTAB nTab);
    ~SpellCheckContext();
    void dispose();

    bool isMisspelled(SCCOL nCol, SCROW nRow) const;
    const std::vector<editeng::MisspellRanges>* getMisspellRanges(SCCOL nCol, SCROW nRow) const;
    void setMisspellRanges(SCCOL nCol, SCROW nRow,
                           const std::vector<editeng::MisspellRanges>* pRanges);

    void reset();
    void resetForContentChange();
    void setTabNo(SCTAB nTab);

private:
    void ensureResults(SCCOL nCol, SCROW nRow);
    void resetCache(bool bContentChangeOnly = false);
    void setup();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
