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
# $Revision: 1.14 $
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
