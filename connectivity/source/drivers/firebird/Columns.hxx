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

#include <connectivity/TColumnsHelper.hxx>

namespace connectivity::firebird
{
        class Columns: public ::connectivity::OColumnsHelper
        {
        protected:
            virtual css::uno::Reference< css::beans::XPropertySet > createDescriptor() override;
        public:
            Columns(Table& rTable,
                    ::osl::Mutex& rMutex,
                    const ::std::vector< OUString> &_rVector);
        };

} // namespace connectivity::firebird

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
