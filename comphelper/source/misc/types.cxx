/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <typelib/typedescription.hxx>
#include <sal/log.hxx>

namespace comphelper
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;

sal_Int64 getINT64(const Any& _rAny)
{
    sal_Int64 nReturn = 0;
    if (!(_rAny >>= nReturn))
        SAL_WARN("comphelper", "conversion from Any to sal_Int64 failed");
    return nReturn;
}

sal_Int32 getINT32(const Any& _rAny)
{
    sal_Int32 nReturn = 0;
    if (!(_rAny >>= nReturn))
        SAL_WARN("comphelper", "conversion from Any to sal_Int32 failed");
    return nReturn;
}

sal_Int16 getINT16(const Any& _rAny)
{
    sal_Int16 nReturn = 0;
    if (!(_rAny >>= nReturn))
        SAL_WARN("comphelper", "conversion from Any to sal_Int16 failed");
    return nReturn;
}

double getDouble(const Any& _rAny)
{
    double nReturn = 0.0;
    if (!(_rAny >>= nReturn))
        SAL_WARN("comphelper", "conversion from Any to double failed");
    return nReturn;
}

float getFloat(const Any& _rAny)
{
    float nReturn = 0.0;
    if (!(_rAny >>= nReturn))
        SAL_WARN("comphelper", "conversion from Any to float failed");
    return nReturn;
}

OUString getString(const Any& _rAny)
{
    OUString nReturn;
    if (!(_rAny >>= nReturn))
        SAL_WARN("comphelper", "conversion from Any to OUString failed");
    return nReturn;
}

bool getBOOL(const Any& _rAny)
{
    bool bReturn = false;
    if (auto b = o3tl::tryAccess<bool>(_rAny))
        bReturn = *b;
    else
        OSL_FAIL("comphelper::getBOOL : invalid argument !");
    return bReturn;
}

sal_Int32 getEnumAsINT32(const Any& _rAny)
{
    sal_Int32 nReturn = 0;
    if (!::cppu::enum2int(nReturn, _rAny))
        throw IllegalArgumentException("enum2int failed",
                                       css::uno::Reference<css::uno::XInterface>(), -1);
    return nReturn;
}

FontDescriptor getDefaultFont()
{
    FontDescriptor aReturn;
    aReturn.Slant = FontSlant_DONTKNOW;
    aReturn.Underline = FontUnderline::DONTKNOW;
    aReturn.Strikeout = com::sun::star::awt::FontStrikeout::DONTKNOW;
    return aReturn;
}

bool isAssignableFrom(const Type& _rAssignable, const Type& _rFrom)
{
    // get the type lib descriptions
    typelib_TypeDescription* pAssignable = nullptr;
    _rAssignable.getDescription(&pAssignable);

    typelib_TypeDescription* pFrom = nullptr;
    _rFrom.getDescription(&pFrom);

    // and ask the type lib
    return typelib_typedescription_isAssignableFrom(pAssignable, pFrom);
}

Type getSequenceElementType(const Type& _rSequenceType)
{
    OSL_ENSURE(_rSequenceType.getTypeClass() == TypeClass_SEQUENCE,
               "getSequenceElementType: must be called with a  sequence type!");

    if (_rSequenceType.getTypeClass() != TypeClass_SEQUENCE)
        return Type();

    TypeDescription aTD(_rSequenceType);
    typelib_IndirectTypeDescription* pSequenceTD
        = reinterpret_cast<typelib_IndirectTypeDescription*>(aTD.get());

    OSL_ASSERT(pSequenceTD && pSequenceTD->pType);
    if (pSequenceTD && pSequenceTD->pType)
        return Type(pSequenceTD->pType);

    return Type();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
