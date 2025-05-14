/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <rtl/string.hxx>

typedef struct TransactionInner YTransaction;
typedef struct Branch Branch;
typedef struct YDoc YDoc;

class IYrsTransactionSupplier
{
public:
    enum class Mode
    {
        Edit,
        Replay
    };

protected:
    Mode m_Mode{ Mode::Edit };

public:
    IYrsTransactionSupplier() = default;
    virtual ~IYrsTransactionSupplier() = default;

    Mode SetMode(Mode const mode)
    {
        Mode ret = mode;
        std::swap(ret, m_Mode);
        return ret;
    }

    virtual YDoc* GetYDoc() = 0;
    virtual Branch* GetCommentMap() = 0;
    virtual Branch* GetCursorMap() = 0;
    virtual YTransaction* GetReadTransaction() = 0;
    virtual YTransaction* GetWriteTransaction() = 0;
    virtual bool CommitTransaction(bool isForce = false) = 0;
    virtual OString GenNewCommentId() = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
