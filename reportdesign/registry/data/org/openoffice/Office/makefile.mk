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
# $Revision: 1.8.6.1 $
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

PRJNAME=reportdesign
TARGET=data_ooOOffice
PACKAGE=org.openoffice.Office

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

LOCALIZEDFILES= \
    Embedding.xcu           \
    ExtendedColorScheme.xcu \
    DataAccess.xcu
    
XCUFILES= \
    $(LOCALIZEDFILES) \
    ReportDesign.xcu \
    Paths.xcu

.INCLUDE :  target.mk

ALLTAR: "$(PWD)$/$(MISC)$/$(EXTNAME)$/registry$/schema$/$(PACKAGEDIR)$/Embedding.xcs" "$(PWD)$/$(MISC)$/$(EXTNAME)$/registry$/schema$/$(PACKAGEDIR)$/ExtendedColorScheme.xcs" "$(PWD)$/$(MISC)$/$(EXTNAME)$/registry$/schema$/$(PACKAGEDIR)$/Paths.xcs"

"$(PWD)$/$(MISC)$/$(EXTNAME)$/registry$/schema$/$(PACKAGEDIR)$/Embedding.xcs" : $(SOLARXMLDIR)$/registry$/schema$/$(PACKAGEDIR)$/Embedding.xcs
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

"$(PWD)$/$(MISC)$/$(EXTNAME)$/registry$/schema$/$(PACKAGEDIR)$/ExtendedColorScheme.xcs" : $(SOLARXMLDIR)$/registry$/schema$/$(PACKAGEDIR)$/ExtendedColorScheme.xcs
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

"$(PWD)$/$(MISC)$/$(EXTNAME)$/registry$/schema$/$(PACKAGEDIR)$/Paths.xcs" : $(SOLARXMLDIR)$/registry$/schema$/$(PACKAGEDIR)$/Paths.xcs
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
