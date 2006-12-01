#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:01:50 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
    PolicyLayerBackend.idl  \
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
