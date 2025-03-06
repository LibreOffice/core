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

#pragma once

#include <component/CTable.hxx>
#include <tools/date.hxx>

namespace com::sun::star::sheet {
    class XSpreadsheet;
}

namespace com::sun::star::util {
    class XNumberFormats;
}


namespace connectivity::calc
{
    typedef component::OComponentTable OCalcTable_BASE;
    class OCalcConnection;

    class OCalcTable :  public OCalcTable_BASE
    {
    private:
        std::vector<sal_Int32> m_aTypes;      // holds all type for columns just to avoid to ask the propertyset
        css::uno::Reference< css::sheet::XSpreadsheet >           m_xSheet;
        OCalcConnection* m_pCalcConnection;
        sal_Int32 m_nStartCol;
        sal_Int32 m_nStartRow;
        sal_Int32 m_nDataCols;
        bool      m_bHasHeaders;
        css::uno::Reference< css::util::XNumberFormats > m_xFormats;
        ::Date m_aNullDate;

        void fillColumns();

    public:
        OCalcTable( sdbcx::OCollection* _pTables,OCalcConnection* _pConnection,
                const OUString& Name,
                const OUString& Type,
                const OUString& Description = OUString(),
                const OUString& SchemaName = OUString(),
                const OUString& CatalogName = OUString()
            );

        virtual bool fetchRow(OValueRefRow& _rRow, const OSQLColumns& _rCols, bool bRetrieveData) override;

        virtual void SAL_CALL disposing() override;

        void construct() override;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
