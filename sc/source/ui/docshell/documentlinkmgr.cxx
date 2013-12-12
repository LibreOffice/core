/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <documentlinkmgr.hxx>
#include <datastream.hxx>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace sc {

struct DocumentLinkManagerImpl : boost::noncopyable
{
    boost::scoped_ptr<DataStream> mpDataStream;

    DocumentLinkManagerImpl() : mpDataStream(NULL) {}
};

DocumentLinkManager::DocumentLinkManager() : mpImpl(new DocumentLinkManagerImpl) {}

void DocumentLinkManager::setDataStream( DataStream* p )
{
    mpImpl->mpDataStream.reset(p);
}

DataStream* DocumentLinkManager::getDataStream()
{
    return mpImpl->mpDataStream.get();
}

const DataStream* DocumentLinkManager::getDataStream() const
{
    return mpImpl->mpDataStream.get();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
