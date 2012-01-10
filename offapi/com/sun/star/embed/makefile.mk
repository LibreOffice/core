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
    XEncryptionProtectedStorage.idl\
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
