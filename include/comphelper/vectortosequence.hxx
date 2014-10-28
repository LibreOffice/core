/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_CONVERTSEQUENCE
#define INCLUDED_COMPHELPER_CONVERTSEQUENCE

#include <vector>
#include <com/sun/star/uno/Sequence.hxx>

namespace comphelper {

/**
 * Converts a Sequence to a std::vector, each element corresponding to one of
 * the other.
 * @param seq The sequence to be converted.
 * @return Resulting vector from seq.
 */
template<typename T>
std::vector<T>
                toVector(const com::sun::star::uno::Sequence<T>& seq)
{
    std::vector<T> vec;
    for(sal_Int32 i=0; i<seq.getLength(); ++i)
        vec.push_back(seq[i]);
    return vec;
}

/**
 * Converts an std::vector to an uno::Sequence, each element corresponding to
 * one of the other.
 * @param vec The std::vector to be converted.
 * @return Resulting uno::Sequence from vec.
 */
template<typename T>
com::sun::star::uno::Sequence<T>
                toSequence(const std::vector<T>& vec)
{
    com::sun::star::uno::Sequence<T> seq(static_cast<sal_Int32>(vec.size()));
    for (size_t i=0; i<vec.size(); ++i)
        seq[static_cast<sal_Int32>(i)] = vec[i];
    return seq;
}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
