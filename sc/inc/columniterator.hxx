/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_COLUMNITERATOR_HXX__
#define __SC_COLUMNITERATOR_HXX__

#include <boost/noncopyable.hpp>

#include "column.hxx"

class ScColumn;

class ScColumnTextWidthIterator : boost::noncopyable
{
    typedef ScColumn::TextWidthType TextWidthType;

    TextWidthType& mrTextWidths;
    const size_t mnEnd;
    size_t mnCurPos;
    TextWidthType::iterator miBlockCur;
    TextWidthType::iterator miBlockEnd;
    mdds::mtv::ushort_element_block::iterator miDataCur;
    mdds::mtv::ushort_element_block::iterator miDataEnd;

public:
    ScColumnTextWidthIterator(ScColumn& rCol, SCROW nStartRow, SCROW nEndRow);

    void next();
    bool hasCell() const;
    SCROW getPos() const;
    sal_uInt16 getValue() const;
    void setValue(sal_uInt16 nVal);

private:
    void getDataIterators(size_t nOffsetInBlock);
    void checkEndRow();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
