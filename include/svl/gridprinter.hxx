/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVL_GRIDPRINTER_HXX
#define INCLUDED_SVL_GRIDPRINTER_HXX

#include <rtl/ustring.hxx>
#include <svl/svldllapi.h>
#include <memory>

namespace svl {

/**
 * Print 2-dimensional data in a nice and pleasant fashion.  Useful when
 * debugging grid layout data.
 */
class SVL_DLLPUBLIC GridPrinter
{
    struct Impl;
    std::unique_ptr<Impl> mpImpl;

public:
    GridPrinter( size_t nRows, size_t nCols, bool bPrint = true );
    ~GridPrinter();
    void set( size_t nRow, size_t nCol, const OUString& rStr );
    void print( const char* pHeader ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
