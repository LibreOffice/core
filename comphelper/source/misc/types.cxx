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
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <osl/diagnose.h>
#include <typelib/typedescription.hxx>

#include <memory.h>



namespace comphelper
{


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;

sal_Int64 getINT64(const Any& _rAny)
{
    sal_Int64 nReturn = 0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


sal_Int32 getINT32(const Any& _rAny)
{
    sal_Int32 nReturn = 0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


sal_Int16 getINT16(const Any& _rAny)
{
    sal_Int16 nReturn = 0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


double getDouble(const Any& _rAny)
{
    double nReturn = 0.0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


float getFloat(const Any& _rAny)
{
    float nReturn = 0.0;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


OUString getString(const Any& _rAny)
{
    OUString nReturn;
    OSL_VERIFY( _rAny >>= nReturn );
    return nReturn;
}


bool getBOOL(const Any& _rAny)
{
    bool bReturn = false;
    if (_rAny.getValueType() == cppu::UnoType<bool>::get())
        bReturn = *static_cast<sal_Bool const *>(_rAny.getValue());
    else
        OSL_FAIL("comphelper::getBOOL : invalid argument !");
    return bReturn;
}


sal_Int32 getEnumAsINT32(const Any& _rAny) throw(IllegalArgumentException)
{
    sal_Int32 nReturn = 0;
    if (! ::cppu::enum2int(nReturn,_rAny) )
        throw IllegalArgumentException();
    return nReturn;
}


FontDescriptor  getDefaultFont()
{
    FontDescriptor aReturn;
    aReturn.Slant = FontSlant_DONTKNOW;
    aReturn.Underline = FontUnderline::DONTKNOW;
    aReturn.Strikeout = FontStrikeout::DONTKNOW;
    return aReturn;
}


bool isAssignableFrom(const Type& _rAssignable, const Type& _rFrom)
{
    // get the type lib descriptions
    typelib_TypeDescription* pAssignable = NULL;
    _rAssignable.getDescription(&pAssignable);

    typelib_TypeDescription* pFrom = NULL;
    _rFrom.getDescription(&pFrom);

    // and ask the type lib
    return typelib_typedescription_isAssignableFrom(pAssignable, pFrom);
}

bool    operator ==(const FontDescriptor& _rLeft, const FontDescriptor& _rRight)
{
    return ( _rLeft.Name.equals( _rRight.Name ) ) &&
    ( _rLeft.Height == _rRight.Height ) &&
    ( _rLeft.Width == _rRight.Width ) &&
    ( _rLeft.StyleName.equals( _rRight.StyleName ) ) &&
    ( _rLeft.Family == _rRight.Family ) &&
    ( _rLeft.CharSet == _rRight.CharSet ) &&
    ( _rLeft.Pitch == _rRight.Pitch ) &&
    ( _rLeft.CharacterWidth == _rRight.CharacterWidth ) &&
    ( _rLeft.Weight == _rRight.Weight ) &&
    ( _rLeft.Slant == _rRight.Slant ) &&
    ( _rLeft.Underline == _rRight.Underline ) &&
    ( _rLeft.Strikeout == _rRight.Strikeout ) &&
    ( _rLeft.Orientation == _rRight.Orientation ) &&
    ( _rLeft.Kerning == _rRight.Kerning ) &&
    ( _rLeft.WordLineMode == _rRight.WordLineMode ) &&
    ( _rLeft.Type == _rRight.Type ) ;
}


Type getSequenceElementType(const Type& _rSequenceType)
{
    OSL_ENSURE(_rSequenceType.getTypeClass() == TypeClass_SEQUENCE,
                "getSequenceElementType: must be called with a  sequence type!");

    if (!(_rSequenceType.getTypeClass() == TypeClass_SEQUENCE))
        return Type();

    TypeDescription aTD(_rSequenceType);
    typelib_IndirectTypeDescription* pSequenceTD =
        reinterpret_cast< typelib_IndirectTypeDescription* >(aTD.get());

    OSL_ASSERT(pSequenceTD && pSequenceTD->pType);
    if (pSequenceTD && pSequenceTD->pType)
        return Type(pSequenceTD->pType);

    return Type();
}

}   // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
