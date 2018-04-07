/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_HSQLIMPORT_HXX
#define INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_HSQLIMPORT_HXX

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include "rowinputbinary.hxx"
#include "hsqlbinarynode.hxx"
#include "columndef.hxx"

namespace dbahsql
{
class SAL_DLLPUBLIC_EXPORT HsqlImporter
{
private:
    css::uno::Reference<css::sdbc::XConnection>& m_rConnection;
    css::uno::Reference<css::embed::XStorage> m_xStorage;

protected:
    void insertRow(const std::vector<css::uno::Any>& xRows, const OUString& sTable,
                   const std::vector<ColumnDefinition>& rColTypes);
    void processTree(HsqlBinaryNode& rNode, HsqlRowInputStream& rStream,
                     const std::vector<ColumnDefinition>& rColTypes, const OUString& sTableName);
    void parseTableRows(const std::vector<sal_Int32>& rIndexes,
                        const std::vector<ColumnDefinition>& rColTypes, const OUString& sTableName);

public:
    HsqlImporter(css::uno::Reference<css::sdbc::XConnection>& rConnection,
                 const css::uno::Reference<css::embed::XStorage>& rStorage);

    void importHsqlDatabase();
};
}

#endif // INCLUDED_DBACCESS_SOURCE_FILTER_HSQLDB_HSQLIMPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
