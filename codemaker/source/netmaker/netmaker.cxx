/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include <sal/main.h>
#include <unoidl/unoidl.hxx>

#include "netoptions.hxx"
#include "netproduce.hxx"

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    try
    {
        NetOptions options;
        NetProducer producer;

        if (options.initOptions(argc, argv))
        {
            producer.initProducer(options);
            producer.produceAll();
        }
    }
    catch (const ::IllegalArgument& e)
    {
        std::cerr << "ERROR: Illegal option " << e.m_message << '\n';
        return EXIT_FAILURE;
    }
    catch (const ::CannotDumpException& e)
    {
        std::cerr << "ERROR: Could not dump as " << e.getMessage() << '\n';
        return EXIT_FAILURE;
    }
    catch (const unoidl::NoSuchFileException& e)
    {
        std::cerr << "ERROR: No such file " << e.getUri() << '\n';
        return EXIT_FAILURE;
    }
    catch (const unoidl::FileFormatException& e)
    {
        std::cerr << "ERROR: Bad format of " << e.getUri() << ", '" << e.getDetail() << "'\n";
        return EXIT_FAILURE;
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
