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

TARGET=cssucb
PACKAGE=com$/sun$/star$/ucb

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AlreadyInitializedException.idl\
    AnyCompareFactory.idl\
    AuthenticationRequest.idl\
    CHAOSProgressStart.idl\
    CachedContentResultSet.idl\
    CachedContentResultSetFactory.idl\
    CachedContentResultSetStub.idl\
    CachedContentResultSetStubFactory.idl\
    CachedDynamicResultSet.idl\
    CachedDynamicResultSetFactory.idl\
    CachedDynamicResultSetStub.idl\
    CachedDynamicResultSetStubFactory.idl\
    CertificateValidationRequest.idl\
    Command.idl\
    CommandAbortedException.idl\
    CommandEnvironment.idl\
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
    FTPContent.idl\
    FTPContentProvider.idl\
    FetchError.idl\
    FetchResult.idl\
    FileContent.idl\
    FileContentProvider.idl\
    FileSystemNotation.idl\
    FolderList.idl\
    FolderListCommand.idl\
    FolderListEntry.idl\
    GIOContentProvider.idl\
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
    IOErrorCode.idl\
    IllegalIdentifierException.idl\
    InsertCommandArgument.idl\
    InteractiveAppException.idl\
    InteractiveAugmentedIOException.idl\
    InteractiveBadTransferURLException.idl\
    InteractiveCHAOSException.idl\
    InteractiveFileIOException.idl\
    InteractiveIOException.idl\
    InteractiveLockingException.idl\
    InteractiveLockingLockNotAvailableException.idl\
    InteractiveLockingLockExpiredException.idl\
    InteractiveLockingLockedException.idl\
    InteractiveLockingNotLockedException.idl\
    InteractiveNetworkConnectException.idl\
    InteractiveNetworkException.idl\
    InteractiveNetworkGeneralException.idl\
    InteractiveNetworkOffLineException.idl\
    InteractiveNetworkReadException.idl\
    InteractiveNetworkResolveNameException.idl\
    InteractiveNetworkWriteException.idl\
    InteractiveWrongMediumException.idl\
    Link.idl\
    ListAction.idl\
    ListActionType.idl\
    ListEvent.idl\
    ListenerAlreadySetException.idl\
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
    PropertyCommandArgument.idl\
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
    TransientDocumentsDocumentContent.idl\
    TransientDocumentsFolderContent.idl\
    TransientDocumentsRootContent.idl\
    TransientDocumentsStreamContent.idl\
    URLAuthenticationRequest.idl\
    UniversalContentBroker.idl\
    UnsupportedCommandException.idl\
    UnsupportedDataSinkException.idl\
    UnsupportedNameClashException.idl\
    UnsupportedOpenModeException.idl\
    VerificationMode.idl\
    WebDAVContentProvider.idl\
    WebDAVDocumentContent.idl\
    WebDAVFolderContent.idl\
    WebDAVHTTPMethod.idl \
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
    XInteractionSupplyAuthentication2.idl\
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

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
