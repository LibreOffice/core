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



# --- Force JDK14 switch  ------------------------------------------

.IF "$(USE_JDK_VERSION)" == "140"
JDK_VERSION=140
JAVA_HOME=$(JDK14PATH)

PATH!:=$(JDK14PATH)/bin$(PATH_SEPERATOR)$(PATH)
XCLASSPATH:=$(JDK14PATH)/jre/lib/rt.jar
CLASSPATH:=$(XCLASSPATH)
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk


# --- ANT build environment  ---------------------------------------

.INCLUDE : antsettings.mk

.INCLUDE : target.mk

CLASSPATH!:=$(CLASSPATH)$(PATH_SEPERATOR)$(ANT_CLASSPATH)$(PATH_SEPERATOR)$(JAVA_HOME)/lib/tools.jar
.EXPORT : CLASSPATH
.EXPORT : PATH

# --- TARGETS -----------------------------------------------------

$(CLASSDIR)/solar.properties : $(SOLARVERSION)/$(INPATH)/inc$(UPDMINOREXT)/minormkchanged.flg $(SOLARENV)/inc/ant.properties
    @echo "Making:   " $@
    @echo solar.build=$(BUILD) > $@
    @echo solar.rscversion=$(USQ)$(RSCVERSION)$(USQ) >> $@
    @echo solar.rscrevision=$(USQ)$(RSCREVISION)$(USQ) >> $@
    @echo solar.minor=$(LAST_MINOR) >> $@
    @echo solar.sourceversion=$(SOURCEVERSION) >> $@
    @echo solar.udkstamp=$(UDKSTAMP) >> $@
    @echo solar.extstamp=$(EXTSTAMP) >> $@
    @cat $(DMAKEROOT)/../ant.properties >> $@

ANTBUILD .PHONY:
    $(ANT) $(ANT_FLAGS)

clean  .PHONY:
    $(ANT) $(ANT_FLAGS) $@

prepare .PHONY:
    $(ANT) $(ANT_FLAGS) $@

main: .PHONY:
    $(ANT) $(ANT_FLAGS) $@

info: .PHONY
    $(ANT) $(ANT_FLAGS) $@

jar .PHONY:
    $(ANT) $(ANT_FLAGS) $@

compile .PHONY:
    $(ANT) $(ANT_FLAGS) $@

depend .PHONY:
    $(ANT) $(ANT_FLAGS) $@

javadoc .PHONY:
    $(ANT) $(ANT_FLAGS) $@

config .PHONY:
    $(ANT) $(ANT_FLAGS) $@

test .PHONY:
    $(ANT) $(ANT_FLAGS) $@


