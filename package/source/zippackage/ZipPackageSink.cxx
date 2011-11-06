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
#include "precompiled_package.hxx"
#include <ZipPackageSink.hxx>

ZipPackageSink::ZipPackageSink(void)
: xStream ( com::sun::star::uno::Reference < com::sun::star::io::XInputStream > (NULL))
{
}
ZipPackageSink::~ZipPackageSink(void)
{
}
void SAL_CALL ZipPackageSink::setInputStream( const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& aStream )
    throw(::com::sun::star::uno::RuntimeException)
{
    xStream = aStream;
}
::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL ZipPackageSink::getInputStream(  )
    throw(::com::sun::star::uno::RuntimeException)
{
    return xStream;
}
