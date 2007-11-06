#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.26 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:37:13 $
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
PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=mozab
TARGET2=$(TARGET)drv

.IF ( "$(SYSTEM_MOZILLA)" == "YES" && "$(WITH_MOZILLA)" == "YES") || "$(WITH_MOZILLA)" == "NO"
all: 
    @echo "		Not building the mozabsrc stuff in OpenOffice.org build"
    @echo "		dependency to Mozilla developer snapshots not feasable at the moment"
    @echo "         see http://bugzilla.mozilla.org/show_bug.cgi?id=135137"
.ENDIF

#mozilla specific stuff.
MOZ_LIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
MOZ_INC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla

.IF "$(OS)"=="WNT" 
.IF "$(USE_SHELL)"=="4nt"
MOZ_EMBED_LIB := $(shell @+-dir /ba:f $(MOZ_LIB)$/embed_base_s.lib 2>NUL )
MOZ_REG_LIB	  := $(shell @+-dir /ba:f $(MOZ_LIB)$/mozreg_s.lib 2>NUL )
.ELSE	#"$(USE_SHELL)"=="4nt"
MOZ_EMBED_LIB := $(shell @-test -f $(MOZ_LIB)$/embed_base_s.lib && echo $(MOZ_LIB)$/embed_base_s.lib )
MOZ_REG_LIB	  := $(shell @-test -f $(MOZ_LIB)$/mozreg_s.lib && echo $(MOZ_LIB)$/mozreg_s.lib )
.ENDIF

.IF X"$(MOZ_EMBED_LIB)"=="X"
MOZ_EMBED_LIB := $(MOZ_LIB)$/baseembed_s.lib
.ENDIF
.IF X"$(MOZ_REG_LIB)" == "X"
MOZ_REG_LIB := $(MOZ_LIB)$/mozreg.lib
.ENDIF
.ENDIF

.IF "$(OS)"=="WNT" 
.IF "$(COM)"=="GCC"
MOZ_LIB_XPCOM= -L$(MOZ_LIB) -lembed_base_s -lnspr4 -lmozreg_s -lxpcom
.ELSE
LIB += $(MOZ_LIB)
MOZ_LIB_XPCOM= $(MOZ_EMBED_LIB) $(MOZ_LIB)$/nspr4.lib $(MOZ_REG_LIB) $(MOZ_LIB)$/xpcom.lib
.ENDIF
.ELSE "$(OS)"=="WNT" 
MOZ_LIB_XPCOM= -L$(MOZ_LIB) -lembed_base_s -lnspr4 -lmozreg_s -lxpcom
.ENDIF
#End of mozilla specific stuff.

# Disable '-z defs' due to broken libxpcom.
LINKFLAGSDEFS=$(0)

USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk

.INCLUDE :  $(PRJ)$/version.mk

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/MDriver.obj						\
        $(SLO)$/MServices.obj

            
# --- MOZAB BASE Library -----------------------------------

SHL1VERSIONMAP= $(TARGET).map
SHL1TARGET=	$(TARGET)$(MOZAB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
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
    $(SLO)$/MNSProfileDirServiceProvider.obj


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
SHL2TARGET=	$(TARGET2)$(MOZAB_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
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

.INCLUDE : target.mk

# --- filter file ------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo _TI				>$@
    @echo _real				>>$@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo _TI				>$@
    @echo _real				>>$@

