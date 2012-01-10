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

TARGET=csstask
PACKAGE=com$/sun$/star$/task

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    ClassifiedInteractionRequest.idl\
    DocumentMacroConfirmationRequest.idl\
    DocumentPasswordRequest.idl\
    DocumentPasswordRequest2.idl\
    DocumentMSPasswordRequest.idl\
    DocumentMSPasswordRequest2.idl\
    ErrorCodeRequest.idl\
    ErrorCodeIOException.idl\
    FutureDocumentVersionProductUpdateRequest.idl\
    InteractionClassification.idl\
    InteractionHandler.idl\
    JobExecutor.idl\
    Job.idl\
    AsyncJob.idl\
    MasterPasswordRequest.idl\
    NoMasterException.idl\
    PasswordContainer.idl\
    PasswordContainerInteractionHandler.idl\
    PasswordRequest.idl\
    PasswordRequestMode.idl\
    PDFExportException.idl\
    OfficeRestartManager.idl\
    UnsupportedOverwriteRequest.idl\
    UrlRecord.idl\
    UserRecord.idl\
    XAsyncJob.idl\
    XInteractionApprove.idl\
    XInteractionAskLater.idl\
    XInteractionDisapprove.idl\
    XInteractionPassword.idl\
    XInteractionPassword2.idl\
    XJob.idl\
    XJobExecutor.idl\
    XJobListener.idl\
    XMasterPasswordHandling.idl\
    XMasterPasswordHandling2.idl\
    XPasswordContainer.idl\
    XRestartManager.idl\
    XStatusIndicator.idl\
    XStatusIndicatorFactory.idl\
    XStatusIndicatorSupplier.idl\
    XAbortChannel.idl\
    XInteractionRequestStringResolver.idl\
    InteractionRequestStringResolver.idl\
    XUrlContainer.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
