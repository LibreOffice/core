#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
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

PRJ=..$/..$/..$/..$/..
PRJNAME=setup_native
TARGET=data_stclient
PACKAGE=org.openoffice.Office

ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

.INCLUDE :  settings.mk

# --- Files  -------------------------------------------------------

.IF "$(BUILD_SPECIAL)"!=""

.IF "$(OS)"!="MACOSX"
XCUFILES= \
    Jobs.xcu
.ELSE
dummy:
    @echo "Nothing to build"
.ENDIF

MODULEFILES=

LOCALIZEDFILES=

PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

MYXCUFILES= \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-stclient.xcu

.ELSE # "$(BUILD_SPECIAL)"!=""

dummy:
    @echo "Nothing to build"

.ENDIF # "$(BUILD_SPECIAL)"!=""

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MYXCUFILES)

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-stclient.xcu : $(PROCESSOUT)$/registry$/data$/$/$(PACKAGEDIR)$/Jobs.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

