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



#ifndef _XMLOFF_XMLREG_HXX
#define _XMLOFF_XMLREG_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

/**
 *  Register subset of UNO services from XML Office library. This is necessary when
 *  linking against the static "xol.lib".
 *
 *  @return returns sal_False if at least one component could not be registered.
 */
sal_Bool XMLRegisterServices( ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > ); /// [all] ServiceProvider to register in.

#endif  //  _XMLOFF_XMLREG_HXX

