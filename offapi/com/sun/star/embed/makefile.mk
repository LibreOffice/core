#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: obo $ $Date: 2008-01-04 16:16:46 $
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
    XStorageRawAccess.idl\
    XExtendedStorageStream.idl\
    XHierarchicalStorageAccess.idl\
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
