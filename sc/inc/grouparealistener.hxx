/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "address.hxx"

#include <svl/listener.hxx>

#include <vector>

class ScFormulaCell;
class ScDocument;
class ScColumn;

namespace sc {

class BulkDataHint;

class FormulaGroupAreaListener final : public SvtListener
{
    ScRange maRange;
    const ScDocument& mrDocument;
    const ScColumn* mpColumn;
    SCROW mnTopCellRow;
    SCROW mnGroupLen;
    bool mbStartFixed;
    bool mbEndFixed;

public:

    FormulaGroupAreaListener() = delete;
    FormulaGroupAreaListener( const ScRange& rRange, const ScDocument& rDocument,
            const ScAddress& rTopCellPos, SCROW nGroupLen, bool bStartFixed, bool bEndFixed );

    virtual ~FormulaGroupAreaListener() override;

    ScRange getListeningRange() const;

    virtual void Notify( const SfxHint& rHint ) override;
    virtual void Query( QueryBase& rQuery ) const override;

    /**
     * Given the row span of changed cells within a single column, collect all
     * formula cells that need to be notified of the change.
     *
     * @param nTab sheet position of the changed cell span.
     * @param nCol column position of the changed cell span.
     * @param nRow1 top row position of the changed cell span.
     * @param nRow2 bottom row position of the changed cell span.
     * @param rCells all formula cells that need to be notified are put into
     *               this container.
     */
    void collectFormulaCells( SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2, std::vector<ScFormulaCell*>& rCells ) const;
    void collectFormulaCells( SCROW nRow1, SCROW nRow2, std::vector<ScFormulaCell*>& rCells ) const;

private:
    void notifyCellChange( const SfxHint& rHint, const ScAddress& rPos );
    void notifyBulkChange( const BulkDataHint& rHint );
    const ScFormulaCell* getTopCell() const;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
