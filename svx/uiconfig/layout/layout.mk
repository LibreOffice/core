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
# TODO: move to solenv/inc
# copies: sw/uiconfig/layout svx/uiconfig/layout

TRALAY=tralay
XML_DEST=$(DLLDEST)
XML_LANGS=$(alllangiso)

ALL_XMLS=$(foreach,i,$(XML_FILES) $(XML_DEST)/$i) $(foreach,i,$(XML_LANGS) $(foreach,j,$(XML_FILES) $(XML_DEST)/$i/$j))

# Must remove the -j (no duplicate base file names) flag
ZIPUPDATE=-u
XML_ZIP = $(PRJNAME)-layout

ALLTAR: $(XML_ZIP)

$(XML_ZIP): $(ALL_XMLS)

ZIP1DIR=$(XML_DEST)
ZIP1TARGET=$(XML_ZIP)
ZIP1LIST=$(ALL_XMLS:s@$(XML_DEST)/@@)

$(foreach,i,$(XML_LANGS) $(XML_DEST)/$i/%.xml): %.xml
    -$(MKDIR) $(@:d)
    @echo $(foreach,i,$(XML_LANGS) $(XML_DEST)/$i/%.xml): %.xml
    $(TRALAY) -m localize.sdf -o $(XML_DEST) -l $(XML_LANGS:f:t" -l ") $<

$(XML_DEST)/%.xml: %.xml
    -$(MKDIR) $(@:d)
    $(COPY) $< $@

# Don't want to overwrite filled localize.sdf with empty template
template.sdf:
    $(foreach,i,$(XML_FILES) $(TRALAY) -l en-US $i) > $@
