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
# unroll begin

.IF "$(SRS$(TNR)NAME)"!=""

$(MISC)/$(TARGET).$(SRS$(TNR)NAME).dprr: $(LOCALIZE_ME_DEST)

$(MISC)/$(TARGET).$(SRS$(TNR)NAME).dprr: $(SRC$(TNR)FILES)
    @echo "Making:   " $(@:f)
    @@-$(RM) $(MISC)/$(TARGET).$(SRS$(TNR)NAME).dprr
    $(COMMAND_ECHO)$(RSC) $(VERBOSITY) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)/$(SRS$(TNR)NAME).srs} -fo=$@ -p=$(TARGET) $(SRC$(TNR)FILES)

.IF "$(WITH_LANG)"!=""

$(foreach,i,$(SRC$(TNR)FILES) $(COMMONMISC)/$(TARGET)/$i) : $$(@:f) $(LOCALIZESDF)  
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)-$(MKDIR) $(@:d)
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)-$(MKDIRHIER)  $(COMMONMISC)$/$(PRJNAME)
    $(COMMAND_ECHO)$(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m $(LOCALIZESDF) -l all
    $(COMMAND_ECHO)$(RENAME) $@.$(INPATH) $@
    $(COMMAND_ECHO)-$(RM) $@.$(INPATH)

$(SRS)/$(SRS$(TNR)NAME).srs: $(LOCALIZE_ME_DEST)

$(SRS)/$(SRS$(TNR)NAME).srs: $(foreach,i,$(SRC$(TNR)FILES) $(COMMONMISC)/$(TARGET)/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)/$(SRS$(TNR)NAME).srs: $(SRC$(TNR)FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo "Making:   " $(@:f)
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.$(INPATH) $@
    $(COMMAND_ECHO)-$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS$(TNR)NAME)"!=""

# unroll end
#######################################################


