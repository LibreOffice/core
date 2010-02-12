#*************************************************************************
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
#***********************************************************************/

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

# The following conditional is an approximation of: UPDATER set to YES and
# SHIPDRIVE set and CWS_WORK_STAMP not set and either SOL_TMP not set or
# SOLARENV set to a pathname of which SOL_TMP is not a prefix:
.IF "$(UPDATER)" == "YES" && "$(SHIPDRIVE)" != "" && \
    "$(CWS_WORK_STAMP)" == "" && "$(SOLARENV:s/$(SOL_TMP)//" == "$(SOLARENV)"
my_instsets = $(shell ls -dt \
    $(SHIPDRIVE)/$(INPATH)/OpenOffice/archive/$(WORK_STAMP)_$(UPDMINOR)_native_packed-*_$(defaultlangiso).$(BUILD))
my_instset = $(my_instsets:1)
.ELSE
my_instset = \
    $(SOLARSRC)/instsetoo_native/$(INPATH)/OpenOffice/archive/install/$(defaultlangiso)
.END

.IF "$(OS)" == "MACOSX"
my_soffice = $(MISC)/$(TARGET)/installation/opt/OpenOffice.org.app/Contents/MacOS/soffice
.ELIF "$(OS)" == "WNT"
my_soffice = `cat \
    $(MISC)/$(TARGET)/installation.flag`'/opt/OpenOffice.org 3/program/soffice.exe'
.ELSE
my_soffice = $(MISC)/$(TARGET)/installation/opt/openoffice.org3/program/soffice
.END

.IF "$(OOO_LIBRARY_PATH_VAR)" != ""
my_cppenv = \
    -env:arg-env=$(OOO_LIBRARY_PATH_VAR)"$${{$(OOO_LIBRARY_PATH_VAR)+=$$$(OOO_LIBRARY_PATH_VAR)}}"
my_javaenv = \
    -Dorg.openoffice.test.arg.env=$(OOO_LIBRARY_PATH_VAR)"$${{$(OOO_LIBRARY_PATH_VAR)+=$$$(OOO_LIBRARY_PATH_VAR)}}"
.END

# Work around Windows problems with long pathnames (see issue 50885) by
# installing into the temp directory instead of the module output tree (in which
# case $(TARGET).installation.flag contains the path to the temp installation,
# which is removed after smoketest); can be removed once issue 50885 is fixed:
.IF "$(OS)" == "WNT"
$(MISC)/$(TARGET)/installation.flag : \
        $(shell ls $(my_instset)/OOo_*_install.zip)
    $(MKDIRHIER) $(@:d)
    my_tmp=$$(cygpath -m $$(mktemp -dt ooosmoke.XXXXXX)) && \
    unzip $(my_instset)/OOo_*_install.zip -d "$$my_tmp" && \
    mv "$$my_tmp"/OOo_*_install "$$my_tmp"/opt && \
    echo "$$my_tmp" > $@
.ELSE
$(MISC)/$(TARGET)/installation.flag : \
        $(shell ls $(my_instset)/OOo_*_install.tar.gz)
    $(RM) -r $(MISC)/$(TARGET)/installation
    $(MKDIRHIER) $(MISC)/$(TARGET)/installation
    cd $(MISC)/$(TARGET)/installation && \
        $(GNUTAR) xfz $(my_instset)/OOo_*_install.tar.gz
    $(MV) $(MISC)/$(TARGET)/installation/OOo_*_install \
        $(MISC)/$(TARGET)/installation/opt
    $(TOUCH) $@
.END

cpptest .PHONY : $(MISC)/$(TARGET)/installation.flag \
        $(MISC)/$(TARGET)/services.rdb
    $(RM) -r $(MISC)/$(TARGET)/user
    $(MKDIRHIER) $(MISC)/$(TARGET)/user
    $(CPPUNITTESTER) \
        -env:UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb \
        -env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/types.rdb \
        -env:arg-path=$(my_soffice) -env:arg-user=$(MISC)/$(TARGET)/user \
        $(my_cppenv) $(OOO_CPPTEST_ARGS)
.IF "$(OS)" == "WNT"
    $(RM) -r $(MISC)/$(TARGET)/installation.flag \
        `cat $(MISC)/$(TARGET)/installation.flag`
.END

$(MISC)/$(TARGET)/services.rdb :
    $(MKDIRHIER) $(@:d)
    $(RM) $@
    $(REGCOMP) -register -r $@ -wop -c bridgefac.uno -c connector.uno \
        -c remotebridge.uno -c uuresolver.uno

.IF "$(SOLAR_JAVA)" == "TRUE"
javatest .PHONY : $(MISC)/$(TARGET)/installation.flag $(JAVATARGET)
    $(RM) -r $(MISC)/$(TARGET)/user
    $(MKDIR) $(MISC)/$(TARGET)/user
    $(JAVAI) $(JAVAIFLAGS) $(JAVACPS) \
        $(OOO_JUNIT_JAR)$(PATH_SEPARATOR)$(CLASSPATH) \
        -Dorg.openoffice.test.arg.path=$(my_soffice) \
        -Dorg.openoffice.test.arg.user=$(my_file)$(PWD)/$(MISC)/$(TARGET)/user \
        $(my_javaenv) org.junit.runner.JUnitCore \
        $(foreach,i,$(JAVATESTFILES) $(subst,/,. $(PACKAGE)).$(i:s/.java//))
.IF "$(OS)" == "WNT"
    $(RM) -r $(MISC)/$(TARGET)/installation.flag \
        `cat $(MISC)/$(TARGET)/installation.flag`
.END
.ELSE
javatest .PHONY :
    echo 'javatest needs SOLAR_JAVA=TRUE'
.END
