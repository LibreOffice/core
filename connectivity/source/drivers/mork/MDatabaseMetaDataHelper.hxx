/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MDATABASEMETADATAHELPER_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MDATABASEMETADATAHELPER_HXX

#include <FDatabaseMetaDataResultSet.hxx>
#include "MConnection.hxx"

namespace connectivity::mork
    {
        class MDatabaseMetaDataHelper
        {
        public:
            MDatabaseMetaDataHelper();
            ~MDatabaseMetaDataHelper();


            static void getTableStrings( OConnection*                        _pCon,
                                         std::vector< OUString >&   _rStrings);

            static void getTables( OConnection* _pCon,
                                const OUString& tableNamePattern,
                                ODatabaseMetaDataResultSet::ORows& _rRows);
        };

}

#endif // INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_MORK_MDATABASEMETADATAHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
