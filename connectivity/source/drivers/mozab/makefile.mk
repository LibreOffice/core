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
PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=mozab
TARGET2=$(TARGET)drv
VISIBILITY_HIDDEN=TRUE

.IF ( "$(SYSTEM_MOZILLA)" == "YES" && "$(WITH_MOZILLA)" == "YES") || "$(WITH_MOZILLA)" == "NO" || ( "$(OS)" == "MACOSX" ) || ( "$(OS)" == "OS2" )
all: 
    @echo "    Not building the mozilla address book driver"
    @echo "    dependency to Mozilla developer snapshots not feasable at the moment"
    @echo "    see http://bugzilla.mozilla.org/show_bug.cgi?id=135137"
    @echo "    see http://www.mozilla.org/issues/show_bug.cgi?id=91209"
.ENDIF

# --- begin of mozilla specific stuff
MOZ_LIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
MOZ_INC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla

.IF "$(OS)"=="WNT" 
  MOZ_EMBED_LIB := $(shell @-test -f $(MOZ_LIB)$/embed_base_s.lib && echo $(MOZ_LIB)$/embed_base_s.lib )
  MOZ_REG_LIB	  := $(shell @-test -f $(MOZ_LIB)$/mozreg_s.lib && echo $(MOZ_LIB)$/mozreg_s.lib )

  MOZ_EMBED_LIB *:= $(MOZ_LIB)$/baseembed_s.lib
  MOZ_REG_LIB *:= $(MOZ_LIB)$/mozreg.lib

  .IF "$(COM)"=="GCC"
    MOZ_LIB_XPCOM= -L$(MOZ_LIB) -lembed_base_s -lnspr4 -lmozreg_s -lxpcom -lxpcom_core
  .ELSE
    LIB += $(MOZ_LIB)
    MOZ_LIB_XPCOM= $(MOZ_EMBED_LIB) $(MOZ_LIB)$/nspr4.lib $(MOZ_REG_LIB) $(MOZ_LIB)$/xpcom.lib $(MOZ_LIB)$/xpcom_core.lib
  .ENDIF

.ELSE "$(OS)"=="WNT" 
  MOZ_LIB_XPCOM = -L$(MOZ_LIB) -lnspr4 -lxpcom_core -lmozreg_s -lembed_base_s
.ENDIF
# --- end of mozilla specific stuff

USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

.IF "$(OS)"!="WNT" 
COMPONENT_CONFIG_DATA=$(TARGET)2.xcu
COMPONENT_CONFIG_SCHEMA=$(TARGET)2.xcs
.ENDIF

# --- Settings ----------------------------------

.INCLUDE : $(PRJ)$/makefile.pmk

.INCLUDE :  $(PRJ)$/version.mk

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/MDriver.obj						\
        $(SLO)$/MServices.obj
            
# --- MOZAB BASE Library -----------------------------------

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1TARGET=	$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SALLIB)					\
    $(DBTOOLSLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)$(DLLPOSTFIX)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
            $(SLB)$/$(TARGET).lib
DEFLIB1NAME=$(TARGET)


# --- Files -------------------------------------
MOZSLOFILES=\
    $(SLO)$/MNSInit.obj			            \
    $(SLO)$/MQueryHelper.obj			    \
    $(SLO)$/MDatabaseMetaDataHelper.obj		\
    $(SLO)$/MQuery.obj			            \
    $(SLO)$/MTypeConverter.obj              \
    $(SLO)$/MNameMapper.obj					\
    $(SLO)$/MNSMozabProxy.obj	\
    $(SLO)$/MNSTerminateListener.obj	\
    $(SLO)$/MMozillaBootstrap.obj	\
    $(SLO)$/MNSFolders.obj	\
    $(SLO)$/MNSProfileDiscover.obj	\
    $(SLO)$/MNSProfileManager.obj	\
    $(SLO)$/MNSINIParser.obj	\
    $(SLO)$/MNSRunnable.obj	\
    $(SLO)$/MNSProfile.obj					\
    $(SLO)$/MNSProfileDirServiceProvider.obj    \
    $(SLO)$/MLdapAttributeMap.obj


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

DEPOBJFILES=$(SLO2FILES)

# --- MOZAB BASE Library -----------------------------------

SHL2VERSIONMAP= $(TARGET2).map
SHL2NOCHECK=TRUE
SHL2TARGET=	$(TARGET2)$(DLLPOSTFIX)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SALLIB)					\
    $(SALHELPERLIB) \
    $(DBTOOLSLIB)				\
    $(COMPHELPERLIB)			\
    $(MOZ_LIB_XPCOM)

.IF "$(GUI)"=="WNT"
    SHL2STDLIBS += \
                 $(SHELL32LIB)
.ENDIF # "$(GUI)"=="WNT"

SHL2DEPN=
SHL2IMPLIB=	i$(TARGET2)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
DEF2NAME=	$(SHL2TARGET)

# --- Targets ----------------------------------

.INCLUDE : $(PRJ)$/target.pmk

# --- filter file ------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo _TI				>$@
    @echo _real				>>$@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo _TI				>$@
    @echo _real				>>$@

