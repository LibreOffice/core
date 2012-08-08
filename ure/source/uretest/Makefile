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

# The following variable can be set, if necessary (see README):
#GCCS_COMPAT := LD_PRELOAD=/lib/libgcc_s.so.1


.DELETE_ON_ERROR:


PRJ = $(OO_SDK_HOME)

include $(PRJ)/settings/settings.mk
include $(PRJ)/settings/std.mk

ifeq "$(PLATFORM)" "windows"
qt = "
qt2 =
cwd = $(subst \,/,$(shell cd))
link_output_switch = -out:
link_std_libs = kernel32.lib msvcrt.lib
ure_java_url = $(subst $(subst .,., ),%%20,$(subst \,/,$(URLPREFIX)$(OO_SDK_URE_JAVA_DIR)))
ure_unorc_url = $(subst $(subst .,., ),%%20,$(subst \,/,$(URLPREFIX)$(OO_SDK_URE_LIB_DIR)/uno.ini))
else
qt = '
qt2 = '
cwd = $(PWD)
link_output_switch = $(subst .,.,-o )
link_std_libs =
ure_java_url = $(URLPREFIX)$(OO_SDK_URE_JAVA_DIR)
ure_unorc_url = $(URLPREFIX)$(OO_SDK_URE_LIB_DIR)/unorc
endif


.PHONY: ALL
ALL: check

include $(PRJ)/settings/stdtarget.mk


.PHONY: check
check: test-cpptest test-javatest test-javanative test-clientserver

.PHONY: test-cpptest
test-cpptest: out.sdk/cppmain.uno.$(SHAREDLIB_EXT) out.sdk/types.rdb \
            out.sdk/services.rdb
	$(GCCS_COMPAT) uno \
            -c test.cpp.cppmain.Component -l $(URLPREFIX)$(cwd)/$< \
            -ro "$(subst \,/,$(URLPREFIX)$(URE_TYPES))" -ro out.sdk/types.rdb \
            -ro "$(subst \,/,$(URLPREFIX)$(URE_SERVICES))" \
            -ro out.sdk/services.rdb

.PHONY: test-javatest
test-javatest: out.sdk/javamain.uno.jar out.sdk/types.rdb out.sdk/services.rdb
	$(GCCS_COMPAT) uno \
            -c test.java.javamain.Component -l $(URLPREFIX)$(cwd)/$< \
            -ro "$(subst \,/,$(URLPREFIX)$(URE_TYPES))" -ro out.sdk/types.rdb \
            -ro "$(subst \,/,$(URLPREFIX)$(URE_SERVICES))" \
            -ro out.sdk/services.rdb

.PHONY: test-javanative
test-javanative: out.sdk/runner.jar out.sdk/javanative.jar \
            out.sdk/javanative.rc out.sdk/services.rdb out.sdk/types.rdb
	$(GCCS_COMPAT) $(SDK_JAVA) -jar out.sdk/runner.jar $(ure_java_url)/ \
            $(URLPREFIX)$(cwd)/out.sdk/javanative.jar \
            $(URLPREFIX)$(cwd)/out.sdk/javanative.rc

do_server = $(GCCS_COMPAT) uno -c test.cpp.cppserver.Component \
    -l $(URLPREFIX)$(cwd)/out.sdk/cppserver.uno.$(SHAREDLIB_EXT) \
    -ro "$(subst \,/,$(URLPREFIX)$(URE_TYPES))" -ro out.sdk/types.rdb \
    -ro "$(subst \,/,$(URLPREFIX)$(URE_SERVICES))" -ro out.sdk/services.rdb \
    -u "uno:pipe,name=ure_test;urp;server" --singleaccept
do_client = $(SDK_JAVA) -jar out.sdk/runner.jar $(ure_java_url)/ \
    $(URLPREFIX)$(cwd)/out.sdk/javaclient.jar \
    "uno:pipe,name=ure_test;urp;server"
.PHONY: test-clientserver
test-clientserver: out.sdk/cppserver.uno.$(SHAREDLIB_EXT) out.sdk/runner.jar \
            out.sdk/javaclient.jar
ifeq "$(PLATFORM)" "windows"
	echo $(subst %,%%,$(do_server)) > out.sdk/server.bat
	echo $(subst %,%%,$(do_client)) > out.sdk/client.bat
	@ echo Execute test-clientserver manually: start out.sdk\server.bat
	@ echo and then simultaneously start out.sdk\client.bat
else
	$(do_server) &
	sleep 5 && $(do_client)
endif


.PHONY: clean
clean:
	$(DELRECURSIVE) $(subst /,$(PS),out.sdk)


out.sdk/cppmain.uno.$(SHAREDLIB_EXT): out.sdk/cppmain.$(OBJ_EXT) | out.sdk
	$(LINK) $(COMP_LINK_FLAGS) $(link_output_switch)$@ $< $(LINK_LIBS) \
            $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB) $(SALHELPERLIB) \
            $(link_std_libs)

out.sdk/cppmain.$(OBJ_EXT): cppmain.cc out.sdk/cpputypes.cppumaker.flag \
            out.sdk/types.cppumaker.flag | out.sdk
	$(CC) $(CC_FLAGS) $(CC_OUTPUT_SWITCH)$@ -I$(PRJ)/include/stl \
            $(CC_INCLUDES) -Iout.sdk/include/cpputypes -Iout.sdk/include/types \
            $(CC_DEFINES) $<


out.sdk/cpptest.uno.$(SHAREDLIB_EXT): out.sdk/cpptest.$(OBJ_EXT) | out.sdk
	$(LINK) $(COMP_LINK_FLAGS) $(link_output_switch)$@ $< $(LINK_LIBS) \
            $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB) $(link_std_libs)

out.sdk/cpptest.$(OBJ_EXT): cpptest.cc out.sdk/cpputypes.cppumaker.flag \
            out.sdk/types.cppumaker.flag | out.sdk
	$(CC) $(CC_FLAGS) $(CC_OUTPUT_SWITCH)$@ -I$(PRJ)/include/stl \
            $(CC_INCLUDES) -Iout.sdk/include/cpputypes -Iout.sdk/include/types \
            $(CC_DEFINES) $<


out.sdk/cppserver.uno.$(SHAREDLIB_EXT): out.sdk/cppserver.$(OBJ_EXT) | out.sdk
	$(LINK) $(COMP_LINK_FLAGS) $(link_output_switch)$@ $< $(LINK_LIBS) \
            $(CPPULIB) $(CPPUHELPERLIB) $(SALLIB) $(link_std_libs)

out.sdk/cppserver.$(OBJ_EXT): cppserver.cc out.sdk/cpputypes.cppumaker.flag \
            out.sdk/types.cppumaker.flag | out.sdk
	$(CC) $(CC_FLAGS) $(CC_OUTPUT_SWITCH)$@ -I$(PRJ)/include/stl \
            $(CC_INCLUDES) -Iout.sdk/include/cpputypes -Iout.sdk/include/types \
            $(CC_DEFINES) $<


out.sdk/cpputypes.cppumaker.flag: | out.sdk
	$(CPPUMAKER) -O./out.sdk/include/cpputypes \
            "-Tcom.sun.star.lang.DisposedException;com.sun.star.lang.EventObject;com.sun.star.lang.XMain;com.sun.star.lang.XMultiComponentFactory;com.sun.star.lang.XMultiServiceFactory;com.sun.star.lang.XSingleComponentFactory;com.sun.star.lang.XSingleServiceFactory;com.sun.star.lang.XTypeProvider;com.sun.star.registry.XRegistryKey;com.sun.star.uno.DeploymentException;com.sun.star.uno.Exception;com.sun.star.uno.RuntimeException;com.sun.star.uno.XAggregation;com.sun.star.uno.XComponentContext;com.sun.star.uno.XCurrentContext;com.sun.star.uno.XInterface;com.sun.star.uno.XWeak" \
            -BUCR "$(URE_TYPES)"
	touch $@

out.sdk/types.cppumaker.flag: out.sdk/types.rdb | out.sdk
	$(CPPUMAKER) -O./out.sdk/include/types -BUCR $< "-X$(URE_TYPES)"
	touch $@


out.sdk/javamain.uno.jar: \
            out.sdk/class/javamain/test/java/javamain/JavaMain.class \
            out.sdk/javamain.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/javamain.mf -C out.sdk/class/javamain test

out.sdk/javamain.mf: javamain.mf.template | out.sdk
	sed -e $(qt)s~^Class-Path:$$~& tester.jar~$(qt) \
            -e $(qt)s~^UNO-Type-Path:$$~& types.jar~$(qt) $< > $@

out.sdk/class/javamain/test/java/javamain/JavaMain.class: JavaMain.java \
            out.sdk/tester.jar | out.sdk/class
	- $(DELRECURSIVE) $(subst /,$(PS),out.sdk/class/javamain)
	$(MKDIR) $(subst /,$(PS),out.sdk/class/javamain)
	$(SDK_JAVAC) $(JAVAC_FLAGS) \
            -classpath "$(CLASSPATH)$(PATH_SEPARATOR)out.sdk/tester.jar" \
            -sourcepath . -d out.sdk/class/javamain $<


out.sdk/runner.jar: out.sdk/class/runner/test/java/runner/Runner.class \
            out.sdk/runner.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/runner.mf -C out.sdk/class/runner test

out.sdk/runner.mf: runner.mf.template | out.sdk
	sed -e \
            $(qt)s~^Class-Path:$$~& $(ure_java_url)/unoloader.jar~$(qt) \
            -e $(qt)s~^UNO-Type-Path:$$~& ~$(qt) $< > $@

out.sdk/class/runner/test/java/runner/Runner.class: Runner.java | out.sdk/class
	- $(DELRECURSIVE) $(subst /,$(PS),out.sdk/class/runner)
	$(MKDIR) $(subst /,$(PS),out.sdk/class/runner)
	$(SDK_JAVAC) $(JAVAC_FLAGS) -sourcepath . -d out.sdk/class/runner $<


out.sdk/tester.jar: out.sdk/class/tester/test/java/tester/Tester.class \
            out.sdk/tester.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/tester.mf -C out.sdk/class/tester test

out.sdk/tester.mf: tester.mf.template | out.sdk
	sed -e $(qt)s~^Class-Path:$$~& types.jar~$(qt) \
            -e $(qt)s~^UNO-Type-Path:$$~& types.jar~$(qt) $< > $@

out.sdk/class/tester/test/java/tester/Tester.class: Tester.java \
            out.sdk/types.jar | out.sdk/class
	- $(DELRECURSIVE) $(subst /,$(PS),out.sdk/class/tester)
	$(MKDIR) $(subst /,$(PS),out.sdk/class/tester)
	$(SDK_JAVAC) $(JAVAC_FLAGS) \
            -classpath "$(CLASSPATH)$(PATH_SEPARATOR)out.sdk/types.jar" \
            -sourcepath . -d out.sdk/class/tester $<


out.sdk/javatest.uno.jar: \
            out.sdk/class/javatest/test/java/javatest/JavaTest.class \
            out.sdk/javatest.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/javatest.mf -C out.sdk/class/javatest test

out.sdk/javatest.mf: javatest.mf.template | out.sdk
	sed -e $(qt)s~^Class-Path:$$~& types.jar~$(qt) \
            -e $(qt)s~^UNO-Type-Path:$$~& types.jar~$(qt) $< > $@

out.sdk/class/javatest/test/java/javatest/JavaTest.class: JavaTest.java \
            out.sdk/types.jar | out.sdk/class
	- $(DELRECURSIVE) $(subst /,$(PS),out.sdk/class/javatest)
	$(MKDIR) $(subst /,$(PS),out.sdk/class/javatest)
	$(SDK_JAVAC) $(JAVAC_FLAGS) \
            -classpath "$(CLASSPATH)$(PATH_SEPARATOR)out.sdk/types.jar" \
            -sourcepath . -d out.sdk/class/javatest $<


out.sdk/javanative.jar: \
            out.sdk/class/javanative/test/java/javanative/JavaNative.class \
            out.sdk/javanative.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/javanative.mf -C out.sdk/class/javanative test

out.sdk/javanative.mf: javanative.mf.template | out.sdk
	sed -e $(qt)s~^Class-Path:$$~& tester.jar~$(qt) \
            -e $(qt)s~^UNO-Type-Path:$$~& types.jar~$(qt) $< > $@

out.sdk/class/javanative/test/java/javanative/JavaNative.class: \
            JavaNative.java out.sdk/tester.jar | out.sdk/class
	- $(DELRECURSIVE) $(subst /,$(PS),out.sdk/class/javanative)
	$(MKDIR) $(subst /,$(PS),out.sdk/class/javanative)
	$(SDK_JAVAC) $(JAVAC_FLAGS) \
            -classpath "$(CLASSPATH)$(PATH_SEPARATOR)out.sdk/tester.jar" \
            -sourcepath . -d out.sdk/class/javanative $<

out.sdk/javanative.rc: | out.sdk
	echo $(qt2)URE_INTERNAL_LIB_DIR=$${$(subst :,\u003A,$(ure_unorc_url)):URE_INTERNAL_LIB_DIR}$(qt2) > $@
	echo $(qt2)URE_INTERNAL_JAVA_DIR=$${$(subst :,\u003A,$(ure_unorc_url)):URE_INTERNAL_JAVA_DIR}$(qt2) >> $@
	echo $(qt2)UNO_TYPES=$${$(subst :,\u003A,$(ure_unorc_url)):UNO_TYPES} $(URLPREFIX)$(cwd)/out.sdk/types.rdb$(qt2) >> $@
	echo $(qt2)UNO_SERVICES=$${$(subst :,\u003A,$(ure_unorc_url)):UNO_SERVICES} $(URLPREFIX)$(cwd)/out.sdk/services.rdb$(qt2) >> $@


out.sdk/javaclient.jar: \
            out.sdk/class/javaclient/test/java/javaclient/JavaClient.class \
            out.sdk/javaclient.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/javaclient.mf -C out.sdk/class/javaclient test

out.sdk/javaclient.mf: javaclient.mf.template | out.sdk
	sed -e $(qt)s~^Class-Path:$$~& types.jar~$(qt) \
            -e $(qt)s~^UNO-Type-Path:$$~& types.jar~$(qt) $< > $@

out.sdk/class/javaclient/test/java/javaclient/JavaClient.class: \
            JavaClient.java out.sdk/types.jar | out.sdk/class
	- $(DELRECURSIVE) $(subst /,$(PS),out.sdk/class/javaclient)
	$(MKDIR) $(subst /,$(PS),out.sdk/class/javaclient)
	$(SDK_JAVAC) $(JAVAC_FLAGS) \
            -classpath "$(CLASSPATH)$(PATH_SEPARATOR)out.sdk/types.jar" \
            -sourcepath . -d out.sdk/class/javaclient $<


out.sdk/types.jar: out.sdk/types.javamaker.flag out.sdk/types.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/types.mf -C out.sdk/class/types test

out.sdk/types.mf: types.mf.template | out.sdk
	sed -e $(qt)s~^Class-Path:$$~& ~$(qt) \
            -e $(qt)s~^UNO-Type-Path:$$~& \<\>~$(qt) $< > $@

out.sdk/types.javamaker.flag: out.sdk/types.rdb | out.sdk out.sdk/class
	- $(DELRECURSIVE) $(subst /,$(PS),out.sdk/class/types)
	$(JAVAMAKER) -O./out.sdk/class/types -BUCR $< "-X$(URE_TYPES)"
	touch $@


out.sdk/types.rdb: out.sdk/types.urd | out.sdk
	$(REGMERGE) $@ /UCR $<

out.sdk/types.urd: types.idl | out.sdk
	$(IDLC) -O$(@D) -I$(IDL_DIR) -cid -we $<


out.sdk/services.rdb: out.sdk/cpptest.uno.$(SHAREDLIB_EXT) \
            out.sdk/javatest.uno.jar | out.sdk
	$(DEL) $(subst /,$(PS),$@)
	$(REGCOMP) -register -r $@ \
            -c $(URLPREFIX)$(cwd)/out.sdk/cpptest.uno.$(SHAREDLIB_EXT)
	$(REGCOMP) -register -br "$(URE_TYPES)" -br "$(URE_SERVICES)" -r $@ \
            -c $(URLPREFIX)$(cwd)/out.sdk/javatest.uno.jar


out.sdk:
	$(MKDIR) $(subst /,$(PS),$@)

out.sdk/class: | out.sdk
	$(MKDIR) $(subst /,$(PS),$@)
