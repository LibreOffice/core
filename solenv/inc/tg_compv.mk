#*************************************************************************
#
#   $RCSfile: tg_compv.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 16:43:53 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

COMPVERMK:=$(SOLARINCDIR)$/comp_ver.mk

.INCLUDE .IGNORE : $(COMPVERMK)

.IF "$(COMNAME)"==""
.IF "$(L10N_framework)"==""

COMNAME:=

.IF "$(COM)"=="GCC"
CFLAGSVERSION=-dumpversion
CFLAGSVERSION_CMD=-dumpversion
CFLAGSNUMVERSION_CMD=-dumpversion |& awk -v num=true -f $(SOLARENV)$/bin$/getcompver.awk
.ENDIF

.IF "$(COM)"=="MSC"
CFLAGSVERSION=
CFLAGSVERSION_CMD= |& $(AWK) -f $(SOLARENV)$/bin$/getcompver.awk
CFLAGSNUMVERSION_CMD= |& $(AWK) -v num=true -f $(SOLARENV)$/bin$/getcompver.awk
.ENDIF

.IF "$(COM)"=="C54" || "$(COM)"=="C52" || "$(COM)"=="C40" || "$(COM)"=="sunpro"
CFLAGSVERSION= -V
CFLAGSVERSION_CMD= -V |& nawk -f $(SOLARENV)$/bin$/getcompver.awk
CFLAGSNUMVERSION_CMD= -V |& nawk -v num=true -f $(SOLARENV)$/bin$/getcompver.awk
.ENDIF

# that's the version known by the specific
# compiler
CCVER:=$(shell -$(CXX) $(CFLAGSVERSION_CMD))

# and a computed integer for comparing
# each point seperated token blown up to 4 digits
CCNUMVER:=$(shell -$(CXX) $(CFLAGSNUMVERSION_CMD))

.IF "$(COM)"=="MSC"
.IF "$(CCNUMVER)">="001200000000"
COMID=MSC
COMNAME=msci
.ENDIF
.ENDIF

.IF "$(COM)"=="GCC"

LIBSTDCPP3:=
SHORTSTDCPP3:=

.IF "$(CCNUMVER)">="000200910000"
COMID=GCC
COMNAME=gcc2
.ENDIF

.IF "$(CCNUMVER)">="000300000001"

COMID=gcc3
COMNAME=gcc3

.IF "$(CCNUMVER)"=="000300000001"
LIBSTDCPP3="3.0.1"
SHORTSTDCPP3="3"
.ENDIF

.IF "$(CCNUMVER)"=="000300000002"
LIBSTDCPP3="3.0.2"
SHORTSTDCPP3="3"
.ENDIF

.IF "$(CCNUMVER)"=="000300000003"
LIBSTDCPP3="3.0.3"
SHORTSTDCPP3="3"
.ENDIF

.IF "$(CCNUMVER)"=="000300000004"
LIBSTDCPP3="3.0.4"
SHORTSTDCPP3="3"
.ENDIF

.IF "$(CCNUMVER)"=="000300010000"
LIBSTDCPP3="4.0.0"
SHORTSTDCPP3="4"
.ENDIF

.IF "$(CCNUMVER)"=="000300010001"
LIBSTDCPP3="4.0.1"
SHORTSTDCPP3="4"
.ENDIF

.IF "$(CCNUMVER)"=="000300020000"
LIBSTDCPP3="5.0.0"
SHORTSTDCPP3="5"
.ENDIF

.IF "$(CCNUMVER)"=="000300020001"
LIBSTDCPP3="5.0.1"
SHORTSTDCPP3="5"
.ENDIF

.IF "$(CCNUMVER)"=="000300020002"
LIBSTDCPP3="5.0.2"
SHORTSTDCPP3="5"
.ENDIF

.IF "$(CCNUMVER)"=="000300020003"
LIBSTDCPP3="5.0.3"
SHORTSTDCPP3="5"
.ENDIF

.IF "$(CCNUMVER)"=="000300030000"
LIBSTDCPP3="5.0.3"
SHORTSTDCPP3="5"
.ENDIF

.ENDIF
.ENDIF

.IF "$(COM)" == "C54" || "$(COM)"=="C52" || "$(COM)"=="C40" || "$(COM)"=="sunpro"
.IF "$(CCNUMVER)">="00050002"
COMID=C52
COMNAME=sunpro5
.ENDIF
.ENDIF

.IF "$(COMNAME)"==""

# "EXCEPTIONSFILES" get compiled before this, but shouldn't
# appear in the first n modules.

compiler_version_error:
    @+echo ++++++++++++++++++++++++++++++++++++
    @+echo  ERROR!
    @+echo  Could not detect compiler version!
    @+echo  Please extend tg_compv.mk in 
    @+echo  "solenv/inc".
    @+echo ++++++++++++++++++++++++++++++++++++
    @+echo "$(CXX) $(CFLAGSVERSION)" returns
    @+$(CXX) $(CFLAGSVERSION)
    @+echo ++++++++++++++++++++++++++++++++++++
    @+exit 255

.ENDIF          # "$(COMNAME)"==""

CDEFS+=-DCPPU_ENV=$(COMNAME)

.ENDIF			# "$(L10N_framework)"==""
.ENDIF			# "$(COMNAME)"==""
