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

#include <file/filedllapi.hxx>
#include <connectivity/sdbcx/VCollection.hxx>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>

namespace connectivity::file
    {
        class OOO_DLLPUBLIC_FILE SAL_NO_VTABLE OTables :
            public sdbcx::OCollection
        {
        protected:
            virtual sdbcx::ObjectType createObject(const OUString& _rName) override;
            virtual void impl_refresh() override;
        public:
            OTables(const css::uno::Reference< css::sdbc::XDatabaseMetaData >& _rMetaData,::cppu::OWeakObject& _rParent, ::osl::Mutex& _rMutex,
                const ::std::vector< OUString> &_rVector) : sdbcx::OCollection(_rParent,_rMetaData->supportsMixedCaseQuotedIdentifiers(),_rMutex,_rVector)
            {}

            virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
        };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
