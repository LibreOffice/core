#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: tg_javav.mk,v $
#
# $Revision: 1.10 $
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

JAVAVERMK:=$(INCCOM)/java_ver.mk

.INCLUDE .IGNORE : $(JAVAVERMK)

.IF "$(JAVAVER)"=="" || "$(JAVALOCATION)"!="$(JAVA_HOME)"
.IF "$(L10N_framework)"==""

.IF "$(SOLAR_JAVA)"!=""
JFLAGSVERSION=-version
JFLAGSVERSION_CMD=-version $(PIPEERROR) $(AWK) -f $(SOLARENV)/bin/getcompver.awk
JFLAGSNUMVERSION_CMD=-version $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)/bin/getcompver.awk

# that's the version known by the specific
# java version
JAVAVER:=$(shell @-$(JAVA_HOME)/bin/java $(JFLAGSVERSION_CMD))

# and a computed integer for comparing
# each point seperated token blown up to 4 digits
JAVANUMVER:=$(shell @-$(JAVA_HOME)/bin/java $(JFLAGSNUMVERSION_CMD))

.ELSE          # "$(SOLAR_JAVA)"!=""
JAVAVER=0.0.0
JAVANUMVER=000000000000
.ENDIF          # "$(SOLAR_JAVA)"!=""
.ENDIF			# "$(L10N_framework)"==""
.ENDIF			# "$(JAVAVER)"=="" || "$(JAVALOCATION)"!="$(JAVA_HOME)"
