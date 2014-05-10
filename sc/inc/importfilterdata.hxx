/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_IMPORTFILTERDATA_HXX
#define INCLUDED_SC_INC_IMPORTFILTERDATA_HXX

#include "address.hxx"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace sc {

/**
 * Stores data imported from the file that need to be processed at the end
 * of the import process.
 */
struct ImportPostProcessData : boost::noncopyable
{
    /**
     * Data stream data needs to be post-processed because it requires
     * ScDocShell instance which is not available in the filter code.
     */
    struct DataStream
    {
        enum InsertPos { InsertTop, InsertBottom };

        OUString maURL;
        ScRange maRange;
        bool mbRefreshOnEmpty;
        InsertPos meInsertPos;

        DataStream();
    };

    boost::scoped_ptr<DataStream> mpDataStream;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
