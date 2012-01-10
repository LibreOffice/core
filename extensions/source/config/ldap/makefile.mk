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

PRJNAME=extensions
TARGET=ldapbe
ENABLE_EXCEPTIONS=TRUE

# Version
LDAP_MAJOR=2

# --- Settings ---

.INCLUDE : settings.mk
DLLPRE =

.IF "$(WITH_LDAP)" != "YES"
@all:
    @echo "LDAP disabled."
.ENDIF

.IF "$(LDAPSDKINCLUDES)" == ""

.IF "$(SYSTEM_MOZILLA)" == "YES" && "$(WITH_OPENLDAP)" != "YES"
LDAPSDKINCLUDES = $(MOZ_LDAP_CFLAGS)
.ELSE
LDAPSDKINCLUDES = -I$(SOLARINCDIR)$/mozilla
.ENDIF
.ENDIF

.IF "$(WITH_OPENLDAP)" == "YES"
CFLAGS += -DWITH_OPENLDAP -DLDAP_DEPRECATED
.ENDIF

SOLARINC+= $(LDAPSDKINCLUDES)
# --- Files ---


SLOFILES=\
    $(SLO)$/ldapuserprofilebe.obj \
    $(SLO)$/ldapaccess.obj	\
    $(SLO)$/componentdef.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib        
LIB1OBJFILES=$(SLOFILES)

SHL1TARGET=$(TARGET)$(LDAP_MAJOR).uno   
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALHELPERLIB) \
        $(SALLIB)
.IF "$(OS)"=="FREEBSD"
SHL1STDLIBS+=-lcompat
# workaround for FreeBSD, which needs -llber50, too
.IF "$(WITH_OPENLDAP)" != "YES"
SHL1STDLIBS+=-Wl,-Bstatic -llber50 -Wl,-Bdynamic
.ENDIF
.ENDIF

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp    
DEF1DES=Configuration: LDAP User Profile Backend

# --- Targets ---

.INCLUDE : target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk


ALLTAR : $(MISC)/ldapbe2.component

$(MISC)/ldapbe2.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ldapbe2.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ldapbe2.component
