#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2006-12-01 17:04:23 $
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
