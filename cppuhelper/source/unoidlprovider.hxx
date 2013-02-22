/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CPPUHELPER_SOURCE_UNOIDLPROVIDER_HXX
#define INCLUDED_CPPUHELPER_SOURCE_UNOIDLPROVIDER_HXX

#include "sal/config.h"

#include "cppuhelper/unoidl.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }
namespace cppu {
    class MappedFile;
    struct MapEntry;
}
namespace rtl { class OUString; }

namespace cppu {

class UnoidlProvider: public unoidl::Provider {
public:
    explicit UnoidlProvider(rtl::OUString const & uri);

    virtual rtl::Reference< unoidl::MapCursor > createRootCursor() const;

    sal_uInt32 find(rtl::OUString const & name, bool * constant = 0) const;

    rtl::Reference< unoidl::Entity > getEntity(sal_uInt32 offset) const;

    css::uno::Any getConstant(sal_uInt32 offset) const;

private:
    virtual ~UnoidlProvider() throw ();

    rtl::Reference< MappedFile > file_;
    MapEntry const * mapBegin_;
    sal_uInt32 mapSize_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
