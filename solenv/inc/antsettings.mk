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




.IF "$(SOLAR_JAVA)"!=""

ANT_LIB*:=$(ANT_HOME)/lib

ANT_CLASSPATH:=$(ANT_LIB)/xercesImpl.jar$(PATH_SEPERATOR)$(ANT_LIB)/xml-apis.jar$(PATH_SEPERATOR)$(ANT_LIB)/ant.jar

# PATH_SEPERATOR won't work here as it is defined
# as ; for wondows (all shells)
#PATH!:=$(ANT_HOME)/bin$(PATH_SEPERATOR)$(PATH)
PATH!:=$(ANT_HOME)/bin:$(PATH)

ANT*:=$(ANT_HOME)/bin/ant
ANT_BUILDFILE*=build.xml

.IF "$(ANT_DEBUG)"==""
.IF "$(debug)"==""
ANT_DEBUG=off
.ELSE
ANT_DEBUG=on
.ENDIF
.ENDIF

.IF "$(ANT_OPT)"==""
.IF "$(optimize)"==""
ANT_OPT=off
.ELSE
ANT_OPT=on
.ENDIF
.ENDIF

.IF "$(JDK)"=="gcj"
JAVA_HOME=
.EXPORT : JAVA_HOME
.ENDIF

.IF "$(JAVACISGCJ)" == "yes"
ANT_FLAGS!:=-Dbuild.compiler=gcj -Dprj=$(PRJ) -Dprjname=$(PRJNAME) -Ddebug=$(ANT_DEBUG) \
 -Doptimize=$(ANT_OPT) -Dtarget=$(TARGET) -Dsolar.update=on -Dout=$(OUT) -Dinpath=$(INPATH) \
 -Dproext="$(PROEXT)" -Dsolar.bin=$(SOLARBINDIR) -Dsolar.jar=$(SOLARBINDIR) \
 -Dsolar.doc=$(SOLARDOCDIR) -Dcommon.jar=$(SOLARCOMMONBINDIR) \
 -Dcommon.doc=$(SOLARCOMMONDOCDIR) -Dsolar.sourceversion=$(SOURCEVERSION) \
 -Dsolar.lastminor=$(LAST_MINOR) -Dsolar.build=$(BUILD) -f $(ANT_BUILDFILE) $(ANT_FLAGS) -emacs
.ELSE
ANT_FLAGS!:=-Dprj=$(PRJ) -Dprjname=$(PRJNAME) -Ddebug=$(ANT_DEBUG) -Doptimize=$(ANT_OPT) \
 -Dtarget=$(TARGET) -Dsolar.update=on -Dout=$(OUT) -Dinpath=$(INPATH) -Dproext="$(PROEXT)" \
 -Dsolar.bin=$(SOLARBINDIR) -Dsolar.jar=$(SOLARBINDIR) -Dsolar.doc=$(SOLARDOCDIR) \
 -Dcommon.jar=$(SOLARCOMMONBINDIR) -Dcommon.doc=$(SOLARCOMMONDOCDIR) \
 -Dsolar.sourceversion=$(SOURCEVERSION) -Dsolar.lastminor=$(LAST_MINOR) \
 -Dsolar.build=$(BUILD) -f $(ANT_BUILDFILE) $(ANT_FLAGS) -emacs
.ENDIF
.ELSE # No java
ANT=
ANT_FLAGS=
.ENDIF

.IF "$(WITH_LANG)"!=""
ANT_FLAGS+=-Dsolar.langs="$(WITH_LANG)" -Dsolar.localized="true"
.ENDIF			# "$(WITH_LANG)"!=""
 
