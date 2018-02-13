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

PRJ=..$/..$/..

PRJNAME=pyuno
TARGET=test
LIBTARGET=NO
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE

my_components = pythonloader

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(CROSS_COMPILING)"=="YES"

all:
# nothing

.ENDIF

# --- Files --------------------------------------------------------
.IF "$(DISABLE_PYTHON)" != "TRUE"
.IF "$(L10N_framework)"==""
PYEXC=$(DLLDEST)$/python$(EXECPOST)
REGEXC=$(DLLDEST)$/regcomp$(EXECPOST)

.IF "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) $(SOLARBINDIR)/python
.ELSE                   # "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) $(PYTHON_FOR_BUILD)
.ENDIF                  # "$(SYSTEM_PYTHON)"!="YES"
.IF "$(OS)"=="WNT"
PYTHONPATH:=$(SOLARLIBDIR)$/pyuno;$(PWD);$(SOLARLIBDIR);$(SOLARLIBDIR)$/python;$(SOLARLIBDIR)$/python$/lib-dynload
.ELSE                   # "$(OS)"=="WNT"
PYTHONPATH:=$(SOLARLIBDIR)$/pyuno:$(PWD):$(SOLARLIBDIR):$(SOLARLIBDIR)$/python:$(SOLARLIBDIR)$/python$/lib-dynload
.ENDIF                  # "$(OS)"=="WNT"
.EXPORT: PYTHONPATH

.IF "$(OS)"!="WNT"
TEST_ENV=export FOO=file://$(shell @pwd)$/$(DLLDEST) \
    UNO_TYPES=uno_types.rdb UNO_SERVICES=pyuno_services.rdb
.ELSE # "$(OS)" != "WNT"
# aaaaaa, how to get the current working directory on windows ???
CWD_TMP=$(strip $(shell @echo "import os;print os.getcwd()" | $(PYTHON)))
TEST_ENV=export FOO=file:///$(strip $(subst,\,/ $(CWD_TMP)$/$(DLLDEST))) && \
        export UNO_TYPES=uno_types.rdb && export UNO_SERVICES=pyuno_services.rdb
.ENDIF  # "$(OS)"!="WNT"
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
.ENDIF # DISABLE_PYTHON

.INCLUDE :  target.mk

.IF "$(DISABLE_PYTHON)" != "TRUE"
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
.ENDIF # DISABLE_PYTHON

