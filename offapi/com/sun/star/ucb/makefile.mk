#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.39 $
#
#   last change: $Author: hr $ $Date: 2003-03-26 13:57:56 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http:#www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=api

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
    ChaosContent.idl\
    ChaosContentProvider.idl\
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
    DataContainer.idl\
    DefaultHierarchyDataSource.idl\
    DocumentHeaderField.idl\
    DocumentStoreMode.idl\
    DuplicateCommandIdentifierException.idl\
    DuplicateProviderException.idl\
    DynamicResultSet.idl\
    Error.idl\
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
     PropertyMatcherFactory.idl\
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
     XSortedDynamicResultSetFactory.idl\
     XSourceInitialization.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
