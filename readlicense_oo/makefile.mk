# *************************************************************
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
# *************************************************************
PRJ=.
PRJNAME=readlicense_oo
TARGET=source

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------

.IF "$(GUI)"=="WNT"
SYSLICBASE=license.txt license.html license.rtf
SYSLICDEST=$(MISC)$/license$/wnt
.ELIF "$(GUI)"=="OS2"
SYSLICBASE=license.txt license.html license.rtf
SYSLICDEST=$(MISC)$/license$/os2
.ELSE          # "$(GUI)"=="WNT"
SYSLICBASE=LICENSE LICENSE.html
SYSLICDEST=$(MISC)$/license$/unx
.ENDIF          # "$(GUI)"=="WNT"

SOURCELICENCES=$(foreach,i,$(SYSLICBASE) $(SYSLICDEST)$/$(i:b)_en-US$(i:e))

fallbacklicenses=$(foreach,i,{$(subst,$(defaultlangiso), $(alllangiso))} $(foreach,j,$(SYSLICBASE) $(SYSLICDEST)$/$(j:b)_$i$(j:e)))

# ------------------------------------------------------------------
.INCLUDE: target.mk
# ------------------------------------------------------------------

ALLTAR: $(SOURCELICENCES) $(fallbacklicenses) just_for_nice_optics

.IF "$(fallbacklicenses)"!=""
$(fallbacklicenses) : $(SOURCELICENCES)
    @$(ECHON) .
    @$(COPY) $(@:d)$(@:b:s/_/./:b)_$(defaultlangiso)$(@:e) $@
.ENDIF          # "$(fallbacklicenses)"!=""

just_for_nice_optics: $(fallbacklicenses)
    @$(ECHONL)

# for windows, convert linends to DOS
$(SYSLICDEST)$/license_en-US.% : source$/license$/license_en-US.%
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(PERL) -p -e 's/\r?\n$$/\r\n/' < $< > $@

# for others just copy
$(SYSLICDEST)$/LICENSE_en-US : source$/license$/license_en-US.txt
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(COPY) $< $@

$(SYSLICDEST)$/LICENSE_en-US.html : source$/license$/license_en-US.html
    @-$(MKDIRHIER) $(SYSLICDEST)
    $(COPY) $< $@
