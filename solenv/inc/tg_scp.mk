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



#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(SCP$(TNR)TARGETN)"!=""

# try to get missing parfiles
$(PAR)/%.par : $(SOLARPARDIR)/%.par
    @@-$(MKDIRHIER) $(@:d:d)
    $(COMMAND_ECHO)$(COPY) $< $@

LOCALSCP$(TNR)FILES+=$(foreach,i,$(SCP$(TNR)FILES) $(foreach,j,$(SCP$(TNR)LINK_PRODUCT_TYPE) $(PAR)/$j/$i ))

$(SCP$(TNR)TARGETN): $(LOCALSCP$(TNR)FILES)
    @echo "Making:   " $(@:f)
    @@-$(MKDIRHIER) $(BIN)/$(SCP$(TNR)LINK_PRODUCT_TYPE)
    $(COMMAND_ECHO)$(SCPLINK) $(SCPLINKFLAGS) @@$(mktmp $(foreach,i,$(SCP$(TNR)FILES) $(subst,$(@:d:d:d), $(@:d:d))/$(i:+","))) -o $@
.ENDIF

# Anweisungen fuer das Linken
# unroll end
#######################################################

