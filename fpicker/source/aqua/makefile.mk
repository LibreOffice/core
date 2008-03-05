#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:39:30 $
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

PRJ=..$/..

PRJNAME=fpicker
TARGET=fps_aqua.uno
TARGETTYPE=GUI
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

DLLPRE=

# ------------------------------------------------------------------

.IF "$(GUIBASE)" != "aqua"

dummy:
    @echo "Nothing to build. GUIBASE == $(GUIBASE)"

.ELSE # we build for aqua


# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/resourceprovider.obj       \
        $(SLO)$/FPentry.obj                \
        $(SLO)$/SalAquaPicker.obj          \
        $(SLO)$/SalAquaFilePicker.obj      \
        $(SLO)$/SalAquaFolderPicker.obj    \
        $(SLO)$/CFStringUtilities.obj      \
        $(SLO)$/FilterHelper.obj           \
        $(SLO)$/ControlHelper.obj          \
        $(SLO)$/NSString_OOoAdditions.obj  \
        $(SLO)$/NSURL_OOoAdditions.obj     \
        $(SLO)$/AquaFilePickerDelegate.obj

SHL1NOCHECK=TRUE
SHL1TARGET= $(TARGET)
SHL1OBJS=   $(SLOFILES)
SHL1STDLIBS=\
    $(VCLLIB) \
    $(TOOLSLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

SHL1VERSIONMAP=exports.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF # "$(GUIBASE)" != "aqua"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
