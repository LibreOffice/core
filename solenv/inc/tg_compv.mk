#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_compv.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:45:32 $
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

COMPVERMK:=$(SOLARINCDIR)$/comp_ver.mk

.INCLUDE .IGNORE : $(COMPVERMK)

.IF "$(COMNAME)"=="" || "$(COMPATH)"!="$(COMPATH_STORED)"
.IF "$(L10N_framework)"==""

COMNAME:=

.IF "$(COM)"=="GCC"
CFLAGSVERSION=-dumpversion
CFLAGSVERSION_CMD=-dumpversion
CFLAGSNUMVERSION_CMD=-dumpversion $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)$/bin$/getcompver.awk
#CFLAGSNUMVERSION_CMD=-dumpversion | 2>&1  $(AWK) -v num=true -f $(SOLARENV)$/bin$/getcompver.awk
.ENDIF

.IF "$(COM)"=="MSC"
CFLAGSVERSION=
CFLAGSVERSION_CMD=  $(PIPEERROR) $(AWK) -f $(SOLARENV)$/bin$/getcompver.awk
CFLAGSNUMVERSION_CMD=  $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)$/bin$/getcompver.awk
.ENDIF

.IF "$(COM)"=="C55" || "$(COM)"=="C54" || "$(COM)"=="C52" || "$(COM)"=="C40" || "$(COM)"=="sunpro"
CFLAGSVERSION= -V
CFLAGSVERSION_CMD= -V  $(PIPEERROR) $(AWK) -f $(SOLARENV)$/bin$/getcompver.awk
CFLAGSNUMVERSION_CMD= -V  $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)$/bin$/getcompver.awk
.ENDIF

.IF "$(COM)"=="C730"
CFLAGSVERSION= -version
CFLAGSVERSION_CMD= -version |& cut -d" " -f4-
CFLAGSNUMVERSION_CMD= -version |& cut -d" " -f4-
.ENDIF

# that's the version known by the specific
# compiler
CCVER:=$(shell @-$(CXX) $(CFLAGSVERSION_CMD))

# and a computed integer for comparing
# each point seperated token blown up to 4 digits
CCNUMVER:=$(shell @-$(CXX) $(CFLAGSNUMVERSION_CMD))

.IF "$(COM)"=="MSC"
.IF "$(CCNUMVER)">="001200000000"
COMID=MSC
COMNAME=msci
.ENDIF
.ENDIF

.IF "$(COM)"=="GCC"

SHORTSTDCPP3:=

.IF "$(CCNUMVER)">="000200910000"
COMID=GCC
COMNAME=gcc2
.ENDIF

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

.IF "$(COM)"=="C730"
COMID=C730
COMNAME=MipsPro
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
