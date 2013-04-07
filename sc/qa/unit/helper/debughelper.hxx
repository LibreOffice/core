/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * [ Copyright (C) 2011 Markus Mohrhard <markus.mohrhard@googlemail.com> (initial developer) ]
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef SC_DEBUG_HELPER_HXX
#define SC_DEBUG_HELPER_HXX

/**
 * Print nicely formatted sheet content to stdout.  Indispensable when
 * debugging the unit test code involving testing of sheet contents.
 */

#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

#ifdef WNT
#if !defined NOMINMAX
#define NOMINMAX
#endif
#include <prewin.h>
#include <postwin.h>
#undef NOMINMAX
#endif

#define MDDS_HASH_CONTAINER_BOOST 1
#include <mdds/mixed_type_matrix.hpp>

#include <iostream>

using namespace ::com::sun::star;
using ::std::cout;
using ::std::cerr;
using ::std::endl;
using ::std::vector;


class SheetPrinter
{
    typedef ::mdds::mixed_type_matrix<OUString, bool> MatrixType;
public:
    SheetPrinter(size_t rows, size_t cols) :
        maMatrix(rows, cols, ::mdds::matrix_density_sparse_empty) {}

    void set(size_t row, size_t col, const OUString& aStr)
    {
        maMatrix.set_string(row, col, new OUString(aStr));
    }

#if CALC_DEBUG_OUTPUT
    void print(const char* header) const
    {
        if (header)
            cout << header << endl;

        MatrixType::size_pair_type ns = maMatrix.size();
        vector<sal_Int32> aColWidths(ns.second, 0);

        // Calculate column widths first.
        for (size_t row = 0; row < ns.first; ++row)
        {
            for (size_t col = 0; col < ns.second; ++col)
            {
                const OUString* p = maMatrix.get_string(row, col);
                if (aColWidths[col] < p->getLength())
                    aColWidths[col] = p->getLength();
            }
        }

        // Make the row separator string.
        OUStringBuffer aBuf;
        aBuf.appendAscii("+");
        for (size_t col = 0; col < ns.second; ++col)
        {
            aBuf.appendAscii("-");
            for (sal_Int32 i = 0; i < aColWidths[col]; ++i)
                aBuf.append(sal_Unicode('-'));
            aBuf.appendAscii("-+");
        }

        OUString aSep = aBuf.makeStringAndClear();

        // Now print to stdout.
        cout << aSep << endl;
        for (size_t row = 0; row < ns.first; ++row)
        {
            cout << "| ";
            for (size_t col = 0; col < ns.second; ++col)
            {
                const OUString* p = maMatrix.get_string(row, col);
                size_t nPadding = aColWidths[col] - p->getLength();
                aBuf.append(*p);
                for (size_t i = 0; i < nPadding; ++i)
                    aBuf.append(sal_Unicode(' '));
                cout << aBuf.makeStringAndClear() << " | ";
            }
            cout << endl;
            cout << aSep << endl;
        }
    }
#else
    void print(const char*) const {}
#endif

    /**
     * Print nested string array which can be copy-n-pasted into the test code
     * for content verification.
     */
    void printArray() const
    {
#if CALC_DEBUG_OUTPUT
        MatrixType::size_pair_type ns = maMatrix.size();
        for (size_t row = 0; row < ns.first; ++row)
        {
            cout << "    { ";
            for (size_t col = 0; col < ns.second; ++col)
            {
                const OUString* p = maMatrix.get_string(row, col);
                if (p->getLength())
                    cout << "\"" << *p << "\"";
                else
                    cout << "0";
                if (col < ns.second - 1)
                    cout << ", ";
            }
            cout << " }";
            if (row < ns.first - 1)
                cout << ",";
            cout << endl;
        }
#endif
    }

    void clear() { maMatrix.clear(); }
    void resize(size_t rows, size_t cols) { maMatrix.resize(rows, cols); }

private:
    MatrixType maMatrix;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
