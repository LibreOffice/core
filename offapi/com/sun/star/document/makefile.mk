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

TARGET=cssdocument
PACKAGE=com$/sun$/star$/document

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AmbigousFilterRequest.idl\
    BrokenPackageRequest.idl\
    CorruptedFilterConfigurationException.idl\
    ChangedByOthersRequest.idl\
    DocumentEvent.idl\
    DocumentInfo.idl\
    DocumentProperties.idl\
    EventDescriptor.idl\
    EventObject.idl\
    Events.idl\
    ExportFilter.idl\
    ExtendedTypeDetection.idl\
    ExtendedTypeDetectionFactory.idl\
    FilterAdapter.idl\
    FilterFactory.idl\
    FilterOptionsRequest.idl\
    HeaderFooterSettings.idl\
    ImportFilter.idl\
    LinkTarget.idl\
    LinkTargets.idl\
    LockFileIgnoreRequest.idl\
    LockedDocumentRequest.idl\
    LockedOnSavingRequest.idl\
    MacroExecMode.idl\
    MediaDescriptor.idl\
    NoSuchFilterRequest.idl\
    OfficeDocument.idl\
    OleEmbeddedServerRegistration.idl\
    OwnLockOnDocumentRequest.idl\
    PDFDialog.idl\
    PrinterIndependentLayout.idl\
    RedlineDisplayType.idl \
    Settings.idl\
    StandaloneDocumentInfo.idl\
    TypeDetection.idl\
    UpdateDocMode.idl\
    XActionLockable.idl\
    XBinaryStreamResolver.idl\
    XDocumentEventBroadcaster.idl\
    XDocumentEventListener.idl\
    XDocumentInfo.idl\
    XDocumentInfoSupplier.idl\
    XDocumentInsertable.idl\
    XDocumentProperties.idl\
    XDocumentPropertiesSupplier.idl\
    XDocumentRecovery.idl\
    XDocumentSubStorageSupplier.idl\
    XEmbeddedObjectResolver.idl\
    XEmbeddedObjectSupplier.idl\
    XEmbeddedObjectSupplier2.idl\
    XEmbeddedScripts.idl\
    XEventBroadcaster.idl\
    XEventListener.idl\
    XEventsSupplier.idl\
    XExporter.idl\
    XExtendedFilterDetection.idl\
    XFilter.idl\
    XFilterAdapter.idl\
    XGraphicObjectResolver.idl\
    XImporter.idl\
    XInteractionFilterOptions.idl\
    XInteractionFilterSelect.idl\
    XLinkTargetSupplier.idl\
    XMimeTypeInfo.idl\
    XOOXMLDocumentPropertiesImporter.idl\
    XRedlinesSupplier.idl \
    XScriptInvocationContext.idl\
    XStandaloneDocumentInfo.idl\
    XStorageBasedDocument.idl\
    XStorageChangeListener.idl\
    XTypeDetection.idl\
    XViewDataSupplier.idl\
    LinkUpdateModes.idl\
    XDocumentRevisionListPersistence.idl\
    DocumentRevisionListPersistence.idl \
    XDocumentLanguages.idl \
    XCodeNameQuery.idl \
    XUndoAction.idl \
    XUndoManager.idl \
    XUndoManagerListener.idl \
    XUndoManagerSupplier.idl \
    UndoManagerEvent.idl \
    EmptyUndoStackException.idl \
    UndoContextNotClosedException.idl \
    UndoFailedException.idl \


# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
