/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CONNECTIVITY_FIREBIRD_UTIL_HXX
#define CONNECTIVITY_FIREBIRD_UTIL_HXX

#include <rtl/ustring.hxx>

#include <com/sun/star/sdbc/DataType.hpp>

namespace connectivity
{
    namespace firebird
    {
        sal_Int32 getColumnTypeFromFBType(int aType);
        ::rtl::OUString getColumnTypeNameFromFBType(int aType);
    }
}
#endif //CONNECTIVITY_FIREBIRD_UTIL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */