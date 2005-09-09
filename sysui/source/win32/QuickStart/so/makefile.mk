#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 11:52:22 $
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

PRJNAME=sysui
TARGET=soquickstart
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Resources ----------------------------------------------------

RCFILES=QuickStart.rc
INCPRE=..

# --- Files --------------------------------------------------------

OBJFILES=$(OBJ)$/QuickStart.obj

APP1OBJS=$(OBJFILES)
APP1NOSAL=TRUE
APP1TARGET=$(TARGET)
APP1STDLIBS=shell32.lib\
            ole32.lib\
            gdi32.lib\
            oleaut32.lib\
            comdlg32.lib\
            kernel32.lib\
            comsupp.lib\
            oleaut32.lib

APP1NOSVRES=$(RES)$/$(TARGET).res

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
