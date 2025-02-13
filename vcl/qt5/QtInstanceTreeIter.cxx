/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceTreeIter.hxx>

QtInstanceTreeIter::QtInstanceTreeIter(QModelIndex aModelIndex)
    : m_aModelIndex(aModelIndex)
{
}

bool QtInstanceTreeIter::equal(const TreeIter& rOther) const
{
    return m_aModelIndex == static_cast<const QtInstanceTreeIter&>(rOther).m_aModelIndex;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
