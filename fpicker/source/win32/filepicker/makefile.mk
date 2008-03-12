#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 07:34:51 $
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
