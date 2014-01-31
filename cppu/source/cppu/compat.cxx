/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstdlib>

#include <typelib/typedescription.h>
#include <rtl/ustring.h>
#include <sal/types.h>

// Stubs for removed functionality, to be killed when we bump sal SONAME

extern "C" {

SAL_DLLPUBLIC_EXPORT void SAL_CALL typelib_static_array_type_init(
    typelib_TypeDescriptionReference **, typelib_TypeDescriptionReference *,
    sal_Int32, ...) SAL_THROW_EXTERN_C()
{
    std::abort();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL typelib_typedescription_newArray(
    typelib_TypeDescription **, typelib_TypeDescriptionReference *,
    sal_Int32, sal_Int32 *) SAL_THROW_EXTERN_C()
{
    std::abort();
}

SAL_DLLPUBLIC_EXPORT void SAL_CALL typelib_typedescription_newUnion(
    typelib_TypeDescription **, rtl_uString *,
    typelib_TypeDescriptionReference *, sal_Int64,
    typelib_TypeDescriptionReference *, sal_Int32, void *)
    SAL_THROW_EXTERN_C()
{
    std::abort();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
