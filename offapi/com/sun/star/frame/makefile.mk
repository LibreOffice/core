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
# $Revision: 1.35 $
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
    DispatchResultState.idl\
    DispatchResultEvent.idl\
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
    FramesContainer.idl\
    FrameSearchFlag.idl\
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
    SynchronousFrameLoader.idl\
    StatusbarController.idl\
    StatusbarControllerFactory.idl\
    Task.idl\
    TemplateAccess.idl\
    TerminationVetoException.idl\
    TitleChangedEvent.idl\
    ToolbarController.idl\
    TransientDocumentsDocumentContentFactory.idl \
    UnknownModuleException.idl\
    WindowArrange.idl\
    XBorderResizeListener.idl\
    XBrowseHistoryRegistry.idl\
    XComponentLoader.idl\
    XComponentRegistry.idl\
    XConfigManager.idl\
    XController.idl\
    XControlNotificationListener.idl\
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
    XFrames.idl\
    XFrameSetModel.idl\
    XFramesSupplier.idl\
    XInplaceLayout.idl\
    XInterceptorInfo.idl\
    XLayoutManager.idl\
    XLayoutManagerEventBroadcaster.idl\
    XLayoutManagerListener.idl\
    XLoadable.idl\
    XLoadEventListener.idl\
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
    XStatusListener.idl\
    XStatusbarController.idl\
    XStorable.idl\
    XSubToolbarController.idl\
    XStorable2.idl\
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
    XUIControllerRegistration.idl\
    XUntitledNumbers.idl\
    XUrlList.idl\
    XWindowArranger.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
