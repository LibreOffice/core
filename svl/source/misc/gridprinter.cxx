/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/gridprinter.hxx>
#include <rtl/ustrbuf.hxx>

#include <mdds/multi_type_vector_types.hpp>
#include <mdds/multi_type_vector_trait.hpp>
#include <mdds/multi_type_vector_custom_func1.hpp>
#include <mdds/multi_type_matrix.hpp>

#include <iostream>

using namespace std;

namespace svl {

// String ID
const mdds::mtv::element_t element_type_string = mdds::mtv::element_type_user_start;
// String block
typedef mdds::mtv::default_element_block<element_type_string, OUString> string_block;

struct custom_string_trait
{
    typedef OUString string_type;
    typedef string_block string_element_block;

    static const mdds::mtv::element_t string_type_identifier = element_type_string;

    typedef mdds::mtv::custom_block_func1<string_block> element_block_func;
};

}

namespace rtl {

// Callbacks for the string block. This needs to be in the same namespace as
// OUString for argument dependent lookup.
MDDS_MTV_DEFINE_ELEMENT_CALLBACKS(OUString, svl::element_type_string, OUString(), svl::string_block)

}

namespace svl {

typedef mdds::multi_type_matrix<custom_string_trait> MatrixImplType;

struct GridPrinter::Impl
{
    MatrixImplType maMatrix;
    bool mbPrint;

    Impl( size_t nRows, size_t nCols, bool bPrint ) :
        maMatrix(nRows, nCols, OUString()), mbPrint(bPrint) {}
};

GridPrinter::GridPrinter( size_t nRows, size_t nCols, bool bPrint ) :
    mpImpl(new Impl(nRows, nCols, bPrint)) {}

GridPrinter::~GridPrinter()
{
}

void GridPrinter::set( size_t nRow, size_t nCol, const OUString& rStr )
{
    mpImpl->maMatrix.set(nRow, nCol, rStr);
}

void GridPrinter::print( const char* pHeader ) const
{
    if (!mpImpl->mbPrint)
        return;

    if (pHeader)
        cout << pHeader << endl;

    MatrixImplType::size_pair_type ns = mpImpl->maMatrix.size();
    vector<sal_Int32> aColWidths(ns.column, 0);

    // Calculate column widths first.
    for (size_t row = 0; row < ns.row; ++row)
    {
        for (size_t col = 0; col < ns.column; ++col)
        {
            OUString aStr = mpImpl->maMatrix.get_string(row, col);
            if (aColWidths[col] < aStr.getLength())
                aColWidths[col] = aStr.getLength();
        }
    }

    // Make the row separator string.
    OUStringBuffer aBuf;
    aBuf.append("+");
    for (size_t col = 0; col < ns.column; ++col)
    {
        aBuf.append("-");
        for (sal_Int32 i = 0; i < aColWidths[col]; ++i)
            aBuf.append(sal_Unicode('-'));
        aBuf.append("-+");
    }

    OUString aSep = aBuf.makeStringAndClear();

    // Now print to stdout.
    cout << aSep << endl;
    for (size_t row = 0; row < ns.row; ++row)
    {
        cout << "| ";
        for (size_t col = 0; col < ns.column; ++col)
        {
            OUString aStr = mpImpl->maMatrix.get_string(row, col);
            size_t nPadding = aColWidths[col] - aStr.getLength();
            aBuf.append(aStr);
            for (size_t i = 0; i < nPadding; ++i)
                aBuf.append(sal_Unicode(' '));
            cout << aBuf.makeStringAndClear() << " | ";
        }
        cout << endl;
        cout << aSep << endl;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
