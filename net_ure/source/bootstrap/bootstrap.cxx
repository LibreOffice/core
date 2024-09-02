/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string_view>

#include <bridges/net_uno/net_context.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <uno/mapping.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace net_uno
{
extern "C" {
SAL_DLLPUBLIC_EXPORT IntPtr bootstrap(const Context aContext)
{
    try
    {
        // Bootstrap UNO and start a LibreOffice process if needed
        Reference<XComponentContext> xContext(::cppu::bootstrap());

        // Get a mapping between the C++ and .NET environments
        Environment cpp_env(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
        Environment net_env(u"" UNO_LB_NET ""_ustr, new Context(aContext));
        Mapping mapping(cpp_env.get(), net_env.get());
        if (!mapping.is())
        {
            Reference<lang::XComponent> xComp(xContext, UNO_QUERY);
            if (xComp.is())
            {
                xComp->dispose();
            }

            throw RuntimeException(u"could not get mapping between C++ and .NET"_ustr);
        }

        // Map the XComponentContext to a .NET proxy
        return mapping.mapInterface(xContext.get(), cppu::UnoType<decltype(xContext)>::get());
    }
    catch (const Exception& exc)
    {
        SAL_WARN("net", ".NET bootstrap error: " << exc.Message);
        aContext.throwError((u"" SAL_WHERE ""_ustr).getStr(), exc.Message.getStr());
        return nullptr;
    }
}

SAL_DLLPUBLIC_EXPORT IntPtr defaultBootstrap_InitialComponentContext(const sal_Unicode* sIniFile,
                                                                     const sal_Unicode* sParams,
                                                                     const Context aContext)
{
    try
    {
        // Set bootstrap parameters, merged into a single string (at least for now)
        // to avoid dealing with lifetimes and memory of multiple strings
        if (sParams)
        {
            OUString paramsStr(sParams);
            for (size_t i = 0; i != std::u16string_view::npos;)
            {
                std::u16string_view name(o3tl::getToken(paramsStr, u'|', i));
                OUString key(name.substr(0, name.find_first_of('=')));
                OUString val(name.substr(key.getLength() + 1));
                ::rtl::Bootstrap::set(key, val);
            }
        }

        // Bootstrap UNO
        Reference<XComponentContext> xContext;
        if (sIniFile)
        {
            xContext = ::cppu::defaultBootstrap_InitialComponentContext(OUString(sIniFile));
        }
        else
        {
            xContext = ::cppu::defaultBootstrap_InitialComponentContext();
        }

        // Get a mapping between the C++ and .NET environments
        Environment cpp_env(CPPU_CURRENT_LANGUAGE_BINDING_NAME);
        Environment net_env(u"" UNO_LB_NET ""_ustr, new Context(aContext));
        Mapping mapping(cpp_env.get(), net_env.get());
        if (!mapping.is())
        {
            Reference<lang::XComponent> xComp(xContext, UNO_QUERY);
            if (xComp.is())
            {
                xComp->dispose();
            }

            throw RuntimeException(u"could not get mapping between C++ and .NET"_ustr);
        }

        // Map the XComponentContext to a .NET proxy
        return mapping.mapInterface(xContext.get(), cppu::UnoType<decltype(xContext)>::get());
    }
    catch (const Exception& exc)
    {
        SAL_WARN("net", ".NET bootstrap error: " << exc.Message);
        aContext.throwError((u"" SAL_WHERE ""_ustr).getStr(), exc.Message.getStr());
        return nullptr;
    }
}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
