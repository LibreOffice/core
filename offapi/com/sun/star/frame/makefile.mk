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

TARGET=cssframe
PACKAGE=com$/sun$/star$/frame

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    BorderWidths.idl\
    CommandGroup.idl\
    Components.idl\
    ContentHandler.idl\
    ContentHandlerFactory.idl\
    ControlCommand.idl\
    ControlEvent.idl\
    Controller.idl\
    Desktop.idl\
    DesktopTask.idl\
    DesktopTasks.idl\
    DispatchDescriptor.idl\
    DispatchHelper.idl\
    DispatchInformation.idl\
    DispatchProvider.idl\
    DispatchRecorder.idl\
    DispatchRecorderSupplier.idl\
    DispatchResultEvent.idl\
    DispatchResultState.idl\
    DispatchStatement.idl\
    DocumentTemplates.idl\
    DoubleInitializationException.idl\
    FeatureStateEvent.idl\
    Frame.idl\
    FrameAction.idl\
    FrameActionEvent.idl\
    FrameControl.idl\
    FrameLoader.idl\
    FrameLoaderFactory.idl\
    FrameSearchFlag.idl\
    FramesContainer.idl\
    IllegalArgumentIOException.idl\
    LayoutManager.idl\
    LayoutManagerEvents.idl\
    MediaTypeDetectionHelper.idl\
    ModuleManager.idl\
    PopupMenuController.idl\
    PopupMenuControllerFactory.idl\
    ProtocolHandler.idl\
    SessionManager.idl\
    Settings.idl\
    StatusbarController.idl\
    StatusbarControllerFactory.idl\
    SynchronousFrameLoader.idl\
    Task.idl\
    TemplateAccess.idl\
    TerminationVetoException.idl\
    TitleChangedEvent.idl\
    ToolbarController.idl\
    ToolbarControllerFactory.idl \
    TransientDocumentsDocumentContentFactory.idl \
    UnknownModuleException.idl\
    WindowArrange.idl\
    XBorderResizeListener.idl\
    XBrowseHistoryRegistry.idl\
    XComponentLoader.idl\
    XComponentRegistry.idl\
    XConfigManager.idl\
    XControlNotificationListener.idl\
    XController.idl\
    XController2.idl\
    XControllerBorder.idl\
    XDesktop.idl\
    XDesktopTask.idl\
    XDispatch.idl\
    XDispatchHelper.idl\
    XDispatchInformationProvider.idl\
    XDispatchProvider.idl\
    XDispatchProviderInterception.idl\
    XDispatchProviderInterceptor.idl\
    XDispatchRecorder.idl\
    XDispatchRecorderSupplier.idl\
    XDispatchResultListener.idl\
    XDocumentTemplates.idl\
    XExtendedFilterDetection.idl\
    XFilterDetect.idl\
    XFrame.idl\
    XFrameActionListener.idl\
    XFrameLoader.idl\
    XFrameLoaderQuery.idl\
    XFrameSetModel.idl\
    XFrames.idl\
    XFramesSupplier.idl\
    XInplaceLayout.idl\
    XInterceptorInfo.idl\
    XLayoutManager.idl\
    XLayoutManagerEventBroadcaster.idl\
    XLayoutManagerListener.idl\
    XLoadEventListener.idl\
    XLoadable.idl\
    XMenuBarAcceptor.idl\
    XMenuBarMergingAcceptor.idl\
    XModel.idl\
    XModel2.idl\
    XModule.idl\
    XModuleManager.idl\
    XNotifyingDispatch.idl\
    XPopupMenuController.idl\
    XRecordableDispatch.idl\
    XSessionManagerClient.idl\
    XSessionManagerListener.idl\
    XSessionManagerListener2.idl\
    XStatusListener.idl\
    XStatusbarController.idl\
    XStorable.idl\
    XStorable2.idl\
    XSubToolbarController.idl\
    XSynchronousDispatch.idl\
    XSynchronousFrameLoader.idl\
    XTask.idl\
    XTasksSupplier.idl\
    XTerminateListener.idl\
    XTerminateListener2.idl\
    XTitle.idl\
    XTitleChangeBroadcaster.idl\
    XTitleChangeListener.idl\
    XToolbarController.idl\
    XTransientDocumentsDocumentContentFactory.idl\
    XUIControllerFactory.idl\
    XUIControllerRegistration.idl\
    XUntitledNumbers.idl\
    XUrlList.idl\
    XWindowArranger.idl \

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
