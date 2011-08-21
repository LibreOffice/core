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

# --- Force JDK14 switch  ------------------------------------------

.IF "$(USE_JDK_VERSION)" == "140"
JDK_VERSION=140
JAVA_HOME=$(JDK14PATH)

PATH!:=$(JDK14PATH)/bin$(PATH_SEPERATOR)$(PATH)
XCLASSPATH:=$(JDK14PATH)/jre/lib/rt.jar
CLASSPATH:=$(XCLASSPATH)
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk


# --- ANT build environment  ---------------------------------------

.INCLUDE : antsettings.mk

.INCLUDE : target.mk

CLASSPATH!:=$(CLASSPATH)$(PATH_SEPERATOR)$(ANT_CLASSPATH)$(PATH_SEPERATOR)$(JAVA_HOME)/lib/tools.jar
.EXPORT : CLASSPATH
.EXPORT : PATH

# --- TARGETS -----------------------------------------------------

$(CLASSDIR)/solar.properties : $(SOLARVERSION)/$(INPATH)/inc/minormkchanged.flg $(SOLARENV)/inc/ant.properties
    @echo "Making:   " $@
    @echo solar.build=$(BUILD) > $@
    @echo solar.rscversion=$(USQ)$(RSCVERSION)$(USQ) >> $@
    @echo solar.rscrevision=$(USQ)$(RSCREVISION)$(USQ) >> $@
    @echo solar.minor=$(LAST_MINOR) >> $@
    @echo solar.sourceversion=$(SOURCEVERSION) >> $@
    @echo solar.udkstamp=$(UDKSTAMP) >> $@
    @echo solar.extstamp=$(EXTSTAMP) >> $@
    @cat $(DMAKEROOT)/../ant.properties >> $@

ANTBUILD .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS)

clean  .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS) $@

prepare .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS) $@

main: .PHONY:
    $(ANT) $(ANT_FLAGS) $@

info: .PHONY
    $(ANT) $(ANT_FLAGS) $@

jar .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS) $@

compile .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS) $@

depend .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS) $@

javadoc .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS) $@

config .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS) $@

test .PHONY:
    $(COMMAND_ECHO)$(ANT) $(ANT_FLAGS) $@
