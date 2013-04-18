/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOIDL_UNOIDLPROVIDER_HXX
#define INCLUDED_UNOIDL_UNOIDLPROVIDER_HXX

#include "sal/config.h"

#include "rtl/ref.hxx"
#include "sal/types.h"
#include "unoidl/detail/dllapi.hxx"
#include "unoidl/unoidl.hxx"

namespace unoidl { namespace detail {
    class MappedFile;
    struct MapEntry;
} }

namespace unoidl {

class LO_DLLPUBLIC_UNOIDL UnoidlProvider: public Provider {
public:
    // throws FileFormatException, NoSuchFileException:
    explicit UnoidlProvider(OUString const & uri);

    // throws FileFormatException:
    virtual rtl::Reference< MapCursor > createRootCursor() const;

    // throws FileFormatException:
    virtual rtl::Reference< Entity > findEntity(OUString const & name)
        const;

    // throws FileFormatException:
    sal_uInt32 find(OUString const & name, bool * constant = 0) const;

    // throws FileFormatException:
    rtl::Reference< Entity > getEntity(sal_uInt32 offset) const;

    // throws FileFormatException:
    ConstantValue getConstant(sal_uInt32 offset) const;

private:
    virtual SAL_DLLPRIVATE ~UnoidlProvider() throw ();

    rtl::Reference< detail::MappedFile > file_;
    detail::MapEntry const * mapBegin_;
    sal_uInt32 mapSize_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
