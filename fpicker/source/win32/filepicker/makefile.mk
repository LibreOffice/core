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
# $Revision: 1.15 $
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
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

CDEFS+=-D_UNICODE
CDEFS+=-DUNICODE
#CDEFS+=-DWIN32_LEAN_AND_MEAN
#CDEFS+=-DWIN32

CDEFS+=-Ic:\progra~1\micros~3\windows\v6.0\include
CDEFS+=-If:\lu\n\msvc8p\atlmfc\include

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
         $(SLO)$/VistaFilePicker.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
