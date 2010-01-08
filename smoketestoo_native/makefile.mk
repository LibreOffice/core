#*************************************************************************
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile,v $
#
# $Revision: 1.4 $
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
#***********************************************************************/

PRJ = .
PRJNAME = smoketestoo_native
TARGET = smoketest

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk

SLOFILES = $(SHL1OBJS)

SHL1TARGET = smoketest
SHL1OBJS = $(SLO)/smoketest.obj
SHL1RPATH = NONE
SHL1STDLIBS = $(CPPUHELPERLIB) $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL1VERSIONMAP = version.map
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

.IF "$(OS)" == "MACOSX"
my_path = $(MISC)/installation/opt/OpenOffice.org.app/Contents/MacOS/soffice
.ELIF "$(OS)" == "WNT"
my_path = \
    `cat $(MISC)/installation.flag`'/opt/OpenOffice.org 3/program/soffice.exe'
.ELSE
my_path = $(MISC)/installation/opt/openoffice.org3/program/soffice
.END

.IF "$(OOO_LIBRARY_PATH_VAR)" != ""
.IF "$(USE_SHELL)" == "bash"
my_arg_env = \
    -env:arg-env=$(OOO_LIBRARY_PATH_VAR)"$${{$(OOO_LIBRARY_PATH_VAR)+=$$$(OOO_LIBRARY_PATH_VAR)}}"
.ELSE
# The tcsh case is somewhat imprecise in that it treats a null environment
# variable the same as an unset one:
.IF "$($(OOO_LIBRARY_PATH_VAR))" == ""
my_arg_env = -env:arg-env=$(OOO_LIBRARY_PATH_VAR)
.ELSE
my_arg_env = -env:arg-env=$(OOO_LIBRARY_PATH_VAR)='$($(OOO_LIBRARY_PATH_VAR))'
.END
.END
.END

.IF "$(USE_SHELL)" == "bash"
my_set_tmp = my_tmp=$$(cygpath -m $$(mktemp -dt ooosmoke.XXXXXX))
.ELSE
my_set_tmp = export my_tmp `mktemp -dt ooosmoke.XXXXXX` && \
    export my_tmp `cygpath -m $my_tmp`
.END

ALLTAR: smoketest

smoketest .PHONY: $(MISC)/installation.flag $(SHL1TARGETN) \
        $(MISC)/services.rdb $(BIN)/smoketestdoc.sxw
    $(RM) -r $(MISC)/user
    $(MKDIR) $(MISC)/user
    $(CPPUNITTESTER) $(SHL1TARGETN) \
        -env:UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/services.rdb \
        -env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/types.rdb \
        -env:arg-path=$(my_path) -env:arg-user=$(MISC)/user \
        -env:arg-doc=$(BIN)/smoketestdoc.sxw $(my_arg_env)
.IF "$(OS)" == "WNT"
    $(RM) -r $(MISC)/installation.flag `cat $(MISC)/installation.flag`
.ENDIF

$(MISC)/installation.flag:
.IF "$(OS)" == "WNT"
    $(my_set_tmp) && \
    unzip \
        $(SRC_ROOT)/instsetoo_native/$(INPATH)/OpenOffice/archive/install/$(defaultlangiso)/OOo_*_install.zip \
        -d "$$my_tmp" && \
    mv "$$my_tmp"/OOo_*_install "$$my_tmp"/opt && \
    echo "$$my_tmp" > $@
.ELSE
    $(RM) -r $(MISC)/installation
    $(MKDIR) $(MISC)/installation
    cd $(MISC)/installation && $(GNUTAR) xfz \
        $(SRC_ROOT)/instsetoo_native/$(INPATH)/OpenOffice/archive/install/$(defaultlangiso)/OOo_*_install.tar.gz
    $(MV) $(MISC)/installation/OOo_*_install $(MISC)/installation/opt
    $(TOUCH) $@
.END

$(MISC)/services.rdb:
    $(RM) $@
    $(REGCOMP) -register -r $@ -wop -c bridgefac.uno -c connector.uno \
        -c remotebridge.uno -c uuresolver.uno

$(BIN)/smoketestdoc.sxw: data/smoketestdoc.sxw
    $(COPY) $< $@
