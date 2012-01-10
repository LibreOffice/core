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



PRJ=..$/..$/..

PRJNAME=filter
TARGET=eppm
TARGET2=epp
DEPTARGET=veppm

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Allgemein ----------------------------------------------------------

.IF "$(editdebug)"!="" || "$(EDITDEBUG)"!=""
CDEFS+= -DEDITDEBUG
.ENDIF

.IF "$(L10N_framework)"==""
SLOFILES =	$(SLO)$/eppm.obj
.ENDIF
# ==========================================================================

.IF "$(L10N_framework)"==""
SHL1TARGET= 	epp$(DLLPOSTFIX)
SHL1IMPLIB= 	eppm
SHL1STDLIBS=	$(TOOLSLIB) $(VCLLIB) $(CPPULIB) $(SVTOOLLIB) $(SALLIB)

SHL1LIBS=		$(SLB)$/eppm.lib

.IF "$(GUI)" != "UNX"
.IF "$(COM)" != "GCC"
SHL1OBJS=		$(SLO)$/eppm.obj
.ENDIF
.ENDIF

SHL1VERSIONMAP=exports.map
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
.ENDIF

# ==========================================================================

.INCLUDE :	target.mk
