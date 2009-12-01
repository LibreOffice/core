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
# $Revision: 1.9 $
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

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
# --- Files --------------------------------------------------------

PYEXC=$(DLLDEST)$/python$(EXECPOST)
REGEXC=$(DLLDEST)$/regcomp$(EXECPOST)

DOLLAR_SIGN=\$$
.IF "$(USE_SHELL)" != "tcsh"
DOLLAR_SIGN=$$
.ENDIF

#these are temporary
REGCOMP=$(WRAPCMD) regcomp
PYTHON=$(WRAPCMD) python

.IF "$(GUI)"!="WNT" && "$(GUI)"!="OS2"
.IF "$(USE_SHELL)"=="bash"
TEST_ENV=export FOO=file://$(shell @pwd)$/$(DLLDEST) \
    UNO_TYPES=pyuno_regcomp.rdb UNO_SERVICES=pyuno_regcomp.rdb
.ELSE
TEST_ENV=\
    setenv FOO file://$(shell @pwd)$/$(DLLDEST) && \
        setenv UNO_TYPES pyuno_regcomp.rdb && setenv UNO_SERVICES pyuno_regcomp.rdb
.ENDIF
.ELSE # "$(GUI)" != "WNT"
# aaaaaa, how to get the current working directory on windows ???
CWD_TMP=$(strip $(shell @echo "import os;print os.getcwd()" | $(PYTHON)))
.IF "$(USE_SHELL)" == "tcsh"
TEST_ENV=setenv FOO file:///$(strip $(subst,\,/ $(CWD_TMP)/$(DLLDEST))) && \
        setenv UNO_TYPES pyuno_regcomp.rdb && setenv UNO_SERVICES pyuno_regcomp.rdb
.ELSE
TEST_ENV=export FOO=file:///$(strip $(subst,\,/ $(CWD_TMP)$/$(DLLDEST))) && \
        export UNO_TYPES=pyuno_regcomp.rdb && export UNO_SERVICES=pyuno_regcomp.rdb
.ENDIF "$(USE_SHELL)" == "tcsh"

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
    $(DLLDEST)$/pyuno_regcomp.rdb		\
    doc					\
    ALLTAR

.INCLUDE :  target.mk

$(DLLDEST)$/%.py: %.py
    cp $? $@

$(DLLDEST)$/python$(EXECPOST) : $(SOLARBINDIR)$/python$(EXECPOST)
    cp $? $@

$(DLLDEST)$/regcomp$(EXECPOST) : $(SOLARBINDIR)$/regcomp$(EXECPOST)
    cp $? $@

$(DLLDEST)$/pyuno_regcomp.rdb: $(DLLDEST)$/uno_types.rdb $(SOLARBINDIR)$/pyuno_services.rdb
    -rm -f $@
    $(WRAPCMD) $(REGMERGE) $(DLLDEST)$/pyuno_regcomp.rdb / $(DLLDEST)$/uno_types.rdb $(SOLARBINDIR)$/pyuno_services.rdb

doc .PHONY:
    @echo start test with  dmake runtest

runtest : ALL
    cd $(DLLDEST) && $(TEST_ENV) && python main.py 
    cd $(DLLDEST) && $(TEST_ENV) && $(REGCOMP) -register -br pyuno_regcomp.rdb -r dummy.rdb \
            -l com.sun.star.loader.Python $(foreach,i,$(PYCOMPONENTS) -c vnd.openoffice.pymodule:$(i))
    cd $(DLLDEST) && $(TEST_ENV) && $(REGCOMP) -register -br pyuno_regcomp.rdb -r dummy2.rdb \
            -l com.sun.star.loader.Python -c vnd.sun.star.expand:$(DOLLAR_SIGN)FOO/samplecomponent.py

