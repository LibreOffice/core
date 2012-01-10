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



MKFILENAME:=TG_OBJ.MK

.IF "$(OBJTARGET)"!=""
$(OBJTARGET): $(OBJFILES) $(IDLOBJFILES)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="OS2"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(&)
.ENDIF		# "$(GUI)"=="OS2"

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    $(ECHONL) $(foreach,i,$(OBJFILES:f) $(ROBJ)/$(i)) > $@
.ELSE			# "$(COM)"=="GCC"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF			# "$(COM)"=="GCC"
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    @echo $(foreach,i,$(OBJFILES:f) $(ROBJ)/$(i:s/.obj/.o/)) | xargs -n1 > $@
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(OBJTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(OBJTARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$(OBJTARGET)"!=""


.IF "$(SECOND_BUILD)"!=""
.IF "$($(SECOND_BUILD)OBJTARGET)"!=""
$($(SECOND_BUILD)OBJTARGET): $(REAL_$(SECOND_BUILD)_OBJFILES)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="OS2"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(&)
.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
#     $(LIBMGR) $(LIBFLAGS) $@ $(OBJFILES)
    $(ECHONL) $(foreach,i,$(REAL_$(SECOND_BUILD)_OBJFILES:f) $(ROBJ)/$(i)) > $@
.ELSE
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(&:+"\n"))
.ENDIF
.ENDIF			# "$(GUI)"=="WNT"
.IF "$(GUI)"=="UNX"
    @echo $(foreach,i,$(REAL_$(SECOND_BUILD)_OBJFILES:f) $(ROBJ)/$(i:s/.obj/.o/)) | xargs -n1 >> $@
.ENDIF			# "$(GUI)"=="UNX"
.ENDIF			# "$($(SECOND_BUILD)OBJTARGET)"!=""
.ENDIF			# "$(SECOND_BUILD)"!=""

.IF "$(GUI)"=="OS2"
.IF "$(LIBTARGET)"!="NO"
    @-$(TYPE) $(mktmp $(&:+"\n")) > $(@:s/.lib/.lin/)
.ENDIF          # "$(LIBTARGET)"!="NO"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(&)
.ENDIF			# "$(GUI)"=="OS2"
