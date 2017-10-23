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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CDATABASEMETADATA_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CDATABASEMETADATA_HXX

#include <component/CDatabaseMetaData.hxx>

namespace connectivity
{
    namespace calc
    {

        //************ Class: java.sql.DatabaseMetaDataDate


        class OCalcDatabaseMetaData :   public component::OComponentDatabaseMetaData
        {
            virtual OUString SAL_CALL getURL(  ) override;
            virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getTables( const css::uno::Any& catalog, const OUString& schemaPattern, const OUString& tableNamePattern, const css::uno::Sequence< OUString >& types ) override;
        protected:
            virtual ~OCalcDatabaseMetaData() override;
        public:
            OCalcDatabaseMetaData(file::OConnection* _pCon);
        };
    }
}

#endif // INCLUDED_CONNECTIVITY_SOURCE_INC_CALC_CDATABASEMETADATA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
