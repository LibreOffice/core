/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WDATABASEMETADATA_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WDATABASEMETADATA_HXX

#include "file/FDatabaseMetaData.hxx"

namespace connectivity
{
namespace writer
{

class OWriterDatabaseMetaData : public file::ODatabaseMetaData
{
    virtual css::uno::Reference< css::sdbc::XResultSet > impl_getTypeInfo_throw() override;
    virtual OUString SAL_CALL getURL() override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getColumns(const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const OUString& columnNamePattern) override;
    virtual sal_Int32 SAL_CALL getMaxBinaryLiteralLength() override;
    virtual sal_Int32 SAL_CALL getMaxCharLiteralLength() override;
    virtual sal_Int32 SAL_CALL getMaxColumnNameLength() override;
    virtual sal_Int32 SAL_CALL getMaxColumnsInIndex() override;
    virtual sal_Int32 SAL_CALL getMaxColumnsInTable() override;
    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTables(const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const css::uno::Sequence< OUString >& types) override;
protected:
    virtual ~OWriterDatabaseMetaData() override;
public:
    OWriterDatabaseMetaData(file::OConnection* pConnection);
};

} // namespace writer
} // namespace connectivity

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WDATABASEMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
