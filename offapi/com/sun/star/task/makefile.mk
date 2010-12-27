#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
# Copyright IBM Corporation 2009.
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
