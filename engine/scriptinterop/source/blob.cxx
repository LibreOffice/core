/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/beans/Optional.hpp>
#include <cpo/uno/Reference.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cpo/uno/XInterface.hpp>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <scriptinterop/XBlob.hpp>

#include "blob.hxx"

namespace {

class BlobImpl: public cppu::WeakImplHelper<scriptinterop::XBlob> {
public:
    BlobImpl(
        cpo::uno::Sequence<sal_Int8> const & data,
        css::beans::Optional<OUString> const & contentType,
        css::beans::Optional<OUString> const & name):
        data_(data), contentType_(contentType), name_(name) {}

    cpo::uno::Sequence<sal_Int8> getBytes() override { return data_; }

    css::beans::Optional<OUString> getContentType() override { return contentType_; }

    css::beans::Optional<OUString> getName() override { return name_; }

private:
    cpo::uno::Sequence<sal_Int8> data_;
    css::beans::Optional<OUString> contentType_;
    css::beans::Optional<OUString> name_;
};

}

cpo::uno::Reference<scriptinterop::XBlob> scriptinterop::detail::createBlob(
    cpo::uno::Sequence<sal_Int8> const & data, css::beans::Optional<OUString> const & contentType,
    css::beans::Optional<OUString> const & name)
{ return new BlobImpl(data, contentType, name); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
