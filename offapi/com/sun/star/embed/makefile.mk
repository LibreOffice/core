#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssembed
PACKAGE=com$/sun$/star$/embed

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    Actions.idl\
    Aspects.idl\
    BaseStorage.idl\
    ElementModes.idl\
    EmbedStates.idl\
    EmbedVerbs.idl\
    EmbedMapUnits.idl\
    EmbedMisc.idl\
    EmbedUpdateModes.idl\
    EmbeddedObjectDescriptor.idl\
    EntryInitModes.idl\
    DocumentCloser.idl\
    FileSystemStorage.idl\
    FileSystemStorageFactory.idl\
    InsertedObjectInfo.idl\
    InstanceLocker.idl\
    Storage.idl\
    StorageStream.idl\
    StorageFactory.idl\
    StorageFormats.idl\
    VerbAttributes.idl\
    VisualRepresentation.idl\
    VerbDescriptor.idl\
    XActionsApproval.idl\
    XPersistanceHolder.idl\
    XEmbeddedObject.idl\
    XVisualObject.idl\
    XCommonEmbedPersist.idl\
    XEmbedPersist.idl\
    XLinkageSupport.idl\
    XClassifiedObject.idl\
    XInplaceObject.idl\
    XEmbeddedClient.idl\
    XEmbedObjectClipboardCreator.idl\
    XEmbedObjectCreator.idl\
    XEmbedObjectFactory.idl\
    XLinkCreator.idl\
    XLinkFactory.idl\
    XEncryptionProtectedSource.idl\
    XEncryptionProtectedSource2.idl\
    XInplaceClient.idl\
    XInsertObjectDialog.idl\
    XWindowSupplier.idl\
    XTransactedObject.idl\
    XTransactionBroadcaster.idl\
    XTransactionListener.idl\
    XRelationshipAccess.idl\
    XStateChangeBroadcaster.idl\
    XStateChangeListener.idl\
    XTransferableSupplier.idl\
    XComponentSupplier.idl\
    XStorage.idl\
    XStorage2.idl\
    XStorageRawAccess.idl\
    XExtendedStorageStream.idl\
    XHierarchicalStorageAccess.idl\
    XHierarchicalStorageAccess2.idl\
    XHatchWindowController.idl\
    XHatchWindowFactory.idl\
    XHatchWindow.idl\
    XPackageStructureCreator.idl\
    XOptimizedStorage.idl\
    UnreachableStateException.idl\
    UseBackupException.idl\
    StateChangeInProgressException.idl\
    WrongStateException.idl\
    NoVisualAreaSizeException.idl\
    ObjectSaveVetoException.idl\
    InvalidStorageException.idl\
    LinkageMisuseException.idl\
    NeedsRunningStateException.idl\
    StorageWrappedTargetException.idl\
    OLESimpleStorage.idl\
    XOLESimpleStorage.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
