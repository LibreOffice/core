#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2003-04-15 14:31:53 $
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

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=$(MOZAB_TARGET)
TARGET2=$(MOZAB_TARGET)drv

.IF "$(OS)"=="MACOSX"
all: 
    @echo "		Not building the mozabsrc stuff in OpenOffice.org build"
    @echo "		dependency to Mozilla developer snapshots not feasable at the moment"

.ENDIF

#mozilla specific stuff.
MOZ_LIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
MOZ_INC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
#.ENDIF

.IF "$(OS)"=="WNT" 
LIB += $(MOZ_LIB)
MOZ_LIB_XPCOM= $(MOZ_LIB)$/baseembed_s.lib $(MOZ_LIB)$/nspr4.lib $(MOZ_LIB)$/mozreg.lib $(MOZ_LIB)$/xpcom.lib
.ELSE
MOZ_LIB_XPCOM= -L$(MOZ_LIB) -lembed_base_s -lnspr4 -lmozreg_s -lxpcom
.ENDIF
#End of mozilla specific stuff.

# Disable '-z defs' due to broken libxpcom.
LINKFLAGSDEFS=$(0)

USE_LDUMP2=TRUE
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
LDUMP=ldump2.exe

# --- Settings ----------------------------------

.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk

.INCLUDE :  $(PRJ)$/version.mk

# --- Recursiveness  ---------------------------------------------------

RC_SUBDIRS = mozillasrc

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/MDriver.obj						\
        $(SLO)$/MServices.obj

# --- MOZAB BASE Library -----------------------------------

SHL1VERSIONMAP= $(TARGET).map
.IF "$(OS)"!="FREEBSD"
SHL1TARGET=	$(TARGET)$(MOZAB_MAJOR)
.ENDIF
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)


SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)$(MOZAB_MAJOR)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(SLB)$/$(TARGET).lib
DEFLIB1NAME=$(TARGET)
#DEF1EXPORTFILE=	exports.dxp


# --- Files -------------------------------------
MOZSLOFILES=\
    $(SLO)$/MNSInit.obj			            \
    $(SLO)$/MQueryHelper.obj			    \
    $(SLO)$/MDatabaseMetaDataHelper.obj		\
    $(SLO)$/MQuery.obj			            \
    $(SLO)$/MTypeConverter.obj              \
    $(SLO)$/MNameMapper.obj

SLO2FILES=\
        $(SLO)$/MConfigAccess.obj				\
        $(SLO)$/MCatalog.obj					\
        $(SLO)$/MColumns.obj					\
        $(SLO)$/MTable.obj						\
        $(SLO)$/MTables.obj						\
        $(SLO)$/MColumnAlias.obj				\
        $(SLO)$/MPreparedStatement.obj			\
        $(SLO)$/MStatement.obj					\
        $(SLO)$/MResultSetMetaData.obj			\
        $(SLO)$/MResultSet.obj					\
        $(SLO)$/MDatabaseMetaData.obj			\
        $(SLO)$/MConnection.obj					\
        $(MOZSLOFILES)

# --- MOZAB BASE Library -----------------------------------

SHL2VERSIONMAP= $(TARGET2).map
SHL2TARGET=	$(TARGET2)$(MOZAB_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)			\
    $(MOZ_LIB_XPCOM)


SHL2DEPN=
SHL2IMPLIB=	i$(TARGET2)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- filter file ------------------------------

.IF "$(depend)"==""

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo _TI				>>$@
    @echo _real				>>$@
.ENDIF

.IF "$(depend)"==""

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
    @echo _TI				>>$@
    @echo _real				>>$@
.ENDIF

killdpc: 
    -+$(RM) $(DPCTARGET)
    -+$(RM) $(DEPFILES)
    @+echo Dependency files removed

