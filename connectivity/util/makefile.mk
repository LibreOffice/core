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



PRJ = ..
PRJNAME = connectivity
TARGET = connectivity

.INCLUDE : settings.mk
.INCLUDE : target.mk

# For any given platform, for each driver .xcu (in $(MY_XCUS)) built on that
# platform (in $(MISC)/registry/data/org/openoffice/Office/DataAccess) there are
# corresponding language-specific .xcu files (in
# $(MISC)/registry/res/%/org/openoffice/Office/DataAccess).  For each language,
# all language-specific .xcu files for that language are assembled into
# $(BIN)$/fcfg_drivers_%.zip.  To meet the requirements of dmake percent rules,
# the first item from $(MY_XCUS) is arbitrarily taken to be the main
# prerequisite while all the items from $(MY_XCUS) are made into indirect
# prerequisites (harmlessly doubling the first item).

MY_XCUS := \
    $(shell cd $(MISC)/registry/data/org/openoffice/Office/DataAccess && \
    ls *.xcu)

.IF "$(MY_XCUS)" != ""

ALLTAR : $(BIN)/fcfg_drivers_{$(alllangiso)}.zip

$(BIN)/fcfg_drivers_{$(alllangiso)}.zip : \
        $(MISC)/registry/res/$$(@:b:s/fcfg_drivers_//)/org/openoffice/Office/DataAccess/{$(MY_XCUS)}

$(BIN)/fcfg_drivers_%.zip : \
        $(MISC)/registry/res/%/org/openoffice/Office/DataAccess/$(MY_XCUS:1)
    zip -j $@ \
        $(foreach,i,$(MY_XCUS) \
            $(MISC)/registry/res/$*/org/openoffice/Office/DataAccess/$i)

.ENDIF
