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

.IF "$(LIB$(TNR)TARGET)" != ""
.IF "$(LIB$(TNR)ARCHIV)" != ""

$(LIB$(TNR)ARCHIV) :	$(LIB$(TNR)TARGET)
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @-$(RM) $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.IF "$(OS)" =="HPUX_FRAG_HR"
    @-$(RM) $(MISC)/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @echo $(LINK) +inst_close -c `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @cat $(MISC)/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
    @+source $(MISC)/$(LIB$(TNR)ARCHIV:b)_closetempl.cmd
.ENDIF
    @echo $(LIBMGR) $(LIB$(TNR)FLAGS) $(LIBFLAGS) $(LIB$(TNR)ARCHIV) `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.IF "$(OS)$(COM)"=="NETBSDGCC"
    @echo  ranlib $(LIB$(TNR)ARCHIV) >> $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ENDIF
.IF "$(OS)" == "MACOSX"
    @echo  ranlib $(LIB$(TNR)ARCHIV) >> $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ENDIF
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)$(COM)"=="WNTGCC"
    @+-$(RM) $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
    @+echo $(LIBMGR) $(LIB$(TNR)FLAGS) $(LIBFLAGS) $(LIB$(TNR)ARCHIV) `cat $(LIB$(TNR)TARGET) | sed s#'^'$(ROUT)#$(PRJ)/$(ROUT)#g` > $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
    @+echo  ranlib $(LIB$(TNR)ARCHIV) >> $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.IF "$(VERBOSE)" == "TRUE"
    @cat $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ENDIF
    @+source $(MISC)/$(LIB$(TNR)ARCHIV:b).cmd
.ELSE
    @echo just a dummy > $@
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
.ENDIF			# "$(GUI)"=="UNX"

.ENDIF			# "$(LIB$(TNR)ARCHIV)" != ""

$(LIB$(TNR)TARGET) :	$(LIB$(TNR)FILES) \
                        $(LIB$(TNR)OBJFILES) \
                        $(LIB$(TNR)DEPN)
.IF "$(VERBOSE)" == "TRUE"
    @echo ------------------------------
    @echo using: $(LIB$(TNR)FILES)
    @echo using: $(LIB$(TNR)TARGET)
    @echo ------------------------------
.ENDIF
    @echo "Making:   " $(@:f)
    @@-$(RM) $@
.IF "$(GUI)"=="UNX"
    @echo $(LIB$(TNR)OBJFILES:s/.obj/.o/) | sed "s#$(PRJ:s/./\./)/$(ROUT)#$(ROUT)#g" | xargs -n 1 > $@
    @cat /dev/null $(LIB$(TNR)FILES:s/.obj/.o/) | xargs -n 1 >> $@
    @$(RM) $(@:d)$(@:b).dump
.IF "$(OS)"=="MACOSX"
    @-nm `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ELSE
    @nm `cat $(LIB$(TNR)TARGET) | sed s\#'^'$(ROUT)\#$(PRJ)/$(ROUT)\#g` > $(@:d)$(@:b).dump
.ENDIF

.ELIF "$(GUI)"=="OS2"
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) $@ $(LIB$(TNR)FILES) $(LIB$(TNR)OBJFILES)
    @+-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB$(TNR)OBJFILES)"!=""    
    @+$(TYPE) $(mktmp $(LIB$(TNR)OBJFILES)) > $(null,$(LIB$(TNR)OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB$(TNR)OBJFILES)"!=""    
.IF "$(LIB$(TNR)FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB$(TNR)FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB$(TNR)FILES)"!=""    
    @+$(ECHONL)

.ELSE			# "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
    +$(ECHONL) $(LIB$(TNR)OBJFILES) | sed "s#$(PRJ:s/././)/$(ROUT)#$(ROUT)#g" | xargs -n1 > $@
    @+cat /dev/null $(LIB$(TNR)FILES) | xargs -n1 >> $@
.ELSE
    $(COMMAND_ECHO)$(LIBMGR) $(LIBFLAGS) /OUT:$@ @$(mktmp $(LIB$(TNR)FILES) $(LIB$(TNR)OBJFILES))
    @-$(RM) $(@:s/.lib/.lin/)
.IF "$(LIB$(TNR)OBJFILES)"!=""    
    @$(TYPE) $(mktmp $(LIB$(TNR)OBJFILES)) > $(null,$(LIB$(TNR)OBJFILES) $(NULLDEV) $(@:s/.lib/.lin/))
.ENDIF          # "$(LIB$(TNR)OBJFILES)"!=""    
.IF "$(LIB$(TNR)FILES)"!=""    
    @-$(TYPE) $(foreach,i,$(LIB$(TNR)FILES) $(i:s/.lib/.lin/)) >> $(@:s/.lib/.lin/)
.ENDIF          # "$(LIB$(TNR)FILES)"!=""    
    @$(ECHONL)
.ENDIF          # "$(LIB$(TNR)FILES)"!=""    
.ELSE			# "$(GUI)"=="WNT"
    @-$(RM) $@
  .IF "$(VERBOSE)" == "TRUE"
    @echo $(LIBMGR) r $@ $(LIB$(TNR)OBJFILES)
  .ENDIF
    $(COMMAND_ECHO)$(LIBMGR) r $@ $(LIB$(TNR)OBJFILES) $(LIB$(TNR)FILES) bla.lib
.ENDIF          # "$(GUI)"=="WNT"
.ENDIF          # "$(GUI)"=="UNX"
.ENDIF          # "$(LIB$(TNR)TARGET)" != ""

# Anweisungen fuer das LIBTARGETs
# unroll end
#######################################################

