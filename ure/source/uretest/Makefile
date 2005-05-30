#*************************************************************************
#
#   $RCSfile: Makefile,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: sb $ $Date: 2005-05-30 07:38:01 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2005 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#*************************************************************************

# The following variable can be set, if necessary (see README):
#GCCS_COMPAT := LD_PRELOAD=/lib/libgcc_s.so.1


.DELETE_ON_ERROR:


PRJ = $(OO_SDK_HOME)

include $(PRJ)/settings/settings.mk
include $(PRJ)/settings/std.mk


.PHONY: ALL
ALL: test

include $(PRJ)/settings/stdtarget.mk


.PHONY: test
test: test-cpptest test-javatest test-clientserver

.PHONY: test-cpptest
test-cpptest: out.sdk/cppmain.uno.so out.sdk/types.rdb out.sdk/services.rdb
	$(GCCS_COMPAT) uno \
            -c test.cpp.cppmain.Component -l $(URLPREFIX)$(PWD)/$< \
            -ro $(OO_SDK_URE_HOME)/share/misc/types.rdb -ro out.sdk/types.rdb \
            -ro $(OO_SDK_URE_HOME)/share/misc/services.rdb \
            -ro out.sdk/services.rdb

.PHONY: test-javatest
test-javatest: out.sdk/javamain.uno.jar out.sdk/types.rdb out.sdk/services.rdb
	$(GCCS_COMPAT) uno \
            -c test.java.javamain.Component -l $(URLPREFIX)$(PWD)/$< \
            -ro $(OO_SDK_URE_HOME)/share/misc/types.rdb -ro out.sdk/types.rdb \
            -ro $(OO_SDK_URE_HOME)/share/misc/services.rdb \
            -ro out.sdk/services.rdb

.PHONY: test-clientserver
test-clientserver: out.sdk/cppserver.uno.so out.sdk/runner.jar \
            out.sdk/javaclient.jar
	$(GCCS_COMPAT) uno \
            -c test.cpp.cppserver.Component \
            -l $(URLPREFIX)$(PWD)/out.sdk/cppserver.uno.so \
            -ro $(OO_SDK_URE_HOME)/share/misc/types.rdb -ro out.sdk/types.rdb \
            -ro $(OO_SDK_URE_HOME)/share/misc/services.rdb \
            -u 'uno:pipe,name=ure_test;urp;server' --singleaccept &
	sleep 5 && \
            $(SDK_JAVA) -jar out.sdk/runner.jar \
            $(URLPREFIX)$(OO_SDK_URE_JAVA_DIR)/ \
            $(URLPREFIX)$(PWD)/out.sdk/javaclient.jar \
            'uno:pipe,name=ure_test;urp;server'


.PHONY: clean
clean:
	$(DELRECURSIVE) out.sdk


out.sdk/cppmain.uno.so: out.sdk/cppmain.o version.map | out.sdk
	$(LINK) $(COMP_LINK_FLAGS) -o $@ $< $(LINK_LIBS) $(CPPULIB) \
            $(CPPUHELPERLIB) $(SALLIB) $(SALHELPERLIB)

out.sdk/cppmain.o: cppmain.cc out.sdk/cpputypes.cppumaker.flag \
            out.sdk/types.cppumaker.flag | out.sdk
	$(CC) $(CC_FLAGS) $(CC_OUTPUT_SWITCH) $@ $(CC_INCLUDES) \
            -Iout.sdk/include/cpputypes -Iout.sdk/include/types \
            $(CC_DEFINES) $<


out.sdk/cpptest.uno.so: out.sdk/cpptest.o version.map | out.sdk
	$(LINK) $(COMP_LINK_FLAGS) -o $@ $< $(LINK_LIBS) $(CPPULIB) \
            $(CPPUHELPERLIB) $(SALLIB)

out.sdk/cpptest.o: cpptest.cc out.sdk/cpputypes.cppumaker.flag \
            out.sdk/types.cppumaker.flag | out.sdk
	$(CC) $(CC_FLAGS) $(CC_OUTPUT_SWITCH) $@ $(CC_INCLUDES) \
            -Iout.sdk/include/cpputypes -Iout.sdk/include/types \
            $(CC_DEFINES) $<


out.sdk/cppserver.uno.so: out.sdk/cppserver.o version.map | out.sdk
	$(LINK) $(COMP_LINK_FLAGS) -o $@ $< $(LINK_LIBS) $(CPPULIB) \
            $(CPPUHELPERLIB) $(SALLIB)

out.sdk/cppserver.o: cppserver.cc out.sdk/cpputypes.cppumaker.flag \
            out.sdk/types.cppumaker.flag | out.sdk
	$(CC) $(CC_FLAGS) $(CC_OUTPUT_SWITCH) $@ $(CC_INCLUDES) \
            -Iout.sdk/include/cpputypes -Iout.sdk/include/types \
            $(CC_DEFINES) $<


out.sdk/cpputypes.cppumaker.flag: cpputypes | out.sdk
	$(CPPUMAKER) -O./out.sdk/include/cpputypes -T`cat cpputypes` -B/UCR \
            $(OO_SDK_URE_HOME)/share/misc/types.rdb
	touch $@

out.sdk/types.cppumaker.flag: out.sdk/types.rdb | out.sdk
	$(CPPUMAKER) -O./out.sdk/include/types -B/UCR $< \
            -X$(OO_SDK_URE_HOME)/share/misc/types.rdb
	touch $@


out.sdk/javamain.uno.jar: \
            out.sdk/class/javamain/test/java/javamain/JavaMain.class \
            out.sdk/javamain.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/javamain.mf -C out.sdk/class/javamain test

out.sdk/javamain.mf: javamain.mf.template | out.sdk
	sed -e 's~^Class-Path:$$~& types.jar~' \
            -e 's~^UNO-Type-Path:$$~& types.jar~' $< > $@

out.sdk/class/javamain/test/java/javamain/JavaMain.class: JavaMain.java \
            out.sdk/types.jar | out.sdk/class
	$(DELRECURSIVE) out.sdk/class/javamain
	$(MKDIR) out.sdk/class/javamain
	$(SDK_JAVAC) $(JAVAC_FLAGS) \
            -classpath $(CLASSPATH)$(PATH_SEPARATOR)out.sdk/types.jar \
            -sourcepath . -d out.sdk/class/javamain $<


out.sdk/runner.jar: out.sdk/class/runner/test/java/runner/Runner.class \
            out.sdk/runner.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/runner.mf -C out.sdk/class/runner test

out.sdk/runner.mf: runner.mf.template | out.sdk
	sed -e \
            's~^Class-Path:$$~& $(URLPREFIX)$(OO_SDK_URE_JAVA_DIR)/unoloader.jar~' \
            -e 's~^UNO-Type-Path:$$~& ~' $< > $@

out.sdk/class/runner/test/java/runner/Runner.class: Runner.java | out.sdk/class
	$(DELRECURSIVE) out.sdk/class/runner
	$(MKDIR) out.sdk/class/runner
	$(SDK_JAVAC) $(JAVAC_FLAGS) -sourcepath . -d out.sdk/class/runner $<


out.sdk/javatest.uno.jar: \
            out.sdk/class/javatest/test/java/javatest/JavaTest.class \
            out.sdk/javatest.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/javatest.mf -C out.sdk/class/javatest test

out.sdk/javatest.mf: javatest.mf.template | out.sdk
	sed -e 's~^Class-Path:$$~& types.jar~' \
            -e 's~^UNO-Type-Path:$$~& types.jar~' $< > $@

out.sdk/class/javatest/test/java/javatest/JavaTest.class: JavaTest.java \
            out.sdk/types.jar | out.sdk/class
	$(DELRECURSIVE) out.sdk/class/javatest
	$(MKDIR) out.sdk/class/javatest
	$(SDK_JAVAC) $(JAVAC_FLAGS) \
            -classpath $(CLASSPATH)$(PATH_SEPARATOR)out.sdk/types.jar \
            -sourcepath . -d out.sdk/class/javatest $<


out.sdk/javaclient.jar: \
            out.sdk/class/javaclient/test/java/javaclient/JavaClient.class \
            out.sdk/javaclient.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/javaclient.mf -C out.sdk/class/javaclient test

out.sdk/javaclient.mf: javaclient.mf.template | out.sdk
	sed -e 's~^Class-Path:$$~& types.jar~' \
            -e 's~^UNO-Type-Path:$$~& types.jar~' $< > $@

out.sdk/class/javaclient/test/java/javaclient/JavaClient.class: \
            JavaClient.java out.sdk/types.jar | out.sdk/class
	$(DELRECURSIVE) out.sdk/class/javaclient
	$(MKDIR) out.sdk/class/javaclient
	$(SDK_JAVAC) $(JAVAC_FLAGS) \
            -classpath $(CLASSPATH)$(PATH_SEPARATOR)out.sdk/types.jar \
            -sourcepath . -d out.sdk/class/javaclient $<


out.sdk/types.jar: out.sdk/types.javamaker.flag out.sdk/types.mf | out.sdk
	$(SDK_JAR) cfm $@ out.sdk/types.mf -C out.sdk/class/types test

out.sdk/types.mf: types.mf.template | out.sdk
	sed -e 's~^Class-Path:$$~& ~' -e 's~^UNO-Type-Path:$$~& \<\>~' $< > $@

out.sdk/types.javamaker.flag: out.sdk/types.rdb | out.sdk out.sdk/class
	$(DELRECURSIVE) out.sdk/class/types
	$(JAVAMAKER) -O./out.sdk/class/types -B/UCR $< \
            -X$(OO_SDK_URE_HOME)/share/misc/types.rdb
	touch $@


out.sdk/types.rdb: out.sdk/types.urd | out.sdk
	$(REGMERGE) $@ /UCR $<

out.sdk/types.urd: types.idl | out.sdk
	$(IDLC) -O$(@D) -I$(IDL_DIR) -cid -we $<


out.sdk/services.rdb: out.sdk/cpptest.uno.so out.sdk/javatest.uno.jar | out.sdk
	$(DEL) $@
	$(REGCOMP) -register -r $@ -c $(URLPREFIX)$(PWD)/out.sdk/cpptest.uno.so
	$(REGCOMP) -register -br $(OO_SDK_URE_HOME)/share/misc/types.rdb \
            -br $(OO_SDK_URE_HOME)/share/misc/services.rdb -r $@ \
            -c $(URLPREFIX)$(PWD)/out.sdk/javatest.uno.jar


out.sdk:
	$(MKDIR) $@

out.sdk/class: | out.sdk
	$(MKDIR) $@
