/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/stream.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include "commonfuzzer.hxx"

extern "C" void* ScCreateDialogFactory() { return nullptr; }

extern "C" bool TestImportWKS(SvStream& rStream);

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    if (__lsan_disable)
        __lsan_disable();

    CommonInitialize(argc, argv);

    // initialise unconfigured UCB:
    css::uno::Reference<css::ucb::XUniversalContentBroker> xUcb(
        comphelper::getProcessServiceFactory()->createInstance(
            "com.sun.star.ucb.UniversalContentBroker"),
        css::uno::UNO_QUERY_THROW);
    css::uno::Sequence<css::uno::Any> aArgs(1);
    aArgs[0] <<= OUString("NoConfig");
    css::uno::Reference<css::ucb::XContentProvider> xFileProvider(
        comphelper::getProcessServiceFactory()->createInstanceWithArguments(
            "com.sun.star.ucb.FileContentProvider", aArgs),
        css::uno::UNO_QUERY_THROW);
    xUcb->registerContentProvider(xFileProvider, "file", true);

    if (__lsan_enable)
        __lsan_enable();

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    SvMemoryStream aStream(const_cast<uint8_t*>(data), size, StreamMode::READ);
    (void)TestImportWKS(aStream);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
