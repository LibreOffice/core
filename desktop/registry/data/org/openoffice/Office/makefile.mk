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

PRJ=..$/..$/..$/..$/..
PRJNAME=setup_native
TARGET=data_registration
PACKAGE=org.openoffice.Office

ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""

# --- Files  -------------------------------------------------------

.IF "$(BUILD_SPECIAL)"!=""

XCUFILES= \
    Jobs.xcu

MODULEFILES=

LOCALIZEDFILES=

PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

MYXCUFILES= \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-registration.xcu

.ELSE # "$(BUILD_SPECIAL)"!=""

dummy:
    @echo "Nothing to build"

.ENDIF # "$(BUILD_SPECIAL)"!=""

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # L10N_framework
ALLTAR : $(MYXCUFILES)

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-registration.xcu : $(PROCESSOUT)$/registry$/data$/$/$(PACKAGEDIR)$/Jobs.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

