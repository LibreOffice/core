#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_javav.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:46:50 $
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

JAVAVERMK:=$(INCCOM)$/java_ver.mk

.INCLUDE .IGNORE : $(JAVAVERMK)

.IF "$(JAVAVER)"=="" || "$(JAVALOCATION)"!="$(JAVA_HOME)"
.IF "$(L10N_framework)"==""

.IF "$(SOLAR_JAVA)"!=""
JFLAGSVERSION=-version
JFLAGSVERSION_CMD=-version $(PIPEERROR) $(AWK) -f $(SOLARENV)$/bin$/getcompver.awk
JFLAGSNUMVERSION_CMD=-version $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)$/bin$/getcompver.awk

# that's the version known by the specific
# java version
JAVAVER:=$(shell @-$(JAVA_HOME)$/bin$/java $(JFLAGSVERSION_CMD))

# and a computed integer for comparing
# each point seperated token blown up to 4 digits
JAVANUMVER:=$(shell @-$(JAVA_HOME)$/bin$/java $(JFLAGSNUMVERSION_CMD))

.ELSE          # "$(SOLAR_JAVA)"!=""
JAVAVER=0.0.0
JAVANUMVER=000000000000
.ENDIF          # "$(SOLAR_JAVA)"!=""
.ENDIF			# "$(L10N_framework)"==""
.ENDIF			# "$(JAVAVER)"=="" || "$(JAVALOCATION)"!="$(JAVA_HOME)"
