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

TARGET=cssform
PACKAGE=com$/sun$/star$/form

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    modules.idl\
    DataAwareControlModel.idl\
    DatabaseDeleteEvent.idl\
    DatabaseParameterEvent.idl\
    DataSelectionType.idl\
    ErrorEvent.idl\
    FormButtonType.idl\
    FormComponent.idl\
    FormComponents.idl\
    FormComponentType.idl\
    FormController.idl\
    FormControllerDispatcher.idl\
    FormControlModel.idl\
    Forms.idl\
    FormSubmitEncoding.idl\
    FormSubmitMethod.idl\
    ListSourceType.idl\
    NavigationBarMode.idl\
    PropertyBrowserController.idl\
    TabulatorCycle.idl\
    XApproveActionBroadcaster.idl\
    XApproveActionListener.idl\
    XBoundComponent.idl\
    XBoundControl.idl\
    XChangeBroadcaster.idl\
    XChangeListener.idl\
    XConfirmDeleteBroadcaster.idl\
    XConfirmDeleteListener.idl\
    XDatabaseParameterBroadcaster.idl\
    XDatabaseParameterBroadcaster2.idl\
    XDatabaseParameterListener.idl\
    XDeleteListener.idl\
    XErrorBroadcaster.idl\
    XErrorListener.idl\
    XForm.idl\
    XFormComponent.idl\
    XFormController.idl\
    XFormControllerListener.idl\
    XFormsSupplier.idl\
    XFormsSupplier2.idl\
    XGrid.idl\
    XGridColumnFactory.idl\
    XGridControl.idl\
    XGridControlListener.idl\
    XGridFieldDataSupplier.idl\
    XGridPeer.idl\
    XImageProducerSupplier.idl\
    XInsertListener.idl\
    XLoadListener.idl\
    XPositioningListener.idl\
    XReset.idl\
    XResetListener.idl\
    XRestoreListener.idl\
    XSubmit.idl\
    XSubmitListener.idl\
    XUpdateBroadcaster.idl\
    XUpdateListener.idl\
    XLoadable.idl\


# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
