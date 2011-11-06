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



#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <com/sun/star/document/BrokenPackageRequest.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>

class RequestPackageReparation_Impl;
class SFX2_DLLPUBLIC RequestPackageReparation
{
    RequestPackageReparation_Impl* pImp;
public:
    RequestPackageReparation( ::rtl::OUString aName );
    ~RequestPackageReparation();
    sal_Bool    isApproved();
    com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionRequest > GetRequest();
};

class NotifyBrokenPackage_Impl;
class SFX2_DLLPUBLIC NotifyBrokenPackage
{
    NotifyBrokenPackage_Impl* pImp;
public:
    NotifyBrokenPackage( ::rtl::OUString aName );
    ~NotifyBrokenPackage();
    sal_Bool    isAborted();
    com::sun::star::uno::Reference < ::com::sun::star::task::XInteractionRequest > GetRequest();
};

