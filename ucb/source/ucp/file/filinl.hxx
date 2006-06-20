/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filinl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 05:20:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
