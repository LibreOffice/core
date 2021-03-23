/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "Table.hxx"

#include <connectivity/TIndexes.hxx>

namespace connectivity::firebird
    {

        /**
         * Firebird has a non-standard DROP INDEX statement, hence we need
         * to override OIndexesHelper::dropObject
         */
        class Indexes: public ::connectivity::OIndexesHelper
        {
        private:
            Table* m_pTable;
        protected:
            // XDrop
            virtual void dropObject(sal_Int32 nPosition,
                                    const OUString& sIndexName) override;
        public:
            Indexes(Table* pTable,
                    ::osl::Mutex& rMutex,
                    const std::vector< OUString>& rVector);
        };

} // namespace connectivity::firebird

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
