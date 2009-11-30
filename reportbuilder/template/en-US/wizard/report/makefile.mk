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
# $Revision: 1.3 $
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

PRJ=..$/..$/..$/..

PRJNAME=reportbuilder
TARGET=template_en_us
PACKAGE=

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCUFILES=

MODULEFILES=

OTR_FILES_IN_REPORT_BUILDER_EXT=\
    default.otr

# OTR_FILES_IN_TEMPLATE_EXT=\
# 	finance.odr \
# 	finance_2.odr \
# 	yellow.odr

ALLTAR: COPYFILES # COPYFILES2

.INCLUDE :  target.mk

#-------------------------------------------------------------------------------

TARGETDIR=$(MISC)$/sun-report-builder$/template$/en-US$/wizard$/report

COPYFILES: $(OTR_FILES_IN_REPORT_BUILDER_EXT) $(XCU_FILES)
    @@-$(MKDIRHIER) $(TARGETDIR)
    $(COPY) $(OTR_FILES_IN_REPORT_BUILDER_EXT) $(TARGETDIR)

#-------------------------------------------------------------------------------

# TARGETDIR2=$(MISC)$/templates$/template$/wizard$/report
# 
# COPYFILES2: $(OTR_FILES_IN_TEMPLATE_EXT) $(XCU_FILES)
# 	@@-$(MKDIRHIER) $(TARGETDIR2)
# 	$(COPY) $(OTR_FILES_IN_TEMPLATE_EXT) $(TARGETDIR2)

