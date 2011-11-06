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


#include "unotools/unotoolsdllapi.h"

#ifndef _UTL_PROGRESSHANDLERWRAP_HXX_
#define _UTL_PROGRESSHANDLERWRAP_HXX_
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>

namespace utl
{

class UNOTOOLS_DLLPUBLIC ProgressHandlerWrap : public ::cppu::WeakImplHelper1< ::com::sun::star::ucb::XProgressHandler >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > m_xStatusIndicator;

public:
    ProgressHandlerWrap( ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > xSI );

    // XProgressHandler
    virtual void SAL_CALL push( const ::com::sun::star::uno::Any& Status )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL update( const ::com::sun::star::uno::Any& Status )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL pop()
        throw (::com::sun::star::uno::RuntimeException);
};

}   // namespace utl

#endif // _UTL_PROGRESSHANDLERWRAP_HXX_

