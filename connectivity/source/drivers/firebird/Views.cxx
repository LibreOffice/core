/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "Tables.hxx"
#include "Views.hxx"
#include "View.hxx"
#include "Catalog.hxx"
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <TConnection.hxx>

connectivity::firebird::Views::Views(
    const css::uno::Reference<css::sdbc::XConnection>& _rxConnection, ::cppu::OWeakObject& _rParent,
    ::osl::Mutex& _rMutex, const ::std::vector<OUString>& _rVector)
    : sdbcx::OCollection(_rParent, true, _rMutex, _rVector)
    , m_xConnection(_rxConnection)
    , m_xMetaData(_rxConnection->getMetaData())
    , m_bInDrop(false)
{
}

connectivity::sdbcx::ObjectType connectivity::firebird::Views::createObject(const OUString& _rName)
{
    OUString sCatalog, sSchema, sTable;
    ::dbtools::qualifiedNameComponents(m_xMetaData, _rName, sCatalog, sSchema, sTable,
                                       ::dbtools::EComposeRule::InDataManipulation);
    return new View(m_xConnection, isCaseSensitive(), sSchema, sTable);
}

void connectivity::firebird::Views::impl_refresh()
{
    static_cast<Catalog&>(m_rParent).refreshViews();
}

css::uno::Reference<css::beans::XPropertySet> connectivity::firebird::Views::createDescriptor()
{
    return new connectivity::sdbcx::OView(true, m_xMetaData);
}

// XAppend
connectivity::sdbcx::ObjectType connectivity::firebird::Views::appendObject(
    const OUString& _rForName, const css::uno::Reference<css::beans::XPropertySet>& descriptor)
{
    createView(descriptor);
    return createObject(_rForName);
}

// XDrop
void connectivity::firebird::Views::dropObject(sal_Int32 _nPos, const OUString& /*_sElementName*/)
{
    if (m_bInDrop)
        return;

    css::uno::Reference<XInterface> xObject(getObject(_nPos));
    bool bIsNew = connectivity::sdbcx::ODescriptor::isNew(xObject);
    if (!bIsNew)
    {
        OUString aSql(u"DROP VIEW"_ustr);

        css::uno::Reference<css::beans::XPropertySet> xProp(xObject, css::uno::UNO_QUERY);
        aSql += ::dbtools::composeTableName(m_xMetaData, xProp,
                                            ::dbtools::EComposeRule::InTableDefinitions, true);

        css::uno::Reference<css::sdbc::XConnection> xConnection = m_xMetaData->getConnection();
        css::uno::Reference<css::sdbc::XStatement> xStmt = xConnection->createStatement();
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}

void connectivity::firebird::Views::dropByNameImpl(const OUString& elementName)
{
    m_bInDrop = true;
    connectivity::sdbcx::OCollection::dropByName(elementName);
    m_bInDrop = false;
}

void connectivity::firebird::Views::createView(
    const css::uno::Reference<css::beans::XPropertySet>& descriptor)
{
    css::uno::Reference<css::sdbc::XConnection> xConnection = m_xMetaData->getConnection();

    OUString sCommand;
    descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_COMMAND))
        >>= sCommand;

    OUString aSql = "CREATE VIEW "
                    + ::dbtools::composeTableName(m_xMetaData, descriptor,
                                                  ::dbtools::EComposeRule::InTableDefinitions, true)
                    + " AS " + sCommand;

    css::uno::Reference<css::sdbc::XStatement> xStmt = xConnection->createStatement();
    if (xStmt.is())
    {
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
    connectivity::firebird::Tables* pTables = static_cast<connectivity::firebird::Tables*>(
        static_cast<connectivity::firebird::Catalog&>(m_rParent).getPrivateTables());
    if (pTables)
    {
        OUString sName = ::dbtools::composeTableName(
            m_xMetaData, descriptor, ::dbtools::EComposeRule::InDataManipulation, false);
        pTables->appendNew(sName);
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
