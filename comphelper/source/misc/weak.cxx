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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#include "comphelper/weak.hxx"

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace comphelper
{

OWeakTypeObject::OWeakTypeObject()
{
}

OWeakTypeObject::~OWeakTypeObject()
{
}

Any SAL_CALL OWeakTypeObject::queryInterface(const Type & rType )   throw (RuntimeException)
{
    if( rType == XTypeProvider::static_type() )
        return Any( Reference< XTypeProvider >(this) );
    else
        return ::cppu::OWeakObject::queryInterface( rType );
}

void SAL_CALL OWeakTypeObject::acquire()    throw ()
{
    ::cppu::OWeakObject::acquire();
}

void SAL_CALL OWeakTypeObject::release()    throw ()
{
    ::cppu::OWeakObject::release();
}

Sequence< Type > SAL_CALL OWeakTypeObject::getTypes(  ) throw (RuntimeException)
{
    return Sequence< Type >();
}

Sequence< ::sal_Int8 > SAL_CALL OWeakTypeObject::getImplementationId(  ) throw (RuntimeException)
{
    return Sequence< ::sal_Int8 >();
}

}

