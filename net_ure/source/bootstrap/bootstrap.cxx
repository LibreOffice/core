/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string_view>

#include <cppuhelper/bootstrap.hxx>
#include <o3tl/string_view.hxx>
#include <rtl/bootstrap.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

extern "C" {

SAL_DLLPUBLIC_EXPORT void* bootstrap(const sal_Unicode* sParams)
{
    try
    {
        if (sParams)
        {
            OUString paramsStr(sParams);
            for (size_t i = 0; i != std::u16string_view::npos;)
            {
                std::u16string_view name(o3tl::getToken(paramsStr, u'=', i));
                OUString key(name.substr(0, name.find_first_of('|')));
                OUString val(name.substr(key.getLength() + 1));
                ::rtl::Bootstrap::set(key, val);
            }
        }

        Reference<XComponentContext> xContext(::cppu::bootstrap());
        return xContext.get();
    }
    catch (...)
    {
        return nullptr;
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
