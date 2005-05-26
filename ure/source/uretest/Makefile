#*************************************************************************
#
#   $RCSfile: Makefile,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: sb $ $Date: 2005-05-26 09:36:11 $
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

# The following variable must be set:
#SDK_HOME := /opt/OpenOffice.org2.0_SDK

# The following variables can be set, if necessary:
#URE_HOME := /opt/openoffice.org/ure
#GCCS_COMPAT := LD_PRELOAD=/lib/libgcc_s.so.1


URE_HOME ?= /opt/openoffice.org/ure

.DELETE_ON_ERROR:


.PHONY: test
test: test-regview test-cpptest test-javatest test-clientserver

.PHONY: test-regview
test-regview: out/services.rdb
	$(URE_HOME)/bin/regview $< > /dev/null

.PHONY: test-cpptest
test-cpptest: out/cppmain.uno.so out/types.rdb out/services.rdb
	$(GCCS_COMPAT) $(URE_HOME)/bin/uno \
            -c test.cpp.cppmain.Component -l file://$(PWD)/$< \
            -ro $(URE_HOME)/share/misc/types.rdb -ro out/types.rdb \
            -ro $(URE_HOME)/share/misc/services.rdb -ro out/services.rdb

.PHONY: test-javatest
test-javatest: out/javamain.uno.jar out/types.rdb out/services.rdb
	$(GCCS_COMPAT) $(URE_HOME)/bin/uno \
            -c test.java.javamain.Component -l file://$(PWD)/$< \
            -ro $(URE_HOME)/share/misc/types.rdb -ro out/types.rdb \
            -ro $(URE_HOME)/share/misc/services.rdb -ro out/services.rdb

.PHONY: test-clientserver
test-clientserver: out/cppserver.uno.so out/runner.jar out/javaclient.jar
	$(GCCS_COMPAT) $(URE_HOME)/bin/uno -c test.cpp.cppserver.Component \
            -l file://$(PWD)/out/cppserver.uno.so \
            -ro $(URE_HOME)/share/misc/types.rdb -ro out/types.rdb \
            -ro $(URE_HOME)/share/misc/services.rdb \
            -u 'uno:pipe,name=ure_test;urp;server' --singleaccept &
	sleep 5 && \
            java -jar out/runner.jar file://$(URE_HOME)/share/java/ \
            file://$(PWD)/out/javaclient.jar 'uno:pipe,name=ure_test;urp;server'


.PHONY: clean
clean:
	rm -rf out


out/cppmain.uno.so: out/cppmain.o version.map | out out/lib/libuno_cppu.so \
            out/lib/libuno_cppuhelpergcc3.so out/lib/libuno_sal.so \
            out/lib/libuno_salhelpergcc3.so
	g++ -shared -o $@ -Wl,-z,defs -Wl,--fatal-warnings \
            -Wl,--version-script=version.map $< -Lout/lib -luno_cppu \
            -luno_cppuhelpergcc3 -luno_sal -luno_salhelpergcc3

out/cppmain.o: cppmain.cc out/cpputypes.cppumaker.flag \
            out/types.cppumaker.flag | out
	g++ -c -o $@ -fpic -Wall -Wno-ctor-dtor-privacy -I $(SDK_HOME)/include \
            -I out/include/cpputypes -I out/include/types -DCPPU_ENV=gcc3 \
            -DLINUX -DUNX $<


out/cpptest.uno.so: out/cpptest.o version.map | out \
            out/lib/libuno_cppu.so out/lib/libuno_cppuhelpergcc3.so \
            out/lib/libuno_sal.so
	g++ -shared -o $@ -Wl,-z,defs -Wl,--fatal-warnings \
            -Wl,--version-script=version.map $< -Lout/lib -luno_cppu \
            -luno_cppuhelpergcc3 -luno_sal

out/cpptest.o: cpptest.cc out/cpputypes.cppumaker.flag \
            out/types.cppumaker.flag | out
	g++ -c -o $@ -fpic -Wall -Wno-ctor-dtor-privacy -I $(SDK_HOME)/include \
            -I out/include/cpputypes -I out/include/types -DCPPU_ENV=gcc3 \
            -DLINUX -DUNX $<


out/cppserver.uno.so: out/cppserver.o version.map | out out/lib/libuno_cppu.so \
            out/lib/libuno_cppuhelpergcc3.so out/lib/libuno_sal.so
	g++ -shared -o $@ -Wl,-z,defs -Wl,--fatal-warnings \
            -Wl,--version-script=version.map $< -Lout/lib -luno_cppu \
            -luno_cppuhelpergcc3 -luno_sal

out/cppserver.o: cppserver.cc out/cpputypes.cppumaker.flag \
            out/types.cppumaker.flag | out
	g++ -c -o $@ -fpic -Wall -Wno-ctor-dtor-privacy -I $(SDK_HOME)/include \
            -I out/include/cpputypes -I out/include/types -DCPPU_ENV=gcc3 \
            -DLINUX -DUNX $<


out/cpputypes.cppumaker.flag: cpputypes | out
	LD_LIBRARY_PATH=$(URE_HOME)/lib $(SDK_HOME)/linux/bin/cppumaker \
            -O./out/include/cpputypes -T`cat cpputypes` -B/UCR \
            $(URE_HOME)/share/misc/types.rdb
	touch $@

out/types.cppumaker.flag: out/types.rdb | out
	LD_LIBRARY_PATH=$(URE_HOME)/lib $(SDK_HOME)/linux/bin/cppumaker \
            -O./out/include/types -B/UCR $< -X$(URE_HOME)/share/misc/types.rdb
	touch $@


out/javamain.uno.jar: out/class/javamain/test/java/javamain/JavaMain.class \
            out/javamain.mf | out
	jar cfm $@ out/javamain.mf -C out/class/javamain test

out/javamain.mf: javamain.mf.template | out
	sed -e 's~^Class-Path:$$~& types.jar~' \
            -e 's~^UNO-Type-Path:$$~& types.jar~' $< > $@

out/class/javamain/test/java/javamain/JavaMain.class: JavaMain.java \
            out/types.jar | out/class
	rm -rf out/class/javamain
	mkdir out/class/javamain
	javac -classpath \
            $(URE_HOME)/share/java/jurt.jar:$(URE_HOME)/share/java/ridl.jar:out/types.jar \
            -sourcepath . -d out/class/javamain $<


out/runner.jar: out/class/runner/test/java/runner/Runner.class out/runner.mf \
            | out
	jar cfm $@ out/runner.mf -C out/class/runner test

out/runner.mf: runner.mf.template | out
	sed -e \
            's~^Class-Path:$$~& file://$(URE_HOME)/share/java/unoloader.jar~' \
            -e 's~^UNO-Type-Path:$$~& ~' $< > $@

out/class/runner/test/java/runner/Runner.class: Runner.java | out/class
	rm -rf out/class/runner
	mkdir out/class/runner
	javac -classpath $(URE_HOME)/share/java/unoloader.jar -sourcepath . \
            -d out/class/runner $<


out/javatest.uno.jar: out/class/javatest/test/java/javatest/JavaTest.class \
            out/javatest.mf | out
	jar cfm $@ out/javatest.mf -C out/class/javatest test

out/javatest.mf: javatest.mf.template | out
	sed -e 's~^Class-Path:$$~& types.jar~' \
            -e 's~^UNO-Type-Path:$$~& types.jar~' $< > $@

out/class/javatest/test/java/javatest/JavaTest.class: JavaTest.java \
            out/types.jar | out/class
	rm -rf out/class/javatest
	mkdir out/class/javatest
	javac -classpath \
            $(URE_HOME)/share/java/jurt.jar:$(URE_HOME)/share/java/ridl.jar:out/types.jar \
            -sourcepath . -d out/class/javatest $<


out/javaclient.jar: out/class/javaclient/test/java/javaclient/JavaClient.class \
            out/javaclient.mf | out
	jar cfm $@ out/javaclient.mf -C out/class/javaclient test

out/javaclient.mf: javaclient.mf.template | out
	sed -e 's~^Class-Path:$$~& types.jar~' \
            -e 's~^UNO-Type-Path:$$~& types.jar~' $< > $@

out/class/javaclient/test/java/javaclient/JavaClient.class: JavaClient.java \
            out/types.jar | out/class
	rm -rf out/class/javaclient
	mkdir out/class/javaclient
	javac -classpath \
            $(URE_HOME)/share/java/juh.jar:$(URE_HOME)/share/java/ridl.jar:out/types.jar \
            -sourcepath . -d out/class/javaclient $<


out/types.jar: out/types.javamaker.flag out/types.mf | out
	jar cfm $@ out/types.mf -C out/class/types test

out/types.mf: types.mf.template | out
	sed -e 's~^Class-Path:$$~& ~' -e 's~^UNO-Type-Path:$$~& \<\>~' $< > $@

out/types.javamaker.flag: out/types.rdb | out out/class
	rm -rf out/class/types
	LD_LIBRARY_PATH=$(URE_HOME)/lib $(SDK_HOME)/linux/bin/javamaker \
            -O./out/class/types -B/UCR $< -X$(URE_HOME)/share/misc/types.rdb
	touch $@


out/types.rdb: out/types.urd | out
	$(URE_HOME)/bin/regmerge $@ /UCR $<

out/types.urd: types.idl | out
	LD_LIBRARY_PATH=$(URE_HOME)/lib $(SDK_HOME)/linux/bin/idlc -O$(@D) \
            -I$(SDK_HOME)/idl -cid -we $<


out/services.rdb: out/cpptest.uno.so out/javatest.uno.jar | out
	rm -f $@
	$(URE_HOME)/bin/regcomp -register -r $@ \
            -c file://$(PWD)/out/cpptest.uno.so
	$(URE_HOME)/bin/regcomp -register \
            -br $(URE_HOME)/share/misc/types.rdb \
            -br $(URE_HOME)/share/misc/services.rdb -r $@ \
            -c file://$(PWD)/out/javatest.uno.jar


out/lib/libuno_cppu.so: | out/lib
	ln -fs $(URE_HOME)/lib/libuno_cppu.so.3 $@

out/lib/libuno_cppuhelpergcc3.so: | out/lib
	ln -fs $(URE_HOME)/lib/libuno_cppuhelpergcc3.so.3 $@

out/lib/libuno_sal.so: | out/lib
	ln -fs $(URE_HOME)/lib/libuno_sal.so.3 $@

out/lib/libuno_salhelpergcc3.so: | out/lib
	ln -fs $(URE_HOME)/lib/libuno_salhelpergcc3.so.3 $@


out:
	mkdir $@

out/class: | out
	mkdir $@

out/lib: | out
	mkdir $@
