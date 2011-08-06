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

.IF "$(SYSTEM_PYTHON)" == "YES"
systempython:
	@echo "Not building python-core because system python is being used"
.ELSE

.INCLUDE : pyversion.mk

PYDIRNAME=python-core-$(PYVERSION)
DESTROOT=$(BIN)/$(PYDIRNAME)
.IF "$(GUI)" == "UNX"
PYTHONBINARY=$(BIN)/python$(EXECPOST).bin
.ELSE
.IF "$(COM)" == "GCC"
PYTHONBINARY=$(DESTROOT)/bin/python.bin
.ELSE
PYTHONBINARY=$(DESTROOT)/bin/python$(EXECPOST)
.ENDIF
.ENDIF

.IF "$(OS)" != "MACOSX"
FINDLIBFILES:=$(subst,$(SOLARLIBDIR)/python, \
    $(shell @$(FIND) $(SOLARLIBDIR)/python -type f| $(GREP) -v "\.pyc" |$(GREP) -v "\.py~" |$(GREP) -v .orig | $(GREP) -v _failed))

FILES=\
    $(PYTHONBINARY) \
    $(foreach,i,$(FINDLIBFILES) $(DESTROOT)/lib$(i))

.IF "$(OS)" == "WNT"
APP1TARGET=python
APP1OBJS=$(OBJFILES) $(SOLARLIBDIR)/pathutils-obj.obj
APP1STDLIBS=
APP1RPATH=BRAND
OBJFILES=$(OBJ)/python.obj
.ENDIF


.INCLUDE: target.mk

ALLTAR: \
    $(BIN)/$(PYDIRNAME).zip
.ENDIF

.IF "$(GUI)" == "UNX"
ALLTAR : $(BIN)/python.sh

STRIPMAC=-e '/^NONMACSECTION/d' -e '/^MACSECTION/,$$d'
STRIPNONMAC=-e '/^NONMACSECTION/,/^MACSECTION/d'

$(BIN)/python.sh : python.sh
	$(COMMAND_ECHO)sed -e 's/%%PYVERSION%%/$(eq,$(OS),MACOSX $(PYMAJOR).$(PYMINOR) $(PYVERSION))/g' -e 's/%%OOO_LIBRARY_PATH_VAR%%/$(OOO_LIBRARY_PATH_VAR)/g' \
		$(eq,$(OS),MACOSX $(STRIPNONMAC) $(STRIPMAC)) < $? > $@
	@chmod +x $@
.ENDIF

$(OBJ)/python.obj: $(OUT)/inc/pyversion.hxx

$(OUT)/inc/pyversion.hxx: pyversion.inc
	$(SED) $(USQ)s/@/$(PYVERSION)/g$(USQ) < $< > $@

$(BIN)/$(PYDIRNAME).zip : $(FILES)
.IF "$(GUI)" == "UNX"
.IF "$(OS)" != "AIX"
	cd $(DESTROOT) && find . -name '*$(DLLPOST)' | xargs strip
.ENDIF
.ENDIF
	-rm -f $@
	cd $(BIN) && zip -r $(PYDIRNAME).zip $(PYDIRNAME)

$(DESTROOT)/lib/% : $(SOLARLIBDIR)/python/%
	-$(MKDIRHIER) $(@:d)
	-rm -f $@
	cat $< > $@

.IF "$(GUI)"== "UNX"
$(BIN)/python$(EXECPOST).bin : $(SOLARBINDIR)/python$(EXECPOST)
	-$(MKDIRHIER) $(@:d)
	-rm -f $@
	cat $< > $@
.IF "$(OS)" != "MACOSX" && "$(OS)" != "AIX"
	strip $@
.ENDIF
	chmod +x $@
.ELSE
.IF "$(COM)" == "GCC"
$(DESTROOT)/bin/python.bin : $(SOLARBINDIR)/python$(EXECPOST)
.ELSE
$(DESTROOT)/bin/python$(EXECPOST) : $(SOLARBINDIR)/python$(EXECPOST)
.ENDIF
	-$(MKDIRHIER) $(@:d)
	-rm -f $@
	cat $< > $@
.ENDIF

.ENDIF
.ELSE
.ENDIF # L10N_framework
