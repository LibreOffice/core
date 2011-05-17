/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
            ::std::vector< ::rtl::OUString >                m_aTableNames;
            ::std::vector< ::rtl::OUString >                m_aTableTypes;
            ::com::sun::star::mozilla::MozillaProductType   m_ProductType;
            ::rtl::OUString                                 m_ProfileName;
            ErrorDescriptor                                 m_aError;

        public:
            MDatabaseMetaDataHelper();
            ~MDatabaseMetaDataHelper();

            //
            sal_Bool getTableStrings( OConnection*                        _pCon,
                                      ::std::vector< ::rtl::OUString >&   _rStrings,
                                      ::std::vector< ::rtl::OUString >&   _rTypes);

            sal_Bool getTables( OConnection* _pCon,
                                const ::rtl::OUString& tableNamePattern,
                                const ::com::sun::star::uno::Sequence< ::rtl::OUString >& types,
                                ODatabaseMetaDataResultSet::ORows& _rRows);
            sal_Bool   testLDAPConnection( OConnection* _pCon );
            sal_Bool   NewAddressBook( OConnection* _pCon,const ::rtl::OUString & aTableName);

            inline const ErrorDescriptor& getError() const { return m_aError; }
        };
    }

}
#endif // _CONNECTIVITY_MAB_DATABASEMETADATAHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
