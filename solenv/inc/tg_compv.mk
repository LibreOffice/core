#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
SHORTSTDC3:="1"

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

.IF "$(COM)"=="C730"
COMID=C730
COMNAME=MipsPro
.ENDIF

.IF "$(COM)"=="CLANG"
COMID=s5abi
COMNAME=s5abi
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
