/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_FORMULAGROUP_HXX
#define SC_FORMULAGROUP_HXX

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace sc {

struct FormulaGroupContext : boost::noncopyable
{
    typedef std::vector<double> DoubleArrayType;
    typedef boost::ptr_vector<DoubleArrayType> ArrayStoreType;

    ArrayStoreType maArrays;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
