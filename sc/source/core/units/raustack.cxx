/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */
#include "raustack.hxx"

using namespace sc::units;

RangeListIterator::RangeListIterator(ScDocument* pDoc, const ScRangeList& rRangeList)
    :
    mRangeList(rRangeList),
    mpDoc(pDoc),
    mIt(pDoc, ScRange()),
    nCurrentIndex(0)
{
}

bool RangeListIterator::first() {
    if (mRangeList.size() > 0) {
        mIt = ScCellIterator(mpDoc, *mRangeList[0]);
        return mIt.first();
    } else {
        return false;
    }
}

const ScAddress& RangeListIterator::GetPos() const {
    return mIt.GetPos();
}

bool RangeListIterator::next() {
    if (!(mRangeList.size() > 0) || nCurrentIndex >= mRangeList.size()) {
        return false;
    }

    if (mIt.next()) {
        return true;
    } else if (++nCurrentIndex < mRangeList.size()) {
        mIt = ScCellIterator(mpDoc, *mRangeList[nCurrentIndex]);
        mIt.first();
        // TODO: if emtpy - skip to next...?
        return true;
    } else {
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

