/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <connectivity/sdbcx/VCollection.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
namespace connectivity::firebird
{
class Views final : public connectivity::sdbcx::OCollection
{
    css::uno::Reference<css::sdbc::XConnection> m_xConnection;
    css::uno::Reference<css::sdbc::XDatabaseMetaData> m_xMetaData;
    bool m_bInDrop;

    // OCollection
    virtual connectivity::sdbcx::ObjectType createObject(const OUString& _rName) override;
    virtual void impl_refresh() override;
    virtual css::uno::Reference<css::beans::XPropertySet> createDescriptor() override;
    virtual sdbcx::ObjectType
    appendObject(const OUString& _rForName,
                 const css::uno::Reference<css::beans::XPropertySet>& descriptor) override;

    void createView(const css::uno::Reference<css::beans::XPropertySet>& descriptor);

public:
    Views(const css::uno::Reference<css::sdbc::XConnection>& _rxConnection,
          ::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
          const ::std::vector<OUString>& _rVector);

    // XDrop
    virtual void dropObject(sal_Int32 _nPos, const OUString& _sElementName) override;

    void dropByNameImpl(const OUString& elementName);
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
