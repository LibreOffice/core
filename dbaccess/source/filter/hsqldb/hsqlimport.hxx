/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>

#include "rowinputbinary.hxx"
#include "hsqlbinarynode.hxx"
#include "columndef.hxx"

namespace weld
{
class Window;
}

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
                     const std::vector<ColumnDefinition>& rColTypes, const OUString& sTableName,
                     sal_Int32 nIndexCount);
    void parseTableRows(const std::vector<sal_Int32>& rIndexes,
                        const std::vector<ColumnDefinition>& rColTypes, const OUString& sTableName);

public:
    /**
     * @param rConnection reference to an sdbc connection. The migration will
     * perform to this connection.
     *
     * @param rStorage Storage where the HSQL database can be found. The schema
     * definition should be in file "script" in form of DDL SQL statements. The
     * data should be found in file "data". These are HSQLDB's own format.
     */
    HsqlImporter(css::uno::Reference<css::sdbc::XConnection>& rConnection,
                 const css::uno::Reference<css::embed::XStorage>& rStorage);

    /**
     * Migrate a HSQL database to another.
     */
    void importHsqlDatabase(weld::Window* pParent);
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
