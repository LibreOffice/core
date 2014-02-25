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

#ifndef _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_
#define _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_

#include "MErrorResource.hxx"
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include "FDatabaseMetaDataResultSet.hxx"

#include <MConnection.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/mozilla/MozillaProductType.hpp>

namespace connectivity
{
    namespace mozab
    {
        class MDatabaseMetaDataHelper
        {
        private:
            sal_Bool                                        m_bProfileExists ;
            ::std::vector< OUString >                m_aTableNames;
            ::std::vector< OUString >                m_aTableTypes;
            ::com::sun::star::mozilla::MozillaProductType   m_ProductType;
            OUString                                 m_ProfileName;
            ErrorDescriptor                                 m_aError;

        public:
            MDatabaseMetaDataHelper();
            ~MDatabaseMetaDataHelper();


            sal_Bool getTableStrings( OConnection*                        _pCon,
                                      ::std::vector< OUString >&   _rStrings,
                                      ::std::vector< OUString >&   _rTypes);

            sal_Bool getTables( OConnection* _pCon,
                                const OUString& tableNamePattern,
                                const ::com::sun::star::uno::Sequence< OUString >& types,
                                ODatabaseMetaDataResultSet::ORows& _rRows);
            sal_Bool   testLDAPConnection( OConnection* _pCon );
            sal_Bool   NewAddressBook( OConnection* _pCon,const OUString & aTableName);

            inline const ErrorDescriptor& getError() const { return m_aError; }
        };
    }

}
#endif // _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
