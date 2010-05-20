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

FILES=\
    $(PYTHONBINARY)	\
    $(foreach,i,$(FINDLIBFILES) $(DESTROOT)$/lib$(i)) 

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
