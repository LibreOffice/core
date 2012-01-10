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



JAVAVERMK:=$(INCCOM)/java_ver.mk

.INCLUDE .IGNORE : $(JAVAVERMK)

.IF "$(JAVAVER)"=="" || "$(JAVALOCATION)"!="$(JAVA_HOME)"
.IF "$(L10N_framework)"==""

.IF "$(SOLAR_JAVA)"!=""
JFLAGSVERSION=-version
JFLAGSVERSION_CMD=-version $(PIPEERROR) $(AWK) -f $(SOLARENV)/bin/getcompver.awk
JFLAGSNUMVERSION_CMD=-version $(PIPEERROR) $(AWK) -v num=true -f $(SOLARENV)/bin/getcompver.awk

# that's the version known by the specific
# java version
JAVAVER:=$(shell @-$(JAVA_HOME)/bin/java $(JFLAGSVERSION_CMD))

# and a computed integer for comparing
# each point seperated token blown up to 4 digits
JAVANUMVER:=$(shell @-$(JAVA_HOME)/bin/java $(JFLAGSNUMVERSION_CMD))

.ELSE          # "$(SOLAR_JAVA)"!=""
JAVAVER=0.0.0
JAVANUMVER=000000000000
.ENDIF          # "$(SOLAR_JAVA)"!=""
.ENDIF			# "$(L10N_framework)"==""
.ENDIF			# "$(JAVAVER)"=="" || "$(JAVALOCATION)"!="$(JAVA_HOME)"
