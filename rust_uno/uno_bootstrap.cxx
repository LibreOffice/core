/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDesktop.hpp>
#include <sal/log.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;

extern "C" {

/**
 * C wrapper for cppu::defaultBootstrap_InitialComponentContext()
 *
 * This function provides a C interface to the LibreOffice UNO bootstrap
 * functionality, allowing Rust code to initialize the UNO component context.
 *
 * @return XInterface* - Opaque pointer to XComponentContext (as XInterface)
 *         Returns nullptr on failure.
 */
SAL_DLLPUBLIC_EXPORT void* defaultBootstrap_InitialComponentContext()
{
    SAL_INFO("rust_uno", "Initializing UNO component context for Rust binding");

    try
    {
        // Call the actual LibreOffice bootstrap function
        SAL_INFO("rust_uno", "Calling cppu::defaultBootstrap_InitialComponentContext");
        Reference<XComponentContext> xContext = cppu::defaultBootstrap_InitialComponentContext();

        if (!xContext.is())
        {
            SAL_WARN("rust_uno", "LibreOffice bootstrap failed - component context is invalid");
            return nullptr;
        }

        SAL_INFO("rust_uno", "Component context created successfully");

        auto res = new Reference<XComponentContext>(xContext);

        if (!res->is())
        {
            SAL_WARN("rust_uno", "Reference wrapper creation failed - context became invalid");
            delete res;
            return nullptr;
        }

        SAL_INFO("rust_uno", "Reference wrapper created successfully - returning context to Rust");
        // Return as opaque XComponentContext pointer for Rust
        // Note: This creates a new Reference that transfers ownership to caller
        return res;
    }
    catch (const Exception& e)
    {
        SAL_WARN("rust_uno",
                 "UNO exception during component context initialization: " << e.Message);
        return nullptr;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("rust_uno",
                 "Standard exception during component context initialization: " << e.what());
        return nullptr;
    }
    catch (...)
    {
        SAL_WARN("rust_uno", "Unknown exception during component context initialization");
        return nullptr;
    }
}
} // extern "C"
