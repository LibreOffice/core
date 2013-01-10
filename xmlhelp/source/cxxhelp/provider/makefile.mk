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
PRJNAME=xmlhelp
TARGET=chelp

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE


# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# GCC versions 4.2.x introduced a warning "allocating zero-element array"
# Allocating zero-element arrays is an allowed if not somewhat dubious 
# technique though, so this warning is plain wrong and has been fixed 
# in gcc 4.3. Unfortunately there is no way at all to suppress this warning.
# Some files in this directory use zero allocated arrays, we need to
# disable the WaE mechanism for the GCC 4.2.x series.
.IF "$(COM)"=="GCC"
.IF "$(CCNUMVER)">="000400020000" && "$(CCNUMVER)"<="000400020003"
CFLAGSWERRCXX:=
.ENDIF # "$(CCNUMVER)">="000400020000" && "$(CCNUMVER)"<="000400020003"
.ENDIF # "$(COM)"=="GCC"

CFLAGS +=  -DHAVE_EXPAT_H

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+= $(LIBXML_CFLAGS)
.ELSE
LIBXMLINCDIR=external$/libxml
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXMLINCDIR)
.ENDIF

.IF "$(SYSTEM_LIBXSLT)" == "YES"
CFLAGS+= $(LIBXSLT_CFLAGS)
.ELSE
LIBXSLTINCDIR=external$/libxslt
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXSLTINCDIR)
.ENDIF

.IF "$(GUI)"=="WNT"
CFLAGS+=-GR
.ENDIF

# --- General -----------------------------------------------------

SLOFILES=\
    $(SLO)$/db.obj                     \
    $(SLO)$/databases.obj          \
    $(SLO)$/services.obj    	   \
    $(SLO)$/resultset.obj     	   \
    $(SLO)$/resultsetbase.obj      \
    $(SLO)$/resultsetforroot.obj   \
    $(SLO)$/resultsetforquery.obj  \
    $(SLO)$/contentcaps.obj        \
    $(SLO)$/provider.obj    	   \
    $(SLO)$/content.obj     	   \
    $(SLO)$/urlparameter.obj       \
    $(SLO)$/inputstream.obj        \
    $(SLO)$/bufferedinputstream.obj

# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

