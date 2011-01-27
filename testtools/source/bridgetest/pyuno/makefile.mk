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

PRJNAME=pyuno
TARGET=test
LIBTARGET=NO
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

my_components = pythonloader

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
# --- Files --------------------------------------------------------
.IF "$(L10N_framework)"==""
PYEXC=$(DLLDEST)$/python$(EXECPOST)
REGEXC=$(DLLDEST)$/regcomp$(EXECPOST)

.IF "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) $(SOLARBINDIR)/python
.ELSE                   # "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) python
.ENDIF                  # "$(SYSTEM_PYTHON)"!="YES"
.IF "$(GUI)"=="WNT"
PYTHONPATH:=$(SOLARLIBDIR)$/pyuno;$(PWD);$(SOLARLIBDIR);$(SOLARLIBDIR)$/python;$(SOLARLIBDIR)$/python$/lib-dynload
.ELSE                   # "$(GUI)"=="WNT"
PYTHONPATH:=$(SOLARLIBDIR)$/pyuno:$(PWD):$(SOLARLIBDIR):$(SOLARLIBDIR)$/python:$(SOLARLIBDIR)$/python$/lib-dynload
.ENDIF                  # "$(GUI)"=="WNT"
.EXPORT: PYTHONPATH

.IF "$(GUI)"!="WNT" && "$(GUI)"!="OS2"
TEST_ENV=export FOO=file://$(shell @pwd)$/$(DLLDEST) \
    UNO_TYPES=uno_types.rdb UNO_SERVICES=pyuno_services.rdb
.ELSE # "$(GUI)" != "WNT"
# aaaaaa, how to get the current working directory on windows ???
CWD_TMP=$(strip $(shell @echo "import os;print os.getcwd()" | $(PYTHON)))
TEST_ENV=export FOO=file:///$(strip $(subst,\,/ $(CWD_TMP)$/$(DLLDEST))) && \
        export UNO_TYPES=uno_types.rdb && export UNO_SERVICES=pyuno_services.rdb
.ENDIF  # "$(GUI)"!="WNT"
PYFILES = \
    $(DLLDEST)$/core.py			\
    $(DLLDEST)$/importer.py			\
    $(DLLDEST)$/main.py			\
    $(DLLDEST)$/impl.py			\
    $(DLLDEST)$/samplecomponent.py		\
    $(DLLDEST)$/testcomp.py			\

PYCOMPONENTS = \
    samplecomponent

ALL : 	\
    $(PYFILES)				\
    $(DLLDEST)/pyuno_services.rdb \
    doc					\
    ALLTAR
.ENDIF # L10N_framework

.INCLUDE :  target.mk
.IF "$(L10N_framework)"==""
$(DLLDEST)$/%.py: %.py
    cp $? $@

$(DLLDEST)$/python$(EXECPOST) : $(SOLARBINDIR)$/python$(EXECPOST)
    cp $? $@

$(DLLDEST)$/regcomp$(EXECPOST) : $(SOLARBINDIR)$/regcomp$(EXECPOST)
    cp $? $@

$(DLLDEST)$/pyuno_services.rdb .ERRREMOVE : \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/pyuno_services.input \
        $(my_components:^"$(SOLARXMLDIR)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/pyuno_services.input

$(MISC)/pyuno_services.input :
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

doc .PHONY:
    @echo start test with  dmake runtest

runtest : ALL
    cd $(DLLDEST) && $(TEST_ENV) && $(PYTHON) main.py
.ENDIF # L10N_framework

