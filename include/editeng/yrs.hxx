/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

extern "C" {
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextern-c-compat"
#endif
#include <libyrs.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
}

#include <rtl/string.hxx>

// check input is valid values to find encoding bugs early
#define yvalidate(cond)                                                                            \
    if (!(cond))                                                                                   \
    {                                                                                              \
        std::abort();                                                                              \
    }

struct YOutputDeleter
{
    void operator()(YOutput* const p) const { youtput_destroy(p); }
};

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
    //    ::std::unique_ptr<YTransaction, ytransaction_commit> GetReadTransaction();
    //    ::std::unique_ptr<YTransaction, ytransaction_commit> GetWriteTransaction();
    virtual YTransaction* GetReadTransaction() = 0;
    virtual YTransaction* GetWriteTransaction() = 0;
    virtual bool CommitTransaction(bool isForce = false) = 0;
    virtual OString GenNewCommentId() = 0;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
