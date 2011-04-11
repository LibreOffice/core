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
# python expects modules without the lib prefix 
# pyuno.so even on Mac OS X, because it is a python module
PYUNO_MODULE=$(DLLDEST)$/pyuno.so
PYUNORC=pyunorc
.ELIF "$(GUI)" == "OS2"
.INCLUDE :  pyversion.mk
PYUNORC=pyuno.ini
.ELSE
.INCLUDE :  pyversion.mk
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
        $(SLO)$/pyuno_runtime.obj 	\
        $(SLO)$/pyuno.obj 		\
        $(SLO)$/pyuno_callable.obj 	\
        $(SLO)$/pyuno_module.obj 	\
        $(SLO)$/pyuno_type.obj 		\
        $(SLO)$/pyuno_util.obj		\
        $(SLO)$/pyuno_except.obj	\
        $(SLO)$/pyuno_adapter.obj	\
        $(SLO)$/pyuno_gc.obj

# remove this, when issue i35064 is integrated
.IF "$(COM)"=="GCC"
NOOPTFILES= \
    $(SLO)$/pyuno_module.obj
.ENDIF			# "$(COM)"=="GCC"


SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)		\
        $(PYTHONLIB) 		\
        $(EXTRA_FRAMEWORK_FLAG) 

SHL1DEPN=
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/pyuno.flt

DEFLIB1NAME=$(TARGET)

# --- Targets ------------------------------------------------------

.IF "$(GUI)$(COM)"=="WNTGCC"
ALLTAR : \
    $(DLLDEST)$/uno.py 		\
    $(DLLDEST)$/unohelper.py	\
    $(PYUNO_MODULE)			\
    $(MISC)$/$(PYUNORC)		\
    $(LB)$/lib$(TARGET).a

$(LB)$/lib$(TARGET).a: $(MISC)$/$(TARGET).def
    dlltool --dllname $(TARGET)$(DLLPOST) --input-def=$(MISC)$/$(TARGET).def --kill-at --output-lib=$(LB)$/lib$(TARGET).a
.ELSE
ALLTAR : \
    $(DLLDEST)$/uno.py 		\
    $(DLLDEST)$/unohelper.py	\
    $(PYUNO_MODULE)			\
    $(MISC)$/$(PYUNORC)		
.ENDIF 
.ENDIF

.INCLUDE :  target.mk
.IF "$(L10N_framework)"==""
$(DLLDEST)$/%.py: %.py
    cp $? $@


.IF "$(GUI)" == "UNX"
$(PYUNO_MODULE) : $(SLO)$/pyuno_dlopenwrapper.obj
.IF "$(OS)" == "LINUX"
    @echo $(LINK) $(LINKFLAGS) $(LINKFLAGSRUNPATH_OOO) $(LINKFLAGSSHLCUI) -ldl -o $@ $(SLO)$/pyuno_dlopenwrapper.o > $(MISC)$/$(@:b).cmd
.ELIF "$(OS)" == "SOLARIS"
    @echo ld -G -ldl -o $@ $(SLO)$/pyuno_dlopenwrapper.o > $(MISC)$/$(@:b).cmd
.ELIF "$(OS)" == "FREEBSD"
    @echo ld -shared -o $@ $(SLO)$/pyuno_dlopenwrapper.o > $(MISC)$/$(@:b).cmd
.ELIF "$(OS)" == "NETBSD"
    @echo $(LINK) $(LINKFLAGSSHLCUI) -o $@ $(SLO)$/pyuno_dlopenwrapper.o > $(MISC)$/$(@:b).cmd
.ELIF "$(OS)" == "OPENBSD"
    @echo ld -shared -o $@ $(SLO)$/pyuno_dlopenwrapper.o > $(MISC)$/$(@:b).cmd
.ELIF "$(OS)" == "DRAGONFLY"
    @echo ld -shared -o $@ $(SLO)$/pyuno_dlopenwrapper.o > $(MISC)$/$(@:b).cmd
.ELIF "$(OS)" == "MACOSX"
    @echo $(CC) -bundle -ldl -o $@ $(SLO)$/pyuno_dlopenwrapper.o $(EXTRA_LINKFLAGS) $(EXTRA_FRAMEWORK_FLAG) > $(MISC)$/$(@:b).cmd
.ELSE
    @echo $(LINK) $(LINKFLAGSSHLCUI) -o $@ $(SLO)$/pyuno_dlopenwrapper.o > $(MISC)$/$(@:b).cmd
.ENDIF
    cat $(MISC)$/$(@:b).cmd
    @+source $(MISC)$/$(@:b).cmd
.ENDIF


$(MISC)$/$(PYUNORC) : pyuno
    -rm -f $@
    cat pyuno > $@ 

$(MISC)$/pyuno.flt : pyuno.flt
    -rm -f $@
    cat $? > $@
.ENDIF # L10N_framework

