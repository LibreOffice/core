/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_MORK_DBMETADATAHELPER_HXX
#define CONNECTIVITY_MORK_DBMETADATAHELPER_HXX

#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include "FDatabaseMetaDataResultSet.hxx"
#include "MErrorResource.hxx"

#include <MConnection.hxx>
#include <com/sun/star/uno/Sequence.hxx>



namespace connectivity
{
    namespace mork
    {
        class MDatabaseMetaDataHelper
        {
        public:
            MDatabaseMetaDataHelper();
            ~MDatabaseMetaDataHelper();

            //
            sal_Bool getTableStrings( OConnection*                        _pCon,
                                      ::std::vector< ::rtl::OUString >&   _rStrings);

            sal_Bool getTables( OConnection* _pCon,
                                const ::rtl::OUString& tableNamePattern,
                                ODatabaseMetaDataResultSet::ORows& _rRows);
        };
    }
}

#endif // CONNECTIVITY_MORK_DBMETADATAHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
