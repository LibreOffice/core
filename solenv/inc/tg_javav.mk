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
# each point separated token blown up to 4 digits
JAVANUMVER:=$(shell @-$(JAVA_HOME)/bin/java $(JFLAGSNUMVERSION_CMD))

.ELSE          # "$(SOLAR_JAVA)"!=""
JAVAVER=0.0.0
JAVANUMVER=000000000000
.ENDIF          # "$(SOLAR_JAVA)"!=""
.ENDIF			# "$(L10N_framework)"==""
.ENDIF			# "$(JAVAVER)"=="" || "$(JAVALOCATION)"!="$(JAVA_HOME)"
