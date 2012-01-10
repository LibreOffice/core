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

TARGET=cssutil
PACKAGE=com$/sun$/star$/util

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    MeasureUnit.idl\
    AliasProgrammaticPair.idl \
    CellProtection.idl\
    ChangesEvent.idl\
    ChangesSet.idl \
    CloseVetoException.idl\
    Color.idl\
    Date.idl\
    DateTime.idl\
    DateTimeRange.idl\
    DiskFullException.idl\
    Duration.idl\
    ElementChange.idl \
    Endianness.idl \
    FileIOException.idl\
    InvalidStateException.idl\
    Language.idl\
    NumberFormat.idl\
    NumberFormatProperties.idl\
    NumberFormats.idl\
    NumberFormatSettings.idl\
    NumberFormatsSupplier.idl\
    NumberFormatter.idl\
    ModeChangeEvent.idl\
    ReplaceDescriptor.idl\
    SearchDescriptor.idl\
    Sortable.idl\
    SortDescriptor.idl\
    SortDescriptor2.idl\
    SortField.idl\
    SortFieldType.idl\
    TextSearch.idl\
    Time.idl\
    TriState.idl\
    URL.idl\
    URLTransformer.idl\
    XArchiver.idl\
    XCancellable.idl\
    XCancelManager.idl\
    XChainable.idl\
    XChangesBatch.idl \
    XChangesListener.idl \
    XChangesNotifier.idl \
    XChangesSet.idl	\
    XCloneable.idl\
    XCloseable.idl\
    XCloseBroadcaster.idl\
    XCloseListener.idl\
    XFlushable.idl\
    XFlushListener.idl\
    XImportable.idl\
    XIndent.idl\
    XLinkUpdate.idl\
    XLocalizedAliases.idl\
    XMergeable.idl\
    XModeChangeBroadcaster.idl\
    XModeChangeListener.idl\
    XModeChangeApproveListener.idl\
    XModeSelector.idl\
    XModifiable.idl\
    XModifiable2.idl\
    XModifyBroadcaster.idl\
    XBroadcaster.idl\
    XModifyListener.idl\
    XNumberFormatPreviewer.idl\
    XNumberFormats.idl\
    XNumberFormatsSupplier.idl\
    XNumberFormatter.idl\
    XNumberFormatTypes.idl\
    XPropertyReplace.idl\
    XProtectable.idl\
    XRefreshable.idl\
    XRefreshListener.idl\
    XReplaceable.idl\
    XReplaceDescriptor.idl\
    XSearchable.idl\
    XSearchDescriptor.idl\
    XSimpleErrorHandler.idl\
    XSortable.idl\
    XStringAbbreviation.idl\
    XStringWidth.idl\
    XStringMapping.idl\
    XStringEscape.idl\
    XTextSearch.idl\
    XUpdatable.idl\
    XURLTransformer.idl\
    XUniqueIDFactory.idl\
    VetoException.idl\
    DataEditorEventType.idl\
    DataEditorEvent.idl\
    XDataEditorListener.idl\
    XDataEditor.idl\
    MalformedNumberFormatException.idl\
    NotNumericException.idl\
    XAtomServer.idl\
    AtomClassRequest.idl\
    AtomDescription.idl\
    XStringSubstitution.idl\
    PathSettings.idl\
    PathSubstitution.idl\
    XTimeStamped.idl\
    OfficeInstallationDirectories.idl\
    XOfficeInstallationDirectories.idl\
    RevisionTag.idl\
    UriAbbreviation.idl\
    XJobManager.idl\
    JobManager.idl\
    XLockable.idl\
    NotLockedException.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
