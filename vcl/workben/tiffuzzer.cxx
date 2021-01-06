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
#include <filter/TiffReader.hxx>
#include "commonfuzzer.hxx"

#include <config_features.h>
#include <osl/detail/component-mapping.h>

const lib_to_factory_mapping *
lo_get_factory_map(void)
{
    static lib_to_factory_mapping map[] = {
        { 0, 0 }
    };

    return map;
}

const lib_to_constructor_mapping *
lo_get_constructor_map(void)
{
    static lib_to_constructor_mapping map[] = {
        { 0, 0 }
    };

    return map;
}

extern "C" void* lo_get_custom_widget_func(const char*)
{
    return nullptr;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    TypicalFuzzerInitialize(argc, argv);
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    try
    {
        SvMemoryStream aStream(const_cast<uint8_t*>(data), size, StreamMode::READ);
        Graphic aGraphic;
        (void)ImportTiffGraphicImport(aStream, aGraphic);
    }
    catch (...)
    {
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
