#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
PRJ=..$/..

PRJNAME=	stoc
TARGET=		excomp
TARGET1=	excomp1
TARGET2=	excomp2
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=	TRUE
COMP1TYPELIST=$(TARGET1)
COMP2TYPELIST=$(TARGET2)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

UNOUCRDEP=	$(BIN)$/excomp.rdb
UNOUCRRDB=	$(BIN)$/excomp.rdb
UNOUCROUT=	$(OUT)$/inc
INCPRE+=	$(OUT)$/inc

UNOTYPES=$($(TARGET1)_XML2CMPTYPES)
UNOTYPES+=$($(TARGET2)_XML2CMPTYPES)
UNOTYPES+=  com.sun.star.registry.XImplementationRegistration \
            com.sun.star.lang.XComponent

# --- Application excomp ------------------------------------------------
APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/excomp.obj 

APP1STDLIBS= \
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 	\
        $(SALHELPERLIB) 	\
        $(SALLIB)

# ---- objects ----
SLOFILES= \
        $(SLO)$/excomp1.obj	\
          $(SLO)$/$(COMP1TYPELIST)_description.obj	\
        $(SLO)$/excomp2.obj	\
          $(SLO)$/$(COMP2TYPELIST)_description.obj

# ---- excomp1 ------
SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALHELPERLIB)		\
        $(SALLIB)	

SHL1DEPN=
SHL1LIBS=
SHL1OBJS=  	$(SLO)$/excomp1.obj	\
              $(SLO)$/$(COMP1TYPELIST)_description.obj
SHL1IMPLIB=	i$(TARGET1)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# ---- excomp2 ------
SHL2TARGET=	$(TARGET2)

SHL2STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALHELPERLIB)		\
        $(SALLIB)

SHL2DEPN=
SHL2LIBS=
SHL2OBJS=  	$(SLO)$/excomp2.obj	\
              $(SLO)$/$(COMP2TYPELIST)_description.obj	
SHL2IMPLIB=	i$(TARGET2)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp

ALLIDLFILES:= example$/XTest.idl example$/ExampleComponent1.idl example$/ExampleComponent2.idl

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL : 		$(BIN)$/excomp.rdb	\
        ALLTAR 
.ELSE
ALL: 		ALLDEP
.ENDIF

.INCLUDE :	target.mk

$(BIN)$/excomp.rdb: $(ALLIDLFILES)
    idlc -I$(PRJ) -I$(SOLARIDLDIR) -O$(MISC)$/excomp $?
    regmerge $@ /UCR $(MISC)$/excomp$/{$(?:f:s/.idl/.urd/)}
    regmerge $@ / $(SOLARBINDIR)$/udkapi.rdb
    touch $@

