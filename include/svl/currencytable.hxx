/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_SVL_CURRENCYTABLE_HXX
#define INCLUDED_SVL_CURRENCYTABLE_HXX

#include <svl/svldllapi.h>
#include <svl/zforlist.hxx>

#include <vector>
#include <memory>

class SVL_DLLPUBLIC NfCurrencyTable
{
    typedef std::vector<std::unique_ptr<NfCurrencyEntry>> DataType;
    DataType maData;

    NfCurrencyTable(NfCurrencyTable const&) = delete;
    void operator=(NfCurrencyTable const&) = delete;

public:
    NfCurrencyTable() {}
    typedef DataType::iterator iterator;
    typedef DataType::const_iterator const_iterator;

    iterator begin();

    NfCurrencyEntry& operator[](size_t i);
    const NfCurrencyEntry& operator[](size_t i) const;

    size_t size() const;

    void insert(const iterator& it, std::unique_ptr<NfCurrencyEntry> p);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
