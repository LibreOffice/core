/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_BASEBMP_ITERATORTRAITS_HXX
#define INCLUDED_BASEBMP_ITERATORTRAITS_HXX

#include <basebmp/accessor.hxx>
#include <basebmp/nonstandarditerator.hxx>

namespace basebmp
{

template< class Iterator > struct IteratorTraits
{
    /// VigraTrueType, if iterator does not provide *operator()/operator[] methods
    typedef typename vigra::IsDerivedFrom<Iterator,NonStandardIterator>::result
            isNonStandardIterator;

    /// Retrieve default accessor for this iterator (and given value type)
    template< typename ValueType > struct defaultAccessor : public
        // select according to non-standardness of iterator type
        vigra::If< isNonStandardIterator,
            NonStandardAccessor< ValueType >,
            StandardAccessor< ValueType > >
    {};

};

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_ITERATORTRAITS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
