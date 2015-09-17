/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_DOCUMENTSTREAMACCESS_HXX
#define INCLUDED_SC_INC_DOCUMENTSTREAMACCESS_HXX

#include <rtl/ustring.hxx>
#include <boost/scoped_ptr.hpp>

class ScDocument;
class ScAddress;
class ScRange;

namespace sc {

struct DocumentStreamAccessImpl;

/**
 * Provides methods to allow direct shifting of document content without
 * broadcasting or shifting of broadcaster positions.
 */
class DocumentStreamAccess
{
    boost::scoped_ptr<DocumentStreamAccessImpl> mpImpl;

    DocumentStreamAccess();

public:
    DocumentStreamAccess( ScDocument& rDoc );
    ~DocumentStreamAccess();

    void setNumericCell( const ScAddress& rPos, double fVal );
    void setStringCell( const ScAddress& rPos, const OUString& rStr );

    /**
     * Clear its internal state, and more importantly all the block position
     * hints currently held.
     */
    void reset();

    /**
     * Pop the top row inside specified range, shift all the other rows up by
     * one, then set the bottom row empty.
     */
    void shiftRangeUp( const ScRange& rRange );

    /**
     * Top the bottom row inside specified range, shift all the other rows
     * above downward by one by inserting an empty row at the top.
     */
    void shiftRangeDown( const ScRange& rRange );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
