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

COMPVERMK:=$(SOLARINCDIR)/comp_ver.mk

.INCLUDE .IGNORE : $(COMPVERMK)

.IF "$(COMNAME)"=="" || "$(COMPATH:s!\!/!)"!="$(COMPATH_STORED)"
.IF "$(L10N_framework)"==""

COMNAME:=

.IF "$(COM)"=="GCC"
CFLAGSVERSION=-dumpversion
CFLAGSVERSION_CMD=-dumpversion
CFLAGSNUMVERSION_CMD=-dumpversion $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)/bin/getcompver.awk
#CFLAGSNUMVERSION_CMD=-dumpversion | 2>&1  $(AWK) -v num=true -f $(SOLARENV)/bin/getcompver.awk
.ENDIF

.IF "$(COM)"=="MSC"
CFLAGSVERSION=
CFLAGSVERSION_CMD=  $(PIPEERROR) $(AWK) -f $(SOLARENV)/bin/getcompver.awk
CFLAGSNUMVERSION_CMD=  $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)/bin/getcompver.awk
.ENDIF

.IF "$(COM)"=="C55" || "$(COM)"=="C54" || "$(COM)"=="C52" || "$(COM)"=="C40" || "$(COM)"=="sunpro"
CFLAGSVERSION= -V
CFLAGSVERSION_CMD= -V  $(PIPEERROR) $(AWK) -f $(SOLARENV)/bin/getcompver.awk
CFLAGSNUMVERSION_CMD= -V  $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)/bin/getcompver.awk
.ENDIF

# that's the version known by the specific
# compiler
CCVER:=$(shell @-$(CXX) $(CFLAGSVERSION_CMD))

# and a computed integer for comparing
# each point separated token blown up to 4 digits
CCNUMVER:=$(shell @-$(CXX) $(CFLAGSNUMVERSION_CMD))

.IF "$(COM)"=="MSC"
COMID=MSC
.IF "$(CPU)"=="I"
COMNAME=msci
.ELSE
COMNAME=mscx
.ENDIF
.ENDIF

.IF "$(COM)"=="GCC"

SHORTSTDCPP3:=
SHORTSTDC3:="1"

.IF "$(CCNUMVER)">="000300000001"

COMID=gcc3
COMNAME=gcc3

.IF "$(CCNUMVER)">="000300000001"
SHORTSTDCPP3="3"
.ENDIF

.IF "$(CCNUMVER)">="000300010000"
SHORTSTDCPP3="4"
.ENDIF

.IF "$(CCNUMVER)">="000300020000"
SHORTSTDCPP3="5"
.ENDIF

.IF "$(CCNUMVER)">="000300040000"
.IF "$(OS)$(CPU)" == "LINUX6" || "$(OS)$(CPU)" == "LINUXH"
#for gcc >= 3.4.0 on m68k-linux this is libgcc_s.so.2.
#for gcc >= 3.4.0 < 4.2.0 on hppa-linux this is libgcc_s.so.2.
SHORTSTDC3:="2"
.ENDIF
SHORTSTDCPP3="6"
.ENDIF

.IF "$(CCNUMVER)">="000400020000"
.IF "$(OS)$(CPU)" == "LINUXH"
#for gcc >= 4.2.0 on hppa-linux this is libgcc_s.so.4.
SHORTSTDC3:="4"
.ENDIF
SHORTSTDCPP3="6"
.ENDIF

.ENDIF
.ENDIF

.IF "$(COM)"=="C55" || "$(COM)" == "C54" || "$(COM)"=="C52" || "$(COM)"=="C40" || "$(COM)"=="sunpro"
.IF "$(CCNUMVER)">="00050002"
COMID=C52
COMNAME=sunpro5
.ENDIF
.ENDIF

.IF "$(COMNAME)"==""

# "EXCEPTIONSFILES" get compiled before this, but shouldn't
# appear in the first n modules.

compiler_version_error:
    @echo ++++++++++++++++++++++++++++++++++++
    @echo  ERROR!
    @echo  Could not detect compiler version!
    @echo  Please extend tg_compv.mk in 
    @echo  "solenv/inc".
    @echo ++++++++++++++++++++++++++++++++++++
    @echo "$(CXX) $(CFLAGSVERSION)" returns
    @$(CXX) $(CFLAGSVERSION)
    @echo ++++++++++++++++++++++++++++++++++++
    force_dmake_to_error

.ENDIF          # "$(COMNAME)"==""

CDEFS+=-DCPPU_ENV=$(COMNAME)

.ENDIF			# "$(L10N_framework)"==""
.ENDIF			# "$(COMNAME)"==""
