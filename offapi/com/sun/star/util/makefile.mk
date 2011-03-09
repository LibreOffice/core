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
