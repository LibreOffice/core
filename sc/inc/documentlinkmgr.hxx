/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SC_DOCUMENTLINKMGR_HXX
#define SC_DOCUMENTLINKMGR_HXX

#include <boost/noncopyable.hpp>

namespace sc {

class DataStream;
struct DocumentLinkManagerImpl;

class DocumentLinkManager : boost::noncopyable
{
    DocumentLinkManagerImpl* mpImpl;

public:
    DocumentLinkManager();
    ~DocumentLinkManager();

    void setDataStream( DataStream* p );
    DataStream* getDataStream();
    const DataStream* getDataStream() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
