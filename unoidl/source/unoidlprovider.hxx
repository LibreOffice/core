/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <set>

#include <rtl/ref.hxx>
#include <sal/types.h>
#include <unoidl/unoidl.hxx>

namespace unoidl::detail {

class MappedFile;
struct MapEntry;

struct Map {
    MapEntry const * begin;
    sal_uInt32 size;
};

struct NestedMap {
    Map map;
    std::set<Map> trace;
};

class UnoidlProvider: public Provider {
public:
    // throws FileFormatException, NoSuchFileException:
    explicit UnoidlProvider(OUString const & uri);

    // throws FileFormatException:
    virtual rtl::Reference< MapCursor > createRootCursor() const override;

    // throws FileFormatException:
    virtual rtl::Reference< Entity > findEntity(OUString const & name)
        const override;

private:
    virtual ~UnoidlProvider() noexcept override;

    rtl::Reference< detail::MappedFile > file_;
    NestedMap map_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
