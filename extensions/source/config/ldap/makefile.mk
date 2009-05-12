#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.12 $
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

PRJ=..$/..$/..

PRJINC=$(PRJ)$/source
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
    $(SLO)$/ldapuserprof.obj	\
    $(SLO)$/ldapaccess.obj	\
    $(SLO)$/ldapuserprofilelayer.obj \
    $(SLO)$/propertysethelper.obj \
    $(SLO)$/componentdef.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib        
LIB1OBJFILES=$(SLOFILES)

SHL1TARGET=$(TARGET)$(LDAP_MAJOR).uno   
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(LDAPSDKLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALHELPERLIB) \
        $(SALLIB)
.IF "$(OS)"=="FREEBSD"
SHL1STDLIBS+=-lcompat
# workaround for FreeBSD, which needs -llber50, too
.IF "$(WITH_OPENLDAP)" != "YES"
SHL1STDLIBS+=-Wl,-Bstatic -llber50
.ENDIF
.ENDIF

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp    
DEF1DES=Configuration: LDAP User Profile Backend

# --- Targets ---

.INCLUDE : target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk

