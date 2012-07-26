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



PRJNAME=pyuno
PRJ=..

TARGET=zipcore
LIBTARGET=NO

.INCLUDE : settings.mk
.IF "$(L10N_framework)"==""
UWINAPILIB =

.IF "$(SYSTEM_PYTHON)" == "YES" || "$(GUI)" == "OS2"
systempython:
        @echo "Not building python-core because system python is being used"
.ELSE

.INCLUDE : pyversion.mk

PYDIRNAME=python-core-$(PYVERSION)
DESTROOT=$(BIN)$/$(PYDIRNAME)
.IF "$(GUI)" == "UNX"
PYTHONBINARY=$(BIN)$/python$(EXECPOST).bin
.ELSE
.IF "$(COM)" == "GCC"
PYTHONBINARY=$(DESTROOT)$/bin$/python.bin
.ELSE
PYTHONBINARY=$(DESTROOT)$/bin$/python$(EXECPOST)
.ENDIF
.ENDIF

FINDLIBFILES_TMP:=$(subst,/,$/ \
    $(shell @$(FIND) $(SOLARLIBDIR)$/python -type f| $(GREP) -v "\.pyc" |$(GREP) -v "\.py~" |$(GREP) -v .orig | $(GREP) -v _failed))
FINDLIBFILES=$(subst,$(SOLARLIBDIR)$/python, $(FINDLIBFILES_TMP))

FINDINCFILES_TMP:=$(subst,/,$/ \
    $(shell @$(FIND) $(SOLARINCDIR)$/python -type f| $(GREP) -v "\.h~" | $(GREP) -v _failed))
FINDINCFILES=$(subst,$(SOLARINCDIR)$/python, $(FINDINCFILES_TMP))

FILES=\
    $(PYTHONBINARY)	\
    $(foreach,i,$(FINDLIBFILES) $(DESTROOT)$/lib$(i)) \
    $(foreach,i,$(FINDINCFILES) $(DESTROOT)$/include$/python$(PYMAJOR).$(PYMINOR)$(i))

.IF "$(OS)" == "WNT"
APP1TARGET = python
APP1OBJS = $(OBJFILES) $(SOLARLIBDIR)$/pathutils-obj.obj
APP1STDLIBS =
APP1RPATH = BRAND
OBJFILES = $(OBJ)$/python.obj
.ENDIF


.INCLUDE: target.mk

ALLTAR: \
    $(BIN)$/$(PYDIRNAME).zip

.IF "$(GUI)" == "UNX"
ALLTAR : $(BIN)$/python.sh
$(BIN)$/python.sh : python.sh
    -rm -f $@
    cat $? > $@
    sed 's/%%PYVERSION%%/$(PYVERSION)/g' < $@ > $@.new
    mv $@.new $@
    chmod +x $@
.ENDIF

$(OBJ)$/python.obj: $(OUT)$/inc$/pyversion.hxx

$(OUT)$/inc$/pyversion.hxx: pyversion.inc
    $(SED) $(USQ)s/@/$(PYVERSION)/g$(USQ) < $< > $@

$(BIN)$/$(PYDIRNAME).zip : $(FILES)
.IF "$(GUI)" == "UNX"
.IF "$(OS)" != "MACOSX"
    cd $(DESTROOT) && find . -name '*$(DLLPOST)' | xargs strip
.ENDIF
.ENDIF
    -rm -f $@
    cd $(BIN) && zip -r $(PYDIRNAME).zip $(PYDIRNAME)

$(DESTROOT)$/lib$/% : $(SOLARLIBDIR)$/python$/%
    -$(MKDIRHIER) $(@:d) 
    -rm -f $@
    cat $< > $@

$(DESTROOT)$/include$/python$(PYMAJOR).$(PYMINOR)%: $(SOLARINCDIR)$/python$/%
    -$(MKDIRHIER) $(@:d)
    -rm -f $@
    cat $< > $@

.IF "$(GUI)"== "UNX"
$(BIN)$/python$(EXECPOST).bin : $(SOLARBINDIR)$/python$(EXECPOST)
    -$(MKDIRHIER) $(@:d)
    -rm -f $@
    cat $< > $@
.IF "$(OS)" != "MACOSX"
    strip $@
.ENDIF
    chmod +x $@
.ELSE
.IF "$(COM)" == "GCC"
$(DESTROOT)$/bin$/python.bin : $(SOLARBINDIR)$/python$(EXECPOST)
.ELSE
$(DESTROOT)$/bin$/python$(EXECPOST) : $(SOLARBINDIR)$/python$(EXECPOST)
.ENDIF
    -$(MKDIRHIER) $(@:d)
    -rm -f $@
    cat $< > $@
.ENDIF

.ENDIF
.ELSE
ivo:
    $(ECHO)
.ENDIF # L10N_framework
