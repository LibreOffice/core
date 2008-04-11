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
# $Revision: 1.48 $
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

TARGET=cssucb
PACKAGE=com$/sun$/star$/ucb

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AlreadyInitializedException.idl\
    AnyCompareFactory.idl\
    AuthenticationRequest.idl\
    CachedContentResultSet.idl\
    CachedContentResultSetFactory.idl\
    CachedContentResultSetStub.idl\
    CachedContentResultSetStubFactory.idl\
    CachedDynamicResultSet.idl\
    CachedDynamicResultSetFactory.idl\
    CachedDynamicResultSetStub.idl\
    CachedDynamicResultSetStubFactory.idl\
    CHAOSProgressStart.idl\
    Command.idl\
    CommandAbortedException.idl\
    CommandFailedException.idl\
    CommandInfo.idl\
    CommandInfoChange.idl\
    CommandInfoChangeEvent.idl\
    ConnectionMode.idl\
    Content.idl\
    ContentAction.idl\
    ContentCreationError.idl\
    ContentCreationException.idl\
    ContentEvent.idl\
    ContentInfo.idl\
    ContentInfoAttribute.idl\
    ContentProvider.idl\
    ContentProviderInfo.idl\
    ContentProviderProxy.idl\
    ContentProviderProxyFactory.idl\
    ContentResultSet.idl\
    ContentResultSetCapability.idl\
    ContentTransmitter.idl\
    Cookie.idl\
    CookiePolicy.idl\
    CookieRequest.idl\
    CrossReference.idl\
    DefaultHierarchyDataSource.idl\
    DocumentHeaderField.idl\
    DocumentStoreMode.idl\
    DuplicateCommandIdentifierException.idl\
    DuplicateProviderException.idl\
    DynamicResultSet.idl\
    Error.idl\
    ExpandContentProvider.idl\
    ExportStreamInfo.idl\
    FetchError.idl\
    FetchResult.idl\
    FileContent.idl\
    FileContentProvider.idl\
    FileSystemNotation.idl\
    FolderList.idl\
    FolderListCommand.idl\
    FolderListEntry.idl\
    FTPContent.idl\
    FTPContentProvider.idl\
    GlobalTransferCommandArgument.idl\
    GnomeVFSContentProvider.idl\
    GnomeVFSDocumentContent.idl\
    GnomeVFSFolderContent.idl\
    HandleCookiesRequest.idl\
    HelpContent.idl\
    HelpContentProvider.idl\
    HierarchyContentProvider.idl\
    HierarchyDataReadAccess.idl\
    HierarchyDataReadWriteAccess.idl\
    HierarchyDataSource.idl\
    HierarchyFolderContent.idl\
    HierarchyLinkContent.idl\
    HierarchyRootFolderContent.idl\
    IllegalIdentifierException.idl\
    InsertCommandArgument.idl\
    InteractiveAppException.idl\
    InteractiveAugmentedIOException.idl\
    InteractiveBadTransferURLException.idl\
    InteractiveCHAOSException.idl\
    InteractiveFileIOException.idl\
    InteractiveIOException.idl\
    InteractiveNetworkConnectException.idl\
    InteractiveNetworkException.idl\
    InteractiveNetworkGeneralException.idl\
    InteractiveNetworkOffLineException.idl\
    InteractiveNetworkReadException.idl\
    InteractiveNetworkResolveNameException.idl\
    InteractiveNetworkWriteException.idl\
    InteractiveWrongMediumException.idl\
    IOErrorCode.idl\
    Link.idl\
    ListAction.idl\
    ListActionType.idl\
    ListenerAlreadySetException.idl\
    ListEvent.idl\
    Lock.idl\
    LockDepth.idl\
    LockEntry.idl\
    LockScope.idl\
    LockType.idl\
    MissingInputStreamException.idl\
    MissingPropertiesException.idl\
     NameClash.idl\
     NameClashException.idl\
     NameClashResolveRequest.idl\
     NumberedSortingInfo.idl\
     ODMAContent.idl\
     ODMAContentProvider.idl\
     OpenCommandArgument.idl\
     OpenCommandArgument2.idl\
     OpenMode.idl\
     OutgoingMessageState.idl\
     PackageContentProvider.idl\
     PackageFolderContent.idl\
     PackageStreamContent.idl\
     PersistentPropertySet.idl\
     PostCommandArgument.idl\
     PostCommandArgument2.idl\
     Priority.idl\
     PropertiesManager.idl\
     PropertySetRegistry.idl\
     PropertyValueInfo.idl\
     PropertyValueState.idl\
     RecipientInfo.idl\
     RememberAuthentication.idl\
     RemoteAccessContentProvider.idl\
     RemoteContentProviderAcceptor.idl\
     RemoteContentProviderChangeAction.idl\
     RemoteContentProviderChangeEvent.idl\
     RemoteProxyContentProvider.idl\
     ResultSetException.idl\
    Rule.idl\
     RuleAction.idl\
     RuleOperator.idl\
     RuleSet.idl\
     RuleTerm.idl\
     SearchCommandArgument.idl\
     SearchCriterium.idl\
     SearchInfo.idl\
     SearchRecursion.idl\
     SendInfo.idl\
     SendMediaTypes.idl\
     ServiceNotFoundException.idl\
     SimpleFileAccess.idl\
     SortedDynamicResultSetFactory.idl\
     SortingInfo.idl\
     Store.idl\
     SynchronizePolicy.idl\
     TransferCommandOperation.idl\
     TransferInfo.idl\
     TransferResult.idl\
    TransientDocumentsContentProvider.idl\
    TransientDocumentsRootContent.idl\
    TransientDocumentsDocumentContent.idl\
    TransientDocumentsFolderContent.idl\
    TransientDocumentsStreamContent.idl\
     UniversalContentBroker.idl\
     UnsupportedCommandException.idl\
     UnsupportedDataSinkException.idl\
     UnsupportedNameClashException.idl\
     UnsupportedOpenModeException.idl\
     VerificationMode.idl\
     WebDAVContentProvider.idl\
     WebDAVDocumentContent.idl\
     WebDAVFolderContent.idl\
     WelcomeDynamicResultSetStruct.idl\
     XAnyCompare.idl\
     XAnyCompareFactory.idl\
     XCachedContentResultSetFactory.idl\
     XCachedContentResultSetStubFactory.idl\
     XCachedDynamicResultSetFactory.idl\
     XCachedDynamicResultSetStubFactory.idl\
     XCommandEnvironment.idl\
     XCommandInfo.idl\
     XCommandInfoChangeListener.idl\
     XCommandInfoChangeNotifier.idl\
     XCommandProcessor.idl\
     XCommandProcessor2.idl\
     XContent.idl\
     XContentAccess.idl\
     XContentCreator.idl\
     XContentEventListener.idl\
     XContentIdentifier.idl\
     XContentIdentifierFactory.idl\
     XContentIdentifierMapping.idl\
     XContentProvider.idl\
     XContentProviderFactory.idl\
     XContentProviderManager.idl\
     XContentProviderSupplier.idl\
     XContentTransmitter.idl\
     XDataContainer.idl\
     XDynamicResultSet.idl\
     XDynamicResultSetListener.idl\
     XFetchProvider.idl\
     XFetchProviderForContentAccess.idl\
     XFileIdentifierConverter.idl\
     XInteractionCookieHandling.idl\
     XInteractionHandlerSupplier.idl\
     XInteractionReplaceExistingData.idl\
     XInteractionSupplyAuthentication.idl\
     XInteractionSupplyName.idl\
     XParameterizedContentProvider.idl\
     XPersistentPropertySet.idl\
     XProgressHandler.idl\
     XPropertyMatcher.idl\
     XPropertyMatcherFactory.idl\
     XPropertySetRegistry.idl\
     XPropertySetRegistryFactory.idl\
     XRecycler.idl\
     XRemoteContentProviderAcceptor.idl\
     XRemoteContentProviderActivator.idl\
     XRemoteContentProviderChangeListener.idl\
     XRemoteContentProviderChangeNotifier.idl\
     XRemoteContentProviderConnectionControl.idl\
     XRemoteContentProviderDistributor.idl\
     XRemoteContentProviderDoneListener.idl\
     XRemoteContentProviderSupplier.idl\
     XSimpleFileAccess.idl\
     XSimpleFileAccess2.idl\
     XSimpleFileAccess3.idl\
     XSortedDynamicResultSetFactory.idl\
    XSourceInitialization.idl\
    XWebDAVCommandEnvironment.idl \
    CertificateValidationRequest.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
