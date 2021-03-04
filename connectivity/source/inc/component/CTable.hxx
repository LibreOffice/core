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

#include <file/FTable.hxx>

namespace com::sun::star::sheet {
    class XSpreadsheet;
}

namespace com::sun::star::util {
    class XNumberFormats;
}


namespace connectivity::component
    {
        typedef file::OFileTable OComponentTable_BASE;

        /// Shared Table base class for Writer tables and Calc sheets.
        class OOO_DLLPUBLIC_FILE OComponentTable :  public OComponentTable_BASE
        {
        protected:
            sal_Int32 m_nDataRows;

            virtual void FileClose() override;
        public:
            OComponentTable( sdbcx::OCollection* _pTables,file::OConnection* _pConnection,
                    const OUString& Name,
                    const OUString& Type,
                    const OUString& Description,
                    const OUString& SchemaName,
                    const OUString& CatalogName
                );

            virtual void refreshColumns() override;
            virtual void refreshIndexes() override;

            virtual sal_Int32 getCurrentLastPos() const override;
            virtual bool seekRow(IResultSetHelper::Movement eCursorPosition, sal_Int32 nOffset, sal_Int32& nCurPos) override;

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
            //XTypeProvider
            virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
