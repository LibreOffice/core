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




MKFILENAME:=TG_SLO.MK

.IF "$(SLOTARGET)"!=""
$(SLOTARGET): $(SLOFILES) $(IDLSLOFILES)
.IF "$(MDB)" != ""
    @echo $(SLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo "Making:   " $(@:f)
#	@$(RM) $@
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(foreach,i,$(SLOFILES:f) $(RSLO)/$(i)) | xargs -n1 > $@
.ELSE
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(&)
.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
    @echo $(foreach,i,$(SLOFILES:f) $(RSLO)/$(i:s/.obj/.o/)) | xargs -n1 > $@
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(SLOTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(SLOTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(SLOTARGET)"!=""

.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)SLOTARGET)"!=""
$($(SECOND_BUILD)SLOTARGET): $(REAL_$(SECOND_BUILD)_SLOFILES)
.IF "$(MDB)" != ""
    @echo $(REAL_$(SECOND_BUILD)SLOTARGET)
    @echo $(&:+"\n")
.ENDIF
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(foreach,i,$(REAL_$(SECOND_BUILD)_SLOFILES:f) $(RSLO)/$(i)) | xargs -n1 > $@
.ELSE
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))

.IF "$(GUI)"=="OS2"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(&)
.ENDIF			# "$(GUI)"=="OS2"

.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
    @echo $(foreach,i,$(REAL_$(SECOND_BUILD)_SLOFILES:f) $(RSLO)/$(i:s/.obj/.o/)) | xargs -n1 > $@
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$($(SECOND_BUILD)SLOTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""
