/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <QtGui/QStandardItemModel>
#include <vcl/weld.hxx>

struct QtInstanceTreeIter final : public weld::TreeIter
{
    QModelIndex m_aModelIndex;

    explicit QtInstanceTreeIter(QModelIndex aModelIndex);
    virtual bool equal(const TreeIter& rOther) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
