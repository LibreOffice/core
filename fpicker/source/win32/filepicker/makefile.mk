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

PRJ=..$/..$/..

PRJNAME=fpicker
TARGET=fps
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

CDEFS+=-D_UNICODE
CDEFS+=-DUNICODE
CDEFS+=-U_WIN32_WINNT -D_WIN32_WINNT=0x0600

# --- Resources ----------------------------------------------------

.IF "$(GUI)" == "WNT"
RCFILES=fps.rc
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/FileOpenDlg.obj\
         $(SLO)$/FPentry.obj\
         $(SLO)$/FilePicker.obj\
         $(SLO)$/WinFileOpenImpl.obj\
         $(SLO)$/FilterContainer.obj\
         $(SLO)$/controlaccess.obj\
         $(SLO)$/dibpreview.obj\
         $(SLO)$/helppopupwindow.obj\
         $(SLO)$/controlcommand.obj\
         $(SLO)$/filepickerstate.obj\
         $(SLO)$/getfilenamewrapper.obj\
         $(SLO)$/asynceventnotifier.obj\
         $(SLO)$/previewadapter.obj\
         $(SLO)$/previewbase.obj\
         $(SLO)$/filepickereventnotification.obj\
         $(SLO)$/customcontrol.obj\
         $(SLO)$/customcontrolcontainer.obj\
         $(SLO)$/customcontrolfactory.obj\
         $(SLO)$/dialogcustomcontrols.obj\
         $(SLO)$/asyncrequests.obj\
         $(SLO)$/VistaFilePickerEventHandler.obj\
         $(SLO)$/VistaFilePickerImpl.obj\
         $(SLO)$/VistaFilePicker.obj\
         $(SLO)$/SolarMutex.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
