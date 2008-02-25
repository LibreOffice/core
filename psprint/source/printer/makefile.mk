#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:28:59 $
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

ENABLE_EXCEPTIONS=TRUE
PRJNAME=psprint
TARGET=printer

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_CUPS)" != ""
CDEFS += -DENABLE_CUPS
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"=="aqua"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"=="aqua"

SLOFILES=\
    $(SLO)$/printerinfomanager.obj	\
    $(SLO)$/jobdata.obj				\
    $(SLO)$/cupsmgr.obj

.ENDIF # GUIBASE = aqua

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

XSALSETLIBNAME=$(DLLPRE)spa$(DLLPOSTFIX)$(DLLPOST)

$(INCCOM)$/rtsname.hxx:
    rm -f $(INCCOM)$/rtsname.hxx ; \
    echo "#define _XSALSET_LIBNAME "\"$(XSALSETLIBNAME)\" > $(INCCOM)$/rtsname.hxx

$(SLO)$/cupsmgr.obj : $(INCCOM)$/rtsname.hxx

