/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FILINL_HXX_
#define _FILINL_HXX_

inline const sal_Bool& SAL_CALL shell::MyProperty::IsNative() const
{
    return isNative;
}
inline const sal_Int32& SAL_CALL shell::MyProperty::getHandle() const
{
    return Handle;
}
inline const com::sun::star::uno::Type& SAL_CALL shell::MyProperty::getType() const
{
    return Typ;
}
inline const com::sun::star::uno::Any& SAL_CALL shell::MyProperty::getValue() const
{
    return Value;
}
inline const com::sun::star::beans::PropertyState& SAL_CALL shell::MyProperty::getState() const
{
    return State;
}
inline const sal_Int16& SAL_CALL shell::MyProperty::getAttributes() const
{
    return Attributes;
}
inline void SAL_CALL shell::MyProperty::setHandle( const sal_Int32& __Handle ) const
{
    (( MyProperty* )this )->Handle = __Handle;
}
inline void SAL_CALL shell::MyProperty::setType( const com::sun::star::uno::Type& __Typ ) const
{
    (( MyProperty* )this )->Typ = __Typ;
}
inline void SAL_CALL shell::MyProperty::setValue( const com::sun::star::uno::Any& __Value ) const
{
    (( MyProperty* )this )->Value = __Value;
}
inline void SAL_CALL shell::MyProperty::setState( const com::sun::star::beans::PropertyState& __State ) const
{
    (( MyProperty* )this )->State = __State;
}
inline void SAL_CALL shell::MyProperty::setAttributes( const sal_Int16& __Attributes ) const
{
    (( MyProperty* )this )->Attributes = __Attributes;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
