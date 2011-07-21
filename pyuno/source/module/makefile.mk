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
PRJ=..$/..

PRJNAME=pyuno
TARGET=pyuno
ENABLE_EXCEPTIONS=TRUE

LINKFLAGSDEFS = # do not fail with missing symbols

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""

#-------------------------------------------------------------------

.IF "$(OS)$(COMEX)" == "SOLARIS4"
# no -Bdirect for SunWS CC
DIRECT = $(LINKFLAGSDEFS)
.ENDIF

# special setting from environment
.IF "$(EXTRA_CFLAGS)"!=""
EXTRA_FRAMEWORK_FLAG=-framework Python
.ENDIF # .IF "$(EXTRA_CFLAGS)"!=""

.IF "$(GUI)" == "UNX"
PYUNORC=pyunorc
.ELSE
.IF "$(CROSS_COMPILING)" != "YES"
.INCLUDE :  pyversion.mk
.ENDIF
PYUNORC=pyuno.ini
DLLPOST=.pyd
.ENDIF

.IF "$(SYSTEM_PYTHON)" == "YES"
PYTHONLIB=$(PYTHON_LIBS)
CFLAGS+=$(PYTHON_CFLAGS)
.ELSE # "$(SYSTEM_PYTHON)" == "YES"
.INCLUDE :  pyversion.mk
CFLAGS+=-I$(SOLARINCDIR)$/python
.ENDIF # "$(SYSTEM_PYTHON)" == "YES"

SHL1TARGET=$(TARGET)
SLOFILES= \
        $(SLO)$/pyuno_runtime.obj \
        $(SLO)$/pyuno.obj \
        $(SLO)$/pyuno_callable.obj \
        $(SLO)$/pyuno_module.obj \
        $(SLO)$/pyuno_type.obj \
        $(SLO)$/pyuno_util.obj \
        $(SLO)$/pyuno_except.obj \
        $(SLO)$/pyuno_adapter.obj \
        $(SLO)$/pyuno_gc.obj

# remove this, when issue i35064 is integrated
.IF "$(COM)"=="GCC"
NOOPTFILES= \
    $(SLO)$/pyuno_module.obj
.ENDIF # "$(COM)"=="GCC"

SHL1STDLIBS= \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(SALLIB) \
        $(PYTHONLIB) \
        $(EXTRA_FRAMEWORK_FLAG)

SHL1DEPN=$(eq,$(OS),MACOSX $(MISC)/framework_link $(NULL))
SHL1LIBS= $(SLB)$/$(TARGET).lib
SHL1IMPLIB= i$(TARGET)

SHL1DEF= $(MISC)$/$(SHL1TARGET).def

DEF1NAME= $(SHL1TARGET)
DEF1DEPN= $(MISC)$/pyuno.flt

DEFLIB1NAME=$(TARGET)

# --- Targets ------------------------------------------------------

.IF "$(GUI)$(COM)"=="WNTGCC"
ALLTAR : \
    $(DLLDEST)$/uno.py \
    $(DLLDEST)$/unohelper.py \
    $(MISC)$/$(PYUNORC) \
    $(LB)$/lib$(TARGET).a

$(LB)$/lib$(TARGET).a: $(MISC)$/$(TARGET).def
    $(DLLTOOL) --dllname $(TARGET)$(DLLPOST) --input-def=$(MISC)$/$(TARGET).def --kill-at --output-lib=$(LB)$/lib$(TARGET).a
.ELSE

.IF "$(GUI)"!="WNT"
# For some reason the build breaks on Windows if this is listed in the
# prerequisite list of ALLTAR, but pyuno.pyd still gets produced. Go
# figure. But we need it on non-Windows.
targetdll=$(LB)$/$(TARGET)$(DLLPOST)
.ENDIF

ALLTAR : \
    $(DLLDEST)$/uno.py \
    $(DLLDEST)$/unohelper.py \
    $(targetdll) \
    $(MISC)$/$(PYUNORC)
.ENDIF
.ENDIF

.INCLUDE :  target.mk
.IF "$(L10N_framework)"==""
$(DLLDEST)$/%.py: %.py
    cp $? $@

# make checkdll happy
$(MISC)/framework_link :
	$(COMMAND_ECHO)ln -sf $(SOLARLIBDIR)/OOoPython.framework $(LB)/OOoPython.framework
	@touch $@

$(MISC)$/$(PYUNORC) : pyuno
    -rm -f $@
    cat pyuno > $@

$(MISC)$/pyuno.flt : pyuno.flt
    -rm -f $@
    cat $? > $@

.IF "$(DLLPRE)"!=""
# python does not accept the "lib" prefix in the module library
$(LB)$/$(TARGET)$(DLLPOST) : $(LB)$/$(DLLPRE)$(TARGET)$(DLLPOST)
    -rm -f $@
    ln -s $? $@
.ENDIF

.ENDIF # L10N_framework

# vim:set shiftwidth=4 softtabstop=4 expandtab:
