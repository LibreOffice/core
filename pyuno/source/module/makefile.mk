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
#.INCLUDE :  pyversion.mk
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

