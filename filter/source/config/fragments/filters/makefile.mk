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



PRJ     = ..$/..$/..$/..
TARGET  = FCFGPkgFilters
PRJNAME = filter

# -----------------------------------------------------------------------------
# include global settings
# -----------------------------------------------------------------------------

.INCLUDE: settings.mk

# -----------------------------------------------------------------------------
# include all package definition files
# -----------------------------------------------------------------------------

.INCLUDE: ..$/packagedef.mk

.INCLUDE: target.mk

ALLTAR: $(ALL_UI_FILTERS)

.IF "$(WITH_LANG)"!=""
$(DIR_LOCFRAG)$/filters$/%.xcu : %.xcu
    -$(MKDIRHIER) $(@:d)
    $(WRAPCMD) $(CFGEX) -p $(PRJNAME) -i $(@:f) -o $@ -m $(LOCALIZESDF) -l all

.IF "$(ALL_UI_FILTERS)"!=""
$(ALL_UI_FILTERS) : $(LOCALIZESDF)
.ENDIF          # "$(ALL_UI_FILTERS)"!=""

.ENDIF 			# "$(WITH_LANG)"!=""
