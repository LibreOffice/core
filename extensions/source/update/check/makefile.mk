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
PRJ=..$/..$/..

PRJNAME=extensions
TARGET=updchk
PACKAGE=org.openoffice.Office

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

# no validation by inspector class
NO_INSPECTION=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# no "lib" prefix
DLLPRE =

# --- Files ---

SRS2NAME=$(TARGET)
SRC2FILES=\
    updatehdl.src

RESLIB2NAME=upd
RESLIB2SRSFILES= $(SRS)$/updchk.srs
RESLIB2DEPN= updatehdl.src updatehdl.hrc

SLOFILES=\
    $(SLO)$/download.obj \
    $(SLO)$/updatecheck.obj \
    $(SLO)$/updatecheckjob.obj \
    $(SLO)$/updatecheckconfig.obj \
    $(SLO)$/updateprotocol.obj \
    $(SLO)$/updatehdl.obj
        
SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET).uno   
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(CURLLIB) \
        $(SALLIB) \
        $(SHELL32LIB) \
        $(OLE32LIB)
        
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

XCUFILES= \
    Addons.xcu \
    Jobs.xcu

MYXCUFILES= \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Addons$/Addons-onlineupdate.xcu \
    $(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-onlineupdate.xcu

LOCALIZEDFILES= \
    Addons.xcu \
    Jobs.xcu

.IF "$(test)" != ""
APP1TARGET=updateprotocoltest
APP1STDLIBS= $(SHL1STDLIBS)
APP1OBJS= \
    $(SLO)$/updateprotocol.obj \
    $(SLO)$/updateprotocoltest.obj


.ENDIF # "$(test)" != ""

# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MYXCUFILES)

.IF "$(WITH_LANG)"!=""
XCU_SOURCEDIR:=$(PROCESSOUT)$/merge$/$(PACKAGEDIR)
.ELSE			# "$(WITH_LANG)"!=""
XCU_SOURCEDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

$(SPOOLDIR)$/$(PACKAGEDIR)$/Addons$/Addons-onlineupdate.xcu : $(XCU_SOURCEDIR)$/Addons.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-onlineupdate.xcu : $(XCU_SOURCEDIR)$/Jobs.xcu
    @-$(MKDIRHIER) $(@:d)
    @$(COPY) $< $@
#	@$(PERL) transform.pl < $< > $@


ALLTAR : $(MISC)/updchk.uno.component

$(MISC)/updchk.uno.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        updchk.uno.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt updchk.uno.component
