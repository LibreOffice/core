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
# $Revision: 1.6 $
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


PRJ=..$/..

PRJNAME=extensions
TARGET=oooimprovement
ENABLE_EXCEPTIONS=TRUE

PACKAGE=org.openoffice.Office
ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES= \
    $(SLO)$/myconfigurationhelper.obj \
    $(SLO)$/config.obj \
    $(SLO)$/corecontroller.obj \
    $(SLO)$/errormail.obj \
    $(SLO)$/invite_job.obj \
    $(SLO)$/logpacker.obj \
    $(SLO)$/logstorage.obj \
    $(SLO)$/onlogrotate_job.obj \
    $(SLO)$/oooimprovement_exports.obj \
    $(SLO)$/soaprequest.obj \
    $(SLO)$/soapsender.obj \

SHL1STDLIBS= \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(UNOTOOLSLIB) \

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1LIBS= $(SLB)$/$(TARGET).lib
SHL1DEF= $(MISC)$/$(SHL1TARGET).def
DEF1NAME= $(SHL1TARGET)
SHL1VERSIONMAP= oooimprovement.map

PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

XCUFILES= \
    Jobs.xcu \
    Logging.xcu

MYXCUFILES= \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-oooimprovement.xcu \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Logging$/Logging-oooimprovement.xcu


# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MYXCUFILES)

XCU_SOURCEDIR:=.

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-oooimprovement.xcu : $(XCU_SOURCEDIR)$/Jobs.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(SPOOLDIR)$/$(PACKAGEDIR)$/Logging$/Logging-oooimprovement.xcu : $(XCU_SOURCEDIR)$/Logging.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
