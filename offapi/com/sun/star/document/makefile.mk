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
    XCompatWriterDocProperties.idl \
    XVbaMethodParameter.idl \
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
