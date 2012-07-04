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



# relevant for non-product builds only, but built unconditionally
.IF "$(ABORT_ON_ASSERTION)" != ""
    DBGSV_ERROR_OUT=abort
    SAL_DIAGNOSE_ABORT=TRUE
    .EXPORT: SAL_DIAGNOSE_ABORT
.ELSE
    DBGSV_ERROR_OUT=shell
.ENDIF
.EXPORT: DBGSV_ERROR_OUT

# don't allow to overwrite DBGSV_ERROR_OUT with an INI file. Otherwise, people
# might be tempted to put an DBGSV_INIT into their .bash_profile which points to a file
# delcaring to ignore all assertions completely ...
DBGSV_INIT=
.EXPORT: DBGSV_INIT

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
        ls $(installationtest_instset)/Apache_OpenOffice_*_install-arc_$(defaultlangiso).zip)
    $(COMMAND_ECHO)$(MKDIRHIER) $(@:d)
    $(COMMAND_ECHO)unzip -q $(installationtest_instset)/Apache_OpenOffice_*_install-arc_$(defaultlangiso).zip -d "$(OOO_EXTRACT_TO)"
    $(COMMAND_ECHO)mv "$(OOO_EXTRACT_TO)"/Apache_OpenOffice_*_install-arc_$(defaultlangiso) "$(OOO_EXTRACT_TO)"/opt
    $(COMMAND_ECHO)echo "$(OOO_EXTRACT_TO)" > $@
.END

cpptest .PHONY :
    $(COMMAND_ECHO)$(RM) -r $(MISC)/$(TARGET)/user
    $(COMMAND_ECHO)$(MKDIRHIER) $(MISC)/$(TARGET)/user
    $(CPPUNITTESTER) \
        -env:UNO_SERVICES=$(my_file)$(SOLARXMLDIR)/ure/services.rdb \
        -env:UNO_TYPES=$(my_file)$(SOLARBINDIR)/types.rdb \
        -env:arg-soffice=$(my_soffice) -env:arg-user=$(MISC)/$(TARGET)/user \
        $(my_cppenv) $(TEST_ARGUMENTS:^"-env:arg-testarg.") --protector \
        $(SOLARSHAREDBIN)/unoexceptionprotector$(DLLPOST) \
        unoexceptionprotector $(CPPTEST_LIBRARY)
# As a workaround for #i111400#, ignore failure of $(RM):
    $(COMMAND_ECHO)- $(RM) -r $(MISC)/$(TARGET)/user
.IF "$(OS)" == "WNT" && "$(OOO_TEST_SOFFICE)" == ""
    $(COMMAND_ECHO)$(RM) -r $(installationtest_instpath) $(MISC)/$(TARGET)/installation.flag
cpptest : $(MISC)/$(TARGET)/installation.flag
.END

.IF "$(SOLAR_JAVA)" == "TRUE" && "$(OOO_JUNIT_JAR)" != ""
javatest_% .PHONY : $(JAVATARGET)
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
javatest .PHONY : $(JAVATARGET)
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
