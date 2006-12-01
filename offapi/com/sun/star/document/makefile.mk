#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.39 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:03:38 $
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
    DocumentInfo.idl\
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
    MacroExecMode.idl\
    MediaDescriptor.idl\
    NoSuchFilterRequest.idl\
    OfficeDocument.idl\
    OleEmbeddedServerRegistration.idl\
    PrinterIndependentLayout.idl\
    RedlineDisplayType.idl \
    Settings.idl\
    StandaloneDocumentInfo.idl\
    TypeDetection.idl\
    UpdateDocMode.idl\
    XActionLockable.idl\
    XBinaryStreamResolver.idl\
    XDocumentInfo.idl\
    XDocumentInfoSupplier.idl\
    XDocumentInsertable.idl\
    XDocumentSubStorageSupplier.idl\
    XEmbeddedObjectResolver.idl\
    XEmbeddedObjectSupplier.idl\
    XEmbeddedObjectSupplier2.idl\
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
    XRedlinesSupplier.idl \
    XStandaloneDocumentInfo.idl\
    XStorageBasedDocument.idl\
    XStorageChangeListener.idl\
    XTypeDetection.idl\
    XViewDataSupplier.idl\
        LinkUpdateModes.idl\
        XDocumentRevisionListPersistence.idl\
        DocumentRevisionListPersistence.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
