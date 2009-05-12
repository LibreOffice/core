#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.9.102.1 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

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
