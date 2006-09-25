#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: vg $ $Date: 2006-09-25 12:52:28 $
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

CDEFS+=-D_WIN32_WINNT=0x0500 -D_UNICODE -DUNICODE

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
         $(SLO)$/dialogcustomcontrols.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
