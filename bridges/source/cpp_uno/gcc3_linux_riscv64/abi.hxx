/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

//#pragma once
#include <uno/data.h>
#include <typelib/typedescription.hxx>

namespace abi_riscv64
{
void countnGreg(sal_Int32& nGreg, sal_Int32& nFreg,
                const typelib_CompoundTypeDescription* pTypeDescr);

void fillStruct(const typelib_TypeDescription* pTypeDescr, sal_Int64* gret, double* fret,
                void* pRegisterReturn);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
