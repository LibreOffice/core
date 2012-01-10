#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ = ..$/..$/..$/..$/..
PRJNAME=offapi

TARGET = csscbackend
PACKAGE = com$/sun$/star$/configuration$/backend

.INCLUDE: $(PRJ)$/util$/makefile.pmk

IDLFILES = \
    Backend.idl				\
    SingleBackend.idl	    \
    LocalSingleBackend.idl	\
    LdapSingleBackend.idl	\
    BackendAdapter.idl		\
    SingleBackendAdapter.idl \
    OfflineBackend.idl		\
    OnlineBackend.idl		\
    HierarchyBrowser.idl	\
    LocalHierarchyBrowser.idl	\
    DataImporter.idl		\
    LocalDataImporter.idl	\
    LayerFilter.idl	\
    LayerUpdateMerger.idl	\
    Importer.idl			\
    MergeImporter.idl		\
    CopyImporter.idl		\
    Schema.idl				\
    Layer.idl				\
    UpdatableLayer.idl      \
    SingleLayerStratum.idl  \
    LocalSingleStratum.idl  \
    LdapSingleStratum.idl	\
    MultiStratumBackend.idl \
    MultiLayerStratum.idl   \
    LdapMultiLayerStratum.idl \
    SchemaSupplier.idl	    \
    LocalSchemaSupplier.idl \
    PlatformBackend.idl		\
    SystemIntegration.idl	\
    LayerDescriber.idl		\
    InteractionHandler.idl  \
    XBackend.idl			\
    XBackendEntities.idl	\
    XSchemaSupplier.idl		\
    XVersionedSchemaSupplier.idl		\
    XSingleLayerStratum.idl	\
    XMultiLayerStratum.idl	\
    XSchema.idl				\
    XLayer.idl				\
    XCompositeLayer.idl     \
    XUpdatableLayer.idl     \
    XSchemaHandler.idl		\
    XLayerHandler.idl		\
    XUpdateHandler.idl      \
    XLayerImporter.idl		\
    XBackendChangesNotifier.idl	\
    XBackendChangesListener.idl	\
    XLayerContentDescriber.idl	\
    ComponentChangeEvent.idl	\
    TemplateIdentifier.idl  \
    SchemaAttribute.idl     \
    NodeAttribute.idl       \
    PropertyInfo.idl			\
    MergeRecoveryRequest.idl	\
    MalformedDataException.idl	\
    BackendSetupException.idl	\
    StratumCreationException.idl	\
    AuthenticationFailedException.idl	\
    CannotConnectException.idl	\
    InvalidAuthenticationMechanismException.idl \
    BackendAccessException.idl	\
    ConnectionLostException.idl	\
    InsufficientAccessRightsException.idl

.INCLUDE: target.mk
.INCLUDE: $(PRJ)$/util$/target.pmk
