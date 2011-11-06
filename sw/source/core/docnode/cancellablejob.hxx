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


#ifndef _CANCELLABLEJOB_HXX
#define _CANCELLABLEJOB_HXX

#include "sal/config.h"
#include "cppuhelper/implbase1.hxx"
#include "com/sun/star/util/XCancellable.hpp"

#include <rtl/ref.hxx>

class ObservableThread;

class CancellableJob : public ::cppu::WeakImplHelper1<com::sun::star::util::XCancellable>
{
public:
    explicit CancellableJob( const ::rtl::Reference< ObservableThread >& rThread );
    ~CancellableJob() {}

    // ::com::sun::star::util::XCancellable:
    virtual void SAL_CALL cancel() throw (com::sun::star::uno::RuntimeException);

private:
    CancellableJob( CancellableJob& ); // not defined
    void operator =( CancellableJob& ); // not defined

    ::rtl::Reference< ObservableThread > mrThread;
};
#endif
