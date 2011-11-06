/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
