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
# $Revision: 1.17 $
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
PRJ=..$/..

PRJNAME=pyuno
TARGET=pythonloader.uno
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

DLLPRE = 

#-------------------------------------------------------------------

.IF "$(OS)$(CPU)$(COMEX)" == "SOLARISS4"
# no -Bdirect for SunWS CC
DIRECT = $(LINKFLAGSDEFS)
.ENDIF

.IF "$(SYSTEM_PYTHON)" == "YES"
PYTHONLIB=$(PYTHON_LIBS)
CFLAGS+=$(PYTHON_CFLAGS)
.IF "$(EXTRA_CFLAGS)"!=""
PYTHONLIB+=-framework Python
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ELSE
.INCLUDE :  pyversion.mk

CFLAGS+=-I$(SOLARINCDIR)$/python
.ENDIF

SHL1TARGET=	$(TARGET)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)		\
        $(PYUNOLIB)		\
        $(PYTHONLIB)

SHL1VERSIONMAP=$(SOLARENV)$/src$/component.map
SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
SLOFILES=       $(SLO)$/pyuno_loader.obj

#COMPONENTS= \
#	tcv		\
#	inv		\
#	corefl		\
#	insp		\
#	invadp		\
#	proxyfac 	\
#	pythonloader.uno \


COMPONENTS= \
    stocservices.uno	\
    invocation.uno		\
    introspection.uno	\
    invocadapt.uno		\
    proxyfac.uno 		\
    reflection.uno	\
    pythonloader.uno

# --- Targets ------------------------------------------------------

ALL : ALLTAR \
    $(DLLDEST)$/pythonloader.py	\
    $(DLLDEST)$/pyuno_services.rdb

.INCLUDE :  target.mk

$(DLLDEST)$/%.py: %.py
    cp $? $@

# For Mac OS X,
# The python loader component is linked against libpyuno.dylib,
# which hasn't been delivered yet but dyld needs to know where it is
# so regcomp can load the component.
$(DLLDEST)$/pyuno_services.rdb : makefile.mk $(DLLDEST)$/$(DLLPRE)$(TARGET)$(DLLPOST)
    -rm -f $@ $(DLLDEST)$/pyuno_services.tmp $(DLLDEST)$/pyuno_services.rdb
.IF "$(OS)"=="MACOSX"
    cd $(DLLDEST) && sh -c "DYLD_LIBRARY_PATH=$(DYLD_LIBRARY_PATH):$(OUT)$/lib;export DYLD_LIBRARY_PATH;regcomp -register -r pyuno_services.tmp $(foreach,i,$(COMPONENTS) -c $(i))"
.ELSE
.IF "$(GUI)$(COM)"=="WNTGCC"
    cd $(DLLDEST) && sh -c "export PATH='$(PATH):$(OUT)$/bin'; regcomp -register -r pyuno_services.tmp $(foreach,i,$(COMPONENTS) -c $(i))"
.ELSE
    cd $(DLLDEST) && regcomp -register -r pyuno_services.tmp $(foreach,i,$(COMPONENTS) -c $(i))
.ENDIF    # "$(GUI)$(COM)"=="WNTGCC" 
.ENDIF    # $(OS)=="MACOSX"
    cd $(DLLDEST) && mv pyuno_services.tmp pyuno_services.rdb

