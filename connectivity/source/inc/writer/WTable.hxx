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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WTABLE_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WTABLE_HXX

#include <component/CTable.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace text
{
class XTextTable;
}
}
}
}

namespace com
{
namespace sun
{
namespace star
{
namespace util
{
class XNumberFormats;
}
}
}
}

namespace connectivity
{
namespace writer
{
using OWriterTable_BASE = component::OComponentTable;
class OWriterConnection;

class OWriterTable : public OWriterTable_BASE
{
private:
    css::uno::Reference<css::text::XTextTable> m_xTable;
    OWriterConnection* m_pWriterConnection;
    sal_Int32 m_nStartCol;
    sal_Int32 m_nDataCols;
    bool m_bHasHeaders;

    void fillColumns();

public:
    OWriterTable(sdbcx::OCollection* _pTables, OWriterConnection* _pConnection,
                 const OUString& Name, const OUString& Type);

    bool fetchRow(OValueRefRow& _rRow, const OSQLColumns& _rCols, bool bRetrieveData) override;

    void SAL_CALL disposing() override;

    // css::lang::XUnoTunnel
    sal_Int64 SAL_CALL getSomething(const css::uno::Sequence<sal_Int8>& rId) override;
    static css::uno::Sequence<sal_Int8> getUnoTunnelImplementationId();

    void construct() override;
};

} // namespace writer
} // namespace connectivity

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_WRITER_WTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
