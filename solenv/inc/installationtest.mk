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

# relevant for non-product builds only, but built unconditionally
DIAGNOSTICS_CONTROL_FILE=$(MISC)/$(TARGET)/dbgsvrc
DBGSV_INIT:=$(MAKEDIR)/$(DIAGNOSTICS_CONTROL_FILE)
.EXPORT: DBGSV_INIT

.IF "$(ABORT_ON_ASSERTION)" != ""
    SAL_DIAGNOSE_ABORT:=TRUE
    .EXPORT: SAL_DIAGNOSE_ABORT
.ENDIF

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
    $(SHIPDRIVE)/$(INPATH)/OpenOffice/archive/$(WORK_STAMP)_$(LAST_MINOR)_native_packed-*_$(defaultlangiso).$(BUILD))
installationtest_instset = $(my_instsets:1)
.ELSE
installationtest_instset = \
    $(SOLARSRC)/instsetoo_native/$(INPATH)/OpenOffice/archive/install/$(defaultlangiso)
.END

.IF "$(OS)" == "WNT"
installationtest_instpath = `cat $(MISC)/$(TARGET)/installation.flag`
.ELSE
installationtest_instpath = $(SOLARVERSION)/$(INPATH)/installation$(UPDMINOREXT)
.END

.IF "$(OS)" == "MACOSX"
my_sofficepath = \
    $(installationtest_instpath)/opt/OpenOffice.org.app/Contents/MacOS/soffice
.ELIF "$(OS)" == "WNT"
my_sofficepath = \
    $(installationtest_instpath)'/opt/OpenOffice.org 3/program/soffice.exe'
.ELSE
my_sofficepath = \
    $(installationtest_instpath)/opt/openoffice.org3/program/soffice
.END

.IF "$(OOO_TEST_SOFFICE)" == ""
my_soffice = path:$(my_sofficepath)
.ELSE
my_soffice = '$(OOO_TEST_SOFFICE:s/'/'\''/)'
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
# which is removed after smoketest); can be removed once issue 50885 is fixed;
# on other platforms, a single installation to solver is created in
# smoketestoo_native:
.IF "$(OS)" == "WNT" && "$(OOO_TEST_SOFFICE)" == ""
OOO_EXTRACT_TO:=$(shell cygpath -m `mktemp -dt ooosmoke.XXXXXX`)
$(MISC)/$(TARGET)/installation.flag : $(shell \
        ls $(installationtest_instset)/OOo_*_install-arc_$(defaultlangiso).zip)
    $(COMMAND_ECHO)$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)unzip -q $(installationtest_instset)/OOo_*_install-arc_$(defaultlangiso).zip -d "$(OOO_EXTRACT_TO)"
    $(COMMAND_ECHO)mv "$(OOO_EXTRACT_TO)"/OOo_*_install-arc_$(defaultlangiso) "$(OOO_EXTRACT_TO)"/opt
    $(COMMAND_ECHO)echo "$(OOO_EXTRACT_TO)" > $@
.END

cpptest .PHONY : $(DIAGNOSTICS_CONTROL_FILE)
    $(COMMAND_ECHO)$(RM) -r $(MISC)/$(TARGET)/user
    $(COMMAND_ECHO)$(MKDIRHIER) $(MISC)/$(TARGET)/user
    $(CPPUNITTESTER) \
        -env:UNO_SERVICES=$(my_file)$(SOLARXMLDIR)/ure/services.rdb \
        -env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/types.rdb \
        -env:arg-soffice=$(my_soffice) -env:arg-user=$(MISC)/$(TARGET)/user \
        $(my_cppenv) $(TEST_ARGUMENTS:^"-env:arg-testarg.") $(CPPTEST_LIBRARY)
# As a workaround for #i111400#, ignore failure of $(RM):
    $(COMMAND_ECHO)- $(RM) -r $(MISC)/$(TARGET)/user
.IF "$(OS)" == "WNT" && "$(OOO_TEST_SOFFICE)" == ""
    $(COMMAND_ECHO)$(RM) -r $(installationtest_instpath) $(MISC)/$(TARGET)/installation.flag
cpptest : $(MISC)/$(TARGET)/installation.flag
.END

.IF "$(SOLAR_JAVA)" == "TRUE" && "$(OOO_JUNIT_JAR)" != ""
javatest_% .PHONY : $(JAVATARGET) $(DIAGNOSTICS_CONTROL_FILE)
    $(COMMAND_ECHO)$(RM) -r $(MISC)/$(TARGET)/user
    $(COMMAND_ECHO)$(MKDIRHIER) $(MISC)/$(TARGET)/user
    $(COMMAND_ECHO)$(JAVAI) $(JAVAIFLAGS) $(JAVACPS) \
        '$(OOO_JUNIT_JAR)$(PATH_SEPERATOR)$(CLASSPATH)' \
        -Dorg.openoffice.test.arg.soffice=$(my_soffice) \
        -Dorg.openoffice.test.arg.user=$(my_file)$(PWD)/$(MISC)/$(TARGET)/user \
        $(my_javaenv) $(TEST_ARGUMENTS:^"-Dorg.openoffice.test.arg.testarg.") \
        org.junit.runner.JUnitCore \
        $(subst,/,. $(PACKAGE)).$(@:s/javatest_//)
    $(RM) -r $(MISC)/$(TARGET)/user
.IF "$(OS)" == "WNT" && "$(OOO_TEST_SOFFICE)" == ""
    $(RM) -r $(installationtest_instpath) $(MISC)/$(TARGET)/installation.flag
javatest : $(MISC)/$(TARGET)/installation.flag
.END
javatest .PHONY : $(JAVATARGET) $(DIAGNOSTICS_CONTROL_FILE)
    $(COMMAND_ECHO)$(RM) -r $(MISC)/$(TARGET)/user
    $(COMMAND_ECHO)$(MKDIRHIER) $(MISC)/$(TARGET)/user
    $(COMMAND_ECHO)$(JAVAI) $(JAVAIFLAGS) $(JAVACPS) \
        '$(OOO_JUNIT_JAR)$(PATH_SEPERATOR)$(CLASSPATH)' \
        -Dorg.openoffice.test.arg.soffice=$(my_soffice) \
        -Dorg.openoffice.test.arg.user=$(my_file)$(PWD)/$(MISC)/$(TARGET)/user \
        $(my_javaenv) $(TEST_ARGUMENTS:^"-Dorg.openoffice.test.arg.testarg.") \
        org.junit.runner.JUnitCore \
        $(foreach,i,$(JAVATESTFILES) $(subst,/,. $(PACKAGE)).$(i:s/.java//))
    $(RM) -r $(MISC)/$(TARGET)/user
.IF "$(OS)" == "WNT" && "$(OOO_TEST_SOFFICE)" == ""
    $(RM) -r $(installationtest_instpath) $(MISC)/$(TARGET)/installation.flag
javatest : $(MISC)/$(TARGET)/installation.flag
.END
.ELSE
javatest .PHONY :
    @echo 'javatest needs SOLAR_JAVA=TRUE and OOO_JUNIT_JAR'
.END

# relevant for non-product builds only, but built unconditionally
$(DIAGNOSTICS_CONTROL_FILE):
    $(COMMAND_ECHO)$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)echo [output] > $@
.IF "$(ABORT_ON_ASSERTION)" != ""
    $(COMMAND_ECHO)echo error=abort >> $@
.ELSE
    $(COMMAND_ECHO)echo error=shell >> $@
.ENDIF
