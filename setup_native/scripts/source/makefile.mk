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



PRJ=..$/..

PRJNAME=setup_native
TARGET=getuid

NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

# no 'lib' prefix
DLLPRE=

# no _version.o
VERSIONOBJ=

.IF "$(OS)" == "LINUX" || "$(OS)" == "SOLARIS"

.IF "$(OS)" == "LINUX" 
CFLAGS+=-D_GNU_SOURCE
.ENDIF

.IF "$(OS)" == "SOLARIS"
.IF "$(CPU)" == "I"
.IF "$(COM)" != "GCC"
CFLAGS+=-D_KERNEL
.ENDIF
.ENDIF
.ENDIF

# this object must not be a Ultra SPARC binary, this does not
# work with /usr/bin/sort and such. Needs to be 32 bit even in
# 64 bit builds
.IF "$(OS)" == "SOLARIS"
.IF "$(COM)" != "GCC"
.IF "$(CPU)" == "S"
ENVCFLAGS=-xarch=v8
.ENDIF
.IF "$(CPU)"=="U"
CFLAGSAPPEND+=-m32
LINKFLAGSSHL+=-m32
.ENDIF
.ENDIF
.ENDIF

SLOFILES = $(SLO)$/getuid.obj

SHL1OBJS=$(SLOFILES)
SHL1TARGET=$(TARGET)
SHL1STDLIBS=-ldl
SHL1CODETYPE=C

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

