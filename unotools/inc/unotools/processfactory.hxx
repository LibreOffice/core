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


#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#define _UNOTOOLS_PROCESSFACTORY_HXX_

#include "unotools/unotoolsdllapi.h"
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Reference.hxx>

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
} } } }

namespace utl
{

/**
 * This function set the process service factory.
 *
 * @author Juergen Schmidt
 */
UNOTOOLS_DLLPUBLIC void setProcessServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xSMgr);

/**
 * This function get the process service factory. If no service factory is set the function returns
 * a null interface.
 *
 * @author Juergen Schmidt
 */
UNOTOOLS_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getProcessServiceFactory();

}

#endif // _UNOTOOLS_PROCESSFACTORY_HXX_

