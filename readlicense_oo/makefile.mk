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

ALL_LICENSE=..$/LICENSE
ALL_NOTICE=..$/NOTICE
.IF "${ENABLE_CATEGORY_B}"=="YES"
    # extend the install set's LICENSE and NOTICE files
    # for content distributed under category-B licenses
    ALL_LICENSE+=..$/LICENSE_category_b
    ALL_NOTICE+=..$/NOTICE_category_b
.ENDIF
.IF "${BUNDLED_EXTENSION_BLOBS}"!=""
    # extend the install set's LICENSE and NOTICE files
    # for content distributed as mere aggregations
    ALL_LICENSE+=..$/LICENSE_aggregated
    ALL_NOTICE+=..$/NOTICE_aggregated
.ENDIF
SUM_LICENSE=$(MISC)$/SUM_LICENSE
SUM_NOTICE=$(MISC)$/SUM_NOTICE

# ------------------------------------------------------------------
.INCLUDE: target.mk
# ------------------------------------------------------------------

ALLTAR: ${SUM_LICENSE} ${SUM_NOTICE} $(SOURCELICENCES) $(fallbacklicenses) just_for_nice_optics

${SUM_LICENSE} : ${ALL_LICENSE}
    cat $< > $@

${SUM_NOTICE} : ${ALL_NOTICE}
    cat $< > $@

.IF "$(fallbacklicenses)"!=""
$(fallbacklicenses) : $(SOURCELICENCES)
    @$(ECHON) .
    @$(COPY) $(@:d)$(@:b:s/_/./:b)_$(defaultlangiso)$(@:e) $@
.ENDIF          # "$(fallbacklicenses)"!=""

just_for_nice_optics: $(fallbacklicenses)
    @$(ECHONL)

# for windows, convert line ends to CR/LF
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
