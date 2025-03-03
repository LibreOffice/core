/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

#include <connectivity/sdbcx/VCollection.hxx>
#include <utility>

namespace connectivity::mysqlc
{
/**
* This implements com.sun.star.sdbcx.Container, which seems to be
* also known by the name of Tables and Collection.
*/
class Tables : public ::connectivity::sdbcx::OCollection
{
protected:
    css::uno::Reference<css::sdbc::XDatabaseMetaData> m_xMetaData;

    // OCollection
    virtual void impl_refresh() override;
    virtual ::css::uno::Reference<css::beans::XPropertySet>
    createObject(const OUString& rName) override;
    virtual css::uno::Reference<css::beans::XPropertySet> createDescriptor() override;
    virtual ::css::uno::Reference<css::beans::XPropertySet>
    appendObject(const OUString& rName,
                 const css::uno::Reference<css::beans::XPropertySet>& rDescriptor) override;

    void createTable(const css::uno::Reference<css::beans::XPropertySet>& descriptor);
    virtual OUString
    getNameForObject(const css::uno::Reference<css::beans::XPropertySet>& _xObject) override;
    // XDrop
    virtual void dropObject(sal_Int32 nPosition, const OUString& rName) override;

public:
    Tables(css::uno::Reference<css::sdbc::XDatabaseMetaData> xMetaData,
           ::cppu::OWeakObject& rParent, ::osl::Mutex& rMutex,
           ::std::vector<OUString> const& rNames)
        : sdbcx::OCollection(rParent, true, rMutex, rNames)
        , m_xMetaData(std::move(xMetaData))
    {
    }

    void appendNew(const OUString& _rsNewTable);
    // TODO: should we also implement XDataDescriptorFactory, XRefreshable,
    // XAppend,  etc. ?
};

} // namespace connectivity::mysqlc

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
