/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "document.hxx"

namespace sc
{
/**
 * Pushes a matrix-resize guard for the lifetime of this object, then drains
 * any pending resizes queued during the guarded iteration when we go out of
 * scope.
 *
 * Use at every scope that iterates the cell store while Interpret() may
 * run on visited cells.
 */
class MatrixResizeGuard
{
    ScDocument& mrDoc;

public:
    explicit MatrixResizeGuard(ScDocument& rDoc)
        : mrDoc(rDoc)
    {
        mrDoc.PushMatrixResizeGuard();
    }

    ~MatrixResizeGuard()
    {
        mrDoc.PopMatrixResizeGuard();
        if (mrDoc.HasPendingMatrixResizes() && !mrDoc.IsMatrixResizeGuarded())
            mrDoc.ProcessPendingMatrixResizes();
    }

    MatrixResizeGuard(const MatrixResizeGuard&) = delete;
    MatrixResizeGuard& operator=(const MatrixResizeGuard&) = delete;
};

} // namespace sc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
