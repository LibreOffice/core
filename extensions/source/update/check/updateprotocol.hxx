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



#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/deployment/UpdateInformationProvider.hpp>

#include <vector>
#include "updateinfo.hxx"

// Returns 'true' if successfully connected to the update server
bool checkForUpdates(
    UpdateInfo& o_rUpdateInfo,
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& rxInteractionHandler,
    const ::com::sun::star::uno::Reference< ::com::sun::star::deployment::XUpdateInformationProvider >& rxProvider
);

// Returns 'true' if there are updates for any extension
bool checkForExtensionUpdates(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
);

bool checkForPendingUpdates(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext
);

bool storeExtensionUpdateInfos(
    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< rtl::OUString > > &rUpdateInfos
);

