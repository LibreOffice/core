/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <svl/currencytable.hxx>

NfCurrencyTable::iterator NfCurrencyTable::begin()
{
    return maData.begin();
}

NfCurrencyEntry& NfCurrencyTable::operator[] ( size_t i )
{
    return *maData[i];
}

const NfCurrencyEntry& NfCurrencyTable::operator[] ( size_t i ) const
{
    return *maData[i];
}

size_t NfCurrencyTable::size() const
{
    return maData.size();
}

void NfCurrencyTable::insert(const iterator& it, std::unique_ptr<NfCurrencyEntry> p)
{
    maData.insert(it, std::move(p));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
