#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

PRJ=..$/..

PRJNAME=svx
TARGET=form
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CDEFS+=-DENABLE_VBA

SRS1NAME=form
SRC1FILES= \
    fmexpl.src 		\
    filtnav.src		\
    fmstring.src 	\
    formshell.src   \
    datanavi.src

LIB1TARGET= $(SLB)$/$(TARGET)-core.lib
LIB1OBJFILES= \
    $(SLO)$/formtoolbars.obj \
    $(SLO)$/fmdocumentclassification.obj \
    $(SLO)$/fmcontrolbordermanager.obj \
    $(SLO)$/fmcontrollayout.obj \
    $(SLO)$/fmtextcontroldialogs.obj \
    $(SLO)$/fmtextcontrolfeature.obj \
    $(SLO)$/fmtextcontrolshell.obj \
    $(SLO)$/ParseContext.obj			\
    $(SLO)$/typeconversionclient.obj    \
    $(SLO)$/dbtoolsclient.obj      \
    $(SLO)$/sqlparserclient.obj      \
    $(SLO)$/dataaccessdescriptor.obj      \
    $(SLO)$/fmpage.obj      \
    $(SLO)$/fmundo.obj  	\
    $(SLO)$/fmmodel.obj		\
    $(SLO)$/navigatortree.obj      \
    $(SLO)$/navigatortreemodel.obj      \
    $(SLO)$/fmexpl.obj      \
    $(SLO)$/formcontroller.obj    \
    $(SLO)$/fmpgeimp.obj	\
    $(SLO)$/fmvwimp.obj     \
    $(SLO)$/fmdpage.obj		\
    $(SLO)$/fmitems.obj  	\
    $(SLO)$/fmobj.obj      	\
    $(SLO)$/fmdmod.obj		\
    $(SLO)$/fmservs.obj     \
    $(SLO)$/typemap.obj		\
    $(SLO)$/fmexch.obj      \
    $(SLO)$/fmtools.obj     \
    $(SLO)$/fmshimp.obj     \
    $(SLO)$/fmshell.obj     \
    $(SLO)$/fmview.obj		\
    $(SLO)$/sdbdatacolumn.obj \
    $(SLO)$/formcontrolling.obj \
    $(SLO)$/formfeaturedispatcher.obj  \
    $(SLO)$/formdispatchinterceptor.obj  \
    $(SLO)$/datanavi.obj \
    $(SLO)$/xfm_addcondition.obj \
    $(SLO)$/datalistener.obj \
    $(SLO)$/fmscriptingenv.obj \
    $(SLO)$/stringlistresource.obj \
    $(SLO)$/delayedevent.obj \
    $(SLO)$/formcontrolfactory.obj \
    $(SLO)$/legacyformcontroller.obj

LIB2TARGET= $(SLB)$/$(TARGET).lib
LIB2OBJFILES= \
    $(SLO)$/dbcharsethelper.obj			\
    $(SLO)$/fmPropBrw.obj			\
    $(SLO)$/tbxform.obj     \
    $(SLO)$/fmobjfac.obj	\
    $(SLO)$/fmsrccfg.obj	\
    $(SLO)$/tabwin.obj      \
    $(SLO)$/filtnav.obj		\
    $(SLO)$/fmsrcimp.obj	\
    $(SLO)$/databaselocationinput.obj

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

# --- Targets ----------------------------------

.INCLUDE :  target.mk

